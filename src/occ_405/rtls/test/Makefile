# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occ_405/rtls/test/Makefile $
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

rtlstest_CFILES = \
        ../../common.c \
        ../../errl/errl.c \
        ../rtls.c \
        rtls_tables.c \
        main.c

all_cfiles = ${rtlstest_CFILES}

APP           =  rtlstest
APP_INCLUDES += -I../../../ssx
APP_INCLUDES += -I../../../lib 
APP_INCLUDES += -I../../incl 
APP_INCLUDES += -I../../trac 
APP_INCLUDES += -I../../errl
APP_INCLUDES += -I../../thread
APP_INCLUDES += -I../../rtls
APP_INCLUDES += -I../../aplt/incl
APP_INCLUDES += -I. 

D = -DOCC_FIRMWARE=1

SOURCES = ${all_cfiles} 
MODE    = validation

PGP_ASYNC_SUPPORT = 1

include ./app.mk

pgas: 
	 $(CC) $(CFLAGS) -c -Wa,-al -Wa,--listing-cont-lines='10'
