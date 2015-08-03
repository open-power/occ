/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_parm_table.c $                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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
