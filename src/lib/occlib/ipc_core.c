/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/occlib/ipc_core.c $                                   */
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

/// \file ipc_core.c
/// \brief Implementation of core IPC (InterProcessor Communication) routines

#include "kernel.h"
#include "ipc_api.h"
#include "occhw_shared_data.h"
#if __PPE42__
    #include "ppe42_cache.h"
#endif

const uint8_t G_ipi_irq[OCCHW_MAX_INSTANCES] =
{
    OCCHW_IRQ_IPI0_HI_PRIORITY,
    OCCHW_IRQ_IPI1_HI_PRIORITY,
    OCCHW_IRQ_IPI2_HI_PRIORITY,
    OCCHW_IRQ_IPI3_HI_PRIORITY,
    OCCHW_IRQ_IPI4_HI_PRIORITY
};

/// If G_ipc_enabled is zero then calls to ipc_send_cmd() will return
/// IPC_RC_SELF_BLOCKED.
uint8_t     G_ipc_enabled = 0;


///////////////////////////////////////////////////////////////////////////////
///Default IPC Handler Done Hook
///
void ipc_irq_handler_default_done_hook() { }

///
///Function pointer that holds address to done hook
///function. It is called inside the IPC IRQ handler when
///all IPC msgs have been processed
void (*G_ipc_irq_handler_done_hook)() = &ipc_irq_handler_default_done_hook;
//Set the done hook function to empty default function
//G_ipc_irq_handler_done_hook =  ;

#ifndef STATIC_IPC_TABLES
    ipc_func_table_entry_t G_ipc_mt_handlers[IPC_MT_MAX_FUNCTIONS];
    ipc_func_table_entry_t G_ipc_st_handlers[IPC_ST_MAX_FUNCTIONS];
#endif

///////////////////////////////////////////////////////////////////////////////
/// Helper function used by ipc_send_cmd and ipc_send_rsp to send a message
///
int ipc_send_msg(ipc_msg_t* msg, uint32_t target_id)
{
    ipc_target_t*           target_cbufs;
    uint8_t*                read_count;
    uint8_t*                write_count;
    ipc_msg_t**             msgs;
    KERN_MACHINE_CONTEXT    ctx;
    int                     rc = IPC_RC_SUCCESS;
    uint8_t                 num_entries;

    do
    {
        // Check for invalid target ID
        if(target_id > OCCHW_INST_ID_MAX)
        {
            rc = IPC_RC_INVALID_TARGET_ID;
            break;
        }

        target_cbufs = &OSD_PTR->ipc_data.targets[target_id];
        msgs = &target_cbufs->cbufs[OCCHW_INST_ID_SELF][0];
        read_count = &target_cbufs->counts.reads.counts8[OCCHW_INST_ID_SELF];
        write_count = &target_cbufs->counts.writes.counts8[OCCHW_INST_ID_SELF];

        //Prevent other threads on this processor from updating the cbuf
        KERN_CRITICAL_SECTION_ENTER(KERN_CRITICAL, &ctx);

        //Determine the number of entries in the buffer
#if __PPE42__
        dcbi(read_count);
#endif
        num_entries = *write_count - *read_count;

        //If the cbuf isn't full, then add the message and raise an interrupt
        if(num_entries < IPC_CBUF_SIZE)
        {
            // Mark the message as being active
            msg->func_id.active_flag = 1;

            msgs[*write_count % IPC_CBUF_SIZE] = msg;
            (*write_count)++;
#ifdef GPE_IPC_TIMERS
            msg->begin_time = in32(OCB_OTBR);
#endif

#if __PPE42__
            dcbf(write_count);
            sync();
#endif

            //raise the IPC interrupt on the target
            KERN_IRQ_STATUS_SET(IPC_GET_IRQ(target_id), 1);
        }
        else
        {
            //Check if cbuf is just full or is blocked
            if(num_entries == IPC_CBUF_SIZE)
            {
                rc = IPC_RC_BUFFER_FULL;
            }
            else
            {
                rc = IPC_RC_TARGET_BLOCKED;
            }
        }

        KERN_CRITICAL_SECTION_EXIT(&ctx);
    }
    while(0);

    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// Send a message as a command
///
int ipc_send_cmd(ipc_msg_t* cmd)
{
    int rc;

    do
    {
        //don't allow sending new commands if IPC is disabled
        if(!G_ipc_enabled)
        {
            rc = IPC_RC_SELF_BLOCKED;
            break;
        }

        //don't send a command if the valid flag is not set
        if(!cmd->func_id.valid_flag)
        {
            rc = IPC_RC_INVALID_FUNC_ID;
            break;
        }

        //don't send a command if the active flag is set
        if(cmd->func_id.active_flag)
        {
            rc = IPC_RC_MSG_ACTIVE;
            break;
        }

        cmd->func_id.response_flag = 0;

        //Set the sender ID here.  Remote side uses this for sending responses.
        cmd->func_id.sender_id = OCCHW_INST_ID_SELF;

        cmd->ipc_rc = 0;

        //place the message on the target's circular buffer
        rc = ipc_send_msg(cmd, cmd->func_id.target_id);

        cmd->ipc_rc = rc;
    }
    while(0);

    return rc;
}

////////////////////////////////////////////////////////////////////////////////
/// Send a command message back to the sender as a response message with status.
///
int ipc_send_rsp(ipc_msg_t* rsp, uint32_t ipc_rc)
{
    int rc;

    if(rsp->func_id.active_flag)
    {
        rsp->func_id.response_flag = 1;
        rsp->ipc_rc = ipc_rc;
        rc = ipc_send_msg(rsp, rsp->func_id.sender_id);
    }
    else
    {
        rc = IPC_RC_MSG_NOT_ACTIVE;
    }

    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// Default IPC handler that is called when no IPC handler has been
/// installed for the IPC function that is being requested.
void ipc_default_handler(ipc_msg_t* msg, void* arg)
{
    //Return code is ignored.  If failure occurs in sending
    //the response then the sender of the command should eventually
    //time out waiting for a response or the sender may be incapacitated.
    ipc_send_rsp(msg, IPC_RC_CMD_NOT_SUPPORTED);
}


///////////////////////////////////////////////////////////////////////////////
/// Set the target ID for a multi-target command message.
///
int ipc_set_cmd_target(ipc_msg_t* cmd, uint32_t target_id)
{
    int rc = IPC_RC_SUCCESS;

    do
    {
        //verify that this is a muti-target function
        if(!cmd->func_id.multi_target_flag)
        {
            rc = IPC_RC_INVALID_FUNC_ID;
            break;
        }
        else
        {
            cmd->func_id.target_id = target_id;
        }
    }
    while(0);

    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// Processes an incoming message (response or command) after it has been
/// removed from the circular buffer.
/// This function is for internal use only!
///
void ipc_process_msg(ipc_msg_t* msg)
{
    uint32_t                table_index;
    uint32_t                table_limit;
    ipc_func_table_entry_t* func_table;

    do
    {
        // If this is a response message, call the response callback function
        if(msg->func_id.response_flag)
        {
            if(msg->resp_callback)
            {
#ifdef GPE_IPC_TIMERS
                msg->end_time = in32(OCB_OTBR);
#endif

                msg->resp_callback(msg, msg->callback_arg);
            }
            else
            {
                //normally, the resp_callback function would call this function
                //to notify users of the message that it is free to be re-used.
                //Since there is no callback for this message we call it here.
                ipc_free_msg(msg);
            }

            break;
        }

        // extract the function table index
        table_index = msg->func_id.table_index;

        //setup for multi-target commands
        if(msg->func_id.multi_target_flag)
        {
            table_limit = IPC_MT_NUM_FUNCIDS;
            func_table = G_ipc_mt_handlers;
        }
        //setup for single-target commands
        else
        {
            table_limit = IPC_ST_NUM_FUNCIDS;
            func_table = G_ipc_st_handlers;
        }

        //Common command handling code
        if(table_index < table_limit)
        {
            func_table[table_index].handler(msg, func_table[table_index].arg);
        }
        else
        {
            //drop errors if this fails.  If target was waiting for a response
            //it should eventually time out and log the message as FFDC.
            ipc_send_rsp(msg, IPC_RC_INVALID_FUNC_ID);
        }
    }
    while(0);
}

///////////////////////////////////////////////////////////////////////////////
/// Removes messages from the circular buffer for the processor associated with
/// sender_id and processes them one at a time.
/// This function is for internal use only!
///
void ipc_process_cbuf(uint32_t sender_id)
{
    ipc_target_t*            my_cbufs = &OSD_PTR->ipc_data.targets[OCCHW_INST_ID_SELF];
    uint8_t*                 read_count = &my_cbufs->counts.reads.counts8[sender_id];
    uint8_t*                 write_count = &my_cbufs->counts.writes.counts8[sender_id];
    ipc_msg_t**               msg_ptrs = &my_cbufs->cbufs[sender_id][0];
    ipc_msg_t*               cur_msg;


    while(*read_count != *write_count)
    {
        // extract the message pointer
        cur_msg = msg_ptrs[*read_count % IPC_CBUF_SIZE];

        // increment the read count
        (*read_count)++;

        ipc_process_msg(cur_msg);
    }
}

///////////////////////////////////////////////////////////////////////////////
/// The IPC interrupt handler.  Finds which circular buffers have messages
/// and processes them.
///
#ifdef __SSX__
    KERN_IRQ_HANDLER(ipc_irq_handler_full)
#else
    KERN_IRQ_HANDLER(ipc_irq_handler)
#endif
{
    ipc_counts_t    xored_counts;
    ipc_target_t*    my_cbufs;
    uint32_t        sender_id;

    // Processors could be sending us new packets while we're
    // processing this interrupt.  We need to mask all new
    // IPI interrupts until we are done processing so that we don't
    // end up processing an interrupt that was already handled.
    KERN_IRQ_DISABLE(IPC_GET_IRQ(OCCHW_INST_ID_SELF));

    // Clear the interrupt bit in the OISR before we check for
    // status.  Checking status and then clearing the OISR bit
    // can lead to a race condition where we loose an interrupt.
    KERN_IRQ_STATUS_CLEAR(IPC_GET_IRQ(OCCHW_INST_ID_SELF));

    my_cbufs = &OSD_PTR->ipc_data.targets[OCCHW_INST_ID_SELF];

    // Make sure we get the most recent write counts from SRAM
    // dcbf(&my_cbufs->counts.writes.counts64);

    // Use XOR to find the buffers that aren't empty (read count != write count)
    xored_counts.counts64 = my_cbufs->counts.reads.counts64 ^
                            my_cbufs->counts.writes.counts64;

    while(1)
    {
        // Use cntlzw to find the first buffer that isn't empty
        sender_id = cntlz64(xored_counts.counts64) / IPC_CBUF_COUNT_BITS;


        // If all buffers are empty then we're done
        if(sender_id > OCCHW_INST_ID_MAX)
        {
            G_ipc_irq_handler_done_hook();
            break;
        }

        // Mark the buffer as empty in our local snapshot
        xored_counts.counts8[sender_id] = 0;

        // Process all new messages in the buffer
        ipc_process_cbuf(sender_id);
    }

#ifndef UNIFIED_IRQ_HANDLER_GPE
    // Unmask the irq before returning
    KERN_IRQ_ENABLE(IPC_GET_IRQ(OCCHW_INST_ID_SELF));
#endif
}


///////////////////////////////////////////////////////////////////////////////
/// This macro creates an assembly function named ipc_irq_handler which handles
/// saving/restoring the context that is required for calling a C
/// function.
///
/// NOTE: This is only needed for SSX.  PPE42 only supports full interrupts.
///
#ifdef __SSX__
    KERN_IRQ_FAST2FULL(ipc_irq_handler, ipc_irq_handler_full);
#endif


///////////////////////////////////////////////////////////////////////////////
/// Initialize IPC control structures.
///
int ipc_init(void)
{
    //prevent new messages from coming in.
    ipc_disable(OCCHW_INST_ID_SELF);

#ifndef STATIC_IPC_TABLES
    int i;

    for(i = 0; i < IPC_MT_MAX_FUNCTIONS; i++)
    {
        G_ipc_mt_handlers[i].handler = ipc_default_handler;
        G_ipc_mt_handlers[i].arg = 0;
    }

    for(i = 0; i < IPC_ST_MAX_FUNCTIONS; i++)
    {
        G_ipc_st_handlers[i].handler = ipc_default_handler;
        G_ipc_st_handlers[i].arg = 0;
    }

#endif


    return IPC_RC_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
/// Enables IPC communications.
///
int ipc_enable(void)
{
    int rc;
    ipc_target_t* my_cbufs;

    do
    {
        // Install the IPI interrupt handler for this processor
        rc = KERN_IRQ_HANDLER_SET(IPC_GET_IRQ(OCCHW_INST_ID_SELF),
                                  ipc_irq_handler,
                                  0,
                                  KERN_CRITICAL);

        if(rc)
        {
            break;
        }

        my_cbufs = &OSD_PTR->ipc_data.targets[OCCHW_INST_ID_SELF];

        //Any messages that were placed on the cbufs before this point
        //are dropped.  Clear any interrupts that might have been raised
        //before this point.
        KERN_IRQ_STATUS_CLEAR(IPC_GET_IRQ(OCCHW_INST_ID_SELF));

        // Clear and open up all receive buffers for this processor
        // by setting the read counts equal to the write counts
        my_cbufs->counts.reads.counts64 = my_cbufs->counts.writes.counts64;

        // Unmask the IPI interrupt for this processor
        KERN_IRQ_ENABLE(IPC_GET_IRQ(OCCHW_INST_ID_SELF));

        //Allow us to send out new commands
        G_ipc_enabled = 1;
    }
    while(0);

    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// Disable recieving new IPC commands for a processor.
///
int ipc_disable(uint32_t target_id)
{
    int i;
    int rc = IPC_RC_SUCCESS;
    ipc_target_t* target_cbufs;
    KERN_MACHINE_CONTEXT    ctx;

    do
    {
        // Check for invalid target ID
        if(target_id > OCCHW_INST_ID_MAX)
        {
            rc = IPC_RC_INVALID_TARGET_ID;
            break;
        }

        // Prevent us from sending out new commands
        if(target_id == OCCHW_INST_ID_SELF)
        {
            G_ipc_enabled = 0;
        }

        //disable interrupts to prevent the IPC interrupt handler or other threads
        //on this instance from interrupting us and changing the read count or
        //interrupt mask bits under our feet.
        KERN_CRITICAL_SECTION_ENTER(KERN_CRITICAL, &ctx);

        //mask off the IPC interrupt for the target (this is mostly for the case
        //where we are diabling IPC for ourselves).
        KERN_IRQ_DISABLE(IPC_GET_IRQ(target_id));

        target_cbufs = &OSD_PTR->ipc_data.targets[target_id];

        // Make each cbuf appear to be more than full. This signals to
        // senders that the buffer is not just full, but blocked.  When
        // the sender sees this it knows not to place more messages on the cbuf.
        // NOTE: we are updating the read register, which is allowed if the cbuf
        // is owned by the instance this code is running on OR if the instance
        // is known to be halted.
        for(i = 0; i <= OCCHW_INST_ID_MAX; i++)
        {
            target_cbufs->counts.reads.counts8[i] =
                target_cbufs->counts.writes.counts8[i] - (IPC_CBUF_SIZE * 2);
        }

        KERN_CRITICAL_SECTION_EXIT(&ctx);

    }
    while(0);

    return rc;
}

///////////////////////////////////////////////////////////////////////////////
/// Sets the IPC IRQ Done Hook function
///
void ipc_set_done_hook(void* f)
{
    G_ipc_irq_handler_done_hook = f;
}
