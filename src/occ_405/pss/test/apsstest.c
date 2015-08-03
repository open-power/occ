/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/pss/test/apsstest.c $                             */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

#include "ssx.h"
#include "ssx_io.h"
#include "simics_stdio.h"
#include <thread.h>
#include <threadSch.h>
#include <errl.h>
#include <apss.h>

// Period in which to run #timer_routine
#define TIMER_INTERVAL (SsxInterval) SSX_MICROSECONDS(5000)
SsxSemaphore prcd_sem;

int g_j = 0;
int g_k = 0;

SimicsStdio simics_stdout;
SimicsStdio simics_stderr;

uint8_t noncritical_stack[NONCRITICAL_STACK_SIZE];
uint8_t critical_stack[CRITICAL_STACK_SIZE];

SsxTimer G_test_timer;

extern void timer_routine(void *private);
extern void rtloop_ocb_init(void);

// Function Specification
//
// Name:  pgp_validation_ssx_main_hook
//
// Description:
//
// End Function Specification
void pgp_validation_ssx_main_hook(void)
{

}

// Function Specification
//
// Name:  Cmd_Hndl_thread_routine
//
// Description:
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
        //printf("Thread A running");
    }while(1);

}

// Function Specification
//
// Name:  App_thread_routine
//
// Description:
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
// Name:  Thermal_Monitor_thread_routine
//
// Description:
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
        //printf("Thread A running");
    }while(1);

}

// Function Specification
//
// Name:  Hlth_Monitor_thread_routine
//
// Description:
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
        //printf("Thread A running");
    }while(1);

}

// Function Specification
//
// Name:  FFDC_thread_routine
//
// Description:
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
        //printf("Thread A running");
    }while(1);

}

/** PRCD Thread
 *
 * This thread loops as the highest priority thread, where it currently
 * just
 *
 */
// Function Specification
//
// Name:  prcd_thread_routine
//
// Description:
//
// End Function Specification
void prcd_thread_routine(void *private)
{
  while(1)
  {
    // Just sit here until this semaphore is posted, which will never happen.
    ssx_semaphore_pend(&prcd_sem, SSX_WAIT_FOREVER);

    // Only trace the first XX times that this function loops
    if(g_j < 20)
    {
      g_k = 0;
      g_j++;

    }
  }
}


// Function Specification
//
// Name:  apss_test_pwr_meas
//
// Description: Request the full power measurement data in a single function call for TESTING ONLY
//
// End Function Specification
extern PoreEntryPoint pore_test; // Sleep for specified amount of time...
void apss_test_pwr_meas(void)
{
  task_apss_start_pwr_meas();

  // Schedule GPE program to delay to ensure the data is available... (BLOCKING)
  // bad: 48, good: 56
  PoreFlex test_request;
  DEBUG_PRINTF(("apss_test_pwr_meas: delay...\n"));
  pore_flex_create(&test_request,
                   &pore_gpe0_queue,
                   (void*)pore_test,         // entry_point
                   (uint32_t)56,             // entry_point argument
                   NULL,                     // callback,
                   NULL,                     // callback arg
                   ASYNC_REQUEST_BLOCKING);  // options
  pore_flex_schedule(&test_request);
  DEBUG_PRINTF(("apss_test_pwr_meas: delay complete\n"));

  task_apss_continue_pwr_meas();

  task_apss_complete_pwr_meas();

} // end apss_test_pwr_meas()




/** Main Thread
 *
 * This thread currently just loops as the lowest priority thread, handling
 * the lowest priority tasks.
 *
 */
// Function Specification
//
// Name: main_thread_routine
//
// Description:
//
// End Function Specification
void main_thread_routine(void *private)
{
  // Start the critical 250uS timer
  ssx_timer_schedule(&timer, 1, TIMER_INTERVAL);

  // Initialize APSS
  errlHndl_t l_errl = apss_initialize();
  if (l_errl)
  {
    // init failed, attempt one more time before giving up
    printf("ERROR: apss_initialize failed! (retrying)\n");
    setErrlSevToInfo(l_errl);
    commitErrl(&l_errl);

    l_errl = apss_initialize();
    if (l_errl)
    {
      printf("ERROR: apss_initialize failed again! (OCC will be reset)");
      commitErrl(&l_errl);

      // $TODO - Request Reset
    }
  }

  // Attempt to retrieve 3 sets of measurements
  printf("Attempting to gather power measurements\n");
  apss_test_pwr_meas();
  apss_test_pwr_meas();
  apss_test_pwr_meas();

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
// Name: dump_thread_info
//
// Description:
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

/** Entry point for OCC execution
 *
 * main() currently initalizes our trace buffer along with creating threads
 * and timers for execution.  Note that once main runs ssx_start_threads, we
 * never return as the SSX kernel takes over.
 *
 */
// Function Specification
//
// Name: main
//
// Description:
//
// End Function Specification
int main(int argc, char **argv)
{
    //locals
    errlHndl_t  l_errl = NULL;

    // Initialize Trace Buffers immediately, so they can be used
    // from this point on.
    //TRAC_init_buffers();

    // Initialize stdout so we can do printf from within simics env
    simics_stdout_create(&simics_stdout);
    simics_stderr_create(&simics_stderr);
    stdout = (FILE *)(&simics_stdout);
    stderr = (FILE *)(&simics_stderr);

    printf("Inside apsstest main\n");

    // Initialize SSX Stacks (note that this also reinitializes the time base to 0)
    ssx_initialize((SsxAddress)noncritical_stack, NONCRITICAL_STACK_SIZE,
           (SsxAddress)critical_stack, CRITICAL_STACK_SIZE,
           0);

    // Create Global Semaphores
    ssx_semaphore_create(&prcd_sem, 0, 13);

    // Create Threads
    ssx_thread_create(&main_thread,
              main_thread_routine,
              (void *)0,
              (SsxAddress)main_thread_stack,
              THREAD_STACK_SIZE,
              1);

    // Create Threads
    ssx_thread_create(&prcd_thread,
              prcd_thread_routine,
              (void *)0,
              (SsxAddress)prcd_thread_stack,
              THREAD_STACK_SIZE,
              0);

    // Make Threads runnable
    ssx_thread_resume(&main_thread);
    ssx_thread_resume(&prcd_thread);

    //Initialize the thread scheduler
    l_errl = initThreadScheduler();

    if( l_errl )
    {
        // Trace and commit error

        // TODO add trace

        // commit log
        // NOTE: log should be deleted by reader mechanism
        commitErrl( &l_errl );
    }

    //kick off timer
    ssx_timer_create(&G_test_timer, dump_thread_info, 0);
    ssx_timer_schedule(&G_test_timer, 1, 500000000);


    // Enter SSX Kernel
    ssx_start_threads();

    return 0;
}

