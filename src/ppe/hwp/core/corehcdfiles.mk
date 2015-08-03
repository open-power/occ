# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/hwp/core/corehcdfiles.mk $
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
#  @file corehcdfiles.mk
#
#  @brief mk for including core hcode object files
#
#  @page ChangeLogs Change Logs
#  @section corehcdfiles.mk
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

CACHE-CPP-SOURCES += p9_hcd_core_arrayinit.C
CACHE-CPP-SOURCES += p9_hcd_core_chiplet_init.C
CACHE-CPP-SOURCES += p9_hcd_core_chiplet_reset.C
CACHE-CPP-SOURCES += p9_hcd_core_gptr_time_initf.C
CACHE-CPP-SOURCES += p9_hcd_core_initf.C
CACHE-CPP-SOURCES += p9_hcd_core_occ_runtime_scom.C
CACHE-CPP-SOURCES += p9_hcd_core_pcb_arb.C
CACHE-CPP-SOURCES += p9_hcd_core_poweron.C
CACHE-CPP-SOURCES += p9_hcd_core_ras_runtime_scom.C
CACHE-CPP-SOURCES += p9_hcd_core_repair_initf.C
CACHE-CPP-SOURCES += p9_hcd_core_runinit.C
CACHE-CPP-SOURCES += p9_hcd_core_scomcust.C
CACHE-CPP-SOURCES += p9_hcd_core_scominit.C
CACHE-CPP-SOURCES += p9_hcd_core_startclocks.C

CACHE-C-SOURCES   +=
CACHE-S-SOURCES   +=

CACHE_OBJECTS     += $(CACHE-CPP-SOURCES:.C=.o)
CACHE_OBJECTS     += $(CACHE-C-SOURCES:.c=.o)
CACHE_OBJECTS     += $(CACHE-S-SOURCES:.S=.o)

