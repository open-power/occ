# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/ppetrace/Makefile $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2019
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
# the location of the "pk.mk" for the build

include img_defs.mk
include pktracefiles.mk

ifeq "$(PK_TIMER_SUPPORT)" "1"
PKTRACE_OBJECTS += ${PKTRACE-TIMER-C-SOURCES:.c=.o} ${PKTRACE-TIMER-S-SOURCES:.S=.o}
endif

ifeq "$(PK_THREAD_SUPPORT)" "1"
PKTRACE_OBJECTS += ${PKTRACE-THREAD-C-SOURCES:.c=.o} ${PKTRACE-THREAD-S-SOURCES:.S=.o}
endif

OBJS := $(addprefix $(OBJDIR)/, $(PKTRACE_OBJECTS))

all: $(OBJS)

$(OBJS) $(OBJS:.o=.d): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

ifneq ($(MAKECMDGOALS),clean)
-include $(OBJS:.o=.d)
endif
