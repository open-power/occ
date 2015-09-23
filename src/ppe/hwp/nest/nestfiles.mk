# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/hwp/nest/nestfiles.mk $
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
#  @file nestfiles.mk
#
#  @brief mk for including nest object files
#
#  @page ChangeLogs Change Logs
#  @section nestfiles.mk
#  @verbatim
#
#  @endverbatim
#
##########################################################################
# Object Files
##########################################################################

NEST-CPP-SOURCES = p9_sbe_mcs_setup.C
NEST-CPP-SOURCES +=p9_sbe_scominit.C
NEST-CPP-SOURCES +=p9_sbe_fabricinit.C

NEST-C-SOURCES =
NEST-S-SOURCES =

NEST_OBJECTS += $(NEST-CPP-SOURCES:.C=.o)
NEST_OBJECTS += $(NEST-C-SOURCES:.c=.o)
NEST_OBJECTS += $(NEST-S-SOURCES:.S=.o)
