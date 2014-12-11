// $Id: time.c,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/time.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file time.c
/// \brief Implementation of functions from <time.h>
///
/// The following standard APIs are currently supported:
///
/// - clock_gettime() with the single clock id CLOCK_REALTIME
///
/// Since SSX does not currently have any per-thread CPU time statistics, we
/// can not implement process-CPU-time-based APIs like the simple clock() or
/// clock_gettime() with CLOCK_REALTIME.

#include "ssx.h"
#include <errno.h>
#include <time.h>

/// Get time from a timer
///
/// \param clock_id This must be the constant CLOCK_REALTIME defined in
/// <time.h>. 
///
/// \param tp A pointer to a struct timespec populated by this API.
///
/// Although the Posix standard requires that clock_gettime() support
/// CLOCK_REALTIME, CLOCK_REALTIME measures standard Unix time (seconds since
/// the epoch) which is not available to SSX.  SSX currently only supports
/// CLOCK_MONOTONIC, which is derived from the SSX timebase.
///
/// \returns Either 0 for success, or -EINVAL in the event of an invalid
/// argument (unrecognized \a clock_id, NULL \a tp pointer).

int
clock_gettime(clockid_t clock_id, struct timespec* tp)
{
    int rc;
    SsxTimebase now;

    if ((clock_id != CLOCK_MONOTONIC) || (tp == 0)) {
        rc = -EINVAL;
    } else {

        now = ssx_timebase_get();
        tp->tv_sec = now / SSX_TIMEBASE_FREQUENCY_HZ;
        tp->tv_nsec = 
            ((now % SSX_TIMEBASE_FREQUENCY_HZ) * 1000000000) / 
            SSX_TIMEBASE_FREQUENCY_HZ;
        rc = 0;
    }

    return rc;
}
