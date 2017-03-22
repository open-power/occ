/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe1_dimm_control.c $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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
#include "ipc_api.h"
#include "ipc_async_cmd.h"
#include "gpe_err.h"
#include "gpe_util.h"
#include "dimm_structs.h"
#include "mca_addresses.h"
#include "gpe1.h"

/*
 * Function Specifications:
 *
 * Name: gpe_dimm_control
 *
 * Description:  DIMM control code on the GPE
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */

void gpe_dimm_control(ipc_msg_t* cmd, void* arg)
{
    // Note: arg was set to 0 in ipc func table (ipc_func_tables.c), so don't use it.
    // the ipc arguments passed through the ipc_msg_t structure, has a pointer
    // to the dimm_control_args_t struct.

    int      rc;
    uint64_t regValue; // a pointer to hold the putscom_abs register value
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    dimm_control_args_t *args = (dimm_control_args_t*)async_cmd->cmd_data;

    int mc = args->mc, port = args->port; // memory controller pair and port

    args->error.error = 0;
    args->error.ffdc = 0;

    do
    {   // read N/M DIMM Throttling Control SCOM Register for specified MC pair and port numbers
        rc = getscom_abs(N_M_DIMM_TCR(mc,port), &regValue);
        if(rc)
        {
            PK_TRACE("gpe_dimm_control: N/M DIMM Throttling Control Register read fails"
                     "MC#|Port:0x%08x, Address:0x%08x, rc:0x%08x",
                     (uint32_t)((mc << 16) | port),
                     N_M_DIMM_TCR(mc,port), rc);

            gpe_set_ffdc(&(args->error), N_M_DIMM_TCR(mc,port),
                         GPE_RC_SCOM_GET_FAILED, rc);
            break;
        }

        // Clear old N values for slot and port
        regValue &= 0x1FFFFFFFF;
        // copy slot and port N values (31 bits) from passed dimmNumeratorValues
        regValue |=  ((uint64_t) (args->dimmNumeratorValues.word32 & 0xFFFFFFFE)) << 32 ;

        // Write new slot and port N Values
        rc = putscom_abs(N_M_DIMM_TCR(mc,port), regValue);
        if(rc)
        {
            PK_TRACE("gpe_dimm_control: N/M DIMM Throttling Control Register write fails"
                     "dimm#:%d, Address:0x%04x, Nvalues:0x%08x, rc:0x%08x",
                     (uint16_t)((mc << 8) | port),
                     N_M_DIMM_TCR(mc,port), regValue, rc);

            gpe_set_ffdc(&(args->error), N_M_DIMM_TCR(mc,port),
                         GPE_RC_SCOM_PUT_FAILED, rc);
            break;
        }
        else
        {
            GPE1_DIMM_DBG("gpe_dimm_control: N/M DIMM Throttling Control Register write"
                          "mc|port#:0x%04x, Address:0x%08x, Nvalues:0x%08x",
                          (uint16_t)((mc << 8) | port),
                          N_M_DIMM_TCR(mc,port), regValue);
        }
    } while(0);

    // send back a response, IPC success even if ffdc/rc are non zeros
    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("gpe_dimm_control: Failed to send response back. Halting GPE1", rc);
        gpe_set_ffdc(&(args->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }
}

/*
 * Function Specifications:
 *
 * Name: gpe_reset_mem_deadman
 *
 * Description:  Read memory deadman timer for one MCA
 *               This effectively resets the memory deadman timer
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */

void gpe_reset_mem_deadman(ipc_msg_t* cmd, void* arg)
{
    // Note: arg was set to 0 in ipc func table (ipc_func_tables.c), so don't use it.
    // the ipc arguments passed through the ipc_msg_t structure, has a pointer
    // to the reset_mem_deadman_args_t struct.

    int      rc = 0;
    // @TODO: uncomment when deadman timer scom registers are definied in simics. RTC: 163713
    //uint64_t regValue; // a pointer to hold the putscom_abs register value
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    reset_mem_deadman_args_t *args = (reset_mem_deadman_args_t*)async_cmd->cmd_data;

    args->error.error = 0;
    args->error.ffdc = 0;

    do
    {   // read Deadman timer's SCOM Register for specified MCA (MC pair and port numbers)
        // @TODO: uncomment when deadman timer scom registers are definied in simics. RTC: 163713, RTC: 163934
#if 0
        int mca = args->mca; // Nimbus MCA; mc_pair = mca >>2 and port = mca & 3

        rc = getscom_abs(DEADMAN_TIMER_MCA(mca), &regValue);
        if(rc)
        {
            PK_TRACE("gpe_reset_mem_deadman: Deadman timer read failed"
                     "MCA:0x%08x, Address:0x%08x, rc:0x%08x",
                     mca, DEADMAN_TIMER_MCA(mca), rc);

            gpe_set_ffdc(&(args->error), DEADMAN_TIMER_MCA(mca),
                         GPE_RC_SCOM_GET_FAILED, rc);
            break;
        }
        else
        {
            GPE1_DIMM_DBG("gpe_reset_mem_deadman: Deadman timer reset successfully"
                     "MCA:0x%08x, Address:0x%08x, deadman value:0x%08x",
                     mca, DEADMAN_TIMER_MCA(mca), regValue);

        }
#endif
    } while(0);

    // send back a response, IPC success even if ffdc/rc are non zeros
    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("gpe_reset_mem_deadman: Failed to send response back. Halting GPE1", rc);
        gpe_set_ffdc(&(args->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }
}
