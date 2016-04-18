# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ssx/trace/Makefile $
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
# This Makefile is designed to be invoked with the -I argument set to
# the location of the "ssx.mk" for the build

include img_defs.mk
include ssxtracefiles.mk

ifeq "$(SSX_TIMER_SUPPORT)" "1"
SSXTRACE_OBJECTS += ${SSXTRACE-TIMER-C-SOURCES:.c=.o} ${SSXTRACE-TIMER-S-SOURCES:.S=.o}
endif

ifeq "$(SSX_THREAD_SUPPORT)" "1"
SSXTRACE_OBJECTS += ${SSXTRACE-THREAD-C-SOURCES:.c=.o} ${SSXTRACE-THREAD-S-SOURCES:.S=.o}
endif

OBJS := $(addprefix $(OBJDIR)/, $(SSXTRACE_OBJECTS))

all: $(OBJS)

$(OBJS) $(OBJS:.o=.d): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

ifneq ($(MAKECMDGOALS),clean)
include $(OBJS:.o=.d) 
endif
