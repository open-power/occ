/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe1_dimm_read.c $                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2024                        */
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

/// \file gpe1_dimm_read.c
/// \brief Functions to handle reading the DIMM temperatures
///

//#define GPE1_DEBUG
#include "pk.h"
#include "ipc_api.h"
#include "ppe42_scom.h"
#include "ipc_async_cmd.h"
#include "gpe1.h"
#include "gpe1_dimm.h"
#include "dimm_structs.h"
#include "i2c.h"

void dimm_write_int_mask(ipc_msg_t* cmd, void* arg);
void dimm_write_mode(ipc_msg_t* cmd, void* arg);
void dimm_write_ts_addr(ipc_msg_t* cmd, void* arg);
void dimm_initiate_read(ipc_msg_t* cmd, void* arg);
void dimm_read_temp(ipc_msg_t* cmd, void* arg);
// from gpe1_dimm_reset.c
void dimm_reset_master(ipc_msg_t* cmd, void* arg);
void dimm_reset_slave(ipc_msg_t* cmd, void* arg);
void dimm_reset_slave_status(ipc_msg_t* cmd, void* arg);

// To test in cronus: (read engine 3 port 8 address 0x30 2 byte temp sensor)
//   geti2c pu 3 8 30 2 5 1 -busspeed 400 -debug5.1i.f

/*
 * Function Specifications:
 *
 * Name: gpe_dimm_sm
 *
 * Description:  DIMM I2C State Machine handler in the GPE
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */
void gpe_dimm_sm(ipc_msg_t* cmd, void* arg)
{
    // Note: arg was set to 0 in ipc func table (ipc_func_tables.c), so don't use it.
    // the ipc arguments passed through the ipc_msg_t structure, has a pointer
    // to the G_gpe_start_pwr_meas_read_args struct.
    int      rc;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    dimm_sm_args_t *args = (dimm_sm_args_t*)async_cmd->cmd_data;

    // clear error
    args->error.error = 0;
    args->error.ffdc = 0;

    // Need to mask machine checks for I2C master:
    //
    // Turn off MCR bit will prevent machine check on error on scom readings (MSR bits 1:7)
    //   rc == 1 resource occupied  (see ppe42_scom.h)   Action: return with rc
    //   rc == 2 Core is fenced, offline                 Action: return with rc
    //   rc == 3 partial good
    //   rc == 4 address error
    //   rc == 5 clock error
    //   rc == 6 packet error
    //   rc == 7 timeout
    //
    // Clear: last SIB rc (SIBRC) and rc accumulator (SIMBRCA)
    // Set: SIB Error Mask to prevent machine checks for any rcs
    const uint32_t orig_msr = mfmsr();
    const uint32_t msr = (orig_msr & ~(MSR_SIBRC | MSR_SIBRCA)) | MSR_SEM;
    mtmsr(msr);

    switch(args->state)
    {
        case DIMM_STATE_INIT:
            dimm_write_int_mask(cmd, arg);
            break;

            // Read DIMM Temperature States

        case DIMM_STATE_WRITE_MODE:
            dimm_write_mode(cmd, arg);
            break;

        case DIMM_STATE_WRITE_ADDR:
            dimm_write_ts_addr(cmd, arg);
            break;

        case DIMM_STATE_INITIATE_READ:
            dimm_initiate_read(cmd, arg);
            break;

        case DIMM_STATE_READ_TEMP:
            dimm_read_temp(cmd, arg);
            break;

            // I2C Reset States

        case DIMM_STATE_RESET_MASTER:
            dimm_reset_master(cmd, arg);
            break;

        case DIMM_STATE_RESET_SLAVE_PORT:
            dimm_reset_slave(cmd, arg);
            break;

        case DIMM_STATE_RESET_SLAVE_PORT_COMPLETE:
            dimm_reset_slave_status(cmd, arg);
            break;

        default:
            PK_TRACE("E>gpe_dimm_sm: Invalid state (0x%02X) received!", args->state);
            args->error.rc = 0;
            gpe_set_ffdc(&(args->error), 0, GPE_RC_INVALID_STATE, args->state);
            break;
    }

    // Send back IPC response of success (IPC operation itself succeeded)
    // (if any operation failed, the error (rc/ffdc) will be non-zero
    //  and can be handled by the 405)
    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("E>gpe_dimm_sm: Failed to send response back. Halting GPE1", rc);
        gpe_set_ffdc(&(args->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }

    // Restore original MSR and clear SIBRC and SIBRCA
    mtmsr(orig_msr & ~(MSR_SIBRC | MSR_SIBRCA));

} // end gpe_dimm_sm()


/*
 * Function Specifications:
 *
 * Name: dimm_write_int_mask
 *
 * Description:  Write the I2C interrupt mask and read I2C status register
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */
void dimm_write_int_mask(ipc_msg_t* cmd, void* arg)
{
    int      rc;
    uint32_t scomAddr;
    uint64_t regValue; // a pointer to hold the putscom_abs register value
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    dimm_sm_args_t *args = (dimm_sm_args_t*)async_cmd->cmd_data;

    scomAddr = I2C_INTERRUPT_MASK_REG | SCOM_ENGINE_OFFSET(args->i2cEngine);
    // Enable the following bit in the interrupt mask:
    // invalid command, LBUS parity, back end overrun, back end access,
    // arbitration lost, nack received, data request, command complete,
    // stop, i2c busy
    regValue = 0x0000FFC000000000;
    rc = putscom_abs(scomAddr, regValue);
    if(rc)
    {
        PK_TRACE("E>dimm_write_int_mask: I2C_INTERRUPT_MASK_REG putscom 0x%08X->0x%08X%08X FAILED. rc = 0x%08x",
                 scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue), rc);
        gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_SCOM_PUT_FAILED, rc);
    }
    else
    {
        GPE1_DIMM_DBG("dimm_write_int_mask: putscom(0x%08X,0x%08X%08X) SUCCESS - INT MASK",
                      scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue));
    }

    // READ I2C STATUS REGISTER
    scomAddr = I2C_STATUS_REG | SCOM_ENGINE_OFFSET(args->i2cEngine);
    rc = getscom_abs(scomAddr, &regValue);
    if(rc)
    {
        PK_TRACE("E>dimm_write_int_mask: I2C_STATUS_REG getscom 0x%08X FAILED. rc = 0x%08x - STATUS",
                 scomAddr, rc);
        gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_SCOM_GET_FAILED, rc);
    }
    else
    {
        GPE1_DIMM_DBG("dimm_write_int_mask: getscom(0x%08X) returned 0x%08X%08X) - STATUS #1",
                      scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue));
        // max_num_of_ports (bits 9:15)
        args->maxPorts = (regValue >> 48) & 0x7F;
        GPE1_DIMM_DBG("dimm_write_int_mask: maxPorts = %d", args->maxPorts);
    }

} // end dimm_write_int_mask()


/*
 * Function Specifications:
 *
 * Name: dimm_write_mode
 *
 * Description:  Write the I2C mode register (set speed and port)
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */

void dimm_write_mode(ipc_msg_t* cmd, void* arg)
{
    // Note: arg was set to 0 in ipc func table (ipc_func_tables.c), so don't use it.
    // the ipc arguments passed through the ipc_msg_t structure, has a pointer
    // to the G_gpe_start_pwr_meas_read_args struct.

    int      rc;
    uint32_t scomAddr;
    uint64_t regValue; // a pointer to hold the putscom_abs register value
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    dimm_sm_args_t *args = (dimm_sm_args_t*)async_cmd->cmd_data;

    // MODE_REGISTER
    scomAddr = I2C_MODE_REG | SCOM_ENGINE_OFFSET(args->i2cEngine);
    // 0-15: Bit Rate Divisor - 0x00AE gives approx 400kHz (and allows margin for clock variation)
    // 16-21: Port Number (0-15)
    // 22-26: reserved (0s)
    // 28: fgat mode. Must be set to allow stop only with secure boot enabled
    regValue = I2C_MODE_REG_DIVISOR;
    if ((args->i2cPort > 0) && (args->i2cPort < 16))
    {
        regValue |= ((uint64_t)args->i2cPort << 42);
    }
    GPE1_DIMM_DBG("dimm_write_mode: I2C_MODE_REG ---> OCMB%d / port %d / divisor 0x%04X",
                  args->ocmb, args->i2cPort, I2C_MODE_REG_DIVISOR);
    rc = putscom_abs(scomAddr, regValue);
    if(rc)
    {
        PK_TRACE("E>dimm_write_mode: I2C_MODE_REG putscom 0x%08X->0x%08X%08X FAILED. rc = 0x%08x",
                 scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue), rc);
        gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_SCOM_PUT_FAILED, rc);
    }
    else
    {
        GPE1_DIMM_DBG("dimm_write_mode: putscom(0x%08X,0x%08X%08X) SUCCESS - MODE (OCMB%d)",
                      scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue), args->ocmb);
    }

} // end dimm_write_mode()


/*
 * Function Specifications:
 *
 * Name: dimm_write_ts_addr
 *
 * Description:  Write the Temperature Sensor address to I2C
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */
void dimm_write_ts_addr(ipc_msg_t* cmd, void* arg)
{
    int      rc;
    uint32_t scomAddr;
    uint64_t regValue; // a pointer to hold the putscom_abs register value
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    dimm_sm_args_t *args = (dimm_sm_args_t*)async_cmd->cmd_data;

    // Write I2C command register with a 1 byte write
    scomAddr = I2C_COMMAND_REG | SCOM_ENGINE_OFFSET(args->i2cEngine);
    // start+address + slave_address, rw=0=write, length=1, i2c address
    regValue = 0xC000000100000000 | ((uint64_t)args->i2cAddr << 48);
    rc = putscom_abs(scomAddr, regValue);
    if(rc)
    {
        PK_TRACE("E>dimm_write_ts_addr: I2C_COMMAND_REG putscom 0x%08X->0x%08X%08X FAILED. rc = 0x%08x",
                 scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue), rc);
        gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_SCOM_PUT_FAILED, rc);
    }
    else
    {
        GPE1_DIMM_DBG("dimm_write_ts_addr: putscom(0x%08X,0x%08X%08X) SUCCESS - COMMAND (write)",
                      scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue));

        // Write the Temperature Sensor address to the FIFO register
        scomAddr = I2C_FIFO1_REG_READ | SCOM_ENGINE_OFFSET(args->i2cEngine);
        // 0x05 = temperature value
        regValue  = 0x0500000000000000;
        rc = putscom_abs(scomAddr, regValue);
        if(rc)
        {
            PK_TRACE("E>dimm_write_ts_addr: I2C_FIFO_REG putscom 0x%08X->0x%08X%08X FAILED. rc = 0x%08x",
                     scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue), rc);
            gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_SCOM_PUT_FAILED, rc);
        }
        else
        {
            GPE1_DIMM_DBG("dimm_write_ts_addr: putscom(0x%08X,0x%08X%08X) SUCCESS - FIFO",
                          scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue));
        }
    }

} // end dimm_write_ts_addr()


/*
 * Function Specifications:
 *
 * Name: dimm_initiate_read
 *
 * Description:  Initiate the read of the temperature sensor
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */
void dimm_initiate_read(ipc_msg_t* cmd, void* arg)
{
    int      rc;
    uint32_t scomAddr;
    uint64_t regValue; // a pointer to hold the putscom_abs register value
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    dimm_sm_args_t *args = (dimm_sm_args_t*)async_cmd->cmd_data;

    // Default the rc to NOT_COMPLETE (meaning the read was not initiated)
    args->error.rc = GPE_RC_NOT_COMPLETE;

    // Read the I2c Status Register to ensure the TS address was written successfully
    scomAddr = I2C_STATUS_REG | SCOM_ENGINE_OFFSET(args->i2cEngine);
    rc = getscom_abs(scomAddr, &regValue);
    if(rc)
    {
        PK_TRACE("E>dimm_initiate_read: I2C_STATUS_REG getscom 0x%08X FAILED. rc = 0x%08x",
                 scomAddr, rc);
        gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_SCOM_GET_FAILED, rc);
    }
    else
    {
        GPE1_DIMM_DBG("dimm_initiate_read: getscom(0x%08X) returned 0x%08X%08X - STATUS",
                      scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue));

        if ((regValue & STATUS_ERROR_OR_COMPLETE_MASK) == STATUS_COMPLETE_MASK)
        {
            // Status register indicates no errors and last command completed.
            // Write the I2C command register with a 2 byte read request.
            // Since FIFO4 can read 4 bytes in one operation, we will do a read of 4 bytes
            // and only look at first 2 bytes.  (FIFO4 will hang if only try to read 2 bytes)
            scomAddr = I2C_COMMAND_REG | SCOM_ENGINE_OFFSET(args->i2cEngine);
            // start+address+stop + slave_address, rw=1=read, length=4
            regValue  = 0xD001000400000000;
            regValue |= ((uint64_t)args->i2cAddr << 48);
            rc = putscom_abs(scomAddr, regValue);
            if(rc)
            {
                PK_TRACE("E>dimm_initiate_read: I2C_COMMAND_REG putscom 0x%08X->0x%08X%08X FAILED. rc = 0x%08x",
                         scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue), rc);
                gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_SCOM_PUT_FAILED, rc);
            }
            else
            {
                // The read command has been started, return success
                GPE1_DIMM_DBG("dimm_initiate_read: putscom(0x%08X,0x%08X%08X) SUCCESS - COMMAND (read)",
                              scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue));
                args->error.error = 0;
                args->error.rc = GPE_RC_SUCCESS;
                args->error.ffdc = 0;
            }
        }
        else
        {
            if ((regValue & STATUS_ERROR_OR_COMPLETE_MASK) != 0)
            {
                // I2C error was found
                PK_TRACE("E>dimm_initiate_read: Error in status register: 0x%08X%08X",
                         WORD_HIGH(regValue), WORD_LOW(regValue));
                gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_I2C_ERROR, regValue);
            }
            else
            {
                // Last command (write TS address) has not completed yet
                PK_TRACE("dimm_initiate_read: last command not complete yet...");
                gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_NOT_COMPLETE, regValue);
            }
            // else, not complete yet
        }
    }

} // end dimm_initiate_read()


/*
 * Function Specifications:
 *
 * Name: dimm_read_temp
 *
 * Description:  Read the temperature sensor value
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */
void dimm_read_temp(ipc_msg_t* cmd, void* arg)
{
    int      rc;
    uint32_t scomAddr;
    uint64_t regValue; // a pointer to hold the putscom_abs register value
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    dimm_sm_args_t *args = (dimm_sm_args_t*)async_cmd->cmd_data;

    // Default the rc to NOT_COMPLETE (meaning the read has not completed)
    args->error.rc = GPE_RC_NOT_COMPLETE;

    // Read the I2C status register
    scomAddr = I2C_STATUS_REG | SCOM_ENGINE_OFFSET(args->i2cEngine);
    rc = getscom_abs(scomAddr, &regValue);
    if(rc)
    {
        PK_TRACE("E>dimm_read_temp: I2C_STATUS_REG getscom 0x%08X FAILED. rc = 0x%08x - STATUS #1",
                 scomAddr, rc);
        gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_SCOM_GET_FAILED, rc);
    }
    else
    {
        GPE1_DIMM_DBG("dimm_read_temp: getscom(0x%08X) returned 0x%08X%08X) - STATUS #1",
                      scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue));

        uint8_t fifoLength = (regValue >> 32) & 0xFF;
        if ((regValue & STATUS_ERROR_MASK) != 0)
        {
            // I2C error was found
            PK_TRACE("E>dimm_read_temp: Error in status register: 0x%08X%08X",
                     WORD_HIGH(regValue), WORD_LOW(regValue));
            gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_I2C_ERROR, regValue);
        }
        else
        {
            // Need to wait until both bytes are available
            if (fifoLength >= 2)
            {
                // Read the sensor value from the FIFO4 register
                scomAddr = I2C_FIFO4_REG_READ | SCOM_ENGINE_OFFSET(args->i2cEngine);
                rc = getscom_abs(scomAddr, &regValue);
                if(rc)
                {
                    PK_TRACE("E>dimm_read_temp: I2C_FIFO4_REG getscom 0x%08X FAILED. rc = 0x%08x",
                             scomAddr, rc);
                    gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_SCOM_GET_FAILED, rc);
                }
                else
                {
                    GPE1_DIMM_DBG("dimm_read_temp: getscom(0x%08X) returned 0x%08X%08X) - FIFO4",
                                  scomAddr, WORD_HIGH(regValue), WORD_LOW(regValue));
                    // Temperature is in bits 4-11
                    args->temp = (regValue >> 52) & 0xFF;
                    args->error.error = 0;
                    args->error.rc = GPE_RC_SUCCESS;
                    args->error.ffdc = 0;
                    GPE1_DIMM_DBG("dimm_read_temp: DIMM%04X temperature=%dC",
                                  (args->i2cPort<<8)|args->dimm, args->temp);

                    // Check for operation complete bit
                    // (operation complete bit will not get set until all
                    //  data has been read from the FIFO.)
                    if (regValue & PEEK_ERROR_MASK)
                    {
                        // I2C error was found
                        PK_TRACE("E>dimm_read_temp: Error in FIFO4 peek data: 0x%08X%08X",
                                 WORD_HIGH(regValue), WORD_LOW(regValue));
                        gpe_set_ffdc(&(args->error), scomAddr, GPE_RC_I2C_ERROR, regValue);
                    }
                    // PEEK_MORE_DATA will be set because we only read 2 of the 4 bytes (ignore this bit)
                }
            }
            // else, all data not available yet (NOT_COMPLETE)
        }
    }

} // end dimm_read_temp()


