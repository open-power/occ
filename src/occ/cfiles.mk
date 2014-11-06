#  @file cfiles.mk
#
#  @brief mk for c-object files for occ application
#
#  @page ChangeLogs Change Logs
#  @section cfiles.mk
#  @verbatim
#
#
# Change Log ******************************************************************
# Flag     Defect/Feature  User        Date         Description
# ------   --------------  ----------  ------------ -----------
# @pb00E                   pbavari     03/11/2012   Makefile ODE support
# @th00c                   thallet     04/23/2012   Centaur files
# @th022                   thallet     10/04/2012   Dcom & CnfgData additions
# @at009   859308          alvinwan    10/15/2012   Added tracepp support
# @ry002   862116          ronda       11/26/2012   Support thermal controller for processor
# @at010   859992          alvinwan    11/07/2012   Added oversubscription feature
# @gs004   883829          gjsilva     05/21/2013   Support for manufacturing commands
# @fk001   879727          fmkassem    04/16/2013   OCC powercap support.
# @jh005   894560          joshych     08/14/2013   Create call home data logs every 24 hours
# @gm010   901580          milesg      10/06/2013   Low Level FFDC support
# @gs012   903325          gjsilva     09/26/2013   Add health monitor files
# @gs014   903552          gjsilva     10/22/2013   Support for Amester parameter interface
#          905504          sbroyles    11/06/2013   Added homer.c
# @rt004   905638          tapiar      11/13/2013   Support for Tunable Parameters
# @gs019   908218          gjsilva     12/04/2013   Support cooling request architecture
# @fk004   907588          fmkassem    11/25/2013   Add support for snapshot buffer.
#          916980          sbroyles    03/10/2014   Added ffdc.c
# @gs027   918066          gjsilva     03/12/2014   Added amec_analytics.c
# @gm033   920448          milesg      03/26/2014   added scom.c
# @endverbatim
#

##########################################################################
# Object files
##########################################################################

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

occ_SFILES = cmdh/ll_ffdc.S

occ_cfiles = ${occ_CFILES:.c=.o}
occ_Sfiles = ${occ_SFILES:.S=.o}

