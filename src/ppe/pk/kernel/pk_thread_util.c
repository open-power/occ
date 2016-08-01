/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/kernel/pk_thread_util.c $                          */
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

/// \file pk_thread_util.c
/// \brief PK thread utility APIs
///
///  The entry points in this file are considered extra routines that will
///  only be included in a PK application that enables threads and uses at
///  least one of these interfaces.

#include "pk.h"
#include "pk_thread.h"

/// Get information about a thread.
///
/// \param thread A pointer to the PkThread to query
///
/// \param state The value returned through this pointer is the current state
/// of the thread; See \ref pk_thread_states. The caller can set this
/// parameter to the null pointer (0) if this information is not required.
///
/// \param priority The value returned through this pointer is the current
/// priority of the thread.  The caller can set this parameter to the null
/// pointer (0) if this information is not required.
///
/// \param runnable The value returned through this pointer is 1 if the thread
/// is in state PK_THREAD_STATE_MAPPED and is currently in the run queue
/// (i.e., neither blocked on a semaphore nor sleeping), otherwise 0. The
/// caller can set this parameter to the null pointer (0) if this information
/// is not required.
///
/// The information returned by this API can only be guaranteed consistent if
/// the API is called from a critical section.
///
/// Return values other than PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion
///
/// \retval -PK_INVALID_THREAD_AT_INFO The \a thread is a null (0) pointer.

int
pk_thread_info_get(PkThread         *thread,
                    PkThreadState    *state,
                    PkThreadPriority *priority,
                    int               *runnable)
{
    if (PK_ERROR_CHECK_API) {
        PK_ERROR_IF(thread == 0, PK_INVALID_THREAD_AT_INFO);
    }

    if (state) {
        *state = thread->state;
    }
    if (priority) {
        *priority = thread->priority;
    }
    if (runnable) {
        *runnable = ((thread->state == PK_THREAD_STATE_MAPPED) &&
                     __pk_thread_queue_member(&__pk_run_queue,
                                               thread->priority));
    }
    return PK_OK;
}


/// Change the priority of a thread.
///
/// \param thread The thread whose priority will be changed
///
/// \param new_priority The new priority of the thread
///
/// \param old_priority The value returned through this pointer is the
/// old priority of the thread prior to the change. The caller can set
/// this parameter to the null pointer (0) if this information is not
/// required.
///
/// Thread priorities can be changed by the \c pk_thread_priority_change()
/// API. This call will fail if the thread pointer is invalid or if the thread
/// is mapped and the new priority is currently in use.  The call will succeed
/// even if the \a thread is suspended, completed or deleted.  The
/// application-level scheduling algorithm is completely responsible for the
/// correctness of the application in the event of suspended, completed or
/// deleted threads.
///
/// Return values other than PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion, including the redundant case of
/// attempting to change the priority of the thread to its current priority.
///
/// \retval -PK_INVALID_THREAD_AT_CHANGE The \a thread is null (0) or 
/// otherwise invalid.
///
/// \retval -PK_INVALID_ARGUMENT_THREAD_CHANGE The \a new_priority is invalid.
///
/// \retval -PK_PRIORITY_IN_USE_AT_CHANGE The \a thread is mapped and the \a
/// new_priority is currently in use by another thread.

int 
pk_thread_priority_change(PkThread         *thread,
                           PkThreadPriority new_priority,
                           PkThreadPriority *old_priority)
{
    PkMachineContext ctx;
    PkThreadPriority priority;

    if (PK_ERROR_CHECK_API) {
        PK_ERROR_IF(thread == 0, PK_INVALID_THREAD_AT_CHANGE);
        PK_ERROR_IF(new_priority > PK_THREADS, 
        PK_INVALID_ARGUMENT_THREAD_CHANGE);
    }

    pk_critical_section_enter(&ctx);

    priority = thread->priority;

    if (priority != new_priority) {

        if (!__pk_thread_is_mapped(thread)) {

            thread->priority = new_priority;

        } else {

            if (PK_ERROR_CHECK_API) {
                PK_ERROR_IF_CRITICAL(__pk_priority_map[new_priority] != 0,
                                      PK_PRIORITY_IN_USE_AT_CHANGE,
                                      &ctx);
            }

            __pk_thread_unmap(thread);
            thread->priority = new_priority;
            __pk_thread_map(thread);
            __pk_schedule();
        }
    }

    if (old_priority) {
        *old_priority = priority;
    }

    pk_critical_section_exit(&ctx);

    return PK_OK;
}


/// Return a pointer to the thread (if any) mapped at a given priority.
///
/// \param priority The thread priority of interest
///
/// \param thread The value returned through this pointer is a pointer to the
/// thread currently mapped at the given priority level.  If no thread is
/// mapped, or if the \a priority is the priority of the idle thread, the
/// pointer returned will be null (0).
///
/// The information returned by this API can only be guaranteed consistent if
/// the API is called from a critical section.
///
/// Return values other than PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion.
///
/// \retval -PK_INVALID_ARGUMENT_THREAD_PRIORITY The \a priority is invalid 
/// or the \a thread parameter is null (0). 

int
pk_thread_at_priority(PkThreadPriority priority,
                       PkThread         **thread)
{
    if (PK_ERROR_CHECK_API) {
        PK_ERROR_IF((priority > PK_THREADS) || (thread == 0),
                     PK_INVALID_ARGUMENT_THREAD_PRIORITY);
    }

    *thread = __pk_thread_at_priority(priority);

    return PK_OK;
}


/// Swap thread priorities
///
/// \param thread_a A pointer to an initialized PkThread
///
/// \param thread_b A pointer to an initialized PkThread
///
/// This API swaps the priorities of \a thread_a and \a thread_b.  The API is
/// provided to support general and efficient application-directed scheduling
/// algorithms.  The requirements on the \a thread_a and \a thread_b arguments
/// are that they are valid pointers to initialized PkThread structures, that
/// the current thread priorities of both threads are legal, and that if a
/// thread is currently mapped, that the new thread priority is not otherwise
/// in use.
///
/// The API does not require either thread to be mapped, or even to be active.
/// It is legal for one or both of the swap partners to be suspended, deleted
/// or completed threads.  The application is completely responsible for the
/// correctness of scheduling algorithms that might operate on inactive or
/// suspended threads.
///
/// The API does not change the mapped status of a thread.  A thread will be
/// mapped after the call of pk_thread_priority_swap() if and only if it was
/// mapped prior to the call.  If the new priority of a mapped thread is
/// currently in use (by a thread other than the swap partner), then the
/// PK_PRIORITY_IN_USE_AT_SWAP error is signalled and the swap does not take 
/// place. This could only happen if the swap partner is not currently mapped.
///
/// It is legal for a thread to swap its own priority with another thread. The
/// degenerate case that \a thread_a and \a thread_b are equal is also legal -
/// but has no effect.
///
/// Return values other than PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion, including the redundant cases that do not
/// actually change priorities, or the cases that assign new priorities to
/// suspended, completed or deleted threads.
///
/// \retval -PK_INVALID_THREAD_AT_SWAP1 One or both of \a thread_a and 
/// \a thread_b is null (0) or otherwise invalid, 
///
/// \retval -PK_INVALID_THREAD_AT_SWAP2 the priorities of One or both of 
/// \a thread_a and \a thread_b are invalid.
///
/// \retval -PK_INVALID_ARGUMENT One or both of the priorities
/// of \a thread_a and \a thread_b is invalid.
///
/// \retval -PK_PRIORITY_IN_USE_AT_SWAP Returned if a thread is mapped and the
/// new thread priority is currently in use by another thread (other than the
/// swap partner).

int
pk_thread_priority_swap(PkThread* thread_a, PkThread* thread_b)
{
    PkMachineContext ctx;
    PkThreadPriority priority_a, priority_b;
    int mapped_a, mapped_b;

    if (PK_ERROR_CHECK_API) {
        PK_ERROR_IF((thread_a == 0) ||  (thread_b == 0), 
                       PK_INVALID_THREAD_AT_SWAP1);
    }

    pk_critical_section_enter(&ctx);

    if (thread_a != thread_b) {

        mapped_a = __pk_thread_is_mapped(thread_a);
        mapped_b = __pk_thread_is_mapped(thread_b);
        priority_a = thread_a->priority;
        priority_b = thread_b->priority;

        if (PK_ERROR_CHECK_API) {
            int priority_in_use;
            PK_ERROR_IF_CRITICAL((priority_a > PK_THREADS) ||
                                  (priority_b > PK_THREADS),
                                  PK_INVALID_THREAD_AT_SWAP2,
                                  &ctx);
            priority_in_use = 
                (mapped_a && !mapped_b &&
                 (__pk_thread_at_priority(priority_b) != 0)) ||
                (!mapped_a && mapped_b && 
                 (__pk_thread_at_priority(priority_a) != 0));
            PK_ERROR_IF_CRITICAL(priority_in_use, 
                                  PK_PRIORITY_IN_USE_AT_SWAP, &ctx); 
        }

        if (mapped_a) {
            __pk_thread_unmap(thread_a);
        }
        if (mapped_b) {
            __pk_thread_unmap(thread_b);
        }            
        thread_a->priority = priority_b;
        thread_b->priority = priority_a;
        if (mapped_a) {
            __pk_thread_map(thread_a);
        }
        if (mapped_b) {
            __pk_thread_map(thread_b);
        }
        __pk_schedule();
    }

    pk_critical_section_exit(&ctx);

    return PK_OK;
}

