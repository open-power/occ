# $Id$
#  @file Makefile
#
#  @brief OCC Product Applet Makefile
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
# @ani01                   abagepa     08/08/2011   Creating to "test" applets
#                                                   as placeholders for real applets
#                                                   Update to include for easier compiling
# @pb006                   pbavari     09/16/2011   Display size support
# @pb004                   pbavari     09/20/2011   Initialization Section support
# @pb00A                   pbavari     11/14/2011   Moved sensor_init from applet to init section
# @th005                   thallet     11/23/2011   Sensor querySensorList support
# @pb00C                   pbavari     01/20/2012   Added debug_trace.mk
# @rc003                   rickylie    02/03/2012   Verify & Clean Up OCC Headers & Comments
# @at009   859308          alvinwan    10/15/2012   Added tracepp support
# @th029                   thallet     01/23/2013   MD5sum in applet version & include aplt headers
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
PRODUCTAPPLET = .
LIB = ../../lib

# New product applet source file must to listed as part of the SOURCES variable
# to create product applet image. Prodcut applet images will be added to the
# mainstore single image in the order source files are listed in SOURCES 
# variable. 
# @ani01c 
# Note: New applets must be updated in the applet enum list
#       see occ/aplt/appletManager.h

# >> gitprep
# Add missing flags for GNU build
LDFLAGS = -L $(SSX)/ssx -L $(SSX)/ppc32 -L $(SSX)/ppc405 -L $(SSX)/pgp \
	  -L $(OCC) -L $(LIB) -lssx -lppc32 --oformat=elf32-powerpc -melf32ppc
buildImage = $(LD) -R $(OCC)/occ.out $(obj) -Tlinkscript $(LDFLAGS) \
	-Map $(basename $(obj)).map -Bstatic -o $(basename $(obj)).out;\
	$(OBJCOPY) -I elf32-powerpc -O binary $(basename $(obj)).out $(basename $(obj)).bin; \
	$(OBJDUMP) -d $(basename $(obj)).out > $(basename $(obj)).dis; \
	$(BOOTLOADER)/imageHdrScript $(basename $(obj)).bin `md5sum $(OCC)/occ.out | cut -c 1-4`; 
# << gitprep

image =  $(BOOTLOADER)/imageHdrScript $(basename $(obj)).bin combineImage;\
         $(BOOTLOADER)/imageHdrScript $(basename $(obj)).out displaySize;
             
#*******************************************************************************
# Includes
#*******************************************************************************
include $(SSX)/pgp/ssx.mk
#$pb00Ca - Added for debug traces
include $(OCC)/debug_trace.mk
include productappletfiles.mk

# >> gitprep
# Add needed includes for GNU build
INCLUDES = -I. -I$(OCC)/incl -I$(OCC)/errl -I$(OCC)/trac -I$(LIB) -I$(SSX)/ssx \
			-I$(OCC)/sensor -I$(OCC) -I$(OCC)/rtls -I$(OCC)/cmdh -I$(OCC)/pss -I$(OCC)/gpe \
			-I$(OCC)/aplt/incl -I$(OCC)/aplt -I$(OCC)/cent -I$(OCC)/proc -I$(OCC)/thread \
			-I$(SSX)/ppc405 -I$(SSX)/pgp -I$(SSX)/ppc32 -I$(SSX)/pgp/registers
# << gitprep

#*******************************************************************************
# Flags
#*******************************************************************************
#D = -DSIMICS_MAGIC_PANIC=1 \
     -DINITIALIZE_SIMICS_IO=1

DEFS += $(D)
DEFS += -DAPPLET_BUILD=1
# >> gitprep
# Very important we build applets with custom cfg header
DEFS += -DUSE_SSX_APP_CFG_H=1
# << gitprep

# If this makefile is called as "make NO_TRAC_STRINGS=1" then trace strings 
# won't be built into the image.  This will be used for metrics regarded to the 
# realistic OCC Code Size. Note that "make clean" must be run before this define
# will be picked up by the compiler, otherwise previously compiled objects will 
# be used.  You can also see the space used by strings by running: 
# strings occ.bin | \grep "ERR\|INF\|IMP" | sed 's/^...: %s: //g' | wc -m
ifdef NO_TRAC_STRINGS
D += -DNO_TRAC_STRINGS=1  
endif

# Do not use SDA sections for product applet
GCC-CFLAGS = -c -g -Wall -fsigned-char -msoft-float -pipe \
             -Wa,-m405 -m32 -mcpu=405 -mmultiple -mstring -meabi \
             -ffreestanding -Os -mno-sdata
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
