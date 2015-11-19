/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe1_main.c $                                    */
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

/// \file gpe1_main.c
/// \brief Sample program that creates and starts a thread
///
/// This file demonstrates how to create a thread and run it.  It also provides
/// an example of how to add traces to the code.

#include "pk.h"
#include "ipc_api.h"

#define KERNEL_STACK_SIZE  256

uint8_t        G_kernel_stack[KERNEL_STACK_SIZE];


// The main function is called by the boot code (after initializing some
// registers)
int main(int argc, char **argv)
{
    int rc;

    // initializes kernel data (stack, threads, timebase, timers, etc.)
    pk_initialize((PkAddress)G_kernel_stack,
                  KERNEL_STACK_SIZE,
                  PK_TIMEBASE_CONTINUES,
                  PPE_TIMEBASE_HZ);

    PK_TRACE("Kernel init completed");

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
