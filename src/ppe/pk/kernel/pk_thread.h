/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/kernel/pk_thread.h $                               */
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
#ifndef __PK_THREAD_H__
#define __PK_THREAD_H__

/// \file pk_thread.h
/// \brief Contains private declarations and definitions needed for threads
///

void
__pk_thread_map(PkThread* thread);

void
__pk_thread_unmap(PkThread *thread);


// Interrupts must be disabled at entry.

static inline int
__pk_thread_is_active(PkThread *thread)
{
    return ((thread->state != PK_THREAD_STATE_COMPLETED) &&
            (thread->state != PK_THREAD_STATE_DELETED));
}


// Interrupts must be disabled at entry.

static inline int
__pk_thread_is_mapped(PkThread *thread)
{
    return (thread->state == PK_THREAD_STATE_MAPPED);
}


// Interrupts must be disabled at entry. This is only called on mapped threads.

static inline int
__pk_thread_is_runnable(PkThread *thread)
{
    return __pk_thread_queue_member(&__pk_run_queue, thread->priority);
}


// Interrupts must be disabled at entry.

static inline PkThread*
__pk_thread_at_priority(PkThreadPriority priority)
{
    return (PkThread*)__pk_priority_map[priority];
}

#endif /* __PK_THREAD_H__ */
