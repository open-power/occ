# $Id$

#  @file app.mk
#
#  @brief mk occ application
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
#
#  @page ChangeLogs Change Logs
#  @section app.mk
#  @verbatim
#
#
# Change Log ******************************************************************
# Flag     Defect/Feature  User        Date            Description
# ------   --------------  ----------  ------------    -----------
# @pb000                    pbavari     07/11/2011     Added imageHdrScript usage
# @dw001                    dwoodham    08/23/2011     Migrate to OCC version of linkocc.cmd
# @pb006                    pbavari     09/16/2011     Display size support
# @pb00D                    pbavari     02/21/2012     Support for using GNU assembler for GPE files
# @at007                    alvinwan    05/25/2012     Use complex method for linking pore and PPC objects
# @at009   859308           alvinwan    10/15/2012     Added tracepp support
# @gm010   901580           milesg      10/06/2013     Low Level FFDC support
#
# @endverbatim
#



#*******************************************************************************
# mk variable Declaration
#*******************************************************************************

OCC = .
SSX = ../ssx
LIB = ../lib
PGP = $(SSX)/pgp
SSXDIR =  $(SSX)/ssx
PPC405 = $(SSX)/ppc405
BOOTLOADER = ../occBootLoader

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

INCLUDES += -I$(OCC) -I$(LIB) 

include $(PGP)/ssx.mk
include $(PGP)/ssxpgpfiles.mk
include $(SSXDIR)/ssxssxfiles.mk
include $(PPC405)/ssxppc405files.mk
include $(LIB)/libgpefiles.mk

# >> gitprep
# Disable this for GNU builds
# @at007a - If it's for OCC_FIRMWARE and non-PGAS_PPC build, the library gpefiles needs to be taken out of libssx.a
#all_gpefiles += $(if $(filter -DOCC_FIRMWARE=1,$(DEFS)),\
#                     $(if $(PGAS_PPC),,${addprefix ../lib/,${LIB_PSOBJECTS}}),\
#                     )
#
# Use this instead:
all_gpefiles += ${addprefix ../lib/,${LIB_PSOBJECTS}}
# << gitprep

GPE_OBJECTS = ${all_gpefiles}
OCC_OBJECTS = ${occ_cfiles} ${occ_Sfiles}
OBJECTS = ${GPE_OBJECTS} ${OCC_OBJECTS}

# >> gitprep
# Need full paths to all .o files for commands that follow
LDFLAGS = \
		  -L $(SSX)/ssx \
		  -L $(SSX)/ppc32 \
		  -L $(SSX)/ppc405 \
		  -L $(SSX)/pgp \
		  -L $(OCC) \
		  -L $(LIB) \
		  -L $(OCC)/amec \
		  -L $(OCC)/aplt \
		  -L $(OCC)/cent \
		  -L $(OCC)/cmdh \
		  -L $(OCC)/dcom \
		  -L $(OCC)/errl \
		  -L $(OCC)/gpe \
		  -L $(OCC)/proc \
		  -L $(OCC)/pss \
		  -L $(OCC)/rtls \
		  -L $(OCC)/sensor \
		  -L $(OCC)/thread \
		  -L $(OCC)/timer \
		  -L $(OCC)/trac \
		  -lssx -lppc32 --oformat=elf32-powerpc -melf32ppc
# << gitprep

# @pb00Da - Added for linking files compiled using gnu assembler	  
LDARGS = --no-warn-mismatch --accept-unknown-input-arch
# @pb00Dc - Changed to use GCC-DEFS instead of DEFS so that different defines
# can be used for compiling .S and .pS files
GCC-DEFS += $(D)

#*******************************************************************************
# compilation 
#*******************************************************************************
# @pb000a - Added $(BOOTLOADER)/imageHdrScript usage for building occ image
# @dw001c - Use OCC version of linkscript vs. the PGP version.
# @th029c - Added CFLAGS to OCC files if GCove is enabled and add md5sum as 
#           version (first 4 bytes)
ifdef GCOV_CODE_COVERAGE      
${occ_cfiles}: CFLAGS += -fprofile-arcs -ftest-coverage
endif

# >> gitprep
# Disable for GNU builds
#ifdef PGAS_PPC
#all: $(OBJECTS) libssx.a
#	$(MAKE) -w -C $(PGP) DEFS="$(DEFS)" -e 
#	$(CPP) -P $(DEFS) < $(OCC)/linkocc.cmd > linkscript
#	$(LD) $(OBJECTS)  \
#	-Tlinkscript $(LDFLAGS) $(LDARGS) -Map $(APP).map -Bstatic -o $(APP).out
#else
# << gitprep

# @at007a - Use complex method for linking pore and PPC objects
all: $(OBJECTS) libssx.a
# >> gitprep
# add -w to inhibit all warnings
	$(MAKE) -w -C $(PGP) DEFS="$(DEFS)" -e 
# << gitprep
	$(CPP) -P $(DEFS) < $(OCC)/linkocc.cmd > linkscript

        # Create gpe binary with symbols undefined
        #   First stage link, no ppc syms yet.  Link with -noinhibit-exec to prevent
        #   undefined syms killing output.  This binary won't be located at the
        #   correct base address, but can reasonably be assumed to be the correct
        #   size.  (If it isn't the right size, then iterate over another link stage.)
	$(PORE-LD) -o gpe_1.out $(GPE_OBJECTS) -noinhibit-exec -Map gpe_1.map -e 0
	$(PORE-OBJCOPY) -O binary gpe_1.out gpe_1.bin

        # Insert gpe binary to occ
        #   Don't link with -zmuldefs here so we get an error if PORE symbols
        #   clash with PowerPC ones.
# >> gitprep
# $(OCC_OBJECTS) does nothing
#	$(LD) -o occ_1.out $(OCC_OBJECTS) -R gpe_1.out \
#	  -Tlinkscript $(LDFLAGS) $(LDARGS) -Map occ_1.map -Bstatic -b binary gpe_1.bin
	$(LD) -o occ_1.out -R gpe_1.out \
	  -Tlinkscript $(LDFLAGS) $(LDARGS) -Map occ_1.map -Bstatic -b binary gpe_1.bin
# << gitprep
        # Find the address of gpe binary and create gpe binary with symbols relocated
        #   Find where pore binary was placed.  The symbol we look for
        #   incorporates the name of the binary we linked above.
        #   Second stage link of pore object at the correct address (from symbols above)
        #   now gets ppc syms added from occ_1.out, -z muldefs stops multiple def
        #   errors when we get pore symbols from occ_1.out too
	pore_base=`nm occ_1.out | grep _binary_gpe_1_bin_start | sed -e s', .*,,'`;$(PORE-LD) -o gpe.out \
	  $(GPE_OBJECTS) -Ttext $$pore_base --no-warn-mismatch --accept-unknown-input-arch -R occ_1.out -z muldefs -Map gpe.map -e 0
	$(PORE-OBJCOPY) -O binary gpe.out gpe.bin
	
        # Insert the gpe binary to occ and relocate the symbols in occ
        #   Create gep_2.out that is used by OCC_OBJECTS to relocate the symboles in occ
        #   so we have all symboles relocated in both OCC_OBJECTS and gpe.bin
# >> gitprep
# Add noinhibit
	pore_base=`nm occ_1.out | grep _binary_gpe_1_bin_start | sed -e s', .*,,'`;$(PORE-LD) -o gpe_2.out \
	  $(GPE_OBJECTS) -Ttext $$pore_base --no-warn-mismatch --accept-unknown-input-arch -noinhibit-exec -Map gpe_2.map -e 0
# << gitprep

# >> gitprep
# $(OCC_OBJECTS) does nothing
#	$(LD) -o occ.out  $(OCC_OBJECTS) -R gpe_2.out \
#	  -Tlinkscript $(LDFLAGS) $(LDARGS)  -Map occ.map -Bstatic -b binary gpe.bin
	$(LD) -o occ.out -R gpe_2.out \
	  -Tlinkscript $(LDFLAGS) $(LDARGS)  -Map occ.map -Bstatic -b binary gpe.bin
# << gitprep

#endif
# >> gitprep
# Specify source object format as elf32-powerpc
	$(OBJCOPY) -I elf32-powerpc -O binary $(APP).out $(APP).bin
# << gitprep
	$(OBJDUMP) -D $(APP).out > $(APP).dis
	$(BOOTLOADER)/imageHdrScript $(APP).bin `md5sum $(APP).out | cut -c 1-4`

libssx.a:
	$(MAKE) -C $(LIB) DEFS="$(DEFS)" -e

.PHONY : combineImage
combineImage:
	 	$(BOOTLOADER)/imageHdrScript $(APP).bin combineImage
	 	$(BOOTLOADER)/imageHdrScript $(APP).out displaySize

#*******************************************************************************
# clean
#*******************************************************************************

.PHONY : clean
clean:
	rm -f *.o *.d *.d.* *.out *.bin *.srec *.dis *.map *.hash linkscript
	rm -f ./*/*.o ./*/*.d ./*/*.d.* ./*/*.hash

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

ifneq ($(MAKECMDGOALS),clean)
include $(OBJECTS:.o=.d) 
endif
