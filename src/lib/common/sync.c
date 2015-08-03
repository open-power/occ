/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/common/sync.c $                                       */
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

/// \file sync.c
/// \brief A library of higher-level synchronization primitives based on
/// low-level kernel services.
///
/// The APIs provided here are currently based on kernel services, but the
/// specifications should allow applications to be ported to other
/// environments if required.  Note that like  kernel services, data
/// structures manipulated by this code are protected in KERN_NONCRITICAL
/// critical sections. 

#include "kernel.h"
#include "sync.h"

/// Create a Barrier
///
/// \param barrier A pointer to an uninitialized or currently unused Barrier
///
/// \param count The number of threads required to pend at the barrier before
/// all threads are released again for execution.  Note that \a count values
/// of 0 and 1 are treated as equivalent - threads will not pend at all in
/// these cases.  Also note that if a watchdog thread is being used, the
/// watchdog thread should not be included in the count.
///
/// \retval 0 Success
///
/// \retval -SYNC_INVALID_OBJECT The \a barrier is NULL (0)

int
barrier_create(Barrier *barrier, KERN_SEMAPHORE_COUNT count)
{
    if (KERN_ERROR_CHECK_API) {
	KERN_ERROR_IF(barrier == 0, SYNC_INVALID_OBJECT);
    }

    barrier->entry_sem = &(barrier->sem[0]);
    barrier->exit_sem = &(barrier->sem[1]);

    KERN_SEMAPHORE_CREATE((KERN_SEMAPHORE *)(barrier->entry_sem), 0, 0);
    KERN_SEMAPHORE_CREATE((KERN_SEMAPHORE *)(barrier->exit_sem), 0, 0);

    barrier->entry_count = 0;
    barrier->exit_count = 0;
    barrier->target_count = count;

    barrier->watchdog_pending = 0;
    barrier->watchdog_entries = 0;

    barrier->callback = 0;
    barrier->arg = 0;
    barrier->run_callback = 0;

    return 0;
}


/// Install a barrier callback
///
/// \param barrier A pointer to an initialized Barrier object
///
/// \param callback A function taking a single (void *) argument, to be
/// executed in the context of the first thread to exit the barrier when the
/// barrier condition is met.
///
/// \param arg The argument of the \a callback.
///
/// The Barrier object supports an optional callback function.  The callback
/// (with the customary single (void *) parameter) is made when the barrier
/// condition is met, in the thread context of the first (highest priority)
/// thread to exit the barrier. The callback is made inside the barrier_pend()
/// call, but outside of a critical section. The specification of the callback
/// is not part of the barrier_create() call, but is provided later by this
/// API. Setting a NULL (0) callback disables the callback mechanism.
///
/// \retval 0 Success
///
/// \retval -SYNC_INVALID_OBJECT The \a barrier is NULL (0)

int
barrier_callback_set(Barrier *barrier, 
		     BarrierCallback callback,
		     void *arg)
{
    KERN_MACHINE_CONTEXT ctx;

    if (KERN_ERROR_CHECK_API) {
	KERN_ERROR_IF(barrier == 0, SYNC_INVALID_OBJECT);
    }

    KERN_CRITICAL_SECTION_ENTER(KERN_NONCRITICAL, &ctx);

    barrier->callback = callback;
    barrier->arg = arg;

    KERN_CRITICAL_SECTION_EXIT(&ctx);

    return 0;
}
    

static int
_barrier_pend(Barrier *barrier, int watchdog)
{
    KERN_MACHINE_CONTEXT ctx;
    int rc = 0;
    KERN_SEMAPHORE *temp_sem;
    BarrierCallback callback = 0; /* Make GCC Happy */
    void *arg = 0;		/* Make GCC Happy */
    int run_callback;

    if (KERN_ERROR_CHECK_API) {
	KERN_ERROR_IF(barrier == 0, SYNC_INVALID_OBJECT);
    }

    KERN_CRITICAL_SECTION_ENTER(KERN_NONCRITICAL, &ctx);

    // A normal thread will pend at the entry unless 1) the thread satisfies
    // the barrier condition, or 2) a watchdog thread is pending here.  A
    // watchdog thread only pends if no other threads are pending.

    if (watchdog) {
	barrier->watchdog_entries++;
    }

    barrier->entry_count++;
    if (!barrier->watchdog_pending &&
	((watchdog && (barrier->entry_count == 1)) ||
	 (!watchdog && (barrier->entry_count < barrier->target_count)))) {

	if (watchdog) {
	    barrier->watchdog_pending = 1;
	}

	// The thread must pend here

	rc = KERN_SEMAPHORE_PEND((KERN_SEMAPHORE *)(barrier->entry_sem), 
				KERN_WAIT_FOREVER);
	if (rc) {
	    goto exit_critical;
	}

    } else {

	// The barrier condition is met - or the watchdog thread is blocked
	// here.  The entry and exit semaphores and counts are swapped. The
	// callback is marked to be called.

	// If the barrier is used incorrectly, or threads are deleted without
	// adjusting the barrier target count then the following condition
	// could become true, which could lead to bad behavior.

	if (barrier->exit_count != 0) {
	    KERN_PANIC(SYNC_BARRIER_INVARIANT);
	}

	barrier->entry_count--;	// Undo preincrement above
	barrier->watchdog_pending = 0;

	temp_sem = (KERN_SEMAPHORE *)(barrier->exit_sem);
	barrier->exit_sem = barrier->entry_sem;
	barrier->entry_sem = temp_sem;

	barrier->exit_count = barrier->entry_count;
	barrier->entry_count = 0;

	barrier->run_callback = 1;
    }

    // This thread either continues to run or just woke up after having
    // blocked at the barrier.  The current thread makes the next thread (if
    // any) runnable as well.  Normally the current thread will be of a higher
    // priority than any blocked threads, so no context switch will occur. The
    // thread that satisfies the barrier condition \e will cause a context
    // switch here, unless it just happens to be the highest priority thread
    // in the barrier group.

    if (barrier->exit_count != 0) {
	barrier->exit_count--;
	rc = KERN_SEMAPHORE_POST((KERN_SEMAPHORE *)(barrier->exit_sem));
    }
    
exit_critical:

    if (rc) {
	KERN_CRITICAL_SECTION_EXIT(&ctx);
	return rc;
    }

    // In the case of a satisfied barrier condition, the first thread to exit
    // the critical section will be the higest priority thread blocked at the
    // barrier.  This thread is tasked with executing the callback, outside of
    // the critical section.

    run_callback = barrier->run_callback;
    barrier->run_callback = 0;
    if (run_callback) {
	callback = barrier->callback;
	arg = (void *)(barrier->arg); /* Cast away 'volatile' */
    }

    KERN_CRITICAL_SECTION_EXIT(&ctx);

    if (run_callback && callback) {
	callback(arg);
    }

    return 0;
}


/// Pend at a Barrier
///
/// \param barrier An initialized barrier object
///
/// A thread will pend at a barrier until \a count number of threads (supplied
/// in the call of barrier_create()) are pending.  If \a count is 0 or 1, the
/// API always returns immediately. 
///
/// If barrier watchdog thread is being used (correctly), then the watchdog
/// will cause thread pending on the barrier to be released whenever all
/// threads in the group are blocked, regardless of whether some of the
/// group's threads are blocked elesewhere.
///
/// \retval 0 Success
///
/// \retval -SYNC_INVALID_OBJECT The \a barrier is NULL (0)
///
/// Other errors may be returned by the embedded call of ssx_semephore_pend().
/// In particular this API will fail if called outside of a thread context
/// since it requires blocking indefinitely on a semaphore.
///
/// \bug The semaphore should be able to provide the number of pending threads
/// - which we should really be using here instead of the barrier counts.  The
/// current implementation can produce some strange behavior if threads are
/// deleted.

int
barrier_pend(Barrier *barrier)
{
    return _barrier_pend(barrier, 0);
}


/// Barrier watchdog thread
///
/// \param arg A pointer to the Barrier object this thread should manage.
///
/// A barrier_watchdog() thread is attached to a Barrier object, and forces
/// the barrier condition to be met whenever it runs.  This thread is designed
/// to be mapped at a priority immediately below the priorities of a group of
/// threads that pend on the barrier.  
///
/// In this way, should every thread in the group become blocked, the watchdog
/// will allow any threads that are blocked on the barrier to run. If all the
/// threads are blocked elsewhere, then the watchdog blocks on the barrier,
/// and as soon as any thread pends again at the barrier the thread remains
/// runnable and the watchdog becomes runnable.
///
/// The barrier_watchdog() thread is not required.  Without the watchdog,
/// otherwise runnable threads in the barrier group will remain blocked on the
/// barrier as long as any of their cohorts remain blocked elsewhere.

void
barrier_watchdog(void *arg)
{
    do {
	_barrier_pend((Barrier *)arg, 1);
    } while (1);
}
