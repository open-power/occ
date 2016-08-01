/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/kernel/pk_core.c $                                 */
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

/// \file pk_core.c
/// \brief Core routines for the PK kernel.
///
///  The entry points in this file are routines that are expected to be needed
///  at runtime by all PK applications.  This file also serves as a place for
///  kernel global variables to be realized.

#define __PK_CORE_C__

#include "pk.h"

#if !PK_TIMER_SUPPORT

/// If there is no timer support, then any call of the timer interrupt handler
/// is considered a fatal error.

void
__pk_timer_handler()
{
    PK_PANIC(PK_NO_TIMER_SUPPORT);
}

#endif  /* PK_TIMER_SUPPORT */


/// Initialize an PkDeque sentinel node
///
/// \param deque The sentinel node of the deque
///
/// PK has no way of knowing whether the \a deque is currently in use, so
/// this API must only be called on unitialized or otherwise unused sentinel
/// nodes.
///
/// \retval 0 success
///
/// \retval -PK_INVALID_DEQUE_SENTINEL The \a deque pointer was null

int
pk_deque_sentinel_create(PkDeque *deque)
{
    if (PK_ERROR_CHECK_API) {
        PK_ERROR_IF(deque == 0, PK_INVALID_DEQUE_SENTINEL);
    }

    deque->next = deque->previous = deque;
    return 0;
}


/// Initialize an PkDeque element
///
/// \param element Typically the PkDeque object of a queable structure
///
/// PK has no way of knowing whether the \a element is currently in use, so
/// this API must only be called on unitialized or otherwise unused deque
/// elements. 
///
/// \retval 0 success
///
/// \retval -PK_INVALID_DEQUE_ELEMENT The \a element pointer was null

int
pk_deque_element_create(PkDeque *element)
{
    if (PK_ERROR_CHECK_API) {
        PK_ERROR_IF(element == 0, PK_INVALID_DEQUE_ELEMENT);
    }

    element->next = 0;
    return 0;
}

#undef __PK_CORE_C__
