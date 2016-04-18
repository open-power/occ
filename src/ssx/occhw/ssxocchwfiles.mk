# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ssx/occhw/ssxocchwfiles.mk $
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
#  @file ssxoccwhfiles.mk
#
#  @brief mk for including occwh object files
#
#  @page ChangeLogs Change Logs
#  @section ssxoccwhfiles.mk
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
OCCHW-C-SOURCES = occhw_init.c occhw_irq_init.c occhw_scom.c occhw_ocb.c occhw_pba.c \
	occhw_id.c occhw_centaur.c
OCCHW-S-SOURCES = occhw_cache.S

OCCHW-TIMER-C-SOURCES = 
OCCHW-TIMER-S-SOURCES = 

OCCHW-THREAD-C-SOURCES = 
OCCHW-THREAD-S-SOURCES = 

OCCHW-ASYNC-C-SOURCES = occhw_async.c occhw_async_ocb.c \
	occhw_async_pba.c occhw_async_gpe.c
OCCHW-ASYNC-S-SOURCES =

OCCHW_OBJECTS += $(OCCHW-C-SOURCES:.c=.o) $(OCCHW-S-SOURCES:.S=.o)

