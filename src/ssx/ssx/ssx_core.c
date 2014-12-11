// $Id: ssx_core.c,v 1.2 2014/02/03 01:30:44 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ssx/ssx_core.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ssx_core.c
/// \brief Core routines for the SSX kernel.
///
///  The entry points in this file are routines that are expected to be needed
///  at runtime by all SSX applications.  This file also serves as a place for
///  kernel global variables to be realized.

#define __SSX_CORE_C__

#include "ssx.h"

#if !SSX_TIMER_SUPPORT

/// If there is no timer support, then any call of the timer interrupt handler
/// is considered a fatal error.

void
__ssx_timer_handler()
{
    SSX_PANIC(SSX_NO_TIMER_SUPPORT);
}

#endif  /* SSX_TIMER_SUPPORT */


/// Initialize an SsxDeque sentinel node
///
/// \param deque The sentinel node of the deque
///
/// SSX has no way of knowing whether the \a deque is currently in use, so
/// this API must only be called on unitialized or otherwise unused sentinel
/// nodes.
///
/// \retval 0 success
///
/// \retval -SSX_INVALID_DEQUE_SENTINEL The \a deque pointer was null

int
ssx_deque_sentinel_create(SsxDeque *deque)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(deque == 0, SSX_INVALID_DEQUE_SENTINEL);
    }

    deque->next = deque->previous = deque;
    return 0;
}


/// Initialize an SsxDeque element
///
/// \param element Typically the SsxDeque object of a queable structure
///
/// SSX has no way of knowing whether the \a element is currently in use, so
/// this API must only be called on unitialized or otherwise unused deque
/// elements. 
///
/// \retval 0 success
///
/// \retval -SSX_INVALID_DEQUE_ELEMENT The \a element pointer was null

int
ssx_deque_element_create(SsxDeque *element)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(element == 0, SSX_INVALID_DEQUE_ELEMENT);
    }

    element->next = 0;
    return 0;
}

#undef __SSX_CORE_C__
