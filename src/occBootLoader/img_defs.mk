# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occBootLoader/img_defs.mk $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2015,2016
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
# $Id: ssx.mk,v 1.2 2014/06/26 12:55:39 cmolsen Exp $
# $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/ssx.mk,v $
#  Make header for PgP SSX builds
#
# The application may define the following variables to control the
# build process:
#
# IMG_INCLUDES       : Aplication-specific header search paths
#
# DEFS               : A string of -D<symbol>[=<value>] to control compilation
#
# SSX_SRCDIR         : Default ..; The path to the SSX source code.
#                      The default is set for building the SSX
#                      subdirectories.
#
# SSX_THREAD_SUPPORT : (0/1, default 1); Compile SSX thread and
#                      semaphore suppprt
#
# SSX_TIMER_SUPPORT  : (0/1, default 1); Compile SSX timer suppprt
#
# PPC405_MMU_SUPPORT : (0/1, default 1); Compile for PPC405 simple MMU protection
#
# SIMICS_ENVIRONMENT : (0/1, current default 0); Compile for Simics
#
# SIMICS_MAGIC_PANIC : (0/1, current default 0); Use Simics Magic
#                      breakpoint for SSX_PANIC() instead of PowerPC trap.
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

IMAGE_NAME := occBootLoader
ifndef IMAGE_SRCDIR
export IMAGE_SRCDIR = $(abspath .)
endif

ifndef IMG_INCLUDES
export IMG_INCLUDES = -I$(IMAGE_SRCDIR)
endif

ifndef GLOBAL_INCLUDES
export GLOBAL_INCLUDES = -I$(IMAGE_SRCDIR)/..
endif

ifndef BASE_OBJDIR
export BASE_OBJDIR = $(abspath ../../obj)
endif

export IMG_OBJDIR = $(BASE_OBJDIR)/$(IMAGE_NAME)

ifndef SSX_SRCDIR
export SSX_SRCDIR = $(abspath ../ssx)
endif

ifndef COMMONLIB_SRCDIR
export COMMONLIB_SRCDIR = $(abspath ../lib/common)
endif

ifndef OCCLIB_SRCDIR
export OCCLIB_SRCDIR = $(abspath ../lib/occlib)
endif

ifndef PPC405LIB_SRCDIR
export PPC405LIB_SRCDIR = $(abspath ../lib/ppc405lib)
endif

ifndef SSXLIB_SRCDIR
export SSXLIB_SRCDIR = $(abspath ../lib/ssxlib)
endif

ifndef OCC405_SRCDIR
export OCC405_SRCDIR = $(abspath ../occ_405)
endif

ifndef OCC405_INCLDIR
export OCC405_INCLDIR = $(OCC405_SRCDIR)/incl
endif

ifndef GCC-TOOL-PREFIX
GCC-TOOL-PREFIX = $(CTEPATH)/tools/ppcgcc/prod/bin/powerpc-linux-
endif

ifndef PPETRACEPP_DIR
export PPETRACEPP_DIR = $(abspath ../ppe/tools/ppetracepp)
endif

ifndef TRACEPP_DIR
export TRACEPP_DIR = $(abspath ../tracepp)
endif

CC_ASM  = $(GCC-TOOL-PREFIX)gcc
TCC     = $(PPETRACEPP_DIR)/ppetracepp $(GCC-TOOL-PREFIX)gcc
THCC     = $(TRACEPP_DIR)/tracepp $(GCC-TOOL-PREFIX)gcc
CC      = $(GCC-TOOL-PREFIX)gcc
AS      = $(GCC-TOOL-PREFIX)as
AR      = $(GCC-TOOL-PREFIX)ar
LD      = $(GCC-TOOL-PREFIX)ld
OBJDUMP = $(GCC-TOOL-PREFIX)objdump
OBJCOPY = $(GCC-TOOL-PREFIX)objcopy
TCPP    = $(PPETRACEPP_DIR)/ppetracepp $(GCC-TOOL-PREFIX)gcc
THASH	= $(PPETRACEPP_DIR)/tracehash.pl
CPP     = $(GCC-TOOL-PREFIX)cpp

ifndef CTEPATH
$(warning The CTEPATH variable is not defined; Defaulting to /afs/awd)
CTEPATH = /afs/awd/projects/cte
endif

OBJDIR = $(IMG_OBJDIR)$(SUB_OBJDIR)

ifndef GCC-O-LEVEL
GCC-O-LEVEL = -Os
endif

ifeq "$(SSX_TIMER_SUPPORT)" ""
SSX_TIMER_SUPPORT  = 1
endif

ifeq "$(SSX_THREAD_SUPPORT)" ""
SSX_THREAD_SUPPORT = 1
endif

# TODO: Enable this once we get MMU support working in simics
# Currently, turning on MMU support causes an SSX panic (in Simics)
ifeq "$(PPC405_MMU_SUPPORT)" ""
PPC405_MMU_SUPPORT = 0
endif

ifeq "$(OCCHW_ASYNC_SUPPORT)" ""
OCCHW_ASYNC_SUPPORT = 1
endif

ifeq "$(SSX_TRACE_SUPPORT)" ""
SSX_TRACE_SUPPORT = 1
endif

ifeq "$(SSX_USE_INIT_SECTION)" ""
SSX_USE_INIT_SECTION = 0
endif

# Generate a 16bit trace string hash prefix value based on the name of this image.  This will form
# the upper 16 bits of the 32 bit trace hash values.
ifndef SSX_TRACE_HASH_PREFIX
SSX_TRACE_HASH_PREFIX := $(shell echo $(IMAGE_NAME) | md5sum | cut -c1-4 | xargs -i printf "%d" 0x{})
endif

ifndef GCC-O-LEVEL
GCC-O-LEVEL = -Os
endif

ifdef TRAC_TO_SIMICS
GCC-DEFS += -DTRAC_TO_SIMICS=1
endif

ifdef STRAIGHT_TO_OBS_HACK
GCC-DEFS += -DSTRAIGHT_TO_OBS_HACK=$(STRAIGHT_TO_OBS_HACK)
else
GCC-DEFS += -DSTRAIGHT_TO_OBS_HACK=1
endif

GCC-DEFS += -DIMAGE_NAME=$(IMAGE_NAME)
GCC-DEFS += -DSSX_TIMER_SUPPORT=$(SSX_TIMER_SUPPORT)
GCC-DEFS += -DSSX_THREAD_SUPPORT=$(SSX_THREAD_SUPPORT)
GCC-DEFS += -DPPC405_MMU_SUPPORT=$(PPC405_MMU_SUPPORT)
GCC-DEFS += -DSSX_TRACE_SUPPORT=$(SSX_TRACE_SUPPORT)
GCC-DEFS += -DSSX_TRACE_HASH_PREFIX=$(SSX_TRACE_HASH_PREFIX)
GCC-DEFS += -DSSX_USE_INIT_SECTION=$(SSX_USE_INIT_SECTION)
GCC-DEFS += -DUSE_SSX_APP_CFG_H=1
GCC-DEFS += -D__SSX__=1
DEFS += $(GCC-DEFS) -DCONFIGURE_PTS_SLW=0

############################################################################

INCLUDES += $(IMG_INCLUDES) $(GLOBAL_INCLUDES) -I$(OCC405_INCLDIR)\
	-I$(SSX_SRCDIR)/ssx -I$(SSX_SRCDIR)/ppc32 -I$(SSX_SRCDIR)/ppc405 \
	-I$(SSX_SRCDIR)/trace -I$(SSX_SRCDIR)/occhw -I$(SSX_SRCDIR)/../lib/common \
	-I$(SSX_SRCDIR)/../include -I$(SSX_SRCDIR)/../include/registers \
	-I$(OCCLIB_SRCDIR) -I$(COMMONLIB_SRCDIR) -I$(SSXLIB_SRCDIR) -I$(PPC405LIB_SRCDIR) \
    -I$(OCC405_SRCDIR)

PIPE-CFLAGS = -pipe -Wa,-m405

GCC-CFLAGS += -g -Wall -fsigned-char -msoft-float  \
	-m32 -mbig-endian -mcpu=405 -mmultiple -mstring \
	-meabi -msdata=eabi -ffreestanding -fno-common \
	-fno-inline-functions-called-once

CFLAGS      =  -c $(GCC-CFLAGS) $(PIPE-CFLAGS) $(GCC-O-LEVEL) $(INCLUDES)

############################################################################

# Build object code
#
# %.o: %.c - Compile C code
#
# %.o: %.S - Compile PowerPC assembler (including PGAS-PPC assembly)

#override the GNU Make implicit rule for going from a .c to a .o
%.o: %.c
$(OBJDIR)/%.o: %.c
	$(TCC) $(CFLAGS) $(DEFS) -o $@ $<

#override the GNU Make implicit rule for going from a .S to a .o
%.o: %.S
$(OBJDIR)/%.o: %.S
	$(TCPP) $(CFLAGS) $(DEFS) -o $@ $<

# Other useful targets
#
# %.S: %.c - See what the assembler produces from the C code, however you can
# also just look at the final disassembly.
#
# %.lst: %.S - Get an assembler listing
#
# %.cpp: %.S - Preprocess PowerPC assembler source to stdout
#
# %.cpp: %.c - Preprocess C source to stdout

%.S: %.c
	$(CC) $(CFLAGS) $(DEFS) -S -o $@ $<

%.lst: %.S
	$(CC_ASM) $(CFLAGS) $(DEFS) -Wa,-al -Wa,--listing-cont-lines=20 $< > $@

%.cpp: %.S
	$(CC_ASM) $(CFLAGS) $(DEFS) -E $<

%.cpp: %.c
	$(CC) $(CFLAGS) $(DEFS) -E $<

# From the GNU 'Make' manual - these scripts uses the preprocessor to
# create dependency files (*.d), then mungs them slightly to make them
# work as Make targets. The *.d files are include-ed in the
# subdirectory Makefiles.

#$(OBJDIR)/%.d: %.c
#	@set -e; rm -f $@; \
#	echo -n "$(OBJDIR)/" > $@.$$$$; \
#	$(CC_ASM) -MM $(INCLUDES) $(CPPFLAGS) $(DEFS) $< >> $@.$$$$; \
#	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
#	rm -f $@.$$$$

#$(OBJDIR)/%.d: %.S
#	@set -e; rm -f $@; \
#	echo -n "$(OBJDIR)/" > $@.$$$$; \
#	$(CC_ASM) -MM $(INCLUDES) $(CPPFLAGS) $(DEFS) $< >> $@.$$$$; \
#	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
#	rm -f $@.$$$$


