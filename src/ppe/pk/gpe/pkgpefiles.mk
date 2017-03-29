# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/pk/gpe/pkgpefiles.mk $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2015,2017
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
#  @file pkgpefiles.mk
#
#  @brief mk for including gpe object files
#
#  @page ChangeLogs Change Logs
#  @section pkgpefiles.mk
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

GPE-C-SOURCES = gpe_init.c gpe_irq_init.c gpe_pba_cntl.c
GPE-S-SOURCES = gpe_scom_handler.S

GPE-TIMER-C-SOURCES = 
GPE-TIMER-S-SOURCES = 

GPE-THREAD-C-SOURCES = 
GPE-THREAD-S-SOURCES = 

GPE-ASYNC-C-SOURCES =
GPE-ASYNC-S-SOURCES =

GPE_OBJECTS = $(GPE-C-SOURCES:.c=.o) $(GPE-S-SOURCES:.S=.o)

