/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/thread/test/threadtest.c $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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

#include "ssx.h"
#include "ssx_io.h"
#include "simics_stdio.h"
#include <errl.h>
#include <rand.h>
#include <thread.h>
#include <threadSch.h>
#include <appletId.h>

extern void __ssx_boot;

IMAGE_HEADER (G_mainAppImageHdr,__ssx_boot,MAIN_APP_ID,OCC_APLT_TEST);

// Period in which to run timer_routine
#define TIMER_INTERVAL (SsxInterval) SSX_MICROSECONDS(5000)

SsxSemaphore prcd_sem;

int g_j = 0;
int g_k = 0;
/*----------------------------------------------------------------------------*/
/* noncrtical/critical stack                                                  */
/*----------------------------------------------------------------------------*/
uint8_t noncritical_stack[NONCRITICAL_STACK_SIZE];
uint8_t critical_stack[CRITICAL_STACK_SIZE];
SimicsStdio simics_stdout;
SimicsStdio simics_stderr;

SsxTimer G_test_timer;

extern void timer_routine(void *private);
extern void rtloop_ocb_init(void);


// Function Specification
//
// Name: pgp_validation_ssx_main_hook
//
// Description: TODO Add description
//
// End Function Specification
void pgp_validation_ssx_main_hook(void)
{

}

// Function Specification
//
// Name: Cmd_Hndl_thread_routine
//
// Description: TODO Add description
//
// End Function Specification
//TODO placeholder
void Cmd_Hndl_thread_routine(void *arg)
{
     do
    {
        int x=0;
        for(x=0; x < 1000; x++)
        {

        }
    }while(1);

}

// Function Specification
//
// Name: App_thread_routine
//
// Description: TODO Add description
//
// End Function Specification
void App_thread_routine(void *arg)
{
    int z=0;
     do
    {
        int x=0;
        for(x=0; x < 10000; x++)
        {
            z++;
        }
    }while(1);

}

// Function Specification
//
// Name: Thermal_Monitor_thread_routine
//
// Description: TODO Add description
//
// End Function Specification
void Thermal_Monitor_thread_routine(void *arg)
{
       int z=0;
     do
    {
        int x=0;
        for(x=0; x < 10000; x++)
        {
            z++;
        }
    }while(1);

}

// Function Specification
//
// Name: Hlth_Monitor_thread_routine
//
// Description: TODO Add description
//
// End Function Specification
void Hlth_Monitor_thread_routine(void *arg)
{
       int z=0;
     do
    {
        int x=0;
        for(x=0; x < 10000; x++)
        {
            z++;
        }
    }while(1);

}

// Function Specification
//
// Name: FFDC_thread_routine
//
// Description: TODO Add description
//
// End Function Specification
void FFDC_thread_routine(void *arg)
{
       int z=0;
     do
    {
        int x=0;
        for(x=0; x < 10000; x++)
        {
            z++;
        }
    }while(1);

}

// Function Specification
//
// Name: main_thread_routine
//
// Description: This thread currently just looks as the lowest priority thread,
//              handling the lowest priority tasks.
//
// End Function Specification
void main_thread_routine(void *private)
{
  // Start the critical 250uS timer
   ssx_timer_schedule(&timer, 1, TIMER_INTERVAL);

  while(1)
  {
    // Only trace the first XX times that this function loops
    if(g_k < 3)
    {
      g_k++;

      // TRACE: Main Thread
    }

    // Sleep for 1000 before we run the loop again
    ssx_sleep_absolute(1000);
  }
}

// Function Specification
//
// Name: timer_routine
//
// Description: TODO Add description
//
// End Function Specification
void  timer_routine (void *arg)
{

}

// Function Specification
//
// Name: dump_thread_info
//
// Description: TODO Add description
//
// End Function Specification
void dump_thread_info(void *arg)
{

    printf("dumping thread info--------------------------\n");
    int l_rc = 0;
    SsxThreadState l_state = 0;
    SsxThreadPriority l_pri = 0;
    int l_runnable=0;
    int x=0;
    for(x=0; x < THREADS_TO_SCHEDULE; x++)
    {
        l_rc = ssx_thread_info_get(G_scheduledThreads[x],
                                    &l_state,
                                    &l_pri,
                                    &l_runnable);

        printf("Thread %p: State %x priority %x runnable %x rc %x Global index %x\n",G_scheduledThreads[x],
                                                           l_state,
                                                            l_pri,
                                                            l_runnable,
                                                            l_rc,
                                                            G_threadSchedulerIndex);

    }

}

// Function Specification
//
// Name: main
//
// Description: Initializes the trace buffer, along with creating threads and timers
//              for execution. Note that once main runs with ssx_start_threads, we
//              never return as the SSX kernel takes over.
//
// End Function Specification
int main(int argc, char **argv)
{
    // Locals
    errlHndl_t  l_errl = INVALID_ERR_HNDL;

    // Initialize Trace Buffers immediately, so they can be used
    // from this point on.

    // Initialize stdout so we can do printf from within simics env
    simics_stdout_create(&simics_stdout);
    simics_stderr_create(&simics_stderr);
    stdout = (FILE *)(&simics_stdout);
    stderr = (FILE *)(&simics_stderr);

    // Initialize SSX Stacks (note that this also reinitializes the time base to 0)
    ssx_initialize((SsxAddress)noncritical_stack, NONCRITICAL_STACK_SIZE,
                   (SsxAddress)critical_stack, CRITICAL_STACK_SIZE, 0);

    // Create Global Semaphores
    ssx_semaphore_create(&prcd_sem, 0, 13);

    ssx_timer_create(&timer, timer_routine, 0);

    // Create thread with already mapped priority
    // Create Threads
    ssx_thread_create(&main_thread,
              main_thread_routine,
              (void *)0,
              (SsxAddress)main_thread_stack,
              THREAD_STACK_SIZE,
              0);

    // Make Threads runnable
    ssx_thread_resume(&main_thread);

    //Initialize the thread scheduler
    l_errl = initThreadScheduler();

    if(l_errl)
    {
        // TODO Trace the error

        // Commit log
        // NOTE: log should be deleted by reader mechanism
        commitErrl( &l_errl );
    }

    // Kick off timer
    ssx_timer_create(&G_test_timer, dump_thread_info, 0);
    ssx_timer_schedule(&G_test_timer, 1, 500000000);


    // Enter SSX Kernel
    ssx_start_threads();

    return 0;
}

