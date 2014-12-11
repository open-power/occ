# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occApplet/productApplet/Makefile $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2011,2014
# [+] Google Inc.
# [+] International Business Machines Corp.
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
# Variables
#*******************************************************************************
OCC = ../../occ
BOOTLOADER = ../../occBootLoader
SSX = ../../ssx
PRODUCTAPPLET = .
LIB = ../../lib

# New product applet source files must be listed in productappletfiles.mk
# New applets must also be given an ID in src/occ/aplt/incl/appletId.
LDFLAGS = -L $(SSX)/ssx -L $(SSX)/ppc32 -L $(SSX)/ppc405 -L $(SSX)/pgp \
-L $(OCC) -L $(LIB) -lssx -lppc32 --oformat=elf32-powerpc -melf32ppc

buildImage = $(LD) -R $(OCC)/occ.out $(obj) -Tlinkscript $(LDFLAGS) \
-Map $(basename $(obj)).map -Bstatic -o $(basename $(obj)).out; \
$(OBJCOPY) -I elf32-powerpc -O binary $(basename $(obj)).out $(basename $(obj)).bin; \
$(OBJDUMP) -d $(basename $(obj)).out > $(basename $(obj)).dis; \
$(BOOTLOADER)/imageHdrScript $(basename $(obj)).bin `md5sum $(OCC)/occ.out | cut -c 1-4`;

image = $(BOOTLOADER)/imageHdrScript $(basename $(obj)).bin combineImage; \
$(BOOTLOADER)/imageHdrScript $(basename $(obj)).out displaySize;

#*******************************************************************************
# Includes
#*******************************************************************************
include $(SSX)/pgp/ssx.mk
include productappletfiles.mk

INCLUDES = -I. -I$(OCC)/incl -I$(OCC)/errl -I$(OCC)/trac -I$(LIB) -I$(SSX)/ssx \
-I$(OCC)/sensor -I$(OCC) -I$(OCC)/rtls -I$(OCC)/cmdh -I$(OCC)/pss -I$(OCC)/gpe \
-I$(OCC)/aplt/incl -I$(OCC)/aplt -I$(OCC)/cent -I$(OCC)/proc -I$(OCC)/thread \
-I$(SSX)/ppc405 -I$(SSX)/pgp -I$(SSX)/ppc32 -I$(SSX)/pgp/registers

#*******************************************************************************
# Defs
#*******************************************************************************
DEFS += $(D)
DEFS += -DAPPLET_BUILD=1
DEFS += -DUSE_SSX_APP_CFG_H=1

# Note: Do not use SDA sections for product applets
GCC-CFLAGS = -c -g -Wall -fsigned-char -msoft-float -pipe \
-Wa,-m405 -m32 -mcpu=405 -mmultiple -mstring -meabi -ffreestanding -Os -mno-sdata

#*******************************************************************************
# Compilation
#*******************************************************************************
all: $(PRDTAPLT_OBJECTS)
	 $(CPP) -P $(DEFS) < linkProductApplet.cmd > linkscript
	 $(foreach obj,$(PRDTAPLT_OBJECTS),$(buildImage))

#*******************************************************************************
# combineImage
#*******************************************************************************
.PHONY : combineImage
combineImage:
	 $(foreach obj,$(PRDTAPLT_OBJECTS),$(image))

#*******************************************************************************
# Clean
#*******************************************************************************
clean: 
	rm -f *.o *.out *.bin *.dis *.map *.hash linkscript
