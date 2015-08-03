/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/polling.c $                                 */
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
// $Id: polling.c,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/polling.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file polling.c
/// \brief Library APIs for polling

#include "polling.h"

int
polling(int* o_rc,
        int (*i_condition)(void* io_arg, int* o_satisfied),
        void* io_arg,
        SsxInterval i_timeout,
        SsxInterval i_sleep)
{
    SsxTimebase start;
    int rc, pollRc, timed_out, done;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((i_condition == 0), POLLING_ERROR);
    }
    
    start = ssx_timebase_get();
    timed_out = 0;

    do {
        pollRc = i_condition(io_arg, &done);
        if (pollRc) {
            rc = POLLING_CONDITION;
            break;
        }
        if (done) {
            rc = 0;
            break;
        }
        if (timed_out) {
            rc = POLLING_TIMEOUT;
            break;
        }
        if (i_sleep != 0) {
            rc = ssx_sleep(i_sleep);
            if (rc) {
                break;
            }
        }
        timed_out = 
            ((i_timeout != SSX_WAIT_FOREVER) &&
             ((ssx_timebase_get() - start) >= i_timeout));

    } while (1);

    if (o_rc) {
        *o_rc = pollRc;
    }

    return rc;
}


void
busy_wait(SsxInterval i_interval)
{
    SsxTimebase start;

    start = ssx_timebase_get();
    while ((ssx_timebase_get() - start) < i_interval);
}

