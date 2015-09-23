# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/tools/image/Makefile $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2015
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
############################################################################

# Makefile for image tools
# works on X86 Linux hosts.

# Make targets:

# all          : 
#
# utilities    : Build utility programs and procedures
#
# clean        : Removes the bin/ directory and all symbolic links
#

############################################################################


ifeq ($(CTEPATH),)
$(warning CTEPATH not defined; defaulting to awd)
CTEPATH = /afs/awd/projects/cte
endif


# Are we setup for eCMD, if so let's get our eCMD Release from there
ifneq ($(strip $(ECMD_RELEASE)),)
  ECMD_RELEASE := $(shell ecmdVersion full)
  # Make sure we got a valid version back
  ifeq ($(findstring ver,$(ECMD_RELEASE)),)
    ECMD_RELEASE := rel
  endif
else
  # If not setup for eCMD, default to rel
  ECMD_RELEASE := rel
endif


# Ok, now set our eCMD Path, if not set already
ifeq ($(strip $(ECMD_PATH)),)
  ECMD_PATH   := ${CTEPATH}/tools/ecmd/${ECMD_RELEASE}/
endif

ifeq ($(strip $(ECMD_PLUGIN)),cro)
# Cronus plugin specific setup
  CRONUS_PATH := $(shell echo ${ECMD_EXE} | sed -n 's|\([a-zA-Z0-9]*\)\(_*\)\([a-zA-Z0-9]*\)_x86\.exe|prcd_d|p')
  ifeq ($(strip $(CRONUS_PATH)),)
    $(error "Error determining CRONUS_PATH from env!")
  endif
endif

# We need common up-to-date headers for FAPI - currently using these.
FAPI = $(ECMD_PATH)ext/fapi

#  Locations of required headers.
INCLUDES += -I. -I../../ -I../../utils 
INCLUDES += -I ../../sbe/image/
INCLUDES += -I ../../sbe/sbefw/
INCLUDES += -I ../../sbe/plat/include
INCLUDES += -I ../../hwpf/plat/include/
INCLUDES += -I ../../pk/ppe42/
INCLUDES += -I ../../pk/kernel/
INCLUDES += -I ../../pk/std/
INCLUDES += -I ../../pk/trace/
INCLUDES += -I ../../tools/ppetracepp/
INCLUDES += -I ../../importtemp/fapi2/include/

INCLUDES +=	-I$(CRONUS_PATH)
INCLUDES +=	-I$(ECMD_PATH)/capi 
INCLUDES +=	-I$(FAPI)/capi

# Under Linux the scheme is to use a common compiler to create procedures.
# However, the common compiler can be VERY slow, so if the system compiler is
# also 4.1.2 we're using that one instead.  Also, the Linux FAPI libraries we
# link with are 32-bit only so we need to force 32-bit mode.

ifeq ($(wildcard /etc/ldap.conf), )
  GSACELL     = ausgsa
else
  GSACELL     = $(shell cat /etc/ldap.conf | grep "host " | \
        cut -d" " -f2 | cut -d. -f1)
endif

GCC-RELEASE = 4.8.2
GCC-VERSION = $(shell gcc -v 2>&1 | grep "$(GCC-RELEASE)")

ifeq ($(GCC-VERSION),)
$(error wrong compiler version. Use $(GCC-RELEASE) compiler. Try: "scl enable devtoolset-2 bash")
else
CC   = gcc
CXX  = g++
endif

#UTILITIES-SOURCES += ../../sbe/image/sbe_xip_image.c 
UTILITIES-SOURCES = sbe_xip_tool.c sbe_default_tool.c

UTILITIES = sbe_xip_tool sbe_default_tool

#  Utility targets
UTILITIES-OBJc         = $(patsubst %.c,bin/%.o,$(UTILITIES-SOURCES))
UTILITIES-OBJECTS      += $(patsubst %.C,bin/%.o,$(UTILITIES-OBJc))
UTILITIES-DEPENDENCIES = $(patsubst %.o,%.d,$(UTILITIES-OBJECTS))
UTILITIES-EXECUTABLES  = $(patsubst %,bin/%,$(UTILITIES))


.PHONY : utilities
utilities: $(UTILITIES-EXECUTABLES)

bin/%.o: %.c
	$(CXX) -std=c++11 $(INCLUDES) $(CXXFLAGS) -DDEBUG_SBE_XIP_IMAGE=1 -DFAPI2_NO_FFDC -c -o $@ $<

bin/sbe_xip_image.o: ../../sbe/image/sbe_xip_image.c
	$(CXX) -std=c++11 $(INCLUDES) $(CXXFLAGS) -DDEBUG_SBE_XIP_IMAGE=1 -DFAPI2_NO_FFDC -c -o $@ $<

bin/sbe_xip_tool: bin/sbe_xip_image.o bin/p9_ring_identification.o bin/sbe_xip_tool.o
	$(CXX) $(CXXFLAGS) ${INCLUDES} -o $@ $^
	ln -sf bin/sbe_xip_tool sbe_xip_tool

bin/sbe_default_tool: bin/sbe_xip_image.o bin/sbe_default_tool.o 
	$(CXX) $(CXXFLAGS) ${INCLUDES} -o $@ $^
	ln -sf bin/sbe_default_tool sbe_default_tool

clean:
	rm sbe_xip_tool sbe_default_tool
	rm -rf bin
	mkdir -p bin