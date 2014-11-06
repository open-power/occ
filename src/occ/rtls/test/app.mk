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
#  @verbatim
#
#
# Change Log ******************************************************************
# Flag     Defect/Feature  User        Date         Description
# ------   --------------  ----------  ------------ -----------
#                          np, dw      08/10/2011   created by nguyenp & dwoodham
#                                                   borrowed from occ/thread/test/app.mk
#  @01                     dwoodham    08/30/2011   Use link script from OCC vs. from SSX
#  @rc003                  rickylie    02/03/2012   Verify & Clean Up OCC Headers & Comments
#
# @endverbatim
#


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

.PHONY : doc
doc:
	doxygen doc/Doxyfile

include $(OBJECTS:.o=.d) 
