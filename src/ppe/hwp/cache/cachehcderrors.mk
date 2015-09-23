# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/hwp/cache/cachehcderrors.mk $
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
#  @file cachehcderrors.mk
#
#  @brief mk for including cache error files
#
#  @page ChangeLogs Change Logs
#  @section cachehcderrors.mk
#
##########################################################################
# Error Files
##########################################################################

CACHE_CURR_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

# This variable name must not change
#ERROR_XML_FILES += $(CACHE_CURR_DIR)/p9_hcd_cache_arrayinit.C
#ERROR_XML_FILES += $(CACHE_CURR_DIR)/p9_hcd_cache_chiplet_init.C
#ERROR_XML_FILES += $(CACHE_CURR_DIR)/p9_hcd_cache_chiplet_reset.C
#ERROR_XML_FILES += $(CACHE_CURR_DIR)/p9_hcd_cache_dpll_setup.C
#ERROR_XML_FILES += $(CACHE_CURR_DIR)/p9_hcd_cache_gptr_time_initf.C
#ERROR_XML_FILES += $(CACHE_CURR_DIR)/p9_hcd_cache_initf.C
#ERROR_XML_FILES += $(CACHE_CURR_DIR)/p9_hcd_cache_occ_runtime_scom.C
#ERROR_XML_FILES += $(CACHE_CURR_DIR)/p9_hcd_cache_poweron.C
#ERROR_XML_FILES += $(CACHE_CURR_DIR)/p9_hcd_cache_ras_runtime_scom.C
#ERROR_XML_FILES += $(CACHE_CURR_DIR)/p9_hcd_cache_repair_initf.C
#ERROR_XML_FILES += $(CACHE_CURR_DIR)/p9_hcd_cache_runinit.C
#ERROR_XML_FILES += $(CACHE_CURR_DIR)/p9_hcd_cache_scomcust.C
#ERROR_XML_FILES += $(CACHE_CURR_DIR)/p9_hcd_cache_scominit.C
#ERROR_XML_FILES += $(CACHE_CURR_DIR)/p9_hcd_cache_startclocks.C
