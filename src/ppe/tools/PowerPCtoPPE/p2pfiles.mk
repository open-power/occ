# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/tools/PowerPCtoPPE/p2pfiles.mk $
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
#  @file p2pfiles.mk
#
#  @brief mk for including P2P support library object files
#

##########################################################################
# Object Files
##########################################################################
P2P-S-SOURCES = ppe42_mulhw.S ppe42_mulhwu.S ppe42_mullw.S \
                ppe42_divw.S ppe42_divwu.S

P2P_OBJECTS = $(P2P-S-SOURCES:.S=.o)


