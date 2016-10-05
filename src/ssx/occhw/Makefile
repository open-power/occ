# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ssx/occhw/Makefile $
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
# This Makefile compiles all of the SSX code required for the OCC port
# of SSX.  See the "img_defs.mk" file in the top directory.

#all generated files from this makefile will end up in obj/$(IMAGE_NAME)/ssx
export SUB_OBJDIR = /ssx

include img_defs.mk
include ssxocchwfiles.mk

ifeq "$(SSX_TIMER_SUPPORT)" "1"
OCCHW_OBJECTS += ${OCCHW-TIMER-C-SOURCES:.c=.o} ${OCCHW-TIMER-S-SOURCES:.S=.o}
endif

ifeq "$(SSX_THREAD_SUPPORT)" "1"
OCCHW_OBJECTS += ${OCCHW-THREAD-C-SOURCES:.c=.o} ${OCCHW-THREAD-S-SOURCES:.S=.o}
endif

ifeq "$(OCCHW_ASYNC_SUPPORT)" "1"
OCCHW_OBJECTS += ${OCCHW-ASYNC-C-SOURCES:.c=.o} ${OCCHW-ASYNC-S-SOURCES:.S=.o}
endif

OBJS := $(addprefix $(OBJDIR)/, $(OCCHW_OBJECTS))

libssx.a: ssx ppc405 ppc32 trace occhw
	$(AR) crs $(OBJDIR)/libssx.a $(OBJDIR)/*.o

.PHONY: clean occhw ssx ppc405 ppc32 trace

occhw: $(OBJS)

trace:
	$(MAKE) -I $(IMAGE_SRCDIR) -C ../trace

ssx:
	$(MAKE) -I $(IMAGE_SRCDIR) -C ../ssx

ppc405:
	$(MAKE) -I $(IMAGE_SRCDIR) -C ../ppc405

ppc32:
	$(MAKE) -I $(IMAGE_SRCDIR) -C ../ppc32

$(OBJS) $(OBJS:.o=.d): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -fr $(OBJDIR)

ifneq ($(MAKECMDGOALS),clean)
-include $(OBJS:.o=.d)
endif
