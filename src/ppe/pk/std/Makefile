# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/pk/std/Makefile $
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
# This Makefile compiles all of the PK code required for the STD (standard PPE) port
# of PK.  See the "pk.mk" file in this directory.

#all generated files from this makefile will end up in obj/$(IMAGE_NAME)/pk
export SUB_OBJDIR = /pk

include img_defs.mk
include pkstdfiles.mk

ifeq "$(PK_TIMER_SUPPORT)" "1"
STD_OBJECTS += ${STD-TIMER-C-SOURCES:.c=.o} ${STD-TIMER-S-SOURCES:.S=.o}
endif

ifeq "$(PK_THREAD_SUPPORT)" "1"
STD_OBJECTS += ${STD-THREAD-C-SOURCES:.c=.o} ${STD-THREAD-S-SOURCES:.S=.o}
endif

ifeq "$(STD_ASYNC_SUPPORT)" "1"
STD_OBJECTS += ${STD-ASYNC-C-SOURCES:.c=.o} ${STD-ASYNC-S-SOURCES:.S=.o}
endif

OBJS := $(addprefix $(OBJDIR)/, $(STD_OBJECTS))

libpk.a: kernel ppe42 trace std
	$(AR) crs $(OBJDIR)/libpk.a $(OBJDIR)/*.o

.PHONY: clean std kernel ppe42 trace
std: $(OBJS)

trace:
	$(MAKE) -I $(IMAGE_SRCDIR) -C ../trace

kernel:
	$(MAKE) -I $(IMAGE_SRCDIR) -C ../kernel

ppe42:
	$(MAKE) -I $(IMAGE_SRCDIR) -C ../ppe42


$(OBJS) $(OBJS:.o=.d): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -fr $(OBJDIR)

ifneq ($(MAKECMDGOALS),clean)
-include $(OBJS:.o=.d)
endif
