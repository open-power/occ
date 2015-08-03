/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/progress.c $                                */
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
// $Id

/// \file progress.c
/// \brief Programmable progress (hang) checking
///
/// This is a simple implementation of a progress (hang) checking
/// facility. The application provides an array of \e pass \e counts that are
/// expected to update/count over time.  For simplicity and generality all
/// pass counts are defined to be \c uint64_t types.  The
/// progress_checker_create() API initializes the checker, and as a
/// convenience clears the pass counts.
///
/// The checker can be created with an optional \a callback function, which
/// has the prototype
///
/// \code
/// typedef int (*ProgressCheckerCallback)(struct ProgressChecker *checker,
/// 				           void* arg,
/// 				           size_t failed)
/// \endcode
///
/// The checker callback is called \e every time a check is made by
/// progress_checker_check().  In addition to a private void* argument, the
/// parameter list of the callback includes a count of the number of counters
/// that failed to make progress - this count will be 0 if the check was
/// successful. The return value of the callback is passed back as the return
/// value of progress_checker_check().  If the callback is specified as NULL
/// (0), then a successful check returns 0, and any failure causes a return
/// code of -PROGRESS_CHECKER_FAILED.
///
/// The application can dynamically mark counters as either \e exempt or \e
/// required.  By default all counts are required to have increased
/// each time a check is made for progress.  Counters marked \e exempt when a
/// progress check is made are not checked for progress.
///
/// The application can also use the progress_checker_schedule() API to
/// schedule either one-shot or periodic checks.  The
/// progress_checker_cancel() API can be used to cancel any scheduled
/// checks. It is never an error to call this API, even if no checks are
/// currently scheduled. Note that each call of progress_checker_schedule()
/// also cancels any outstanding scheduled requests before (re-) scheduling
/// the checker.  If using the built-in timer mechanism, any calls of
/// progress_checker_check that return a non-0 value will cause a kernel panic.
///
/// If failures are detected and caught, the ProgressChecker provides a
/// primitive iteration facility for the callback or the applicaton to
/// determine which counters have failed to update. Calling
/// progress_checker_next_failure() returns either the index of the next
/// failing counter, or -1 to indicate no more failures. This iteration
/// facility is reset every time a check is made by progress_checker_check()
/// (including those made implcitly by the timer-based mechanism). There is no
/// API to reset the iteration.
///
/// The implemetation provides 2 standard callback functions:
/// progess_checker_printk() and progress_checker_printk_dump().  The former
/// callback uses printk() to print a simple report of failed counters, and if
/// there were any failures it then returns its argument as a return code.  If
/// the return code is non-zero then the lack of progress will cause a kernel
/// panic (test failure). The later callback first calls
/// progress_checker_printk(). If progress_checker_printk() returns a non-0
/// value then progress_checker_printk_dump() enters an SSX_CRITICAL crictal
/// section and prints a full kernel state dump that may be useful to help
/// diagnose the hang.
///
/// \note We do not make the kernel dump the default or only behavior because
/// it could take 1ms or more to produce the large quantity of formatted
/// output required, which could be a significant amount of wall time in a
/// logic simulation environment.
///
/// The progress_checker_create() API could be used in a couple of ways as
/// illustrated below:
///
/// \code 
///
/// ProgressChecker progress;
/// uint64_t counter;
///
/// progress_checker_create(&progress, "progress", counter, 1, 
///                         progress_checker_printk,
///                         (void*)-PROGRESS_CHECKER_FAILED);
///
/// OR
///
/// progress_checker_create(&progress, "progress", counter, 1, 
///                         progress_checker_printk, 0);
///
/// \endcode
///
/// The first usage prints a report and panics the test if lack of progress is
/// detected.  The second form simply prints a report in the event of a lack
/// or progress.  The second form may be useful to report on counters that
/// only have a statistical probability of making progress, however be aware
/// that the report is generated in an interrupt context and all thread
/// activity will be blocked until the formatted I/O is complete.
///
/// Notes:
/// 
/// This implementation requires the \c byte_pool facility and malloc() to be
/// set up as the ProgressChecker allocates dynamic storage during
/// initialization to store the previous pass counts.
///
/// It is probably not a good idea to use a single ProgressChecker for both
/// manual and timer-based checking, since there is no protection in the
/// implementation for mutiple accesses to the ProgressChecker.

#include "progress.h"
#include "ssx_dump.h"

// The built-in timer callback

static void
progress_callback(void *arg)
{
    ProgressChecker *checker = (ProgressChecker *)arg;

    if (progress_checker_check(checker)){
	if (0) {
	    progress_checker_dump(checker);
	}
	SSX_PANIC(PROGRESS_CHECKER_FAILED);
    }
}


// Bit-vector operations manage the array of bits using little-endian
// protocols 

static inline void
bit_vector_set(uint8_t *vector, size_t bit)
{
    vector[bit / 8] |= (1 << (bit % 8));
}


static inline void
bit_vector_clear(uint8_t *vector, size_t bit)
{
    vector[bit / 8] &= ~(1 << (bit % 8));
}


static inline int
bit_vector_is_set(uint8_t *vector, size_t bit)
{
    return ((vector[bit / 8] & (1 << (bit % 8))) != 0);
}


// NB: We don't have a bit-vector object with a size included.  For this
// application we can only call this API if we know that there is at least 1
// bit set in the vector.

static size_t
bit_vector_find_first_set(uint8_t *vector)
{
    size_t byte = 0;
    
    while (vector[byte] == 0) {
	byte++;
    }

    return (8 * byte) + __builtin_ffs(vector[byte]) - 1;
}


/// Create a progress checker
///
/// \param checker A pointer to an uninitialized or idle ProgressChecker
///
/// \param name An optional character string associated with the checker
///
/// \param pass_counts An array of pass counters - the array will be cleared
/// by this API.
///
/// \param counters The number of counters in the array
///
/// \param callback This function is called \e every time a check is
/// completed.  
///
/// \param arg The private argument of the callback function
///
/// For an overview of the ProgressChecker and its APIs, see the documentation
/// for the file progress.c
///
/// Return values other than 0 are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -PROGRESS_CHECKER_INVALID_OBJECT A null (0) pointer was provided
/// as the \a checker argument.
///
/// \retval -PROGRESS_CHECKER_INVALID_ARGUMENT A null (0) pointer was provided
/// as the \a pass_counts argument, or the number of \a counters can not be
/// represented as a signed integer.
///
/// \retval -PROGRESS_CHECKER_ALLOCATION_FAILED Memory allocation of dynamic
/// memory failed.  This is treated as a fatal error here.
///
/// This API may also return or signal other errors from its implementation
/// APIs. 

int
progress_checker_create(ProgressChecker *checker,
			const char* name,
			uint64_t *pass_counts,
			size_t counters,
			ProgressCheckerCallback callback,
			void *arg)
{
    int rc, bytes;
    void *memory;

    if (SSX_ERROR_CHECK_API) {
	SSX_ERROR_IF((checker == 0),
		     PROGRESS_CHECKER_INVALID_OBJECT);
	SSX_ERROR_IF((pass_counts == 0) ||
		     (counters != (int)counters),
		     PROGRESS_CHECKER_INVALID_ARGUMENT);
    }

    // Install and clear the counters

    checker->pass_counts = pass_counts;
    memset((void *)pass_counts, 0, counters * sizeof(uint64_t));
    checker->counters = counters;

    // Allocate and clear dynamic memory

    memory = calloc(counters, sizeof(uint64_t));
    checker->saved_counts = (uint64_t *)memory;

    bytes = (counters / 8) + (counters % 8 ? 1 : 0);
    checker->bit_vector_bytes = bytes;

    memory = calloc(bytes, 1);
    checker->exempt = (uint8_t *)memory;
    
    memory = calloc(bytes, 1);
    checker->failure = (uint8_t *)memory;

    if (SSX_ERROR_CHECK_API) {
	SSX_ERROR_IF((counters != 0) && 
		     ((checker->saved_counts == 0) ||
		      (checker->exempt == 0) ||
		      (checker->failure == 0)),
		     PROGRESS_CHECKER_ALLOCATION_FAILED);
    }

    // Initialize other fields

    checker->name = name;
    checker->callback = callback;
    checker->arg = arg;
    checker->failed = 0;
    checker->checks = 0;

    // Initialize the timer structure.

    rc = ssx_timer_create(&(checker->timer), 
			  progress_callback, 
			  (void *)checker);
    if (rc) return rc;

    return 0;
}


/// Exempt a pass count from progress checking
///
/// \param checker A pointer to an initialized ProgressChecker
///
/// \param counter The index of the counter to exempt
///
/// For an overview of the ProgressChecker and its APIs, see the documentation
/// for the file progress.c
///
/// Return values other than 0 are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -PROGRESS_CHECKER_INVALID_OBJECT A null (0) pointer was provided
/// as the \a checker argument.
///
/// \retval -PROGRESS_CHECKER_INVALID_ARGUMENT The \a counter argument is not
/// valid.

int
progress_checker_exempt(ProgressChecker *checker,
			size_t counter)
{
    if (SSX_ERROR_CHECK_API) {
	SSX_ERROR_IF(checker == 0,
		     PROGRESS_CHECKER_INVALID_OBJECT);
	SSX_ERROR_IF(counter >= checker->counters,
		     PROGRESS_CHECKER_INVALID_ARGUMENT);
    }

    bit_vector_set(checker->exempt, counter);

    return 0;
}


/// Exempt all pass counts from progress checking
///
/// \param checker A pointer to an initialized ProgressChecker
///
/// For an overview of the ProgressChecker and its APIs, see the documentation
/// for the file progress.c
///
/// This API is provided to support applications where pass-count-updating
/// processes are added dynamically. This API coule typically be called
/// immediately after progress_checker_create(). Them, as each process was
/// created it would call progress_checker_require() for the pass count.
///
/// Return values other than 0 are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -PROGRESS_CHECKER_INVALID_OBJECT A null (0) pointer was provided
/// as the \a checker argument.
int
progress_checker_exempt_all(ProgressChecker *checker)
{
    if (SSX_ERROR_CHECK_API) {
	SSX_ERROR_IF(checker == 0,
		     PROGRESS_CHECKER_INVALID_OBJECT);
    }

    memset(checker->exempt, -1, checker->bit_vector_bytes);

    return 0;
}


/// Require a pass count to update for progress checking
///
/// \param checker A pointer to an initialized ProgressChecker
///
/// \param counter The index of the counter to require
///
/// For an overview of the ProgressChecker and its APIs, see the documentation
/// for the file progress.c
///
/// Return values other than 0 are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -PROGRESS_CHECKER_INVALID_OBJECT A null (0) pointer was provided
/// as the \a checker argument.
///
/// \retval -PROGRESS_CHECKER_INVALID_ARGUMENT The \a counter argument is not
/// valid.

int
progress_checker_require(ProgressChecker *checker,
			size_t counter)
{
    if (SSX_ERROR_CHECK_API) {
	SSX_ERROR_IF(checker == 0,
		     PROGRESS_CHECKER_INVALID_OBJECT);
	SSX_ERROR_IF(counter >= checker->counters,
		     PROGRESS_CHECKER_INVALID_ARGUMENT);
    }

    bit_vector_clear(checker->exempt, counter);

    return 0;
}


/// Require a pass count to update for progress checking avoiding races
///
/// \param checker A pointer to an initialized ProgressChecker
///
/// \param counter The index of the counter to require
///
/// For an overview of the ProgressChecker and its APIs, see the documentation
/// for the file progress.c
///
/// If a pass counter is marked "exempt" but then later marked "required",
/// there is a potential race between the update of the pass counter and the
/// next check, particularly when the checker is scheduled periodically. This
/// form of the progress_checker_require() marks the progress checker such
/// that \e all checks are deferred on the next call of
/// progress_checker_check() targeting the object in order to avoid the race.
///
/// Return values other than 0 are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -PROGRESS_CHECKER_INVALID_OBJECT A null (0) pointer was provided
/// as the \a checker argument.
///
/// \retval -PROGRESS_CHECKER_INVALID_ARGUMENT The \a counter argument is not
/// valid.
int
progress_checker_require_defer(ProgressChecker *checker,
                               size_t counter)
{
    int rc;
    SsxMachineContext ctx;

    ssx_critical_section_enter(SSX_CRITICAL, &ctx);

    rc = progress_checker_require(checker, counter);
    checker->defer = 1;

    ssx_critical_section_exit(&ctx);

    return rc;
}


/// Check for progress in every required pass counter.
///
/// \param checker A pointer to an initialized ProgressChecker
///
/// For an overview of the ProgressChecker and its APIs, see the documentation
/// for the file progress.c
///
/// Return values other than 0 are not necessarily errors; see \ref
/// ssx_errors 
///
/// \retval various  Except for the error listed below,
/// progress_checker_check() returns the code returned by the callback
/// function. If no callback was provided when the checker was created, then 0
/// is returned for success and -PROGRESS_CHECKER_FAILED is returned in the
/// event of a lack of progress.
///
/// \retval -PROGRESS_CHECKER_INVALID_OBJECT A null (0) pointer was provided
/// as the \a checker argument.

int
progress_checker_check(ProgressChecker *checker)
{
    size_t i;

    if (SSX_ERROR_CHECK_API) {
	SSX_ERROR_IF(checker == 0,
		     PROGRESS_CHECKER_INVALID_OBJECT);
    }

    // Avoid doing this step unless necessary

    if (checker->failed != 0) {
	checker->failed = 0;
	memset((void *)checker->failure, 0, checker->bit_vector_bytes);
    }

    // Check, unless checking has been deferred for 1 time by
    // progress_checker_require_defer().

    if (checker->defer) {

        checker->defer = 0;

    } else {

        SSX_ATOMIC(SSX_CRITICAL, checker->start_check = ssx_timebase_get());

        for (i = 0; i < checker->counters; i++) {
	
            if ((checker->pass_counts[i] <= checker->saved_counts[i]) && 
                !(bit_vector_is_set(checker->exempt, i))) {

                checker->failed++;
                bit_vector_set(checker->failure, i);
            }
            checker->saved_counts[i] = checker->pass_counts[i];
        }

        SSX_ATOMIC(SSX_CRITICAL, checker->end_check = ssx_timebase_get());
    }

    checker->checks++;

    if (checker->callback) {
	return checker->callback(checker, checker->arg, checker->failed);
    } else if (checker->failed != 0) {
	return -PROGRESS_CHECKER_FAILED;
    } else {
	return 0;
    }
}


/// Schedule progress checks (periodically) in the future.
///
/// \param checker A pointer to an initialized ProgressChecker
///
/// \param interval The relative time of the (first) check
///
/// \param period If non-zero, checks will be made periodically with this
/// period. 
///
/// For an overview of the ProgressChecker and its APIs, see the documentation
/// for the file progress.c. See the SSX documentation for a discussion of
/// timer scheduling in SSX.
///
/// Return values other than 0 errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -PROGRESS_CHECKER_INVALID_OBJECT A null (0) pointer was provided
/// as the \a checker argument.
///
/// This API may also return or signal other errors from its implementation
/// APIs. 


int
progress_checker_schedule(ProgressChecker *checker,
			  SsxInterval interval,
			  SsxInterval period)
{
    int rc;

    if (SSX_ERROR_CHECK_API) {
	SSX_ERROR_IF(checker == 0,
		     PROGRESS_CHECKER_INVALID_OBJECT);
    }

    rc = ssx_timer_cancel(&(checker->timer));
    if (rc != -SSX_TIMER_NOT_ACTIVE) return rc;

    rc = ssx_timer_schedule(&(checker->timer), interval, period);
    if (rc) return rc;

    return 0;
}

    
/// Cancel all future (periodic) progress checks
///
/// \param checker A pointer to an initialized ProgressChecker
///
/// For an overview of the ProgressChecker and its APIs, see the documentation
/// for the file progress.c. See the SSX documentation for a discussion of
/// timer scheduling in SSX.
///
/// Return values other than 0 errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -PROGRESS_CHECKER_INVALID_OBJECT A null (0) pointer was provided
/// as the \a checker argument.
///
/// This API may also return or signal other errors from its implementation
/// APIs. 

int
progress_checker_cancel(ProgressChecker *checker)
{
    int rc;

    if (SSX_ERROR_CHECK_API) {
	SSX_ERROR_IF(checker == 0,
		     PROGRESS_CHECKER_INVALID_OBJECT);
    }

    rc = ssx_timer_cancel(&(checker->timer));
    if (rc) return rc;

    return 0;
}

    
/// Iterate over progress check failures
///
/// \param checker A pointer to an initialized ProgressChecker
///
/// \param counter Will return the index of the next failing counter, or -1 to
/// indicate no more failing counters.
///
/// For an overview of the ProgressChecker and its APIs, see the documentation
/// for the file progress.c. 
///
/// Return values other than 0 errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -PROGRESS_CHECKER_INVALID_OBJECT A null (0) pointer was provided
/// as the \a checker argument.

int
progress_checker_next_failure(ProgressChecker *checker, int *counter)
{
    if (SSX_ERROR_CHECK_API) {
	SSX_ERROR_IF(checker == 0,
		     PROGRESS_CHECKER_INVALID_OBJECT);
    }

    if (checker->failed == 0) {
	*counter = -1;
	return 0;
    }

    *counter = bit_vector_find_first_set(checker->failure);
    bit_vector_clear(checker->failure, *counter);
    checker->failed--;

    return 0;
}


/// A standard way to print the results of a progress check failure, suitable
/// for use as a ProgressChecker callback.
///
/// \param checker The checker - which may or may not have failed. If the
/// checker did fail, then a failure report is printed using printk().
///
/// \param arg The value to return in case of failure.  In case of
/// success, 0 is returned.
///
/// \param failed - The number of failed checks

int
progress_checker_printk(ProgressChecker *checker,
			void *arg,
			size_t failed)
{
    int counter;

    if (!failed) {
	return 0;
    }

    printk("---------------------------------------------------------------\n");
    printk("-- Progress check failed for \"%s\" (%p).\n", 
	   checker->name, checker);
    printk("-- Check %zu over interval 0x%016llx - 0x%016llx\n",
           checker->checks, checker->start_check, checker->end_check);
    printk("-- %zu failed counter%s listed below\n", 
	   failed, failed > 1 ? "s are" : " is");
    printk("---------------------------------------------------------------\n");

    do {
	progress_checker_next_failure(checker, &counter);
	if (counter < 0) {
	    break;
	}
	printk("%4d. 0x%016llx\n", counter, checker->pass_counts[counter]);
    } while (1);

    printk("---------------------------------------------------------------\n");

    return (int)arg;
}    


/// Call progress_checker_printk(), then create a kernel dump on failure
///
/// \param checker The checker - which may or may not have failed. If the
/// checker did fail, then a failure report is printed using
/// progress_checker_printk().
///
/// \param arg The value to return in case of failure.  In case of
/// success, 0 is returned.
///
/// \param failed - The number of failed checks
///
/// If progress_checker_printk() fails with a non-0 return code then this API
/// prints a full SSX kernel dump after the progress_checker_printk() report.
int
progress_checker_printk_dump(ProgressChecker *checker,
                             void *arg,
                             size_t failed)
{
    int rc;
    SsxMachineContext ctx;
    
    rc = progress_checker_printk(checker, arg, failed);
    if (rc != 0) {
        ssx_critical_section_enter(SSX_CRITICAL, &ctx);
        ssx_dump(ssxout, 0);
        ssx_critical_section_exit(&ctx);
    }

    return rc;
}    


/// Dump a progress checker structure using printk()

void
progress_checker_dump(ProgressChecker *checker)
{
    size_t i;

    printk("Dump of progress checker \"%s\" (%p)\n"
	   "  Counters = %zu\n"
	   "  Checks   = %zu\n"
	   "  Failed   = %zu\n"
	   "  Callback = %p(%p)\n",
	   checker->name, checker, checker->counters, checker->checks,
	   checker->failed, checker->callback, checker->arg);
    
    printk("  Pass Counts (%p) :\n", checker->pass_counts);
    for (i = 0; i < checker->counters; i++) {
    	printk("    %9d%c 0x%016llx\n", 
    	       i, 
    	       bit_vector_is_set(checker->exempt, i) ? '*' : ' ',
    	       checker->pass_counts[i]);
    }
    printk("  Saved Counts (%p) :\n", checker->saved_counts);
    for (i = 0; i < checker->counters; i++) {
    	printk("    %9d   0x%016llx\n", 
    	       i, checker->saved_counts[i]);
    }
}

    
    
	   


    

