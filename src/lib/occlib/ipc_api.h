/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/occlib/ipc_api.h $                                    */
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
#ifndef __IPC_API_H__
#define __IPC_API_H__

/// \file ipc_api.h
/// \brief Common header for Interprocessor Communications API
///
#include "ipc_structs.h"

#ifndef __ASSEMBLER__
///////////////////////////////////////////////////////////////////////////////
/// Initialize an IPC command message
///
/// \param msg A pointer to the command message
///
/// \param func_id A user-defined function ID that is known to both the sender
/// and receiver of a command message.  (Defined in \a ipc_func_ids.h)
///
/// \param resp_callback A user-defined function that should be called when
/// the message is returned as a response to the command.  This must be set
/// to 0 if no callback function should be called.
///
/// \param callback_arg A pointer to user-defined data that will be passed in
/// to the callback function when it is called. This should be set to 0 if
/// no data needs to be passed.
///
/// This function (or \a ipc_init_msgq_msg) must be called on a message at
/// least once before it is sent via the \a ipc_send_msg interface.
///
/// There are two types of function ID's. Function ID's that only work on a
/// single processor are called \e single-target ID's.  These function
/// ID's have the target ID embedded as part of the ID.  FUnction ID's
/// that are supported on multiple processors are called \e multi-target ID's.
/// Command messages associated with multi-target function ID's must go through
/// the extra step of setting the target ID by calling the \a ipc_set_cmd_target
/// interface on the command message.
///
/// If a callback function is provided, that callback function should cause
/// (directly or indirectly) the \ipc_free_msg interface to be called once it
/// is known that it is safe for the message to be reused (sent as a command
/// again).
///
void ipc_init_msg(ipc_msg_t* msg,
                  uint32_t func_id,
                  ipc_msg_handler_t resp_callback,
                  void* callback_arg);

//Use these to statically initialize an IPC message
#define IPC_MSG_INIT(_func_id, _resp_callback, _callback_arg) \
    {\
        {.node = KERN_DEQUE_ELEMENT_INIT()}, \
        .func_id.word32 = _func_id, \
                          .ipc_rc = IPC_RC_SUCCESS, \
                                    .resp_callback = _resp_callback, \
                                            .callback_arg = _callback_arg \
    }

#define IPC_MSG_CREATE(msg_name, _func_id, _resp_callback, _callback_arg) \
    ipc_msg_t msg_name = IPC_MSG_INIT(_func_id, _resp_callback, _callback_arg)

///////////////////////////////////////////////////////////////////////////////
/// Free up a message to be reused.
///
/// \param msg a pointer to a message.
///
/// This interface should be called on a message when it is known that it is
/// safe to reuse the message.  Normally, this would be one of the last things
/// performed in the response callback function for a command, but it may also
/// be called when it is known that a peer has stopped functioning.
///
static inline void ipc_free_msg(ipc_msg_t* msg)
{
    msg->func_id.active_flag = 0;
}

///////////////////////////////////////////////////////////////////////////////
/// Set the target ID for a multi-target command message.
///
/// \param cmd A pointer to an initialized command message.
///
/// \param target_id The target ID of the processor the command is intended
/// for.
///
/// The following return codes are possible:
///
/// \retval IPC_RC_SUCCESS The command's target ID was updated.
///
/// \retval IPC_RC_INVALID_FUNC_ID The function ID associated with this
/// command is not a valid mult-target function ID.
///
int ipc_set_cmd_target(ipc_msg_t* cmd, uint32_t target_id);


///////////////////////////////////////////////////////////////////////////////
/// Send a message as a command
///
/// \param cmd A pointer to an initialized command message
///
/// It is expected that at some point prior to calling this function the
/// message was initialized with a call to \a ipc_init_msg or
/// \a ipc_init_msgq_msg.
///
/// Once a message has been sent it is not safe to send again until it has been
/// sent back to the sender as a response.
///
/// The following return codes are possible:
///
/// \retval IPC_RC_SUCCESS The message was successfully placed on the target's
/// receive buffer.
///
/// \retval IPC_RC_SELF_BLOCKED The call was made prior to calling
/// \a ipc_enable.
///
/// \retval IPC_RC_INVALID_FUNC_ID The command was initialized with an invalid
/// function ID.
///
/// \retval IPC_RC_MSG_ACTIVE The message is currently in use.
///
/// \retval IPC_RC_INVALID_TARGET_ID The message was initialized with an
/// invalid target ID.  This can happen if a multi-target command has not had
/// its target set via the \a ipc_set_cmd_target function at least one time.
///
/// \retval IPC_RC_BUFFER_FULL The command could not be sent because the target's
/// receive buffer is full.
///
/// \retval IPC_RC_TARGET_BLOCKED The command could not be sent because the
/// target is blocking any new messages.
///
int ipc_send_cmd(ipc_msg_t* cmd);


////////////////////////////////////////////////////////////////////////////////
/// Send a command message back to the sender as a response message with status.
///
/// \param rsp A pointer to a message that was recieved as a command message.
///
/// \param ipc_rc This should be either \a IPC_RC_SUCCESS if the command was
/// successful or \a IPC_RC_CMD_FAILED if the command failed.  If
/// command-specific return codes are needed, they should be returned as
/// command-specific fields instead of returning them here so that there
/// is no risk of overlapping return codes.
///
/// It is expected that at some point prior to calling this function the
/// message was initialized with a call to \a ipc_init_msg or
/// \a ipc_init_msgq_msg.
///
/// Once a message has been sent it is not safe to send again until it has been
/// sent back to the sender as a response.
///
/// The following return codes are possible:
///
/// \retval IPC_RC_SUCCESS The message was successfully placed on the target's
/// receive buffer.
///
/// \retval IPC_RC_MSG_NOT_ACTIVE The message is not from an active command.
///
/// \retval IPC_RC_INVALID_TARGET_ID The target id for the sender is invalid.
/// This likely means that the message has been corrupted.
///
/// \retval IPC_RC_BUFFER_FULL The response could not be sent because the target's
/// recieve buffer is full.
///
/// \retval IPC_RC_TARGET_BLOCKED The response could not be sent because the
/// target is blocking any new messages.
///
int ipc_send_rsp(ipc_msg_t* rsp, uint32_t return_code);


///////////////////////////////////////////////////////////////////////////////
/// Retrieves the IPC return code embedded in the response message.
///
/// \param rsp A pointer to a response message.
///
/// The embedded IPC return code is how the remote processor communicates
/// IPC internal failures to the local processor.  It can also be used by
/// non-IPC code on the remote processor to signal success or failure when it
/// sends the response message via the \a ipc_send_rsp interface.
///
/// The IPC return code should always be checked to verify that a command
/// message was processed successfully.
///
/// The following return codes are possible:
///
/// \retval IPC_RC_SUCCESS The message was successfully processed.
///
/// \retval IPC_RC_CMD_FAILED The command was processed on the remote end but
/// did not complete successfully.
///
/// \retval IPC_RC_CMD_NOT_SUPPORTED The function ID for the command is valid
/// but the remote end does not have support for that function ID.
///
/// \retval IPC_RC_INVALID_FUNC_ID The function ID for the command is invalid.
///
static inline int ipc_get_rc(ipc_msg_t* rsp)
{
    return rsp->ipc_rc;
}

///////////////////////////////////////////////////////////////////////////////
/// Retrieve the IPC function ID for a message
///
/// \param msg A pointer to an IPC message
///
/// This interface should be used to extract the IPC function ID of a message.
///
/// The IPC function ID is returned.
///
static inline int ipc_get_funcid(ipc_msg_t* msg)
{
    int func_id = msg->func_id.word32;

    //Multi-target function ID's always have the target ID set to
    //so that if the caller doesn't set it to a proper target id it will
    //flag an error.
    if(func_id & IPC_FLAG_MT)
    {
        func_id |= IPC_TARGET_MASK;
    }

    //Clear the active and response flags in case they are set along
    //with the sender ID.
    func_id &= ~(IPC_FLAG_ACTIVE | IPC_FLAG_RESPONSE | IPC_SENDER_MASK);

    return func_id;
}

///////////////////////////////////////////////////////////////////////////////
/// Determine if a message is a response or a command
///
/// \param msg A pointer to an IPC message
///
/// This function should be used to determine if a message is a response or a
/// command.  The function will return a non-zero value if the message is a
/// response and 0 otherwise.
///
static inline int ipc_is_a_response(ipc_msg_t* msg)
{
    return msg->func_id.response_flag;
}

///////////////////////////////////////////////////////////////////////////////
/// Determine if a message is free to be re-used
///
/// \param msg A pointer to an IPC message
///
/// This function should be used to determine if a message is free to re-use.
/// The function will return a non-zero value if the message is free and
/// 0 otherwise.
///
static inline int ipc_is_free(ipc_msg_t* msg)
{
    return !msg->func_id.active_flag;
}

///////////////////////////////////////////////////////////////////////////////
/// Retrieve the sender ID of a message.
///
/// \param msg A pointer to an IPC message
///
/// This function should be used to retrieve the sender ID of a message.  It
/// returns the sender ID.
///
static inline int ipc_sender_id(ipc_msg_t* msg)
{
    return msg->func_id.sender_id;
}

///////////////////////////////////////////////////////////////////////////////
/// Retrieve the target ID of a message.
///
/// \param msg A pointer to an IPC message
///
/// This function should be used to retrieve the target ID of a message.  It
/// returns the target ID.
///
static inline int ipc_target_id(ipc_msg_t* msg)
{
    return msg->func_id.target_id;
}

///////////////////////////////////////////////////////////////////////////////
/// Initializes IPC control structures.
///
/// Clears the IPC buffers for this processor and places them in a state
/// where new messages are blocked.  Also initializes the IPC handler table
/// for this processor if STATIC_IPC_TABLES has not been defined.
///
/// This function always returns \a IPC_RC_SUCCESS
///
int ipc_init(void);


///////////////////////////////////////////////////////////////////////////////
/// Enables IPC communications.
///
/// Unmasks the IPC interrupt for this processor and places the circular
/// buffers in a state where they can recieve messages.
///
/// This function must be called before using the \a ipc_send_cmd function or
/// it will return \a IPC_RC_SELF_BLOCKED.
///
/// The function always returns \a IPC_RC_SUCCESS.
///
int ipc_enable(void);


///////////////////////////////////////////////////////////////////////////////
/// Disable recieving new IPC commands for a processor.
///
/// \param target_id The target ID of the processor to disable receiving new
/// IPC commands on.
///
/// This interface should be used by a processor that knows it is about to go
/// down or by a processor that knows that one of its peers has halted.
///
/// Calling this function on a processor will cause other processors to get
/// a return code of \a IPC_RC_TARGET_BLOCKED with subsequent calls to
/// \a ipc_send_cmd.  Calling this function on one's self (using a target ID of
/// OCCHW_INST_ID_SELF) will cause subsequent calls to \a ipc_send_cmd to
/// return \a IPC_RC_SELF_BLOCKED.
///
/// Possible return codes for this function are:
///
/// \retval IPC_RC_SUCCESS The target processor was successfully disabled.
///
/// \retval IPC_RC_INVALID_TARGET_ID The target ID is invalid.
///
int ipc_disable(uint32_t target_id);


///////////////////////////////////////////////////////////////////////////////
/// Associates an IPC function ID with a handler function
///
/// \param func_id A user-defined function ID that is known to both the sender
/// and receiver of a command message.  (Defined in \a ipc_func_ids.h)
///
/// \param handler A pointer to the function that handles command messages that
/// have been initialized with \a func_id.
///
/// \param callback_arg A pointer to data that is passed as an argument to the
/// handler function when it is called.
///
/// This function should be used to link an IPC function ID with a function.
/// Once this has been done, if the local processor recieves a command message
/// with an IPC function ID that matches \a func_id then it will call the
/// handler function that was specified by \a handler.
///
/// NOTE: All handler functions will be called from an interrupt context.
///
/// Possible return codes are:
///
/// \retval IPC_RC_SUCCESS The operation completed successfully.
///
/// \retval IPC_RC_INVALID_TARGET_ID The function ID is a single-target
/// function ID that does not target this processor.
///
/// \retval IPC_RC_INVALID_FUNC_ID The function ID is not a valid IPC function
/// ID.
///
/// \retval IPC_RC_INVALID_ARG The handler parameter must be a non-zero value.
///
int ipc_set_handler(uint32_t func_id,
                    ipc_msg_handler_t handler,
                    void* callback_arg);


///////////////////////////////////////////////////////////////////////////////
/// Initialize an IPC message queue.
///
/// \param msgq A pointer to a message queue.
///
/// All message queues must be initialized one time prior to use with other
/// interfaces.
///
void ipc_init_msgq(ipc_msgq_t* msgq);

//Use this to statically initialize an IPC message queue
#define IPC_MSGQ_CREATE(msgq) \
    ipc_msgq_t msgq = \
                      {\
                       .msg_head = KERN_DEQUE_SENTINEL_INIT(&msgq.msg_head),\
                       .msg_sem = KERN_SEMAPHORE_INITIALIZATION(0, 0)\
                      }

///////////////////////////////////////////////////////////////////////////////
/// Initialize an IPC message and associate it with an IPC message queue
///
/// \param msg A pointer to an IPC message.
///
/// \param func_id A user-defined function ID that is known to both the sender
/// and receiver of the command message.  (Defined in \a ipc_func_ids.h)
///
/// \param msgq A pointer to an initialized IPC message queue.
///
/// This interface should be used in place of \a ipc_init_msg when the caller
/// wishes to have the command response placed on the specified IPC message
/// queue.  This allows a thread to block (via the \a ipc_msq_recv interface)
/// on the message queue until a response to a command has arrived.
///
/// \note An IPC message queue can be associated with more than one IPC
/// message.
///
/// See \a ipc_init_msg for more information.
///
void ipc_init_msgq_msg(ipc_msg_t* msg, uint32_t func_id, ipc_msgq_t* msgq);

//Use this to statically create an initialized IPC message queue message
#define IPC_MSGQ_MSG_CREATE(msg_name, func_id, msgq) \
    IPC_MSG_CREATE(msg_name, func_id, ipc_msgq_handler, msgq)

///////////////////////////////////////////////////////////////////////////////
/// Wait (with timeout) for an IPC message on an IPC message queue.
///
/// \param msg A pointer to an IPC message pointer.
///
/// \param msgq A pointer to an initialized IPC message queue.
///
/// \param timeout The time to wait for the next IPC message to arrive.
///
/// This interface can be used in a thread context to block while waiting for
/// the next message (command or response) to arrive on an IPC message queue.
///
/// For an IPC command message to be placed on an IPC message queue, the
/// function ID for the command must first be associated with the message queue
/// by making a call to \a ipc_register_msgq.
///
/// For an IPC response message to be placed on an IPC message queue, the
/// message must be initialized via the \a ipc_init_msgq_msg interface.
///
/// If \a ipc_msgq_recv returns a value of \a IPC_RC_SUCCESS then \a msg is
/// guaranteed to point to a new message.  Otherwise, an IPC message was not
/// retrieved from the message queue and \a msg will be set to 0.
///
/// Possible return codes for this function are:
///
/// \retval IPC_RC_SUCCESS A new IPC message was received and \a msg has been
/// set to point to the new message.
///
/// \retval IPC_RC_TIMEOUT No new messages were recieved within the timeout
/// period and \msg was set to 0.
///
/// \retval IPC_RC_NO_MSG This should never happen, but if it does it indicates
/// an internal failure occurred.
///
int ipc_msgq_recv(ipc_msg_t** msg, ipc_msgq_t* msgq, KERN_INTERVAL timeout);


///////////////////////////////////////////////////////////////////////////////
/// Associate an IPC message queue with an IPC function ID
///
/// \param func_id A user-defined function ID that is known to both the sender
/// and receiver of a command message.  (Defined in \a ipc_func_ids.h)
///
/// \param msgq A pointer to an initialized IPC message queue.
///
/// This interface associates an IPC function ID with an IPC message queue so
/// that when the calling processor recieves a command with the specified
/// function ID the message will be place on the message queue and a thread
/// that is blocked waiting for a message on the queue (using the
/// \a ipc_msgq_recv interface) will be woken up and given the message.
///
/// This function should be called in place of the \a ipc_set_handler
/// interface.
///
/// NOTE: Multiple function ID's can be associated with a single queue.
///
/// Possible return codes are:
///
/// \retval IPC_RC_SUCCESS The operation completed successfully.
///
/// \retval IPC_RC_INVALID_TARGET_ID The function ID is a single-target
/// function ID that does not target this processor.
///
/// \retval IPC_RC_INVALID_FUNC_ID The function ID is not a valid IPC function
/// ID.
///
int ipc_register_msgq(uint32_t func_id, ipc_msgq_t* msgq);

///////////////////////////////////////////////////////////////////////////////
/// Internal function that places an IPC message on an IPC message queue
///
void ipc_msgq_handler(ipc_msg_t* msg, void* arg);

///////////////////////////////////////////////////////////////////////////////
/// The default IPC command handler simply sends a response with the IPC return
/// code set to IPC_RC_CMD_NOT_SUPPORTED
///
void ipc_default_handler(ipc_msg_t* msg, void* arg);

#endif /*__ASSEMBLER__*/
#endif  /* __IPC_API_H__ */
