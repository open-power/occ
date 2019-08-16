# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/boltonlib/gpe/Makefile $
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
# This Makefile compiles all of the PK code required for the GPE port
# of PK.  See the "pk.mk" file in this directory.

#all generated files from this makefile will end up in obj/$(IMAGE_NAME)/pk
export SUB_OBJDIR = /pk

include img_defs.mk
include pkgpefiles.mk

ifeq "$(PK_TIMER_SUPPORT)" "1"
GPE_OBJECTS += ${GPE-TIMER-C-SOURCES:.c=.o} ${GPE-TIMER-S-SOURCES:.S=.o}
endif

ifeq "$(PK_THREAD_SUPPORT)" "1"
GPE_OBJECTS += ${GPE-THREAD-C-SOURCES:.c=.o} ${GPE-THREAD-S-SOURCES:.S=.o}
endif

ifeq "$(GPE_ASYNC_SUPPORT)" "1"
GPE_OBJECTS += ${GPE-ASYNC-C-SOURCES:.c=.o} ${GPE-ASYNC-S-SOURCES:.S=.o}
endif

OBJS := $(addprefix $(OBJDIR)/, $(GPE_OBJECTS))

libpk.a: kernel ppe42 trace gpe
	$(AR) crs $(OBJDIR)/libpk.a $(OBJDIR)/*.o

.PHONY: clean gpe kernel ppe42 trace
gpe: $(OBJS)

trace:
	$(MAKE) -I $(IMAGE_SRCDIR) -C ../../ppetrace

kernel:
	$(MAKE) -I $(IMAGE_SRCDIR) -C ../../pk/kernel

ppe42:
	$(MAKE) -I $(IMAGE_SRCDIR) -C ../../pk/ppe42
	$(MAKE) -I $(IMAGE_SRCDIR) -C ../../baselib


$(OBJS) $(OBJS:.o=.d): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -fr $(OBJDIR)

ifneq ($(MAKECMDGOALS),clean)
-include $(OBJS:.o=.d)
endif
