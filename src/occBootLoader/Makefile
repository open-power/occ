# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occBootLoader/Makefile$
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2011,2014
# [+] Google Inc.
# [+] International Business Machines Corp.
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.
#
# IBM_PROLOG_END_TAG

ifndef ROOTPATH
ROOTPATH = $(shell pwd)/../
export OCCROOT = $(ROOTPATH)
endif

#*******************************************************************************
# Variables
#*******************************************************************************
OCC = ../occ
BOOTLOADER = .
SSX = ../ssx

#*******************************************************************************
# Includes
#*******************************************************************************
include bootfiles.mk
include ../ssx/pgp/ssx.mk

INCLUDES = -I. -I$(SSX)/ppc32 -I$(SSX)/pgp -I$(OCC)/incl -I$(OCC)/ -I$(SSX)/ppc405

#*******************************************************************************
# Flags
#*******************************************************************************
OBJECTS   = ${BOOTLOADER_OBJECTS}
EXECUTABLE = bootloader
imageHdrScript = imageHdrScript
imageHdrScript_CC = gcc

DEFS += $(D)

#*******************************************************************************
# Compilation
#*******************************************************************************

all: ${BOOTLOADER_OBJECTS} imageHdrScript
	$(CPP) -P $(DEFS) < linkboot.cmd > linkscript
	$(LD)  ${BOOTLOADER_OBJECTS} -Tlinkscript $(LDFLAGS) -zmuldefs -Map $(EXECUTABLE).map -melf32ppc --oformat=elf32-powerpc -Bstatic -o $(EXECUTABLE).out
	$(OBJCOPY) -I elf32-powerpc -O binary $(EXECUTABLE).out $(EXECUTABLE).bin
	$(OBJDUMP) -d $(EXECUTABLE).out > $(EXECUTABLE).dis
	$(BOOTLOADER)/$(imageHdrScript) $(EXECUTABLE).bin t1

.PHONY : combineImage
combineImage:
	 	$(BOOTLOADER)/imageHdrScript $(EXECUTABLE).bin combineImage
	 	$(BOOTLOADER)/imageHdrScript $(EXECUTABLE).out displaySize

imageHdrScript: imageHdrScript.c
	$(imageHdrScript_CC) $(LDFLAGS) -I$(BOOTLOADER) -I$(OCC)/incl -I$(OCC)/ imageHdrScript.c -o $@

#*******************************************************************************
# Clean
#*******************************************************************************
clean:
	rm -f *.o *.d *.out *.bin *.srec *.dis *.map linkscript imageHdrScript
	rm -rf $(EXECUTABLE) $(IMAGE_FILE)
