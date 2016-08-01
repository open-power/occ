/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/kernel/pk_semaphore_init.c $                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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

/// \file pk_semaphore_init.c
/// \brief PK semaphore API initialization routines
///
/// The entry points in this file are routines that are typically used during
/// initialization, and their code space could be deallocated and recovered if
/// no longer needed by the application after initialization.

#include "pk.h"

/// Create (initialize) a semaphore
///
/// \param semaphore A pointer to an PkSemaphore structure to initialize
///
/// \param initial_count The initial count of the semaphore
///
/// \param max_count The maximum count allowed in the semaphore, for error
/// checking 
///
/// Semaphores are created (initialized) by a call of \c
/// pk_semaphore_create(), using an application-provided instance of an \c
/// PkSemaphore structure.  This structure \e is the semaphore, so the
/// application must never modify the structure if the semaphore is in use.
/// PK has no way to know if an \c PkSemaphore structure provided to
/// \c pk_semaphore_create() is safe to use as a semaphore, and will silently
/// modify whatever memory is provided.
///
/// PK provides two simple overflow semantics based on the value of max_count
/// in the call of \c pk_semaphore_create().
///
/// If \a max_count = 0, then posting to the semaphore first increments the
/// internal count by 1.  Overflows are ignored and will wrap the internal
/// count through 0.
///
/// If \a max_count != 0, then posting to the semaphore first increments the
/// internal count by 1, wrapping through 0 in the event of overflow. If the
/// resulting count is greater than max_count, \c pk_semaphore_post() will
/// return the error \c -PK_SEMAPHORE_POST_OVERFLOW to the caller.
///
/// In most applications it is probably best to use the \a max_count != 0
/// semantics to trap programming errors, unless there is a specific
/// application where overflow is expected and ignorable. As a fine point of
/// the specification, a \a max_count of 0 is equivalent to a max_count of
/// 0xFFFFFFFF.
///
/// Return values other then PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion
///
/// \retval -PK_INVALID_SEMAPHORE_AT_CREATE The \a semaphore is a null (0) 
/// pointer.
/// 
/// \retval -PK_INVALID_ARGUMENT_SEMAPHORE The \a max_count is non-zero 
/// and less than the \a initial_count.

int
pk_semaphore_create(PkSemaphore      *semaphore,
                     PkSemaphoreCount initial_count,
                     PkSemaphoreCount max_count)
{
    if (PK_ERROR_CHECK_API) {
        PK_ERROR_IF(semaphore == 0, PK_INVALID_SEMAPHORE_AT_CREATE);
        PK_ERROR_IF((max_count != 0) && (initial_count > max_count),
                     PK_INVALID_ARGUMENT_SEMAPHORE);
    }

    __pk_thread_queue_clear(&(semaphore->pending_threads));
    semaphore->count = initial_count;
    semaphore->max_count = max_count;

    return PK_OK;
}

                             



