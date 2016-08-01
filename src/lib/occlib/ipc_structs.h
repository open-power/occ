/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/occlib/ipc_structs.h $                                */
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
#ifndef __IPC_STRUCTS_H__
#define __IPC_STRUCTS_H__

/// \file ipc_structs.h
/// \brief Common header for Interprocessor Communication structures in the
///        OCC complex
///
#include "kernel.h"         //kernel wrapper macros
#include "occhw_common.h"
#include "ipc_macros.h"


#ifndef IPC_CBUF_SIZE
#define IPC_CBUF_SIZE 8 //number of messages must be a power of 2
#endif

//the maximum number of multi-target functions (common functions)
//This can be overriden in a global header that's included by all occ images
//so that it is the same across all processors.
#ifndef IPC_MT_MAX_FUNCTIONS
#define IPC_MT_MAX_FUNCTIONS 8
#endif

//the maximum number of single-target functions (processor-specific functions)
//This can be overridden in a local header file and does not need to be the
//same across all processors.
#ifndef IPC_ST_MAX_FUNCTIONS 
#define IPC_ST_MAX_FUNCTIONS 16
#endif

/// IPC return codes
#define IPC_RC_SUCCESS               0
#define IPC_RC_CMD_FAILED            0x00472000
#define IPC_RC_BUFFER_FULL           0x00472001
#define IPC_RC_SELF_BLOCKED          0x00472002
#define IPC_RC_TARGET_BLOCKED        0x00472003
#define IPC_RC_MSG_ACTIVE            0x00472004
#define IPC_RC_INVALID_FUNC_ID       0x00472005
#define IPC_RC_CMD_NOT_SUPPORTED     0x00472006
#define IPC_RC_INVALID_TARGET_ID     0x00472007
#define IPC_RC_INVALID_ARG           0x00472008
#define IPC_RC_MSG_NOT_ACTIVE        0x00472009
#define IPC_RC_NO_MSG                0x0047200a
#define IPC_RC_TIMEOUT               0x0047200b

/// IPC Message Flags
#define IPC_FLAG_MT             0x00000100
#define IPC_FLAG_RESPONSE       0x00000200
#define IPC_FLAG_VALID          0x00000400
#define IPC_FLAG_ACTIVE         0x00000800

// Function ID field masks
#define IPC_TARGET_MASK         0xff000000
#define IPC_SENDER_MASK         0x00ff0000
#define IPC_FLAGS_MASK          0x0000ff00
#define IPC_INDEX_MASK          0x000000ff

#define IPC_TARGET_SHIFT        24

#define IPC_CBUF_COUNT_BYTES        1
#define IPC_CBUF_COUNT_BITS         8

#ifndef __ASSEMBLER__

// If an occ application does not wish to use IPC then it should not
// define the GLOBAL_CFG_USE_IPC macro.  This allows IPC to compile
// without errors.
#ifdef GLOBAL_CFG_USE_IPC
#include "ipc_func_ids.h"
#else
IPC_FUNCIDS_TABLE_START
    IPC_FUNCIDS_MT_START
    IPC_FUNCIDS_MT_END
    IPC_FUNCIDS_ST_START(OCCHW_INST_ID_GPE0)
    IPC_FUNCIDS_ST_END(OCCHW_INST_ID_GPE0)
    IPC_FUNCIDS_ST_START(OCCHW_INST_ID_GPE1)
    IPC_FUNCIDS_ST_END(OCCHW_INST_ID_GPE1)
    IPC_FUNCIDS_ST_START(OCCHW_INST_ID_GPE2)
    IPC_FUNCIDS_ST_END(OCCHW_INST_ID_GPE2)
    IPC_FUNCIDS_ST_START(OCCHW_INST_ID_GPE3)
    IPC_FUNCIDS_ST_END(OCCHW_INST_ID_GPE3)
    IPC_FUNCIDS_ST_START(OCCHW_INST_ID_PPC)
    IPC_FUNCIDS_ST_END(OCCHW_INST_ID_PPC)
IPC_FUNCIDS_TABLE_END
#endif /*GLOBAL_CFG_USE_IPC*/

//Statically check that the function tables are large enough
KERN_STATIC_ASSERT(IPC_MT_NUM_FUNCIDS <= IPC_MT_MAX_FUNCTIONS);
KERN_STATIC_ASSERT(IPC_ST_NUM_FUNCIDS <= IPC_ST_MAX_FUNCTIONS);

#ifdef __SSX__
extern KERN_DEQUE G_ipc_deferred_queue;
#endif

struct ipc_msg;
typedef struct ipc_msg ipc_msg_t;

typedef union
{
    uint64_t    counts64;
    uint8_t     counts8[sizeof(uint64_t) / IPC_CBUF_COUNT_BYTES];
} ipc_counts_t;

/// Circular buffer read and write counts are grouped together by target_id
/// so that an interrupt hander can quickly tell which buffer requires
/// service. Each counter is 1 byte.
///
/// Note: index 0 (or most significant byte) is for GPE0
typedef struct
{
    ipc_counts_t    reads;
    ipc_counts_t    writes;
}ipc_rwcounts_t;

typedef struct
{
    ipc_rwcounts_t counts;
    ipc_msg_t*     cbufs[OCCHW_MAX_INSTANCES][IPC_CBUF_SIZE];
    uint8_t        pad[16]; 
}ipc_target_t; //size is 6 x 32 = 192 bytes

/// All of the shared data for IPC is contained in this structure
typedef struct
{
    ipc_target_t      targets[OCCHW_MAX_INSTANCES]; //880 bytes
}ipc_shared_data_t;

//prototype for ipc handlers and callback functions
typedef void (*ipc_msg_handler_t)(ipc_msg_t*, void *);

//function table entry
typedef struct
{
    ipc_msg_handler_t   handler;
    void*               arg;
}ipc_func_table_entry_t;

extern ipc_func_table_entry_t G_ipc_mt_handlers[IPC_MT_MAX_FUNCTIONS];

extern ipc_func_table_entry_t G_ipc_st_handlers[IPC_ST_MAX_FUNCTIONS];

typedef union
{
    struct
    {
        uint32_t    target_id:          8;
        uint32_t    sender_id:          8;
        uint32_t    reserved:           4;
        uint32_t    active_flag:        1;
        uint32_t    valid_flag:         1;
        uint32_t    response_flag:      1;
        uint32_t    multi_target_flag:  1;
        uint32_t    table_index:        8;
    };
    uint32_t    word32;
}ipc_func_id_t;

#define IPC_MSG_DEQUEUE_SZ 8 //expected size of a deque structure
struct ipc_msg
{
    // but this file is shared by both, so use a void* type
    // instead.
    union
    {
        KERN_DEQUE          node;
        uint8_t             pad[IPC_MSG_DEQUEUE_SZ];
    };

    //function ID of the function that the sender wants executed
    volatile ipc_func_id_t  func_id;

    //The IPC return code
    volatile uint32_t       ipc_rc;

    //function to call if this is a response message
    ipc_msg_handler_t       resp_callback;

    //Argument passed into the callback function
    void                    *callback_arg;
};

//Do a static check on the size of KERN_DEQUE.  IPC_MSG_DEQUE_SZ must be <= sizeof(KERN_DEQUE).
//If the compiler fails here then you probably need to update the value of IPC_MSG_DEQUEUE_SZ.
//NOTE: this is needed because ipc_msg_t is used in both PK and SSX environments and there is
//      no guarantee that the size of a deque will be the same in both environments.
KERN_STATIC_ASSERT(sizeof(KERN_DEQUE) <= IPC_MSG_DEQUEUE_SZ);

//A message queue that threads can block on while they wait for new messages.
typedef struct
{
    KERN_DEQUE          msg_head;
    KERN_SEMAPHORE      msg_sem; //posted whenever a new message is queued
}ipc_msgq_t;

#endif /*__ASSEMBLER__*/

#endif  /* __IPC_STRUCTS_H__ */
