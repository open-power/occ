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

    static bool L_init_complete[NUM_NIMBUS_MCAS] = {FALSE};
    int      rc = 0;
    uint64_t regValue; // a pointer to hold get/put SCOM register value
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    reset_mem_deadman_args_t *args = (reset_mem_deadman_args_t*)async_cmd->cmd_data;

    args->error.error = 0;
    args->error.ffdc = 0;

    do
    {
        int mca = args->mca; // Nimbus MCA; mc_pair = mca >>2 and port = mca & 3

        // Part of init is to enable the deadman timer for this MCA
        if(!L_init_complete[mca])
        {
            // Read STR Register 0
            rc = getscom_abs(STR_REG0_MCA(mca), &regValue);

            if(rc)
            {
                PK_TRACE("gpe_reset_mem_deadman: Failed to read STR0 to program deadman timer"
                         " MCA:0x%08x, Address:0x%08x, rc:0x%08x", mca, STR_REG0_MCA(mca), rc);

                gpe_set_ffdc(&(args->error), STR_REG0_MCA(mca),
                             GPE_RC_SCOM_GET_FAILED, rc);
                break;
            }

            // set the deadman timer to the max value: bits 57:60 = 0b1000
            regValue |= 0x0000000000000040;
            regValue &= ~(0x0000000000000038);

            // Write Modified STR Register 0
            rc = putscom_abs(STR_REG0_MCA(mca), regValue);

            if(rc)
            {
                PK_TRACE("gpe_reset_mem_deadman: Failed to program deadman timer"
                         " MCA:0x%08x, Data:0x%08x, rc:0x%08x", mca, (uint32_t)regValue, rc);

                gpe_set_ffdc(&(args->error), STR_REG0_MCA(mca),
                             GPE_RC_SCOM_PUT_FAILED, rc);
                break;
            }
        }

        // read Deadman timer's SCOM Register for specified MCA to reset the timer
        rc = getscom_abs(DEADMAN_TIMER_MCA(mca), &regValue);
        if(rc)
        {
            PK_TRACE("gpe_reset_mem_deadman: Deadman timer read failed"
                     " MCA:0x%08x, Address:0x%08x, rc:0x%08x",
                     mca, DEADMAN_TIMER_MCA(mca), rc);

            gpe_set_ffdc(&(args->error), DEADMAN_TIMER_MCA(mca),
                         GPE_RC_SCOM_GET_FAILED, rc);
            break;
        }

        // Now that we are poking the deadman timer as second part of init check for and clear
        // any previous emergency throttle that may have happened from the last time OCC was running
        if(!L_init_complete[mca])
        {
            L_init_complete[mca] = TRUE; // Done handling initialization

            // Read Emergency Throttle Register
            rc = getscom_abs(ER_THROTTLE_MCA(mca), &regValue);

            if(rc)
            {
                PK_TRACE("gpe_reset_mem_deadman: Failed to read emergency throttle register"
                         " MCA:0x%08x, Address:0x%08x, rc:0x%08x", mca, ER_THROTTLE_MCA(mca), rc);

                gpe_set_ffdc(&(args->error), ER_THROTTLE_MCA(mca), GPE_RC_SCOM_GET_FAILED, rc);
                break;
            }

            // clear Emergency Throttle In-Progress bit if set, this is indication that OCC has been
            // re-started from permanent safe mode without an IPL 
            if(regValue & ER_THROTTLE_IN_PROGRESS_MASK)
            {
                PK_TRACE("gpe_reset_mem_deadman: Enabled timer and clearing throttle for MCA:0x%08x", mca);
                regValue &= ~(ER_THROTTLE_IN_PROGRESS_MASK);

                // Write Modified Emergency Throttle Register
                rc = putscom_abs(ER_THROTTLE_MCA(mca), regValue);

                if(rc)
                {
                    PK_TRACE("gpe_reset_mem_deadman: Failed to clear emergency throttle"
                             " MCA:0x%08x, rc:0x%08x", mca, rc);

                    gpe_set_ffdc(&(args->error), ER_THROTTLE_MCA(mca), GPE_RC_SCOM_PUT_FAILED, rc);
                    break;
                }

                // Clear the emergency throttle engaged FIR bit
                rc = getscom_abs(MCA_CAL_FIR_REG_MCA(mca), &regValue);

                if(rc)
                {
                    PK_TRACE("gpe_reset_mem_deadman: Failed to read MCA FIR register"
                             " MCA:0x%08x, Address:0x%08x, rc:0x%08x",
                             mca, MCA_CAL_FIR_REG_MCA(mca), rc);

                    gpe_set_ffdc(&(args->error), MCA_CAL_FIR_REG_MCA(mca), GPE_RC_SCOM_GET_FAILED, rc);
                    break;
                }
                regValue &= ~(MCA_FIR_THROTTLE_ENGAGED_MASK);

                // Write Modified MCA FIR Register
                rc = putscom_abs(MCA_CAL_FIR_REG_MCA(mca), regValue);

                if(rc)
                {
                    PK_TRACE("gpe_reset_mem_deadman: Failed to clear emergency throttle FIR bit"
                             " MCA:0x%08x, rc:0x%08x", mca, rc);

                    gpe_set_ffdc(&(args->error), MCA_CAL_FIR_REG_MCA(mca), GPE_RC_SCOM_PUT_FAILED, rc);
                    break;
                }
            }
            else
            {
                PK_TRACE("gpe_reset_mem_deadman: Enabled timer for MCA:0x%08x", mca);
            }
        }  // if !L_init_complete

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
