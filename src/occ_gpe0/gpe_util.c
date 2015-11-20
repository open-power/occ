/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/gpe_util.c $                                     */
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

#include "pk.h"
#include "ppe42_scom.h"
#include "pss_constants.h"
#include "gpe_util.h"
#define SPIPSS_P2S_ONGOING_MASK 0x8000000000000000

/*
 * Function Specification
 *
 * Name: gpe_set_ffdc
 *
 * Description: Fills up the error struct with the given data.
 *
 * End Function Specification
 */
void gpe_set_ffdc(GpeErrorStruct *o_error, uint32_t i_addr, uint32_t i_rc, uint64_t i_ffdc)
{

    o_error->addr = i_addr;
    //Return codes defined in apss_struct.h
    o_error->rc = i_rc;
    o_error->ffdc = i_ffdc;

}

/*
 * Function Specification:
 *
 * Name: wait_spi_completion
 *
 * Description:  Read the specified register (SPIPSS_P2S_STATUS_REG
 *               or SPIPSS_ADC_STATUS_REG), and check if it's p2s_ongoing
 *               bit is 0 (operations done).  If not, wait
 *               up to the timeout usec (~1usec per retry).
 *               If still not clear, continue looping,
 *               If error/reserved bits are set, a return code will be sent back
 *
 * Inputs:       error:    a pointer to a GpeErrorStruct, to populate rc, ffdc,
 *                         and address, in case a scom get error happens
 *               timeout:  # usec to wait for ongoing bit to clear
 *               Register: SPIPSS_P2S_STATUS_REG or SPIPSS_ADC_STATUS_REG
 *
 * return:       0 -> Success: spi transaction completed within timeout limit
 *               not 0 -> timeout: spi transaction did not complete within timeout
 *                        bits 0:31 are masked, and returned back for potential
 *                        analysis of the reason that the transaction timed out
 *
 * End Function Specification
 */

int wait_spi_completion(GpeErrorStruct *error, uint32_t reg, uint8_t timeout)
{
    int         i = 0;
    int         rc;
    uint64_t    status;

    if((reg != SPIPSS_P2S_STATUS_REG) && (reg != SPIPSS_ADC_STATUS_REG))
    {
        PK_TRACE("gpe0:wait_spi_completion failed: Invalid Register 0x%08x", reg);
        rc = GPE_RC_INVALID_REG;
        gpe_set_ffdc(error, reg, rc, 0x00);
    }
    else
    {
        // Keep polling the P2S_ONGOING bits for timeout
        for (i = 0; i< timeout; i++)
        {
            rc = getscom_abs(reg, &status);
            if(rc)
            {
                PK_TRACE("gpe0:wait_spi_completion failed with rc = 0x%08x", rc);
                gpe_set_ffdc(error, reg, GPE_RC_SCOM_GET_FAILED, rc);
                rc = GPE_RC_SCOM_GET_FAILED;
                break;
            }


            // bit zero is the P2s_ONGOING / HWCTRL_ONGOING
            // set to 1: means operation is in progress (ONGOING)
            // set to 0: means operation is complete, therefore exit for loop.
            if((status & SPIPSS_P2S_ONGOING_MASK) == 0)
            {
                rc = 0;
                break;
            }

            // sleep for 1 microsecond before retry
            busy_wait(1);
        }
    }

    //Timed out waiting on P2S_ONGOING / HWCTRL_ONGOING bit.
    if (i >= timeout)
    {
        PK_TRACE("gpe0:wait_spi_completion Timed out waiting for p2s_ongoing to clear.");
        gpe_set_ffdc(error, reg, GPE_RC_SPI_TIMEOUT, rc);
        rc = GPE_RC_SPI_TIMEOUT;
    }

    return rc;
}

/*
 * Function Specification:
 *
 * Name: busy_wait
 *
 * Description:  a counting loop to simulate sleep calls, and is ISR safe.
 *
 * Inputs:       t_microseconds: time to sleep in microseconds
 *
 * return:       none
 *
 * End Function Specification
 */


void busy_wait(uint32_t t_microseconds)
{
    uint32_t start_decrementer_value;      // The decrementer register value at the beginning
    uint32_t end_decrementer_value;        // The decrementer register value at the end
    uint32_t current_decrementer_value;    // The current decrementer register value
    uint32_t duration;
    MFDEC(start_decrementer_value);        // get the decrementer register value at the beginning
    current_decrementer_value = start_decrementer_value;

    // multiply the delay time by the 37.5 MHz external clock frequency.
    // I believe that the ppe42_mullw macro works fine, but the
    // compiler parameter settings seems broken.
    // @todo: This is a temporary fix, use external frequency directive.
    duration =  (t_microseconds << 5)
        + (t_microseconds << 2)
        + (t_microseconds)
        + (t_microseconds >> 1);

    // Calculate the decrementer register value at the end of the busy wait period
    end_decrementer_value = start_decrementer_value - duration;

    if(start_decrementer_value < end_decrementer_value);          // decrementer overflows during the busy wait?
    {
        MFDEC(current_decrementer_value);
        while(current_decrementer_value < end_decrementer_value)  // Wait until Decrementer overflows
            MFDEC(current_decrementer_value);
    }

    while (current_decrementer_value > end_decrementer_value)     // Wait until end_decrementer_value is reached
        MFDEC(current_decrementer_value);
}

void ipc_scom_operation(ipc_msg_t* cmd, void* arg)
{
    int l_rc;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    ipc_scom_op_t *scom_op = (ipc_scom_op_t*) async_cmd->cmd_data;

    if (scom_op->read)
    {
        l_rc = getscom_abs(scom_op->addr, &scom_op->data);
    }
    else
    {
        l_rc = putscom_abs(scom_op->addr, scom_op->data);
    }

    if(l_rc)
    {
        scom_op->rc = l_rc;
        PK_TRACE("Error doing generic scom! RC: 0x%08X Addr: 0x%08X Read: %d Data: 0x%08X", l_rc,
                 scom_op->addr, scom_op->read, scom_op->data);
    }
}
