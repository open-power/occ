/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/pgp_async.c $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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
/// \file pgp_async.c
/// \brief Support for asynchronous request queuing and callback mechanisms
///
/// This file implements device drivers for asynchronous requests.  The model
/// for devices like the PORE engines and the PBA block copy engines is that
/// the application creates self-contained requests for jobs to run on the
/// engine.  The application then schedules the request and continues normal
/// processing, or threads can request to simply block in place until the
/// request finishes.  Queue management is handled in the interrupt handler
/// for the engine. As each queued job finishes, the next request (if any) is
/// started on the engine and the optional callback is invoked or scheduled.
///
/// The application can either use a polling protocol or the asynchronous
/// callback mechanism to determine the state of the request.  A timeout
/// mechanism is also provided that cancels or kills a job that does not
/// complete within a fixed time. Error handling in the event that the request
/// does not complete in time or has failed is standardized by the
/// implementation, but can be overridden by the application.
///
/// Asynchronous request interrupt handlers can run either as critical or
/// noncritical handlers.  Some engines may support queuing requests that
/// complete immediately; therefore the kernel context of the callback may be
/// either a thread or interupt context, and in general, callbacks should not
/// make assumptions about the kernel context when they run.
///
/// If the callback is non-NULL and the request was created with \c
/// ASYNC_CALLBACK_IMMEDIATE, the callback is then immediately invoked in the
/// current context. In general, immediate callbacks should be short and sweet
/// (to reduce interrupt latency) and should not make SSX kernel calls.
///
/// If the request has a non-NULL callback and was created with \c
/// ASYNC_CALLBACK_DEFERRED then the callback will be deferred to a
/// noncritical interrupt context. Deferred callbacks are queued, then run
/// later in response to a reserved IPI.  Deferred callbacks always run as
/// noncritical interrupt handlers with noncritical interrupts \e enabled,
/// similar to SSX timer callbacks.
///
/// If the request has a non-null callback and was created with \c
/// ASYNC_CALLBACK_NONCRITICAL, then the callback will be run immediately from
/// a noncritical handler or thread environment, or deferred from a critical
/// interrupt context.  Similar to immediate callbacks, callbacks marked
/// noncritical should be short and sweet (to reduce interrupt latency), but
/// may make SSX kernel calls.
///
/// Regardless of whether the callback is critical or noncritical, the
/// callback is coded as a normal C or assembler subroutine with a single
/// void* argument.
///
/// As a programming shortcut, the AsyncRequest includes a semaphore object.
/// A special AsyncRequest option ASYNC_REQUEST_BLOCKING indicates that the
/// thread scheduling the request should block on the semaphore of the
/// AsyncRequest (with SSX_WAIT_FOREVER) until the request is complete. Note
/// that a separate callback can be specified even if ASYNC_REQUEST_BLOCKING
/// is specified.
///
/// Requests are always timestamped. The \a start_time field of the request is
/// set from the SSX timebase when the request is first 'run' on the device.
/// Request types that may require multiple back-to-back 'runs' (like PBA
/// block copies with more than 4K data) only record the initial 'run'. The \a
/// end_time field of the request is set from the SSX timebase when the job
/// finishes on the hardware, but before any callback is run.  These
/// timestamps cover time intervals not visible to the application; The
/// application is responsible for timestamping the period between request
/// queing and the \a start_time, and between the \a end_time and callback
/// completion if required.  The timestamps can be robustly recovered from the
/// request using the API async_request_timestamps_get().
///
/// This is largely a generic implementation, designed to reduce code space by
/// allowing the PORE, PBA and OCB drivers to use the same generic data
/// structures and code.  This is supported by the 'single-inheritence class
/// hierarchy' described in the comments for pgp_async.h.
///
/// <b> Request Completion and Callback States </b>
///
/// The application can determine what happend to the job by observing the
/// request state and callback return code when the request becomes idle.
/// A request is not considered idle until the job has run to completion or
/// been aborted, any callback has been run, any timeout has been cancelled,
/// and any thread pending on request completion has been maxde runnable.
///
/// Normally the application should see the request state (\a request->state)
/// ASYNC_REQUEST_STATE_COMPLETE and a  callback return code 
/// (\a request=->callback_rc) of 0 which indicates that the job and callback
/// both completed normally.  If the request did not complete normally it
/// could be due to one of several reasons.  For further analysis the request
/// also includes a field \a abort_state that records the state the job was in
/// when it was aborted (i.e., cancelled, killed, timed out or error-out).
///
/// <b> Timeout Semantics </b>
///
/// Any timeout other than SSX_WAIT_FOREVER specifies a timeout covering the
/// interval spanning the time a job is scheduled until the time the job
/// completes on the hardware.  If the job is still wating to execute when it
/// times out then the job is simply removed from the queue and marked as
/// having timed out.  If the job is running when it times out then the job is
/// forceably removed from the hardware, which may have unintended or
/// indeterminate consequences. The application may need to consider whether
/// it is safe to continue after a forced timeout.
///
/// Specifying a timeout involves quite a bit of overhead, since a timer needs
/// to be scheduled and cancelled each time a job is run.  If the interrupt
/// handler for a device is a critical handler then the timeout cancellation
/// will need to be deferred to the callback queue, potentially increasing
/// overhead even further.
///
/// <b> Implementation Notes </b>
///
/// - The \e queue objects will normally be global data structures that
/// persist throughout the life of the application.
///
/// - The \e request objects may be either global or local data
/// structures. However, since \e request objects are not copied, and pointers
/// to them are stored in the \e queue objects, \a request objects should not
/// be allocated on the stack if the stack frame could become invalid before
/// the request completes.
///
/// \todo Once all function is developed and tested, convert interrupt
/// handling to fast-mode assembler routines.

#include "ssx.h"


////////////////////////////////////////////////////////////////////////////
// Global Data
////////////////////////////////////////////////////////////////////////////

/// Queue of deferred callbacks

static SsxDeque G_async_callback_queue;


////////////////////////////////////////////////////////////////////////////
// FFDC 
////////////////////////////////////////////////////////////////////////////

/// Collect FFDC for the PLB (OCI) arbiter
///
/// \param ffdc A pointer to an OciFfdc structure to be filled in.
///
/// \param master_id The PLB (OCI) master Id of the master of interest.
///
/// Note: The PEAR and PESR hold error information for all OCI masters
/// _except_ the OCC ICU and DCU. ICU and DCU errors are in the STO_PESR and
/// STO_PEAR. Currently there is no need to collect those DCRs for 'async'
/// errors. 

void
oci_ffdc(OciFfdc* ffdc, int master_id)
{
    uint32_t pesr_lock_mask;

    ffdc->pearl.value = mfdcr(PLB_PEARL);
    ffdc->pesr.value = mfdcr(PLB_PESR);

    pesr_lock_mask = 0x30000000 >> (4 * master_id);
    if (ffdc->pesr.value & pesr_lock_mask) {
        ffdc->mine = 1;
        mtdcr(PLB_PESR, pesr_lock_mask);
    } else {
        ffdc->mine = 0;
    }
}



////////////////////////////////////////////////////////////////////////////
// AsyncQueue
////////////////////////////////////////////////////////////////////////////

// Start an asynchronous request on the device with timestamping.  This will
// always be called from a critical section, and any timestamp is collected
// immediately before the request is kicked off on the device. Devices like
// the BCE engines and the OCB queue drivers may run the same request multiple
// times to get all of the data moved. Therefore the initial timestamp is only
// captured the first time the request is run on the device.

static inline int
async_request_run(AsyncRequest *request)
{
    if (request->state != ASYNC_REQUEST_STATE_RUNNING) {
        request->start_time = ssx_timebase_get();
    }
    request->state = ASYNC_REQUEST_STATE_RUNNING;
    return request->run_method(request);
}


// Create (initialize) a generic AsyncQueue
//
// This is an internal API used to initialize generic request queues.  The
// caller is assumed to have done all error checking on the parameters.
//
// This is a simple initialization that resets the queues and sets the state
// to QUEUE_STATE_IDLE.  This routine should only be called on uninitialized
// AsyncQueue objects.

int
async_queue_create(AsyncQueue *queue, AsyncEngine engine)
{
    ssx_deque_sentinel_create(&(queue->deque));
    queue->current = 0;
    queue->engine = engine;
    queue->state = ASYNC_QUEUE_STATE_IDLE;

    return 0;
}


// Generic completion of a request.  This is called both by async_handler()
// and async_request_deque().

static void
async_request_complete(AsyncRequest *request)
{
    SsxMachineContext ctx;
    AsyncRequestCallback callback;
    int completed;

    // Handle callbacks and deferred processing of the job that just
    // finished.  No callback is easy.  If the job does have a callback
    // that we can execute immediately then that is done immediately.
    // Note that 'completed' here means only that the callback is complete.

    callback = request->callback;
    if (!callback) {

        completed = 1;

    } else if ((request->options & ASYNC_CALLBACK_IMMEDIATE) ||
               ((request->options & ASYNC_CALLBACK_NONCRITICAL) &&
                !__ssx_kernel_context_critical_interrupt())) {

        request->state = ASYNC_REQUEST_STATE_CALLBACK_RUNNING;
        request->callback_rc = callback(request->arg);
        completed = 1;

    } else {

        request->state = ASYNC_REQUEST_STATE_CALLBACK_QUEUED;
        completed = 0;
    }


    // If the callback completed then we go ahead and cancel any timeout
    // and/or wake the thread if possible.  In critical interrupt contexts
    // we always have to defer these operations, so we may lose 'complete'
    // status. 

    if (completed && 
        ((request->timeout != SSX_WAIT_FOREVER) ||
         (request->options & ASYNC_REQUEST_BLOCKING))) {

        if (__ssx_kernel_context_critical_interrupt()) {

            request->state = ASYNC_REQUEST_STATE_POSTPROCESSING;
            completed = 0;

        } else {
            if (request->timeout != SSX_WAIT_FOREVER) {
                ssx_timer_cancel(&(request->timer));
            }
            if (request->options & ASYNC_REQUEST_BLOCKING) {
                ssx_semaphore_post(&(request->sem));
            }
        }
    }


    // A truly completed job gets its completion state here.  Otherwise we
    // have to schedule the deferred postprocessing.

    if (completed) {
            
        request->state = request->completion_state;

    } else {

        ssx_critical_section_enter(SSX_CRITICAL, &ctx);

        if (request->options & ASYNC_CALLBACK_PRIORITY) {
            ssx_deque_push_front(&G_async_callback_queue,
                                 &(request->deque));
        } else {
            ssx_deque_push_back(&G_async_callback_queue,
                                &(request->deque));
        }

        ssx_critical_section_exit(&ctx);
        ssx_irq_status_set(PGP_IRQ_ASYNC_IPI, 1);
    }
}


// The generic handler for asynchonous device completion.
//
// This handler processes completions of generic device requests, as well as
// the initial running of jobs when the queue is idle.  Error completions are
// initially handled by async_error_handler() which then calls async_handler()
// to finish the failed job and start the next job if possible.  The initial
// device handler must manage interrupts and provide a method to run the
// 'current' job in the queue. The engine-specific handler may also iterate
// the current job until it is complete - this handler should only be called
// when the current job is complete.
//
// Some engines may have jobs that can complete immediately. This handler
// iterates over jobs in the queue as long as the run method of the current
// job returns the code -ASYNC_REQUEST_COMPLETE.
//
// NB : Normally this call is made from an interrupt handler, however it may
// be called from job scheduling code if the engine is idle. Regardless, the
// caller must insure that any call for a queue is protected against
// interrupts for that queue.

void
async_handler(AsyncQueue *queue)
{
    AsyncRequest *finished, *current;
    int rc;

    // This loop is repeated as long as any job started in a loop completes
    // immediately.

    do {

        // This API may be called on an idle queue, which indicates that we
        // should simply start the job on the head of the queue.  Otherwise
        // save a pointer to the job that just finished and update its
        // end_time.

        if (queue->state == ASYNC_QUEUE_STATE_IDLE) {

            finished = 0;

        } else {

            finished = (AsyncRequest *)(queue->current);

            if (SSX_ERROR_CHECK_KERNEL && (finished == 0)) {
                SSX_PANIC(ASYNC_PHANTOM_INTERRUPT);
            }

            finished->end_time = ssx_timebase_get();

        }


        // If the queue is in an error state we will not schedule any further
        // jobs on this queue.  Otherwise we start the next job running on the
        // engine. 

        if (queue->state == ASYNC_QUEUE_STATE_ERROR) {

            queue->current = 0;
            rc = 0;

        } else {

            current = (AsyncRequest *)ssx_deque_pop_front(&(queue->deque));
            queue->current = current;

            if (current) {
                queue->state = ASYNC_QUEUE_STATE_RUNNING;
                rc = async_request_run(current);
            } else {
                queue->state = ASYNC_QUEUE_STATE_IDLE;
                rc = 0;
            }
        }


        // If no job just finished, continue with the loop.  If the job we
        // just enqueued finished immediately it will be 'finished' on the
        // next loop (it would have given an rc ==
        // -ASYNC_REQUEST_COMPLETE). Otherwise complete the request.

        if (finished != 0) {

            async_request_complete(finished);
        }

   } while (rc == -ASYNC_REQUEST_COMPLETE);
}


/// Schedule (queue for execution) a generic asynchronous request.
///
/// \param request An initialized and idle AsyncRequest
///
///
/// The request is queued for execution with all of the parameters provided
/// when the request was created.  It is considered an error to (re)schedule a
/// request that is currently scheduled, running, or has the callback queued or
/// in execution. Requests either need to be idle, or must be explicitly
/// cancelled or killed before thay can be (re)scheduled.  Because engine queue
/// interrupt handlers may run as critical interrupts, this routine operates in
/// a short \c SSX_CRITICAL critical section.
///
/// If the request is made to an otherwise empty queue then the async_handler()
/// must also be called immediately to begin the job. This will extend the
/// critical section slightly. This routine will not start a request on a
/// queue that has halted due to an error. The request will simply be enqueued
/// in that case.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_OBJECT_SCHEDULE The \a request is NULL (0).
///
/// \retval -ASYNC_REQUEST_NOT_IDLE The \a request is (still) active in some
/// way at entry.
///
/// \retval -ASYNC_REQUEST_NOT_COMPLETE This code is returned for requests that
/// do not complete successfully, but only if ASYNC_REQUEST_BLOCKING is 
/// specified. The caller will need to examine the request state if necessary 
/// to determine whether the request failed, was cancelled or timed out.
///
/// \todo Consider making the critical section priority depend on the device
/// queue priority here, by adding a priority field to the queue. Without this
/// we always have to run the async_handler() in an SSX_CRITICAL critical
/// section.

int
async_request_schedule(AsyncRequest *request)
{
    SsxMachineContext ctx = SSX_THREAD_MACHINE_CONTEXT_DEFAULT; // For GCC
    AsyncQueue *queue;
    int rc;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(request == 0, ASYNC_INVALID_OBJECT_SCHEDULE);
    }

    rc = 0;
    ssx_critical_section_enter(SSX_CRITICAL, &ctx);

    do {

        // Check to insure the request is idle (which check must be done in
        // the critical section), then start any required timeout.

        if (SSX_ERROR_CHECK_API) {
            if (!async_request_is_idle(request)) {
                rc = -ASYNC_REQUEST_NOT_IDLE;
                break;
            }
        }

        if (request->timeout != SSX_WAIT_FOREVER) {
            rc = ssx_timer_schedule(&(request->timer), request->timeout, 0);
            if (rc) break;
        }
    

        // Enqueue the request and initialize the request state.

        queue = request->queue;

        if (request->options & ASYNC_REQUEST_PRIORITY) {
            ssx_deque_push_front(&(queue->deque), &(request->deque));
        } else {
            ssx_deque_push_back(&(queue->deque), &(request->deque));
        }

        request->state = ASYNC_REQUEST_STATE_QUEUED;
        request->completion_state = ASYNC_REQUEST_STATE_COMPLETE;
        request->callback_rc = 0;


        //  If the queue is idle, call the async_handler() to start the job.
        //  Then block the calling thread if required.

        if (queue->state == ASYNC_QUEUE_STATE_IDLE) {
            async_handler(queue);
        }

        if (request->options & ASYNC_REQUEST_BLOCKING) {
            rc = ssx_semaphore_pend(&(request->sem), SSX_WAIT_FOREVER);
            if (rc) break;

            if (!async_request_completed(request)) {
                rc = -ASYNC_REQUEST_NOT_COMPLETE;
                break;
            }
        }

    } while (0);

    ssx_critical_section_exit(&ctx);

    return rc;
}


// The generic error handler
//
// This is a generic handler called in response to an error interrupt or
// timeout.  This handler calls a request-specific error method to stop the
// hardware device, collect FFDC and make the hardware device runnable again
// if possible.  Then the current request is marked as having failed, and the
// generic async_handler() is called which will start the next job (if any)
// and take care of the callbacks for the failed request.  If the error
// method returns a non-0 return code then the error is non-recoverable and
// the queue is marked with the error state.

void
async_error_handler(AsyncQueue* queue, uint8_t completion_state)
{
    AsyncRequest *finished;

    finished = (AsyncRequest *)(queue->current);

    if (SSX_ERROR_CHECK_KERNEL && (finished == 0)) {
        SSX_PANIC(ASYNC_PHANTOM_ERROR);
    }

    if (finished->error_method) {
        if (finished->error_method(finished)) {
            queue->state = ASYNC_QUEUE_STATE_ERROR;
        }
    }
    finished->abort_state = finished->state;
    finished->completion_state = completion_state;
    async_handler(queue);
}


////////////////////////////////////////////////////////////////////////////
// AsyncRequest
////////////////////////////////////////////////////////////////////////////

// Dequeue a queued AsyncRequest
//
// This is an internal API, always called from an SSX_CRITICAL critical
// section. The request is known to be queued in one of the async queues.  It
// is removed from the queue and its state is updated.

static void
async_request_dequeue(AsyncRequest* request, uint8_t completion_state)
{
    ssx_deque_delete((SsxDeque*)request);
    request->abort_state = request->state;
    request->completion_state = completion_state;
    async_request_complete(request);
}


// Time out an AsyncRequest
//
// This is an internal API, the timer callback used to time out long-running
// AsyncRequest. 
//
// This timer callback must run in an SSX_CRITICAL critical section to
// guarantee atomic access to the AsyncRequest object.

static void
async_timeout(void* arg)
{
    AsyncRequest *request = (AsyncRequest*)arg;
    SsxMachineContext ctx;

    ssx_critical_section_enter(SSX_CRITICAL, &ctx);

    // The behavior at timeout depends on the mode.  We'll handle the cases
    // from easiest to hardest.

    if (request->state & ASYNC_REQUEST_CALLBACK_GROUP) {

        // If the request has already queued or is running the callback (which
        // could happen due to interrupt interleaving) then the request is
        // already finished, and it will eventually make a (redundant) call to
        // ssx_timer_cancel() to cancel the timer.  So there's nothing to do.


    } else if (request->state & ASYNC_REQUEST_IDLE_GROUP) {

        // If the request is idle we panic - This can't happen as it would
        // indicate that the timer was not cancelled when the request
        // finished.  

        SSX_PANIC(ASYNC_TIMEOUT_BUG);


    } else if (request->state & ASYNC_REQUEST_QUEUED_GROUP) {

        // If the request is still in the queue then we can simply cancel it,
        // which includes running the callback.

        async_request_dequeue(request, ASYNC_REQUEST_STATE_TIMED_OUT); 


    } else if (request->state & ASYNC_REQUEST_RUNNING_GROUP) {

        // If the request is running on the hardware, then we need to call
        // the async_error_handler() to remove the job and restart the
        // hardware, which includes running the callback.

        async_error_handler(request->queue, ASYNC_REQUEST_STATE_TIMED_OUT);


    } else {

        SSX_PANIC(ASYNC_INVALID_STATE);

    }

    ssx_critical_section_exit(&ctx);
}


// Create (initialize) a generic AsyncRequest
//
// This is an internal API used to initialize generic requests. However this
// API does some error checking, and any errors will be returned by the
// higher-level call.
//
// \retval -ASYNC_INVALID_OBJECT_REQUEST The \a request or \a queue was 
/// null (0).
//
// \retval -ASYNC_INVALID_OPTIONS The \a options argument contains invalid
// options, or more than one callback protocol was selected.
//
// \retval -ASYNC_INVALID_ARGUMENT The \a run_method was null.
//
// \retval -ASYNC_CALLBACK_PROTOCOL_UNSPECIFIED The request includes a
// non-NULL callback, but no protocol for running the callback was specified.

int
async_request_create(AsyncRequest *request,
                     AsyncQueue *queue,
                     AsyncRunMethod run_method,
                     AsyncErrorMethod error_method,
                     SsxInterval timeout,
                     AsyncRequestCallback callback,
                     void *arg,
                     int options)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((request == 0) || (queue == 0), 
                      ASYNC_INVALID_OBJECT_REQUEST);
        SSX_ERROR_IF((options & ~ASYNC_GENERIC_OPTIONS) ||
                     (__builtin_popcount(options & ASYNC_CALLBACK_OPTIONS) > 1),
                     ASYNC_INVALID_OPTIONS);
        SSX_ERROR_IF(run_method == 0, ASYNC_INVALID_ARGUMENT);
        SSX_ERROR_IF((callback != 0) && 
                     ((options & ASYNC_CALLBACK_OPTIONS) == 0),
                     ASYNC_CALLBACK_PROTOCOL_UNSPECIFIED);
    }

    ssx_deque_element_create(&(request->deque));
    request->queue = queue;
    request->run_method = run_method;
    request->error_method = error_method;
    request->timeout = timeout;
    request->state = ASYNC_REQUEST_STATE_INITIALIZED;
    request->callback = callback;
    request->arg = arg;
    request->options = options;
    
    if (request->timeout != SSX_WAIT_FOREVER) {
        ssx_timer_create(&(request->timer), async_timeout, (void*)request);
    }

    if (options & ASYNC_REQUEST_BLOCKING) {
        ssx_semaphore_create(&(request->sem), 0, 1);
    }


    return 0;
}


/// Get timestamps from an AsyncRequest object
///
/// \param request A pointer to an AsyncRequest
///
/// \param start_time A pointer to a location to get the \a start_time of the
/// request, or NULL (0) if this data is not required.
///
/// \param end_time A pointer to a location to get the \a end_time of the
/// request, or NULL (0) is this data is not required.
///
/// \retval 0 The request contains valid timestamps and they have been
/// returned. 
///
/// \retval -ASYNC_INVALID_TIMESTAMPS The caller's timestamps have been
/// updated, but the timestamps are fully or partially invalid.  This could be
/// due to several reasons:
///
/// - The request has never been scheduled
/// - The request has been scheduled but has not completed on the device
/// - For space/time reasons, timestamps are not supported

int
async_request_timestamps_get(AsyncRequest* request,
                             SsxTimebase* start_time,
                             SsxTimebase* end_time)
{
    int rc;
    SsxMachineContext ctx;

    ssx_critical_section_enter(SSX_CRITICAL, &ctx);

    if (start_time) {
        *start_time = request->start_time;
    }
    if (end_time) {
        *end_time = request->end_time;
    }

    if ((request->state & ASYNC_REQUEST_IDLE_GROUP) &&
        (request->state != ASYNC_REQUEST_STATE_INITIALIZED)) {
        rc = 0;
    } else {
        rc = -ASYNC_INVALID_TIMESTAMPS;
    }

    ssx_critical_section_exit(&ctx);

    return rc;
}


/// Compute the latency of an AsyncRequest
///
/// \param request A pointer to an AsyncRequest
///
/// \param latency A pointer to a location to receive the latency (end time -
/// start time) computed from the timestamps of \a request.
///
/// \retval 0 The request contains valid timestamps and they have been
/// returned. 
///
/// \retval -ASYNC_INVALID_TIMESTAMPS The latancy has been computed but may be
/// invalid.  This could be due to several reasons:
///
/// - The request has never been scheduled
/// - The request has been scheduled but has not completed on the device
/// - For space/time reasons, timestamps are not supported

int
async_request_latency(AsyncRequest* request, SsxTimebase* latency)
{
    int rc;
    SsxTimebase start, end;

    rc = async_request_timestamps_get(request, &start, &end);
    *latency = end - start;
    return rc;
}


// Dump an AsyncRequest

void
async_request_printk(AsyncRequest *request)
{
    printk("----------------------------------------\n");
    printk("-- AsyncRequest @@ %p\n", request);
    printk("--   deque            = %p\n", &(request->deque));
    printk("--   start_time       = 0x%016llx\n", request->start_time);
    printk("--   end_time         = 0x%016llx\n", request->end_time);
    printk("--   sem              = %p\n", &(request->sem));
    printk("--   queue            = %p\n", request->queue);
    printk("--   run_method       = %p\n", request->run_method);
    printk("--   error_method     = %p\n", request->error_method);
    printk("--   callback         = %p\n", request->callback);
    printk("--   arg              = %p\n", request->arg);
    printk("--   state            = 0x%02x\n", request->state);
    printk("--   completion_state = 0x%02x\n", request->completion_state);
    printk("--   options          = 0x%04x\n", request->options);
    printk("----------------------------------------\n");
}


////////////////////////////////////////////////////////////////////////////
// Callback Queue
////////////////////////////////////////////////////////////////////////////

SSX_IRQ_FAST2FULL(async_callback_handler, async_callback_handler_full);

// The handler for the asynchronous callback queue
//
// This is a full-mode noncritical interrupt handler.  It is activated to run
// 1) all deferred callbacks, and 2) noncritical callbacks invoked from
// critical interrupt handlers, and 3) Thread-unblock and/or timeout-cancel
// requests that needed to be deferred to a noncritical context. The callback
// is enqueued in the SsxDeque passed as the private argument, and an IPI is
// used to activate this handler.
//
// This handler runs each callback in order.  Since the callback queue may be
// managed by critical interrupt handlers we need to disable critical
// interrupts when popping the next element from the queue.
//
// Deferred callbacks are run with noncritical interrupts \e enabled, similar
// to how timer callbacks are run.
//
// Noncritical callbacks that were deferred here (by being invoked from a
// critical handler) run with interrupts \e disabled, to be consistent with
// the expected environment. Interrupts are then renabled briefly for
// interrupt latency mitigation.
//
// Note that NULL callbacks may be enqueued here but only in the state
// ASYNC_REQUEST_STATE_POSTPROCESSING.

// The handler runs with its own IRQ disabled to avoid infinite interrupt
// loops caused by enabling interrupt preemption. The IRQ status can only be
// cleared inside an SSX_CRITICAL critical section.  For efficiency we only do
// this when we know that no more callbacks are queued.

// Final request completion has been broken out into a generic routine,
// async_request_finalize().  This routine is also called by the PTS
// completion queue handler, which handles its requests ion a slightly
// different way from the other async handlers.

void
async_request_finalize(AsyncRequest* request)
{
    if (request->state == ASYNC_REQUEST_STATE_CALLBACK_QUEUED) {

        request->state = ASYNC_REQUEST_STATE_CALLBACK_RUNNING;

        if (request->options & ASYNC_CALLBACK_DEFERRED) {

            ssx_interrupt_preemption_enable();
            request->callback_rc = request->callback(request->arg);

        } else {

            request->callback_rc = request->callback(request->arg);
            ssx_interrupt_preemption_enable();
        }
        ssx_interrupt_preemption_disable();
    }

    request->state = request->completion_state;

    if (request->timeout != SSX_WAIT_FOREVER) {
        ssx_timer_cancel(&(request->timer));
    }

    if (request->options & ASYNC_REQUEST_BLOCKING) {
        ssx_semaphore_post(&(request->sem));
    }
}


void
async_callback_handler_full(void *arg, SsxIrqId irq, int priority)
{
    SsxMachineContext ctx;
    SsxDeque *queue = (SsxDeque *)arg;
    AsyncRequest *request;

    ssx_irq_disable(irq);

    do {

        ssx_critical_section_enter(SSX_CRITICAL, &ctx);

        request = (AsyncRequest *)ssx_deque_pop_front(queue);
        if (!request) {
            ssx_irq_status_clear(irq);
            break;
        }

        ssx_critical_section_exit(&ctx);

        async_request_finalize(request);

    } while (1);

    ssx_critical_section_exit(&ctx);
    ssx_irq_enable(irq);
}


////////////////////////////////////////////////////////////////////////////
// Initialization
////////////////////////////////////////////////////////////////////////////

// These initialization routines are for boot-time initialization, or
// test-mode reinitialization of interface parameters when the interface is
// known to be idle. They are not robust enough for mid-application
// reset/reprogramming of the asynchronous interfaces in the event of errors.

// For the interrupt setup, whether or not the interrupt is enabled at the
// exit of setup depends on the particular driver being initialized.

void
async_edge_handler_setup(SsxIrqHandler handler,
                         void *arg,
                         SsxIrqId irq,
                         int priority)
{
    ssx_irq_disable(irq);
    ssx_irq_setup(irq,
                  SSX_IRQ_POLARITY_ACTIVE_HIGH,
                  SSX_IRQ_TRIGGER_EDGE_SENSITIVE);
    ssx_irq_handler_set(irq, handler, arg, priority);
    ssx_irq_status_clear(irq);
}


void
async_level_handler_setup(SsxIrqHandler handler,
                          void *arg,
                          SsxIrqId irq,
                          int priority,
                          int polarity)
{
    ssx_irq_disable(irq);
    ssx_irq_setup(irq,
                  polarity,
                  SSX_IRQ_TRIGGER_LEVEL_SENSITIVE);
    ssx_irq_handler_set(irq, handler, arg, priority);
}


void
async_callbacks_initialize(SsxDeque *queue, SsxIrqId irq)
{
    ssx_deque_sentinel_create(queue);
    async_edge_handler_setup(async_callback_handler,
                             (void *)queue,
                             irq, SSX_NONCRITICAL);
    ssx_irq_enable(irq);
}


/// Create all of the PgP asynchronous request structures and install and
/// activate the interrupt handlers.

void
async_initialize()
{
    // This is the callback queue used e.g. when critical interrupts need to
    // run non-critical callbacks.

    async_callbacks_initialize(&G_async_callback_queue, PGP_IRQ_ASYNC_IPI);

    // PORE

    async_pore_initialize(&G_pore_gpe0_queue, ASYNC_ENGINE_PORE_GPE0);
    async_pore_initialize(&G_pore_gpe1_queue, ASYNC_ENGINE_PORE_GPE1);


#if CONFIGURE_PTS
    // PTS

    async_pts_initialize(&G_pts_gpe0_queue, ASYNC_ENGINE_PORE_GPE0);
    async_pts_initialize(&G_pts_gpe1_queue, ASYNC_ENGINE_PORE_GPE1);
#endif

    // BCE

    async_bce_initialize(&G_pba_bcde_queue,
                         ASYNC_ENGINE_BCDE,
                         PGP_IRQ_PBA_BCDE_ATTN);

    async_bce_initialize(&G_pba_bcue_queue,
                         ASYNC_ENGINE_BCUE,
                         PGP_IRQ_PBA_BCUE_ATTN);

    // OCB

    async_ocb_initialize(&(G_ocb_read_queue[0]),
                         ASYNC_ENGINE_OCB_PUSH0,
                         G_ocb_read0_buffer,
                         OCB_READ0_LENGTH,
                         OCB_READ0_PROTOCOL);

    async_ocb_initialize(&(G_ocb_read_queue[1]),
                         ASYNC_ENGINE_OCB_PUSH1,
                         G_ocb_read1_buffer,
                         OCB_READ1_LENGTH,
                         OCB_READ1_PROTOCOL);

    async_ocb_initialize(&(G_ocb_read_queue[2]),
                         ASYNC_ENGINE_OCB_PUSH2,
                         G_ocb_read2_buffer,
                         OCB_READ2_LENGTH,
                         OCB_READ2_PROTOCOL);

    async_ocb_initialize(&(G_ocb_write_queue[0]),
                         ASYNC_ENGINE_OCB_PULL0,
                         G_ocb_write0_buffer,
                         OCB_WRITE0_LENGTH,
                         OCB_WRITE0_PROTOCOL);

    async_ocb_initialize(&(G_ocb_write_queue[1]),
                         ASYNC_ENGINE_OCB_PULL1,
                         G_ocb_write1_buffer,
                         OCB_WRITE1_LENGTH,
                         OCB_WRITE1_PROTOCOL);

    async_ocb_initialize(&(G_ocb_write_queue[2]),
                         ASYNC_ENGINE_OCB_PULL2,
                         G_ocb_write2_buffer,
                         OCB_WRITE2_LENGTH,
                         OCB_WRITE2_PROTOCOL);

    // PBAX

    async_pbax_initialize(&G_pbax_read_queue[0],
                          ASYNC_ENGINE_PBAX_PUSH0,
                          PGP_IRQ_PBA_OCC_PUSH0,
                          G_pbax_read0_buffer,
                          PBAX_READ0_LENGTH,
                          PBAX_READ0_PROTOCOL);

    async_pbax_initialize(&G_pbax_read_queue[1],
                          ASYNC_ENGINE_PBAX_PUSH1,
                          PGP_IRQ_PBA_OCC_PUSH1,
                          G_pbax_read1_buffer,
                          PBAX_READ1_LENGTH,
                          PBAX_READ1_PROTOCOL);
}
