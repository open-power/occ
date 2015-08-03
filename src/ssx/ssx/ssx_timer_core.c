/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ssx/ssx_timer_core.c $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2015                        */
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

/// \file ssx_timer_core.c
/// \brief SSX portable kernel timer handler
///
/// This file contains core routines that would be needed by any application
/// that requires SSX timer support at runtime.
///
/// SSX implements a 'tickless' kernel - all events are scheduled at absolute
/// times of the SSX timebase.  This approach gives the application full
/// control over granularity of event scheduling.  Scheduling in absolute time
/// opens up the possibility of scheduling events "in the past".  SSX
/// uniformly handles this case by scheduling "past" events to occur 1
/// timebase tick in the future, so that timer callbacks are always run in the
/// expected noncritical interrupt context.
///
/// SSX implements the time queue as a simple unordered list of events, plus a
/// dedicated variable that holds the earliest timeout of any event in the
/// list.  This is thought to be an appropriate data structure for the
/// following reasons:
///
/// - SSX applications will be small and will not schedule a large number of
/// events.  Therefore the cost of scanning the list each time an event times
/// out is balanced against the cost of maintaining the list as a sorted data
/// structure each time an event is added or removed from the event queue.
///
/// - SSX applications may schedule and cancel many, many more events (safety
/// timeouts) than are ever allowed to expire. Events can be added and deleted
/// from the simple DEQUE very quickly since there is no sorting
/// overhead.
///
/// Events are added to the queue simply by placing them at the end of the
/// queue.  If the new event times out earlier than the previous earliest
/// event, the hardware timeout is rescheduled for the new event time.  Events
/// are deleted from the queue (cancelled) simply by deleting them.  Deletion
/// does not affect the hardware timeout, even if the deleted event would have
/// been the next to time out.  It is not an error for the timer handler to
/// take a timer interrupt and find no events pending.  Pending events can
/// also be rescheduled in place.
///
/// When a timeout occurs the event list is scanned from the beginning, and
/// any event that has timed out is rescheduled if necessary (periodic events)
/// and its callback is processed.  Since event and callback processing take
/// time, the list is potentially scanned multiple times until there are no
/// more timed-out events in the list.
///
/// Note that callbacks are not necessarily processed in time-order.  In this
/// sense the SSX time queue is like a traditional tick-based time queue in
/// that events are effectively lumped into groups of events that time out
/// together.  In a tick-based kernel the 'lump' is the tick interval; here
/// the 'lump' is a variable interval that corresponds to the time it takes to
/// process the entire event list.
///
/// Timer callbacks are typically run with interrupt preemption enabled.
/// Special callbacks may run without preemption.  This is the only part of
/// the SSX kernel where data structures of indeterminate size are processed.
/// During processing of the event list by the timer interrupt handler, the
/// consideration of each event always includes a window of preemptability.

#define __SSX_TIMER_CORE_C__

#include "ssx.h"

// This routine is only used in this file, and will always be called in 
// critical section.

static inline int
timer_active(SsxTimer* timer)
{
    return ssx_deque_is_queued((SsxDeque*)timer);
}


// This is the kernel version of ssx_timer_cancel().
//
// This routine is used here and by thread and semaphore routines.
// Noncritical interrupts must be disabled at entry.
//
// If the timer is active, then there is a special case if we are going to
// delete the 'cursor' - that is the timer that __ssx_timer_handler() is going
// to handle next.  In this case we need to move the cursor to the next timer
// in the queue.
//
//  Note that cancelling a timer does not cause a re-evaluation of the next
//  timeout.  This will happen naturally when the current timeout expires.

int
__ssx_timer_cancel(SsxTimer *timer)
{
    int rc;
    SsxDeque* timer_deque = (SsxDeque*)timer;
    SsxTimeQueue* tq = &__ssx_time_queue;

    if (!timer_active(timer)) {

        rc = -SSX_TIMER_NOT_ACTIVE;

    } else {

        if (timer_deque == tq->cursor) {
            tq->cursor = tq->cursor->next;
        }
        ssx_deque_delete(timer_deque);
        rc = 0;
    }
    return rc;
}


// This is the kernel version of ssx_timer_schedule().
//
// This routine is used here and by thread and semaphore routines.
// Noncritical interrupts must be disabled at entry.
//
// Unless the timer is already active it is enqueued in the doubly-linked
// timer list by inserting the timer at the end of the queue. Then the
// hardware timeout is scheduled if necessary. If the time queue 'cursor' != 0
// we are in the midst of processing the time queue, and the end of time queue
// processing will schedule the next hardware timemout.

void
__ssx_timer_schedule(SsxTimer* timer)
{
    SsxTimeQueue* tq = &__ssx_time_queue;

    if (!timer_active(timer)) {
        ssx_deque_push_back((SsxDeque*)tq, (SsxDeque*)timer);
    }

    if (timer->timeout < tq->next_timeout) {
        tq->next_timeout = timer->timeout;
        if (tq->cursor == 0) {
            __ssx_schedule_hardware_timeout(tq->next_timeout);
        }
    }
}
        

// The tickless timer mechanism has timed out.  Note that due to timer
// deletions and other factors, there may not actually be a timer in the queue
// that has timed out - but it doesn't matter (other than for efficiency).
//
// Noncritical interrupts are (must be) disabled at entry, and this invariant
// is checked. This routine must not be entered reentrantly. 
//
// First, time out any timers that have expired.  Timers in the queue are
// unordered, so we have to check every one.  Since passing through the
// loop takes time, we may have to make multiple passes until we know
// that there are no timers in the queue that have already timed
// out. Note that it would also work to only go through the loop once and
// let the hardware scheduler take care of looping, but that would imply
// more overhead than the current implementation.
//
// On each pass through the loop tq->next_timeout computes the minimum timeout
// of events remaining in the queue.  This is the only part of the kernel that
// searches a list of indefinite length. Kernel interrupt latency is mitigated
// by running callbacks with interrupts disabled either during or after the
// call for timed out events, and also after every check for events that have
// not timed out.
//
// Because interrupt preemption is enabled during processing, and preempting
// handlers may invoke time queue operations, we need to establish a pointer
// to the next entry to be examined (tq->cursor) before enabling interupts.
// It's possible that this pointer will be changed by other interrupt handlers
// that cancel the timer pointed to by tq->cursor. 
//
// The main loop iterates on the SsxDeque form of the time queue, casting each
// element back up to the SsxTimer as it is processed.

void
__ssx_timer_handler()
{
    SsxTimeQueue* tq;
    SsxTimebase now;
    SsxTimer* timer;
    SsxDeque* timer_deque;
    SsxTimerCallback callback;

    tq = &__ssx_time_queue;

    if (SSX_ERROR_CHECK_KERNEL) {
        if (tq->cursor != 0) {
            SSX_PANIC(SSX_TIMER_HANDLER_INVARIANT);
        }
    }

    while ((now = ssx_timebase_get()) >= tq->next_timeout) {

        tq->next_timeout = SSX_TIMEBASE_MAX;
        timer_deque = ((SsxDeque*)tq)->next;

        while (timer_deque != (SsxDeque*)tq) {
    
            timer = (SsxTimer*)timer_deque;
            tq->cursor = timer_deque->next;

            if (timer->timeout <= now) {

                // The timer timed out.  It is removed from the queue unless
                // it is a peridic timer that needs to be rescheduled.  We do
                // rescheduling here in the critical section to correctly
                // handle timers whose callbacks may cancel the timer.  The
                // timer is rescheduled in absolute time.
                //
                // The callback may be made with interrupt preemption enabled
                // or disabled.  However to mitigate kernel interrupt latency
                // we go ahead and open up to interrupts after the callback if
                // the callback itself was not preemptible.

                if (timer->period == 0) {
                    ssx_deque_delete(timer_deque);
                } else {
                    timer->timeout += timer->period;
                    tq->next_timeout = MIN(timer->timeout, tq->next_timeout);
                }

                callback = timer->callback;
                if (callback) {
                    if (timer->options & SSX_TIMER_CALLBACK_PREEMPTIBLE) {
                        ssx_interrupt_preemption_enable();
                        callback(timer->arg);
                    } else {
                        callback(timer->arg);
                        ssx_interrupt_preemption_enable();
                    }
                }                        
                ssx_interrupt_preemption_disable();

            } else {

                // This timer has not timed out.  Its timeout will simply
                // participate in the computation of the next timeout.  For
                // interrupt latency reasons we always allow a period of
                // interrupt preemption.

                tq->next_timeout = MIN(timer->timeout, tq->next_timeout);
                ssx_interrupt_preemption_enable();
                ssx_interrupt_preemption_disable();
            }

            timer_deque = tq->cursor;
        }
    }
    
    tq->cursor = 0;

    // Finally, reschedule the next timeout

    __ssx_schedule_hardware_timeout(tq->next_timeout);
}


/// Schedule a timer in absolute time.
///
/// \param timer The SsxTimer to schedule.
///
/// \param timeout The timer will be scheduled to time out at this absolute
/// time.  Note that if the \a timeout is less than the current time then the
/// timer will be scheduled at a minimum timeout in the future and the
/// callback will be executed in an interrupt context.
///
/// \param period If non-0, then when the timer times out it will rescheduled
/// to time out again at the absolute time equal to the last timeout time plus
/// the \a period.  By convention a \a period of 0 indicates a one-shot
/// timer that is not rescheduled.
///
/// Once created with ssx_timer_create() a timer can be \e scheduled, which
/// queues the timer in the kernel time queue.  It is not an error to call 
/// ssx_timer_schedule() on a timer that is already scheduled in the time
/// queue - the timer is simply rescheduled with the new characteristics.
///
/// Return values other than SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_TIMER_AT_SCHEDULE A a null (0) pointer was provided as 
/// the \a timer argument.
///
/// \retval -SSX_ILLEGAL_CONTEXT_TIMER The call was made from a critical 
/// interrupt context. 

int
ssx_timer_schedule_absolute(SsxTimer    *timer,
                            SsxTimebase timeout,
                            SsxInterval period)
                   
{
    SsxMachineContext ctx;

    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(timer == 0, SSX_INVALID_TIMER_AT_SCHEDULE);
        SSX_ERROR_IF(__ssx_kernel_context_critical_interrupt(),
                     SSX_ILLEGAL_CONTEXT_TIMER);
    }

    timer->timeout = timeout;
    timer->period  = period;
    __ssx_timer_schedule(timer);

    ssx_critical_section_exit(&ctx);

    return SSX_OK;
}


/// Schedule a timer for an interval relative to the current time.
///
/// \param timer The SsxTimer to schedule.
///
/// \param interval The timer will be scheduled to time out at the current
/// time (ssx_timebase_get()) plus this \a interval.
///
/// \param period If non-0, then when the timer times out it will rescheduled
/// to time out again at the absolute time equal to the last timeout time plus
/// the \a period.  By convention a \a period of 0 indicates a one-shot
/// timer that is not rescheduled.
///
/// Once created with ssx_timer_create() a timer can be \e scheduled, which
/// queues the timer in the kernel time queue.  It is not an error to call \c
/// ssx_timer_schedule() on a timer that is already scheduled in the time
/// queue - the timer is simply rescheduled with the new characteristics.
///
/// Return values other than SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_TIMER_AT_SCHEDULE A a null (0) pointer was provided as 
/// the \a timer argument.
///
/// \retval -SSX_ILLEGAL_CONTEXT_TIMER The call was made from a critical 
/// interrupt context. 

int
ssx_timer_schedule(SsxTimer    *timer, 
                   SsxInterval interval, 
                   SsxInterval period)
{
    return ssx_timer_schedule_absolute(timer,
                                       ssx_timebase_get() + interval,
                                       period);
}


/// Cancel (dequeue) a timer.
///
/// \param timer The SsxTimer to cancel.
///
/// Timers can be canceled at any time.  It is never an error to call 
/// ssx_timer_cancel() on an SsxTimer object after it is created. Memory used
/// by an SsxTimer can be safely reused for another purpose after a successful
/// call ofssx_timer_cancel().
///
/// Return values other than SSX_OK (0) are not necessarily errors; see \ref
/// ssx_errors 
///
/// The following return codes are non-error codes:
///
/// \retval 0 Successful completion
///
/// \retval -SSX_TIMER_NOT_ACTIVE The \a timer is not currently scheduled,
/// i.e. it was never scheduled or has timed out.  This code is returned for
/// information only and is not considered an error.
///
/// The following return codes are  error codes:
///
/// \retval -SSX_INVALID_TIMER_AT_CANCEL The \a timer is a null (0) pointer.
///
/// \retval -SSX_ILLEGAL_CONTEXT_TIMER The call was made from a critical 
/// interrupt context. 
///

int
ssx_timer_cancel(SsxTimer *timer)
{
    SsxMachineContext ctx;
    int rc = SSX_OK;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF_CRITICAL_INTERRUPT_CONTEXT();
        SSX_ERROR_IF(timer == 0, SSX_INVALID_TIMER_AT_CANCEL);
    }

    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    rc = __ssx_timer_cancel(timer);

    ssx_critical_section_exit(&ctx);

    return rc;
}


/// Get information about a timer.
///
/// \param timer The SsxTimer to query
///
/// \param timeout The API returns the absolute timeout of the timer through
/// this pointer.  If the timer is active, this is the current timeout.  If
/// the timer has timed out then this is the previous absolute timeout.  If
/// the timer was never scheduled this will be 0.  The caller can set this
/// parameter to the null pointer (0) if this information is not required.
///
/// \param active If the value returned through this pointer is 1 then the
/// timer is active (currently scheduled), otherwise the value will be 0
/// indicating an inactive timer. The caller can set this parameter to the
/// null pointer (0) if this information is not required.
///
/// The information returned by this API can only be guaranteed consistent if
/// the API is called from an SSX_NONCRITICAL critical section. Since the
/// implementation of this API does not require a critical section, it is not
/// an error to call this API from a critical interrupt context.
///
/// Return values other than SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_TIMER_AT_INFO The \a timer is a null (0) pointer.

int
ssx_timer_info_get(SsxTimer    *timer,
                   SsxTimebase *timeout,
                   int         *active)
                   
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(timer == 0, SSX_INVALID_TIMER_AT_INFO);
    }

    if (timeout) {
        *timeout = timer->timeout;
    }
    if (active) {
        *active = timer_active(timer);
    }

    return SSX_OK;
}

#undef __SSX_TIMER_CORE_C__
