/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/occlib/ipc_init.c $                                   */
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

/// \file ipc_init.c
/// \brief Implementation of IPC (InterProcessor Communication) routines for
///        Setting up the IPC function tables, messages and message queues.
///
/// NOTE: The functions that these interfaces peform can all be done statically.
/// This code will not be included in an image if none of the functions are
/// referenced.

#include "ipc_api.h"
#include "ipc_ping.h"

///////////////////////////////////////////////////////////////////////////////
/// Associate an IPC function ID with a handler function
///
int ipc_set_handler(uint32_t            function_id,
                    ipc_msg_handler_t   handler,
                    void*               callback_arg)
{
    ipc_func_table_entry_t *func_table;
    uint32_t                table_limit;
    int                     rc = IPC_RC_SUCCESS;
    ipc_func_id_t           func_id = {{0}};

    do
    {
        func_id.word32 = function_id;

        //setup for multi-target commands
        if(func_id.multi_target_flag)
        {
            table_limit = IPC_MT_NUM_FUNCIDS;
            func_table = G_ipc_mt_handlers;
        }
        //setup for single-target commands
        else
        {
            //make sure the function id targets this processor
            if(func_id.target_id != OCCHW_INST_ID_SELF)
            {
                rc = IPC_RC_INVALID_TARGET_ID;
                break;
            }
            table_limit = IPC_ST_NUM_FUNCIDS;
            func_table = G_ipc_st_handlers;
        }

        //make sure the function id is valid
        if((func_id.table_index >= table_limit) || !func_id.valid_flag)
        {
            rc = IPC_RC_INVALID_FUNC_ID;
            break;
        }

        if(!handler)
        {
            rc = IPC_RC_INVALID_ARG;
            break;
        }

        func_table[func_id.table_index].handler = handler;
        func_table[func_id.table_index].arg = callback_arg;
    }while(0);
    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// Initialize an IPC command message
///
void ipc_init_msg(ipc_msg_t* msg, 
                 uint32_t func_id,
                 ipc_msg_handler_t resp_callback,
                 void* callback_arg)
{
    KERN_DEQUE_ELEMENT_CREATE(&msg->node);
    msg->func_id.word32 = func_id;
    msg->ipc_rc = IPC_RC_SUCCESS;
    msg->resp_callback = resp_callback;
    msg->callback_arg = callback_arg;
}

///////////////////////////////////////////////////////////////////////////////
/// Initialize an IPC message queue.
///
void ipc_init_msgq(ipc_msgq_t* msgq)
{
    KERN_DEQUE_SENTINEL_CREATE(&msgq->msg_head);

    //set the initial count to 0 with no max count
    KERN_SEMAPHORE_CREATE(&msgq->msg_sem, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////
/// Initialize an IPC message and associate it with an IPC message queue
///
void ipc_init_msgq_msg(ipc_msg_t* msg, uint32_t func_id, ipc_msgq_t* msgq)
{
    ipc_init_msg(msg, func_id, ipc_msgq_handler, msgq);
}


///////////////////////////////////////////////////////////////////////////////
/// Initialize an IPC ping command message
///
#ifdef IPC_ENABLE_PING
int ipc_ping_cmd_init(ipc_ping_cmd_t* ping_cmd)
{
    int rc;

    do
    {
        //initialize the message
        ipc_init_msg(&ping_cmd->msg, IPC_MT_PING, ipc_ping_response, 0);

        //initialize the semaphore count to 0 and set the max count to 1
        rc = KERN_SEMAPHORE_CREATE(&ping_cmd->sem, 0, 1);
        if(rc)
        {
            break;
        }
    }while(0);
    return rc;
}
#endif /*IPC_ENABLE_PING*/
