/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/sbefw/sbemain.C $                                 */
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

/* @file: ppe/sbe/sbefw/sbemain.C
 *
 * @brief This file does the following
 *        - SBE Application Main entry point
 *        - PK initialization
 *        - Thread initialization
 *        - Semaphore initialization
 *        - IRQ setup
 *        - Scheduling of the threads and
 *        - Starting of the control loop code flow
 *
 */


#include "sbeexeintf.H"
#include "sbetrace.H"


////////////////////////////////////////////////////////////////
// @brief Global semaphores
////////////////////////////////////////////////////////////////
PkSemaphore g_sbeSemCmdRecv;
PkSemaphore g_sbeSemCmdProcess;
PkSemaphore g_sbeSemFifoReset;

////////////////////////////////////////////////////////////////
// @brief Stacks for Non-critical Interrupts and Threads
////////////////////////////////////////////////////////////////
uint8_t g_sbe_Kernel_NCInt_stack[SBE_NONCRITICAL_STACK_SIZE];
uint8_t g_sbeCommandReceiver_stack[SBE_THREAD_CMD_RECV_STACK_SIZE];
uint8_t g_sbeSyncCommandProcessor_stack[SBE_THREAD_SYNC_CMD_PROC_STACK_SIZE];
uint8_t g_sbeAsyncCommandProcessor_stack[SBE_THREAD_ASYNC_CMD_PROC_STACK_SIZE];

////////////////////////////////////////////////////////////////
// @brief PkThread structure for SBE Command Receiver thread
////////////////////////////////////////////////////////////////
PkThread g_sbeCommandReceiver_thread;

////////////////////////////////////////////////////////////////
// @brief PkThread structure for SBE Synchronous ChipOps
//        processing thread
////////////////////////////////////////////////////////////////
PkThread g_sbeSyncCommandProcessor_thread;

////////////////////////////////////////////////////////////////
//// @brief PkThread structure for SBE Asynchronous ChipOps
////        processing thread
////////////////////////////////////////////////////////////////
PkThread g_sbeAsyncCommandProcessor_thread;


////////////////////////////////////////////////////////////////
// @brief  sbeInitSems - Create the necessary semaphores
//
// @return  PK_OK                          - Success
//         PK_INVALID_SEMAPHORE_AT_CREATE - Invalid PkSemaphore
//         PK_INVALID_ARGUMENT_SEMAPHORE  - max_count is non-zero
//                               and less than the initial_count
////////////////////////////////////////////////////////////////
uint32_t sbeInitSems(void)
{
    SBE_ENTER("sbeInitSems");
    int l_rc = PK_OK;

    do
    {
        l_rc = pk_semaphore_create(&g_sbeSemCmdRecv, 0, 1);
        if (l_rc)
        {
            break;
        }
        l_rc = pk_semaphore_create(&g_sbeSemCmdProcess, 0, 1);
        if (l_rc)
        {
            break;
        }
        l_rc = pk_semaphore_create(&g_sbeSemFifoReset, 0, 1);
        if (l_rc)
        {
            break;
        }
    } while (false);

    if (l_rc)
    {
        SBE_ERROR ("pk_semaphore_create, rc=[%d]", l_rc);
    }
    return l_rc;
}

////////////////////////////////////////////////////////////////
// @brief  createAndResumeThreadHelper
//            - Create and resume the given thread
//
// @param[in/out] io_thread  A pointer to an PkThread structure to initialize
// @param[in]     i_thread_routine The subroutine that implements the thread
// @param[in/out] io_arg     Private data to be passed as the argument to the
//                              thread routine when it begins execution
// @param[in]     i_stack    The stack space of the thread
// @param[in]     i_stack_size The size of the stack in bytes
// @param[in]     i_priority The initial priority of the thread
//
// @return        PK_OK Successfully created and resumed the thread
//
// @return        PK_INVALID_THREAD_AT_CREATE io_thread is null
// @return        PK_INVALID_ARGUMENT_THREAD1 i_thread_routine is null
// @return        PK_INVALID_ARGUMENT_THREAD2 i_priority is invalid
// @return        PK_INVALID_ARGUMENT_THREAD3 the stack area wraps around
//                                      the end of memory.
// @return        PK_STACK_OVERFLOW           The stack area at thread creation
//                                      is smaller than the min safe size
// @return        PK_INVALID_THREAD_AT_RESUME1 io_thread is null (unlikely)
// @return        PK_INVALID_THREAD_AT_RESUME2 The thread is not active,
//                                      i.e. has completed or been deleted,
// @return        PK_PRIORITY_IN_USE_AT_RESUME Another thread is already
//                                      mapped at the priority of the thread
////////////////////////////////////////////////////////////////
uint32_t createAndResumeThreadHelper(PkThread    *io_pThread,
                                PkThreadRoutine   i_thread_routine,
                                void             *io_pArg,
                                PkAddress         i_stack,
                                size_t            i_stack_size,
                                sbeThreadPriorities  i_priority)
{
    int l_rc = PK_OK;

    // Thread creation
    l_rc =  pk_thread_create(io_pThread,
                             i_thread_routine,
                             io_pArg,
                             i_stack,
                             i_stack_size,
                             (PkThreadPriority)i_priority);
    if(l_rc == PK_OK)
    {
        // resume the thread once created
        l_rc = pk_thread_resume(io_pThread);
    }

    // Check for errors creating or resuming the thread
    if(l_rc != PK_OK)
    {
        SBE_ERROR ("Failure creating/resuming thread, rc=[%d]", l_rc);
    }

    return l_rc;
}

////////////////////////////////////////////////////////////////
// @brief   sbeInitThreads
//          Create the resume all the firmware threads
//
// @return  See createAndResumeThreadHelper for more details
////////////////////////////////////////////////////////////////
int sbeInitThreads(void)
{
    // Locals
    uint32_t l_rc         = PK_OK;

    do
    {
        // Initialize Command receiver thread
        l_rc = createAndResumeThreadHelper(&g_sbeCommandReceiver_thread,
                            sbeCommandReceiver_routine,
                            (void *)0,
                            (PkAddress)g_sbeCommandReceiver_stack,
                            SBE_THREAD_CMD_RECV_STACK_SIZE,
                            THREAD_PRIORITY_5);
        if (l_rc)
        {
            break;
        }

        // Initialize Synchronous Command Processor thread
        l_rc = createAndResumeThreadHelper(&g_sbeSyncCommandProcessor_thread,
                            sbeSyncCommandProcessor_routine,
                            (void *)0,
                            (PkAddress)g_sbeSyncCommandProcessor_stack,
                            SBE_THREAD_SYNC_CMD_PROC_STACK_SIZE,
                            THREAD_PRIORITY_7);
        if (l_rc)
        {
            break;
        }

        // Initialize Asynchronous Command Processor thread
        l_rc = createAndResumeThreadHelper(&g_sbeAsyncCommandProcessor_thread,
                            sbeAsyncCommandProcessor_routine,
                            (void *)0,
                            (PkAddress)g_sbeAsyncCommandProcessor_stack,
                            SBE_THREAD_ASYNC_CMD_PROC_STACK_SIZE,
                            THREAD_PRIORITY_6);
        if (l_rc)
        {
            break;
        }
    } while (false);

    // If there are any errors initializing the threads
    if( l_rc )
    {
        SBE_ERROR ("Error Initializing a thread, rc=[%d]", l_rc);
    }

    return l_rc;
}

////////////////////////////////////////////////////////////////
// @brief - main : SBE Application main
////////////////////////////////////////////////////////////////
uint32_t main(int argc, char **argv)
{
    SBE_TRACE("Enter SBE main");
    int l_rc = 0;

    // @TODO via RTC : 128818
    //       Explore on reclaiming the stack
    //       used by this Initialization code

    do
    {
        // initializes kernel data -
        // stack, threads, timebase, timers, etc.
        l_rc = pk_initialize((PkAddress)g_sbe_Kernel_NCInt_stack,
             SBE_NONCRITICAL_STACK_SIZE,
             0,
             500000000); // @TODO via RTC : 128819
                         //       Need to obtain at Runtime, a new attribute?
        if (l_rc)
        {
            break;
        }

        SBE_DEBUG("Completed PK init");

        // Initialize the semaphores
        l_rc = sbeInitSems();
        if (l_rc)
        {
            break;
        }

        // Initialize SBE control loop threads
        l_rc = sbeInitThreads();
        if (l_rc)
        {
            break;
        }

        // Setup SBE PPE IRQs
        l_rc = sbeIRQSetup();
        if (l_rc)
        {
            break;
        }

        // Start running the highest priority thread.
        // This function never returns
        pk_start_threads();

    } while (false);

    return l_rc;
}
