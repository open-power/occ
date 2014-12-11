# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occ/thread/test/Makefile $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2011,2014
# [+] Google Inc.
# [+] International Business Machines Corp.
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.
#
# IBM_PROLOG_END_TAG

threadtest_CFILES = \
            ../../common.c \
            ../../errl/errl.c \
            ../threadSch.c \
        threadtest.c

all_cfiles = ${threadtest_CFILES}

APP     = threadtest
APP_INCLUDES += -I../../../ssx
APP_INCLUDES += -I../../../lib 
APP_INCLUDES += -I../../incl 
APP_INCLUDES += -I../../trac 
APP_INCLUDES += -I../../errl
APP_INCLUDES += -I../../thread
APP_INCLUDES += -I../../aplt/incl
APP_INCLUDES += -I. 



#D = -DVERIFICATION=1 \
    -DSSX_STACK_CHECK=0 \
    -DINITIALIZE_PMC=0 \
    -DINITIALIZE_SIMICS_IO=0 \
    -DINITIALIZE_RTX_IO=1 \
    -DINITIALIZE_PBA=1 \
    -DSIMICS_MAGIC_PANIC=1 \
    -DSSX_KERNEL_TRACE_ENABLE=1


SOURCES = ${all_cfiles} 
MODE    = validation

PGP_ASYNC_SUPPORT = 1

include ./app.mk

pgas: 
	 $(CC) $(CFLAGS) -c -Wa,-al -Wa,--listing-cont-lines='10'
