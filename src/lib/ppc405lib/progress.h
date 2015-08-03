/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/progress.h $                                */
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
#ifndef __PROGRESS_H__
#define __PROGRESS_H__

// $Id$

/// \file progress.h
/// \brief Programmable progress (hang) checking

#include <time.h>
#include "ssx.h"
#include "byte_pool.h"
#include "ssx_io.h"

#ifndef __ASSEMBLER__

struct ProgressChecker;

/// ProgressChecker callback type
///
/// \param checker The checker that has just been checked
///
/// \param arg The private argument provided when the checker was created
///
/// \param failed The number of failed pass counts - 0 indicates no failures

typedef int (*ProgressCheckerCallback)(struct ProgressChecker* checker,
				       void*  arg,
				       size_t failed);


/// A simple progress (hang) checker.  For API details see file progress.c

typedef struct ProgressChecker {

    /// The application provided pass-count array
    uint64_t *pass_counts;

    /// The number of pass-count counters in the array.
    size_t counters;

    /// The (optional) name of the checker for reporting purposes.
    const char *name;

    /// The (optional) checker callback.
    ProgressCheckerCallback callback;

    /// The checker callback private argument
    void *arg;

    /// The dynamically-allocated saved pass counts.
    uint64_t *saved_counts;

    /// The dynamically-allocated exemption bit-vector
    ///
    /// \todo Get or implement a generic unlimited-precision bit vector
    uint8_t *exempt;

    /// The dynamically-allocated failure bit-vector
    uint8_t *failure;

    /// The number of bytes in the bit vector
    size_t bit_vector_bytes;

    /// Defer all checking the next time progress_checker_check() is called.
    ///
    /// See progress_checker_require_defer()
    int defer;

    /// The number of failures present in the *failure vector 
    size_t failed;

    /// A timer object to support time-based checking.
    SsxTimer timer;

    /// The number of times progress_checker_check() has been called on the
    /// object. 
    size_t checks;

    /// The time the last check started
    SsxTimebase start_check;

    /// The time the last check ended
    SsxTimebase end_check;

} ProgressChecker;


int
progress_checker_create(ProgressChecker *checker,
			const char *name,
			uint64_t *pass_counts,
			size_t counters,
			ProgressCheckerCallback callback,
			void *arg);

int
progress_checker_exempt(ProgressChecker *checker,
			size_t counter);

int
progress_checker_exempt_all(ProgressChecker *checker);

int
progress_checker_require(ProgressChecker *checker,
			 size_t counter);

int
progress_checker_require_defer(ProgressChecker *checker,
                               size_t counter);

int
progress_checker_check(ProgressChecker *checker);

int
progress_checker_schedule(ProgressChecker *checker,
			  SsxInterval interval,
			  SsxInterval period);

int
progress_checker_cancel(ProgressChecker *checker);

int
progress_checker_next_failure(ProgressChecker *checker, int *counter);

int 
progress_checker_delete(ProgressChecker *checker);

int
progress_checker_printk(ProgressChecker *checker,
			void *arg,
			size_t failed);

int
progress_checker_printk_dump(ProgressChecker *checker,
                             void *arg,
                             size_t failed);

void
progress_checker_dump(ProgressChecker *checker);


#endif	/* __ASSEMBLER__ */

// Error/Panic codes

#define PROGRESS_CHECKER_INVALID_OBJECT    0x00776001
#define PROGRESS_CHECKER_INVALID_ARGUMENT  0x00776002
#define PROGRESS_CHECKER_FAILED            0x00776003
#define PROGRESS_CHECKER_CALLBACK_PANIC    0x00776004
#define PROGRESS_CHECKER_INVARIANT         0x00776005
#define PROGRESS_CHECKER_ALLOCATION_FAILED 0x00776006

#endif	/* __PROGRESS_H__ */
