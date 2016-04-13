# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ssx/ssx/Makefile $
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

include ../pgp/ssx.mk
include ssxssxfiles.mk

ifeq "$(SSX_TIMER_SUPPORT)" "1"
SSX_OBJECTS += ${SSX-TIMER-C-SOURCES:.c=.o}
endif

ifeq "$(SSX_THREAD_SUPPORT)" "1"
SSX_OBJECTS += ${SSX-THREAD-C-SOURCES:.c=.o} 
endif

all: $(SSX_OBJECTS)

.PHONY : clean
clean:
	rm -f *.o *.d *.d.*

ifneq ($(MAKECMDGOALS),clean)
include $(SSX_OBJECTS:.o=.d)
endif
