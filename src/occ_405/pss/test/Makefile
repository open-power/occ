# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occ_405/pss/test/Makefile $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2011,2015
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

apsstest_CFILES = \
	    ../../common.c \
	    ../../errl/errl.c \
	    ../../thread/threadSch.c \
	    ../apss.c \
	    apsstest.c
all_cfiles = ${apsstest_CFILES} 


occ_GPEFILES = ../../gpe/apss_init.S \
	       ../../gpe/apss_composite.S \
	       ../../gpe/apss_meas_read_start.S \
	       ../../gpe/apss_meas_read_cont.S \
	       ../../gpe/apss_meas_read_complete.S \
	       ../../gpe/pore_test.S
all_gpefiles = ${occ_GPEFILES}


APP     = apsstest
APP_INCLUDES += -I../../../ssx
APP_INCLUDES += -I../../../lib 
APP_INCLUDES += -I../../incl 
APP_INCLUDES += -I../../trac 
APP_INCLUDES += -I../../async
APP_INCLUDES += -I../../errl
APP_INCLUDES += -I../../gpe
APP_INCLUDES += -I../../thread
#APP_INCLUDES += -I../../aplt
#APP_INCLUDES += -I../../rtls
#APP_INCLUDES += -I../../sensor
APP_INCLUDES += -I../../pss
APP_INCLUDES += -I. 

D = -DSIMICS_MAGIC_PANIC=1

#D = -DVERIFICATION=1 \
    -DSSX_STACK_CHECK=0 \
    -DINITIALIZE_PMC=0 \
    -DINITIALIZE_SIMICS_IO=0 \
    -DINITIALIZE_RTX_IO=1 \
    -DINITIALIZE_PBA=1 \
    -DSIMICS_MAGIC_PANIC=1 \
    -DSSX_KERNEL_TRACE_ENABLE=1


SOURCES = ${all_cfiles} ${all_gpefiles}
MODE    = validation

PGP_ASYNC_SUPPORT = 1

include ./app.mk

pgas: 
	 $(CC) $(CFLAGS) -c -Wa,-al -Wa,--listing-cont-lines='10' ${all_gpefiles}
