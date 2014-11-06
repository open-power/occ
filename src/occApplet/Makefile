#  @file Makefile
#
#  @brief OCC Applet Makefile
#

#  @page ChangeLogs Change Logs
#  @section Makefile
#  @verbatim
#
#
# Change Log ******************************************************************
# Flag     Defect/Feature  User        Date         Description
# ------   --------------  ----------  ------------ -----------
#  @pb001                  pbavari     07/21/2011   Created
#  @rc003                  rickylie    02/03/2012   Verify & Clean Up OCC Headers & Comments
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
SUBDIRS = productApplet testApplet
CLEANCMD = $(MAKE) clean -C $(dir); 
ALLCMD = $(MAKE) -j6 -C $(dir);
COMBINEIMAGE = $(MAKE) combineImage -C $(dir);

#*******************************************************************************
# Compilation 
#*******************************************************************************
all:
	$(foreach dir,$(SUBDIRS),$(ALLCMD))

#*******************************************************************************
# combineImage 
#*******************************************************************************
.PHONY : combineImage
combineImage:
	 $(foreach dir,$(SUBDIRS),$(COMBINEIMAGE))
	
#*******************************************************************************
# Clean
#*******************************************************************************
clean: 
	$(foreach dir,$(SUBDIRS),$(CLEANCMD))
	
