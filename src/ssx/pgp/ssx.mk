# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ssx/pgp/ssx.mk $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2014,2016
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
#  Make header for PgP SSX builds
#
# The application may define the following variables to control the
# build process:
#
# APP_INCLUDES       : Aplication-specific header search paths
#
# DEFS               : A string of -D<symbol>[=<value>] to control compilation
#
# SSX                : Default ..; The path to the SSX source code.
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
# POREPATH           : Overrideable path to the PORE binutils

export OCCROOT = $(dir $(lastword $(MAKEFILE_LIST)))../../

ifndef SIMICS_ENVIRONMENT
SIMICS_ENVIRONMENT=0
endif

ifndef GCC-TOOL-PREFIX

# CROSS_PREFIX may be set by
# op-build/openpower/package/occ/occ.mk

ifdef CROSS_PREFIX
GCC-TOOL-PREFIX = $(CROSS_PREFIX)
else
GCC-TOOL-PREFIX = powerpc64-unknown-linux-gnu-
endif

endif

ifndef HOST-PREFIX
HOST-PREFIX = x86_64-pc-linux-gnu-
endif

CC_ASM  = $(GCC-TOOL-PREFIX)gcc
CC      = $(TRACEPP) $(GCC-TOOL-PREFIX)gcc
LD      = $(GCC-TOOL-PREFIX)ld
OBJDUMP = $(GCC-TOOL-PREFIX)objdump

JAIL    = $(HOST-PREFIX)jail

ifndef OCC_OP_BUILD
AS      = $(JAIL) /usr/bin/as
AR      = $(JAIL) /usr/bin/ar
OBJCOPY = $(JAIL) /usr/bin/objcopy
CPP     = $(JAIL) /usr/bin/cpp
else
AS      = $(GCC-TOOL-PREFIX)as
AR      = $(GCC-TOOL-PREFIX)ar
OBJCOPY = $(GCC-TOOL-PREFIX)objcopy
CPP     = $(GCC-TOOL-PREFIX)cpp
endif

ifndef POREPATH
$(warning The POREPATH variable is not defined; Defaulting to current PATH)
endif

PORE-AS      = $(POREPATH)pore-elf64-as
PORE-AS      = $(POREPATH)pore-elf64-as
PORE-LD      = $(POREPATH)pore-elf64-ld
PORE-OBJCOPY = $(POREPATH)pore-elf64-objcopy

ifeq "$(SSX)" ""
SSX = ..
endif

ifeq "$(LIB)" ""
LIB = ../../lib
endif

ifeq "$(SSX_TIMER_SUPPORT)" ""
SSX_TIMER_SUPPORT  = 1
endif

ifeq "$(SSX_THREAD_SUPPORT)" ""
SSX_THREAD_SUPPORT = 1
endif

ifeq "$(PPC405_MMU_SUPPORT)" ""
PPC405_MMU_SUPPORT = 1
endif

ifeq "$(PGP_ASYNC_SUPPORT)" ""
PGP_ASYNC_SUPPORT = 1
endif

ifndef GCC-O-LEVEL
GCC-O-LEVEL = -Os
endif

GCC-DEFS += -DSIMICS_ENVIRONMENT=$(SIMICS_ENVIRONMENT)
GCC-DEFS += -DSSX_TIMER_SUPPORT=$(SSX_TIMER_SUPPORT) 
GCC-DEFS += -DSSX_THREAD_SUPPORT=$(SSX_THREAD_SUPPORT) 
GCC-DEFS += -DPPC405_MMU_SUPPORT=$(PPC405_MMU_SUPPORT)
DEFS += $(GCC-DEFS) -DPGAS_PPC=1 -DCONFIGURE_PTS_SLW=0
PORE-DEFS += $(GCC-DEFS)

############################################################################

INCLUDES += $(APP_INCLUDES) \
	-I$(SSX)/ssx -I$(SSX)/ppc32 -I$(SSX)/ppc405 \
	-I$(SSX)/pgp -I$(SSX)/pgp/registers \
	-I$(LIB)

PIPE-CFLAGS = -pipe -Wa,-m405

GCC-CFLAGS += -g -Wall -fsigned-char -msoft-float  \
	-m32 -mbig-endian -mcpu=405 -mmultiple -mstring \
	-meabi -msdata=eabi -ffreestanding -fno-common \
	-fno-inline-functions-called-once

CFLAGS      =  -c $(GCC-CFLAGS) $(PIPE-CFLAGS) $(GCC-O-LEVEL) $(INCLUDES) 
PORE-CFLAGS =  -E $(GCC-CFLAGS) $(OPT) $(INCLUDES) 
CPPFLAGS += -m32 -mcpu=405 -msdata=eabi -meabi -mstring -mmultiple

############################################################################

# Build object code
#
# %.o: %.c - Compile C code
#
# %.o: %.S - Compile PowerPC assembler (including PGAS-PPC assembly)

%.o: %.c
	$(CC) $(CFLAGS) $(DEFS) -o $@ $<

%.o: %.S
	$(CC_ASM) $(CFLAGS) $(DEFS) -o $@ $<

#Temporary always use PGAS PPC Assembler for compiling .pS files.
#relocatable symbols are being added to the GPE (.pS) files and
#so need new way to compile using GNU assembler.
PGAS_PPC=1

# use "make PGAS_PPC=1" to compile .pS file using PGAS PPC assembler.
# If PGAS_PPC=1 is not part of the make command, then GNU assembler is
# used for compiling .pS files.
ifdef PGAS_PPC

ifneq ($(MAKE_PORE_HOOKS),)

# This Makefile included here defines how to convert *.pS into both the object
# file and the hooks object file.

include $(MAKE_PORE_HOOKS)

else

%.o: %.pS
	$(CC_ASM) -x assembler-with-cpp $(PORE-CFLAGS) $(PORE-DEFS)  $< | $(PORE-AS) - -o $@
endif

%.lst: %.pS
	$(CC_ASM) -x assembler-with-cpp  $(CFLAGS) $(DEFS) \
		-Wa,-al -Wa,--listing-cont-lines=20 $< > $@

else
%.o: %.pS
	$(CPP_ASM) -x assembler-with-cpp $(PORE-CFLAGS) $(PORE-DEFS)  $< | $(PORE-AS) - -o $@
endif

# Other useful targets
#
# %.S: %.c - See what the assembler produces from the C code, however you can
# also just look at the final disassembly.
#
# %.lst: %.S - Get an assembler listing
#
# %.cpp: %.S - Preprocess PowerPC assembler source to stdout
#
# %.cpp: %.pS - Preprocess PORE assembler source to stdout
#
# %.cpp: %.c - Preprocess C source to stdout

%.S: %.c
	$(CC) $(CFLAGS) $(DEFS) -S -o $@ $<

%.lst: %.S
	$(CC_ASM) $(CFLAGS) $(DEFS) -Wa,-al -Wa,--listing-cont-lines=20 $< > $@

%.cpp: %.S
	$(CC_ASM) $(CFLAGS) $(DEFS) -E $<

%.cpp: %.pS
	$(CC) -x assembler-with-cpp $(CFLAGS) $(DEFS) -E $<

%.cpp: %.c
	$(CC) $(CFLAGS) $(DEFS) -E $<

# From the GNU 'Make' manual - these scripts uses the preprocessor to
# create dependency files (*.d), then mungs them slightly to make them
# work as Make targets. The *.d files are include-ed in the
# subdirectory Makefiles.

%.d: %.c
	@set -e; rm -f $@; \
	$(CC_ASM) -MM $(INCLUDES) $(CPPFLAGS) $(DEFS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d: %.S
	@set -e; rm -f $@; \
	$(CC_ASM) -MM $(INCLUDES) $(CPPFLAGS) $(DEFS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d: %.pS
	@set -e; rm -f $@; \
	$(CC_ASM) -MM $(INCLUDES) $(CPPFLAGS) -x assembler-with-cpp $(DEFS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

############################################################################
#
# GCC Compiler flags used in these builds.  Comments, or reasons for
# non-obvious choices appear in [] after the GCC documentation.
#
#`-Os'
#     Optimize for size.  `-Os' enables all `-O2' optimizations that do
#     not typically increase code size.  It also performs further
#     optimizations designed to reduce code size.
#
#     `-Os' disables the following optimization flags:
#          -falign-functions  -falign-jumps  -falign-loops
#          -falign-labels  -freorder-blocks  -freorder-blocks-and-partition
#          -fprefetch-loop-arrays  -ftree-vect-loop-version
#
#     If you use multiple `-O' options, with or without level numbers,
#     the last such option is the one that is effective.
#
#`-g'
#     Produce debugging information in the operating system's native
#     format (stabs, COFF, XCOFF, or DWARF 2).  GDB can work with this
#     debugging information.
#
#     On most systems that use stabs format, `-g' enables use of extra
#     debugging information that only GDB can use; this extra information
#     makes debugging work better in GDB but will probably make other
#     debuggers crash or refuse to read the program.  If you want to
#     control for certain whether to generate the extra information, use
#     `-gstabs+', `-gstabs', `-gxcoff+', `-gxcoff', or `-gvms' (see
#     below).
#
#     GCC allows you to use `-g' with `-O'.  The shortcuts taken by
#     optimized code may occasionally produce surprising results: some
#     variables you declared may not exist at all; flow of control may
#     briefly move where you did not expect it; some statements may not
#     be executed because they compute constant results or their values
#     were already at hand; some statements may execute in different
#     places because they were moved out of loops.
#
#     Nevertheless it proves possible to debug optimized output.  This
#     makes it reasonable to use the optimizer for programs that might
#     have bugs.
#
#`-Wall'
#     Turns on all optional warnings which are desirable for normal code.
#     At present this is `-Wcomment', `-Wtrigraphs', `-Wmultichar' and a
#     warning about integer promotion causing a change of sign in `#if'
#     expressions.  Note that many of the preprocessor's warnings are on
#     by default and have no options to control them.
#
#`-Werror'
#     Make all warnings into errors.
#  
#`-fsigned-char'
#     Let the type `char' be signed, like `signed char'.
#
#     Note that this is equivalent to `-fno-unsigned-char', which is the
#     negative form of `-funsigned-char'.  Likewise, the option
#     `-fno-signed-char' is equivalent to `-funsigned-char'.
#
#`-msoft-float'
#     Generate output containing library calls for floating point.
#     *Warning:* the requisite libraries are not available for all ARM
#     targets.  Normally the facilities of the machine's usual C
#     compiler are used, but this cannot be done directly in
#     cross-compilation.  You must make your own arrangements to provide
#     suitable library functions for cross-compilation.
#
#     `-msoft-float' changes the calling convention in the output file;
#     therefore, it is only useful if you compile _all_ of a program with
#     this option.  In particular, you need to compile `libgcc.a', the
#     library that comes with GCC, with `-msoft-float' in order for this
#     to work.
#
#`-pipe'
#     Use pipes rather than temporary files for communication between the
#     various stages of compilation.  This fails to work on some systems
#     where the assembler is unable to read from a pipe; but the GNU
#     assembler has no trouble.
#
#`-mmultiple'
#`-mno-multiple'
#     Generate code that uses (does not use) the load multiple word
#     instructions and the store multiple word instructions.  These
#     instructions are generated by default on POWER systems, and not
#     generated on PowerPC systems.  Do not use `-mmultiple' on little
#     endian PowerPC systems, since those instructions do not work when
#     the processor is in little endian mode.  The exceptions are PPC740
#     and PPC750 which permit the instructions usage in little endian
#     mode.
#
#`-mstring'
#`-mno-string'
#     Generate code that uses (does not use) the load string instructions
#     and the store string word instructions to save multiple registers
#     and do small block moves.  These instructions are generated by
#     default on POWER systems, and not generated on PowerPC systems.
#     Do not use `-mstring' on little endian PowerPC systems, since those
#     instructions do not work when the processor is in little endian
#     mode.  The exceptions are PPC740 and PPC750 which permit the
#     instructions usage in little endian mode.
#
#`-meabi'
#`-mno-eabi'
#     On System V.4 and embedded PowerPC systems do (do not) adhere to
#     the Embedded Applications Binary Interface (eabi) which is a set of
#     modifications to the System V.4 specifications.  Selecting `-meabi'
#     means that the stack is aligned to an 8 byte boundary, a function
#     `__eabi' is called to from `main' to set up the eabi environment,
#     and the `-msdata' option can use both `r2' and `r13' to point to
#     two separate small data areas.  Selecting `-mno-eabi' means that
#     the stack is aligned to a 16 byte boundary, do not call an
#     initialization function from `main', and the `-msdata' option will
#     only use `r13' to point to a single small data area.  The `-meabi'
#     option is on by default if you configured GCC using one of the
#     `powerpc*-*-eabi*' options.
#
# [We elected to use the EABI to reduce stack requirements and possibly reduce
# code size and improve performance.  In practice it probably has little real
# effect since the code size and performance improvements only apply to global
# variables <= 8 bytes, and our applications will not have deeply nested call
# trees. Still, much of the assembler code requires/assumes the EABI is in
# place, and it certainly doesn't hurt anything to use it.]
#
#`-msdata=eabi'
#     On System V.4 and embedded PowerPC systems, put small initialized
#     `const' global and static data in the `.sdata2' section, which is
#     pointed to by register `r2'.  Put small initialized non-`const'
#     global and static data in the `.sdata' section, which is pointed
#     to by register `r13'.  Put small uninitialized global and static
#     data in the `.sbss' section, which is adjacent to the `.sdata'
#     section.  The `-msdata=eabi' option is incompatible with the
#     `-mrelocatable' option.  The `-msdata=eabi' option also sets the
#     `-memb' option.
#
#`-memb'
#     On embedded PowerPC systems, set the PPC_EMB bit in the ELF flags
#     header to indicate that `eabi' extended relocations are used.
#
#`-ffreestanding'
#     Assert that compilation takes place in a freestanding environment.
#     This implies `-fno-builtin'.  A freestanding environment is one
#     in which the standard library may not exist, and program startup
#     may not necessarily be at `main'.  The most obvious example is an
#     OS kernel.  This is equivalent to `-fno-hosted'.
#
#`-fno-common'
#     In C, allocate even uninitialized global variables in the data
#     section of the object file, rather than generating them as common
#     blocks.  This has the effect that if the same variable is declared
#     (without `extern') in two different compilations, you will get an
#     error when you link them.  The only reason this might be useful is
#     if you wish to verify that the program will work on other systems
#     which always work this way.
#
# [It is always assumed to be an error if two C source files declare global
# variables of the same name, since it is not clear whether this was intended
# or not.]
# 
#`-finline-functions-called-once'
#`-fno-inline-functions-called-once'
#     Consider all `static' functions called once for inlining into their
#     caller even if they are not marked `inline'.  If a call to a given
#     function is integrated, then the function is not output as
#     assembler code in its own right.
#
#     Enabled if `-funit-at-a-time' is enabled.
#
# [There is a 'bug' in GCC related to inlining static, called-once
# functions. GCC allocates stack space in the caller for the stacks of ALL
# inlined functions of this type, even if they are called sequentially,
# leading in some cases to kilobytes of wasted stack space. If you want to
# inline a static called-once function you will need to explicity declare it
# as 'inline'.]


