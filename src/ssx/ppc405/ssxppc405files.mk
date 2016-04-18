# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ssx/ppc405/ssxppc405files.mk $
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
#  @file ssxppc405files.mk
#
#  @brief mk for including ppc405 object files
#
#  @page ChangeLogs Change Logs
#  @section ssxppc405files.mk
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
# Include Files
##########################################################################

			

##########################################################################
# Object Files
##########################################################################
PPC405-C-SOURCES = ppc405_core.c \
				   ppc405_lib_core.c \
				   ppc405_cache_core.c \
				   ppc405_init.c \
				   ppc405_irq_core.c \
				   ppc405_irq_init.c 
				   
PPC405-S-SOURCES =  ppc405_boot.S \
					ppc405_exceptions.S \
					ppc405_cache_init.S \
					ppc405_mmu_asm.S \
					ppc405_breakpoint.S

PPC405-TIMER-C-SOURCES = 
PPC405-TIMER-S-SOURCES = 

PPC405-THREAD-C-SOURCES += 
PPC405-THREAD-S-SOURCES += ppc405_thread_init.S

PPC405-MMU-C-SOURCES += ppc405_mmu.c
PPC405-MMU-S-SOURCES += 

PPC405_OBJECTS += $(PPC405-C-SOURCES:.c=.o) $(PPC405-S-SOURCES:.S=.o)



