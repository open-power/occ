/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/mutex.c $                                   */
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
// $Id$

/// \file mutex.c
/// \brief A ThreadX-style mutual exclusion object

#include "mutex.h"

int
mutex_create(Mutex* i_mutex)
{
    int rc;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(i_mutex == 0, SSX_INVALID_OBJECT);
    }

    rc = ssx_semaphore_create(&(i_mutex->sem), 1, 1);
    i_mutex->thread = 0;
    i_mutex->count = 0;

    return rc;
}


// If the current thread owns the Mutex we simply increment the count,
// otherwise pend for the semaphore.
//
// Note: It's possible this doesn't need to be done in a critical section. The
// fact that ssx_semaphore_pend() is atomic may be sufficient since it locks
// the Mutex.

int
mutex_pend(Mutex* i_mutex, SsxInterval i_timeout)
{
    int rc;
    SsxMachineContext ctx;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(i_mutex == 0, SSX_INVALID_OBJECT);
        SSX_ERROR_UNLESS_THREAD_CONTEXT();
    }

    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    if (i_mutex->thread == ssx_current()) {

        i_mutex->count++;
        if (i_mutex->count == 0) {
            rc = MUTEX_OVERFLOW;
        } else {
            rc = 0;
        }

    } else {

        rc = ssx_semaphore_pend(&(i_mutex->sem), i_timeout);
        if (rc == 0) {
            i_mutex->thread = ssx_current();
            i_mutex->count = 1;
        }
    }

    ssx_critical_section_exit(&ctx);

    return rc;
}


// If the current thread owns the Mutex we decrement the count and free the
// object when the count goes to 0.
//
// Note: It's possible this doesn't need to be done in a critical section. The
// fact that ssx_semaphore_pend() is atomic may be sufficient since it locks
// the Mutex.

int
mutex_post(Mutex* i_mutex)
{
    int rc;
    SsxMachineContext ctx;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(i_mutex == 0, SSX_INVALID_OBJECT);
        SSX_ERROR_UNLESS_THREAD_CONTEXT();
    }

    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    SSX_ERROR_IF(i_mutex->thread != ssx_current(), MUTEX_NOT_OWNED);

    if (--i_mutex->count == 0) {
        i_mutex->thread = 0;
        rc = ssx_semaphore_post(&(i_mutex->sem));
    } else {
        rc = 0;
    }

    ssx_critical_section_exit(&ctx);

    return rc;
}

    

    
