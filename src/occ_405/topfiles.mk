# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occ_405/topfiles.mk $
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
TOP-C-SOURCES = aplt/appletManager.c \
                cmdh/cmdh_fsp.c \
                cmdh/cmdh_fsp_cmds.c \
                cmdh/cmdh_fsp_cmds_datacnfg.c \
                cmdh/cmdh_mnfg_intf.c \
                cmdh/cmdh_thread.c \
                cmdh/cmdh_tunable_parms.c \
                cmdh/snapshot.c \
                cmdh/ffdc.c \
                errl/errl.c \
                homer.c \
                main.c \
                occ_sys_config.c \
                occbuildname.c \
                pss/apss.c \
                reset.c \
                rtls/rtls.c \
                rtls/rtls_tables.c \
                scom.c \
                state.c \
                thread/threadSch.c \
                thread/thrm_thread.c \
                timer/timer.c \
                trac/trac_interface.c \

TOP-S-SOURCES = cmdh/ll_ffdc.S \


TOP_OBJECTS = $(TOP-C-SOURCES:.c=.o) $(TOP-S-SOURCES:.S=.o)
