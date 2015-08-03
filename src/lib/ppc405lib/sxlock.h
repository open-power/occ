/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/sxlock.h $                                  */
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
#ifndef __SXLOCK_H__
#define __SXLOCK_H__

// $Id$

/// \file sxlock.h
/// \brief The implementation of a SharedExclusiveLock
///
/// The SharedExclusiveLock is documented in the comments for the file
/// sxlock.c 

// Error/panic codes

#define SXLOCK_INVALID_OBJECT             0x00795501
#define SXLOCK_INVALID_ARGUMENT           0x00795502
#define SXLOCK_TIMED_OUT                  0x00795503
#define SXLOCK_SHARED_UNDERFLOW           0x00795504
#define SXLOCK_EXCLUSIVE_UNDERFLOW        0x00795505
#define SXLOCK_SHARED_EXCLUSIVE_INVARIANT 0x00795506 

/// A shared-exclusive lock object (also called a readers-write lock)
///
/// This facility is documented in the file sxlock.c

typedef struct {

    /// A semaphore for threads requesting shared access
    SsxSemaphore shared_sem;

    /// A semaphore for threads requesting exclusive access
    SsxSemaphore exclusive_sem;

    /// The number of threads running shared
    SsxSemaphoreCount running_shared;

    /// The number of threads running exclusive
    SsxSemaphoreCount running_exclusive;

} SharedExclusiveLock;


/// Static initialization of a shared-exclusive lock object
///
/// \param[in] shared The number of threads running shared at static
/// initialization
///
/// \param[in] exclusive The number of threads running exclusive at static
/// initialization. 
///
/// Note that it is an error to specify both \a shared and \a exclusive as
/// non-0.

#define SXLOCK_INITIALIZATION(shared, exclusive)        \
    {                                                   \
        SSX_SEMAPHORE_INITIALIZATION(0, 0),             \
            SSX_SEMAPHORE_INITIALIZATION(0, 0),         \
            (shared), (exclusive)                       \
            }

/// Declare and initialize a shared-exclusive lock

#define SXLOCK(sxlock, shared, exclusive)                               \
    SharedExclusiveLock sxlock = SXLOCK_INITIALIZATION(shared, exclusive)



int
sxlock_create(SharedExclusiveLock* sxlock,
              SsxSemaphoreCount shared,
              SsxSemaphoreCount exclusive);

int
sxlock_lock_shared(SharedExclusiveLock* sxlock, SsxInterval timeout);

int
sxlock_unlock_shared(SharedExclusiveLock* sxlock);

int
sxlock_lock_exclusive(SharedExclusiveLock* sxlock, SsxInterval timeout);

int
sxlock_unlock_exclusive(SharedExclusiveLock* sxlock);

#endif // __SXLOCK_H__
