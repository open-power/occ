# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/lib/ppc405lib/libppc405files.mk $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2015,2016
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

##########################################################################
# INCLUDES
##########################################################################

C-SOURCES = \
    assert.c \
	ctype.c \
	ctype_table.c \
	fgetc.c \
	polling.c \
	printf.c \
	puts.c \
	simics_stdio.c \
	sprintf.c \
	ssx_dump.c \
	ssx_io.c \
	strcasecmp.c \
	string_stream.c

S-SOURCES =  

LIBPPC405_OBJECTS = $(C-SOURCES:.c=.o) $(S-SOURCES:.S=.o)
