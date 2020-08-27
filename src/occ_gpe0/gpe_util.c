/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/gpe_util.c $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2020                        */
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

//#define DEBUG_APSS_SEQ

#include "pk.h"
#include "ppe42_scom.h"
#include "pss_constants.h"
#include "gpe_util.h"
#include "gpe_export.h"

#define SPIPSS_P2S_ONGOING_MASK 0x8000000000000000

extern gpe_shared_data_t * G_gpe_shared_data;
#ifdef DEBUG_APSS_SEQ
extern uint32_t g_apss_trace_count;
#endif

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

int wait_spi_completion(GpeErrorStruct *error, uint32_t reg, uint32_t i_timeout)
{
    int         i = 0;
    int         rc = 0;
    uint64_t    status = 0;
    uint32_t    wait_time = 0;
    uint32_t    num_reads = 0;

    if((reg != SPIPSS_P2S_STATUS_REG) && (reg != SPIPSS_ADC_STATUS_REG))
    {
        PK_TRACE("gpe0:wait_spi_completion failed: Invalid Register 0x%08x", reg);
        rc = GPE_RC_INVALID_REG;
        gpe_set_ffdc(error, reg, rc, 0x00);
    }
    else
    {
        // Read the P2S_ONGOING bits every 10us for i_timeout, if i_timeout is less than 10
        // just wait i_timeout and check once
        if(i_timeout >= 10)
        {
            wait_time = 10;
            num_reads = i_timeout / 10;
        }
        else
        {
             wait_time = i_timeout;
             num_reads = 1;
        }

#ifdef DEBUG_APSS_SEQ
        if (g_apss_trace_count < APSS_MAX_FAIL_TRACE)
            PK_TRACE("wait_spi_completion: calling busy_wait(%d) for %d reads",wait_time, num_reads);
#endif
        for (i = 0; i< num_reads; i++)
        {
            busy_wait(wait_time);

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
        }
    }

    // Check if timed out waiting on P2S_ONGOING / HWCTRL_ONGOING bit
    if (i >= num_reads)
    {
        PK_TRACE("wait_spi_completion: Timed out waiting for p2s_ongoing to clear. (%d reads)",
                 num_reads);
        rc = GPE_RC_SPI_TIMEOUT;
        gpe_set_ffdc(error, reg, GPE_RC_SPI_TIMEOUT, rc);
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
 * Inputs:       i_microseconds: time to sleep in microseconds
 *
 * Note:         i_microseconds must be < (2^16)/occ_freq microseconds
 *                  about 114 seconds for the fastest occ_freq supported.
 * return:       none
 *
 * End Function Specification
 */
void busy_wait(uint32_t i_microseconds)
{
    uint32_t current_count = pk_timebase32_get();
    uint32_t start_count = current_count;
    uint32_t change_timeout = 0;
    uint32_t end_count = PK_INTERVAL_SCALE((uint32_t)PK_MICROSECONDS(i_microseconds));
    static bool L_traced = false;

    while((current_count - start_count) < end_count)
    {
        uint32_t prev_count = current_count;

        current_count = pk_timebase32_get();

        if(prev_count == current_count)
        {
            ++change_timeout;
            if(change_timeout > 32)
            {
                if (!L_traced)
                {
                    PK_TRACE("busy_wait: TIMEBASE is not moving! (current_count=%d)");
                    L_traced = true;
                }
                // timebase is not moving
                break;
            }
        }
        else
        {
            if (L_traced)
            {
                PK_TRACE("busy_wait: TIMEBASE is moving again! (current_count=%d, prev_count=%d, change_timeout=%d)", current_count, prev_count, change_timeout);
                L_traced = false;
            }
            change_timeout = 0;
        }
    }
}

/*
 * Function Specification
 *
 * Name: ipc_scom_operation
 *
 * Description: Does a getscom or putscom for the 405
 *
 * End Function Specification
 */
void ipc_scom_operation(ipc_msg_t* cmd, void* arg)
{
    int l_rc;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    ipc_scom_op_t *scom_op = (ipc_scom_op_t*) async_cmd->cmd_data;

    if (scom_op->read)
    {
        l_rc = getscom_abs(scom_op->addr, &scom_op->data);
        if( l_rc )
        {
            PK_TRACE("ipc_scom_operation: Error doing getscom! rc: 0x%08X addr: 0x%08X", l_rc, scom_op->addr);
            gpe_set_ffdc(&(scom_op->error), scom_op->addr, GPE_RC_SCOM_GET_FAILED, l_rc);
        }
    }
    else
    {
        l_rc = putscom_abs(scom_op->addr, scom_op->data);
        if( l_rc )
        {
            PK_TRACE("ipc_scom_operation: Error doing putscom! rc: 0x%08X addr: 0x%08X", l_rc, scom_op->addr);
            gpe_set_ffdc(&(scom_op->error), scom_op->addr, GPE_RC_SCOM_PUT_FAILED, l_rc);
        }
    }

    // send back a response, IPC success even if ffdc/rc are non zeros
    l_rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
    if(l_rc)
    {
        PK_TRACE("ipc_scom_operation: Failed to send response back. Halting GPE0", l_rc);
        gpe_set_ffdc(&(scom_op->error), 0x00, GPE_RC_IPC_SEND_FAILED, l_rc);
        pk_halt();
    }
}


/*
 * Function Specification:
 *
 * Name: gpe0_nop
 *
 * Description: a function that does nothing. Called to measure IPC timing
 *
 * Inputs:      none
 *
 * return:      none
 *
 * End Function Specification
 */

void gpe0_nop(ipc_msg_t* cmd, void* arg)
{
    int rc;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    nop_t *args = (nop_t*)async_cmd->cmd_data;

    // send back a response, IPC success even if ffdc/rc are non zeros
    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("gpe0_nop: Failed to send response back. Halting GPE0", rc);
        gpe_set_ffdc(&(args->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }
}
