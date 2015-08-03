# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/hwp/perv/pervfiles.mk $
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
#  @file pervfiles.mk
#
#  @brief mk for including perv object files
#
#  @page ChangeLogs Change Logs
#  @section pervfiles.mk
#  @verbatim
#
#  @endverbatim
#
##########################################################################
# Object Files
##########################################################################

PERV-CPP-SOURCES = p9_sbe_setup_evid.C
PERV-CPP-SOURCES +=p9_sbe_attr_setup.C
PERV-CPP-SOURCES +=p9_sbe_tp_chiplet_init1.C
PERV-CPP-SOURCES +=p9_sbe_check_master.C
PERV-CPP-SOURCES +=p9_sbe_tp_chiplet_init2.C
PERV-CPP-SOURCES +=p9_sbe_enable_seeprom.C
PERV-CPP-SOURCES +=p9_sbe_tp_chiplet_init3.C
PERV-CPP-SOURCES +=p9_sbe_lpc_init.C
PERV-CPP-SOURCES +=p9_sbe_tp_chiplet_reset.C
PERV-CPP-SOURCES +=p9_sbe_nest_arrayinit.C
PERV-CPP-SOURCES +=p9_sbe_tp_gptr_time_repr_initf.C
PERV-CPP-SOURCES +=p9_sbe_nest_chiplet_init.C
PERV-CPP-SOURCES +=p9_sbe_tp_initf.C
PERV-CPP-SOURCES +=p9_sbe_nest_chiplet_reset.C
PERV-CPP-SOURCES +=p9_sbe_tp_ld_image.C
PERV-CPP-SOURCES +=p9_sbe_nest_gptr_time_repr_initf.C
PERV-CPP-SOURCES +=p9_sbe_tp_switch_gears.C
PERV-CPP-SOURCES +=p9_sbe_nest_initf.C
PERV-CPP-SOURCES +=p9_select_boot_master.C
PERV-CPP-SOURCES +=p9_sbe_nest_startclocks.C
PERV-CPP-SOURCES +=p9_select_clock_mux.C
PERV-CPP-SOURCES +=p9_sbe_npll_initf.C
PERV-CPP-SOURCES +=p9_set_fsi_gp_shadow.C
PERV-CPP-SOURCES +=p9_sbe_npll_setup.C
PERV-CPP-SOURCES +=p9_setup_clock_term.C
PERV-CPP-SOURCES +=p9_sbe_select_ex.C
PERV-CPP-SOURCES +=p9_setup_sbe_config.C
PERV-CPP-SOURCES +=p9_sbe_tp_arrayinit.C
PERV-CPP-SOURCES +=p9_start_cbs.C

PERV-C-SOURCES =
PERV-S-SOURCES =

PERV_OBJECTS += $(PERV-CPP-SOURCES:.C=.o)
PERV_OBJECTS += $(PERV-C-SOURCES:.c=.o)
PERV_OBJECTS += $(PERV-S-SOURCES:.S=.o)
