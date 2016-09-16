/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/nop.c $                                          */
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
#include "gpe_export.h"
#include "ipc_api.h"
#include "ipc_async_cmd.h"
#include "gpe_util.h"

/*
 * Function Specification:
 *
 * Name: gpe1_nop
 *
 * Description: a function that does nothing. Called to measure IPC timing
 *
 * Inputs:      none
 *
 * return:      none
 *
 * End Function Specification
 */

void gpe1_nop(ipc_msg_t* cmd, void* arg)
{
    int rc;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    nop_t *args = (nop_t*)async_cmd->cmd_data;

    // send back a response, IPC success even if ffdc/rc are non zeros
    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("gpe1_nop: Failed to send response back. Halting GPE1", rc);
        gpe_set_ffdc(&(args->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }
}
