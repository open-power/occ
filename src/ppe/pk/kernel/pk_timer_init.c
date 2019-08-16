/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/kernel/pk_timer_init.c $                           */
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

/// \file pk_timer_init.c
/// \brief PK timer initialization
///
/// The entry points in this file might only be used during initialization of
/// the application.  In this case the code space for these routines could be
/// recovered and reused after initialization.

#include "pk.h"


/// Create (initialize) a timer.
///
/// \param timer The PkTimer to initialize.
///
/// \param callback The timer callback
///
/// \param arg Private data provided to the callback.
///
/// Once created with pk_timer_create() a timer can be scheduled with
/// pk_timer_schedule() or pk_timer_schedule_absolute(), which queues the
/// timer in the kernel time queue.  Timers can be cancelled by a call of
/// pk_timer_cancel().
///
/// Timers created with pk_timer_create() are always run as
/// bottom-half handlers with interrupt preemption enabled. Timer callbacks are
/// free to enter critical sections if required, but must
/// always exit with interrupts enabled.
///
/// Caution: PK has no way to know if an PkTimer structure provided to
/// pk_timer_create() is safe to use as a timer, and will silently modify
/// whatever memory is provided.
///
/// Return values other then PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion
///
/// \retval -PK_INVALID_TIMER_AT_CREATE The \a timer is a null (0) pointer.

int
pk_timer_create(PkTimer*         timer,
                PkTimerCallback callback,
                void*             arg)
{
    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF((timer == 0), PK_INVALID_TIMER_AT_CREATE);
    }

    pk_deque_element_create((PkDeque*)timer);
    timer->timeout = 0;
    timer->callback = callback;
    timer->arg = arg;

    return PK_OK;
}


