/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/test/main.c $                                      */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2019                             */
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
#include "pk.h"
#include "ipc_async_cmd.h"
//#include "ipc_func_ids.h"
#include "ipc_structs.h"

#define KERNEL_STACK_SIZE   512
#define THREAD_STACK_SIZE   512

uint64_t G_kernel_stack[KERNEL_STACK_SIZE / 8];
uint64_t G_test_thread_stack[THREAD_STACK_SIZE / 8];
PkThread G_test_thread;


void test_thread(void* arg)
{
    int rc = 0;
    ipc_msgq_t ipc_msg_q;
    ipc_msg_t* ipc_msg;
    KERN_INTERVAL ipc_msg_timeout = 0;

    PK_TRACE("test_thread() started");

    rc = ipc_init();

    if(rc)
    {
        PK_TRACE("ipc_init() failed! rc = %d", rc);
        pk_halt();
    }

    ipc_init_msgq( &ipc_msg_q);

    rc = ipc_register_msgq( IPC_MT_TEST, &ipc_msg_q);

    if (rc)
    {
        PK_TRACE("ipc_register_msgq() failed! rc = %d", rc);
        pk_halt();
    }

    rc = ipc_enable();

    if (rc)
    {
        PK_TRACE("ipc_enable() failed!. rc = %d", rc);
        pk_halt();
    }

    ipc_msg_timeout = PK_MICROSECONDS(1) / 10;

    do
    {
        do
        {
            rc = ipc_msgq_recv( &ipc_msg, &ipc_msg_q, ipc_msg_timeout);

            if (rc && rc != IPC_RC_TIMEOUT && rc != IPC_RC_NO_MSG)
            {
                PK_TRACE("ipc_msgq_recv() failed w/rc=0x%08x", rc);
                pk_halt();
            }
        }
        while (rc == IPC_RC_TIMEOUT || rc == IPC_RC_NO_MSG);

        uint32_t* cmd_data = (uint32_t*)(((ipc_async_cmd_t*)ipc_msg)->cmd_data);
        PK_TRACE("Got message %08x", *cmd_data);

        rc = ipc_send_rsp(ipc_msg, rc);

        if (rc)
        {
            PK_TRACE("ipc_send_rsp() failed! rc = %d", rc);
            pk_halt();
        }
    }
    while(1);
}


int main(int argc, char** argv)
{
    pk_initialize( (PkAddress)G_kernel_stack,
                   KERNEL_STACK_SIZE,
                   0,
                   PPE_TIMEBASE_HZ);

    // Initialize thread control block for each thread.
    pk_thread_create( &G_test_thread,
                      (PkThreadRoutine)test_thread,
                      (void*)NULL,
                      (PkAddress)G_test_thread_stack,
                      (size_t) THREAD_STACK_SIZE,
                      (PkThreadPriority)1);

    // test divide (DERP/DORP) for compile
    PK_TRACE("Divide me %d", (uint32_t)argc / (uint32_t)argv);


    // Make each thread runable
    pk_thread_resume( &G_test_thread);

    // Start thread(s) - this function never returns.
    pk_start_threads();

    return 0;
}
