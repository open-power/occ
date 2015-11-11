# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/Makefile $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2014,2015
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
ifndef BASE_OBJDIR
BASE_OBJDIR = $(abspath ../obj)
endif

ifndef BASE_SRCDIR
BASE_SRCDIR = $(abspath ./)
endif

OBJDIR = $(BASE_OBJDIR)$(SUB_OBJDIR)
SRCDIR = $(BASE_SRCDIR)$(SUB_SRCDIR)

ifndef PPETRACEPP_DIR
export PPETRACEPP_DIR = $(abspath ppe/tools/ppetracepp)
endif

ifndef TRACEPP_DIR
export TRACEPP_DIR = $(abspath tracepp)
endif

THASH	= $(PPETRACEPP_DIR)/tracehash.pl

OCC_405_IMAGE_NAME = occ_405
OCC_GPE0_IMAGE_NAME = occ_gpe0
OCC_GPE1_IMAGE_NAME = occ_gpe1

NEEDED_IMAGES = \
     $(TRACEPP_DIR)/tracepp \
     $(OBJDIR)/$(OCC_405_IMAGE_NAME)/$(OCC_405_IMAGE_NAME).out \
     $(OBJDIR)/$(OCC_GPE0_IMAGE_NAME)/$(OCC_GPE0_IMAGE_NAME).out \
     $(OBJDIR)/$(OCC_GPE1_IMAGE_NAME)/$(OCC_GPE1_IMAGE_NAME).out

#SIMICS_WS = /gsa/ausgsa/home/d/a/davidy/work/simics-ppeWork

#default is to build images
needed_images: $(NEEDED_IMAGES) tracehash

run: $(NEEDED_IMAGES)
	$(SIMICS_WS)/simics \
	           -e '$$occ_405_binary_to_load=$(OBJDIR)/$(OCC_405_IMAGE_NAME)/$(OCC_405_IMAGE_NAME).out' \
	           -e '$$occ_gpe0_binary_to_load=$(OBJDIR)/$(OCC_GPE0_IMAGE_NAME)/$(OCC_GPE0_IMAGE_NAME).out' \
	           -e '$$occ_gpe1_binary_to_load=$(OBJDIR)/$(OCC_GPE1_IMAGE_NAME)/$(OCC_GPE1_IMAGE_NAME).out' \
	           modelsetup.simics



#clean the obj directory
clean:
	rm -fr $(OBJDIR)
	rm -f $(TRACEPP_DIR)/tracepp

#make binary application images
$(OBJDIR)/$(OCC_405_IMAGE_NAME)/$(OCC_405_IMAGE_NAME).out:
	(cd $(SRCDIR)/$(OCC_405_IMAGE_NAME) && make)

$(OBJDIR)/$(OCC_GPE0_IMAGE_NAME)/$(OCC_GPE0_IMAGE_NAME).out:
	(cd $(SRCDIR)/$(OCC_GPE0_IMAGE_NAME) && make)

$(OBJDIR)/$(OCC_GPE1_IMAGE_NAME)/$(OCC_GPE1_IMAGE_NAME).out:
	(cd $(SRCDIR)/$(OCC_GPE1_IMAGE_NAME) && make)

$(TRACEPP_DIR)/tracepp:
	(cd $(TRACEPP_DIR) && make)

# collect all of the trace hash files for all OCC images into a single trexStringFile
.PHONY : tracehash
tracehash:
	mkdir -p $(BASE_OBJDIR)
	$(THASH) -c -d $(BASE_OBJDIR) -s $(BASE_OBJDIR)/combStringFile

