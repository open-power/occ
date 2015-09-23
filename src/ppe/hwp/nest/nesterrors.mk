# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/hwp/nest/nesterrors.mk $
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
#  @file nesterrors.mk
#
#  @brief mk for including error files
#
#  @page ChangeLogs Change Logs
#  @section nesterrors.mk
#
##########################################################################
# Error Files
##########################################################################

NEST_CURR_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

# This variable name must not change
ERROR_XML_FILES += $(NEST_CURR_DIR)/p9_sbe_fabricinit_errors.xml


