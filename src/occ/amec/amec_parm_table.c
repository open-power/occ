/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_parm_table.c $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h> //STATIC_ASSERT macro
#include <amec_parm.h>
#include <amec_sys.h>
#include <proc_pstate.h> //global pstate table parameter
#include <amec_wof.h>    // externs for WOF parameters
#include <pstates.h>     // for global pstate table

//*************************************************************************
// Externs
//*************************************************************************

extern int32_t g_amec_eff_vlow;
extern int32_t g_amec_eff_vhigh;
extern uint32_t g_amec_wof_iout;
//*************************************************************************
// Macros
//*************************************************************************

// Macros for parameters based on simple variables (read-write, not vectors)
/**
 * Create or update a parameter
 *
 * i_name: Name of parameter.  If this name is the name of an existing
 * parameter, then the existing parameter fields will be updated
 * i_value: A pointer to the bytes representing the value of the parameter
 * i_length: The length in bytes of the parameter value
 * n: The number of elements if this parameter is a vector, otherwise = 1.
 */
#define AMEC_PARM_UINT8(i_num, i_name, i_value)             \
    [i_num] = {i_name,(void*)i_value,1,1,AMEC_PARM_TYPE_UINT8,0}
#define AMEC_PARM_UINT16(i_num, i_name, i_value)            \
    [i_num] = {i_name,(void*)i_value,2,1,AMEC_PARM_TYPE_UINT16,0}
#define AMEC_PARM_UINT32(i_num, i_name, i_value)            \
    [i_num] = {i_name,(void*)i_value,4,1,AMEC_PARM_TYPE_UINT32,0}
#define AMEC_PARM_UINT64(i_num, i_name, i_value)            \
    [i_num] = {i_name,(void*)i_value,8,1,AMEC_PARM_TYPE_UINT64,0}
#define AMEC_PARM_INT8(i_num, i_name, i_value)          \
    [i_num] = {i_name,(void*)i_value,1,1,AMEC_PARM_TYPE_INT8,0}
#define AMEC_PARM_INT16(i_num, i_name, i_value)             \
    [i_num] = {i_name,(void*)i_value,2,1,AMEC_PARM_TYPE_INT16,0}
#define AMEC_PARM_INT32(i_num, i_name, i_value)             \
    [i_num] = {i_name,(void*)i_value,4,1,AMEC_PARM_TYPE_INT32,0}
#define AMEC_PARM_INT64(i_num, i_name, i_value)         \
    [i_num] = {i_name,(void*)i_value,8,1,AMEC_PARM_TYPE_IN64,0}
#define AMEC_PARM_STRING(i_num, i_name, i_value, i_length)      \
    [i_num] = {i_name,(void*)i_value,i_length,1,AMEC_PARM_TYPE_STRING,0}
#define AMEC_PARM_RAW(i_num, i_name, i_value, i_length)         \
    [i_num] = {i_name,(void*)i_value,i_length,1,AMEC_PARM_TYPE_RAW,0}

//Use these macros when the parameter is an array of values.
#define AMEC_PARM_UINT8_ARRAY(i_num, i_name, i_value, n)        \
    [i_num] = {i_name,(void*)i_value,1,n,AMEC_PARM_TYPE_UINT8,0}
#define AMEC_PARM_UINT16_ARRAY(i_num, i_name, i_value, n)       \
    [i_num] = {i_name,(void*)i_value,2,n,AMEC_PARM_TYPE_UINT16,0}
#define AMEC_PARM_UINT32_ARRAY(i_num, i_name, i_value, n)       \
    [i_num] = {i_name,(void*)i_value,4,n,AMEC_PARM_TYPE_UINT32,0}
#define AMEC_PARM_UINT64_ARRAY(i_num, i_name, i_value, n)       \
    [i_num] = {i_name,(void*)i_value,8,n,AMEC_PARM_TYPE_UINT64,0}
#define AMEC_PARM_INT8_ARRAY(i_num, i_name, i_value, n)     \
    [i_num] = {i_name,(void*)i_value,1,n,AMEC_PARM_TYPE_INT8,0}
#define AMEC_PARM_INT16_ARRAY(i_num, i_name, i_value, n)        \
    [i_num] = {i_name,(void*)i_value,2,n,AMEC_PARM_TYPE_INT16,0}
#define AMEC_PARM_INT32_ARRAY(i_num, i_name, i_value, n)        \
    [i_num] = {i_name,(void*)i_value,4,n,AMEC_PARM_TYPE_INT32,0}
#define AMEC_PARM_INT64_ARRAY(i_num, i_name, i_value, n)    \
    [i_num] = {i_name,(void*)i_value,8,n,AMEC_PARM_TYPE_IN64,0}
#define AMEC_PARM_STRING_ARRAY(i_num, i_name, i_value, i_length, n) \
    [i_num] = {i_name,(void*)i_value,i_length,n,AMEC_PARM_TYPE_STRING,0}
#define AMEC_PARM_RAW_ARRAY(i_num, i_name, i_value, i_length, n)    \
    [i_num] = {i_name,(void*)i_value,i_length,n,AMEC_PARM_TYPE_RAW,0}

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Structures
//*************************************************************************

extern amec_sys_t g_amec_sys;
 
//*************************************************************************
// Globals
//*************************************************************************

// This is the list of all parameters seen by Amester
//
// Note: The parameters must be in the same order as in AMEC_PARM_ENUM
// in amec_parm.h
//
// Future optimization: This table could be placed in main memory, not
// the SRAM tank, since slow access to it is OK.
amec_parm_t g_amec_parm_list[] = {
    // System fmin and fmax
    AMEC_PARM_UINT16(PARM_SYS_FMAX,"sys_fmax",&g_amec_sys.sys.fmax),
    AMEC_PARM_UINT16(PARM_SYS_FMIN,"sys_fmin",&g_amec_sys.sys.fmin),
    AMEC_PARM_RAW(PARM_DCOM_POBID,"dcom_pobid",&G_pob_id,sizeof(pob_id_t)),
    // Global Pstate table
    AMEC_PARM_RAW(PARM_GPST,"gpst",&G_global_pstate_table,sizeof(GlobalPstateTable)),
    // MHz per pstate
    AMEC_PARM_UINT32(PARM_PSTATE_MHZ,"pstate_mhz",&G_mhz_per_pstate),
    // frequency reason code per-core
    AMEC_PARM_UINT32_ARRAY(PARM_FREQ_REASON,"freq_reason",g_amec_sys.proc[0].parm_f_reason,MAX_NUM_CORES),
    // frequency override speed in MHz per-core
    AMEC_PARM_UINT16_ARRAY(PARM_FREQ_OR,"freq_or",g_amec_sys.proc[0].parm_f_override,MAX_NUM_CORES),
    // frequency override enable bit (1=active)
    AMEC_PARM_UINT8(PARM_FREQ_OR_EN,"freq_or_en",&g_amec_sys.proc[0].parm_f_override_enable),

    // Thermal controller parameters
    AMEC_PARM_UINT16(PARM_SYS_THRM_SP,"sys_thrm_sp",&g_amec_sys.thermalproc.setpoint),
    AMEC_PARM_UINT16(PARM_SYS_THRM_GAIN,"sys_thrm_gain",&g_amec_sys.thermalproc.Pgain),
    AMEC_PARM_UINT16(PARM_SYS_THRM_RES,"sys_thrm_res",&g_amec_sys.thermalproc.total_res),
    AMEC_PARM_UINT16(PARM_SYS_THRM_SPEED,"sys_thrm_speed",&g_amec_sys.thermalproc.speed_request),
    AMEC_PARM_UINT16(PARM_SYS_THRM_FREQ,"sys_thrm_freq",&g_amec_sys.thermalproc.freq_request),

    // Partition related parameters
    AMEC_PARM_UINT16(PARM_SOFT_FMIN,"part_soft_fmin",&g_amec_sys.part_config.part_list[0].soft_fmin),
    AMEC_PARM_UINT16(PARM_SOFT_FMAX,"part_soft_fmax",&g_amec_sys.part_config.part_list[0].soft_fmax),
    AMEC_PARM_RAW(PARM_TOD,"apss_tod",&G_dcom_slv_inbox_doorbell_rx.tod,8),

    // WOF: Workload-Optimized Frequency
    AMEC_PARM_UINT16(PARM_WOF_VDD_EFF,"wof_vdd_eff",&g_amec_sys.wof.vdd_eff),
    AMEC_PARM_UINT16(PARM_WOF_CUR_OUT,"wof_cur_out",&g_amec_sys.wof.cur_out),
    AMEC_PARM_UINT16(PARM_WOF_LOADLINE,"wof_loadline",&g_amec_sys.wof.loadline),
    AMEC_PARM_UINT16(PARM_WOF_V_CHIP,"wof_v_chip",&g_amec_sys.wof.v_chip),
    AMEC_PARM_UINT8(PARM_WOF_IDDQ_I,"wof_iddq_i",&g_amec_sys.wof.iddq_i),
    AMEC_PARM_UINT8_ARRAY(PARM_WOF_IDDQ_I_CORE,"wof_iddq_ic",g_amec_sys.wof.iddq_i_core,MAX_NUM_CORES),
    AMEC_PARM_UINT16(PARM_WOF_IDDQ85C,"wof_iddq85c",&g_amec_sys.wof.iddq85c),
    AMEC_PARM_UINT16_ARRAY(PARM_WOF_IDDQ85C_CORE,"wof_iddq85cc",g_amec_sys.wof.iddq85c_core,MAX_NUM_CORES),
    AMEC_PARM_UINT16(PARM_WOF_IDDQ,"wof_iddq",&g_amec_sys.wof.iddq),
    AMEC_PARM_UINT16(PARM_WOF_IDDQ_CHIP,"wof_iddq_chip",&g_amec_sys.wof.iddq_chip),
    AMEC_PARM_UINT16_ARRAY(PARM_WOF_IDDQ_CORE,"wof_iddq_core",g_amec_sys.wof.iddq_core,MAX_NUM_CORES),
    AMEC_PARM_UINT16(PARM_WOF_AC,"wof_ac",&g_amec_sys.wof.ac),
    AMEC_PARM_UINT32_ARRAY(PARM_WOF_CEFF_TDP,"wof_ceff_tdp",
                           &g_amec_sys.wof.ceff_tdp,MAX_NUM_CORES+1),
    AMEC_PARM_UINT32(PARM_WOF_CEFF,"wof_ceff",&g_amec_sys.wof.ceff),
    AMEC_PARM_UINT32(PARM_WOF_CEFF_OLD,"wof_ceff_old",&g_amec_sys.wof.ceff_old),
    AMEC_PARM_UINT16(PARM_WOF_CEFF_VOLT,"wof_ceff_volt",
                     &g_amec_sys.wof.ceff_volt),
    AMEC_PARM_UINT16(PARM_WOF_CEFF_RATIO,"wof_ceff_ratio",&g_amec_sys.wof.ceff_ratio),
    AMEC_PARM_INT16(PARM_WOF_F_UPLIFT,"wof_f_uplift",&g_amec_sys.wof.f_uplift),
    AMEC_PARM_UINT16(PARM_WOF_F_VOTE,"wof_f_vote",&g_amec_sys.wof.f_vote),
    AMEC_PARM_UINT16(PARM_WOF_VOTE_VREG,"wof_vote_vreg",&g_amec_sys.wof.vote_vreg),
    AMEC_PARM_UINT16(PARM_WOF_VOTE_VCHIP,"wof_vote_vchip",&g_amec_sys.wof.vote_vchip),
    AMEC_PARM_UINT8(PARM_WOF_ERROR,"wof_error",&g_amec_sys.wof.error),
    AMEC_PARM_UINT8(PARM_WOF_STATE,"wof_state",&g_amec_sys.wof.state),
    AMEC_PARM_UINT8(PARM_WOF_ENABLE,"wof_enable",&g_amec_sys.wof.enable_parm),
    AMEC_PARM_UINT8(PARM_WOF_CORES_ON,"wof_cores_on",&g_amec_sys.wof.cores_on),
    AMEC_PARM_RAW(PARM_WOF_WAKE_MASK,"wof_wake_mask",&g_amec_sys.wof.wake_up_mask_save,sizeof(uint64_t)),
    AMEC_PARM_RAW(PARM_WOF_PM_STATE,"wof_pm_state",&g_amec_sys.wof.pm_state,sizeof(uint8_t)*MAX_NUM_CORES),
    AMEC_PARM_UINT8(PARM_WOF_PSTATE_READY,"wof_ps_ready",&g_amec_wof_pstate_table_ready),
    AMEC_PARM_INT32(PARM_WOF_EFFVLOW,"wof_effvlow",&g_amec_eff_vlow),
    AMEC_PARM_INT32(PARM_WOF_EFFVHIGH,"wof_effvhigh",&g_amec_eff_vhigh),
    AMEC_PARM_UINT32(PARM_WOF_IOUT,"wof_iout",&g_amec_wof_iout),
    AMEC_PARM_UINT8(PARM_WOF_GOOD_CORES,"wof_good_cores",
                    &G_wof_max_cores_per_chip),
    AMEC_PARM_UINT16(PARM_WOF_LEAK_OVERHEAD,"wof_leak_ovrhd",
                     &g_amec_wof_leak_overhead),
    AMEC_PARM_UINT8(PARM_WOF_VDD_ITER,"wof_vdd_iter",
                    &g_amec_sys.wof.vdd_iter),
    AMEC_PARM_UINT32(PARM_WOF_VDD_T1,"wof_vdd_t1",&g_amec_sys.wof.vdd_t1),
    AMEC_PARM_UINT16(PARM_WOF_VDD_PIN,"wof_vdd_pin",&g_amec_sys.wof.vdd_pin),
    AMEC_PARM_UINT16(PARM_WOF_VDD_VSET,"wof_vdd_vset",&g_amec_sys.wof.vdd_vset),
    AMEC_PARM_INT32(PARM_WOF_VDD_VHI,"wof_vdd_vhi",&g_amec_sys.wof.vdd_vhi),
    AMEC_PARM_INT32(PARM_WOF_VDD_VLO,"wof_vdd_vlo",&g_amec_sys.wof.vdd_vlo),
    AMEC_PARM_UINT16_ARRAY(PARM_WOF_VDD_IOUTI,"wof_vdd_iouti",
                           g_amec_sys.wof.vdd_iouti,AMEC_WOF_VDD_ITER_BUFF),
    AMEC_PARM_UINT16_ARRAY(PARM_WOF_VDD_VOUTI,"wof_vdd_vouti",
                           g_amec_sys.wof.vdd_vouti,AMEC_WOF_VDD_ITER_BUFF),
    AMEC_PARM_UINT16_ARRAY(PARM_WOF_VDD_EFFI,"wof_vdd_effi",
                           g_amec_sys.wof.vdd_effi,AMEC_WOF_VDD_ITER_BUFF),
    AMEC_PARM_INT32_ARRAY(PARM_WOF_VDD_EFFHII,"wof_vdd_effhii",
                           g_amec_sys.wof.vdd_effhii,AMEC_WOF_VDD_ITER_BUFF),
    AMEC_PARM_INT32_ARRAY(PARM_WOF_VDD_EFFLOI,"wof_vdd_effloi",
                           g_amec_sys.wof.vdd_effloi,AMEC_WOF_VDD_ITER_BUFF),
    AMEC_PARM_UINT32(PARM_WOF_VDD_T2,"wof_vdd_t2",&g_amec_sys.wof.vdd_t2),
    AMEC_PARM_UINT16_ARRAY(PARM_WOF_EFF_TABLE,"wof_vdd_efftab",
                           G_amec_wof_vrm_eff_table,
                           AMEC_WOF_VRM_EFF_TBL_ROWS*AMEC_WOF_VRM_EFF_TBL_CLMS),
    AMEC_PARM_UINT16_ARRAY(PARM_WOF_UPLIFT_TABLE,"wof_uplifttab",
                           G_amec_wof_uplift_table,
                           AMEC_WOF_UPLIFT_TBL_ROWS*AMEC_WOF_UPLIFT_TBL_CLMS),
    AMEC_PARM_RAW(PARM_WOF_IDDQ_TABLE,"wof_iddq_tab",
                  &G_sysConfigData.iddq_table,sizeof(IddqTable)),
    AMEC_PARM_RAW(PARM_SYS_CONFIG,"sys_config",
                  &G_sysConfigData,sizeof(occSysConfigData_t)),
};

//Throw a compiler error when the enum and array are not both updated
STATIC_ASSERT((AMEC_PARM_NUMBER_OF_PARAMETERS != (sizeof(g_amec_parm_list)/sizeof(amec_parm_t))));

void amec_parm_preread(AMEC_PARM_GUID i_parm_guid)
{
    switch (i_parm_guid)
    {
    default:
        break;
    }
}

void amec_parm_postwrite(AMEC_PARM_GUID i_parm_guid)
{
    switch (i_parm_guid)
    {
    default:
        break;
    }
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
