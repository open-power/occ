# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/pk/ppe42/pkppe42files.mk $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2015,2019
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
#  @file  pkppe42files.mk
#
#  @brief mk for including ppe42 object files
#
#  @page ChangeLogs Change Logs
#  @section pkppe42files.mk
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
# Include Files
##########################################################################



##########################################################################
# Object Files
##########################################################################
PPE42-C-SOURCES = ppe42_core.c \
		  ppe42_init.c \
		  ppe42_irq_core.c\
		  ppe42_irq_init.c\

PPE42-S-SOURCES =  ppe42_boot.S \
		   ppe42_exceptions.S\
		   ppe42_timebase.S\

PPE42-TIMER-C-SOURCES =
PPE42-TIMER-S-SOURCES =

PPE42-THREAD-C-SOURCES =
PPE42-THREAD-S-SOURCES = ppe42_thread_init.S

PPE42_THREAD_OBJECTS= $(PPE42-THREAD-S-SOURCES:.S=.o)
PPE42_OBJECTS = $(PPE42-C-SOURCES:.c=.o)
PPE42_OBJECTS += $(PPE42-S-SOURCES:.S=.o)

