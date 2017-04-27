/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_parm.h $                                */
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

/*
  This interface takes named memory objects (such as global variables)
  and makes them accessible to Amester parameter for reading and
  writing.  Any memory location may become an Amester parameter.

  To add a parameter, 
  1. Add a new parameter id number in the AMEC_PARM_ENUM below.
  2. In the same position, add the parameter to g_amec_parm_list in 
     amec_parm_table.c
     There are macros that help in adding a parameter to the table.
     The macro typically takes a) the parameter id, b) a string name,
     and c) a pointer to the memory location.
*/

#ifndef _AMEC_PARM_H
#define _AMEC_PARM_H

//*************************************************************************
// Includes
//*************************************************************************
#include <amec_amester.h>

//*************************************************************************
// Defines/Enums
//*************************************************************************

// List of all parameters
// NOTE: The parameters must be in the same order as g_amec_parm_list[] in 
// amec_parm_table.c
typedef enum
{
    PARM_SYS_FMAX,
    PARM_SYS_FMIN,
    PARM_GPST,
    PARM_PSTATE_MHZ,
    PARM_FREQ_REASON,
    PARM_FREQ_OR,
    PARM_FREQ_OR_EN,
    PARM_SYS_THRM_SP,
    PARM_SYS_THRM_GAIN,
    PARM_SYS_THRM_RES,
    PARM_SYS_THRM_SPEED,
    PARM_SYS_THRM_FREQ,
    PARM_SOFT_FMIN,
    PARM_SOFT_FMAX,
    PARM_TOD,
    // WOF Parameters
    PARM_WOF_HDR_VERSION,
    PARM_VFRT_BLOCK_SIZE,
    PARM_VFRT_BLOCK_HEADER_SZ,
    PARM_VFRT_DATA_SIZE,
    PARM_ACTIVE_QUADS_SIZE,
    PARM_CORE_COUNT,
    PARM_VDN_START,
    PARM_VDN_STEP,
    PARM_VDN_SIZE,
    PARM_VDD_START,
    PARM_VDD_STEP,
    PARM_VDD_SIZE,
    PARM_VRATIO_START,
    PARM_VRATIO_STEP,
    PARM_VRATIO_SIZE,
    PARM_FRATIO_START,
    PARM_FRATIO_STEP,
    PARM_FRATIO_SIZE,
    PARM_VDN_PERCENT,
    PARM_SOCKET_POWER_W,
    PARM_NEST_FREQ_MHZ,
    PARM_NOM_FREQ_MHZ,
    PARM_RDP_CAPACITY,
    PARM_WOF_TBLS_SRC_TAG,
    PARM_PACKAGE_NAME_HI,
    PARM_PACKAGE_NAME_LO,
    PARM_VDD_STEP_FROM_START,
    PARM_VDN_STEP_FROM_START,
    PARM_QUAD_STEP_FROM_START,
    PARM_V_CORE,
    PARM_CORE_PWR_ON,
    PARM_CORES_ON_PER_QUAD,
    PARM_WOF_DISABLED,
    PARM_VOLT_VDD_SENSE,
    PARM_TEMPPROCTHERMC,
    PARM_TEMPNEST,
    PARM_TEMPQ,
    PARM_CURVDD_SENSE,
    PARM_CURVDN_SENSE,
    PARM_VOLTVDN,
    PARM_QUAD_X_PSTATES,
    PARM_QUAD_V_IDX,
    PARM_IVRM_STATES,
    PARM_IDC_VDD,
    PARM_IDC_VDN,
    PARM_IDC_QUAD,
    PARM_IAC_VDD,
    PARM_IAC_VDN,
    PARM_IAC_TDP_VDD,
    PARM_VOLTAGE_IDX,
    PARM_V_RATIO,
    PARM_F_RATIO,
    PARM_V_CLIP,
    PARM_F_CLIP,
    PARM_CEFF_TDP_VDD,
    PARM_CEFF_VDD,
    PARM_CEFF_RATIO_VDD,
    PARM_CEFF_TDP_VDN,
    PARM_CEFF_VDN,
    PARM_CEFF_RATIO_VDN,
    PARM_ALL_CORES_OFF_ISO,
    PARM_ALL_CACHES_ON_ISO,
    PARM_QUAD_GOOD_CORES_ONLY,
    PARM_QUAD_ON_CORES,
    PARM_QUAD_BAD_OFF_CORES,
    PARM_REQ_ACTIVE_QUAD_UPDATE,
    PARM_PREV_REQ_ACTIVE_QUADS,
    PARM_NUM_ACTIVE_QUADS,
    PARM_CURR_PING_PONG_BUF,
    PARM_NEXT_PING_PONG_BUF,
    PARM_CURR_VFRT_MAIN_MEM_ADDR,
    PARM_NEXT_VFRT_MAIN_MEM_ADDR,
    PARM_VFRT_TBLS_MAIN_MEM_ADDR,
    PARM_VFRT_TBLS_LEN,
    PARM_WOF_INIT_STATE,
    PARM_QUAD_STATE_0_ADDR,
    PARM_QUAD_STATE_1_ADDR,
    PARM_PGPE_WOF_STATE_ADDR,
    PARM_REQ_ACTIVE_QUADS_ADDR,
    PARM_CORE_LEAKAGE_PERCENT,
    // End WOF Parameters
    AMEC_PARM_NUMBER_OF_PARAMETERS
} AMEC_PARM_ENUM;

typedef enum
{
    AMEC_PARM_TYPE_UINT8 = 0,
    AMEC_PARM_TYPE_UINT16,
    AMEC_PARM_TYPE_UINT32,
    AMEC_PARM_TYPE_UINT64,
    AMEC_PARM_TYPE_INT8,
    AMEC_PARM_TYPE_INT16,
    AMEC_PARM_TYPE_INT32,
    AMEC_PARM_TYPE_INT64,
    AMEC_PARM_TYPE_STRING,
    AMEC_PARM_TYPE_RAW
		
} AMEC_PARM_TYPE_ENUM;

#define AMEC_PARM_MODE_NORMAL   (0x00)
#define AMEC_PARM_MODE_READONLY (0x01)

// Length includes null byte terminator. 15 readable characters are allowed.
#define AMEC_PARM_NAME_LENGTH 16

typedef struct amec_parm_s
{
    /// name of parameter
    CHAR name[AMEC_PARM_NAME_LENGTH];
    /// value_ptr: pointer to data
    UINT8 *value_ptr;
    /// number of bytes in base data
    UINT32 length; 
    /// vector_length is the number of items in the array pointed by value ptr.
    UINT32 vector_length; 
    /// Type of data
    UINT8 type : 4;
    /// Mode of data (read-write, read-only, etc.)
    UINT8 mode : 1;
    /// If preread is 1, call amec_parm_preread(GUID) before reading parameter value.
    UINT8 preread : 1;
    /// If postwrite is 1, call amec_parm_postwrite(GUID) before reading parameter value.
    UINT8 postwrite : 1;
} amec_parm_t;

typedef  UINT16     AMEC_PARM_GUID;

extern amec_parm_t  g_amec_parm_list[];

/*******************************************************************/
/* Function Definitions                                            */
/*******************************************************************/

/**
 * Get number of parameters tracked by OCC
 *
 */
void amec_parm_get_number(const IPMIMsg_t *i_psMsg,
                          UINT8 *o_pu8Resp,
                          UINT16 *o_pu16RespLength,
                          UINT8 *o_retval);

/**
 * Get parameter configuration (names, types, size, etc.)
 *
 */
void amec_parm_get_config(const IPMIMsg_t *i_psMsg,
                          UINT8 *o_pu8Resp,
                          UINT16 *o_pu16RespLength,
                          UINT8 *o_retval);

/**
 * Read a parameter value 
 *  
 */
void amec_parm_read(const IPMIMsg_t *const i_psMsg,
                    UINT8 *const o_pu8Resp,
                    UINT16 *const o_pu16RespLength,
                    UINT8 *const o_retval);

/**
 * Write a value to a parameter
 *
 */
void amec_parm_write(const IPMIMsg_t *const i_psMsg,
                     UINT8 *const o_pu8Resp,
                     UINT16 *const o_pu16RespLength,
                     UINT8 *const o_retval);

/**
 * Update parameter value before reading
 *
 * Some parameters need to be updated before reading.
 * For example, a parameter that points to double-buffered 
 * that may be at a new memory location each time the
 * parameter is examined. 
 * This routine only needs to be called when the parameter
 * has a 'preread' field with a value of 1.
 */
void amec_parm_preread(AMEC_PARM_GUID i_parm_guid);

/**
 * Update parameter value after writing 
 *
 * Some parameters trigger actions after writing.
 * This routine only needs to be called when the parameter
 * has been written and has a 'postwrite' field with a value of 1.
 */
void amec_parm_postwrite(AMEC_PARM_GUID i_parm_guid);

#endif
