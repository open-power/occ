# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/pk/ppe/pkppefiles.mk $
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
#  @file pkppefiles.mk
#
#  @brief mk for including ppe object files
#
#  @page ChangeLogs Change Logs
#  @section pkppefiles.mk
#  @verbatim
#
#
# Change Log ******************************************************************
# Flag     Defect/Feature  User        Date         Description
# ------   --------------  ----------  ------------ -----------
#
# @endverbatim
#
##########################################################################
# Object Files
##########################################################################

PPE-C-SOURCES = ppe_init.c
PPE-S-SOURCES =

PPE-TIMER-C-SOURCES = 
PPE-TIMER-S-SOURCES = 

PPE-THREAD-C-SOURCES = 
PPE-THREAD-S-SOURCES = 

PPE-ASYNC-C-SOURCES =
PPE-ASYNC-S-SOURCES =

PPE_OBJECTS += $(PPE-C-SOURCES:.c=.o) $(PPE-S-SOURCES:.S=.o)

