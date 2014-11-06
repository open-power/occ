# $Id$

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
# pb000                    pbavari     07/21/11     Created
# th000                    thallet     01/09/12     Removed simics-workspace 
# at009    859308          alvinwan    10/15/2012   Added tracepp support
# at011    866759          alvinwan    01/15/2013   Fix OCC trexStringFile and trace
# rt001    901927          tapiar      10/01/2013   SRC plugin file generation
# gitprep                  sbroyles    11/02/2014   Prepare files for move to git/gnu build
# @endverbatim
#

# >> gitprep
ifndef ROOTPATH
ROOTPATH = $(shell pwd)
export OCCROOT = $(ROOTPATH)
endif
# << gitprep

#*******************************************************************************
# mk variable Declaration
#*******************************************************************************
SUBDIRS = occBootLoader occ occApplet
COMBINE_IMAGE_SUBDIRS = occBootLoader occ occApplet
CLEAN_SUBDIRS = occBootLoader occ occApplet ssx/pgp lib
CLEANCMD = $(MAKE) clean -C $(dir); 
ALLCMD = $(MAKE) -j6 -C $(dir);
COMBINEIMAGE = $(MAKE) combineImage -C $(dir);
IMAGEFILE = image.bin

#*******************************************************************************
# Compilation 
#*******************************************************************************

all:
	$(foreach dir,$(SUBDIRS),$(ALLCMD))

#*******************************************************************************
# Combine Objects/Executables into OCC Image
#*******************************************************************************
.PHONY : combineImage
combineImage:
	 rm -rf $(IMAGEFILE)
	 $(foreach dir,$(COMBINE_IMAGE_SUBDIRS),$(COMBINEIMAGE))

#*******************************************************************************
# Create lid or ruhx out of OCC Image
#*******************************************************************************
.PHONY : 81e004%.ruhx
81e004%.ruhx:
	./occ/tools/occgenlid.pl -i ${IMAGEFILE} -l $(basename $@)

.PHONY : 81e004%.lid
81e004%.lid:
	cp ${IMAGEFILE} $(dir $(IMAGEFILE))/$@

#*******************************************************************************
# Create trexStringFile
#*******************************************************************************
.PHONY : tracehash
tracehash:
	mkdir -p ${sb}/../obj
	tracehash.pl -c -d ${SANDBOXBASE}/obj/ppc/occc/405/ -s ${SANDBOXBASE}/obj/trexStringFile

#*******************************************************************************
# Create occcSrcParse.C @rt001
#*******************************************************************************
.PHONY : srcPluginFile
srcPluginFile:
	./occ/tools/scanforsrcs.pl

#*******************************************************************************
# Clean
#*******************************************************************************
clean: 
	$(foreach dir,$(CLEAN_SUBDIRS),$(CLEANCMD))
	rm -rf image.bin
	
