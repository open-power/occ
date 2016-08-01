/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/occhw/occhw_async_gpe.c $                             */
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

/// \file occhw_async_gpe.c
/// \brief Async driver code for GPE

#include "ssx.h"
#include "occhw_async.h"

///////////////////////////////////////////////////////////////////////////////
/// Global Data
///////////////////////////////////////////////////////////////////////////////
#define ASYNC_NUM_GPE_QUEUES 4
#define ASYNC_ENG2GPE(eng) ((eng) & 0x0000000f)

// The GPE queue objects.
GpeQueue G_async_gpe_queue0;
GpeQueue G_async_gpe_queue1;
GpeQueue G_async_gpe_queue2;
GpeQueue G_async_gpe_queue3;

// Each GPE queue gets one IPC command.  These are allocated separately so that
// they can be allocated in a non-cacheable section.
ipc_async_cmd_t G_async_ipc_cmd[ASYNC_NUM_GPE_QUEUES] SECTION_ATTRIBUTE(".noncacheable");

///////////////////////////////////////////////////////////////////////////////
/// GpeQueue
///////////////////////////////////////////////////////////////////////////////

/// Create (initialize) a GpeQueue
///
/// \param queue An uninitialized or otherwise idle GpeeQueue
///
/// \param engine The identifier of a GPE engine associated with this queue.
///
/// This API initializes the GpeQueue structure.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_OBJECT_PORE_QUEUE The \a queue was NULL (0).
///
/// \retval -ASYNC_INVALID_ENGINE_GPE The \a engine is not a (valid)
/// GPE engine.

int
gpe_queue_create(GpeQueue* queue, int engine)
{
    if (SSX_ERROR_CHECK_API)
    {
        SSX_ERROR_IF(queue == 0, ASYNC_INVALID_OBJECT_GPE_QUEUE);
        SSX_ERROR_IF(!(engine & ASYNC_ENGINE_GPE), ASYNC_INVALID_ENGINE_GPE);
        SSX_ERROR_IF((ASYNC_ENG2GPE(engine) >= ASYNC_NUM_GPE_QUEUES), ASYNC_INVALID_ENGINE_GPE);
    }

    //initialize the base async queue
    async_queue_create(&queue->queue, engine);

    //assign an IPC message to be used with the queue
    //This is kept as a pointer so that the message can kept in a
    //cache-inhibited section of SRAM.
    queue->ipc_cmd = &G_async_ipc_cmd[ASYNC_ENG2GPE(engine)];

    //The IPC target ID that all messages on this queue will be sent to
    queue->ipc_target_id = ASYNC_ENG2GPE(engine);

    return 0;
}

////////////////////////////////////////////////////////////////////////////
// async_ipc_callback
////////////////////////////////////////////////////////////////////////////

/// Internal function that handles aysnc IPC command responses

void
gpe_async_handler(ipc_msg_t* rsp, void* arg)
{
    // check for errors detected by the GPE code
    if(rsp->ipc_rc != IPC_RC_SUCCESS)
    {
        //calls gpe_error_method before calling async_handler
        async_error_handler((AsyncQueue*)arg, ASYNC_REQUEST_STATE_FAILED);
    }
    else
    {

        //handle async callbacks and process the next gpe request in the queue
        //(if any)
        async_handler((AsyncQueue*) arg);
    }
}

////////////////////////////////////////////////////////////////////////////
// GpeRequest
////////////////////////////////////////////////////////////////////////////

/// Create (initialize) the GpeRequest base class
///
/// \param request An uninitialized or otherwise idle GpeRequest.
///
/// \param queue An initialized GpeQueue.
///
/// \param func_id The IPC function ID of the GPE command.
///
/// \param cmd_data A pointer to command-specific input and output data.
///
/// \param timeout If not specified as SSX_WAIT_FOREVER, then this request
/// will be governed by a private watchdog timer that will cancel a queued job
/// or kill a running job if the hardware operation does not complete before
/// it times out.
///
/// \param callback The callback to execute when the GPE command completes,
/// or NULL (0) to indicate no callback.
///
/// \param arg The parameter to the callback routine; ignored if the \a
/// callback is NULL.
///
/// \param options Options to control request priority and callback context.
///
/// This routine has no way to know if the GpeRequest structure is currently
/// in use, so this API should only be called on uninitialized or otherwise
/// idle GpeRequest structures.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_OBJECT_GPE_REQUEST The \a request was NULL (0)
/// or the \a queue was NULL (0) or not a GpeQueue.
///
/// \retval IPC_RC_INVALID_FUNC_ID The func_id has an invalid target id
/// for the specified GPE queue.
///
/// See async_request_create() for other errors that may be returned by this
/// call.

int
gpe_request_create(GpeRequest* request,
                   GpeQueue* queue,
                   ipc_func_enum_t func_id,
                   void* cmd_data,
                   SsxInterval timeout,
                   AsyncRequestCallback callback,
                   void* arg,
                   int options)
{
    AsyncQueue*  async_queue = (AsyncQueue*)queue;
    uint32_t    targeted_func_id;
    int         rc;

    if (SSX_ERROR_CHECK_API)
    {
        SSX_ERROR_IF(!(async_queue->engine & ASYNC_ENGINE_GPE),
                     ASYNC_INVALID_OBJECT_GPE_REQUEST);
    }

    //initialize the base async request
    rc = async_request_create(&(request->request),
                              async_queue,
                              gpe_run_method,
                              gpe_error_method,
                              timeout,
                              callback,
                              arg,
                              options);

    if(!rc)
    {

        //If this is a multi-target function ID we need to set the target id.
        if(IPC_FUNCID_IS_MT(func_id))
        {
            //This macro will set the target to an invalid target id if this
            //function id is not a multi-target function ID and this condition
            //will be caught when we check that the target id for the request
            //matches the target id for the queue.
            targeted_func_id = IPC_SET_MT_TARGET(func_id, queue->ipc_target_id);
        }
        else
        {
            //single target function IDs already have a target
            targeted_func_id = func_id;
        }

        //check that target id of the command matches the target id
        //of the queue.
        if (IPC_GET_TARGET_ID(targeted_func_id) != queue->ipc_target_id)
        {
            rc = IPC_RC_INVALID_FUNC_ID;
        }
        else
        {
            //initialize data that will be used when sending the command
            request->cmd_data = cmd_data;
            request->targeted_func_id = targeted_func_id;
        }
    }

    return rc;
}


// Start a GpeRequest on a GPE
//
// \param async_request A GpeRequest upcast to an AsyncRequest.
//
// This is an internal API.
//
// This routine sends an async_request to a GPE.
//

int
gpe_run_method(AsyncRequest* async_request)
{
    GpeQueue*        queue = (GpeQueue*)(async_request->queue);
    GpeRequest*      request = (GpeRequest*)async_request;
    ipc_async_cmd_t* ipc_cmd = queue->ipc_cmd;
    int rc;

    //Initialize the IPC command message
    ipc_init_msg(&ipc_cmd->cmd,
                 request->targeted_func_id,
                 gpe_async_handler,
                 queue);
    ipc_cmd->cmd_data = request->cmd_data;

    //Send the IPC command
    rc = ipc_send_cmd(&ipc_cmd->cmd);

    //If there's an error in the send, collect ffdc and mark it as
    //having failed.
    if(rc)
    {
        gpe_error_method(async_request);
        async_request->completion_state = ASYNC_REQUEST_STATE_FAILED;
        rc = -ASYNC_REQUEST_COMPLETE;
    }

    return rc;
}


// GPE FFDC collection
//
// \param async_request A GpeRequest upcast to an AsyncRequest
//
// This is an internal API, called from the async base code when an async
// request times out.
//
// GPE async error handling procedure:
//
// - Collect FFDC from the failing engine
//
// Currently all GPE errors are treated as recoverable


int
gpe_error_method(AsyncRequest* async_request)
{
    GpeQueue* queue = (GpeQueue*)(async_request->queue);
    GpeRequest* request = (GpeRequest*)async_request;

    // Collect data that could explain why a GPE command
    // couldn't be sent or timed out on the response and save it
    // in the ffdc fields

    //retrieve IPC data
    request->ffdc.func_id = queue->ipc_cmd->cmd.func_id.word32;
    request->ffdc.ipc_rc = queue->ipc_cmd->cmd.ipc_rc;

    //retrieve XIR data
    request->ffdc.xir_dump_rc =
        occhw_xir_dump(queue->ipc_target_id, &request->ffdc.xir_dump);

    return 0;
}


////////////////////////////////////////////////////////////////////////////
// Initialization
////////////////////////////////////////////////////////////////////////////

void
async_gpe_initialize(GpeQueue* queue, int engine)
{
    gpe_queue_create(queue, engine);
}
