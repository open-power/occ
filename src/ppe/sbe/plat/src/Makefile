# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/sbe/plat/src/Makefile $
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
# This Makefile is designed to be invoked with the -I argument

export SUB_OBJDIR = /fapi2

include img_defs.mk
include fapi2sbefiles.mk


OBJS := $(addprefix $(OBJDIR)/, $(FAPI2LIB_OBJECTS))

libfapi2.a: fapi2 hwpf
	$(AR) crs $(OBJDIR)/libfapi2.a $(OBJDIR)/*.o

.PHONY: clean fapi2 hwpf
fapi2: $(OBJS)

#plat:
#	$(MAKE) -I $(IMAGE_SRCDIR) -C $(PLAT_FAPI2_DIR)

hwpf:
	@echo "Processing fapi2 hwpf makefile"
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(PPE_FAPI2_DIR)/src

$(OBJS) $(OBJS:.o=.d): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

ifneq ($(MAKECMDGOALS),clean)
include $(OBJS:.o=.d) 
endif

