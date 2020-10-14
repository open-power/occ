# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/tools/cft/makefile $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2020
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
# Makefile for Converged Manufacturing Toolkit(CMT)

ifeq ($(strip $(BUILD_INFO)),)
  $(info No BUILD_INFO specified...)
  $(info Using: BUILD_INFO="-DOPLTK_VERSION=20190109 -DOPFTK_VERSION=20190110")
  BUILD_INFO="-DOPLTK_VERSION=20190109 -DOPFTK_VERSION=20190110"
endif

BUILD_HOST_NAME := $(shell hostname)
BUILD_HOST_IP   := $(shell hostname -I)
BUILD_OS        := $(shell lsb_release -sd)

# Cleanup multiple //'s in the path built above
OBJPATH                         := ../../../obj/ppetools/cft/
BINPATH                         := ../../../obj/ppetools
occtool_SOURCE                  := cftocctool.C cft_utils.C cft_tmgt.C cft_occ.C

CFLAGS          := ${CFLAGS} -g -I/usr/include/libxml2 -Icmd -Icapi -Imnfg -Ihwp -Ijson/single_include "-DBUILD_HOST_NAME=$(BUILD_HOST_NAME)" "-DBUILD_HOST_IP=$(BUILD_HOST_IP)" -DBUILD_OS=$(BUILD_OS) $(BUILD_INFO)

# Choose the eCMD Release to build against
# Are we setup for eCMD, if so let's get our eCMD Release from there
ifeq ($(strip $(ECMD_RELEASE)),)
 ifneq ($(strip $(ECMD_DLL_FILE)),)
   ECMD_RELEASE := $(shell ecmdVersion)
   # Make sure we got a valid version back, if not default to rel
   ifeq ($(findstring ver,$(ECMD_RELEASE)),)
     ECMD_RELEASE := rel
   endif
 else
 # If not setup for eCMD, default to rel
   ECMD_RELEASE := rel
 endif
endif

# Ok, now set our eCMD Path, if not set already
ifeq ($(strip $(ECMD_PATH)),)
  ECMD_PATH   := ${CTEPATH}/tools/ecmd/${ECMD_RELEASE}/
endif

######## Default things we need setup for every compile ########
# CC = the compiler
# CC_R = the reentrant compiler, only different for AIX
# CFLAGS = flags to pass to the compiler
# LD = the linker
# LD_R = the reentrant linker, only different for AIX
# LDFLAGS = flags to pass to the linker when linking exe's
# SLDFLAGS = flags to pass to the linker when linking shared libs
# AR = the archive creator
# DEFINES = -D defines to pass thru
#
CC := g++
# Assign default values, unless the user set them before calling the script
LD  := ${CC}
# Update CFLAGS to include ECMD headers
CFLAGS                      := ${CFLAGS} -Wall -m64 -I${ECMD_PATH}/capi -Iincludes
LINK_OBJS                   := ${ECMD_PATH}/x86_64/lib/ecmdClientCapi.a
occtool_LDFLAGS             := ${LDFLAGS} -m64 -L${ECMD_PATH}/x86_64/lib -lecmd -ldl

LDFLAGS                     := ${LDFLAGS} -m64 -L${ECMD_PATH}/x86_64/lib -ldl -lecmd -lxml2 -lz -l:libssh2.so.1 -lssl -lcrypto -ldl -lrt

all: dir cftocctool

clean: objclean

objclean:
	rm -rf ${OBJPATH}*

dir:
	@mkdir -p ${OBJPATH}

install:
	cp -v ${BINPATH}/cftocctool ${HOME}/bin/

# *****************************************************************************
#  Object Build Targets
# *****************************************************************************

occtool_SOURCE_OBJS  = $(basename $(occtool_SOURCE))
occtool_SOURCE_OBJS := $(addprefix ${OBJPATH}, $(occtool_SOURCE_OBJS))
occtool_SOURCE_OBJS := $(addsuffix .o, $(occtool_SOURCE_OBJS))

# *****************************************************************************
#  Compile code for the common C++ objects if their respective
#  code has been changed.  Or, compile everything if a header
#  file has changed.
# *****************************************************************************
$(occtool_SOURCE_OBJS): ${OBJPATH}%.o : %.C ${INCLUDES} ${INT_INCLUDES}
	$(CC) -c $(CFLAGS) $< -o $@ $(DEFINES)

# *****************************************************************************
#  Create the binary
# *****************************************************************************
cftocctool: ${occtool_SOURCE_OBJS}  ${LINK_OBJS}
	${LD} ${occtool_LDFLAGS} $^ -o ${BINPATH}/cftocctool ${LDLIBS}

