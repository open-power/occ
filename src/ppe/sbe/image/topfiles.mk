# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/sbe/image/topfiles.mk $
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
TOP-C-SOURCES = sbe_loader.c
TOP-CPP-SOURCES = sbe_main.C
TOP-S-SOURCES = base_ppe_header.S

TOP-FIXED-HEADERS += $(IMAGE_SRCDIR)/proc_sbe_fixed_perv.H
TOP-FIXED-HEADERS += $(IMAGE_SRCDIR)/proc_sbe_fixed_proc_chip.H
TOP-FIXED-HEADERS += $(IMAGE_SRCDIR)/proc_sbe_fixed_core.H
TOP-FIXED-HEADERS += $(IMAGE_SRCDIR)/proc_sbe_fixed_ex.H
TOP-FIXED-HEADERS += $(IMAGE_SRCDIR)/proc_sbe_fixed_eq.H

TOP_OBJECTS = $(TOP-C-SOURCES:.c=.o) $(TOP-CPP-SOURCES:.C=.o) $(TOP-S-SOURCES:.S=.o)
