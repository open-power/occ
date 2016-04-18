/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ssx/ssx_semaphore_init.c $                            */
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
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ssx_semaphore_init.c
/// \brief SSX semaphore API initialization routines
///
/// The entry points in this file are routines that are typically used during
/// initialization, and their code space could be deallocated and recovered if
/// no longer needed by the application after initialization.

#include "ssx.h"

/// Create (initialize) a semaphore
///
/// \param semaphore A pointer to an SsxSemaphore structure to initialize
///
/// \param initial_count The initial count of the semaphore
///
/// \param max_count The maximum count allowed in the semaphore, for error
/// checking
///
/// Semaphores are created (initialized) by a call of \c
/// ssx_semaphore_create(), using an application-provided instance of an \c
/// SsxSemaphore structure.  This structure \e is the semaphore, so the
/// application must never modify the structure if the semaphore is in use.
/// SSX has no way to know if an \c SsxSemaphore structure provided to
/// \c ssx_semaphore_create() is safe to use as a semaphore, and will silently
/// modify whatever memory is provided.
///
/// SSX provides two simple overflow semantics based on the value of max_count
/// in the call of \c ssx_semaphore_create().
///
/// If \a max_count = 0, then posting to the semaphore first increments the
/// internal count by 1.  Overflows are ignored and will wrap the internal
/// count through 0.
///
/// If \a max_count != 0, then posting to the semaphore first increments the
/// internal count by 1, wrapping through 0 in the event of overflow. If the
/// resulting count is greater than max_count, \c ssx_semaphore_post() will
/// return the error \c -SSX_SEMAPHORE_POST_OVERFLOW to the caller.
///
/// In most applications it is probably best to use the \a max_count != 0
/// semantics to trap programming errors, unless there is a specific
/// application where overflow is expected and ignorable. As a fine point of
/// the specification, a \a max_count of 0 is equivalent to a max_count of
/// 0xFFFFFFFF.
///
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_SEMAPHORE_AT_CREATE The \a semaphore is a null (0)
/// pointer.
///
/// \retval -SSX_INVALID_ARGUMENT_SEMAPHORE The \a max_count is non-zero
/// and less than the \a initial_count.

int
ssx_semaphore_create(SsxSemaphore*      semaphore,
                     SsxSemaphoreCount initial_count,
                     SsxSemaphoreCount max_count)
{
    if (SSX_ERROR_CHECK_API)
    {
        SSX_ERROR_IF(semaphore == 0, SSX_INVALID_SEMAPHORE_AT_CREATE);
        SSX_ERROR_IF((max_count != 0) && (initial_count > max_count),
                     SSX_INVALID_ARGUMENT_SEMAPHORE);
    }

    __ssx_thread_queue_clear(&(semaphore->pending_threads));
    semaphore->count = initial_count;
    semaphore->max_count = max_count;

    return SSX_OK;
}





