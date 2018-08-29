# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occ_gpe0/img_defs.mk $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2015,2018
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
#  Make header for GPE PK builds
#
# The application may define the following variables to control the
# build process:
#
# IMG_INCLUDES       : Aplication-specific header search paths
#
# DEFS               : A string of -D<symbol>[=<value>] to control compilation
#
# PK                : Default ..; The path to the PK source code.
#                      The default is set for building the PK
#                      subdirectories. 
#
# PK_THREAD_SUPPORT : (0/1, default 1); Compile PK thread and
#                      semaphore suppprt 
#
# PK_TIMER_SUPPORT  : (0/1, default 1); Compile PK timer suppprt
#
# SIMICS_ENVIRONMENT : (0/1, current default 0); Compile for Simics
#
# SIMICS_MAGIC_PANIC : (0/1, current default 0); Use Simics Magic
#                      breakpoint for PK_PANIC() instead of PowerPC trap.
#                      Note that Simics does not model trap correctly in
#                      external debug mode.
#
# GCC-O-LEVEL        : The optimization level passed to GCC (default -Os). May
#                      also be defined empty (GCC-O-LEVEL=) to disable
#                      optimization. This variable can also be used to pass
#                      any other non-default setting to GCC, e.g.
#                      make GCC-O-LEVEL="-Os -fno-branch-count-reg"
#
# GCC-TOOL-PREFIX    : The full path (including executable file prefixes) to
#                      the GCC cross-development tools to use.  The default is 
#                      "ppcnf-mcp5-"
#
# CTEPATH            : This variable defaults to the afs/awd CTE tool
#                      installation - The PORE binutils are stored there. If
#                      you are not in Austin be sure to define CTEPATH in
#                      your .profile.
#
# OBJDIR             : target directory for all generated files

IMAGE_NAME := occ_gpe0

PPE_TYPE := gpe

ifndef IMAGE_SRCDIR
export IMAGE_SRCDIR = $(abspath .)
endif

ifndef IMG_INCLUDES
export IMG_INCLUDES = -I$(IMAGE_SRCDIR) -I$(IMAGE_SRCDIR)/firdata/
endif

ifndef GLOBAL_INCLUDES
export GLOBAL_INCLUDES = -I$(IMAGE_SRCDIR)/..
endif

ifndef BASE_OBJDIR
export BASE_OBJDIR = $(abspath ../../obj)
endif

export IMG_OBJDIR = $(BASE_OBJDIR)/$(IMAGE_NAME)

ifndef PPETOOLS_OBJDIR
export PPETOOLS_OBJDIR = $(abspath ../../obj/ppetools)
endif

ifndef PK_SRCDIR
export PK_SRCDIR = $(abspath ../ppe/pk)
endif

ifndef PPE_STDINCDIR
export PPE_STDINCDIR = $(abspath ../ppe/std)
endif

ifndef COMMONLIB_SRCDIR
export COMMONLIB_SRCDIR = $(abspath ../lib/common)
endif

ifndef OCC_COMMON_TYPES_DIR
export OCC_COMMON_TYPES_DIR = $(abspath ../occ_405/incl/)
endif

ifndef OCCLIB_SRCDIR
export OCCLIB_SRCDIR = $(abspath ../lib/occlib)
endif

ifndef CTEPATH
$(warning The CTEPATH variable is not defined; Defaulting to /afs/awd)
export CTEPATH = /afs/awd/projects/cte
endif

# libs needed by compiler
ifndef PPE_TOOL_PATH
PPE_TOOL_PATH = $(CTEPATH)/tools/ppetools/prod
LD_LIBRARY_PATH += :$(PPE_TOOL_PATH)/lib:
export LD_LIBRARY_PATH
endif

ifndef GCC-TOOL-PREFIX
GCC-TOOL-PREFIX = $(PPE_TOOL_PATH)/bin/powerpc-eabi-
endif

ifndef PPETRACEPP_DIR
export PPETRACEPP_DIR = $(abspath ../ppe/tools/ppetracepp)
endif

ifndef BOOTLOADER_OBJDIR
export BOOTLOADER_OBJDIR = $(BASE_OBJDIR)/occBootLoader
endif

ifndef IMGHDRSCRIPT
export IMGHDRSCRIPT = $(BOOTLOADER_OBJDIR)/imageHdrScript
endif

OBJDIR = $(IMG_OBJDIR)$(SUB_OBJDIR)


CC_ASM  = $(GCC-TOOL-PREFIX)gcc
TCC     = $(PPETOOLS_OBJDIR)/ppetracepp $(GCC-TOOL-PREFIX)gcc
CC      = $(GCC-TOOL-PREFIX)gcc
AS      = $(GCC-TOOL-PREFIX)as
AR      = $(GCC-TOOL-PREFIX)ar
LD      = $(GCC-TOOL-PREFIX)ld
OBJDUMP = $(GCC-TOOL-PREFIX)objdump
OBJCOPY = $(GCC-TOOL-PREFIX)objcopy
TCPP    = $(PPETOOLS_OBJDIR)/ppetracepp $(GCC-TOOL-PREFIX)gcc
THASH	= $(PPETRACEPP_DIR)/tracehash.pl
CPP     = $(GCC-TOOL-PREFIX)gcc
TCXX    = $(PPETOOLS_OBJDIR)/ppetracepp $(GCC-TOOL-PREFIX)g++
CXX     = $(GCC-TOOL-PREFIX)g++

ifeq "$(PK_TIMER_SUPPORT)" ""
PK_TIMER_SUPPORT  = 1
endif

ifeq "$(PK_THREAD_SUPPORT)" ""
PK_THREAD_SUPPORT = 1
endif

ifeq "$(PK_TRACE_SUPPORT)" ""
PK_TRACE_SUPPORT = 1
endif

# Generate a 16bit trace string hash prefix value based on the name of this image.  This will form
# the upper 16 bits of the 32 bit trace hash values.
ifndef PK_TRACE_HASH_PREFIX
PK_TRACE_HASH_PREFIX := $(shell echo $(IMAGE_NAME) | md5sum | cut -c1-4 | xargs -i printf "%d" 0x{})
endif


ifndef GCC-O-LEVEL
GCC-O-LEVEL = -Os
endif

GCC-DEFS += -DIMAGE_NAME=$(IMAGE_NAME)
GCC-DEFS += -DPK_TIMER_SUPPORT=$(PK_TIMER_SUPPORT)
GCC-DEFS += -DPK_THREAD_SUPPORT=$(PK_THREAD_SUPPORT)
GCC-DEFS += -DPK_TRACE_SUPPORT=$(PK_TRACE_SUPPORT)
GCC-DEFS += -DPK_TRACE_HASH_PREFIX=$(PK_TRACE_HASH_PREFIX)
GCC-DEFS += -DUSE_PK_APP_CFG_H=1
GCC-DEFS += -D__PK__=1
DEFS += $(GCC-DEFS)

############################################################################

INCLUDES += $(IMG_INCLUDES) $(GLOBAL_INCLUDES) \
	-I$(PK_SRCDIR)/kernel -I$(PK_SRCDIR)/ppe42 -I$(PK_SRCDIR)/trace \
	-I$(PK_SRCDIR)/$(PPE_TYPE) -I$(PK_SRCDIR)/../../include \
	-I$(PK_SRCDIR)/../../include/registers -I$(OCCLIB_SRCDIR) -I$(COMMONLIB_SRCDIR) \
    -I$(OCC_COMMON_TYPES_DIR) -I$(IMAGE_SRCDIR)/../common -I$(PPE_STDINCDIR)

PIPE-CFLAGS = -pipe

GCC-CFLAGS += -g -gpubnames -gdwarf-3
GCC-CFLAGS += -Wall -Werror
GCC-CFLAGS += -fsigned-char
GCC-CFLAGS += -ffunction-sections
GCC-CFLAGS += -fdata-sections
GCC-CFLAGS += -msoft-float
GCC-CFLAGS += -mcpu=ppe42
GCC-CFLAGS += -meabi
GCC-CFLAGS += -msdata=eabi
GCC-CFLAGS += -ffreestanding
GCC-CFLAGS += -fno-common
GCC-CFLAGS += -fno-inline-functions-called-once

CFLAGS      =  -c -std=gnu89 $(GCC-CFLAGS) $(PIPE-CFLAGS) $(GCC-O-LEVEL) $(INCLUDES)

CXXFLAGS    = -c -std=c++11 -nostdinc++ -fno-rtti -fno-exceptions \
                   $(GCC-CFLAGS) $(PIPE-CFLAGS) $(GCC-O-LEVEL) $(INCLUDES)

CPPFLAGS    = -E

ASFLAGS		= -mppe42

#use this to enable sda optimizations
PCP-FLAG = -e
############################################################################

#override the GNU Make implicit rule for going from a .c to a .o
%.o: %.c

$(OBJDIR)/%.s: %.c
	$(TCC) $(CFLAGS) $(DEFS) -S -o $@ $<

%.o: %.C

$(OBJDIR)/%.s: %.C
	$(TCXX) $(CXXFLAGS) $(DEFS) -S -o $@ $<


#override the GNU Make implicit rule for going from a .S to a .o
%.o: %.S

$(OBJDIR)/%.s: %.S
	$(TCPP) $(CFLAGS) $(DEFS) $(CPPFLAGS) -o $@ $<
.PRECIOUS: $(OBJDIR)/%.s

$(OBJDIR)/%.d: %.c
	@set -e; rm -f $@; \
	if [ "$(dir $*)" != "./" ]; then \
		echo -n "$(OBJDIR)/$(dir $*)" > $@.$$$$; \
	else \
		echo -n "$(OBJDIR)/" > $@.$$$$; \
	fi ; \
	$(CC_ASM) -MM $(INCLUDES) $(CPPFLAGS) $(DEFS) $< >> $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	sed 's,\($*\)\.d[ :]*,\1.s $@ : ,g' < $@ > $@.$$$$; \
	sed 's,\($*\)\.d[ :]*,\1.es $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJDIR)/%.d: %.C
	@set -e; rm -f $@; \
	echo -n "$(OBJDIR)/" > $@.$$$$; \
	$(CXX) -MM $(INCLUDES) $(CPPFLAGS) $(DEFS) $< >> $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJDIR)/%.d: %.S
	@set -e; rm -f $@; \
	if [ "$(dir $*)" != "./" ]; then \
		echo -n "$(OBJDIR)/$(dir $*)" > $@.$$$$; \
	else \
		echo -n "$(OBJDIR)/" > $@.$$$$; \
	fi ; \
	$(CC_ASM) -MM $(INCLUDES) $(CPPFLAGS) $(DEFS) $< >> $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	sed 's,\($*\)\.d[ :]*,\1.s $@ : ,g' < $@ > $@.$$$$; \
	sed 's,\($*\)\.d[ :]*,\1.es $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJDIR)/%.o: $(OBJDIR)/%.s
	$(AS) $(ASFLAGS) -o $@ $<
