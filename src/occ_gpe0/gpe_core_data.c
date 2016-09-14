/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/gpe_core_data.c $                                */
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

#include "core_data.h"
#include "ipc_async_cmd.h"
#include "gpe_err.h"
#include "gpe_util.h"
#include "proc_shared.h"
#include "nest_dts.h"

/*
 * Function Specifications:
 *
 * Name: gpe_get_core_data
 *
 * Description:  extract core number, call get_core data with the
 *               proper core id and pointer to CoreData
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */
void gpe_get_core_data(ipc_msg_t* cmd, void* arg)
{
    uint32_t rc;    // return code
    ipc_async_cmd_t *async_cmd  = (ipc_async_cmd_t*)cmd;
    ipc_core_data_parms_t *args = (ipc_core_data_parms_t*) async_cmd->cmd_data;


    rc = get_core_data(args->core_num,     // core ID
                       args->data);        // CoreData*

    if(rc)
    {
        PK_TRACE("gpe_get_core_data: get_core_data failed, rc = 0x%08x, core = 0x%08x",
                 rc, args->core_num);
        gpe_set_ffdc(&(args->error), args->core_num,
                      GPE_RC_GET_CORE_DATA_FAILED, rc);
    }

    // send back a response, IPC success even if ffdc/rc are non zeros
    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("gpe_get_core_data: Failed to send response back. Halting GPE0", rc);
        gpe_set_ffdc(&(args->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }

}


/*
 * Function Specifications:
 *
 * Name: gpe_get_nest_dts
 *
 * Description:  Get the 3 NEST DTS sensor readings
 *
 * Inputs:       cmd is a pointer to IPC msg's cmd and cmd_data struct
 *
 * Outputs:      error: sets rc, address, and ffdc in the cmd_data's
 *                      GpeErrorStruct
 *
 * End Function Specification
 */
void gpe_get_nest_dts(ipc_msg_t* cmd, void* arg)
{
    uint32_t rc;    // return code
    ipc_async_cmd_t *async_cmd  = (ipc_async_cmd_t*)cmd;
    ipc_nest_dts_parms_t *args = (ipc_nest_dts_parms_t*) async_cmd->cmd_data;

    args->error.error = 0;
    args->error.ffdc = 0;

    rc = get_nest_dts(&args->data);        // NestDts_t*

    if(rc)
    {
        PK_TRACE("gpe_get_nest_dts: get_nest_dts failed, rc = 0x%08x", rc);
        gpe_set_ffdc(&(args->error), 0x00,
                      GPE_RC_GET_NEST_DTS_FAILED, rc);
    }

    // send back a response, IPC success even if ffdc/rc are non zeros
    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("gpe_get_nest_dts: Failed to send response back. Halting GPE0", rc);
        gpe_set_ffdc(&(args->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }
}
