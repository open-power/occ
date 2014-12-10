# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occ/proc/test/app.mk $
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

#  Description: mk occ application
#
#  This Makefile is included-ed into application Makefiles and
#  encapsulates the steps necessary to create application images.
#
#  The application Makefile (user) must define the following variables:
#
#  APP     - The name of the application
#  SOURCES - The list of local source files that implement the
#            application.
#
#  The application Makefile (user) may optionally define the following
#  variables:  
#
#  D    - The value of $(D) is appended to DEFS defined by ssx.mk
#
#  MODE - The following modes are recognized:
#
#         validation - (Default) An application that requires all SSX
#         services. 
#
#         firmware - An interrupt only configuration. 
#
#  The make process creates the following files:
#
#  $(APP).out  - The PowerPC-ELF version of the application
#  $(APP).bin  - A binary SRAM image of the application
#  $(APP).map  - The linker map of the application

OCC = ../../
SSX = ../../../ssx
LIB = ../../../lib
PGP = $(SSX)/pgp

ifeq "$(MODE)" "firmware"
SSX_TIMER_SUPPORT = 0
SSX_THREAD_SUPPORT = 0
endif

export SSX_TIMER_SUPPORT
export SSX_THREAD_SUPPORT
export PPC405_MMU_SUPPORT
export PGP_ASYNC_SUPPORT

INCLUDES = -I $(OCC) -I$(LIB)

include $(PGP)/ssx.mk

C-OBJECTS = $(SOURCES:.c=.o)
OBJECTS   = $(C-OBJECTS:.S=.o)

LDFLAGS = -L $(SSX)/ssx -L $(SSX)/ppc32 -L $(SSX)/ppc405 -L $(SSX)/pgp \
	  -L $(OCC) -L $(LIB) -lssx -lppc32

DEFS += $(D)

all: $(OBJECTS) libssx.a
	$(MAKE) -C $(PGP) DEFS="$(DEFS)" -e
	$(CPP) -P $(DEFS) < $(OCC)/linkocc.cmd > linkscript
	$(LD) $(OBJECTS) \
	-Tlinkscript $(LDFLAGS) -Map $(APP).map -Bstatic -o $(APP).out
	$(OBJCOPY) -O binary $(APP).out $(APP).bin
	$(OBJDUMP) -d $(APP).out > $(APP).dis

libssx.a:
	$(MAKE) -C $(LIB) DEFS="$(DEFS)" -e


.PHONY : clean
clean:
	rm -f *.o *.d *.d.* *.out *.bin *.srec *.dis *.map linkscript
	rm -f ./*/*.o ./*/*.d ./*/*.d.*	

.PHONY : clean_all
clean_all:
	$(MAKE) clean
	$(MAKE) -C $(PGP) clean

include $(OBJECTS:.o=.d) 
