/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/polling.h $                                           */
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
#ifndef __POLLING_H__
#define __POLLING_H__
/// \file polling.c
/// \brief Library APIs for polling and busy-waiting

#include "ssx.h"

// Return/Panic codes

#define POLLING_TIMEOUT   0x00765501
#define POLLING_ERROR     0x00765502
#define POLLING_CONDITION 0x00765503


#ifndef __ASSEMBLER__

/// Poll for a condition or a timeout with optional sleep
///
/// \param[out] o_rc The last return code from calling \a i_condition. This
/// will only be valid if the return code from polling() is
/// POLLING_CONDITION. This argument may be passed as NULL (0) if the caller
/// does not require this information.
///
/// \param[in] i_condition A function of two arguments, returning an integer
/// return code - 0 for success, non-0 for failure. The first argument is a
/// private state or parameter variable.  The second argument is used to
/// return the truth value of the \a i_condition predicate (0 for false, non-0
/// for true), and is only considered if the return value of \a i_condition is
/// 0.
///
/// \param[in,out] io_arg The private argument of the \a condition function.
///
/// \param[in] i_timeout The maximum amount of time to poll the \a condition
/// before declaring a timeout. The special value SSX_WAIT_FOREVER can be 
/// used to specify polling without timeout.
///
/// \param[in] i_sleep If non-0 at entry, then the thread will sleep for this
/// interval between polls of the condition.  Otherwise the polling is
/// continuous. polling() can only be called with i_sleep non-0 from a
/// thread context (since interrupt contexts can not block).
///
/// polling() implements a generic polling protocol for conditions that can
/// not be recognized as interrupt events.  polling() polls the \a i_condition
/// until either an error is encountered, the condition is true, or the
/// polling times out as measured by the SSX timebase. Whenever a timeout is
/// detected the condition is polled once more to exclude false timeouts that
/// may have been caused by thread preemption.
///
/// The \a i_sleep A non-0 value of \a i_sleep specifies that the thread
/// should sleep for the given interval between polling tries instead of
/// polling continuously. A non-0 \a i_sleep argument is only legal in thread
/// contexts.
///
/// \retval 0 Success; The condition was satisfied prior to the timeout.
///
/// \retval POLLING_TIMEOUT A timeout was detected before the condition became
/// valid. 
///
/// \retval POLLING_ERROR This code is returned if any of the arguments of
/// polling() are invalid.
///
/// \retval POLLING_CONDITION This code is returned if the \a i_condition
/// function returns a non-0 return code.
///
/// If the embedded call of ssx_sleep() fails for some reason then the return
/// code will be the code returned by ssx_sleep().
int
polling(int* o_rc,
        int (*i_condition)(void* io_arg, int* o_satisfied),
        void* io_arg,
        SsxInterval i_timeout,
        SsxInterval i_sleep);


/// A busy-wait loop
///
/// \param[in] i_interval The interval of time to busy-wait. The actual
/// interval may be more than this if the thread is interrupted. If called
/// from a context with interrupts disabled the timing should be very precise.
void
busy_wait(SsxInterval i_interval);

#endif // __ASSEMBLER__

#endif // __POLLING_H__
