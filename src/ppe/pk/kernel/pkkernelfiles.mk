# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/pk/kernel/pkkernelfiles.mk $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2015,2016
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
#  @file pkkernelfiles.mk
#
#  @brief mk for including architecture independent pk object files
#
#  @page ChangeLogs Change Logs
#  @section pkkernelfiles.mk
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
# Include
##########################################################################


##########################################################################
# Object Files 
##########################################################################
PK-C-SOURCES = pk_core.c pk_init.c pk_stack_init.c pk_bh_core.c pk_debug_ptrs.c

PK-TIMER-C-SOURCES = pk_timer_core.c pk_timer_init.c

PK-THREAD-C-SOURCES = pk_thread_init.c pk_thread_core.c pk_thread_util.c \
	 pk_semaphore_init.c pk_semaphore_core.c

PK_TIMER_OBJECTS=$(PK-TIMER-C-SOURCES:.c=.o)
PK_THREAD_OBJECTS=$(PK-THREAD-C-SOURCES:.c=.o)
PK_OBJECTS = $(PK-C-SOURCES:.c=.o)

