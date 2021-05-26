/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe_membuf.c $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2019                        */
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
#include "gpe_membuf.h"
#include "ipc_async_cmd.h"
#include "gpe_util.h"

MemBufConfiguration_t * G_membuf_config = NULL;

void gpe_membuf_init(ipc_msg_t* i_cmd, void* i_arg)
{
    int      rc;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)i_cmd;
    MemBufConfigParms_t* payload = (MemBufConfigParms_t*)async_cmd->cmd_data;

    MemBufConfiguration_t * config = payload->membufConfiguration;
    G_membuf_config = config;

    payload->error.error = 0;
    payload->error.ffdc  = 0;

    if(G_membuf_config == NULL)
    {
        PK_TRACE("gpe_membuf_init: membufConfiguration data ptr is NULL!");
        rc = GPE_RC_CONFIG_DATA_NULL_PTR;
    }
    else
    {
        if(payload->mem_type == MEMTYPE_OCMB)
        {
            PK_TRACE("Ocmb_configuration. MSR:%08x",mfmsr());
            rc = gpe_ocmb_configuration_create(G_membuf_config);
        }
        else
        {
            rc = GPE_RC_INVALID_MEMBUF_TYPE;
        }
        // Must set membuf_type AFTER config created!
        G_membuf_config->membuf_type = payload->mem_type;

        // Initialize/reset ocmb related
        if(!rc)
        {
            rc = gpe_ocmb_init(G_membuf_config);
        }
    }

    payload->error.rc = rc;

    // Send response
    rc = ipc_send_rsp(i_cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("gpe_membuf_init: Failed to send response. rc = %x. Halting GPE1.",
                 rc);

        gpe_set_ffdc(&(payload->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }
}

void gpe_membuf_scom(ipc_msg_t* i_cmd, void* i_arg)
{
    static int g_log_once = 0;
    int      rc;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)i_cmd;
    MemBufScomParms_t * scomParms = (MemBufScomParms_t*)async_cmd->cmd_data;

    if(g_log_once == 0)
    {
        g_log_once = 1;
        PK_TRACE("gpe_membuf_scom. MSR:%08x",mfmsr());
    }
    gpe_inband_scom(G_membuf_config, scomParms);

    // Send response
    rc = ipc_send_rsp(i_cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("gpe_membuf_scom: Failed to send response. rc = %x. Halting GPE1.",
                 rc);

        gpe_set_ffdc(&(scomParms->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }


}

void gpe_membuf_data(ipc_msg_t* i_cmd, void* i_arg)
{
    static int g_log_once = 0;
    int      rc;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)i_cmd;

    MemBufGetMemDataParms_t * dataParms =
        (MemBufGetMemDataParms_t *)async_cmd->cmd_data;

    if(g_log_once == 0)
    {
        g_log_once = 1;
        PK_TRACE("MemBuf Data. MSR:%08x",mfmsr());
    }
    rc = get_ocmb_sensorcache(G_membuf_config, dataParms);

    dataParms->error.rc = rc;

    // Send response
    rc = ipc_send_rsp(i_cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("gpe_membuf_data: Failed to send response. rc = %x. Halting GPE1.",
                 rc);

        gpe_set_ffdc(&(dataParms->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }
}


