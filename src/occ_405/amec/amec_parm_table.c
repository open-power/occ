/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_parm_table.c $                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2020                        */
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

//*************************************************************************
// Externs
//*************************************************************************

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
// Future optimization: This table could be placed in main memory, not
// the SRAM tank, since slow access to it is OK.
amec_parm_t g_amec_parm_list[] = {
    // System fmin and fmax
    AMEC_PARM_UINT16(PARM_SYS_FMAX,"sys_fmax",&g_amec_sys.sys.fmax),
    AMEC_PARM_UINT16(PARM_SYS_FMIN,"sys_fmin",&g_amec_sys.sys.fmin),

    // MHz per pstate
    AMEC_PARM_UINT32(PARM_PSTATE_MHZ,"pstate_mhz",&G_mhz_per_pstate),
    // frequency reason code per-core
    AMEC_PARM_UINT32_ARRAY(PARM_FREQ_REASON,"freq_reason",g_amec_sys.proc[0].parm_f_reason,MAX_NUM_CORES),
    // frequency override speed in MHz per-core
    AMEC_PARM_UINT16_ARRAY(PARM_FREQ_OR,"freq_or",g_amec_sys.proc[0].parm_f_override,MAX_NUM_CORES),
    // frequency override enable bit (1=active)
    AMEC_PARM_UINT8(PARM_FREQ_OR_EN,"freq_or_en",&g_amec_sys.proc[0].parm_f_override_enable),

    // Altitude in meters
    AMEC_PARM_UINT16(PARM_ALTITUDE,"altitude",&g_amec_sys.sys.altitude),

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

    // Begin WOF parameters
    AMEC_PARM_UINT32(PARM_WOF_DISABLED, "wof_disabled", &g_amec_sys.wof.wof_disabled),
    AMEC_PARM_UINT16(PARM_VDD_STEP_FROM_START, "vddStpFrmStart", &g_amec_sys.wof.vdd_step_from_start),
    AMEC_PARM_UINT16(PARM_VCS_STEP_FROM_START, "vcsStpFrmStart", &g_amec_sys.wof.vcs_step_from_start),
    AMEC_PARM_UINT16(PARM_IO_PWR_STEP_FROM_START, "ioPwrStpFrmStrt", &g_amec_sys.wof.io_pwr_step_from_start),
    AMEC_PARM_UINT16(PARM_AMBIENT_STEP_FROM_START, "ambStpFrmStrt", &g_amec_sys.wof.ambient_step_from_start),
    AMEC_PARM_UINT32(PARM_VOLT_VDD_SENSE, "voltvddsense", &g_amec_sys.wof.Vdd_chip_p1mv),
    AMEC_PARM_UINT32(PARM_VOLT_VCS_SENSE, "voltvcssense", &g_amec_sys.wof.Vcs_chip_p1mv),
    AMEC_PARM_UINT16_ARRAY(PARM_TEMPPROCTHERMC, "tempprocthrmc", &g_amec_sys.wof.tempprocthrmc, MAX_NUM_CORES),
    AMEC_PARM_UINT16(PARM_TEMP_RT_AVG, "tempRTavg", &g_amec_sys.wof.T_racetrack),
    AMEC_PARM_UINT16(PARM_CURVDD_SENSE, "curvdd", &g_amec_sys.wof.curvdd_sensor),
    AMEC_PARM_UINT16(PARM_CURVCS_SENSE, "curvcs", &g_amec_sys.wof.curvcs_sensor),
    AMEC_PARM_UINT32(PARM_IDC_VDD, "idc_vdd", &g_amec_sys.wof.iddq_ua),
    AMEC_PARM_UINT32(PARM_IDC_VCS, "idc_vcs", &g_amec_sys.wof.icsq_ua),
    AMEC_PARM_UINT32(PARM_IAC_VDD, "iac_vdd", &g_amec_sys.wof.iac_vdd),
    AMEC_PARM_UINT32(PARM_IAC_VCS, "iac_vcs", &g_amec_sys.wof.iac_vcs),
    AMEC_PARM_UINT32(PARM_IAC_TDP_VDD, "iac_tdp_vdd", &g_amec_sys.wof.iac_tdp_vdd),
    AMEC_PARM_UINT16(PARM_V_RATIO, "Vratio", &g_amec_sys.wof.v_ratio),
    AMEC_PARM_UINT8(PARM_F_CLIP_PS, "Fclip_PS", &g_amec_sys.wof.f_clip_ps),
    AMEC_PARM_UINT32(PARM_F_CLIP_FREQ, "Fclip_Freq", &g_amec_sys.wof.f_clip_freq),
    AMEC_PARM_UINT32(PARM_CEFF_TDP_VDD, "ceff_tdp_vdd", &g_amec_sys.wof.ceff_tdp_vdd),
    AMEC_PARM_UINT32(PARM_CEFF_VDD, "ceff_vdd", &g_amec_sys.wof.ceff_vdd),
    AMEC_PARM_UINT32(PARM_CEFF_RATIO_VDD, "ceff_ratio_vdd", &g_amec_sys.wof.ceff_ratio_vdd),
    AMEC_PARM_UINT32(PARM_CEFF_TDP_VCS, "ceff_tdp_vcs", &g_amec_sys.wof.ceff_tdp_vcs),
    AMEC_PARM_UINT32(PARM_CEFF_VCS, "ceff_vcs", &g_amec_sys.wof.ceff_vcs),
    AMEC_PARM_UINT32(PARM_CEFF_RATIO_VCS, "ceff_ratio_vcs", &g_amec_sys.wof.ceff_ratio_vcs),
    AMEC_PARM_UINT8(PARM_VOLTAGE_IDX, "voltage_idx", &g_amec_sys.wof.Vdd_chip_index),

    AMEC_PARM_UINT32(PARM_CURR_PING_PONG_BUF, "currPingPongBuf", &g_amec_sys.wof.curr_ping_pong_buf),
    AMEC_PARM_UINT32(PARM_NEXT_PING_PONG_BUF, "nextPingPongBuf", &g_amec_sys.wof.next_ping_pong_buf),
    AMEC_PARM_UINT32(PARM_VRT_MAIN_MEM_ADDR, "vrtMainMemAddr", &g_amec_sys.wof.vrt_main_mem_addr),
    AMEC_PARM_UINT32(PARM_VRT_BCE_OFFSET, "vrtBceOffset", &g_amec_sys.wof.vrt_bce_table_offset),
    AMEC_PARM_UINT8(PARM_WOF_INIT_STATE, "wof_init_state", &g_amec_sys.wof.wof_init_state),
    AMEC_PARM_UINT32(PARM_GPE_REQ_RC, "gpeReqRc", &g_amec_sys.wof.gpe_req_rc),
    AMEC_PARM_UINT32(PARM_CONTROL_IPC_RC, "ctrlIpcRc", &g_amec_sys.wof.control_ipc_rc),
    AMEC_PARM_UINT8(PARM_VRT_CALLBACK_ERR, "vrtCallbackErr", &g_amec_sys.wof.vrt_callback_error),
    AMEC_PARM_UINT8(PARM_PGPE_WOF_OFF, "pgpeWofOff", &g_amec_sys.wof.pgpe_wof_off),
    AMEC_PARM_UINT8(PARM_PGPE_WOF_DISABLED, "pgpeWofDisabled", &g_amec_sys.wof.pgpe_wof_disabled),
    AMEC_PARM_UINT32(PARM_VRT_MM_OFFSET, "vrt_mm_offset", &g_amec_sys.wof.vrt_mm_offset),
    AMEC_PARM_UINT8(PARM_VRT_REQ_RC, "wof_vrt_req_rc", &g_amec_sys.wof.wof_vrt_req_rc ),
    AMEC_PARM_UINT32(PARM_VDD_RATIO_VOLT, "vddRatioVolt", &g_amec_sys.wof.c_ratio_vdd_volt),
    AMEC_PARM_UINT32(PARM_VDD_RATIO_FREQ, "vddRatioFreq", &g_amec_sys.wof.c_ratio_vdd_freq),
    AMEC_PARM_UINT32(PARM_VCS_RATIO_VOLT, "vcsRatioVolt", &g_amec_sys.wof.c_ratio_vcs_volt),
    AMEC_PARM_UINT32(PARM_VCS_RATIO_FREQ, "vcsRatioFreq", &g_amec_sys.wof.c_ratio_vcs_freq),
    AMEC_PARM_UINT8(PARM_VRT_STATE,"vrtState", &g_amec_sys.wof.vrt_state),
    AMEC_PARM_UINT64(PARM_PGPE_WOF_DW0, "PGPE_WOF_dw0", &g_amec_sys.wof.pgpe_wof_values_dw0 ),
    AMEC_PARM_UINT64(PARM_PGPE_WOF_DW1, "PGPE_WOF_dw1", &g_amec_sys.wof.pgpe_wof_values_dw1 ),
    AMEC_PARM_UINT64(PARM_PGPE_WOF_DW2, "PGPE_WOF_dw2", &g_amec_sys.wof.pgpe_wof_values_dw2 ),
    AMEC_PARM_UINT64(PARM_PGPE_WOF_DW3, "PGPE_WOF_dw3", &g_amec_sys.wof.pgpe_wof_values_dw3 ),
    AMEC_PARM_UINT8(PARM_OCS_DIRTY,"OCS_DIRTY_BITS", &g_amec_sys.wof.ocs_dirty),
    AMEC_PARM_UINT16(PARM_OCS_CEFF_UP_ADDR,"OcsFixedUpAmt", &g_amec_sys.wof.ocs_increase_ceff),
    AMEC_PARM_UINT16(PARM_OCS_CEFF_DOWN_ADDR,"OcsFixedDownAmt", &g_amec_sys.wof.ocs_decrease_ceff),
    AMEC_PARM_UINT16(PARM_OCS_CALC_ADDR,"OCS_calc_addr", &g_amec_sys.wof.vdd_oc_ceff_add),
    AMEC_PARM_UINT16(PARM_CEFF_ADJ_PREV,"ceffAdjPrev", &g_amec_sys.wof.vdd_ceff_ratio_adj_prev),
    AMEC_PARM_UINT32(PARM_VDD_TDP_100UV,"VddTDP100UV", &g_amec_sys.wof.vdd_avg_tdp_100uv),
    AMEC_PARM_UINT32(PARM_OCS_NOT_DIRTY_TYPE0_CNT,"NotDirtyTyp0Cnt", &g_amec_sys.wof.ocs_not_dirty_count),
    AMEC_PARM_UINT32(PARM_OCS_NOT_DIRTY_TYPE1_CNT,"NotDirtyTyp1Cnt", &g_amec_sys.wof.ocs_not_dirty_type1_count),
    AMEC_PARM_UINT32(PARM_OCS_DIRTY_TYPE0_CNT,"DirtyTyp0Cnt", &g_amec_sys.wof.ocs_dirty_type0_count),
    AMEC_PARM_UINT32(PARM_OCS_DIRTY_TYPE1_CNT,"DirtyTyp1Cnt", &g_amec_sys.wof.ocs_dirty_type1_count),
    AMEC_PARM_UINT32(PARM_AMBIENT_CONDITION,"AmbientCondition", &g_amec_sys.wof.ambient_condition),
    // End WOF parameters
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
