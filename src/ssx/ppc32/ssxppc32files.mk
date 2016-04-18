# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ssx/ppc32/ssxppc32files.mk $
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
#  @file ssxppc32files.mk
#
#  @brief mk for including ppc32 object files
#
#  @page ChangeLogs Change Logs
#  @section ssxppc32files.mk
#  @verbatim
#
#
# Change Log ******************************************************************
# Flag     Defect/Feature  User        Date         Description
# ------   --------------  ----------  ------------ -----------
# @pb00E                   pbavari     03/11/2012   Makefile ODE support
#
# @endverbatim
#

##########################################################################
# Object Files
##########################################################################
PPC32-C-SOURCES = ppc32_gcc.c
PPC32-S-SOURCES = div64.S savegpr.S

PPC32_OBJECTS = $(PPC32-C-SOURCES:.c=.o) $(PPC32-S-SOURCES:.S=.o)



