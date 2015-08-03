# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/tools/PowerPCtoPPE/Makefile $
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

export SUB_OBJDIR = /p2p

include img_defs.mk
include p2pfiles.mk

OBJS := $(addprefix $(OBJDIR)/, $(P2P_OBJECTS))

libp2p.a: $(OBJS)
	$(AR) crs $(OBJDIR)/libp2p.a $(OBJDIR)/*.o

.PHONY: clean p2p
p2p: $(OBJS)

$(OBJS) $(OBJS:.o=.d): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

ifneq ($(MAKECMDGOALS),clean)
include $(OBJS:.o=.d)
endif

