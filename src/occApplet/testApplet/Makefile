#  @file Makefile
#
#  @brief OCC Test Applet Makefile
#

#  @page ChangeLogs Change Logs
#  @section Makefile
#  @verbatim
#
#
# Change Log ******************************************************************
# Flag     Defect/Feature  User        Date         Description
# ------   --------------  ----------  ------------ -----------
# @pb001                   pbavari     07/18/2011   Created
# @pb002                   pbavari     08/17/2011   Added sensorTest.c
# @cc000                   cjcain      08/30/2011   Added apssTest.c
# @pb004                   pbavari     09/15/2011   Added support for calling
#                                                   product aplt from test aplt
# @02                      tapiar      10/03/2011   Adding new dir includes
# @dw000                   dwodham     12/13/2011   Add include for appletId
# @rc001                   rickylie    01/16/2012   Include debug_trace.mk
# @rc003                   rickylie    02/03/2012   Verify & Clean Up OCC Headers & Comments
# @at002                   alvinwan    02/10/2012   Added errlTest.c
# @at009   859308          alvinwan    10/15/2012   Added tracepp support
# @th029                   thallet     01/23/2013   Added md5sum in version
#
# @endverbatim
#

# >> gitprep
ifndef ROOTPATH
ROOTPATH = $(shell pwd)/../../
export OCCROOT = $(ROOTPATH)
endif
# << gitprep

#*******************************************************************************
# mk variable Declaration
#*******************************************************************************
OCC = ../../occ
BOOTLOADER = ../../occBootLoader
SSX = ../../ssx
TESTAPPLET = .
LIB = ../../lib

# >> gitprep
# Add missing link flags fir GNU build
LDFLAGS += --oformat=elf32-powerpc -melf32ppc

buildImage = $(LD) -R $(OCC)/occ.out $(obj) -Tlinkscript $(LDFLAGS) \
			-Map $(basename $(obj)).map -Bstatic -o $(basename $(obj)).out;\
	        $(OBJCOPY) -I elf32-powerpc -O binary $(basename $(obj)).out $(basename $(obj)).bin; \
			$(OBJDUMP) -d $(basename $(obj)).out > $(basename $(obj)).dis; \
			$(BOOTLOADER)/imageHdrScript $(basename $(obj)).bin `md5sum $(OCC)/occ.out | cut -c 1-4`; \
# << gitprep

#*******************************************************************************
# Includes
#*******************************************************************************
include $(SSX)/pgp/ssx.mk
#@rc001a
include $(OCC)/debug_trace.mk
include testappletfiles.mk

INCLUDES = -I. -I$(OCC)/rtls -I$(OCC)/incl -I$(OCC)/errl -I$(OCC)/trac -I$(LIB) -I$(SSX)/ssx 
INCLUDES += -I$(SSX)/ppc405 -I$(SSX)/pgp -I$(SSX)/ppc32 -I$(SSX)/pgp/registers
INCLUDES += -I$(OCC)/sensor -I$(OCC)/pss -I$(OCC)/gpe -I$(OCC) -I$(OCC)/aplt -I$(OCC)/dcom 
# >> gitprep
# Add missing include paths
INCLUDES += -I$(OCC)/aplt/incl -I$(OCC)/cmdh
# << gitprep

#*******************************************************************************
# Flags
#*******************************************************************************
#D = -DSIMICS_MAGIC_PANIC=1 \
     -DINITIALIZE_SIMICS_IO=1

DEFS += $(D)
DEFS +=  -DOCC=1 \
    	 -DUSE_SSX_APP_CFG_H=1

# Do not use SDA sections for product applet 
# >> gitprep
# Add missing compile flags for GNU build
GCC-CFLAGS = -c -g -Wall -fsigned-char -msoft-float -pipe \
             -m32 -Wa,-m405 -mcpu=405 -mmultiple -mstring -meabi \
             -ffreestanding -Os -mno-sdata 
# << gitprep

#*******************************************************************************
# Compilation 
#*******************************************************************************
all: $(TESTAPLT_OBJECTS) 
	 $(CPP) -P $(DEFS) < linkTestApplet.cmd > linkscript
	 $(foreach obj,$(TESTAPLT_OBJECTS),$(buildImage))

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
