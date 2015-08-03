# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/lib/ppc405lib/libppc405files.mk $
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
#  @file libppc405files.mk
#
#  @brief mk for libppc405.a object files
#
#  @page ChangeLogs Change Logs
#  @section libppc405files.mk
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
# INCLUDES
##########################################################################

C-SOURCES = \
    assert.c \
	byte_pool.c \
	ctype.c \
	ctype_table.c \
	fgetc.c \
	initcall.c \
	lfsr.c \
	mutex.c \
	periodic_semaphore.c \
	polling.c \
	printf.c \
	progress.c \
	puts.c \
	rtx_stdio.c \
	simics_stdio.c \
	sprintf.c \
	ssx_dump.c \
	ssx_io.c \
	stdlib.c \
	strcasecmp.c \
	strdup.c \
	string.c \
	string_stream.c \
	strtox.c \
	sxlock.c \
	time.c

S-SOURCES =  

LIBPPC405_OBJECTS = $(C-SOURCES:.c=.o) $(S-SOURCES:.S=.o)
