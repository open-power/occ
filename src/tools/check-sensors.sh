#!/bin/bash
# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/tools/check-sensors.sh $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2016,2019
# [+] International Business Machines Corp.
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.
#
# IBM_PROLOG_END_TAG

# This script checks to ensure that the lists of sensors used by
# the OCC are fully populated in order to ensure that when a sensor
# is added, it is added to all relevant sensor lists.

# Usage: check-sensors.sh objdump path/to/occ_405/sensor/
#
# A version of objdump that supports powerpc object disassembly
# must be used.

# Get file name of script
SCRIPTNAME=`basename "$0"`

# Check that we have two arguments
if [ $# -ne 2 ]; then
    echo ${SCRIPTNAME}: usage: ${SCRIPTNAME} /path/to/powerpc-objdump /path/to/obj/occ_405/
    exit 1
fi

# Get paths to required tools
WC=`which wc`
READELF=`which readelf`

# Assign arguments to coherent variables
OBJDUMP=$1
OBJ_PATH=$2

# Object files that contain the sensor lists
OCC_405_OUT=${OBJ_PATH}/occ_405.out
SENSOR_INFO=${OBJ_PATH}/sensor/sensor_info.o

# Keep track of if a sensor is missing from a list
ERROR=0

# This disassembles G_amec_sensor_list and takes advantage
# of the 4-byte data alignment that objdump uses in its
# output in addition to the fact that G_amec_sensor_list
# is an array of pointers. On the 405 processor, pointers
# are 4 bytes, so this looks for any lines in the objdump
# output that are NULL pointers, indicating either a missing
# sensor or one that was improperly implemented. We have to
# dump the full 405 binary because later gcc versions don't
# add the sensor addresses until after linking.
NUM_ZERO_ENTRIES=$(${OBJDUMP} -Dz ${OCC_405_OUT} \
      | awk -v RS= '/^[[:xdigit:]].*<G_amec_sensor_list>/' \
      | grep .long\ 0x0 \
      | ${WC} -l)

# Error message if there are missing/incomplete sensors in G_amec_sensor_list
if [ ${NUM_ZERO_ENTRIES} -ne 0 ]; then
    echo ERROR: There are ${NUM_ZERO_ENTRIES} missing entries in G_amec_sensor_list! >&2
    ERROR=1
fi

# This dumps the .rodata section of sensor_info.o, which for
# this particular file, only contains G_sensor_info. It then
# takes advantage of the fact that G_sensor_info is an array
# of sensor_info_t structures, which is 33 bytes. readelf's
# output here is rows of 16 bytes each, so if there are any
# missing sensors, there will be at least one row of all 0s.
NUM_ZERO_ENTRIES=$(${READELF} ${SENSOR_INFO} -x .rodata \
      | grep 00000000\ 00000000\ 00000000\ 00000000 \
      | ${WC} -l)

if [ ${NUM_ZERO_ENTRIES} -ne 0 ]; then
    NUM_MISSING=`expr ${NUM_ZERO_ENTRIES} / 2`
    NUM_MISSING2=`expr ${NUM_MISSING} + 1`
    RANGE=${NUM_MISSING}-${NUM_MISSING2}
    echo ERROR: There are approximately ${RANGE} missing entries in G_sensor_info! >&2
    ERROR=1
fi

exit ${ERROR}
