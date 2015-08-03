/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/periodic_semaphore.h $                      */
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
#ifndef __PERIODIC_SEMAPHORE_H__
#define __PERIODIC_SEMAPHORE_H__

// $Id$

/// \file perodic_semaphore.h
/// \brief Periodic semphores
///
/// The PeriodicSemphore is a simple abstraction introduced to simplify coding
/// peridic threads. A periodic thread creates the PeriodicSemaphore after
/// thread initialization, but prior to the entry to the periodic infinite
/// loop. This creates the periodicSemaphore with a count of 0. Once thread
/// processing is finished, the thread pends on the PeriodicSemaphore. A
/// periodic timer posts to the PeriodicSemaphore on a fixed, absolute period
/// to reschedule the thread.
///
/// If the thread pends on the PeriodicSemaphore and the timer has already
/// posted to the semaphore, the call of periodic_semaphore_pend() clears the
/// semaphore and terminates immediately with the return code
/// -PERIODIC_SEMAPHORE_OVERRUN, indicating that the thread has overrun its
/// period. The thread can choose the appropriate action upon obtaining this
/// return code.
///
/// The PeriodicSemaphore can also be cancelled, which simply cancels the
/// periodic timer posting to the semaphore. If the thread needs to
/// re-initialize the PeriodicSemaphore for any reason (e.g, to resynchronize
/// after an overrun) it must be cancelled first.

// Error/Panic codes

#define PERIODIC_SEMAPHORE_OVERRUN 0x0077e601


/// A periodic semaphore

typedef struct {

    /// The semaphore
    SsxSemaphore sem;

    /// The timer
    SsxTimer timer;

} PeriodicSemaphore;


/// Create (initialize) a PeriodicSemaphore
///
/// \param sem A pointer to an uninitialized or inactive
/// PeriodicSemaphore.
///
/// \param period The semaphore period
///
/// This API creates the embedded semaphore as a binary semaphore with an
/// initial value of 0, and schedules a periodic timer to post to the
/// semaphore.
///
/// \retval 0 Success
///
/// \retval -SSX_INVALID_OBJECT The \a sem was NULL (0) or otherwise invalid.
///
/// Other return codes are possible from the embedded calls of SSX APIs.
int
periodic_semaphore_create(PeriodicSemaphore* sem, SsxInterval period);


/// Pend on a PeriodicSemaphore
///
/// \param sem A pointer to an initialized PeriodicSemaphore
///
/// Pend on a PeriodicSemaphore.  It is considered a non-fatal error if the
/// semaphore has a non-0 count as this may indicate that a periodic thread
/// has missed a deadline.
///
/// Return values other than SSX_OK (0) are not necessarily errors; see \ref
/// ssx_errors 
///
/// The following return codes are non-error codes:
///
/// \retval 0 Success. In particular, the semaphore count was 0 at entry.
///
/// \retval -PERIODIC_SEMAPHORE_OVERRUN This return code indicates that the
/// semaphore count was 1 at entry. This code is always returned (never causes
/// a panic). 
///
/// The following return codes are  error codes:
///
/// \retval -SSX_IVALID_OBJECT The \a sem was NULL () or otherwise invalid at
/// entry.
///
/// Other error return codes are possible from embedded calls of SSX APIs.
int
periodic_semaphore_pend(PeriodicSemaphore* sem);


/// Cancel a periodic semaphore
///
/// \param sem A pointer to an initialized PeriodicSemaphore
///
/// Cancel the PeriodicSemaphore timeout. This is a required step if the
/// PeriodicSemaphore is to be reinitialized. This is also required if the
/// PeriodicSemaphore is created on the thread stack and the thread
/// terminates.  PeriodicSemaphore can be canceled at any time.  It is never
/// an error to call periodic_semaphore_cancel() on a PeriodicSemaphore object
/// after it is created.
///
/// Return values other than SSX_OK (0) are not necessarily errors; see \ref
/// ssx_errors 
///
/// The following return codes are non-error codes:
///
/// \retval 0 Successful completion
///
/// \retval -SSX_TIMER_NOT_ACTIVE The embedded timer is not currently
/// scheduled, meaning that the PeriodicSemaphore was previosly
/// cancelled.
///
/// The following return codes are  error codes:
///
/// \retval -SSX_IVALID_OBJECT The \a sem was NULL () or otherwise invalid at
/// entry.
///
/// Other error return codes are possible from embedded calls of SSX APIs.
int
periodic_semaphore_cancel(PeriodicSemaphore* sem);


#endif // __PERIODIC_SEMAPHORE_H__
