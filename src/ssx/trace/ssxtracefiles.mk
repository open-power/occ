# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ssx/trace/ssxtracefiles.mk $
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
#  @file ssxtracefiles.mk
#
#  @brief mk for including SSX trace object files
#
#  @page ChangeLogs Change Logs
#  @section ssxtracefiles.mk
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
SSXTRACE-C-SOURCES = ssx_trace_core.c ssx_trace_big.c ssx_trace_binary.c
				   
SSXTRACE-S-SOURCES = 

SSXTRACE-TIMER-C-SOURCES = 
SSXTRACE-TIMER-S-SOURCES = 

SSXTRACE-THREAD-C-SOURCES += 
SSXTRACE-THREAD-S-SOURCES += 


SSXTRACE_OBJECTS += $(SSXTRACE-C-SOURCES:.c=.o) $(SSXTRACE-S-SOURCES:.S=.o)



