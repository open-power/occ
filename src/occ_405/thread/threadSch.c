/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/thread/threadSch.c $                              */
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

#include <occ_common.h>
#include <threadSch.h>
#include "ssx.h"
#include "thread_service_codes.h"
#include "occ_service_codes.h"
#include <appletManager.h>
#include <trac.h>
#include <state.h>
#include "cmdh_snapshot.h"

// Numbers of threads to schedule
#define THREADS_TO_SCHEDULE (sizeof(G_scheduledThreads)/sizeof(SsxThread*))

#define THREAD_TIME_SLICE (SsxInterval) SSX_MILLISECONDS(10) // 10ms

// Thread Timer to reprioritize the threads
SsxTimer G_threadSchTimer;

// Index of highest priority thread in G_scheduledThreads
uint16_t G_threadSchedulerIndex = 0;

// Array that holds the threads that need scheduling
SsxThread* G_scheduledThreads[] =
{
    &Main_thread,
    &Cmd_Hndl_thread,
    &App_thread,
    &TestAppletThread,
    &Dcom_thread,
};

// Error log counter for the callback so that only 1 error log is created
uint8_t G_threadSwapErrlCounter = 0;

// Global for the parameter to the applet thread routine
OCC_APLT_TYPE G_apltPdtType = APLT_TYPE_PRODUCT;
OCC_APLT_TYPE G_apltTestType = APLT_TYPE_TEST;

//Thread Stacks
uint8_t main_thread_stack[THREAD_STACK_SIZE];
uint8_t Cmd_hndl_thread_stack[THREAD_STACK_SIZE];
uint8_t App_thread_stack[THREAD_STACK_SIZE];
uint8_t testAppletThreadStack[THREAD_STACK_SIZE];
uint8_t dcomThreadStack[THREAD_STACK_SIZE];

// Our idle thread. See main_thread_routine
SsxThread Main_thread;

// Command handler thread
SsxThread Cmd_Hndl_thread;

// Application manager thread
SsxThread App_thread;

// Test applet thread
SsxThread TestAppletThread;

// Dcom thread
SsxThread Dcom_thread;

// Function Specification
//
// Name: createAndResumeThreadHelper
//
// Description: create and resume thread helper
//
// End Function Specification
int createAndResumeThreadHelper(SsxThread         *io_thread,
                                SsxThreadRoutine  i_thread_routine,
                                void              *io_arg,
                                SsxAddress        i_stack,
                                size_t            i_stack_size,
                                THREAD_PRIORITY   i_priority)
{

    // Locals
    int l_rc = SSX_OK;

    // Thread creation
    l_rc =  ssx_thread_create(io_thread,
                              i_thread_routine,
                              io_arg,
                              i_stack,
                              i_stack_size,
                              (SsxThreadPriority)i_priority);

    //check for errors creating a thread
    if(l_rc != SSX_OK)
    {
        TRAC_ERR("Failure creating thread. rc: 0x%x", -l_rc);
    }
    else
    {
        //resume thread once created
        l_rc = ssx_thread_resume(io_thread);
    }


    return l_rc;
}

// Function Specification
//
// Name:  initThreadScheduler
//
// Description: Init the threads in the scheduler and start the
//              timer.
//
// End Function Specification
void initThreadScheduler(void)
{
    // Locals
    int l_appThreadRc        = SSX_OK;
    int l_cmdThreadRc        = SSX_OK;
    int l_timerRc            = SSX_OK;
    int l_testAppletThreadRc = SSX_OK;
    int l_dcomThreadRc       = SSX_OK;
    int l_snapshotTimerRc    = SSX_OK;

    // Creating threads that need to be scheduled
    // Thread priority range should match scheduled
    //  threads in G_scheduledThreads ie highest priority thread should be
    //  index 0 of G_scheduledThreads

/* TEMP -- NOT USED IN PHASE1
    l_cmdThreadRc = createAndResumeThreadHelper(&Cmd_Hndl_thread,
              Cmd_Hndl_thread_routine,
              (void *)0,
              (SsxAddress)Cmd_hndl_thread_stack,
              THREAD_STACK_SIZE,
              THREAD_PRIORITY_3);
*/
    l_appThreadRc = createAndResumeThreadHelper(&App_thread,
              App_thread_routine,
              (void *)&G_apltPdtType,
              (SsxAddress)App_thread_stack,
              THREAD_STACK_SIZE,
              THREAD_PRIORITY_4);

    l_testAppletThreadRc = createAndResumeThreadHelper(&TestAppletThread,
                 App_thread_routine,
                 (void *)&G_apltTestType,
                 (SsxAddress)testAppletThreadStack,
                 THREAD_STACK_SIZE,
                 THREAD_PRIORITY_5);
/* TEMP -- NOT USED IN PHASE1
   l_dcomThreadRc = createAndResumeThreadHelper(&Dcom_thread,
                 Dcom_thread_routine,
                 (void *)0,
                 (SsxAddress)dcomThreadStack,
                 THREAD_STACK_SIZE,
                 THREAD_PRIORITY_6);
*/
    // Create the thread scheduler timer
    l_timerRc = ssx_timer_create(&G_threadSchTimer, threadSwapcallback, 0);

    // Check for errors creating the timer
    if(l_timerRc == SSX_OK)
    {
        TRAC_INFO("timer created and scheduled");
        //schedule the timer so that it runs every THREAD_TIME_SLICE
        l_timerRc = ssx_timer_schedule(&G_threadSchTimer, 1, THREAD_TIME_SLICE);
    }
    else
    {
        TRAC_INFO("Error creating timer: RC: %d", l_timerRc);
    }
/* TEMP -- NOT USED IN PHASE1
    // Create snapshot timer
    l_snapshotTimerRc = ssx_timer_create(&G_snapshotTimer, cmdh_snapshot_callback, 0);
    // Check for errors creating the timer
    if(l_snapshotTimerRc == SSX_OK)
    {
        // Schedule the timer so that it runs every 30 seconds.
        l_snapshotTimerRc = ssx_timer_schedule(&G_snapshotTimer, 0, SSX_SECONDS(30));
        if (l_snapshotTimerRc != SSX_OK)
        {
            TRAC_ERR("cmdh_snapshot_sync: reseting the snapshot timer failed.");
        }
    }
    else
    {
        TRAC_INFO("Error creating timer: RC: %d", l_snapshotTimerRc);
    }
*/

    // If there are any errors creating the threads or starting the
    // timer create an error log to pass back.
    if(    l_appThreadRc
        || l_testAppletThreadRc
        || l_cmdThreadRc
        || l_dcomThreadRc
        || l_timerRc
        || l_snapshotTimerRc )
    {
// TEMP -- UNRESOLVED TRACE ERROR
/*        TRAC_ERR("Error creating thread: l_appThreadRc: %d, "
                 "l_testAppletThreadRc: %d, l_cmdThreadRc: %d, "
                 "l_dcomThreadRc: %d", l_appThreadRc,l_testAppletThreadRc,
                 l_timerRc,l_cmdThreadRc,l_dcomThreadRc);
*/
        TRAC_ERR("Error starting timers: timerRc: %d, snapshotTimerRc: %d.",
                  l_timerRc, l_snapshotTimerRc);
        // Create error log and log it
        // TODO use correct trace
        tracDesc_t  l_trace = NULL;

        /* @
         * @errortype
         * @moduleid    THRD_MID_INIT_THREAD_SCHDLR
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   Schedule timer return code
         * @userdata2   Snapshot timer return code
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     SSX thread related failure
         */
        errlHndl_t l_rc = createErrl(THRD_MID_INIT_THREAD_SCHDLR,   // ModId
                                     SSX_GENERIC_FAILURE,           // Reasoncode
                                     OCC_NO_EXTENDED_RC,            // Extended reasoncode
                                     ERRL_SEV_UNRECOVERABLE,        // Severity
                                     l_trace,                       // Trace Buf
                                     DEFAULT_TRACE_SIZE,            // Trace Size
                                     l_timerRc,                     // Userdata1
                                     l_snapshotTimerRc);            // Userdata2

        REQUEST_RESET(l_rc);
    }
}


// Function Specification
//
// Name:  threadSwapcallback
//
// Description: a periodic timer callback to swap prorities of scheduled threads
//
// End Function Specification
void threadSwapcallback(void * arg)
{
    // Locals
    int l_rc = SSX_OK;

    // Current location of index in scheduled thread array
    int l_threadAIndex = G_threadSchedulerIndex;

    // If global index == last item swap priorities with 1st
    int l_threadBIndex = (G_threadSchedulerIndex == (THREADS_TO_SCHEDULE-1)) ? 0 : ++G_threadSchedulerIndex;

    // Swap priorities with global index +1
    l_rc = ssx_thread_priority_swap(G_scheduledThreads[l_threadAIndex],G_scheduledThreads[l_threadBIndex]);

    if(l_rc != SSX_OK)
    {
        // TODO trace error

        // Create and commit error log
        if(G_threadSwapErrlCounter == 0)
        {
            // TODO use correct trace
            tracDesc_t l_trace = NULL;

            /*
             * @errortype
             * @moduleid    THRD_MID_THREAD_SWAP_CALLBACK
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   Return code of thread priority swap
             * @userdata2   Current location of index in scheduled thread array
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     SSX thread related failure
             */
            errlHndl_t  l_err = createErrl(
                    THRD_MID_THREAD_SWAP_CALLBACK,  // ModId
                    SSX_GENERIC_FAILURE,            // Reasoncode
                    OCC_NO_EXTENDED_RC,             // Extended reasoncode
                    ERRL_SEV_PREDICTIVE,            // Severity
                    l_trace,                        // Trace Buf
                    DEFAULT_TRACE_SIZE,             // Trace Size
                    l_rc,                           // Userdata1
                    l_threadAIndex                  // Userdata2
                    );

            // Commit log
            // NOTE: Log should be deleted by reader mechanism
            commitErrl( &l_err );

            // Increment errl counter
             G_threadSwapErrlCounter++;

        }// End thread swap counter if
    }
    else
    {
        // Reset counter since it started working again
        G_threadSwapErrlCounter = 0;

        // Set the global to the new location of the highest priority thread
        G_threadSchedulerIndex = l_threadBIndex;
    }
}

