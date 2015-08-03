/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/mutex.h $                                   */
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
#ifndef __MUTEX_H__
#define __MUTEX_H__

// $Id$

/// \file mutex.h
/// \brief A ThreadX-style mutual exclusion object
///
/// A Mutex is a binary semaphore with the concept of thread ownership.  A
/// thread first obtains the Mutex using the mutex_pend() API, which may block
/// if the Mutex is currently owned by another thread.  Once a thread owns a
/// Mutex, subsequent calls of mutex_pend() by the same thread simply
/// increment an internal counter, but do not block. Once a thread has
/// executed a matching mutex_post() call for every mutex_pend() call, the
/// Mutex is free for another thread.
///
/// This type of mutual exclusion object is useful for example to control
/// access to data structures that are manipulated by APIs with several common
/// entry points. Each call of an API in the chain will 'lock' the data
/// structure using mutex_pend()/mutex_post(). The Mutex semantics allows
/// multiple "locks" by the same thread, but requires a corresponding "unlock"
/// for every "lock".
///
/// The Mutex usage counter is a 32-bit unsigned integer. If a thread makes
/// 2^32 calls to mutex_pend() without an intervening call of mutex_post(), an
/// overflow is signalled. This error should be considered unrecoverable to
/// the application.
///
/// Like the SSX semaphore, no record is kept in the thread of which Mutex
/// objects are currently owned by the thread.  If a thread terminates or is
/// deleted while holding a Mutex it is likely that the application will
/// hang. Unlike the SSX semaphore, it is absolutely illegal to call
/// mutex_pend() and mutex_post() from interrupt contexts. It is also illegal
/// for a thread to call mutex_post() for a mutex it does not own.
///
/// Mutex objects are easily created with the static initialization macro
/// MUTEX_INITIALIZATION as in the following example.
///
/// \code 
///
/// Mutex G_mutex = MUTEX_INITIALIZATION;
///
/// \endcode
///
/// The API mutex_create() is also provided for run-time initialization.

#include "ssx.h"


// Mutex error/panic codes

#define MUTEX_OVERFLOW  0x00688901
#define MUTEX_NOT_OWNED 0x00688902


#ifndef __ASSEMBLER__

/// Static initialization of a Mutex
///
/// For a full description of the Mutex please see the documentation fof the
/// file mutex.h.
#define MUTEX_INITIALIZATION {SSX_SEMAPHORE_INITIALIZATION(1, 1), 0, 0}


/// The Mutex object

typedef struct {

    /// The binary semaphore
    SsxSemaphore sem;

    /// A pointer to the owning thread, or NULL (0)
    SsxThread* thread;

    /// The count of unmatched mutex_pend() calls made by the owning thread.
    uint32_t count;

} Mutex;


/// Create (initialize) a Mutex
///
/// \param[in] i_mutex A pointer to the Mutex object to initialize.
///
/// For a full description of the Mutex please see the documentation for the
/// file mutex.h.
///
/// \retval 0 Success
///
/// \retval 0 -SSX_INVALID_OBJECT The \a i_mutex is NULL (0).
int
mutex_create(Mutex* i_mutex);


/// Pend on a Mutex with optional timeout
///
/// \param[in] i_mutex A pointer to the Mutex
///
/// \param[in] i_timeout Either the constant SSX_WAIT_FOREVER, or a timeout
/// interval specification.
///
/// For a full description of the Mutex please see the documentation for the
/// file mutex.h.
///
/// \retval 0 Success
///
/// \retval -SSX_INVALID_OBJECT The \a i_mutex is NULL (0).
/// 
/// \retval -SSX_ILLEGAL_CONTEXT The call was not made from a thread context.
///
/// \retval -SSX_SEMEPHORE_PEND_TIMED_OUT The thread was not able to obtain
/// the Mutex before the timeout.
///
/// \retval -MUTEX_OVERFLOW The owning thread has made 2^32 unmatched calls of
/// mutex_pend(). 
int
mutex_pend(Mutex* i_mutex, SsxInterval i_timeout);


/// Post to a Mutex
///
/// \param[in] i_mutex A pointer to the Mutex
///
/// For a full description of the Mutex please see the documentation for the
/// file mutex.h.
///
/// \retval 0 Success
///
/// \retval -SSX_INVALID_OBJECT The \a i_mutex is NULL (0).
///
/// \retval -SSX_ILLEGAL_CONTEXT The call was not made from a thread context.
///
/// \retval -MUTEX_NOT_OWNED The thread calling mutex_post() does not own the
/// Mutex. 
int
mutex_post(Mutex* i_mutex);

#endif // __ASSEMBLER__

#endif // __MUTEX_H__
