/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/kernel/pk_bh_core.c $                              */
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
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2015
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pk_bh_core.c
/// \brief PK bottom half APIs
///
///  The entry points in this file are considered 'core' routines that will
///  always be present at runtime in any PK application.

#include "pk.h"

/// Statically initialize the bottom half queue
PK_DEQUE_SENTINEL_STATIC_CREATE(_pk_bh_queue);

void _pk_process_bh(void)
{
    PkBottomHalf *bh;
    while((bh = (PkBottomHalf*)pk_deque_pop_front(&_pk_bh_queue)) != 0)
    {
        bh->bh_handler(bh->arg);
    }
    return;
}


#undef __PK_THREAD_CORE_C__
