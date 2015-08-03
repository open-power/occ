/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/sxlock.c $                                  */
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

/// \file sxlock.c
/// \brief API for the SharedExclusiveLock
///
/// The APIs in this file implement a shared-exclusive lock for SSX
/// applications.  This type of lock is also called a readers-writer lock. The
/// lock is implemented in terms of SSX semaphores, so its use is limited to
/// threads willing to block for access to a resource. 
///
/// The SharedExclusiveLock allows multiple threads shared access to a
/// resource, while limiting exclusive access to a single thread. There are
/// several ways that this type of lock might be specified. The specification
/// implemented here is an "exclusive-biasing" lock. As long as the lock is
/// held or requested in exclusive mode, all new shared-mode requests will
/// block, and only exclusive accesses will be allowed. If multiple threads
/// are blocked exclusive the requests are honored in priority order (as
/// the underlying implementation is an SSX semaphore). Once the exclusive
/// lock is cleared, any/all threads blocked for shared access are released
/// simultaneously (using an SSX semaphore as a thread barrier).
///
/// The lock is created (initialized) by sxlock_create(), which allows
/// specification of an initial value of the number of shared or exclusive
/// accesses outstanding.  The lock/unlock APIs are as follows:
///
/// - sxlock_lock_shared(SharedExclusiveLock* sxlock, SsxInterval timeout)
/// - sxlock_unlock_shared(SharedExclusiveLock* sxlock)
/// - sxlock_lock_exclusive(SharedExclusiveLock* sxlock, SsxInterval timeout)
/// - sxlock_unlock_exclusive(SharedExclusiveLock* sxlock)
///
/// Threads must always issue *_lock() and *_unlock() requests in matched
/// pairs in order to avoid errors and deadlock. The *_lock() APIs allow
/// specification of a timeout, which may be SSX_WAIT_FOREVER to indicate no
/// timeout. The *_lock() APIs will return the code -SXLOCK_TIMED_OUT if the
/// timeout occurs before the thread acquires the resource. If called from an
/// interrupt context then the only legal timeout specification is
/// SSX_NO_WAIT (0).  
///
/// If a *_lock() request times out then the thread \e has \e not acquired the
/// resource and \e must \e not call *_unlock(). As with semaphores there is
/// no record that a thread holds a lock, so if a thread completes or is
/// deleted while holding a lock it is likely that the application will
/// deadlock.

#include "ssx.h"
#include "sxlock.h"

/// Create (initialize) a SharedExclusiveLock
///
/// \param sxlock A pointer to the SharedExclusiveLock object to
/// initialize
///
/// \param shared The initial numbers of shared accesses
///
/// \param exclusive The initial numbers of exclusive accesses
///
/// Create (initialize) a SharedExclusiveLock and optionally specify an
/// initial state. The initial number of shared or exclusive accesses can be
/// specified, however at most one of \a nshared and \a exclusive can be
/// non-0. If \a shared or \a exclusive are non-0 then eventually a
/// thread(s) will need to issue unmatched *_unlock() call(s) to allow
/// progress for other threads requiring the resource.
///
/// \retval 0 Success
///
/// \retval SXLOCK_INVALID_OBJECT The \a sxlock parameter is NULL (0) or
/// otherwise invalid.
///
/// \retval SXLOCK_INVALID_ARGUMENT Both of the \a shared and \a exclusive
/// parameters are non-0.
///
/// \retval others sxlock_create() may also return codes from
/// ssx_semaphore_create(), which would indicate a serious bug.

int
sxlock_create(SharedExclusiveLock* sxlock, 
              SsxSemaphoreCount shared, 
              SsxSemaphoreCount exclusive)
{
    int rc;

    rc = 0;
    do {

        if (SSX_ERROR_CHECK_API) {
            SSX_ERROR_IF(sxlock == 0, 
                         SXLOCK_INVALID_OBJECT);
            SSX_ERROR_IF((shared != 0) && (exclusive != 0), 
                         SXLOCK_INVALID_ARGUMENT);
        }
        
        rc = ssx_semaphore_create(&(sxlock->shared_sem), 0, 0);
        if (rc) break;

        rc = ssx_semaphore_create(&(sxlock->exclusive_sem), 0, 0);
        if (rc) break;

        sxlock->running_shared = shared;
        sxlock->running_exclusive = exclusive;
        
    } while (0);

    return rc;
}


/// Lock a SharedExclusiveLock for shared access
///
/// \param sxlock A pointer to the SharedExclusiveLock object to lock
///
/// \param timeout The maximum amount of time to wait for access, or the
/// constant SSX_WAIT_FOREVER to wait forever.
///
/// Acquire a SharedExclusiveLock for shared access, potentially blocking
/// forever or until a specified timeout if access is not immediately
/// granted. Access will be blocked as long as one or more threads request or
/// control exclusive acesss to the resource. Once the access is granted, the
/// thread maintains shared access to the resource until a subsequent call of
/// sxlock_unlock_shared().
///
/// Return values other than SSX_OK (0) are not necessarily errors; see \ref
/// ssx_errors 
///
/// The following return codes are non-error codes:
///
/// \retval 0 Successful completion
///
/// \retval -SXLOCK_TIMED_OUT Shared access was not acquired before the
/// timeout expired.
///
/// The following return codes are error codes:
///
/// \retval -SXLOCK_INVALID_OBJECT The \a sxlock parameter was NULL (0) or
/// otherwise invalid.
///
/// \retval -SSX_ILLEGAL_CONTEXT The API was called from a critical
/// interrupt context.
/// 
/// \retval -SSX_SEMAPHORE_PEND_WOULD_BLOCK The call was made from an
/// interrupt context (or before threads have been started), shared access was
/// not immediately available and a non-zero timeout was specified.
///
/// \retval others This API may also return codes from SSX semaphore APIs,
/// which should be considered as non-recoverable errors.


int
sxlock_lock_shared(SharedExclusiveLock* sxlock, SsxInterval timeout)
{
    SsxMachineContext ctx;
    int rc, pending_exclusive;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(sxlock == 0, SXLOCK_INVALID_OBJECT);
    }

    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    rc = 0;
    do {

        // NB: This is the only way to correctly compute the number of threads
        // pending exclusive, given that threads could be removed from the
        // exclusive_sem by timeout.

        rc = ssx_semaphore_info_get(&(sxlock->exclusive_sem),
                                    0, &pending_exclusive);
        if (rc) break;

        if ((sxlock->running_exclusive == 0) && (pending_exclusive == 0)) {

            // If no other thread has or is requesting exclusive access, the
            // current thread gets immediate access.

            sxlock->running_shared++;

        } else {

            // If threads are running or pending exclusive, this thread must
            // pend shared. The thread will be unblocked by an exclusive
            // unlock, which is responsible for adjusting
            // sxlock->running_shared in this case.
        
            rc = ssx_semaphore_pend(&(sxlock->shared_sem), timeout);
            if (rc == -SSX_SEMAPHORE_PEND_TIMED_OUT) {
                rc = -SXLOCK_TIMED_OUT;
            } 
        }

    } while (0);

    ssx_critical_section_exit(&ctx);

    return rc;
}


/// Relase a SharedExclusiveLock from shared access
///
/// \param sxlock A pointer to the SharedExclusiveLock object to unlock
///
/// Release a SharedExclusiveLock from shared access, signalling that the
/// thread no longer requires or expects shared access to the resource.  It is
/// an error for a thread to use this API if it has not previously locked
/// shared access by a call of sxlock_pend_shared() (or the thread is
/// unlocking a lock initialized in the shared-locked state).
///
/// Return values other than SSX_OK (0) are errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_ILLEGAL_CONTEXT The API was called from a critical interrupt
/// context.
///
/// \retval -SXLOCK_INVALID_OBJECT The \a sxlock parameter was NULL (0) or
/// otherwise invalid.
///
/// \retval -SXLOCK_SHARED_UNDERFLOW There was apparently no matched call of
/// sxlock_lock_shared() prior to this call.
///
/// \retval others This API may also return codes from SSX semaphore APIs,
/// which should be considered as non-recoverable errors.

int
sxlock_unlock_shared(SharedExclusiveLock* sxlock)
{
    SsxMachineContext ctx;
    int rc, pending_exclusive;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF_CRITICAL_INTERRUPT_CONTEXT();
        SSX_ERROR_IF(sxlock == 0, SXLOCK_INVALID_OBJECT);
        SSX_ERROR_IF(sxlock->running_shared == 0, SXLOCK_SHARED_UNDERFLOW);
    }

    if (SSX_ERROR_CHECK_KERNEL) {
        SSX_PANIC_IF(sxlock->running_exclusive != 0, 
                     SXLOCK_SHARED_EXCLUSIVE_INVARIANT);
    }

    rc = 0;
    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    do {

        // If this is the last shared thread running, and a thread wants
        // exclusive access, grant it. Note that there may be shared requests
        // pending on the shared_sem but we always give preference to
        // exclusive requests.

        sxlock->running_shared--;
        if (sxlock->running_shared == 0) {

            // Wake any single thread pending exclusive

            rc = ssx_semaphore_info_get(&(sxlock->exclusive_sem),
                                        0, &pending_exclusive);
            if (rc) break;

            if (pending_exclusive != 0) {
                
                sxlock->running_exclusive = 1;
                rc = ssx_semaphore_post(&(sxlock->exclusive_sem));
                if (rc) break;
            }
        }

    } while(0);

    ssx_critical_section_exit(&ctx);
    
    return rc;
}


/// Lock a SharedExclusiveLock for exclusive access
///
/// \param sxlock A pointer to the SharedExclusiveLock object to lock
///
/// \param timeout The maximum amount of time to wait for access, or the
/// constant SSX_WAIT_FOREVER to wait forever.
///
/// Acquire a SharedExclusiveLock for exclusive access, potentially blocking
/// forever or until a specified timeout if access is not immediately
/// granted. Access will be blocked as long as one or more threads control
/// shared acesss to the resource, however once the thread requests exclusive
/// access all new shared access requests will block. Once the access is
/// granted, the thread maintains exclusive access to the resource until a
/// subsequent call of sxlock_unlock_exclusive().
///
/// Return values other than SSX_OK (0) are not necessarily errors; see \ref
/// ssx_errors 
///
/// The following return codes are non-error codes:
///
/// \retval 0 Successful completion
///
/// \retval -SXLOCK_TIMED_OUT Exclusive access was not acquired before the
/// timeout expired.
///
/// The following return codes are error codes:
///
/// \retval -SXLOCK_INVALID_OBJECT The \a sxlock parameter was NULL (0) or
/// otherwise invalid.
///
/// \retval -SSX_ILLEGAL_CONTEXT The API was called from a critical
/// interrupt context.
/// 
/// \retval -SSX_SEMAPHORE_PEND_WOULD_BLOCK The call was made from an
/// interrupt context (or before threads have been started), exclusive access
/// was not immediately available and a non-zero timeout was specified.
///
/// \retval others This API may also return codes from SSX semaphore APIs,
/// which shoudl be considered as non-recoverable errors.


int
sxlock_lock_exclusive(SharedExclusiveLock* sxlock, SsxInterval timeout)
{
    SsxMachineContext ctx;
    int rc, pending_exclusive, pending_shared;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(sxlock == 0, SXLOCK_INVALID_OBJECT);
    }

    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    rc = 0;
    do {

        if ((sxlock->running_shared == 0) &&
            (sxlock->running_exclusive == 0)) {

            // If no other thread has acquired the lock, this thread gets
            // immediate access.

            sxlock->running_exclusive = 1;

        } else {

            // Some other thread has acquired the lock.  This thread must pend
            // exclusive. In this case the sxlock->running_exclusive must be
            // set by the *_unlock() operation that unblocks the thread.
        
            rc = ssx_semaphore_pend(&(sxlock->exclusive_sem), timeout);
            if (rc == -SSX_SEMAPHORE_PEND_TIMED_OUT) {

                // This exclusive request timed out.  Since the request may
                // have blocked shared requests, then if this is the only
                // exclusive request or thread we need to unblock any pending
                // shared requests.

                if (sxlock->running_exclusive == 0) {

                    rc = ssx_semaphore_info_get(&(sxlock->exclusive_sem),
                                                0, &pending_exclusive);
                    if (rc) break;

                    if (pending_exclusive == 0) {

                        rc = ssx_semaphore_info_get(&(sxlock->shared_sem),
                                                    0, &pending_shared);
                        if (rc) break;

                        if (pending_shared != 0) {

                            sxlock->running_shared += pending_shared;
                            rc = ssx_semaphore_release_all(&(sxlock->shared_sem));
                            if (rc) break;
                        }
                    }
                }

                rc = -SXLOCK_TIMED_OUT;
            }
        }

    } while (0);

    ssx_critical_section_exit(&ctx);

    return rc;
}


/// Release a SharedExclusiveLock from exclusive access
///
/// \param sxlock A pointer to the SharedExclusiveLock object to unlock
///
/// Release a SharedExclusiveLock from exclusive access, signalling that the
/// thread no longer requires or expects exclusive access to the resource.  It
/// is an error for a thread to use this API if it has not previously locked
/// exclusive access by a call of sxlock_lock_exclusive() (or the thread is
/// unlocking a lock initialized in the exclusive-locked state).
///
/// Return values other than SSX_OK (0) are errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_ILLEGAL_CONTEXT The API was called from a critical interrupt
/// context.
///
/// \retval -SXLOCK_INVALID_OBJECT The \a sxlock parameter was NULL (0) or
/// otherwise invalid.
///
/// \retval -SXLOCK_EXCLUSIVE_UNDERFLOW There was apparently no matched call of
/// sxlock_lock_exclusive() prior to this call.
///
/// \retval others This API may also return codes from SSX semaphore APIs,
/// which should be considered as non-recoverable errors.

int
sxlock_unlock_exclusive(SharedExclusiveLock* sxlock)
{
    SsxMachineContext ctx;
    int rc, pending_exclusive, pending_shared;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF_CRITICAL_INTERRUPT_CONTEXT();
        SSX_ERROR_IF(sxlock == 0, SXLOCK_INVALID_OBJECT);
        SSX_ERROR_IF(sxlock->running_exclusive != 1, SXLOCK_SHARED_UNDERFLOW);
    }

    if (SSX_ERROR_CHECK_KERNEL) {
        SSX_PANIC_IF(sxlock->running_shared != 0, 
                     SXLOCK_SHARED_EXCLUSIVE_INVARIANT);
    }

    rc = 0;
    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    do {

        rc = ssx_semaphore_info_get(&(sxlock->exclusive_sem),
                                    0, &pending_exclusive);
        if (rc) break;

        if (pending_exclusive != 0) {

            // If there are other threads pending exclusive, make the
            // highest-priority one of them
            // runnable. sxlock->running_exclusive remains equal to 1.

            rc = ssx_semaphore_post(&(sxlock->exclusive_sem));
            if (rc) break;

        } else {

            // Otherwise unblock any/all threads pending shared

            sxlock->running_exclusive = 0;

            rc = ssx_semaphore_info_get(&(sxlock->shared_sem),
                                        0, &pending_shared);
            if (rc) break;

            if (pending_shared != 0) {

                sxlock->running_shared = pending_shared;
                rc = ssx_semaphore_release_all(&(sxlock->shared_sem));
                if (rc) break;
            }
        }
    } while (0);

    ssx_critical_section_exit(&ctx);
    
    return rc;
}
