/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/sample/sample_main.C $                            */
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

/// \file sample_main.c
/// \brief Sample program that creates and starts a thread
///
/// This file demonstrates how to create a thread and run it.  It also provides
/// an example of how to add traces to the code.
extern "C" {
#include "pk.h"
#include "pk_trace.h"
#include "pk_scom.h"
#include "pk_trace_wrap.h"
}
#define KERNEL_STACK_SIZE   256
#define MAIN_THREAD_STACK_SIZE  256



uint8_t     G_kernel_stack[KERNEL_STACK_SIZE];
uint8_t     G_main_thread_stack[MAIN_THREAD_STACK_SIZE];
PkThread    G_main_thread;

class Thetest {

 public:
    int i;

};

// A simple thread that just increments a local variable and sleeps
void main_thread(void* arg)
{
    //Thetest mytest;

    //mytest.i = 0;

    //std::cout << "sdfds" << std::endl;

    uint16_t a = 0;
 
    pk_trace_wrap("sdfsdf");
    //PK_TRACE("thread started");


    while(1)
    {
        // PK_TRACE can take up to 4 parameters
        // (not including the format string)
        //PK_TRACE("thread seconds = %d", a);
        pk_sleep(PK_SECONDS(1));

        uint64_t i_data = 0x0110;

        putscom(0x33333, 0x11111, i_data);

        getscom(0x33333, 0x11111, &i_data);

        a++;
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

    //PK_TRACE("Kernel init completed");

    //Initialize the thread control block for G_main_thread
    pk_thread_create(&G_main_thread,
                      (PkThreadRoutine)main_thread,
                      (void*)NULL,
                      (PkAddress)G_main_thread_stack,
                      (size_t)MAIN_THREAD_STACK_SIZE,
                      (PkThreadPriority)1);

    //PK_TRACE_BIN("G_main_thread", &G_main_thread, sizeof(G_main_thread));

    //Make G_main_thread runnable
    pk_thread_resume(&G_main_thread);

    //PK_TRACE("Starting thread(s)");

    // Start running the highest priority thread.
    // This function never returns
    pk_start_threads();
    
    return 0;
}
