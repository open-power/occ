# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occApplet/testApplet/Makefile $
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
ROOTPATH = $(shell pwd)/../../
export OCCROOT = $(ROOTPATH)
endif

#*******************************************************************************
# make variables
#*******************************************************************************
OCC = ../../occ
BOOTLOADER = ../../occBootLoader
SSX = ../../ssx
TESTAPPLET = .
LIB = ../../lib
LDFLAGS += --oformat=elf32-powerpc -melf32ppc

buildImage = $(LD) -R $(OCC)/occ.out $(obj) -Tlinkscript $(LDFLAGS) \
-Map $(basename $(obj)).map -Bstatic -o $(basename $(obj)).out && \
$(OBJCOPY) -I elf32-powerpc -O binary $(basename $(obj)).out $(basename $(obj)).bin && \
$(OBJDUMP) -d $(basename $(obj)).out > $(basename $(obj)).dis && \
$(BOOTLOADER)/imageHdrScript $(basename $(obj)).bin `md5sum $(OCC)/occ.out | cut -c 1-4`

#*******************************************************************************
# Includes
#*******************************************************************************
include $(SSX)/pgp/ssx.mk
include testappletfiles.mk

INCLUDES = -I. -I$(OCC)/rtls -I$(OCC)/incl -I$(OCC)/errl -I$(OCC)/trac -I$(LIB) -I$(SSX)/ssx 
INCLUDES += -I$(SSX)/ppc405 -I$(SSX)/pgp -I$(SSX)/ppc32 -I$(SSX)/pgp/registers
INCLUDES += -I$(OCC)/sensor -I$(OCC)/pss -I$(OCC)/gpe -I$(OCC) -I$(OCC)/aplt -I$(OCC)/dcom 
INCLUDES += -I$(OCC)/aplt/incl -I$(OCC)/cmdh

#*******************************************************************************
# Defs
#*******************************************************************************
DEFS += $(D)
DEFS += -DOCC=1 -DUSE_SSX_APP_CFG_H=1

GCC-CFLAGS = -c -g -Wall -fsigned-char -msoft-float -pipe \
-m32 -mbig-endian -Wa,-m405 -mcpu=405 -mmultiple -mstring -meabi \
-ffreestanding -Os -mno-sdata 

#*******************************************************************************
# Compilation 
#*******************************************************************************
all: $(TESTAPLT_OBJECTS) 
	 $(CPP) -P $(DEFS) < linkTestApplet.cmd > linkscript
	 $(foreach obj,$(TESTAPLT_OBJECTS),$(buildImage) &&) true

#*******************************************************************************
# combineImage 
#*******************************************************************************
.PHONY : combineImage
combineImage:

#*******************************************************************************
# Clean
#*******************************************************************************
clean: 
	rm -f *.o *.out *.bin *.dis *.map *.hash linkscript
