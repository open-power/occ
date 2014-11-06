#  @file Makefile
#
#  @brief bootloader Makefile
#

#  @page ChangeLogs Change Logs
#  @section Makefile
#  @verbatim
#
#
# Change Log ******************************************************************
# Flag     Defect/Feature  User        Date         Description
# ------   --------------  ----------  ------------ -----------
# @pb000                   pbavari     07/01/2011   Created
# @pb001                   pbavari     07/16/2011   Changed to combine image
#                                                   as separate command
# @pb006                   pbavari     09/16/2011   Display size support
# @pb004                   pbavari     09/20/2011   Added occ/ in include path
# @pb007                   pbavari     09/29/2011   Added ppc405 in include path
# @rc003                   rickylie    02/03/2012   Verify & Clean Up OCC Headers & Comments
#
# @endverbatim
#

# >> gitprep
ifndef ROOTPATH
ROOTPATH = $(shell pwd)/../
export OCCROOT = $(ROOTPATH)
endif
# << gitprep

#*******************************************************************************
# mk variable Declaration
#*******************************************************************************
OCC = ../occ
BOOTLOADER = .
SSX = ../ssx
#*******************************************************************************
# Includes
#*******************************************************************************
include bootfiles.mk
include ../ssx/pgp/ssx.mk

INCLUDES = -I. -I$(SSX)/ppc32 -I$(SSX)/pgp -I$(OCC)/incl -I$(OCC)/ \
           -I$(SSX)/ppc405

#*******************************************************************************
# Flags
#*******************************************************************************
OBJECTS   = ${BOOTLOADER_OBJECTS}
EXECUTABLE = bootloader
imageHdrScript = imageHdrScript
imageHdrScript_CC = gcc

#D = -DSIMICS_MAGIC_PANIC=1 \
     -DINITIALIZE_SIMICS_IO=1

DEFS += $(D)

#*******************************************************************************
# Compilation 
#*******************************************************************************

all: ${BOOTLOADER_OBJECTS} imageHdrScript
	$(CPP) -P $(DEFS) < linkboot.cmd > linkscript
	# >> gitprep
	# Add missing link and objcopy flags
	$(LD)  ${BOOTLOADER_OBJECTS} \
		-Tlinkscript $(LDFLAGS) -zmuldefs -Map $(EXECUTABLE).map -melf32ppc --oformat=elf32-powerpc -Bstatic -o \
		$(EXECUTABLE).out
	$(OBJCOPY) -I elf32-powerpc -O binary $(EXECUTABLE).out $(EXECUTABLE).bin
	# << gitprep
	$(OBJDUMP) -d $(EXECUTABLE).out > $(EXECUTABLE).dis
	$(BOOTLOADER)/$(imageHdrScript) $(EXECUTABLE).bin t1
	
.PHONY : combineImage
combineImage:
	 	$(BOOTLOADER)/imageHdrScript $(EXECUTABLE).bin combineImage
	 	$(BOOTLOADER)/imageHdrScript $(EXECUTABLE).out displaySize

imageHdrScript: imageHdrScript.c
	$(imageHdrScript_CC) $(LDFLAGS) -I$(BOOTLOADER) -I$(OCC)/incl -I$(OCC)/ \
	     imageHdrScript.c -o $@
	
#*******************************************************************************
# Clean
#*******************************************************************************
clean: 
	rm -f *.o *.d *.out *.bin *.srec *.dis *.map linkscript imageHdrScript
	rm -rf $(EXECUTABLE) $(IMAGE_FILE)
