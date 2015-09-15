# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occ/cfiles.mk $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2011,2015
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
#
#  Description: mk for c-object files for occ application
#

occ_CFILES = \
                amec/amec_init.c \
                amec/amec_tasks.c \
                amec/amec_slave_smh.c \
                amec/amec_master_smh.c \
                amec/amec_sensors_core.c \
                amec/amec_sensors_centaur.c \
                amec/amec_sensors_power.c \
                amec/amec_sensors_fw.c \
                amec/amec_data.c \
                amec/amec_freq.c \
                amec/amec_amester.c \
                amec/amec_dps.c \
                amec/amec_part.c \
                amec/amec_perfcount.c \
                amec/amec_controller.c \
                amec/amec_oversub.c \
                amec/amec_pcap.c \
                amec/amec_health.c \
                amec/amec_parm.c \
                amec/amec_parm_table.c \
                amec/amec_analytics.c \
		amec/amec_wof.c \
                aplt/appletManager.c \
                cent/centaur_data.c \
                cent/centaur_control.c \
                cmdh/cmdh_fsp.c \
                cmdh/cmdh_thread.c \
                cmdh/cmdh_fsp_cmds.c \
                cmdh/cmdh_fsp_cmds_datacnfg.c \
                cmdh/cmdh_mnfg_intf.c \
                cmdh/cmdh_tunable_parms.c \
                cmdh/cmdh_snapshot.c \
                cmdh/ffdc.c \
                dcom/dcom.c \
                dcom/dcom_thread.c \
                dcom/dcomSlaveTx.c \
                dcom/dcomSlaveRx.c \
                dcom/dcomMasterTx.c \
                dcom/dcomMasterRx.c \
                errl/errl.c \
                proc/proc_data.c \
                proc/proc_data_control.c \
                proc/proc_pstate.c \
                pss/apss.c \
                pss/dpss.c \
                rtls/rtls_tables.c \
                rtls/rtls.c \
                sensor/sensor.c \
                sensor/sensor_table.c \
                thread/threadSch.c \
                thread/chom.c \
                thread/thrm_thread.c \
                timer/timer.c \
                trac/trac_interface.c \
                occ_sys_config.c \
                arl_test.c \
                state.c \
                reset.c \
                mode.c \
                main.c \
                scom.c \
                homer.c \
                occbuildname.c \
                common.c \

occ_CFILES += firdata/ecc.c
occ_CFILES += firdata/fir_data_collect.c
occ_CFILES += firdata/firData.c
occ_CFILES += firdata/fsi.c
occ_CFILES += firdata/lpc.c
occ_CFILES += firdata/native.c
occ_CFILES += firdata/nor_micron.c
occ_CFILES += firdata/pnor_util.c
occ_CFILES += firdata/scom_trgt.c
occ_CFILES += firdata/scom_util.c
occ_CFILES += firdata/sfc_ast2400.c

occ_SFILES = cmdh/ll_ffdc.S


occ_cfiles = ${occ_CFILES:.c=.o}
occ_Sfiles = ${occ_SFILES:.S=.o}

