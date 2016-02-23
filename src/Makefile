# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/Makefile $
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

ifndef ROOTPATH
ROOTPATH = $(shell pwd)
export OCCROOT = $(ROOTPATH)
endif

#*******************************************************************************
# mk variable Declaration
#*******************************************************************************
SUBDIRS = tracepp occBootLoader occ occApplet
COMBINE_IMAGE_SUBDIRS = occBootLoader occ occApplet
CLEAN_SUBDIRS = tracepp occBootLoader occ occApplet ssx/pgp lib
CLEANCMD = $(MAKE) clean -C $(dir)
ALLCMD = $(MAKE) -C $(dir)
COMBINEIMAGE = $(MAKE) combineImage -C $(dir)
IMAGEFILE = image.bin

export TRACEPP = $(ROOTPATH)/tracepp/tracepp
TRACEHASH = $(ROOTPATH)/tracepp/tracehash.pl

#*******************************************************************************
# Compilation 
#*******************************************************************************
all:
	$(foreach dir,$(SUBDIRS),$(ALLCMD) &&) true

#*******************************************************************************
# Combine Objects/Executables into OCC Image
#*******************************************************************************
.PHONY : combineImage
combineImage:
	 rm -rf $(IMAGEFILE)
	 $(foreach dir,$(COMBINE_IMAGE_SUBDIRS),$(COMBINEIMAGE) &&) true

#*******************************************************************************
# Create lid or ruhx out of OCC Image
#*******************************************************************************
.PHONY : 81e004%.ruhx
81e004%.ruhx:
	occgenlid.pl -i ${IMAGEFILE} -l $(basename $@)

.PHONY : 81e004%.lid
81e004%.lid:
	cp ${IMAGEFILE} $(dir $(IMAGEFILE))/$@

#*******************************************************************************
# Create occStringFile
#*******************************************************************************
# FIXME depending on where .o files end up
.PHONY : tracehash
tracehash:
	$(TRACEHASH) -c -d . -s $(ROOTPATH)/occStringFile

#*******************************************************************************
# Create occcSrcParse.C
#*******************************************************************************
# FIXME error log parser plugins not relevant to OpenPOWER, tool location unknown
.PHONY : srcPluginFile
srcPluginFile:
	scanforsrcs.pl

#*******************************************************************************
# Clean
#*******************************************************************************
.PHONY : clean
clean: 
	$(foreach dir,$(CLEAN_SUBDIRS),$(CLEANCMD) &&) true
	rm -f image.bin
	rm -f occStringFile
	
