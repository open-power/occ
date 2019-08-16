/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/kernel/pk_thread_core.c $                          */
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

/// \file pk_thread_core.c
/// \brief PK thread APIs
///
///  The entry points in this file are considered 'core' routines that will
///  always be present at runtime in any PK application that enables threads.

#include "pk.h"
#include "pk_thread.h"

#define __PK_THREAD_CORE_C__


// This routine is only used locally.  Interrupts must be disabled
// at entry.  The caller must also have checked that the priority is free.
// This routine is only called on threads known to be in a suspended state,
// either PK_THREAD_STATE_SUSPENDED_RUNNABLE or
// PK_THREAD_STATE_SUSPENDED_BLOCKED.  Mapping a runnable thread adds it to
// the run queue.  Mapping a thread pending on a semaphore either takes the
// count and becomes runnable or adds the thread to the pending queue for the
// semaphore.  Mapping a sleeping thread requires no further action
// here. Scheduling after the map must be handled by the caller.

void
__pk_thread_map(PkThread* thread)
{
    PkThreadPriority priority;

    priority = thread->priority;
    __pk_priority_map[priority] = thread;

    if (thread->state == PK_THREAD_STATE_SUSPENDED_RUNNABLE)
    {

        __pk_thread_queue_insert(&__pk_run_queue, priority);

    }
    else if (thread->flags & PK_THREAD_FLAG_SEMAPHORE_PEND)
    {

        if (thread->semaphore->count)
        {

            thread->semaphore->count--;
            __pk_thread_queue_insert(&__pk_run_queue, priority);

        }
        else
        {

            __pk_thread_queue_insert(&(thread->semaphore->pending_threads),
                                     priority);
        }
    }

    thread->state = PK_THREAD_STATE_MAPPED;

    if (PK_KERNEL_TRACE_ENABLE)
    {
        if (__pk_thread_is_runnable(thread))
        {
            PK_KERN_TRACE("THREAD_MAPPED_RUNNABLE(%d)", priority);
        }
        else if (thread->flags & PK_THREAD_FLAG_SEMAPHORE_PEND)
        {
            PK_KERN_TRACE("THREAD_MAPPED_SEMAPHORE_PEND(%d)", priority);
        }
        else
        {
            PK_KERN_TRACE("THREAD_MAPPED_SLEEPING(%d)", priority);
        }
    }
}


// This routine is only used locally.  Interrupts must be disabled
// at entry.  This routine is only ever called on threads in the
// PK_THREAD_STATE_MAPPED. Unmapping a thread removes it from the priority
// map, the run queue and any semaphore pend, but does not cancel any
// timers. Scheduling must be handled by the code calling
// __pk_thread_unmap().

void
__pk_thread_unmap(PkThread* thread)
{
    PkThreadPriority priority;

    priority = thread->priority;
    __pk_priority_map[priority] = 0;

    if (__pk_thread_is_runnable(thread))
    {

        thread->state = PK_THREAD_STATE_SUSPENDED_RUNNABLE;
        __pk_thread_queue_delete(&__pk_run_queue, priority);

    }
    else
    {

        thread->state = PK_THREAD_STATE_SUSPENDED_BLOCKED;

        if (thread->flags & PK_THREAD_FLAG_SEMAPHORE_PEND)
        {
            __pk_thread_queue_delete(&(thread->semaphore->pending_threads),
                                     priority);
        }
    }
}


// Schedule and run the highest-priority mapped runnable thread.
//
// The priority of the next thread to run is first computed.  This may be
// PK_THREADS, indicating that the only thread to run is the idle thread.
// This will always cause (or defer) a 'context switch' to the idle thread.
// Otherwise, if the new thread is not equal to the current thread this will
// also cause (or defer) a context switch. Note that scheduling is defined in
// terms of priorities but actually implemented in terms of PkThread pointers.
//
// If we are not yet in thread mode we're done - threads will be started by
// pk_start_threads() later. If we're in thread context a context switch
// happens immediately. In an interrupt context the switch is deferred to the
// end of interrupt processing.

void
__pk_schedule(void)
{
    __pk_next_priority = __pk_thread_queue_min(&__pk_run_queue);
    __pk_next_thread = __pk_priority_map[__pk_next_priority];

    if ((__pk_next_thread == 0) ||
        (__pk_next_thread != __pk_current_thread))
    {

        if (__pk_kernel_mode_thread())
        {
            if (__pk_kernel_context_thread())
            {
                if (__pk_current_thread != 0)
                {
                    __pk_switch();
                }
                else
                {
                    __pk_next_thread_resume();
                }
            }
            else
            {
                __pk_delayed_switch = 1;
            }
        }
    }
}


// This routine is only used locally.
//
// Completion and deletion are pretty much the same thing.  Completion is
// simply self-deletion of the current thread (which is mapped by
// definition.) The complete/delete APIs have slightly different error
// conditions but are otherwise the same.
//
// Deleting a mapped thread first unmaps (suspends) the thread, which takes
// care of removing the thread from any semaphores it may be pending on.  Then
// any outstanding timer is also cancelled.
//
// If the current thread is being deleted we install the idle thread as
// __pk_current_thread, so scheduling is forced and no context is saved on
// the context switch.
//
// Note that we do not create trace events for unmapped threads since the trace
// tag only encodes the priority, which may be in use by a mapped thread.

void
__pk_thread_delete(PkThread* thread, PkThreadState final_state)
{
    PkMachineContext ctx;
    int mapped;

    pk_critical_section_enter(&ctx);

    mapped = __pk_thread_is_mapped(thread);

    if (mapped)
    {
        __pk_thread_unmap(thread);
    }

    __pk_timer_cancel(&(thread->timer));
    thread->state = final_state;

    if (mapped)
    {

        if (PK_KERNEL_TRACE_ENABLE)
        {
            if (final_state == PK_THREAD_STATE_DELETED)
            {
                PK_KERN_TRACE("THREAD_DELETED(%d)", thread->priority);
            }
            else
            {
                PK_KERN_TRACE("THREAD_COMPLETED(%d)", thread->priority);
            }
        }

        if (thread == __pk_current_thread)
        {
            __pk_current_thread = 0;
        }

        __pk_schedule();
    }

    pk_critical_section_exit(&ctx);
}


// Generic thread timeout
//
// This routine is called as a timer callback either because a sleeping thread
// has timed out or a thread pending on a semaphore has timed out.  If the
// thread is not already runnable then the the timeout flag is set, and if the
// thread is mapped it is scheduled.
//
// This implementation allows that a thread blocked on a timer may have been
// made runnable by some other mechanism, such as acquiring a semaphore.  In
// order to provide an iteration-free implementation of
// pk_semaphore_release_all(), cancelling any semaphore timeouts is deferred
// until the thread runs again.
//
// Note that we do not create trace events for unmapped threads since the trace
// tag only encodes the priority, which may be in use by a mapped thread.

void
__pk_thread_timeout(void* arg)
{
    PkMachineContext ctx;
    PkThread* thread = (PkThread*)arg;

    pk_critical_section_enter(&ctx);

    switch (thread->state)
    {

        case PK_THREAD_STATE_MAPPED:
            if (!__pk_thread_is_runnable(thread))
            {
                thread->flags |= PK_THREAD_FLAG_TIMED_OUT;
                __pk_thread_queue_insert(&__pk_run_queue, thread->priority);
                __pk_schedule();
            }

            break;

        case PK_THREAD_STATE_SUSPENDED_RUNNABLE:
            break;

        case PK_THREAD_STATE_SUSPENDED_BLOCKED:
            thread->flags |= PK_THREAD_FLAG_TIMED_OUT;
            thread->state = PK_THREAD_STATE_SUSPENDED_RUNNABLE;
            break;

        default:
            PK_PANIC(PK_THREAD_TIMEOUT_STATE);
    }

    pk_critical_section_exit(&ctx);
}


// This routine serves as a container for the PK_START_THREADS_HOOK and
// actually starts threads.  The helper routine __pk_call_pk_start_threads()
// arranges this routine to be called with interrupts disabled while running
// on the kernel stack.
//
// The reason for this roundabout is that we want to be able to run a hook
// routine (transparent to the application) that can hand over every last byte
// of free memory to "malloc()" - including the stack of main().  Since we
// always need to run on some stack, we chose to run the hook on the kernel
// stack. However to do this safely we need to make sure
// that no interrupts will happen during this time. When __pk_thread_resume()
// is finally called all stack-based context is lost but it doesn't matter at
// that point - it's a one-way street into thread execution.
//
// This is considered part of pk_start_threads() and so is also considered a
// 'core' routine.

void
__pk_start_threads(void)
{
    PK_START_THREADS_HOOK;

    __pk_next_thread_resume();

    PK_PANIC(PK_START_THREADS_RETURNED);
}


/// Start PK threads
///
/// This routine starts the PK thread scheduler infrastructure. This routine
/// must be called after a call of \c pk_initialize().  This routine never
/// returns. Interrupt (+ timer) only configurations of PK need not call this
/// routine.
///
/// Note: This tiny routine is considered a 'core' routine so that the
/// initialziation code can safely recover all 'init' code space before
/// starting threads.
///
/// This routine typically does not return - any return value indicates an
/// error; see \ref pk_errors
///
/// \retval -PK_ILLEGAL_CONTEXT_THREAD The API was called twice.

int
pk_start_threads(void)
{
    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF(__pk_kernel_mode_thread(), PK_ILLEGAL_CONTEXT_THREAD);
    }

    __pk_call_pk_start_threads();

    return 0;
}


/// Resume a suspended thread
///
/// \param thread The thread to resume
///
/// PK only allows one thread at a time to run at a given priority, and
/// implements the notion of a thread \e claiming a priority.  A suspended
/// thread claims a priority when it is mapped by a call of
/// pk_thread_resume().  This API will succeed only if no other active thread
/// is currently mapped at the priority assigned to the thread.  PK provides
/// the pk_thread_at_priority() API which allows an application-level
/// scheduler to correctly manage multiple threads running at the same
/// priority.
///
/// If the thread was sleeping while suspended it remains asleep. However if
/// the sleep timer timed out while the thread was suspended it will be
/// resumed runnable.
///
/// If the thread was blocked on a semaphore when it was suspended, then when
/// the thread is resumed it will attempt to reacquire the semaphore.
/// However, if the thread was blocked on a semaphore with timeout while
/// suspended and the timeout interval has passed, the thread will be resumed
/// runnable and see that the semaphore pend timed out.
///
/// It is not an error to call pk_thread_resume() on a mapped
/// thread.  However it is an error to call pk_thread_resume() on a completed
/// or deleted thread.
///
/// Return values other than PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion, including calls on a \a thread that is
/// already mapped.
///
/// \retval -PK_INVALID_THREAD_AT_RESUME1 The \a thread is a null (0) pointer.
///
/// \retval -PK_INVALID_THREAD_AT_RESUME2 The \a thread is not active,
/// i.e. has completed or been deleted.
///
/// \retval -PK_PRIORITY_IN_USE_AT_RESUME Another thread is already mapped at
/// the priority of the \a thread.

int
pk_thread_resume(PkThread* thread)
{
    PkMachineContext ctx;

    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF(thread == 0, PK_INVALID_THREAD_AT_RESUME1);
    }

    pk_critical_section_enter(&ctx);

    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF_CRITICAL(!__pk_thread_is_active(thread),
                             PK_INVALID_THREAD_AT_RESUME2,
                             &ctx);
    }

    if (!__pk_thread_is_mapped(thread))
    {

        if (PK_ERROR_CHECK_API)
        {
            PK_ERROR_IF_CRITICAL(__pk_priority_map[thread->priority] != 0,
                                 PK_PRIORITY_IN_USE_AT_RESUME,
                                 &ctx);
        }

        __pk_thread_map(thread);
        __pk_schedule();
    }

    pk_critical_section_exit(&ctx);

    return PK_OK;
}


/// Suspend a thread
///
/// Any active thread can be suspended.  A suspended thread 1) remains active
/// but will not be scheduled; 2) relinquishes its priority assignment,
/// allowing another thread to be resumed at the suspended thread's priority;
/// and 3) disassociates from any semaphore mutual exclusion it may have been
/// participating in.
///
/// If a sleeping thread is suspended, the sleep timer remains active but a
/// timeout of the timer simply marks the thread as runnable, but does not
/// resume the thread.
///
/// If a thread blocked on a semaphore is suspended, the thread no longer
/// participates in the semaphore mutual exclusion. If the thread is later
/// resumed it will attempt to acquire the semaphore again the next time it
/// runs (unless it was blocked with a timeout and the timeout has expired).
///
/// If a thread blocked on a semaphore with timeout is suspended, the
/// semaphore timeout timer continues to run. If the timer times out while the
/// thread is suspended the thread is simply marked runnable. If the thread is
/// later resumed, the suspended call of \c pk_semaphore_pend() will return the
/// timeout code -PK_SEMAPHORE_PEND_TIMED_OUT.
///
/// Return values other than PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion, including calls on a \a thread that is
/// already suspended.
///
/// \retval -PK_INVALID_THREAD_AT_SUSPEND1 The \a thread is a null (0) pointer
///
/// \retval -PK_INVALID_THREAD_AT_SUSPEND2 The \a thread is not active,
/// i.e. has completed or been deleted.

int
pk_thread_suspend(PkThread* thread)
{
    PkMachineContext ctx;

    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF((thread == 0), PK_INVALID_THREAD_AT_SUSPEND1);
    }

    pk_critical_section_enter(&ctx);

    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF_CRITICAL(!__pk_thread_is_active(thread),
                             PK_INVALID_THREAD_AT_SUSPEND2,
                             &ctx);
    }

    if (__pk_thread_is_mapped(thread))
    {

        PK_KERN_TRACE("THREAD_SUSPENDED(%d)", thread->priority);
        __pk_thread_unmap(thread);
        __pk_schedule();
    }

    pk_critical_section_exit(&ctx);

    return PK_OK;
}


/// Delete a thread
///
/// Any active thread can be deleted.  If a thread is deleted it is removed
/// from the run queue, deleted from the timer queue (if sleeping or blocked
/// on a semaphore with timeout), and deleted from the semaphore mutual
/// exclusion if blocked on a semaphore. The thread control block is then
/// marked as deleted.
///
/// Once a thread has completed or been deleted the thread structure and
/// thread stack areas can be used for other purposes.
///
/// \param thread The thread to delete
///
/// Return values other than PK_OK (0) are errors; see \ref pk_errors.  If a
/// thread deletes itself this API does not return at all.
///
/// \retval 0 Successful completion, including calls on a \a thread that has
/// completed or had already been deleted.
///
///
/// \retval -PK_INVALID_THREAD_AT_DELETE The \a thread is a null (0) pointer.

int
pk_thread_delete(PkThread* thread)
{
    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF(thread == 0, PK_INVALID_THREAD_AT_DELETE);
    }

    __pk_thread_delete(thread, PK_THREAD_STATE_DELETED);

    return PK_OK;
}


/// Complete a thread
///
/// If a thread ever returns from the subroutine defining the thread entry
/// point, the thread is removed from all PK kernel data structures and
/// marked completed. The thread routine can also use the API pk_complete()
/// to make this more explicit if desired. PK makes no distinction between
/// completed and deleted threads, but provides these indications for
/// the benefit of the application.
///
/// Note that this API is only available from the current thread to mark its
/// own completion.
///
/// Once a thread has completed or been deleted the thread structure and
/// thread stack areas can be used for other purposes.
///
/// Any return value indicates an error; see \ref pk_errors.  In the event of
/// a successful completion this API does not return to the caller, which is
/// always the thread context being completed.
///
/// \retval -PK_ILLEGAL_CONTEXT_THREAD The API was not called from a thread
/// context.

// Note: Casting __pk_current_thread removes the 'volatile' attribute.

int
pk_complete(void)
{
    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_UNLESS_THREAD_CONTEXT();
    }

    __pk_thread_delete((PkThread*)__pk_current_thread,
                       PK_THREAD_STATE_COMPLETED);

    return PK_OK;
}

/// Sleep a thread for an interval relative to the current time.
///
/// \param interval A time interval relative to the current timebase.
///
/// Threads can use this API to sleep for a time relative to the current
/// timebase.  The absolute timeout is \c pk_timebase_get() + \a interval.
///
///  Sleeping threads are not scheduled, although they maintain their
/// priorities.  This differs from thread suspension, where the suspended
/// thread relinquishes its priority.  When the sleep timer times out the
/// thread becomes runnable again, and will run as soon as it becomes the
/// highest-priority mapped runnable thread.
///
/// Sleeping threads may also be later suspended. In this case the Sleep timer
/// continues to run, and if it times out before the thread is resumed the
/// thread will be immediately runnable when it is resumed.
///
/// See the PK specification for a full discussion of how PK handles
/// scheduling events at absolute times "in the past". Briefly stated, if the
/// \a interval is 0 or is so small that the absolute time becomes a "past"
/// time before the Sleep is actually scheduled, the thread will Sleep for the
/// briefest possible period supported by the hardware.
///
/// Return values other than PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion.
///
/// \retval -PK_ILLEGAL_CONTEXT_THREAD The API was not called from a thread
/// context.

int
pk_sleep(PkInterval interval)
{
    PkTimebase  time;
    PkMachineContext ctx;
    PkThread* current;

    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_UNLESS_THREAD_CONTEXT();
    }

    time = pk_timebase_get() + PK_INTERVAL_SCALE(interval);

    pk_critical_section_enter(&ctx);

    current = (PkThread*)__pk_current_thread;

    current->timer.timeout = time;
    __pk_timer_schedule(&(current->timer));

    current->flags |= PK_THREAD_FLAG_TIMER_PEND;

    PK_KERN_TRACE("THREAD_SLEEP(%d)", current->priority);

    __pk_thread_queue_delete(&__pk_run_queue, current->priority);
    __pk_schedule();

    current->flags &= ~(PK_THREAD_FLAG_TIMER_PEND | PK_THREAD_FLAG_TIMED_OUT);

    pk_critical_section_exit(&ctx);

    return PK_OK;
}

#undef __PK_THREAD_CORE_C__
