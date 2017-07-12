/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/gpe_get_tod.c $                                  */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017,2017                        */
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

/**
 * @file gpe_get_tod.c
 *
 * This file defines the functions for getting the current value of the Time Of
 * Day (TOD) register.
 */

//******************************************************************************
// Includes
//******************************************************************************

#include <stdint.h>                 // For uint*_t
#include "pk.h"                     // For PK_TRACE() and pk_halt()
#include "ipc_api.h"                // For ipc_msg_t
#include "ipc_async_cmd.h"          // For ipc_async_cmd_t
#include "ppe42_scom.h"             // For getscom_abs()
#include "gpe_util.h"               // For gpe_set_ffdc()
#include "get_tod_structs.h"        // For gpe_get_tod_args_t
#include "gpe_err.h"                // For GPE_RC_*
#include "pss_constants.h"          // For TOD_STATUS_REG and TOD_VALUE_REG


//******************************************************************************
// Defines
//******************************************************************************

/**
 * Returns the IS_RUNNING bit within the TOD_STATUS_REG register.
 *
 * IS_RUNNING is bit 20 in the register value.  Note that bit 0 is the most
 * significant bit.
 *
 * @param reg_value value of TOD_STATUS_REG register
 * @return IS_RUNNING bit.  If 1, the TOD is running.
 */
#define TOD_IS_RUNNING(reg_value) \
    (((uint8_t) (((uint64_t) (reg_value)) >> 43)) & 0x01u)

/**
 * Returns the TOD_VALUE field within the TOD_VALUE_REG register.
 *
 * The TOD_VALUE field is located in bits 0:59 of the register value.  Note that
 * bit 0 is the most significant bit.  In the returned value we must set the low
 * order 4 bits (60:63) to 0 rather than right shifting 4 bits.
 *
 * @param reg_value value of TOD_VALUE_REG register
 * @return TOD_VALUE field
 */
#define TOD_VALUE(reg_value)  (((uint64_t) (reg_value)) & 0xFFFFFFFFFFFFFFF0ull)


//******************************************************************************
// Functions
//******************************************************************************

/**
 * Reads the value of the TOD_STATUS_REG and TOD_VALUE_REG registers.  If the
 * TOD is running, the current Time Of Day value is stored in the tod field of
 * the args parameter.
 *
 * @param args Arguments passed from the OCC 405 via the IPC framework
 */
void gpe_read_tod_registers(gpe_get_tod_args_t * args)
{
    // Initialize output arguments to default values
    args->error.rc   = GPE_RC_SUCCESS;
    args->error.addr = 0;
    args->error.ffdc = 0;
    args->tod        = TOD_VALUE_UNKNOWN;

    // Read value of TOD_STATUS_REG register
    uint64_t l_reg_val;
    uint32_t rc = getscom_abs(TOD_STATUS_REG, &l_reg_val);
    if (rc != 0)
    {
        gpe_set_ffdc(&(args->error), TOD_STATUS_REG, GPE_RC_SCOM_GET_FAILED, rc);
        return;
    }

    // Check if TOD is running based on TOD_STATUS_REG value.
    if (!TOD_IS_RUNNING(l_reg_val))
    {
        // TOD is not running.  Operating system is likely fixing it.  Not an error.
        return;
    }

    // Read value of TOD_VALUE_REG register
    rc = getscom_abs(TOD_VALUE_REG, &l_reg_val);
    if (rc != 0)
    {
        gpe_set_ffdc(&(args->error), TOD_VALUE_REG, GPE_RC_SCOM_GET_FAILED, rc);
        return;
    }

    // The TOD_VALUE_REG register contains two fields.  Get the TOD_VALUE field
    // and store that as the Time Of Day value.
    args->tod = TOD_VALUE(l_reg_val);
}


/**
 * IPC function that gets the current Time Of Day (TOD) value.
 *
 * First reads the TOD_STATUS_REG register to make sure the TOD is running.
 * Then reads the TOD_VALUE_REG register to get the TOD value.
 *
 * @param cmd A pointer to the IPC command message
 * @param arg IPC function argument.  Currently not used.
 */
void gpe_get_tod(ipc_msg_t * cmd, void * arg)
{
    // Cast command message pointer to more specific type to access cmd_data field
    ipc_async_cmd_t * async_cmd = (ipc_async_cmd_t *) cmd;

    // Cast cmd_data field to specific type of arguments for this IPC function
    gpe_get_tod_args_t * args = (gpe_get_tod_args_t *) async_cmd->cmd_data;

    // Read the TOD registers to get the TOD value.  Store the result in args.
    gpe_read_tod_registers(args);

    // Send back a response with IPC success even if ffdc/rc fields are non-zero
    int rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
    if (rc != 0)
    {
        PK_TRACE("gpe_get_tod: Failed to send response back. Halting GPE0. rc=0x%08X", rc);
        gpe_set_ffdc(&(args->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }
}
