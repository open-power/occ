# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/hwp/lib/libcommonfiles.mk $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2015
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
#  @file libcommonfiles.mk
#
#  @brief mk for including library common object files
#
#  @page ChangeLogs Change Logs
#  @section libcommonfiles.mk
#  @verbatim
#
#
# Change Log ******************************************************************
# Flag     Defect/Feature  User        Date         Description
# ------   --------------  ----------  ------------ -----------
#
# @endverbatim
#
##########################################################################
# Object Files
##########################################################################

LIB-CPP-SOURCES += p9_common_poweronoff.C
LIB-CPP-SOURCES += p9_common_pro_epi_log.C

LIB-C-SOURCES   +=
LIB-S-SOURCES   +=

LIB_OBJECTS     += $(LIB-CPP-SOURCES:.C=.o)
LIB_OBJECTS     += $(LIB-C-SOURCES:.c=.o)
LIB_OBJECTS     += $(LIB-S-SOURCES:.S=.o)

