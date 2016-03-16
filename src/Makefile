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
OCC_BOOTLOADER_DIR_NAME = occBootLoader
OCC_BOOTLOADER_NAME = bootloader
IMAGE_HDR_SCRIPT = imageHdrScript

COMBINE_IMAGE_SUBDIRS = occBootLoader occ_405 occ_gpe0 occ_gpe1
COMBINEIMAGE = $(MAKE) combineImage -C $(dir)
IMAGEFILE = $(OBJDIR)/image.bin

NEEDED_IMAGES = \
     $(TRACEPP_DIR)/tracepp \
     $(OBJDIR)/$(OCC_BOOTLOADER_DIR_NAME)/$(IMAGE_HDR_SCRIPT) \
     $(OBJDIR)/$(OCC_405_IMAGE_NAME)/$(OCC_405_IMAGE_NAME).out \
     $(OBJDIR)/$(OCC_GPE0_IMAGE_NAME)/$(OCC_GPE0_IMAGE_NAME).out \
     $(OBJDIR)/$(OCC_GPE1_IMAGE_NAME)/$(OCC_GPE1_IMAGE_NAME).out \
     $(OBJDIR)/$(OCC_BOOTLOADER_DIR_NAME)/$(OCC_BOOTLOADER_NAME).out \

.PHONY : all
all: $(NEEDED_IMAGES) combineImage tracehash

.PHONY : needed_images
needed_images: $(NEEDED_IMAGES)

.PHONY : combineImage
combineImage: $(NEEDED_IMAGES)
	rm -rf $(IMAGEFILE)
	BASE_OBJDIR=$(BASE_OBJDIR) $(OBJDIR)/$(OCC_BOOTLOADER_DIR_NAME)/$(IMAGE_HDR_SCRIPT) \
    $(OBJDIR)/$(OCC_BOOTLOADER_DIR_NAME)/$(OCC_BOOTLOADER_NAME).bin \
	$(OBJDIR)/$(OCC_405_IMAGE_NAME)/$(OCC_405_IMAGE_NAME).bin \
	$(OBJDIR)/$(OCC_GPE0_IMAGE_NAME)/$(OCC_GPE0_IMAGE_NAME).bin \
	$(OBJDIR)/$(OCC_GPE1_IMAGE_NAME)/$(OCC_GPE1_IMAGE_NAME).bin \
    t2 \
	`md5sum $(OBJDIR)/$(OCC_405_IMAGE_NAME)/$(OCC_405_IMAGE_NAME).bin | cut -c 1-4`

#clean the obj directory
.PHONY : clean
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

$(OBJDIR)/$(OCC_BOOTLOADER_DIR_NAME)/$(OCC_BOOTLOADER_NAME).out $(OBJDIR)/$(OCC_BOOTLOADER_DIR_NAME)/$(IMAGE_HDR_SCRIPT):
	(cd $(SRCDIR)/$(OCC_BOOTLOADER_DIR_NAME) && make)

$(TRACEPP_DIR)/tracepp:
	(cd $(TRACEPP_DIR) && make)

# collect all of the trace hash files for all OCC images into a single trexStringFile
.PHONY : tracehash
tracehash:
	mkdir -p $(BASE_OBJDIR)
	$(THASH) -c -d $(BASE_OBJDIR) -s $(BASE_OBJDIR)/combStringFile

