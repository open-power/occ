# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/include/ppe_compiler.mk $
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
#  Make header to set up PPE Compiler
#
ifndef CC_VERSION
export CC_VERSION = 4.8.3
endif

ifndef CC_TARGET
export CC_TARGET = powerpc-buildroot-linux-gnu
endif

# GCC libraries
ifndef LIB_GCC_DIR
export LIB_GCC_DIR = $(CC_ROOT)/usr/lib/gcc/$(CC_TARGET)/$(CC_VERSION)
endif
LIB_DIRS += -L$(LIB_GCC_DIR)

# GCC libraries
ifndef LIB_GCC_BASE
export LIB_GCC_BASE = $(CC_ROOT)/usr/lib/gcc/$(CC_TARGET)
endif
LIB_DIRS += -L$(LIB_GCC_BASE)

# GCC libraries
ifndef LIB_BASE
export LIB_BASE = $(CC_ROOT)/usr/lib
endif
LIB_DIRS += -L$(LIB_BASE)

# Shared Object C and C++ libraries
ifndef LIB_TARGET
export LIB_TARGET = $(CC_ROOT)/usr/$(CC_TARGET)/lib
endif
#CLIBS += $(LIB_TARGET)/libstdc++.a
#LIB_DIRS += -L$(LIB_TARGET)

# Static C and C++ libraries
ifndef LIB_CLIB_DIR
export LIB_CLIB_DIR = $(CC_ROOT)/usr/$(CC_TARGET)/sysroot/lib
endif
LIB_DIRS += -L$(LIB_CLIB_DIR)

# Runtime Static C and C++ libraries
ifndef LIB_CULIB_DIR
export LIB_CULIB_DIR = $(CC_ROOT)/usr/$(CC_TARGET)/sysroot/usr/lib
endif
LIB_DIRS += -L$(LIB_CULIB_DIR)

GCCLIBS += $(LIB_CULIB_DIR)/libstdc++.a
GCCLIBS += $(LIB_CULIB_DIR)/libm.a
GCCLIBS += $(LIB_CULIB_DIR)/libc.a

ifdef ENABLE_UCLIB
GCCLIBS += $(LIB_CULIB_DIR)/uclibc_nonshared.a
GCCLIBS += $(LIB_CULIB_DIR)/crt1.o
GCCLIBS += $(LIB_CULIB_DIR)/crti.o
endif

GCCLIBS += $(LIB_GCC_DIR)/libgcc.a
GCCLIBS += $(LIB_GCC_DIR)/libgcc_eh.a
