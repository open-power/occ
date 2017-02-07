/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe1_24x7.c $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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
#include "gpe_util.h"
#include "gpe_24x7_structs.h"

/*
 * Function Specifications:
 *
 * Name: gpe_24x7
 *
 * Description:  24x7 code on the GPE.  Owned by the performance team
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */

void gpe_24x7(ipc_msg_t* cmd, void* arg)
{
    // Note: arg was set to 0 in ipc func table (ipc_func_tables.c), so don't use it.
    // the ipc arguments passed through the ipc_msg_t structure, has a pointer
    // to the gpe_24x7_args_t struct.

    int      rc = 0;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    gpe_24x7_args_t *args = (gpe_24x7_args_t*)async_cmd->cmd_data;

    uint8_t ticks = args->numTicksPassed; // number of 250us ticks since last call
    static uint8_t  L_current_state = 1;  // 24x7 collection "state" to execute when called

    args->error.error = 0; // default success
    args->error.ffdc = 0;

    if(ticks == 0)  // First time 24x7 called since OCC started?
    {
        PK_TRACE("gpe_24x7: First call since OCC started");
    }
    else if(ticks > 1)  // longer than 250us since last call?
    {
       // It has been ticks*250us since last call
        PK_TRACE("gpe_24x7: It has been 0x%02X ticks since last call", ticks);
    }

    switch(L_current_state)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
            break;

        default:
            PK_TRACE("gpe_24x7: Invalid collection state: 0x%02X", L_current_state);
            break;
    }

    // Setup state to run on next call
    if(L_current_state == MAX_24x7_STATES)
        L_current_state = 1;
    else
        L_current_state++;

    // send back a response, IPC success even if ffdc/rc are non zeros
    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("gpe_24x7: Failed to send response back. Halting GPE1");
        gpe_set_ffdc(&(args->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }
}
