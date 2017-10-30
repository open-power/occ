#include "gpe_centaur.h"
#include "ipc_async_cmd.h"
#include "gpe_util.h"

CentaurConfiguration_t * G_centaur_config = NULL;

void gpe_centaur_init(ipc_msg_t* i_cmd, void* i_arg)
{
    int      rc;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)i_cmd;
    CentaurConfigParms_t* payload = (CentaurConfigParms_t*)async_cmd->cmd_data;

    CentaurConfiguration_t * config = payload->centaurConfiguration;
    G_centaur_config = config;

    payload->error.error = 0;
    payload->error.ffdc  = 0;

    if(G_centaur_config == NULL)
    {
        PK_TRACE("gpe_centaur_init: centaurConfiguration data ptr is NULL!");
        rc = GPE_RC_CONFIG_DATA_NULL_PTR;
    }
    else
    {
        rc = gpe_centaur_configuration_create(G_centaur_config);
    }

    payload->error.rc = rc;

    // Send response
    rc = ipc_send_rsp(i_cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("gpe_centaur_init: Failed to send response. rc = %x. Halting GPE1.",
                 rc);

        gpe_set_ffdc(&(payload->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }
}

void gpe_centaur_scom(ipc_msg_t* i_cmd, void* i_arg)
{
    int      rc;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)i_cmd;
    CentaurScomParms_t * scomParms = (CentaurScomParms_t*)async_cmd->cmd_data;

    gpe_scom_centaur(G_centaur_config, scomParms);

    // Send response
    rc = ipc_send_rsp(i_cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("gpe_centaur_scom: Failed to send response. rc = %x. Halting GPE1.",
                 rc);

        gpe_set_ffdc(&(scomParms->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }


}

void gpe_centaur_data(ipc_msg_t* i_cmd, void* i_arg)
{
    int      rc;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)i_cmd;

    CentaurGetMemDataParms_t * dataParms =
        (CentaurGetMemDataParms_t *)async_cmd->cmd_data;

    rc = centaur_get_mem_data(G_centaur_config, dataParms);

    dataParms->error.rc = rc;

    // Send response
    rc = ipc_send_rsp(i_cmd, IPC_RC_SUCCESS);
    if(rc)
    {
        PK_TRACE("gpe_centaur_init: Failed to send response. rc = %x. Halting GPE1.",
                 rc);

        gpe_set_ffdc(&(dataParms->error), 0x00, GPE_RC_IPC_SEND_FAILED, rc);
        pk_halt();
    }
}


