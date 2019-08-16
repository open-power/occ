/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/kernel/pk_semaphore_core.c $                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2019                        */
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

/// \file pk_semaphore_core.c
/// \brief PK semaphore APIs
///
///  The entry points in this file are considered 'core' routines that will
///  always be present at runtime in any PK application that enables
///  semaphores.

#include "pk.h"

/// Post a count to a semaphore
///
/// \param semaphore A pointer to the semaphore
///
/// If any thread is pending on the semaphore, the highest priority thread
/// will be made runnable and the internal count will remain 0.
///
/// If no thread is pending on the semaphore then the internal count will be
/// incremented by 1, with overflow wrapping the internal count through 0. If
/// the \a max_count argument supplied when the semaphore was created is
/// non-zero and the new internal count is greater than the \a max_count, an
/// overflow error will be signalled.
///
/// Return values other than PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion
///
/// \retval -PK_INVALID_SEMAPHORE_AT_POST The \a semaphore is a null (0) pointer.
///
/// \retval -PK_SEMAPHORE_OVERFLOW The \a max_count argument supplied when
/// the semaphore was created is non-zero and the new internal count is
/// greater than the \a max_count.

int
pk_semaphore_post(PkSemaphore* semaphore)
{
    PkMachineContext ctx;
    PkThreadPriority priority;

    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF(semaphore == 0, PK_INVALID_SEMAPHORE_AT_POST);
    }

    pk_critical_section_enter(&ctx);

    priority = __pk_thread_queue_min(&(semaphore->pending_threads));

    if (priority != PK_IDLE_THREAD_PRIORITY)
    {

        __pk_thread_queue_delete(&(semaphore->pending_threads), priority);
        __pk_thread_queue_insert(&__pk_run_queue, priority);

        PK_KERN_TRACE("THREAD_SEMAPHORE_POST(%d)", priority);

        __pk_schedule();

    }
    else
    {

        semaphore->count++;

        if (PK_ERROR_CHECK_API)
        {
            PK_ERROR_IF((semaphore->max_count > 0) &&
                        (semaphore->count > semaphore->max_count),
                        PK_SEMAPHORE_OVERFLOW);
        }
    }

    pk_critical_section_exit(&ctx);

    return PK_OK;
}


/// Pend on a semaphore with timeout
///
/// \param semaphore A pointer to the semaphore
///
/// \param timeout A relative timeout in PK timebase ticks, including the
/// special values PK_NO_WAIT and PK_WAIT_FOREVER
///
/// This API is normally called from threads, and can only be successfully
/// called from interupt handlers under special conditions.
///
/// If the internal count of the \a semaphore is non-zero, the internal count
/// is decremented by one and execution of the caller continues.
///
/// If the internal count of the \a semaphore is zero and the \a timeout is
/// PK_NO_WAIT (0) then the call returns immediately with the informational
/// code -PK_SEMAPHORE_PEND_NO_WAIT.
///
/// If the internal count of the \a semaphore is zero and the \a timeout is
/// non-zero then a thread will block until either a semaphore count is
/// acquired or the relative timeout expires.  If this condition occurs in a
/// call from an interrupt context or before threads have been started then
/// the call will fail with the error \c -PK_SEMAPHORE_PEND_WOULD_BLOCK.
///
/// Once timed out the thread is removed from the semaphore pending queue and
/// made runnable, and the pk_semaphore_pend() operation will fail, even if
/// the semaphore count becomes available before the thread runs again. The
/// pk_semaphore_pend() API returns the informational code
/// -PK_SEMAPHORE_PEND_TIMED_OUT in this case.
///
/// By convention, a timeout interval equal to the maximum possible value of
/// the \c PkInterval type is taken to mean "wait forever".  A thread blocked
/// on a semaphore in this mode will never time out. PK provides this
/// constant as \c PK_WAIT_FOREVER.
///
/// Return values other than PK_OK (0) are not necessarily errors; see \ref
/// pk_errors
///
/// The following return codes are non-error codes:
///
/// \retval 0 Successful completion
///
/// \retval -PK_SEMAPHORE_PEND_NO_WAIT timeout is set to PK_NO_WAIT
///
/// \retval -PK_SEMAPHORE_PEND_TIMED_OUT The semaphore was not acquired
/// before the timeout expired.
///
/// The following return codes are error codes:
///
/// \retval -PK_INVALID_SEMAPHORE_AT_PEND The \a semaphore is a null (0)
/// pointer.
///
/// \retval -PK_SEMAPHORE_PEND_WOULD_BLOCK The call was made from an
/// interrupt context (or before threads have been started), the semaphore
/// internal count was 0 and a non-zero timeout was specified.

// Note: Casting __pk_current_thread removes the 'volatile' attribute.

int
pk_semaphore_pend(PkSemaphore* semaphore,
                  PkInterval  timeout)
{
    PkMachineContext ctx;
    PkThreadPriority priority;
    PkThread*         thread;
    PkTimer*          timer = 0;
    PkInterval       scaled_timeout = PK_INTERVAL_SCALE(timeout);

    int rc = PK_OK;

    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF(semaphore == 0, PK_INVALID_SEMAPHORE_AT_PEND);
    }

    pk_critical_section_enter(&ctx);

    if (semaphore->count != 0)
    {

        semaphore->count--;

    }
    else if (timeout == PK_NO_WAIT)
    {

        rc = -PK_SEMAPHORE_PEND_NO_WAIT;

    }
    else
    {

        if (PK_ERROR_CHECK_API)
        {
            PK_ERROR_IF_CRITICAL(!__pk_kernel_context_thread(),
                                 PK_SEMAPHORE_PEND_WOULD_BLOCK,
                                 &ctx);
        }

        thread = (PkThread*)__pk_current_thread;
        priority = thread->priority;

        __pk_thread_queue_insert(&(semaphore->pending_threads), priority);

        thread->semaphore = semaphore;
        thread->flags |= PK_THREAD_FLAG_SEMAPHORE_PEND;

        PK_KERN_TRACE("THREAD_SEMAPHORE_PEND(%d)", priority);

        if (timeout != PK_WAIT_FOREVER)
        {
            timer = &(thread->timer);
            timer->timeout = pk_timebase_get() + scaled_timeout;
            __pk_timer_schedule(timer);
            thread->flags |= PK_THREAD_FLAG_TIMER_PEND;
        }

        __pk_thread_queue_delete(&__pk_run_queue, priority);
        __pk_schedule();

        thread->flags &= ~PK_THREAD_FLAG_SEMAPHORE_PEND;

        if (thread->flags & PK_THREAD_FLAG_TIMER_PEND)
        {
            if (thread->flags & PK_THREAD_FLAG_TIMED_OUT)
            {
                rc = -PK_SEMAPHORE_PEND_TIMED_OUT;
                __pk_thread_queue_delete(&(semaphore->pending_threads), thread->priority);
            }
            else
            {
                __pk_timer_cancel(timer);
            }

            thread->flags &=
                ~(PK_THREAD_FLAG_TIMER_PEND | PK_THREAD_FLAG_TIMED_OUT);
        }
    }

    pk_critical_section_exit(&ctx);

    return rc;
}


/// Release all threads blocked on a semaphore
///
/// \param semaphore A pointer to a semaphore
///
/// This API is provided to allow an PK semaphore to be used as a thread
/// barrier. pk_semaphore_release_all() simultaneously unblocks all threads
/// (if any) currently pending on a semaphore. A semaphore to be used as a
/// thread barrier will typically be initialized with
/// pk_semaphore_create(\a sem, 0, 0), and sxx_semaphore_post() would never be
/// called on the \a sem.
///
/// This API never modifies the \a count field of the semaphore; If any
/// threads are blocked on a semaphore the semaphore count is 0 by definition.
///
/// Return values other than PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion
///
/// \retval -PK_INVALID_SEMAPHORE_AT_RELEASE The \a semaphore is a null (0)
/// pointer.

int
pk_semaphore_release_all(PkSemaphore* semaphore)
{
    PkMachineContext ctx;

    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF(semaphore == 0, PK_INVALID_SEMAPHORE_AT_RELEASE);
    }

    pk_critical_section_enter(&ctx);

    __pk_thread_queue_union(&__pk_run_queue, &(semaphore->pending_threads));
    __pk_thread_queue_clear(&(semaphore->pending_threads));
    __pk_schedule();

    pk_critical_section_exit(&ctx);

    return PK_OK;
}


/// Get information about a semaphore.
///
/// \param semaphore A pointer to the PkSemaphore to query
///
/// \param count The value returned through this pointer is the current count
/// of the semaphore.  The caller can set this parameter to the null pointer
/// (0) if this information is not required.
///
/// \param pending The value returned through this pointer is the current
/// number of threads pending on the semaphore. The caller can set this
/// parameter to the null pointer (0) if this information is not required.
///
/// The information returned by this API can only be guaranteed consistent if
/// the API is called from a critical section.
///
/// Return values other than PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion
///
/// \retval -PK_INVALID_SEMAPHORE_AT_INFO The \a semaphore is a null (0)
/// pointer.

int
pk_semaphore_info_get(PkSemaphore*      semaphore,
                      PkSemaphoreCount* count,
                      int*               pending)

{
    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF(semaphore == 0, PK_INVALID_SEMAPHORE_AT_INFO);
    }

    if (count)
    {
        *count = semaphore->count;
    }

    if (pending)
    {
        *pending = __pk_thread_queue_count(&(semaphore->pending_threads));
    }

    return PK_OK;
}


/// An simple interrupt handler that posts to a semaphore.
///
/// To implement basic event-driven blocking of a thread, install
/// pk_semaphore_post_handler() as the handler for an interrupt
/// and provide a pointer to the semaphore as the \a arg argument in
/// pk_irq_handler_set().  The semaphore should be initialized with
/// pk_semaphore_create(&sem, 0, 1).  This handler simply disables (masks)
/// the interrupt, clears the status and calls pk_semaphore_post() on the
/// semaphore.
///
/// Note that clearing the status in the interrupt controller as done here is
/// effectively a no-op for level-sensitive interrupts. In the level-sensitive
/// case any thread pending on the semaphore must reset the interrupt
/// condition in the device before re-enabling the interrupt.
#if 0
void
pk_semaphore_post_handler(void* arg, PkIrqId irq, int priority)
{
    pk_irq_disable(irq);
    pk_irq_status_clear(irq);
    pk_semaphore_post((PkSemaphore*)arg);
}

#endif
