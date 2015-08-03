/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/occlib/ipc_ping.c $                                   */
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
#include "ipc_ping.h"

#ifdef IPC_ENABLE_PING
//server side ping message handler
void ipc_ping_handler(ipc_msg_t* cmd, void* arg)
{
    //NOTE: this will run in a critical interrupt when sent to the 405
    //ignore return codes
    ipc_send_rsp(cmd, IPC_RC_SUCCESS);
}

//Note: This runs in a critical interrupt on the 405 but SSX functions
//      can not be called from a critical interrupt.  Instead, it must be
//      deferred to a non-critical handler.
void ipc_ping_response(ipc_msg_t* rsp, void* arg)
{
    ipc_ping_cmd_t *ping_cmd = (ipc_ping_cmd_t*)rsp;

    if(KERN_CONTEXT_CRITICAL_INTERRUPT())
    {
        //NOTE: this is a no-op on PPE
        IPC_DEFER_TO_NONCRITICAL(rsp);
    }
    else
    {
        KERN_SEMAPHORE_POST(&ping_cmd->sem);
        ipc_free_msg(&ping_cmd->msg);
    }

}


//Command that can be run in a thread context to ping another target
//The message is allocated on the stack
int ipc_ping(ipc_ping_cmd_t* ping_cmd, uint32_t target_id)
{
    int rc;

    do
    {
        //set the target (since this is a multi-target command)
        rc = ipc_set_cmd_target(&ping_cmd->msg, target_id);
        if(rc)
        {
            break;
        }

        //send the command
        rc = ipc_send_cmd(&ping_cmd->msg);
        if(rc)
        {
            break;
        }

        //assume that if we timed out then the target must have gone down.
        rc = KERN_SEMAPHORE_PEND(&ping_cmd->sem, KERN_SECONDS(1));
        if(rc)
        {
            if(rc == -KERN_SEMAPHORE_PEND_TIMED_OUT)
            {
                rc = IPC_RC_TIMEOUT;
            }
            break;
        }

        //response message was received.  Now return the ipc_rc
        rc = ipc_get_rc(&ping_cmd->msg);
        
    }while(0);
    return rc;
}
#endif /*IPC_ENABLE_PING*/
