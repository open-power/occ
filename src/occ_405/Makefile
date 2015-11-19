# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occ_405/Makefile $
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
# This Makefile compiles all of the SSX code required for the P9 port
# of SSX.  See the "img_defs.mk" file in this directory.

#Pull in the definitions that affect all makefiles for this image
include img_defs.mk

#Use tracepp instead of ppetracepp for 405 code
include occ_defs.mk

#Pull in object file names for the top directory
include topfiles.mk

SSX_MAKE_DIR := $(SSX_SRCDIR)/occhw
OBJS := $(addprefix $(OBJDIR)/, $(TOP_OBJECTS))
OBJDIRS = $(dir ${OBJS})
SSXLIB := $(OBJDIR)/ssx/libssx.a
COMMONLIB := $(OBJDIR)/common/libcommon.a
OCCLIB := $(OBJDIR)/occlib/libocc.a
PPC405LIB := $(OBJDIR)/ppc405lib/libppc405.a
LINK_OBJS = $(OBJS) $(SSXLIB) $(COMMONLIB) $(OCCLIB) $(PPC405LIB)
LINK_SCRIPT = $(addprefix $(OBJDIR)/, linkscript)
LINK_CMD_SCRIPT = linkocc.cmd
LDFLAGS += --oformat=elf32-powerpc -melf32ppc
LIB_DIRS =  -L$(OBJDIR) \
            -L$(OBJDIR)/ssx \
            -L$(OBJDIR)/ssx/ppc405 \
            -L$(OBJDIR)/ssx/ppc32 \
            -L$(OBJDIR)/commonlib \
            -L$(OBJDIR)/occlib \
            -L$(OBJDIR)/ppc405lib \
            -L$(OBJDIR)/cmdh \
            -L$(OBJDIR)/dcom \
            -L$(OBJDIR)/dimm \
            -L$(OBJDIR)/errl \
            -L$(OBJDIR)/pss \
            -L$(OBJDIR)/rtls \
            -L$(OBJDIR)/sensor \
            -L$(OBJDIR)/thread \
            -L$(OBJDIR)/timer \
            -L$(OBJDIR)/trac \
            -L$(OBJDIR)/amec \
            -L$(OBJDIR)/dcom \
            -L$(OBJDIR)/proc \
            -L$(OBJDIR)/firdata

#default target is to make a binary application image
#This removes all unecessary headers from the ELF executable
$(OBJDIR)/$(IMAGE_NAME).bin $(OBJDIR)/$(IMAGE_NAME).dis: $(OBJDIR)/$(IMAGE_NAME).out
	$(OBJCOPY) -O binary  $< $(OBJDIR)/$(IMAGE_NAME).bin
	$(OBJDUMP) -S $< > $(OBJDIR)/$(IMAGE_NAME).dis

#create a linked ELF executable
$(OBJDIR)/$(IMAGE_NAME).out: $(TRACEPP_DIR)/tracepp $(LINK_OBJS) $(LINK_SCRIPT)
	$(LD) -e __ssx_boot -T$(LINK_SCRIPT) $(LDFLAGS) -Map $(OBJDIR)/$(IMAGE_NAME).map -Bstatic -o $(OBJDIR)/$(IMAGE_NAME).out $(LIB_DIRS) -lssx -locc -lppc405 -lcommon

#pass the link command file through the C preprocessor to evaluate macros and remove comments
#$(LINK_SCRIPT): linkocc.cmd
$(LINK_SCRIPT): $(LINK_CMD_SCRIPT)
	$(CPP) -E -x c -P $(DEFS) $(LINK_CMD_SCRIPT) -o $(LINK_SCRIPT)

#Create an obj directory if needed
$(LINK_OBJS) $(OBJS) $(OBJS:.o=.d): | $(OBJDIRS)

$(OBJDIRS):
	mkdir -p $(OBJDIRS)

#Build the SSX kernel library
$(SSXLIB):
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(SSX_MAKE_DIR)

#Build the code that is common for all processors (PPE's and 405)
$(COMMONLIB):
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(COMMONLIB_SRCDIR)

#Build the code that is common for all OCC processors (GPEs and 405)
$(OCCLIB):
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(OCCLIB_SRCDIR)

#Build the library code that only works on the ppc405
$(PPC405LIB):
	$(MAKE) -I $(IMAGE_SRCDIR) -C $(PPC405LIB_SRCDIR)

$(TRACEPP_DIR)/tracepp:
	(cd $(TRACEPP_DIR) && make)

# collect all of the trace hash files for this image into a single trexStringFile
.PHONY : tracehash
tracehash:
	mkdir -p $(OBJDIR)
	$(THASH) -c -d $(OBJDIR) -s $(OBJDIR)/trexStringFile

# load and run the 405 image in simics
run: $(OBJDIR)/$(IMAGE_NAME).out
	$(SIMICS_WS)/simics \
		-e '$$occ_405_binary_to_load=$(OBJDIR)/$(IMAGE_NAME).out' modelsetup.simics

#clean out all generated files
.PHONY : clean
clean:
	rm -fr $(OBJDIR)

#Add dependencies to header files
#ifneq ($(MAKECMDGOALS),clean)
#include $(OBJS:.o=.d)
#endif
