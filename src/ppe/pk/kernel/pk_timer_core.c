/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/kernel/pk_timer_core.c $                           */
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

/// \file pk_timer_core.c
/// \brief PK portable kernel timer handler
///
/// This file contains core routines that would be needed by any application
/// that requires PK timer support at runtime.
///
/// PK implements a 'tickless' kernel - all events are scheduled at absolute
/// times of the PK timebase.  This approach gives the application full
/// control over granularity of event scheduling.  Scheduling in absolute time
/// opens up the possibility of scheduling events "in the past".  PK
/// uniformly handles this case by scheduling "past" events to occur 1
/// timebase tick in the future, so that timer callbacks are always run in the
/// expected interrupt context.
///
/// PK implements the time queue as a simple unordered list of events, plus a
/// dedicated variable that holds the earliest timeout of any event in the
/// list.  This is thought to be an appropriate data structure for the
/// following reasons:
///
/// - PK applications will be small and will not schedule a large number of
/// events.  Therefore the cost of scanning the list each time an event times
/// out is balanced against the cost of maintaining the list as a sorted data
/// structure each time an event is added or removed from the event queue.
///
/// - PK applications may schedule and cancel many, many more events (safety
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
/// any event that has timed out has its callback processed.
/// Since event and callback processing take
/// time, the list is potentially scanned multiple times until there are no
/// more timed-out events in the list.
///
/// Note that callbacks are not necessarily processed in time-order.  In this
/// sense the PK time queue is like a traditional tick-based time queue in
/// that events are effectively lumped into groups of events that time out
/// together.  In a tick-based kernel the 'lump' is the tick interval; here
/// the 'lump' is a variable interval that corresponds to the time it takes to
/// process the entire event list.
///
/// Timer callbacks are typically run with interrupt preemption enabled.
/// Special callbacks may run without preemption.  This is the only part of
/// the PK kernel where data structures of indeterminate size are processed.
/// During processing of the event list by the timer interrupt handler, the
/// consideration of each event always includes a window of preemptability.

#define __PK_TIMER_CORE_C__

#include "pk.h"

// Declare the timer bottom half handler
static PK_BH_HANDLER(__pk_timer_bh_handler);

// Define the timer bottom half handler that the interrupt handler will
// schedule
PK_BH_STATIC_CREATE(pk_timer_bh, __pk_timer_bh_handler, 0);


// This routine is only used in this file, and will always be called in a
// critical section.

static inline int
timer_active(PkTimer* timer)
{
    return pk_deque_is_queued((PkDeque*)timer);
}


// This is the kernel version of pk_timer_cancel().
//
// This routine is used here and by thread and semaphore routines.
// External interrupts must be disabled at entry.
//
// If the timer is active, then there is a special case if we are going to
// delete the 'cursor' - that is the timer that __pk_timer_handler() is going
// to handle next.  In this case we need to move the cursor to the next timer
// in the queue.
//
//  Note that cancelling a timer does not cause a re-evaluation of the next
//  timeout.  This will happen naturally when the current timeout expires.

int
__pk_timer_cancel(PkTimer* timer)
{
    int rc;
    PkDeque* timer_deque = (PkDeque*)timer;
    PkTimeQueue* tq = &__pk_time_queue;

    if (!timer_active(timer))
    {

        rc = -PK_TIMER_NOT_ACTIVE;

    }
    else
    {

        if (timer_deque == tq->cursor)
        {
            tq->cursor = tq->cursor->next;
        }

        pk_deque_delete(timer_deque);
        rc = 0;
    }

    return rc;
}


// This is the kernel version of pk_timer_schedule().
//
// This routine is used here and by thread and semaphore routines.
// interrupts must be disabled at entry.
//
// Unless the timer is already active it is enqueued in the doubly-linked
// timer list by inserting the timer at the end of the queue. Then the
// hardware timeout is scheduled if necessary. If the time queue 'cursor' != 0
// we are in the midst of processing the time queue, and the end of time queue
// processing will schedule the next hardware timemout.

void
__pk_timer_schedule(PkTimer* timer)
{
    PkTimeQueue* tq = &__pk_time_queue;

    if (!timer_active(timer))
    {
        pk_deque_push_back((PkDeque*)tq, (PkDeque*)timer);
    }

    if (timer->timeout < tq->next_timeout)
    {
        tq->next_timeout = timer->timeout;

        if (tq->cursor == 0)
        {
            __pk_schedule_hardware_timeout(tq->next_timeout);
        }
    }
}


// The tickless timer mechanism has timed out.  Note that due to timer
// deletions and other factors, there may not actually be a timer in the queue
// that has timed out - but it doesn't matter (other than for efficiency).
//
// This routine must not be entered reentrantly.
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
// by running this function as a bottom half.  As such, interrupts are only
// disabled when explicitly requested.
//
// Because interrupt preemption is enabled during processing, and preempting
// handlers may invoke time queue operations, we need to establish a pointer
// to the next entry to be examined (tq->cursor) before enabling interupts.
// It's possible that this pointer will be changed by other interrupt handlers
// that cancel the timer pointed to by tq->cursor.
//
// The main loop iterates on the PkDeque form of the time queue, casting each
// element back up to the PkTimer as it is processed.

static void
__pk_timer_bh_handler(void* arg)
{
    PkMachineContext ctx;
    PkTimeQueue* tq;
    PkTimebase now;
    PkTimer* timer;
    PkDeque* timer_deque;
    PkTimerCallback callback;

    tq = &__pk_time_queue;

    // Check if we entered the function while it was running in another context.
    if (PK_ERROR_CHECK_KERNEL)
    {
        if (tq->cursor != 0)
        {
            PK_PANIC(PK_TIMER_HANDLER_INVARIANT);
        }
    }

    pk_critical_section_enter(&ctx);

    while ((now = pk_timebase_get()) >= tq->next_timeout)
    {
        tq->next_timeout = PK_TIMEBASE_MAX;
        timer_deque = ((PkDeque*)tq)->next;

        // Iterate through the entire timer list, calling the callback of
        // timed-out elements and finding the timer that will timeout next,
        // which is stored in tq->next_timeout.
        while (timer_deque != (PkDeque*)tq)
        {

            timer = (PkTimer*)timer_deque;

            // Setting this to a non-zero value indicates we are in the middle
            // of processing the time queue.
            tq->cursor = timer_deque->next;

            if (timer->timeout <= now)
            {

                // The timer timed out.  It is removed from the queue.
                //
                // The callback may be made with interrupt preemption enabled
                // or disabled.  However to mitigate kernel interrupt latency
                // we go ahead and open up to interrupts after the callback if
                // the callback itself was not preemptible.

                pk_deque_delete(timer_deque);

                pk_critical_section_exit(&ctx);

                callback = timer->callback;

                if (callback)
                {
                    callback(timer->arg);
                }

            }
            else
            {

                // This timer has not timed out.  Its timeout will simply
                // participate in the computation of the next timeout.
                tq->next_timeout = MIN(timer->timeout, tq->next_timeout);
                pk_critical_section_exit(&ctx);
            }

            timer_deque = tq->cursor;
            pk_critical_section_enter(&ctx);
        }

        // Time has passed since we checked the time.  Loop back
        // to check the time again and see if enough time has passed
        // that the next timer has timed out too.
    }

    pk_critical_section_exit(&ctx);

    // This marks that we are no longer processing the time queue
    tq->cursor = 0;

    // Finally, reschedule the next timeout
    __pk_schedule_hardware_timeout(tq->next_timeout);
}


void
__pk_timer_handler(void)
{
    //schedule the timer bottom half handler which
    //is preemptible.
    pk_bh_schedule(&pk_timer_bh);
}


/// Schedule a timer for an interval relative to the current time.
///
/// \param timer The PkTimer to schedule.
///
/// \param interval The timer will be scheduled to time out at the current
/// time (pk_timebase_get()) plus this \a interval.
///
/// Once created with pk_timer_create() a timer can be \e scheduled, which
/// queues the timer in the kernel time queue.  It is not an error to call \c
/// pk_timer_schedule() on a timer that is already scheduled in the time
/// queue - the timer is simply rescheduled with the new characteristics.
///
/// Return values other than PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion
///
/// \retval -PK_INVALID_TIMER_AT_SCHEDULE A a null (0) pointer was provided as
/// the \a timer argument.
///

int
pk_timer_schedule(PkTimer*    timer,
                  PkInterval interval)
{
    PkMachineContext ctx;
    PkTimebase  timeout = pk_timebase_get() + PK_INTERVAL_SCALE(interval);

    pk_critical_section_enter(&ctx);

    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF(timer == 0, PK_INVALID_TIMER_AT_SCHEDULE);
    }

    timer->timeout = timeout;
    __pk_timer_schedule(timer);

    pk_critical_section_exit(&ctx);

    return PK_OK;
}


/// Cancel (dequeue) a timer.
///
/// \param timer The PkTimer to cancel.
///
/// Timers can be canceled at any time.  It is never an error to call
/// pk_timer_cancel() on an PkTimer object after it is created. Memory used
/// by an PkTimer can be safely reused for another purpose after a successful
/// call ofpk_timer_cancel().
///
/// Return values other than PK_OK (0) are not necessarily errors; see \ref
/// pk_errors
///
/// The following return codes are non-error codes:
///
/// \retval 0 Successful completion
///
/// \retval -PK_TIMER_NOT_ACTIVE The \a timer is not currently scheduled,
/// i.e. it was never scheduled or has timed out.  This code is returned for
/// information only and is not considered an error.
///
/// The following return codes are  error codes:
///
/// \retval -PK_INVALID_TIMER_AT_CANCEL The \a timer is a null (0) pointer.
///

int
pk_timer_cancel(PkTimer* timer)
{
    PkMachineContext ctx;
    int rc = PK_OK;

    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF(timer == 0, PK_INVALID_TIMER_AT_CANCEL);
    }

    pk_critical_section_enter(&ctx);

    rc = __pk_timer_cancel(timer);

    pk_critical_section_exit(&ctx);

    return rc;
}


/// Get information about a timer.
///
/// \param timer The PkTimer to query
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
/// the API is called from a critical section.
///
/// Return values other than PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion
///
/// \retval -PK_INVALID_TIMER_AT_INFO The \a timer is a null (0) pointer.

int
pk_timer_info_get(PkTimer*    timer,
                  PkTimebase* timeout,
                  int*         active)

{
    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF(timer == 0, PK_INVALID_TIMER_AT_INFO);
    }

    if (timeout)
    {
        *timeout = timer->timeout;
    }

    if (active)
    {
        *active = timer_active(timer);
    }

    return PK_OK;
}

#undef __PK_TIMER_CORE_C__
