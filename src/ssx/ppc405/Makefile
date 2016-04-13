# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ssx/ppc405/Makefile $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2014,2016
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
# This Makefile is designed to be invoked with the -I argument set to
# the location of the "ssx.mk" for the build

SSX = ..
PGP = $(SSX)/pgp

include $(PGP)/ssx.mk
include ssxppc405files.mk

ifeq "$(SSX_TIMER_SUPPORT)" "1"
PPC405_OBJECTS += ${PPC405-TIMER-C-SOURCES:.c=.o} ${PPC405-TIMER-S-SOURCES:.S=.o}
endif

ifeq "$(SSX_THREAD_SUPPORT)" "1"
PPC405_OBJECTS += ${PPC405-THREAD-C-SOURCES:.c=.o} ${PPC405-THREAD-S-SOURCES:.S=.o}
endif

ifeq "$(PPC405_MMU_SUPPORT)" "1"
PPC405_OBJECTS += ${PPC405-MMU-C-SOURCES:.c=.o} ${PPC405-MMU-S-SOURCES:.S=.o}
endif


all: local
	$(MAKE) -I ../pgp -C ../ppc32

local: $(PPC405_OBJECTS)

.PHONY : clean
clean:
	rm -f *.o *.d *.d.*
	$(MAKE) -I ../pgp -C ../ppc32 clean

ifneq ($(MAKECMDGOALS),clean)
include $(PPC405_OBJECTS:.o=.d) 
endif
