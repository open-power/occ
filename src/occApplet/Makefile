# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occApplet/Makefile $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2011,2014
# [+] Google Inc.
# [+] International Business Machines Corp.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.
#
# IBM_PROLOG_END_TAG

ifndef ROOTPATH
ROOTPATH = $(shell pwd)/../
export OCCROOT = $(ROOTPATH)
endif

#*******************************************************************************
# Variables
#*******************************************************************************
SUBDIRS = productApplet testApplet
CLEANCMD = $(MAKE) clean -C $(dir)
ALLCMD = $(MAKE) -C $(dir)
COMBINEIMAGE = $(MAKE) combineImage -C $(dir)

#*******************************************************************************
# Compilation 
#*******************************************************************************
all:
	$(foreach dir,$(SUBDIRS),$(ALLCMD) &&) true

#*******************************************************************************
# combineImage 
#*******************************************************************************
.PHONY : combineImage
combineImage:
	 $(foreach dir,$(SUBDIRS),$(COMBINEIMAGE) &&) true

#*******************************************************************************
# Clean
#*******************************************************************************
clean: 
	$(foreach dir,$(SUBDIRS),$(CLEANCMD) &&) true

