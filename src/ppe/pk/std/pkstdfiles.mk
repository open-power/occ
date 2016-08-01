# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/pk/std/pkstdfiles.mk $
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

#  @file pkstdfiles.mk
#
#  @brief mk for including std object files

##########################################################################
# Object Files
##########################################################################

STD-C-SOURCES = std_init.c std_irq_init.c
STD-S-SOURCES =

STD-TIMER-C-SOURCES = 
STD-TIMER-S-SOURCES = 

STD-THREAD-C-SOURCES = 
STD-THREAD-S-SOURCES = 

STD-ASYNC-C-SOURCES =
STD-ASYNC-S-SOURCES =

STD_OBJECTS += $(STD-C-SOURCES:.c=.o) $(STD-S-SOURCES:.S=.o)

