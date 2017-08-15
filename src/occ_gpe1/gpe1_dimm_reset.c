/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe1_dimm_reset.c $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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

/// \file gpe1_dimm_reset.c
/// \brief Functions to handle resetting the I2C engine
///

//#define GPE1_DEBUG
#include "pk.h"
#include "ipc_api.h"
#include "ppe42_scom.h"
#include "ipc_async_cmd.h"
#include "gpe1.h"
#include "gpe1_dimm.h"
#include "dimm_structs.h"


/*
 * Function Specifications:
 *
 * Name: dimm_reset_master
 *
 * Description:  Reset the I2C master
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */
void dimm_reset_master(ipc_msg_t* cmd, void* arg)
{
    // Note: arg was set to 0 in ipc func table (ipc_func_tables.c), so don't use it.
    // the ipc arguments passed through the ipc_msg_t structure, has a pointer
    // to the G_gpe_start_pwr_meas_read_args struct.

    int      rc;
    uint32_t scomAddr;
    uint64_t regValue; // a pointer to hold the putscom_abs register value
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    dimm_sm_args_t *args = (dimm_sm_args_t*)async_cmd->cmd_data;

    // Reset I2C Master
    scomAddr = I2C_IMM_RESET_I2C | SCOM_ENGINE_OFFSET(args->i2cEngine);
    regValue = 0x0000000000000000;
    rc = putscom_abs(scomAddr, regValue);
    if(rc)
    {
        PK_TRACE("dimm_reset_master: I2C_IMM_RESET_I2C putscom 0x%08X->0x%08X%08X FAILED. rc = 0x%08x",
                 scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue), rc);
        gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_SCOM_PUT_FAILED, rc);
    }
    else
    {
        GPE1_DIMM_DBG("dimm_reset_master: putscom(0x%08X,0x%08X%08X) SUCCESS - IMM_RESET_I2C",
                      scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue));

        // Force reset of Port_busy_register
        scomAddr = I2C_BUSY_REGISTER | SCOM_ENGINE_OFFSET(args->i2cEngine);
        regValue = 0x8000000000000000;
        rc = putscom_abs(scomAddr, regValue);
        if(rc)
        {
            PK_TRACE("dimm_reset_master: I2C_BUSY_REGISTER putscom 0x%08X->0x%08X%08X FAILED. rc = 0x%08x",
                     scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue), rc);
            gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_SCOM_PUT_FAILED, rc);
        }
        else
        {
            GPE1_DIMM_DBG("dimm_reset_master: putscom(0x%08X,0x%08X%08X) SUCCESS - I2C_BUSY_REGISTER",
                          scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue));
        }
    }

} // end dimm_reset_master()


/*
 * Function Specifications:
 *
 * Name: dimm_reset_slave
 *
 * Description:  Start reset of I2C slave
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */

void dimm_reset_slave(ipc_msg_t* cmd, void* arg)
{
    int      rc;
    uint32_t scomAddr;
    uint64_t regValue; // a pointer to hold the putscom_abs register value
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    dimm_sm_args_t *args = (dimm_sm_args_t*)async_cmd->cmd_data;

    // Write I2C mode register with the speed/port
    scomAddr = I2C_MODE_REG | SCOM_ENGINE_OFFSET(args->i2cEngine);
    // 0-15: Bit Rate Divisor - 0x0049 gives approx 391kHz (and allows margin for clock variation)
    // 16-21: Port Number (0-5)
    // 22-26: reserved (0s)
    regValue = 0x0049000000000000;
    if ((args->i2cPort > 0) && (args->i2cPort < 6))
    {
        regValue |= ((uint64_t)args->i2cPort << 42);
    }
    rc = putscom_abs(scomAddr, regValue);
    if(rc)
    {
        PK_TRACE("dimm_reset_slave: I2C_MODE_REG putscom 0x%08X->0x%08X%08X FAILED. rc = 0x%08x",
                 scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue), rc);
        gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_SCOM_PUT_FAILED, rc);
    }
    else
    {
        GPE1_DIMM_DBG("dimm_reset_slave: putscom(0x%08X,0x%08X%08X) SUCCESS - MODE",
                      scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue));

        // Force a stop-condition to the I2C slave
        scomAddr = I2C_COMMAND_REG | SCOM_ENGINE_OFFSET(args->i2cEngine);
        // stop bit
        regValue = 0x1000000000000000;
        rc = putscom_abs(scomAddr, regValue);
        if(rc)
        {
            PK_TRACE("dimm_reset_slave: I2C_COMMAND_REG putscom 0x%08X->0x%08X%08X FAILED. rc = 0x%08x",
                     scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue), rc);
            gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_SCOM_PUT_FAILED, rc);
        }
        else
        {
            GPE1_DIMM_DBG("dimm_reset_slave: putscom(0x%08X,0x%08X%08X) SUCCESS - COMMAND (stop)",
                          scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue));
        }
    }

} // end dimm_reset_slave()


/*
 * Function Specifications:
 *
 * Name: dimm_reset_slave_status
 *
 * Description:  Read I2C status register to ensure slave has been reset
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */
void dimm_reset_slave_status(ipc_msg_t* cmd, void* arg)
{
    int      rc;
    uint32_t scomAddr;
    uint64_t regValue; // a pointer to hold the putscom_abs register value
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    dimm_sm_args_t *args = (dimm_sm_args_t*)async_cmd->cmd_data;

    // Read I2C status register
    scomAddr = I2C_STATUS_REG | SCOM_ENGINE_OFFSET(args->i2cEngine);
    rc = getscom_abs(scomAddr, &regValue);
    if(rc)
    {
        PK_TRACE("dimm_reset_slave_status: I2C_STATUS_REG getscom 0x%08X FAILED. rc = 0x%08x - STATUS #1",
                 scomAddr, rc);
        gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_SCOM_GET_FAILED, rc);
    }
    else
    {
        GPE1_DIMM_DBG("dimm_reset_slave_status: getscom(0x%08X) returned 0x%08X%08X) - STATUS #1",
                      scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue));

        // Wait until command complete is set
        if (regValue & STATUS_COMPLETE_MASK)
        {
            // Reset completed
            args->error.rc = GPE_RC_SUCCESS;
            PK_TRACE("dimm_reset_slave_status: I2C Slave Port %d Reset completed", args->i2cPort);

            if ((regValue & STATUS_ERROR_OR_COMPLETE_MASK) != STATUS_COMPLETE_MASK)
            {
                // I2C errors found
                PK_TRACE("dimm_reset_slave_status: I2C errors in status register: %08X%08X",
                         WORD_HIGH(regValue), WORD_LOW(regValue));
                // Continue with other slave resets and may trigger another reset...
            }
        }
        else
        {
            // reset not complete yet...
            args->error.rc = GPE_RC_NOT_COMPLETE;
        }
    }

} // end dimm_reset_slave_status()


