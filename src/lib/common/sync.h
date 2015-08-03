/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/common/sync.h $                                       */
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
#ifndef __SYNC_H__
#define __SYNC_H__

// $Id$

/// \file sync.h
/// \brief A library of higher-level synchronization primitives based on
/// low-level kernel services.
///
/// The APIs provided here are currently based on SSX services, but the
/// specifications should allow applications to be fairly easily ported to
/// other environments if required.
///
/// \todo Consider implementing a subset of the POSIX pthreads standards
/// instead of these non-standard synchronization primitives. Ideally all
/// synchronization primitives would be part of the SSX kernel so that they
/// would correctly handle thread suspension and deletion while pending.

#include "kernel.h"

// Error/panic codes

#define SYNC_INVALID_OBJECT         0x00896201
#define SYNC_INVALID_ARGUMENT       0x00896202
#define SYNC_BARRIER_PEND_TIMED_OUT 0x00896203
#define SYNC_BARRIER_OVERFLOW       0x00896204
#define SYNC_BARRIER_UNDERFLOW      0x00896205
#define SYNC_BARRIER_INVARIANT      0x00896206
#define SYNC_SHARED_UNDERFLOW       0x00896207

////////////////////////////////////////////////////////////////////////////
// Barrier
////////////////////////////////////////////////////////////////////////////

typedef void (*BarrierCallback)(void *);

/// A thread barrier object 
///
/// A Barrier allows multiple threads to pend until a group of threads are all
/// pending at the barrier.  Once all threads are pending at the barrier, all
/// threads are released again for execution.  The barrier guarantees that
/// once released, lower-priority threads will have the chance to execute
/// before the barrier condition is satisfied again.  Thus the Barrier can be
/// used as a form of fair scheduling for a group of threads that execute in a
/// loop of doing work followed by pending at the barrier.
///
/// Although the barrier guarantees fairness, it can not by itself guarantee
/// progress. If several threads in a barrier group are
/// pending at the barrier while other threads in the group are blocked away
/// from the barrier, the threads at the barrier should be running (if allowed
/// by the priority mappings etc.).  The only way to guarantee constant
/// progress by any unblocked thread in a group is to created another,
/// 'watchdog' thread that has lower priority than the other threads in the
/// group.  The watchdog thread only executes when all threads in the group
/// are blocked, and ensures progress.  This implementation provides a
/// watchdog thread routine as barrier_watchdog().
///
/// Normally the threads in a barrier group will be assigned consecutive
/// priorities - otherwise various forms of priority inversion can arise.  The
/// watchdog thread, if any, will normally be assigned the priority
/// immediately lower than the lowest priority thread in the group.
///
/// The Barrier object supports an optional callback function.  The callback
/// (with the customary single (void *) parameter) is made when the barrier
/// condition is met, in the thread context of the first (highest priority)
/// thread to exit the barrier. The callback is made inside the barrier_pend()
/// call, but outside of a critical section. The specification of the callback
/// is not part of the barrier_create() call, but is provided later by the
/// barrier_callback_set() call. A NULL (0) callback (the default) is ignored.
///
/// NB: All barrier APIs (other than barrier_create()) must be made from
/// thread mode - they will fail if called from interrupt handlers or before
/// threads have started.

typedef struct Barrier {

    /// Semaphore array; see \a entry_sem and \a exit_sem;
    KERN_SEMAPHORE sem[2];

    /// The entry semaphore.
    ///
    /// Threads pending at the barrier initially block here.  Once all of the
    /// threads in the group are pending here, the entry and exit semaphores
    /// are swapped and threads are released (in priority order) from the new
    /// exit semaphore.
    volatile KERN_SEMAPHORE *entry_sem;

    /// The exit semaphore.
    volatile KERN_SEMAPHORE *exit_sem;

    /// The current count of threads pending at \a entry_sem.
    volatile KERN_SEMAPHORE_COUNT entry_count;

    /// The current count of threads pending at \a exit_sem.
    volatile KERN_SEMAPHORE_COUNT exit_count;

    /// The target number of threads required to release the barrier
    volatile KERN_SEMAPHORE_COUNT target_count;

    /// A flag - Is the watchdog thread pending at the barrier?
    volatile int watchdog_pending;

    /// Statistics - The number of times the watchdog has entered the barrier.
    volatile uint32_t watchdog_entries;
    
    /// The barrier condition callback function 
    volatile BarrierCallback callback;

    /// The argument of the callback function
    volatile void *arg;

    /// This flag is set to tell the first thread to exit the barrier to
    /// execute the callback (if any).
    volatile int run_callback;

} Barrier;

int
barrier_create(Barrier *barrier, KERN_SEMAPHORE_COUNT count);

int
barrier_callback_set(Barrier *barrier, 
		     BarrierCallback callback,
		     void *arg);

int
barrier_pend(Barrier *barrier);

void
barrier_watchdog(void *arg);


#endif // __SYNC_H__








    
 
