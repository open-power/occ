/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ssx/ssx_timer_init.c $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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

/// \file ssx_timer_init.c
/// \brief SSX timer initialization
///
/// The entry points in this file might only be used during initialization of
/// the application.  In this case the code space for these routines could be
/// recovered and reused after initialization.

#include "ssx.h"

// Implementation of timer creation

static int
_ssx_timer_create(SsxTimer*         timer,
                  SsxTimerCallback callback,
                  void*             arg,
                  int              options)
{
    if (SSX_ERROR_CHECK_API)
    {
        SSX_ERROR_IF((timer == 0), SSX_INVALID_TIMER_AT_CREATE);
    }

    ssx_deque_element_create((SsxDeque*)timer);
    timer->timeout = 0;
    timer->period = 0;
    timer->callback = callback;
    timer->arg = arg;
    timer->options = options;

    return SSX_OK;
}


/// Create (initialize) a preemptible timer.
///
/// \param timer The SsxTimer to initialize.
///
/// \param callback The timer callback
///
/// \param arg Private data provided to the callback.
///
/// Once created with ssx_timer_create() a timer can be scheduled with
/// ssx_timer_schedule() or ssx_timer_schedule_absolute(), which queues the
/// timer in the kernel time queue.  Timers can be cancelled by a call of
/// ssx_timer_cancel().
///
/// Timers created with ssx_timer_create() are always run as noncritical
/// interrupt handlers with interrupt preemption enabled. Timer callbacks are
/// free to enter critical sections of any priorioty if required, but must
/// always exit with noncritical interrupts enabled.
///
/// Caution: SSX has no way to know if an SsxTimer structure provided to
/// ssx_timer_create() is safe to use as a timer, and will silently modify
/// whatever memory is provided.
///
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_TIMER_AT_CREATE The \a timer is a null (0) pointer.

int
ssx_timer_create(SsxTimer*         timer,
                 SsxTimerCallback callback,
                 void*             arg)
{
    return _ssx_timer_create(timer, callback, arg,
                             SSX_TIMER_CALLBACK_PREEMPTIBLE);
}


/// Create (initialize) a nonpreemptible timer.
///
/// \param timer The SsxTimer to initialize.
///
/// \param callback The timer callback
///
/// \param arg Private data provided to the callback.
///
/// Once created with ssx_timer_create_preemptible() a timer can be scheduled
/// with ssx_timer_schedule() or ssx_timer_schedule_absolute(), which queues
/// the timer in the kernel time queue.  Timers can be cancelled by a call of
/// ssx_timer_cancel().
///
/// Timers created with ssx_timer_create_nonpreemptible() are always run as
/// noncritical interrupt handlers with interrupt preemption disabled. Timer
/// callbacks are free to later enable preemption if desired, but must always
/// exit with noncritical interrupts disabled.
///
/// \note The use of ssx_timer_create_nonpreemptible() should be rare, and the
/// timer callbacks should be short and sweet to avoid long interrupt
/// latencies for other interrupts. This API was initially introduced for use
/// by the SSX kernel itself when scheduling thread-timer callbacks to avoid
/// potential race conditions with other interrupts that may modify thread
/// state or the state of the time queue. Applications may also require this
/// facility to guarantee a consistent state in the event that other
/// interrupts may cancel the timer.
///
/// Caution: SSX has no way to know if an SsxTimer structure provided to
/// ssx_timer_create() is safe to use as a timer, and will silently modify
/// whatever memory is provided.
///
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_TIMER_AT_CREATE The \a timer is a null (0) pointer.

int
ssx_timer_create_nonpreemptible(SsxTimer*         timer,
                                SsxTimerCallback callback,
                                void*             arg)
{
    return _ssx_timer_create(timer, callback, arg, 0);
}


