/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_parm.h $                                */
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
typedef enum
{
    PARM_SYS_FMAX,
    PARM_SYS_FMIN,
    PARM_PSTATE_MHZ,
    PARM_FREQ_REASON,
    PARM_FREQ_OR,
    PARM_FREQ_OR_EN,
    PARM_ALTITUDE,
    PARM_SYS_THRM_SP,
    PARM_SYS_THRM_GAIN,
    PARM_SYS_THRM_RES,
    PARM_SYS_THRM_SPEED,
    PARM_SYS_THRM_FREQ,
    PARM_SOFT_FMIN,
    PARM_SOFT_FMAX,
    PARM_TOD,
    // WOF Parameters
    PARM_WOF_DISABLED,
    PARM_VDD_STEP_FROM_START,
    PARM_VCS_STEP_FROM_START,
    PARM_IO_PWR_STEP_FROM_START,
    PARM_AMBIENT_STEP_FROM_START,
    PARM_VOLT_VDD_SENSE,
    PARM_VOLT_VCS_SENSE,
    PARM_TEMPPROCTHERMC,
    PARM_TEMP_RT_AVG,
    PARM_CURVDD_SENSE,
    PARM_CURVCS_SENSE,
    PARM_IDC_VDD,
    PARM_IDC_VCS,
    PARM_IAC_VDD,
    PARM_IAC_VCS,
    PARM_IAC_TDP_VDD,
    PARM_V_RATIO,
    PARM_F_CLIP_PS,
    PARM_CEFF_RATIO_VDD_N,
    PARM_CEFF_RATIO_VDD_D,
    PARM_CEFF_RATIO_VDD,
    PARM_CEFF_RATIO_VCS_N,
    PARM_CEFF_RATIO_VCS_D,
    PARM_CEFF_RATIO_VCS,
    PARM_VOLTAGE_IDX,
    PARM_CURR_PING_PONG_BUF,
    PARM_NEXT_PING_PONG_BUF,
    PARM_VRT_MAIN_MEM_ADDR,
    PARM_VRT_BCE_OFFSET,
    PARM_WOF_INIT_STATE,
    PARM_GPE_REQ_RC,
    PARM_CONTROL_IPC_RC,
    PARM_VRT_CALLBACK_ERR,
    PARM_PGPE_WOF_OFF,
    PARM_PGPE_WOF_DISABLED,
    PARM_VRT_MM_OFFSET,
    PARM_VRT_REQ_RC,
    PARM_VDD_RATIO,
    PARM_CEFF_RATIO_FREQ,
    PARM_VCS_RATIO,
    PARM_VRT_STATE,
    PARM_PGPE_WOF_DW0,
    PARM_PGPE_WOF_DW1,
    PARM_PGPE_WOF_DW2,
    PARM_PGPE_WOF_DW3,
    PARM_OCS_DIRTY,
    PARM_OCS_CEFF_UP_ADDR,
    PARM_OCS_CEFF_DOWN_ADDR,
    PARM_OCS_CALC_ADDR,
    PARM_CEFF_ADJ_PREV,
    PARM_OCS_NOT_DIRTY_TYPE0_CNT,
    PARM_OCS_NOT_DIRTY_TYPE1_CNT,
    PARM_OCS_DIRTY_TYPE0_CNT,
    PARM_OCS_DIRTY_TYPE1_CNT,
    PARM_AMBIENT_CONDITION,
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
