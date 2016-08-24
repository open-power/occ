# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occBootLoader/Makefile $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2011,2016
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

include img_defs.mk
include bootfiles.mk

#*******************************************************************************
# Variables
#*******************************************************************************
OBJECTS   = $(addprefix $(OBJDIR)/, $(notdir ${BOOTLOADER_OBJECTS}))
LINK_SCRIPT = $(OBJDIR)/linkscript
EXECUTABLE = $(OBJDIR)/bootloader
imageHdrScript = $(OBJDIR)/imageHdrScript
imageHdrScript_CC = gcc

#*******************************************************************************
# Flags
#*******************************************************************************
DEFS += $(D)

#*******************************************************************************
# Compilation
#*******************************************************************************
.PHONY : all
all: $(PPETOOLS_OBJDIR)/ppetracepp $(OBJDIR) ${OBJECTS} ${imageHdrScript}
	$(CPP) -P $(DEFS) < linkboot.cmd > $(LINK_SCRIPT)
	$(LD)  ${OBJECTS} -T$(LINK_SCRIPT) $(LDFLAGS) -zmuldefs -Map $(EXECUTABLE).map -melf32ppc --oformat=elf32-powerpc -Bstatic -o $(EXECUTABLE).out -L$(OBJDIR)
	$(OBJCOPY) -I elf32-powerpc -O binary $(EXECUTABLE).out $(EXECUTABLE).bin
	$(OBJDUMP) -d $(EXECUTABLE).out > $(EXECUTABLE).dis

$(PPETOOLS_OBJDIR)/ppetracepp: $(PPETOOLS_OBJDIR)
	g++ -O3 -w -g -I$(PPETRACEPP_DIR)/ $(PPETRACEPP_DIR)/ppetracepp.C -o $(PPETOOLS_OBJDIR)/ppetracepp

$(PPETOOLS_OBJDIR):
	mkdir -p $(PPETOOLS_OBJDIR)

$(OBJDIR)/imageHdrScript: imageHdrScript.c
	$(imageHdrScript_CC) -g $(LDFLAGS) -I. -I$(OCC405_INCLDIR)/ -I$(OCC405_SRCDIR)/ imageHdrScript.c -o $(OBJDIR)/imageHdrScript

$(OBJDIR)/occbuildname.o:
	$(TCC) $(CFLAGS) $(DEFS) -o $@ $(OCC405_SRCDIR)/occbuildname.c

$(OBJDIR)/savegpr.o:
	$(TCPP) $(CFLAGS) $(DEFS) -o $@ $(SSX_SRCDIR)/ppc32/savegpr.S

$(OBJDIR):
	mkdir -p $(OBJDIR)
#*******************************************************************************
# Clean
#*******************************************************************************
.PHONY : clean
clean:
	rm -rf $(OBJDIR)
