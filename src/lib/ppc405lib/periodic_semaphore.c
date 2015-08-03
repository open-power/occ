/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/periodic_semaphore.c $                      */
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

/// \file perodic_semaphore.h
/// \brief Periodic semphores

#include "ssx.h"
#include "periodic_semaphore.h"

// The timer callback is created nonpreemptible, so noncritical interrupts are
// disabled at entry.

static void
_periodic_semaphore_timeout(void* arg)
{
    PeriodicSemaphore* ps;

    ps = (PeriodicSemaphore*)arg;

    if (ps->sem.count != 1) {
        ssx_semaphore_post(&(ps->sem));
    }
}


int
periodic_semaphore_create(PeriodicSemaphore* sem, SsxInterval period)
{
    int rc;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(sem == 0, SSX_INVALID_OBJECT);
    }

    do {
        rc = ssx_semaphore_create(&(sem->sem), 0, 1);
        if (rc) break;

        rc = ssx_timer_create_nonpreemptible(&(sem->timer), 
                                             _periodic_semaphore_timeout,
                                             sem);
        if (rc) break;

        rc = ssx_timer_schedule(&(sem->timer),
                                period,
                                period);
        if (rc) break;

    } while (0);

    return rc;
}


int
periodic_semaphore_pend(PeriodicSemaphore* sem)
{
    int rc;
    SsxMachineContext ctx;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(sem == 0, SSX_INVALID_OBJECT);
    }

    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    if (sem->sem.count == 0) {

        rc = ssx_semaphore_pend(&(sem->sem), SSX_WAIT_FOREVER);
        
    } else {

        sem->sem.count = 0;
        rc = -PERIODIC_SEMAPHORE_OVERRUN;
    }

    ssx_critical_section_exit(&ctx);

    return rc;
}


int
periodic_semaphore_cancel(PeriodicSemaphore* sem)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(sem == 0, SSX_INVALID_OBJECT);
    }

    return ssx_timer_cancel(&(sem->timer));
}
