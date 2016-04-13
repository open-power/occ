# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ssx/pgp/Makefile $
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
# This Makefile compiles all of the SSX code required for the PgP port
# of SSX.  See the "ssx.mk" file in this directory.

include ssx.mk
include ssxpgpfiles.mk


ifeq "$(SSX_TIMER_SUPPORT)" "1"
PGP_OBJECTS += ${PGP-TIMER-C-SOURCES:.c=.o} ${PGP-TIMER-S-SOURCES:.S=.o}
endif

ifeq "$(SSX_THREAD_SUPPORT)" "1"
PGP_OBJECTS += ${PGP-THREAD-C-SOURCES:.c=.o} ${PGP-THREAD-S-SOURCES:.S=.o}
endif

ifeq "$(PGP_ASYNC_SUPPORT)" "1"
PGP_OBJECTS += ${PGP-ASYNC-C-SOURCES:.c=.o} ${PGP-ASYNC-S-SOURCES:.S=.o}
endif


all: local
	$(MAKE) -I ../pgp -C ../ssx
	$(MAKE) -I ../pgp -C ../ppc405

local: $(PGP_OBJECTS)


.PHONY : clean
clean:
	rm -f *.o *.d *.d.* *.ps *.pdf
	$(MAKE) -I ../pgp -C ../ssx    clean
	$(MAKE) -I ../pgp -C ../ppc405 clean

ifneq ($(MAKECMDGOALS),clean)
include $(PGP_OBJECTS:.o=.d) 
endif
