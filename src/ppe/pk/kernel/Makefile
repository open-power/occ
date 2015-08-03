# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/pk/kernel/Makefile $
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
# This Makefile is designed to be invoked with the -I argument set to
# the location of the "pk.mk" for the build

include img_defs.mk
include pkkernelfiles.mk

ifeq "$(PK_TIMER_SUPPORT)" "1"
PK_OBJECTS += ${PK-TIMER-C-SOURCES:.c=.o}
endif

ifeq "$(PK_THREAD_SUPPORT)" "1"
PK_OBJECTS += ${PK-THREAD-C-SOURCES:.c=.o} 
endif

OBJS := $(addprefix $(OBJDIR)/, $(PK_OBJECTS))

all: $(OBJS)

$(OBJS) $(OBJS:.o=.d): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

ifneq ($(MAKECMDGOALS),clean)
include $(OBJS:.o=.d)
endif
