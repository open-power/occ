# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/baselib/baselibfiles.mk $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2019
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
PPEBASELIB-C-SOURCES = \
		ppe42_gcc.c\
		ppe42_scom.c\
		eabi.c\
		ppe42_math.c\
		ppe42_string.c
PPEBASELIB-S-SOURCES = \
		div64.S \
		div32.S

PPEBASELIB_OBJECTS=$(PPEBASELIB-C-SOURCES:.c=.o)
PPEBASELIB_OBJECTS += $(PPEBASELIB-S-SOURCES:.S=.o)

