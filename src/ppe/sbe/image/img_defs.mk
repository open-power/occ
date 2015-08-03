# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/sbe/image/img_defs.mk $
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
#  Make header for SBE image build
#
# Settings to for SBE image file build
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

IMAGE_NAME := sbe_main

PPE_TYPE := std

ifndef IMAGE_SRCDIR
export IMAGE_SRCDIR = $(abspath .)
endif

ifndef CACHE_SRCDIR
export CACHE_SRCDIR = $(abspath ../../hwp/cache)
endif

ifndef CORE_SRCDIR
export CORE_SRCDIR = $(abspath ../../hwp/core)
endif

ifndef PERV_SRCDIR
export PERV_SRCDIR = $(abspath ../../hwp/perv)
endif

ifndef HWPLIB_SRCDIR
export HWPLIB_SRCDIR = $(abspath ../../hwp/lib)

endif

ifndef IMG_INCLUDES
export IMG_INCLUDES = -I$(IMAGE_SRCDIR) -I$(CACHE_SRCDIR) -I$(CORE_SRCDIR)

endif

ifndef IMG_INCLUDES
export IMG_INCLUDES = -I$(IMAGE_SRCDIR) -I$(CACHE_SRCDIR) -I$(CORE_SRCDIR)
endif
ifndef BASE_OBJDIR
export BASE_OBJDIR = $(abspath ../obj)
endif

export IMG_OBJDIR = $(BASE_OBJDIR)/$(IMAGE_NAME)

ifndef PK_SRCDIR
export PK_SRCDIR = $(abspath ../../pk)
endif

ifndef TOOLS_ATTR_DIR
export TOOLS_ATTR_DIR = $(abspath ../../tools/scripts)
endif



ifndef GCC-TOOL-PREFIX
GCC-TOOL-PREFIX = $(CTEPATH)/tools/gcc405lin/prod/usr/bin/powerpc-linux-
endif


ifndef P2P_SRCDIR
export P2P_SRCDIR = $(abspath ../../tools/PowerPCtoPPE)
endif

ifndef PPETRACEPP_DIR
export PPETRACEPP_DIR = $(abspath ../../tools/ppetracepp)
endif

ifndef PLAT_FAPI2_DIR
export PLAT_FAPI2_DIR = $(abspath ../plat)
endif


ifndef PPE_FAPI2_DIR
export PPE_FAPI2_DIR = $(abspath ../../hwpf/plat)
endif

ifndef BASE_FAPI2_DIR
export BASE_FAPI2_DIR = $(abspath /afs/awd/projects/eclipz/lab/p8/u/rembold/ekbgit/hwpf/fapi2)
endif


ifndef CC_ROOT
export CC_ROOT = ${CTEPATH}/tools/gcc405lin/prod
endif


ifndef GCC-TOOL-PREFIX
GCC-TOOL-PREFIX  = ${CC_ROOT}/usr/bin/powerpc-linux-
endif

ifndef BINUTILS-TOOL-PREFIX
BINUTILS-TOOL-PREFIX = $(CTEPATH)/tools/ppetools/prod/powerpc-eabi/bin/
endif

OBJDIR = $(BASE_OBJDIR)$(SUB_OBJDIR)

CC_ASM  = $(GCC-TOOL-PREFIX)gcc
TCC     = $(PPETRACEPP_DIR)/ppetracepp $(GCC-TOOL-PREFIX)g++
CC      = $(PPETRACEPP_DIR)/ppetracepp $(GCC-TOOL-PREFIX)gcc
AS      = $(BINUTILS-TOOL-PREFIX)as
AR      = $(BINUTILS-TOOL-PREFIX)ar
LD      = $(BINUTILS-TOOL-PREFIX)ld
OBJDUMP = $(BINUTILS-TOOL-PREFIX)objdump
OBJCOPY = $(BINUTILS-TOOL-PREFIX)objcopy
TCPP    = $(PPETRACEPP_DIR)/ppetracepp $(GCC-TOOL-PREFIX)gcc
THASH	= $(PPETRACEPP_DIR)/tracehash.pl
CPP     = $(GCC-TOOL-PREFIX)gcc

ifdef P2P_ENABLE
PCP     = $(P2P_SRCDIR)/ppc-ppe-pcp.py
endif


ifndef CTEPATH
$(warning The CTEPATH variable is not defined; Defaulting to /afs/awd)
export CTEPATH = /afs/awd/projects/cte
endif

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
#GCC-O-LEVEL = -Os
#GCC-O-LEVEL = -O -g
GCC-O-LEVEL = -O
endif

GCC-DEFS += -DIMAGE_NAME=$(IMAGE_NAME)
GCC-DEFS += -DPK_TIMER_SUPPORT=$(PK_TIMER_SUPPORT) 
GCC-DEFS += -DPK_THREAD_SUPPORT=$(PK_THREAD_SUPPORT) 
GCC-DEFS += -DPK_TRACE_SUPPORT=$(PK_TRACE_SUPPORT)
GCC-DEFS += -DPK_TRACE_HASH_PREFIX=$(PK_TRACE_HASH_PREFIX)
GCC-DEFS += -D__PK__=1
GCC-DEFS += -D__SBE__=1
DEFS += $(GCC-DEFS)
export LD_LIBRARY_PATH = /afs/awd.austin.ibm.com/proj/p3/cte/tools/gcc405lin/vol1/usr/lib

############################################################################


INCLUDES += $(IMG_INCLUDES) \
	-I$(IMAGE_SRCDIR)/../../../include \
	-I$(PLAT_FAPI2_DIR)/include \
	-I$(PPE_FAPI2_DIR)/include \
	-I$(BASE_FAPI2_DIR)/include \
	-I$(PK_SRCDIR)/../include \
	-I$(PK_SRCDIR)/$(PPE_TYPE) \
	-I$(PK_SRCDIR)/../include \
	-I$(PK_SRCDIR)/kernel \
        -I$(PK_SRCDIR)/ppe \
        -I$(PK_SRCDIR)/ppe42 \
        -I$(PK_SRCDIR)/trace

PIPE-CFLAGS = -pipe -Wa,-m405

GCC-CFLAGS += -g -Wall -Werror -Wno-error=unused-label \
	-msoft-float -mcpu=405 -mmulhw \
	-meabi -msdata=eabi  \
	-ffreestanding \
        -fno-common \
	-fsigned-char \
	-fno-inline-functions-called-once \
	-ffixed-r11 \
	-ffixed-r12 \
	-ffixed-r14 \
	-ffixed-r15 \
	-ffixed-r16 \
	-ffixed-r17 \
	-ffixed-r18 \
	-ffixed-r19 \
	-ffixed-r20 \
	-ffixed-r21 \
	-ffixed-r22 \
	-ffixed-r23 \
	-ffixed-r24 \
	-ffixed-r25 \
	-ffixed-r26 \
	-ffixed-r27 \
	-ffixed-cr1 \
	-ffixed-cr2 \
	-ffixed-cr3 \
	-ffixed-cr4 \
	-ffixed-cr5 \
	-ffixed-cr6 \
	-ffixed-cr7


CFLAGS      	=  -c $(GCC-CFLAGS) $(PIPE-CFLAGS) $(GCC-O-LEVEL) $(INCLUDES) 


CPPFLAGS    	= -E -std=c++11
#CPPFLAGS    	= -E

ASFLAGS		= -mppe42

ifdef P2P_ENABLE
#use this to disable optimizations (fused compare/branch etc.)
PCP-FLAG    =  

#use this to enable optimizations
#PCP-FLAG =
endif
############################################################################

#override the GNU Make implicit rule for going from a .C to a .o
%.o: %.C

$(OBJDIR)/%.s: %.C
	$(TCC) $(PPE-CFLAGS) $(DEFS) -S -std=c++11 -o $@ $<


#override the GNU Make implicit rule for going from a .c to a .o
%.o: %.c

$(OBJDIR)/%.s: %.c
	$(CC) $(PPE-CFLAGS) $(DEFS) -S -o $@ $<

#override the GNU Make implicit rule for going from a .S to a .o
%.o: %.S

$(OBJDIR)/%.s: %.S
	$(TCPP) $(PPE-CFLAGS) $(DEFS) $(CPPFLAGS) -o $@ $<
.PRECIOUS: $(OBJDIR)/%.s

ifndef P2P_ENABLE

$(OBJDIR)/%.o: $(OBJDIR)/%.s
	$(AS) $(ASFLAGS) -o $@ $<

else

$(OBJDIR)/%.es: $(OBJDIR)/%.s
	$(PCP) $(PCP-FLAG) -f $<
.PRECIOUS: $(OBJDIR)/%.es

$(OBJDIR)/%.o: $(OBJDIR)/%.es
	$(AS) $(ASFLAGS) -o $@ $<

endif

# From the GNU 'Make' manual - these scripts uses the preprocessor to
# create dependency files (*.d), then mungs them slightly to make them
# work as Make targets. The *.d files are include-ed in the
# subdirectory Makefiles.

$(OBJDIR)/%.d: %.C
	@set -e; rm -f $@; \
	echo -n "$(OBJDIR)/" > $@.$$$$; \
	$(CC_ASM) -MM $(INCLUDES) $(CPPFLAGS) $(DEFS) $< >> $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJDIR)/%.d: %.c
	@set -e; rm -f $@; \
	echo -n "$(OBJDIR)/" > $@.$$$$; \
	echo "$(INCLUDES)"; \
	$(CC_ASM) -MM $(INCLUDES) $(CPPFLAGS) $(DEFS) $< >> $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJDIR)/%.d: %.S
	@set -e; rm -f $@; \
	echo -n "$(OBJDIR)/" > $@.$$$$; \
	$(CC_ASM) -MM $(INCLUDES) $(CPPFLAGS) $(DEFS) $< >> $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

