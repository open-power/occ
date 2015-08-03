# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occ_405/pss/test/app.mk $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2011,2015
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

OCC = ../../
SSX = ../../../ssx
LIB = ../../../lib
PGP = $(SSX)/pgp

ifeq "$(MODE)" "firmware"
SSX_TIMER_SUPPORT = 0
SSX_THREAD_SUPPORT = 0
endif

#*******************************************************************************
# Export
#*******************************************************************************
export SSX_TIMER_SUPPORT
export SSX_THREAD_SUPPORT
export PPC405_MMU_SUPPORT
export PGP_ASYNC_SUPPORT

#*******************************************************************************
# Flags
#*******************************************************************************

INCLUDES = -I $(OCC) -I$(LIB)

include $(PGP)/ssx.mk

C-OBJECTS = $(SOURCES:.c=.o)
OBJECTS   = $(C-OBJECTS:.S=.o)

LDFLAGS = -L $(SSX)/ssx -L $(SSX)/ppc32 -L $(SSX)/ppc405 -L $(SSX)/pgp \
	  -L $(OCC) -L $(LIB) -lssx -lppc32


DEFS += $(D)

#*******************************************************************************
# compilation 
#*******************************************************************************
all: $(OBJECTS) libssx.a
	$(MAKE) -C $(PGP) DEFS="$(DEFS)" -e
	$(CPP) -P $(DEFS) < $(PGP)/linkssx.cmd > linkscript
	$(LD) $(OBJECTS) \
	-Tlinkscript $(LDFLAGS) -Map $(APP).map -Bstatic -o $(APP).out
	$(OBJCOPY) -O binary $(APP).out $(APP).bin
	$(OBJDUMP) -d $(APP).out > $(APP).dis


libssx.a:
	$(MAKE) -C $(LIB) DEFS="$(DEFS)" -e

#*******************************************************************************
# clean
#*******************************************************************************

.PHONY : clean
clean:
	rm -f *.o *.d *.d.* *.out *.bin *.srec *.dis *.map linkscript
	rm -f ./*/*.o ./*/*.d ./*/*.d.*	

.PHONY : clean_all
clean_all:
	$(MAKE) clean
	$(MAKE) -C $(PGP) clean

#*******************************************************************************
# Doxygen
#*******************************************************************************

.PHONY : doc
doc:
	doxygen doc/Doxyfile

#*******************************************************************************
# .d file creation
#*******************************************************************************

-include $(OBJECTS:.o=.d) 
