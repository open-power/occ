# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/Makefile $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2014,2020
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

ifndef BASE_BINDIR
BASE_BINDIR = $(abspath ../bin)
endif

OBJDIR = $(BASE_OBJDIR)$(SUB_OBJDIR)
SRCDIR = $(BASE_SRCDIR)$(SUB_SRCDIR)

ifndef PPETRACEPP_DIR
export PPETRACEPP_DIR = $(abspath ppe/ppetrace/ppetracepp)
endif

ifndef PPETOOLS_OBJDIR
export PPETOOLS_OBJDIR = $(abspath ../obj/ppetools)
endif

ifndef TRACEPP_DIR
export TRACEPP_DIR = $(abspath tools/tracepp)
endif

ifndef TOOLS_OBJDIR
export TOOLS_OBJDIR = $(abspath ../obj/tools)
endif

ifndef PARSER_DIR
export PARSER_DIR = $(abspath tools/ffdcparser)
endif

ifndef OCCUTIL_DIR
export OCCUTIL_DIR = $(abspath tools/occutil)
endif

ifndef PK_SRCDIR
export PK_SRCDIR = $(abspath ppe/pk)
endif

ifndef GPE1_BIN_IMAGE_PATH
export GPE1_BIN_IMAGE_PATH = $(BASE_BINDIR)
endif

THASH	= $(PPETRACEPP_DIR)/tracehash.pl

OCC_405_IMAGE_NAME = occ_405
OCC_GPE0_IMAGE_NAME = occ_gpe0
ifndef OPOCC_GPU_SUPPORT
OCC_GPE1_IMAGE_NAME = occ_gpe1
else
OCC_GPE1_IMAGE_NAME = gpu_gpe1
endif
OCC_BOOTLOADER_DIR_NAME = occBootLoader
OCC_BOOTLOADER_NAME = bootloader
IMAGE_HDR_SCRIPT = imageHdrScript

COMBINE_IMAGE_SUBDIRS = occBootLoader occ_405 occ_gpe0 occ_gpe1
COMBINEIMAGE = $(MAKE) combineImage -C $(dir)
IMAGEFILE = $(OBJDIR)/image.bin


ifndef OPOCC_GPU_SUPPORT
NEEDED_IMAGES = \
     $(OBJDIR)/$(OCC_BOOTLOADER_DIR_NAME)/$(IMAGE_HDR_SCRIPT) \
     $(OBJDIR)/$(OCC_405_IMAGE_NAME)/$(OCC_405_IMAGE_NAME).out \
     $(OBJDIR)/$(OCC_GPE0_IMAGE_NAME)/$(OCC_GPE0_IMAGE_NAME).out \
     $(OBJDIR)/$(OCC_GPE1_IMAGE_NAME)/$(OCC_GPE1_IMAGE_NAME).out \
     $(OBJDIR)/$(OCC_BOOTLOADER_DIR_NAME)/$(OCC_BOOTLOADER_NAME).out
else
NEEDED_IMAGES = \
     $(OBJDIR)/$(OCC_BOOTLOADER_DIR_NAME)/$(IMAGE_HDR_SCRIPT) \
     $(OBJDIR)/$(OCC_405_IMAGE_NAME)/$(OCC_405_IMAGE_NAME).out \
     $(OBJDIR)/$(OCC_GPE0_IMAGE_NAME)/$(OCC_GPE0_IMAGE_NAME).out \
     $(OBJDIR)/$(OCC_BOOTLOADER_DIR_NAME)/$(OCC_BOOTLOADER_NAME).out
endif

ifdef P10_HW_BUILD
export GCC-DEFS += -DP10_HW_BUILD=1
endif

.PHONY : all
all: tools $(NEEDED_IMAGES) combineImage tracehash

.PHONY: tools
tools: $(TOOLS_OBJDIR) ppetools ffdcparser wofparser occutil

.PHONY: ppetools
ppetools: $(PPETOOLS_OBJDIR)/ppetracepp $(PPETOOLS_OBJDIR)/ppe2fsp $(PPETOOLS_OBJDIR)/tracepp

$(PPETOOLS_OBJDIR)/ppetracepp: $(PPETOOLS_OBJDIR)
	g++ -O3 -w -g -I$(PPETRACEPP_DIR)/ $(PPETRACEPP_DIR)/ppetracepp.C -o $(PPETOOLS_OBJDIR)/ppetracepp

$(PPETOOLS_OBJDIR)/ppe2fsp: $(PPETOOLS_OBJDIR)
	gcc -w -g -I./ -I$(PPETRACEPP_DIR)/.. $(PPETRACEPP_DIR)/ppe2fsp.c $(PPETRACEPP_DIR)/ppe2fsp_cmd.c -o $(PPETOOLS_OBJDIR)/ppe2fsp

$(PPETOOLS_OBJDIR):
	mkdir -p $(PPETOOLS_OBJDIR)/

ffdcparser: $(TOOLS_OBJDIR) $(PARSER_DIR)/ffdcparser.c $(PARSER_DIR)/parser_common.h
	gcc $(PARSER_DIR)/ffdcparser.c -o $(TOOLS_OBJDIR)/ffdcparser

wofparser: $(TOOLS_OBJDIR) $(PARSER_DIR)/wofparser.c  $(PARSER_DIR)/parser_common.h
	gcc $(PARSER_DIR)/wofparser.c -o  $(TOOLS_OBJDIR)/wofparser

occutil: $(TOOLS_OBJDIR)/occutil
	(cd $(OCCUTIL_DIR) && make)

$(TOOLS_OBJDIR):
	mkdir -p $(TOOLS_OBJDIR)/

$(TOOLS_OBJDIR)/occutil:
	mkdir -p $(TOOLS_OBJDIR)/occutil

.PHONY : needed_images
needed_images: $(NEEDED_IMAGES)

ifndef OPOCC_GPU_SUPPORT
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
else
.PHONY : combineImage
combineImage: $(NEEDED_IMAGES)
	rm -rf $(IMAGEFILE)
	BASE_OBJDIR=$(BASE_OBJDIR) $(OBJDIR)/$(OCC_BOOTLOADER_DIR_NAME)/$(IMAGE_HDR_SCRIPT) \
    $(OBJDIR)/$(OCC_BOOTLOADER_DIR_NAME)/$(OCC_BOOTLOADER_NAME).bin \
	$(OBJDIR)/$(OCC_405_IMAGE_NAME)/$(OCC_405_IMAGE_NAME).bin \
	$(OBJDIR)/$(OCC_GPE0_IMAGE_NAME)/$(OCC_GPE0_IMAGE_NAME).bin \
	$(GPE1_BIN_IMAGE_PATH)/$(OCC_GPE1_IMAGE_NAME).bin \
    t2 \
	`md5sum $(OBJDIR)/$(OCC_405_IMAGE_NAME)/$(OCC_405_IMAGE_NAME).bin | cut -c 1-4`
endif

#clean the obj directory
.PHONY : clean
clean:
	rm -fr $(OBJDIR)

# Make binary application images
.PHONY : $(OBJDIR)/$(OCC_405_IMAGE_NAME)/$(OCC_405_IMAGE_NAME).out
$(OBJDIR)/$(OCC_405_IMAGE_NAME)/$(OCC_405_IMAGE_NAME).out:
	(cd $(SRCDIR)/$(OCC_405_IMAGE_NAME) && make)

.PHONY : $(OBJDIR)/$(OCC_GPE0_IMAGE_NAME)/$(OCC_GPE0_IMAGE_NAME).out
$(OBJDIR)/$(OCC_GPE0_IMAGE_NAME)/$(OCC_GPE0_IMAGE_NAME).out:
	(cd $(SRCDIR)/$(OCC_GPE0_IMAGE_NAME) && make)

.PHONY : $(OBJDIR)/$(OCC_GPE1_IMAGE_NAME)/$(OCC_GPE1_IMAGE_NAME).out
$(OBJDIR)/$(OCC_GPE1_IMAGE_NAME)/$(OCC_GPE1_IMAGE_NAME).out:
	(cd $(SRCDIR)/$(OCC_GPE1_IMAGE_NAME) && make)

.PHONY : $(OBJDIR)/$(OCC_BOOTLOADER_DIR_NAME)/$(OCC_BOOTLOADER_NAME).out
$(OBJDIR)/$(OCC_BOOTLOADER_DIR_NAME)/$(OCC_BOOTLOADER_NAME).out $(OBJDIR)/$(OCC_BOOTLOADER_DIR_NAME)/$(IMAGE_HDR_SCRIPT):
	(cd $(SRCDIR)/$(OCC_BOOTLOADER_DIR_NAME) && make)

$(PPETOOLS_OBJDIR)/tracepp:
	(cd $(TRACEPP_DIR) && make)

# collect all of the trace hash files for all OCC images into a single trexStringFile
.PHONY : tracehash
tracehash:
	mkdir -p $(BASE_OBJDIR)
	$(THASH) -c -d $(BASE_OBJDIR) -s $(BASE_OBJDIR)/occStringFile

