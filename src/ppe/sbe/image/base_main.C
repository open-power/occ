/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/image/base_main.C $                               */
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
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file base_main.c
/// \brief base program that creates and starts a thread
///
/// This file is a placeholder code in order to compile. I will be replaced in future by the code placed into the .text section.

extern "C" {
#include "pk.h"
#include "pk_trace.h"
#include "base_ppe_demo.h"
#include "sbe_xip_image.h"
}
#define KERNEL_STACK_SIZE   256
#define MAIN_THREAD_STACK_SIZE  256

uint8_t     G_kernel_stack[KERNEL_STACK_SIZE];
uint8_t     G_main_thread_stack[MAIN_THREAD_STACK_SIZE];
PkThread    G_main_thread;

// A simple thread that just increments a local variable and sleeps
void main_thread(void* arg)
{

    while(1)
    {

        pk_sleep(PK_SECONDS(1));

    }
}


// The main function is called by the boot code (after initializing some
// registers)
int main(int argc, char **argv)
{
    // initializes kernel data (stack, threads, timebase, timers, etc.)
    pk_initialize((PkAddress)G_kernel_stack,
                  KERNEL_STACK_SIZE,
                  0,
                  500000000);


    //Initialize the thread control block for G_main_thread
    pk_thread_create(&G_main_thread,
                      (PkThreadRoutine)main_thread,
                      (void*)NULL,
                      (PkAddress)G_main_thread_stack,
                      (size_t)MAIN_THREAD_STACK_SIZE,
                      (PkThreadPriority)1);



    //Make G_main_thread runnable
    pk_thread_resume(&G_main_thread);

    // Start running the highest priority thread.
    // This function never returns
    pk_start_threads();
    
    return 0;
}
