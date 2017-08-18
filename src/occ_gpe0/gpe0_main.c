/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/gpe0_main.c $                                    */
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

/// \file gpe0_main.c
/// \brief Sample program that creates and starts a thread
///
/// This file demonstrates how to create a thread and run it.  It also provides
/// an example of how to add traces to the code.

#include "pk.h"
#include "pk_trace.h"
#include "ipc_api.h"
#include "ipc_ping.h"
#include "gpe_export.h"

/* Checkstop analysis is using at least 700 bytes */
#define KERNEL_STACK_SIZE  1024

uint8_t        G_kernel_stack[KERNEL_STACK_SIZE];

//Point to the GPE shared structure
#define GPE_SHARED_DATA_ADDR 0xFFFB3C00
#define GPE_SHARED_DATA_SIZE 256

gpe_shared_data_t * G_gpe_shared_data = (gpe_shared_data_t*) GPE_SHARED_DATA_ADDR;

extern PkTraceBuffer* g_pk_trace_buf_ptr;

//statically initialize a ping command message
IPC_PING_CMD_CREATE(G_ping_cmd);

//statically initialize an IPC message queue
IPC_MSGQ_CREATE(G_gpe0_test_msgq0);

//statically initialize an IPC message queue message.  Responses to
//this message will automatically be placed on the message queue.
IPC_MSGQ_MSG_CREATE(G_test_msg, IPC_ST_TEST_FUNC0, &G_gpe0_test_msgq0);

// The main function is called by the boot code (after initializing some
// registers)
int main(int argc, char **argv)
{
    int rc;
    uint32_t l_timebase = G_gpe_shared_data->nest_freq_div;

    // Don't initialize with a 0
    if (!l_timebase)
    {
        l_timebase = PPE_TIMEBASE_HZ;
    }

    // Mark the location of the trace buffer in shared data
    G_gpe_shared_data->gpe0_tb_ptr = (uint32_t) g_pk_trace_buf_ptr;
    G_gpe_shared_data->gpe0_tb_sz = sizeof(PkTraceBuffer);

    // initializes kernel data (stack, threads, timebase, timers, etc.)
    pk_initialize((PkAddress)G_kernel_stack,
                  KERNEL_STACK_SIZE,
                  PK_TIMEBASE_CONTINUES,
                  l_timebase);

    PK_TRACE("Kernel init completed, timebase is %d Hz", l_timebase);

    // Disable IPC's and register the IPC interrupt handler 
    rc = ipc_init();
    if(rc)
    {
        PK_TRACE("ipc_init failed with rc = 0x%08x", rc);
        pk_halt();
    }

    // enable IPC's
    rc = ipc_enable();
    if(rc)
    {
        PK_TRACE("ipc_enable failed with rc = 0x%08x", rc);
        pk_halt();
    }

    return 0;
}
