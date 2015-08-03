# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/sbe/image/Makefile $
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
#remove this once we have a real compiler
export P2P_ENABLE = 1

#remove this once we have a real compiler
export SBEFW_ENABLE = 1

#Pull in the definitions that affect all makefiles for this image
include img_defs.mk

#Pull in object file names for the top directory
include topfiles.mk

ifdef P2P_ENABLE
include $(P2P_SRCDIR)/p2pfiles.mk
endif

#Pull in PPE complier libraries .  
# Sets up the following variables:
#   $(GCCLIBS)  ->  Libary objects to be linked
#   $(LIB_DIRS) ->  Include directories 
include ../../include/ppe_compiler.mk


PK_MAKE_DIR := $(PK_SRCDIR)/$(PPE_TYPE)
LIB_DIRS += -L$(OBJDIR)/pk
PKLIB := $(OBJDIR)/pk/libpk.a
LLIBS += -lpk

SBEFW_MAKE_DIR := ../sbefw
LIB_DIRS += -L$(OBJDIR)/sbefw
SBEFWLIB := $(OBJDIR)/sbefw/libsbefw.a
LLIBS += -lsbefw

SAMPLE_MAKE_DIR := ../sample
LIB_DIRS += -L$(OBJDIR)/sample
SAMPLELIB := $(OBJDIR)/sample/libsample.a
#LLIBS += -lsample

# FAPI2 library
FAPI2_MAKE_DIR := $(PLAT_FAPI2_DIR)
LIB_DIRS += -L$(OBJDIR)/fapi2
FAPI2LIB := $(OBJDIR)/fapi2/libfapi2.a
LLIBS += -lfapi2

ifdef P2P_ENABLE
LIB_DIRS += -L$(OBJDIR)/p2p
P2PLIB := $(OBJDIR)/p2p/libp2p.a
LINK_OBJS += $(P2PLIB)
LLIBS += -lp2p
endif

# HWP LIB library
HWPLIB_MAKE_DIR := $(HWPLIB_SRCDIR)
LIB_DIRS += -L$(OBJDIR)/lib
HWPLIB := $(OBJDIR)/libcommon.a
LLIBS += -lcommon

# Common Cache HWP Exit library
CACHE_MAKE_DIR := $(CACHE_SRCDIR)
LIB_DIRS += -L$(OBJDIR)/cache
CACHELIB := $(OBJDIR)/cache/libcache.a
LLIBS += -lcache


# Common Core libraries
CORE_MAKE_DIR := $(CORE_SRCDIR)
LIB_DIRS += -L$(OBJDIR)/core
CORELIB := $(OBJDIR)/core/libcore.a
LLIBS += -lcore

# Common Perv libraries
PERV_MAKE_DIR := $(PERV_SRCDIR)
LIB_DIRS += -L$(OBJDIR)/perv
PERVLIB := $(OBJDIR)/perv/libperv.a
LLIBS += -lperv

LINK_OBJS = $(OBJS) $(PKLIB) $(SBEFWLIB) $(FAPI2LIB) $(CACHELIB) $(CORELIB) $(HWPLIB) $(P2PLIB) $(PERVLIB)

# Define the objects
OBJS := $(addprefix $(OBJDIR)/, $(TOP_OBJECTS))
LINK_OBJS += $(OBJS) $(GCCLIBS)
#LINK_OBJS += $(OBJS) $(PKLIB) $(CPPLIB) $(PPELIB)
LINK_SCRIPT = $(addprefix $(OBJDIR)/, linkscript)



#default target is to make a binary application image
#This removes all unecessary headers from the ELF executable
$(OBJDIR)/$(IMAGE_NAME).bin $(OBJDIR)/$(IMAGE_NAME).dis: $(OBJDIR)/$(IMAGE_NAME).out
	$(OBJCOPY) -O binary  $< $(OBJDIR)/$(IMAGE_NAME).bin --pad-to 0x`/usr/bin/nm $(OBJDIR)/$(IMAGE_NAME).out | grep "A _sbe_base_end" | cut -d " " -f 1`
	$(OBJDUMP) -S $< > $(OBJDIR)/$(IMAGE_NAME).dis

#create a linked ELF executable
$(OBJDIR)/$(IMAGE_NAME).out: $(LINK_OBJS) $(LINK_SCRIPT)
	$(LD) -e __system_reset  -T$(LINK_SCRIPT) -Map $(OBJDIR)/$(IMAGE_NAME).map -Bstatic -o $(OBJDIR)/$(IMAGE_NAME).out $(LIB_DIRS) $(OBJS)  --start-group  $(LLIBS) --end-group  
#	$(LD) -e __system_reset  -T$(LINK_SCRIPT) -Map $(OBJDIR)/$(IMAGE_NAME).map -Bstatic -o $(OBJDIR)/$(IMAGE_NAME).out $(LIB_DIRS) $(OBJS) -lpk -lp2p #-lcommon
#	$(LD) -e __system_reset  -T$(LINK_SCRIPT) -Map $(OBJDIR)/$(IMAGE_NAME).map -Bstatic -o $(OBJDIR)/$(IMAGE_NAME).out $(LIB_DIRS) $(OBJS) $(LLIBS)


#pass the link command file through the C preprocessor to evaluate macros and remove comments
$(LINK_SCRIPT): link.cmd
	$(CPP) -E -x c++ -P $(DEFS) link.cmd -o $(LINK_SCRIPT)

#Create an obj directory if needed
$(LINK_OBJS) $(OBJS) $(OBJS:.o=.d): | $(OBJDIR)

$(OBJDIR):
	$(BASE_FAPI2_DIR)/tools/ppeParseProcSbeFixed.pl . $(TOOLS_ATTR_DIR)/p9_ppe_attributes1.xml $(ATTRFILES)
	$(BASE_FAPI2_DIR)/tools/ppeParseAttributeInfo.pl $(PPE_FAPI2_DIR)/include $(TOOLS_ATTR_DIR)/p9_ppe_attributes1.xml $(ATTRFILES)
	$(BASE_FAPI2_DIR)/tools/ppeCreateAttrGetSetMacros.pl -path $(BASE_FAPI2_DIR)/tools -inc $(PPE_FAPI2_DIR)/include -src $(PPE_FAPI2_DIR)/src
	$(BASE_FAPI2_DIR)/tools/ppeCreateIfAttrService.pl $(PPE_FAPI2_DIR)/include $(TOOLS_ATTR_DIR)/p9_ppe_attributes1.xml $(ATTRFILES)
	mkdir -p $(OBJDIR)

.PHONY: clean topfixedheaders $(PKLIB) $(P2PLIB) $(SBEFWLIB) $(PPELIB) $(FAPI2LIB) $(CACHELIB) $(CORELIB) $(PERVLIB) $(HWPLIB)

ATTRFILES += $(TOOLS_ATTR_DIR)/perv_attributes.xml
ATTRFILES += $(TOOLS_ATTR_DIR)/proc_attributes.xml
ATTRFILES += $(TOOLS_ATTR_DIR)/ex_attributes.xml
ATTRFILES += $(TOOLS_ATTR_DIR)/eq_attributes.xml
ATTRFILES += $(TOOLS_ATTR_DIR)/core_attributes.xml

topfixedheaders:
	$(BASE_FAPI2_DIR)/tools/ppeParseProcSbeFixed.pl . $(TOOLS_ATTR_DIR)/p9_ppe_attributes1.xml $(ATTRFILES)

attrids:
	$(BASE_FAPI2_DIR)/tools/ppeParseAttributeInfo.pl $(PPE_FAPI2_DIR)/include $(TOOLS_ATTR_DIR)/p9_ppe_attributes1.xml $(ATTRFILES)

platattr:
	$(BASE_FAPI2_DIR)/tools/ppeCreateAttrGetSetMacros.pl --path $(BASE_FAPI2_DIR)/tools --inc $(PPE_FAPI2_DIR)/include --src $(PPE_FAPI2_DIR)/src

attrserv:
	$(BASE_FAPI2_DIR)/tools/ppeCreateIfAttrService.pl $(PPE_FAPI2_DIR)/include $(TOOLS_ATTR_DIR)/p9_ppe_attributes1.xml $(ATTRFILES)


#Build macro-specific kernel code
$(PKLIB):
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(PK_MAKE_DIR)

#Build macro-specific kernel code
$(SBEFWLIB):
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(SBEFW_MAKE_DIR)


#Build the code that is common for all processors (PPEs and 405)
$(PPELIB):
	@echo "Processing ppelibmakefile"
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(LIB_MAKE_DIR)

#Build the cache exit code shared with the SBE
$(CACHELIB):
	@echo "Processing cache makefile"
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(CACHE_MAKE_DIR)

#Build the core exit code shared with the SBE
$(CORELIB):
	@echo "Processing core makefile"
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(CORE_MAKE_DIR)

#Build the perv code
$(PERVLIB):
	@echo "Processing perv makefile"
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(PERV_MAKE_DIR)

#Build the comming HWP lib procedures
$(HWPLIB):
	@echo "Processing HWP lib makefile"
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(HWPLIB_MAKE_DIR)

#Build the FAPI2 library
$(FAPI2LIB):
	@echo "Processing fapi2 makefile"
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(FAPI2_MAKE_DIR)/src

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
	rm $(TOP-FIXED-HEADERS)
	rm *.dump


dump:
	objdump -s $(OBJDIR)/$(IMAGE_NAME).out > $(IMAGE_NAME).dump

#Add dependencies to header files
ifneq ($(MAKECMDGOALS),clean)
include $(OBJS:.o=.d) 
endif
