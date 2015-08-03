# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/hwp/perv/Makefile $
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

# This Makefile compiles all of the core hardware procedure code. See the
# "pervfiles.mk" file in this directory.

#all generated files from this makefile will end up in obj/perv
export SUB_OBJDIR = /perv

include img_defs.mk
include pervfiles.mk


OBJS := $(addprefix $(OBJDIR)/, $(PERV_OBJECTS))

libperv.a: perv
	$(AR) crs $(OBJDIR)/libperv.a $(OBJDIR)/*.o

.PHONY: clean perv
perv: $(OBJS)

$(OBJS) $(OBJS:.o=.d): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -fr $(OBJDIR)

ifneq ($(MAKECMDGOALS),clean)
include $(OBJS:.o=.d)
endif
