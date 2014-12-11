// $Id: ssx_thread_core.c,v 1.2 2014/02/03 01:30:44 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ssx/ssx_thread_core.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ssx_thread_core.c
/// \brief SSX thread APIs
///
///  The entry points in this file are considered 'core' routines that will
///  always be present at runtime in any SSX application that enables threads.

#include "ssx.h"

#define __SSX_THREAD_CORE_C__


// This routine is only used locally.  Noncritical interrupts must be disabled
// at entry.

static inline int
__ssx_thread_is_active(SsxThread *thread)
{
    return ((thread->state != SSX_THREAD_STATE_COMPLETED) &&
            (thread->state != SSX_THREAD_STATE_DELETED));
}


// This routine is only used locally.  Noncritical interrupts must be disabled
// at entry.  

static inline int
__ssx_thread_is_mapped(SsxThread *thread)
{
    return (thread->state == SSX_THREAD_STATE_MAPPED);
}


// This routine is only used locally.  Noncritical interrupts must be disabled
// at entry.  This is only called on mapped threads.

static inline int
__ssx_thread_is_runnable(SsxThread *thread)
{
    return __ssx_thread_queue_member(&__ssx_run_queue, thread->priority);
}


// This routine is only used locally.  Noncritical interrupts must be disabled
// at entry.

static inline SsxThread*
__ssx_thread_at_priority(SsxThreadPriority priority)
{
    return (SsxThread*)__ssx_priority_map[priority];
}


// This routine is only used locally.  Noncritical interrupts must be disabled
// at entry.  The caller must also have checked that the priority is free.
// This routine is only called on threads known to be in a suspended state,
// either SSX_THREAD_STATE_SUSPENDED_RUNNABLE or
// SSX_THREAD_STATE_SUSPENDED_BLOCKED.  Mapping a runnable thread adds it to
// the run queue.  Mapping a thread pending on a semaphore either takes the
// count and becomes runnable or adds the thread to the pending queue for the
// semaphore.  Mapping a sleeping thread requires no further action
// here. Scheduling after the map must be handled by the caller.

void
__ssx_thread_map(SsxThread* thread)
{
    SsxThreadPriority priority;

    priority = thread->priority;
    __ssx_priority_map[priority] = thread;

    if (thread->state == SSX_THREAD_STATE_SUSPENDED_RUNNABLE) {

        __ssx_thread_queue_insert(&__ssx_run_queue, priority);

    } else if (thread->flags & SSX_THREAD_FLAG_SEMAPHORE_PEND) {

        if (thread->semaphore->count) {

            thread->semaphore->count--;
            __ssx_thread_queue_insert(&__ssx_run_queue, priority);

        } else {

            __ssx_thread_queue_insert(&(thread->semaphore->pending_threads),
                                      priority);
        }
    }

    thread->state = SSX_THREAD_STATE_MAPPED;

    if (SSX_KERNEL_TRACE_ENABLE) {
        if (__ssx_thread_is_runnable(thread)) {
            SSX_TRACE_THREAD_MAPPED_RUNNABLE(priority);
        } else if (thread->flags & SSX_THREAD_FLAG_SEMAPHORE_PEND) {
            SSX_TRACE_THREAD_MAPPED_SEMAPHORE_PEND(priority);
        } else {
            SSX_TRACE_THREAD_MAPPED_SLEEPING(priority);
        }
    }
}            

    
// This routine is only used locally.  Noncritical interrupts must be disabled
// at entry.  This routine is only ever called on threads in the
// SSX_THREAD_STATE_MAPPED. Unmapping a thread removes it from the priority
// map, the run queue and any semaphore pend, but does not cancel any
// timers. Scheduling must be handled by the code calling
// __ssx_thread_unmap().

void
__ssx_thread_unmap(SsxThread *thread)
{
    SsxThreadPriority priority;

    priority = thread->priority;
    __ssx_priority_map[priority] = 0;

    if (__ssx_thread_is_runnable(thread)) {

        thread->state = SSX_THREAD_STATE_SUSPENDED_RUNNABLE;
        __ssx_thread_queue_delete(&__ssx_run_queue, priority);

    } else {

        thread->state = SSX_THREAD_STATE_SUSPENDED_BLOCKED;
        if (thread->flags & SSX_THREAD_FLAG_SEMAPHORE_PEND) {
            __ssx_thread_queue_delete(&(thread->semaphore->pending_threads), 
                                      priority);
        }
    }
}


// Schedule and run the highest-priority mapped runnable thread.
//
// The priority of the next thread to run is first computed.  This may be
// SSX_THREADS, indicating that the only thread to run is the idle thread.
// This will always cause (or defer) a 'context switch' to the idle thread.
// Otherwise, if the new thread is not equal to the current thread this will
// also cause (or defer) a context switch. Note that scheduling is defined in
// terms of priorities but actually implemented in terms of SsxThread pointers.
//
// If we are not yet in thread mode we're done - threads will be started by
// ssx_start_threads() later. If we're in thread context a context switch
// happens immediately. In an interrupt context the switch is deferred to the
// end of SSX_NONCRITICAL interrupt processing.

void
__ssx_schedule(void)
{
    __ssx_next_priority = __ssx_thread_queue_min(&__ssx_run_queue);
    __ssx_next_thread = __ssx_priority_map[__ssx_next_priority];

    if ((__ssx_next_thread == 0) ||
        (__ssx_next_thread != __ssx_current_thread)) {

        if (__ssx_kernel_mode_thread()) {
            if (__ssx_kernel_context_thread()) {
                if (__ssx_current_thread != 0) {
                    __ssx_switch();
                } else {
                    __ssx_next_thread_resume();
                }
            } else {
                __ssx_delayed_switch = 1;
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
// __ssx_current_thread, so scheduling is forced and no context is saved on
// the context switch.
//
// Note that we do not create trace events for unmapped threads since the trace
// tag only encodes the priority, which may be in use by a mapped thread.

void
__ssx_thread_delete(SsxThread *thread, SsxThreadState final_state)
{
    SsxMachineContext ctx;
    int mapped;

    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    mapped = __ssx_thread_is_mapped(thread);

    if (mapped) {
        __ssx_thread_unmap(thread);
    }

    __ssx_timer_cancel(&(thread->timer));
    thread->state = final_state;

    if (mapped) {

        if (SSX_KERNEL_TRACE_ENABLE) {
            if (final_state == SSX_THREAD_STATE_DELETED) {
                SSX_TRACE_THREAD_DELETED(thread->priority);
            } else {
                SSX_TRACE_THREAD_COMPLETED(thread->priority);
            }
        }                
    
        if (thread == __ssx_current_thread) {
            __ssx_current_thread = 0;
        }
        __ssx_schedule();
    }

    ssx_critical_section_exit(&ctx);
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
// ssx_semaphore_release_all(), cancelling any semaphore timeouts is deferred
// until the thread runs again.
//
// __ssx_thread_timeout() is currenly the only timer interrupt called from a
// critical section.
//
// Note that we do not create trace events for unmapped threads since the trace
// tag only encodes the priority, which may be in use by a mapped thread.

void
__ssx_thread_timeout(void *arg)
{
    SsxThread *thread = (SsxThread *)arg;

    switch (thread->state) {

    case SSX_THREAD_STATE_MAPPED:
        if (!__ssx_thread_is_runnable(thread)) {
            thread->flags |= SSX_THREAD_FLAG_TIMED_OUT;
            __ssx_thread_queue_insert(&__ssx_run_queue, thread->priority);
            __ssx_schedule();
        }
        break;

    case SSX_THREAD_STATE_SUSPENDED_RUNNABLE:
        break;

    case SSX_THREAD_STATE_SUSPENDED_BLOCKED:
        thread->flags |= SSX_THREAD_FLAG_TIMED_OUT;
        thread->state = SSX_THREAD_STATE_SUSPENDED_RUNNABLE;
        break;

    default:
        SSX_PANIC(SSX_THREAD_TIMEOUT_STATE);
    }
}
        

// This routine serves as a container for the SSX_START_THREADS_HOOK and
// actually starts threads.  The helper routine __ssx_call_ssx_start_threads()
// arranges this routine to be called with interrupts disabled while running
// on the noncritical interrupt stack.
//
// The reason for this roundabout is that we want to be able to run a hook
// routine (transparent to the application) that can hand over every last byte
// of free memory to "malloc()" - including the stack of main().  Since we
// always need to run on some stack, we chose to run the hook on the kernel
// noncritical interrupt stack. However to do this safely we need to make sure
// that no interrupts will happen during this time. When __ssx_thread_resume()
// is finally called all stack-based context is lost but it doesn't matter at
// that point - it's a one-way street into thread execution.
//
// This is considered part of ssx_start_threads() and so is also considered a
// 'core' routine.

void
__ssx_start_threads(void)
{
    SSX_START_THREADS_HOOK;

    __ssx_next_thread_resume();

    SSX_PANIC(SSX_START_THREADS_RETURNED);
}
    

/// Start SSX threads
///
/// This routine starts the SSX thread scheduler infrastructure. This routine
/// must be called after a call of \c ssx_initialize().  This routine never
/// returns. Interrupt (+ timer) only configurations of SSX need not call this
/// routine.
///
/// Note: This tiny routine is considered a 'core' routine so that the
/// initialziation code can safely recover all 'init' code space before
/// starting threads.
///
/// This routine typically does not return - any return value indicates an
/// error; see \ref ssx_errors
///
/// \retval -SSX_ILLEGAL_CONTEXT_THREAD The API was called twice.

int
ssx_start_threads(void)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(__ssx_kernel_mode_thread(), SSX_ILLEGAL_CONTEXT_THREAD);
    }

    __ssx_call_ssx_start_threads();

    return 0;
}


/// Resume a suspended thread
///
/// \param thread The thread to resume
///
/// SSX only allows one thread at a time to run at a given priority, and
/// implements the notion of a thread \e claiming a priority.  A suspended
/// thread claims a priority when it is mapped by a call of
/// ssx_thread_resume().  This API will succeed only if no other active thread
/// is currently mapped at the priority assigned to the thread.  SSX provides
/// the ssx_thread_at_priority() API which allows an application-level
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
/// It is not an error to call ssx_thread_resume() on a mapped
/// thread.  However it is an error to call ssx_thread_resume() on a completed
/// or deleted thread.
/// 
/// Return values other than SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion, including calls on a \a thread that is
/// already mapped.
///
/// \retval -SSX_ILLEGAL_CONTEXT_THREAD The API was called 
/// from a critical interrupt context. 
///
/// \retval -SSX_INVALID_THREAD_AT_RESUME1 The \a thread is a null (0) pointer.
/// 
/// \retval -SSX_INVALID_THREAD_AT_RESUME2 The \a thread is not active, 
/// i.e. has completed or been deleted.
/// 
/// \retval -SSX_PRIORITY_IN_USE_AT_RESUME Another thread is already mapped at 
/// the priority of the \a thread.

int
ssx_thread_resume(SsxThread *thread)
{
    SsxMachineContext ctx;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF_CRITICAL_INTERRUPT_CONTEXT();
        SSX_ERROR_IF(thread == 0, SSX_INVALID_THREAD_AT_RESUME1);
    }

    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF_CRITICAL(!__ssx_thread_is_active(thread),
                              SSX_INVALID_THREAD_AT_RESUME2,
                              &ctx);
    }

    if (!__ssx_thread_is_mapped(thread)) {

        if (SSX_ERROR_CHECK_API) {
            SSX_ERROR_IF_CRITICAL(__ssx_priority_map[thread->priority] != 0,
                                  SSX_PRIORITY_IN_USE_AT_RESUME,
                                  &ctx);
        }
        __ssx_thread_map(thread);
        __ssx_schedule();
    }

    ssx_critical_section_exit(&ctx);

    return SSX_OK;
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
/// later resumed, the suspended call of \c ssx_semaphore_pend() will return the
/// timeout code -SSX_SEMAPHORE_PEND_TIMED_OUT.
///
/// Return values other than SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion, including calls on a \a thread that is
/// already suspended.
///
/// \retval -SSX_ILLEGAL_CONTEXT_THREAD The API was called from a critical 
/// interrupt context. 
///
/// \retval -SSX_INVALID_THREAD_AT_SUSPEND1 The \a thread is a null (0) pointer
/// 
/// \retval -SSX_INVALID_THREAD_AT_SUSPEND2 The \a thread is not active, 
/// i.e. has completed or been deleted.

int
ssx_thread_suspend(SsxThread *thread)
{
    SsxMachineContext ctx;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF_CRITICAL_INTERRUPT_CONTEXT();
        SSX_ERROR_IF((thread == 0), SSX_INVALID_THREAD_AT_SUSPEND1);
    }

    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF_CRITICAL(!__ssx_thread_is_active(thread),
                              SSX_INVALID_THREAD_AT_SUSPEND2,
                              &ctx);
    }

    if (__ssx_thread_is_mapped(thread)) {

        SSX_TRACE_THREAD_SUSPENDED(thread->priority);
        __ssx_thread_unmap(thread);
        __ssx_schedule();
    }

    ssx_critical_section_exit(&ctx);

    return SSX_OK;
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
/// Return values other than SSX_OK (0) are errors; see \ref ssx_errors.  If a
/// thread deletes itself this API does not return at all.
///
/// \retval 0 Successful completion, including calls on a \a thread that has
/// completed or had already been deleted. 
///
/// \retval -SSX_ILLEGAL_CONTEXT_THREAD The API was called from a critical 
/// interrupt context. 
///
/// \retval -SSX_INVALID_THREAD_AT_DELETE The \a thread is a null (0) pointer.

int
ssx_thread_delete(SsxThread *thread)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF_CRITICAL_INTERRUPT_CONTEXT();
        SSX_ERROR_IF(thread == 0, SSX_INVALID_THREAD_AT_DELETE);
    }

    __ssx_thread_delete(thread, SSX_THREAD_STATE_DELETED);

    return SSX_OK;
}


/// Complete a thread
///
/// If a thread ever returns from the subroutine defining the thread entry
/// point, the thread is removed from all SSX kernel data structures and
/// marked completed. The thread routine can also use the API ssx_complete()
/// to make this more explicit if desired. SSX makes no distinction between
/// completed and deleted threads, but provides these indications for
/// the benefit of the application.
///
/// Note that this API is only available from the current thread to mark its
/// own completion.
///
/// Once a thread has completed or been deleted the thread structure and
/// thread stack areas can be used for other purposes.
///
/// Any return value indicates an error; see \ref ssx_errors.  In the event of
/// a successful completion this API does not return to the caller, which is
/// always the thread context being completed.
///
/// \retval -SSX_ILLEGAL_CONTEXT_THREAD The API was not called from a thread 
/// context.

// Note: Casting __ssx_current_thread removes the 'volatile' attribute.

int
ssx_complete(void)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_UNLESS_THREAD_CONTEXT();
    }

    __ssx_thread_delete((SsxThread *)__ssx_current_thread, 
                        SSX_THREAD_STATE_COMPLETED);

    return SSX_OK;
}


/// Sleep a thread until an absolute time
///
/// \param time An absolute time as measured by the SSX timebase
///
/// Threads can use this API to sleep until an absolute time. Sleeping threads
/// are not scheduled, although they maintain their priorities.  This differs
/// from thread suspension, where the suspended thread relinquishes its
/// priority.  When the sleep timer times out the thread becomes runnable
/// again, and will run as soon as it becomes the highest-priority mapped
/// runnable thread.
///
/// Sleeping threads may also be later suspended. In this case the Sleep timer
/// continues to run, and if it times out before the thread is resumed the
/// thread will be immediately runnable when it is resumed.
///
/// See the SSX specification for a full discussion of how SSX handles
/// scheduling events at absolute times "in the past". Briefly stated, if the
/// \a time is in the past, the thread will Sleep for the briefest possible
/// period supported by the hardware.
///
/// Return values other than SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion.
///
/// \retval -SSX_ILLEGAL_CONTEXT_THREAD The API was not called from a thread 
/// context.

// Note: Casting __ssx_current_thread removes the 'volatile' attribute.

int
ssx_sleep_absolute(SsxTimebase time)
{
    SsxMachineContext ctx;
    SsxThread *current;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_UNLESS_THREAD_CONTEXT();
    }

    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    current = (SsxThread *)__ssx_current_thread;

    current->timer.timeout = time;
    __ssx_timer_schedule(&(current->timer));

    current->flags |= SSX_THREAD_FLAG_TIMER_PEND;

    SSX_TRACE_THREAD_SLEEP(current->priority);

    __ssx_thread_queue_delete(&__ssx_run_queue, current->priority);
    __ssx_schedule();

    current->flags &= ~(SSX_THREAD_FLAG_TIMER_PEND | SSX_THREAD_FLAG_TIMED_OUT);

    ssx_critical_section_exit(&ctx);

    return SSX_OK;
}

/// Sleep a thread for an interval relative to the current time.
///
/// \param interval A time interval relative to the current timebase.
///
/// Threads can use this API to sleep for a time relative to the current
/// timebase.  The absolute timeout is \c ssx_timebase_get() + \a interval.
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
/// See the SSX specification for a full discussion of how SSX handles
/// scheduling events at absolute times "in the past". Briefly stated, if the
/// \a interval is 0 or is so small that the absolute time becomes a "past"
/// time before the Sleep is actually scheduled, the thread will Sleep for the
/// briefest possible period supported by the hardware.
///
/// Return values other than SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion.
///
/// \retval -SSX_ILLEGAL_CONTEXT_THREAD The API was not called from a thread 
/// context.

int
ssx_sleep(SsxInterval interval) 
{
    return ssx_sleep_absolute(ssx_timebase_get() + interval);
}


/// Get information about a thread.
///
/// \param thread A pointer to the SsxThread to query
///
/// \param state The value returned through this pointer is the current state
/// of the thread; See \ref ssx_thread_states. The caller can set this
/// parameter to the null pointer (0) if this information is not required.
///
/// \param priority The value returned through this pointer is the current
/// priority of the thread.  The caller can set this parameter to the null
/// pointer (0) if this information is not required.
///
/// \param runnable The value returned through this pointer is 1 if the thread
/// is in state SSX_THREAD_STATE_MAPPED and is currently in the run queue
/// (i.e., neither blocked on a semaphore nor sleeping), otherwise 0. The
/// caller can set this parameter to the null pointer (0) if this information
/// is not required.
///
/// The information returned by this API can only be guaranteed consistent if
/// the API is called from an SSX_NONCRITICAL critical section. Since the
/// implementation of this API does not enforce a critical section, it is not
/// an error to call this API from a critical interrupt context.
///
/// Return values other than SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_THREAD_AT_INFO The \a thread is a null (0) pointer.

int
ssx_thread_info_get(SsxThread         *thread,
                    SsxThreadState    *state,
                    SsxThreadPriority *priority,
                    int               *runnable)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(thread == 0, SSX_INVALID_THREAD_AT_INFO);
    }

    if (state) {
        *state = thread->state;
    }
    if (priority) {
        *priority = thread->priority;
    }
    if (runnable) {
        *runnable = ((thread->state == SSX_THREAD_STATE_MAPPED) &&
                     __ssx_thread_queue_member(&__ssx_run_queue,
                                               thread->priority));
    }
    return SSX_OK;
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
/// Thread priorities can be changed by the \c ssx_thread_priority_change()
/// API. This call will fail if the thread pointer is invalid or if the thread
/// is mapped and the new priority is currently in use.  The call will succeed
/// even if the \a thread is suspended, completed or deleted.  The
/// application-level scheduling algorithm is completely responsible for the
/// correctness of the application in the event of suspended, completed or
/// deleted threads.
///
/// Return values other than SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion, including the redundant case of
/// attempting to change the priority of the thread to its current priority.
///
/// \retval -SSX_ILLEGAL_CONTEXT_THREAD the API was called from a critical 
/// interrupt context. 
///
/// \retval -SSX_INVALID_THREAD_AT_CHANGE The \a thread is null (0) or 
/// otherwise invalid.
///
/// \retval -SSX_INVALID_ARGUMENT_THREAD_CHANGE The \a new_priority is invalid.
///
/// \retval -SSX_PRIORITY_IN_USE_AT_CHANGE The \a thread is mapped and the \a
/// new_priority is currently in use by another thread.

int 
ssx_thread_priority_change(SsxThread         *thread,
                           SsxThreadPriority new_priority,
                           SsxThreadPriority *old_priority)
{
    SsxMachineContext ctx;
    SsxThreadPriority priority;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF_CRITICAL_INTERRUPT_CONTEXT();
        SSX_ERROR_IF(thread == 0, SSX_INVALID_THREAD_AT_CHANGE);
        SSX_ERROR_IF(new_priority > SSX_THREADS, 
        SSX_INVALID_ARGUMENT_THREAD_CHANGE);
    }

    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    priority = thread->priority;

    if (priority != new_priority) {

        if (!__ssx_thread_is_mapped(thread)) {

            thread->priority = new_priority;

        } else {

            if (SSX_ERROR_CHECK_API) {
                SSX_ERROR_IF_CRITICAL(__ssx_priority_map[new_priority] != 0,
                                      SSX_PRIORITY_IN_USE_AT_CHANGE,
                                      &ctx);
            }

            __ssx_thread_unmap(thread);
            thread->priority = new_priority;
            __ssx_thread_map(thread);
            __ssx_schedule();
        }
    }

    if (old_priority) {
        *old_priority = priority;
    }

    ssx_critical_section_exit(&ctx);

    return SSX_OK;
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
/// the API is called from an SSX_NONCRITICAL critical section. Since the
/// implementation of this API does not require a critical section, it is not
/// an error to call this API from a critical interrupt context.
///
/// Return values other than SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion.
///
/// \retval -SSX_INVALID_ARGUMENT_THREAD_PRIORITY The \a priority is invalid 
/// or the \a thread parameter is null (0). 

int
ssx_thread_at_priority(SsxThreadPriority priority,
                       SsxThread         **thread)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((priority > SSX_THREADS) || (thread == 0),
                     SSX_INVALID_ARGUMENT_THREAD_PRIORITY);
    }

    *thread = __ssx_thread_at_priority(priority);

    return SSX_OK;
}


/// Swap thread priorities
///
/// \param thread_a A pointer to an initialized SsxThread
///
/// \param thread_b A pointer to an initialized SsxThread
///
/// This API swaps the priorities of \a thread_a and \a thread_b.  The API is
/// provided to support general and efficient application-directed scheduling
/// algorithms.  The requirements on the \a thread_a and \a thread_b arguments
/// are that they are valid pointers to initialized SsxThread structures, that
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
/// mapped after the call of ssx_thread_priority_swap() if and only if it was
/// mapped prior to the call.  If the new priority of a mapped thread is
/// currently in use (by a thread other than the swap partner), then the
/// SSX_PRIORITY_IN_USE_AT_SWAP error is signalled and the swap does not take 
/// place. This could only happen if the swap partner is not currently mapped.
///
/// It is legal for a thread to swap its own priority with another thread. The
/// degenerate case that \a thread_a and \a thread_b are equal is also legal -
/// but has no effect.
///
/// Return values other than SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion, including the redundant cases that do not
/// actually change priorities, or the cases that assign new priorities to
/// suspended, completed or deleted threads.
///
/// \retval -SSX_ILLEGAL_CONTEXT_THREAD the API was called from a critical 
/// interrupt context. 
///
/// \retval -SSX_INVALID_THREAD_AT_SWAP1 One or both of \a thread_a and 
/// \a thread_b is null (0) or otherwise invalid, 
///
/// \retval -SSX_INVALID_THREAD_AT_SWAP2 the priorities of One or both of 
/// \a thread_a and \a thread_b are invalid.
///
/// \retval -SSX_INVALID_ARGUMENT One or both of the priorities
/// of \a thread_a and \a thread_b is invalid.
///
/// \retval -SSX_PRIORITY_IN_USE_AT_SWAP Returned if a thread is mapped and the
/// new thread priority is currently in use by another thread (other than the
/// swap partner).

int
ssx_thread_priority_swap(SsxThread* thread_a, SsxThread* thread_b)
{
    SsxMachineContext ctx;
    SsxThreadPriority priority_a, priority_b;
    int mapped_a, mapped_b;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF_CRITICAL_INTERRUPT_CONTEXT();
        SSX_ERROR_IF((thread_a == 0) ||  (thread_b == 0), 
                       SSX_INVALID_THREAD_AT_SWAP1);
    }

    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    if (thread_a != thread_b) {

        mapped_a = __ssx_thread_is_mapped(thread_a);
        mapped_b = __ssx_thread_is_mapped(thread_b);
        priority_a = thread_a->priority;
        priority_b = thread_b->priority;

        if (SSX_ERROR_CHECK_API) {
            int priority_in_use;
            SSX_ERROR_IF_CRITICAL((priority_a > SSX_THREADS) ||
                                  (priority_b > SSX_THREADS),
                                  SSX_INVALID_THREAD_AT_SWAP2,
                                  &ctx);
            priority_in_use = 
                (mapped_a && !mapped_b &&
                 (__ssx_thread_at_priority(priority_b) != 0)) ||
                (!mapped_a && mapped_b && 
                 (__ssx_thread_at_priority(priority_a) != 0));
            SSX_ERROR_IF_CRITICAL(priority_in_use, 
                                  SSX_PRIORITY_IN_USE_AT_SWAP, &ctx); 
        }

        if (mapped_a) {
            __ssx_thread_unmap(thread_a);
        }
        if (mapped_b) {
            __ssx_thread_unmap(thread_b);
        }            
        thread_a->priority = priority_b;
        thread_b->priority = priority_a;
        if (mapped_a) {
            __ssx_thread_map(thread_a);
        }
        if (mapped_b) {
            __ssx_thread_map(thread_b);
        }
        __ssx_schedule();
    }

    ssx_critical_section_exit(&ctx);

    return SSX_OK;
}


#undef __SSX_THREAD_CORE_C__
