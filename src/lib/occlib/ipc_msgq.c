/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/occlib/ipc_msgq.c $                                   */
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
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2015
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ipc_msgq.c
/// \brief Implementation of IPC (InterProcessor Communication) routines that
///        involve using message queues (primarily in a thread context).

#include "ipc_api.h"

///////////////////////////////////////////////////////////////////////////////
// Handles msgq messages (commands and responses) by placing them on a message
// queue and then posting the message queue semaphore.
// This function is for internal use only!
void ipc_msgq_handler(ipc_msg_t* msg, void* arg)
{
    ipc_msgq_t*  msgq = (ipc_msgq_t*)arg;

    //NOTE: this is hard coded to 0 on PPE
    if(KERN_CONTEXT_CRITICAL_INTERRUPT())
    {
        //NOTE: this is a no-op on PPE
        IPC_DEFER_TO_NONCRITICAL(msg);
    }
    else
    {
        KERN_DEQUE_PUSH_BACK(&msgq->msg_head, &msg->node);
        KERN_SEMAPHORE_POST(&msgq->msg_sem);
    }
}


///////////////////////////////////////////////////////////////////////////////
/// Wait (with timeout) for an IPC message on an IPC message queue.
///
int ipc_msgq_recv(ipc_msg_t** msg, ipc_msgq_t* msgq, KERN_INTERVAL timeout)
{
    int                     rc;
    ipc_msg_t*               popped_msg = 0;
    KERN_MACHINE_CONTEXT    ctx;

    // First check for pending messages already on the queue.
    KERN_CRITICAL_SECTION_ENTER(KERN_CRITICAL, &ctx);
    popped_msg = (ipc_msg_t*)KERN_DEQUE_POP_FRONT(&msgq->msg_head);

    if(popped_msg)
    {
        KERN_CRITICAL_SECTION_EXIT(&ctx);
        rc = IPC_RC_SUCCESS;
    }
    else  // no message - wait for one
    {

        rc = KERN_SEMAPHORE_PEND(&msgq->msg_sem, timeout);
        KERN_CRITICAL_SECTION_EXIT(&ctx);

        if(rc)
        {
            if(rc == -KERN_SEMAPHORE_PEND_TIMED_OUT ||
               rc == -KERN_SEMAPHORE_PEND_NO_WAIT)
            {
                rc = IPC_RC_TIMEOUT;
            }
        }
        else
        {
            //The queue is also modified in the IPC interrupt context so
            //we need to make sure interrupts are disabled while we modify it.
            KERN_CRITICAL_SECTION_ENTER(KERN_CRITICAL, &ctx);
            popped_msg = (ipc_msg_t*)KERN_DEQUE_POP_FRONT(&msgq->msg_head);
            KERN_CRITICAL_SECTION_EXIT(&ctx);

            if(popped_msg)
            {
                rc = IPC_RC_SUCCESS;
            }
            else
            {
                rc = IPC_RC_NO_MSG;
            }
        }
    }

    *msg = popped_msg;
    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// Associate a message queue with a function ID (commands recieved with the
/// specified function id will be placed on the message queue).
///
int ipc_register_msgq(uint32_t func_id, ipc_msgq_t* msgq)
{
    return ipc_set_handler(func_id, ipc_msgq_handler, msgq);
}



