/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_parm_table.c $                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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
// Note: The parameters must be in the same order as in AMEC_PARM_ENUM
// in amec_parm.h
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
    AMEC_PARM_UINT8(PARM_SIZE_OF_VFRT , "size_of_vfrt", &g_amec_sys.wof.size_of_vfrt ),
    AMEC_PARM_UINT8(PARM_VFRT_DATA_SIZE , "vfrt_data_size", &g_amec_sys.wof.vfrt_data_size ),
    AMEC_PARM_UINT8(PARM_ACTIVE_QUADS_START , "actv_quads_start", &g_amec_sys.wof.active_quads_start ),
    AMEC_PARM_UINT8(PARM_ACTIVE_QUADS_SIZE , "actv_quads_size", &g_amec_sys.wof.active_quads_size ),
    AMEC_PARM_UINT8(PARM_VDN_START , "vdn_start", &g_amec_sys.wof.vdn_start ),
    AMEC_PARM_UINT8(PARM_VDN_STEP , "vdn_step", &g_amec_sys.wof.vdn_step ),
    AMEC_PARM_UINT8(PARM_VDN_SIZE , "vdn_size", &g_amec_sys.wof.vdn_size ),
    AMEC_PARM_UINT8(PARM_VDD_START , "vdd_start", &g_amec_sys.wof.vdd_start ),
    AMEC_PARM_UINT8(PARM_VDD_STEP , "vdd_step", &g_amec_sys.wof.vdd_step ),
    AMEC_PARM_UINT8(PARM_VDD_SIZE , "vdd_size", &g_amec_sys.wof.vdd_size ),
    AMEC_PARM_UINT16(PARM_VDD_STEP_FROM_START, "vdd_stp_frm_strt", &g_amec_sys.wof.vdd_step_from_start),
    AMEC_PARM_UINT16(PARM_VDN_STEP_FROM_START, "vdn_stp_frm_strt", &g_amec_sys.wof.vdn_step_from_start),
    AMEC_PARM_UINT8(PARM_QUAD_STEP_FROM_START, "quadStpFrmStrt", &g_amec_sys.wof.quad_step_from_start),
    AMEC_PARM_UINT32_ARRAY(PARM_V_CORE, "v_core_100uV", &g_amec_sys.wof.v_core_100uV, MAXIMUM_QUADS),
    AMEC_PARM_UINT32(PARM_CORE_PWR_ON, "core_pwr_on", &g_amec_sys.wof.core_pwr_on),
    AMEC_PARM_UINT8_ARRAY(PARM_CORES_ON_PER_QUAD, "coreson_per_quad", &g_amec_sys.wof.cores_on_per_quad, MAXIMUM_QUADS),
    AMEC_PARM_UINT16(PARM_WOF_DISABLED, "wof_disabled", &g_amec_sys.wof.wof_disabled),
    AMEC_PARM_UINT32(PARM_VOLT_VDD_SENSE, "voltvddsense", &g_amec_sys.wof.voltvddsense_sensor),
    AMEC_PARM_UINT16_ARRAY(PARM_TEMPPROCTHERMC, "tempprocthrmc", &g_amec_sys.wof.tempprocthrmc, MAX_NUM_CORES),
    AMEC_PARM_UINT16(PARM_TEMPNEST, "tempnest_sensor", &g_amec_sys.wof.tempnest_sensor),
    AMEC_PARM_UINT16_ARRAY(PARM_TEMPQ, "tempq", &g_amec_sys.wof.tempq, MAXIMUM_QUADS),
    AMEC_PARM_UINT16(PARM_VOLTVDN, "voltvdn_sensor", &g_amec_sys.wof.voltvdn_sensor),
    AMEC_PARM_UINT8_ARRAY(PARM_QUAD_X_PSTATES, "quad_x_pstates", &g_amec_sys.wof.quad_x_pstates, MAXIMUM_QUADS),
    AMEC_PARM_UINT8(PARM_IVRM_STATES, "quad_ivrm_states", &g_amec_sys.wof.quad_ivrm_states),
    AMEC_PARM_UINT32(PARM_IDC_VDD, "idc_vdd", &g_amec_sys.wof.idc_vdd),
    AMEC_PARM_UINT32(PARM_IDC_VDN, "idc_vdn", &g_amec_sys.wof.idc_vdn),
    AMEC_PARM_UINT32(PARM_IDC_QUAD, "idc_quad", &g_amec_sys.wof.idc_quad),
    AMEC_PARM_UINT32(PARM_IAC_VDD, "iac_vdd", &g_amec_sys.wof.iac_vdd),
    AMEC_PARM_UINT32(PARM_IAC_VDN, "iac_vdn", &g_amec_sys.wof.iac_vdn),
    AMEC_PARM_UINT32(PARM_IAC_TDP_VDD, "iac_tdp_vdd", &g_amec_sys.wof.iac_tdp_vdd),
    AMEC_PARM_UINT32(PARM_IAC_TDP_VDN, "iac_tdp_vdn", &g_amec_sys.wof.iac_tdp_vdn),
    AMEC_PARM_UINT32(PARM_V_RATIO, "Vratio", &g_amec_sys.wof.v_ratio),
    AMEC_PARM_UINT32(PARM_F_RATIO, "Fratio", &g_amec_sys.wof.f_ratio),
    AMEC_PARM_UINT32(PARM_V_CLIP, "Vclip", &g_amec_sys.wof.v_clip),
    AMEC_PARM_UINT32(PARM_F_CLIP, "Fclip", &g_amec_sys.wof.f_clip),
    AMEC_PARM_UINT32(PARM_CEFF_TDP_VDD, "ceff_tdp_vdd", &g_amec_sys.wof.ceff_tdp_vdd),
    AMEC_PARM_UINT32(PARM_CEFF_VDD, "ceff_vdd", &g_amec_sys.wof.ceff_vdd),
    AMEC_PARM_UINT32(PARM_CEFF_RATIO_VDD, "ceff_ratio_vdd", &g_amec_sys.wof.ceff_ratio_vdd),
    AMEC_PARM_UINT32(PARM_CEFF_TDP_VDN, "ceff_tdp_vdn", &g_amec_sys.wof.ceff_tdp_vdn),
    AMEC_PARM_UINT32(PARM_CEFF_VDN, "ceff_vdn", &g_amec_sys.wof.ceff_vdn),
    AMEC_PARM_UINT32(PARM_CEFF_RATIO_VDN, "ceff_ratio_vdn", &g_amec_sys.wof.ceff_ratio_vdn),

    AMEC_PARM_UINT8(PARM_VOLTAGE_IDX, "voltage_idx", &g_amec_sys.wof.voltage_idx),
    AMEC_PARM_UINT32(PARM_ALL_CORES_OFF_ISO, "allcores_off_iso", &g_amec_sys.wof.all_cores_off_iso),
    AMEC_PARM_UINT32(PARM_ALL_CACHES_ON_ISO, "allcaches_on_iso", &g_amec_sys.wof.all_caches_on_iso),
    AMEC_PARM_UINT16_ARRAY(PARM_QUAD_GOOD_CORES_ONLY, "quad_good_cores", &g_amec_sys.wof.quad_good_cores_only, MAXIMUM_QUADS),
    AMEC_PARM_UINT16_ARRAY(PARM_QUAD_ON_CORES, "quad_on_cores", &g_amec_sys.wof.quad_on_cores, MAXIMUM_QUADS),
    AMEC_PARM_UINT16_ARRAY(PARM_QUAD_BAD_OFF_CORES,"quadBadOffCores", &g_amec_sys.wof.quad_on_cores, MAXIMUM_QUADS),
    AMEC_PARM_UINT32(PARM_NEST_MULT, "nest_mult", &g_amec_sys.wof.nest_mult),
    AMEC_PARM_UINT32_ARRAY(PARM_CORE_MULT, "core_mult", &g_amec_sys.wof.core_mult, MAX_NUM_CORES),
    AMEC_PARM_UINT32_ARRAY(PARM_QUAD_MULT, "quad_mult", &g_amec_sys.wof.quad_mult, MAXIMUM_QUADS),
    AMEC_PARM_INT16(PARM_NEST_DELTA_TEMP, "nest_delta_temp", &g_amec_sys.wof.nest_delta_temp),
    AMEC_PARM_INT16_ARRAY(PARM_CORE_DELTA_TEMP, "core_delta_temp", &g_amec_sys.wof.core_delta_temp, MAX_NUM_CORES),
    AMEC_PARM_INT16_ARRAY(PARM_QUAD_DELTA_TEMP, "quad_delta_temp", &g_amec_sys.wof.quad_delta_temp, MAX_NUM_CORES),
    AMEC_PARM_UINT16(PARM_TVPD_LEAK_OFF, "tvpd_leak_off", &g_amec_sys.wof.tvpd_leak_off),
    AMEC_PARM_UINT16(PARM_TVPD_LEAK_ON, "tvpd_leak_on", &g_amec_sys.wof.tvpd_leak_on),
    AMEC_PARM_UINT16(PARM_TVPD_LEAK_CACHE, "tvpd_leak_cache", &g_amec_sys.wof.tvpd_leak_cache),
    AMEC_PARM_UINT16(PARM_TVPD_LEAK_NEST, "tvpd_leak_nest", &g_amec_sys.wof.tvpd_leak_nest),
    AMEC_PARM_UINT8(PARM_REQ_ACTIVE_QUAD_UPDATE, "req_active_quad", &g_amec_sys.wof.req_active_quad_update),
    AMEC_PARM_UINT8(PARM_PREV_REQ_ACTIVE_QUADS, "prevActiveQuads", &g_amec_sys.wof.prev_req_active_quads),
    AMEC_PARM_UINT32(PARM_CURR_PING_PONG_BUF, "currPingPongBuf", &g_amec_sys.wof.curr_ping_pong_buf),
    AMEC_PARM_UINT32(PARM_NEXT_PING_PONG_BUF, "nextPingPongBuf", &g_amec_sys.wof.next_ping_pong_buf),
    AMEC_PARM_UINT32(PARM_CURR_VFRT_MAIN_MEM_ADDR, "vfrtMainMemAddr", &g_amec_sys.wof.curr_vfrt_main_mem_addr),
    AMEC_PARM_UINT32(PARM_NEXT_VFRT_MAIN_MEM_ADDR, "nxt_vfrt_MM_addr", &g_amec_sys.wof.next_vfrt_main_mem_addr),
    AMEC_PARM_UINT32(PARM_ACTIVE_QUADS_SRAM_ADDR, "activQuadSramPtr", &g_amec_sys.wof.active_quads_sram_addr),
    AMEC_PARM_UINT32(PARM_VFRT_TBLS_MAIN_MEM_ADDR, "vfrtTblsMMAddr", &g_amec_sys.wof.vfrt_tbls_main_mem_addr),
    AMEC_PARM_UINT32(PARM_VFRT_TBLS_LEN, "vfrt_tbls_len", &g_amec_sys.wof.vfrt_tbls_len),
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
