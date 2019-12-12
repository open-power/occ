# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occ_gpe0/Makefile $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2015,2019
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

#Pull in the definitions that affect all makefiles for this image
include img_defs.mk

#Pull in object file names for the top directory
include topfiles.mk

ifdef P2P_ENABLE
include $(P2P_SRCDIR)/p2pfiles.mk
endif

PK_MAKE_DIR := $(PK_SRCDIR)/../boltonlib/$(PPE_TYPE)
OBJS := $(addprefix $(OBJDIR)/, $(TOP_OBJECTS))
OBJDIRS = $(sort $(dir ${OBJS}))
PKLIB := $(OBJDIR)/pk/libpk.a
COMMONLIB := $(OBJDIR)/commonlib/libcommon.a
OCCLIB := $(OBJDIR)/occlib/libocc.a
LIB_DIRS += -L$(OBJDIR)/pk -L$(OBJDIR)/commonlib -L$(OBJDIR)/occlib -L$(OBJDIR)/pkbaselib
LINK_OBJS = $(OBJS) $(PKLIB) $(COMMONLIB) $(OCCLIB)
LINK_SCRIPT = $(addprefix $(OBJDIR)/, linkscript)

ifdef P2P_ENABLE
P2PLIB := $(OBJDIR)/p2p/libp2p.a
LIB_DIRS += -L$(OBJDIR)/p2p
LINK_OBJS += $(P2PLIB)
endif

#default target is to make a binary application image and ppetracepp
all: $(PPETOOLS_OBJDIR)/ppetracepp $(OBJDIR)/$(IMAGE_NAME).bin $(OBJDIR)/$(IMAGE_NAME).dis

#This removes all unecessary headers from the ELF executable
$(OBJDIR)/$(IMAGE_NAME).bin $(OBJDIR)/$(IMAGE_NAME).dis: $(OBJDIR)/$(IMAGE_NAME).out
	$(OBJCOPY) -O binary  $< $(OBJDIR)/$(IMAGE_NAME).bin
	$(OBJDUMP) -S $< > $(OBJDIR)/$(IMAGE_NAME).dis

#create a linked ELF executable
$(OBJDIR)/$(IMAGE_NAME).out: $(LINK_OBJS) $(LINK_SCRIPT)
	$(LD) -e __system_reset -N -T$(LINK_SCRIPT) -Map $(OBJDIR)/$(IMAGE_NAME).map \
	-Bstatic -o $(OBJDIR)/$(IMAGE_NAME).out $(LIB_DIRS) $(OBJS) \
	-locc -lcommon -lpk -lpkbase

#pass the link command file through the C preprocessor to evaluate macros and remove comments
$(LINK_SCRIPT): link.cmd
	$(CPP) -E -x c -P $(DEFS) link.cmd -o $(LINK_SCRIPT)

#Create an obj directory if needed
$(LINK_OBJS) $(OBJS) $(OBJS:.o=.d): | $(OBJDIRS)

$(OBJDIRS):
	mkdir -p $(OBJDIRS)

$(PPETOOLS_OBJDIR)/ppetracepp: $(PPETOOLS_OBJDIR)
	g++ -O3 -w -g -I$(PPETRACEPP_DIR)/.. -I$(PPETRACEPP_DIR)/ $(PPETRACEPP_DIR)/ppetracepp.C -o $(PPETOOLS_OBJDIR)/ppetracepp

$(PPETOOLS_OBJDIR):
	mkdir -p $(PPETOOLS_OBJDIR)

.PHONY: clean $(PKLIB) $(P2PLIB)

#Build macro-specific kernel code
$(PKLIB):
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(PK_MAKE_DIR)

#Build the code that is common for all processors (PPEs and 405)
$(COMMONLIB):
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(COMMONLIB_SRCDIR)

#Build the code that is common for all OCC processors (GPEs and 405)
$(OCCLIB):
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(OCCLIB_SRCDIR)

ifdef P2P_ENABLE
$(P2PLIB):
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(P2P_SRCDIR)
endif

# collect all of the trace hash files for this image into a single trexStringFile
.PHONY : tracehash
tracehash:
	mkdir -p $(OBJDIR)
	$(THASH) -c -d $(OBJDIR) -s $(OBJDIR)/trexStringFile

#clean the kernel directory first, then the application level clean
clean:
	rm -fr $(OBJDIR)

#Add dependencies to header files
ifneq ($(MAKECMDGOALS),clean)
-include $(OBJS:.o=.d)
endif
