/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/occlib/ipc_macros.h $                                 */
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
#ifndef __IPC_MACROS_H__
#define __IPC_MACROS_H__

/// \file ipc_macros.h
/// \brief Contains macros related to the Interprocessor Communications (IPC)
/// API.
///

///////////////////////////////////////////////////////////////////////////////
/// Retrieves the IRQ number for the specified OCC processor instance
///
#define IPC_GET_IRQ(instance_id) (OCCHW_IRQ_IPI0_HI_PRIORITY + instance_id)

///////////////////////////////////////////////////////////////////////////////
/// Marks the start of the IPC function ID table
///
#define IPC_FUNCIDS_TABLE_START \
    typedef enum \
    {

///////////////////////////////////////////////////////////////////////////////
/// Marks the end of the IPC function ID table
///
#define IPC_FUNCIDS_TABLE_END \
    } ipc_func_enum_t;

///////////////////////////////////////////////////////////////////////////////
/// Marks the start of the IPC multi-target function IDs within the IPC
/// function ID table.
///
#define IPC_FUNCIDS_MT_START \
    IPC_MT_START = (int)((IPC_TARGET_MASK | IPC_FLAG_MT | IPC_FLAG_VALID | (((uint32_t)((uint8_t)OCCHW_INST_ID_SELF)) << 16)) - 1),

///////////////////////////////////////////////////////////////////////////////
/// Marks the end of the IPC multi-target function IDs within the IPC function
/// ID table.
///
#define IPC_FUNCIDS_MT_END \
    IPC_MT_END,

#define IPC_CONCAT_INST(name, inst) name ## inst

///////////////////////////////////////////////////////////////////////////////
/// Marks the start of the IPC single-target function IDs within the IPC
/// ID table.
///
/// \param target_id The instance ID of the processor that the following
/// function IDs will target.
///
/// Each processor has it's own set of single-target function IDs.  Messages
/// that are initialized with these function ID's can only be sent to the
/// processor specified by \a target_id.
///
#define IPC_FUNCIDS_ST_START(target_id) \
    IPC_CONCAT_INST(IPC_ST_START_, target_id) = \
            (int)(((((uint32_t)target_id) << 24) | IPC_FLAG_VALID ) - 1),

///////////////////////////////////////////////////////////////////////////////
/// Marks the end of the IPC single-target function IDs for the specified
/// target ID, \a target_id.
///
#define IPC_FUNCIDS_ST_END(target_id) \
    IPC_CONCAT_INST(IPC_ST_END_, target_id),

///////////////////////////////////////////////////////////////////////////////
/// Create an IPC function ID.
///
/// \param The name of the IPC function ID
///
/// This macro should only be used inside the IPC function ID table.  Under
/// the covers, an enum with a name of \a name is created.
///
#define IPC_FUNC_ID(name) \
    name,

///////////////////////////////////////////////////////////////////////////////
#define IPC_MT_NUM_FUNCIDS \
    ((IPC_MT_END - IPC_MT_START) - 1)

///////////////////////////////////////////////////////////////////////////////
#define IPC_ST_TARGET_NUM_FUNCIDS(target_id) \
    ((IPC_CONCAT_INST(IPC_ST_END_, target_id) - IPC_CONCAT_INST(IPC_ST_START_, target_id)) - 1)

///////////////////////////////////////////////////////////////////////////////
#define IPC_ST_NUM_FUNCIDS IPC_ST_TARGET_NUM_FUNCIDS(OCCHW_INST_ID_SELF)

///////////////////////////////////////////////////////////////////////////////
/// Macros for statically initializing the IPC function tables
#ifdef STATIC_IPC_TABLES

#define IPC_MT_FUNC_TABLE_START \
    ipc_func_table_entry_t G_ipc_mt_handlers[IPC_MT_MAX_FUNCTIONS] = \
            {

#define IPC_ST_FUNC_TABLE_START \
    ipc_func_table_entry_t G_ipc_st_handlers[IPC_ST_MAX_FUNCTIONS] = \
            {

#define IPC_HANDLER(func, arg) \
    {func, arg},

#define IPC_HANDLER_DEFAULT \
    {ipc_default_handler, 0},

#define IPC_MSGQ_HANDLER(msgq_ptr) \
    {ipc_msgq_handler, msgq_ptr},

#define IPC_MT_FUNC_TABLE_END \
    };

#define IPC_ST_FUNC_TABLE_END \
    };

#else

#define IPC_MT_FUNC_TABLE_START

#define IPC_ST_FUNC_TABLE_START

#define IPC_HANDLER(func, arg)

#define IPC_HANDLER_DEFAULT

#define IPC_MSGQ_HANDLER(msgq_ptr)

#define IPC_MT_FUNC_TABLE_END

#define IPC_ST_FUNC_TABLE_END

#endif /*STATIC_IPC_TABLES*/

///////////////////////////////////////////////////////////////////////////////
/// Convenience macro for defering handling of a command or
/// response in a noncritical interrupt context.  This was
/// specifically added for ipc functions that need to call
/// ssx functions on the 405. (ssx functions can not be called
/// inside a critical interrupt context).
#ifdef __SSX__
#define IPC_DEFER_TO_NONCRITICAL(ipc_msg) \
    { \
        ssx_deque_push_back(&G_ipc_deferred_queue, &ipc_msg->node); \
        ssx_irq_status_set(OCCHW_IRQ_ASYNC_IPI, 1); \
    }

#else
#define IPC_DEFER_TO_NONCRITICAL(ipc_msg)
#endif

///////////////////////////////////////////////////////////////////////////////
/// Determine if an IPC function ID is a multi-target ID
///////////////////////////////////////////////////////////////////////////////
#define IPC_FUNCID_IS_MT(funcid) (IPC_FLAG_MT & (funcid))

///////////////////////////////////////////////////////////////////////////////
/// Set the target ID of a multi-target function ID
/// Sets the target to be invalid if it's not a multi-target function id
///////////////////////////////////////////////////////////////////////////////
#define IPC_SET_MT_TARGET(funcid, targetid) \
    (((funcid) & IPC_FLAG_MT)?  \
     (((targetid) << IPC_TARGET_SHIFT)  | ((funcid) & ~(IPC_TARGET_MASK))): \
     (IPC_TARGET_MASK | (funcid)))

///////////////////////////////////////////////////////////////////////////////
/// Retrieve the target ID from an IPC function id
///////////////////////////////////////////////////////////////////////////////
#define IPC_GET_TARGET_ID(funcid) (((uint32_t)(funcid)) >> IPC_TARGET_SHIFT)

#endif /*__IPC_MACROS_H__*/
