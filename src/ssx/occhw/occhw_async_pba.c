/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/occhw/occhw_async_pba.c $                             */
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
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file occhw_async_pba.c
/// \brief Async device drivers for the PBA block copy engines and PBAX

#include "ssx.h"
#include "occhw_async.h"

////////////////////////////////////////////////////////////////////////////
// Global Data
////////////////////////////////////////////////////////////////////////////

BceQueue G_pba_bcue_queue;
BceQueue G_pba_bcde_queue;

PbaxQueue G_pbax_read_queue[PBAX_QUEUES];


////////////////////////////////////////////////////////////////////////////
// Local Data
////////////////////////////////////////////////////////////////////////////


/// Combined FFDC for all PBA subunits
static PbaUnitFfdc G_pba_ffdc = {{{{{0}}}}};


PBAX_CQ_READ_BUFFER(G_pbax_read0_buffer, PBAX_READ0_LENGTH);
PBAX_CQ_READ_BUFFER(G_pbax_read1_buffer, PBAX_READ1_LENGTH);

// The generic driver code stores an engine-specfic engine ID in the
// queue. Here arrays are set up that contain the OCI control register
// addresses for an engine indexed by the queue-specific value.
//
/// \todo Once the designers actually define register addresses, see about
/// modifying these tables to be macros instead.
///
/// \todo Actually, why don't we just store this data directly in the queue?

/// PBA BCDE/BCUE PowerBus BAR[0..31]
static const SsxAddress G_bce_pbadr[BCE_ENGINES] =
    {PBA_BCDE_PBADR, PBA_BCUE_PBADR};

/// PBA BCDE/BCUE OCI BAR
static const SsxAddress G_bce_ocibar[BCE_ENGINES] =
    {PBA_BCDE_OCIBAR, PBA_BCUE_OCIBAR};

/// PBA BCDE/BCUE SET register
static const SsxAddress G_bce_set[BCE_ENGINES] =
    {PBA_BCDE_SET, PBA_BCUE_SET};

/// PBA BCDE/BCUE Control register
static const SsxAddress G_bce_ctl[BCE_ENGINES] =
    {PBA_BCDE_CTL, PBA_BCUE_CTL};

/// PBA BCDE/BCUE Status register
static const SsxAddress G_bce_stat[BCE_ENGINES] =
    {PBA_BCDE_STAT, PBA_BCUE_STAT};

/// PBAX Push Queue Control/Status Register addresses
static const SsxAddress G_pba_xshcsn[PBAX_ENGINES] =
    {PBA_XSHCS0, PBA_XSHCS1};

/// PBAX Push Queue Base Register addresses
static const SsxAddress G_pba_xshbrn[PBAX_ENGINES] =
    {PBA_XSHBR0, PBA_XSHBR1};

/// PBAX Push Queue Increment Register addresses
static const SsxAddress G_pba_xshincn[PBAX_ENGINES] =
    {PBA_XSHINC0, PBA_XSHINC1};


////////////////////////////////////////////////////////////////////////////
// PBA FFDC Structures
////////////////////////////////////////////////////////////////////////////

// NB: Collection of FFDC for PBA will be very time consuming due to the large
// amount of information required to fully diagnose any problem.  This will be
// done in a critical interrupt so it may have some impact on micro-timescale
// realtime operations.

// Collect PBA common FFDC
//
// Most of this is just collection of all of the setup registers required to
// diagnose problems with the bridge and block copy engines.  Not all of this
// data is required for PBAX errors but we collect it anyway.
//
// FFDC is only collected for the first error, until the error flag is reset.

static void
pba_common_ffdc(PbaCommonFfdc* ffdc)
{
    int i;

    if (ffdc->error == 0) {

        oci_ffdc(&(ffdc->oci_ffdc), OCI_MASTER_ID_PBA);

        ffdc->mode.value = in64(PBA_MODE);
    
        for (i = 0; i < PBA_READ_BUFFERS; i++) {
            getscom(PBA_RBUFVALN(i), &(ffdc->rbufval[i].value));
        }

        for (i = 0; i < PBA_WRITE_BUFFERS; i++) {
            getscom(PBA_WBUFVALN(i), &(ffdc->wbufval[i].value));
        }

        for (i = 0; i < PBA_BARS; i++) {
            getscom(PBA_BARN(i), &(ffdc->bar[i].value));
            getscom(PBA_BARMSKN(i), &(ffdc->barmsk[i].value));
        }

        getscom(PBA_FIR, &(ffdc->fir.value));
        getscom(PBA_ERRPT0, &(ffdc->errpt0.value));
        getscom(PBA_ERRPT1, &(ffdc->errpt1.value));
        getscom(PBA_ERRPT2, &(ffdc->errpt2.value));

        ffdc->error = 1;
    }
}


// Collect FFDC for generic PBA bridge errors
//
// This extends the common collection with PBA slave control/status
// information.

static void
pba_bridge_ffdc(PbaBridgeFfdc* ffdc)
{
    int i;

    if (ffdc->common.error == 0) {

        pba_common_ffdc(&(ffdc->common));

        ffdc->slvrst.value = in64(PBA_SLVRST);

        for (i = 0; i > PBA_SLAVES; i++) {
            ffdc->slvctl[i].value = in64(PBA_SLVCTLN(i));
        }
    }
}


// Collect FFDC for a particular Block Copy Engine
//
// The engine ID here is either 0 (BCDE) or 1 (BCUE)

static void
bce_ffdc(BceFfdc* ffdc, int engine)
{
    if (ffdc->common.error == 0) {

        pba_common_ffdc(&(ffdc->common));

        ffdc->ctl.value = in64(G_bce_ctl[engine]);
        ffdc->set.value = in64(G_bce_set[engine]);
        ffdc->pbadr.value = in64(G_bce_pbadr[engine]);
        ffdc->stat.value = in64(G_bce_stat[engine]);
    }
}


// Collect FFDC for PBAX send

static void
pbax_send_ffdc(PbaxSendFfdc* ffdc)
{
    if (ffdc->common.error == 0) {

        pba_common_ffdc(&(ffdc->common));
    
        ffdc->xcfg.value = in64(PBA_XCFG);
        ffdc->xsndtx.value = in64(PBA_XSNDTX);
        ffdc->xsndstat.value = in64(PBA_XSNDSTAT);
    }
}


// Collect FFDC for PBAX receive
//
// The drivers currently do not distinguish errors between the two receive
// queues as the hardware design does not provide a clean separation.

static void
pbax_receive_ffdc(PbaxReceiveFfdc* ffdc)
{
    int i;

    if (ffdc->common.error == 0) {

        pba_common_ffdc(&(ffdc->common));
    
        ffdc->xcfg.value = in64(PBA_XCFG);
        ffdc->xrcvstat.value = in64(PBA_XRCVSTAT);
    
        for (i = 0; i < PBAX_ENGINES; i++) {
            ffdc->xshbrn[i].value = in64(PBA_XSHBRN(i));
            ffdc->xshcsn[i].value = in64(PBA_XSHCSN(i));
        }
    }
}


////////////////////////////////////////////////////////////////////////////
// BceRequest
////////////////////////////////////////////////////////////////////////////

// Start a request running on a PBA Block Copy Engine
//
// \param queue A BceQueue upcast to an AsyncQueue
//
// \param request A BceRequest upcast to an AsyncRequest
//
// This routine takes a Bce objects cast to Async objects so that it can be
// called from the generic Async interrupt handler.
//
// This is an internal API. This routine must be called from a context where
// critical interrupts are disabled. Prior to this call, scheduling code will
// have installed the new request as the \a current request of the queue, and
// marked both the queue and the request as running.
//
// The PBA driver handles requests that require multiple runs of the PBA to
// complete the request.  Starting a PBA job requires setting up 3 registers,
// hitting the "go" bit, and computing the amount of work remaining.
//
// Note that PBA will signal an error and lock up the system if the START bit
// is written while a BC-engine is running.

// Recall that the engines have identical control structures with identical
// relative offsets between registers.  So we use BCDE offsets and BCDE
// register layout structures, but they work for BCUE as well.

static int
bce_async_run_method(AsyncRequest *async_request)
{
    BceQueue *queue = (BceQueue *)(async_request->queue);
    BceRequest *request = (BceRequest *)async_request;
    int rc, engine;
    pba_bcde_pbadr_t pbadr;
    pba_bcde_set_t set;
    pba_bcde_ctl_t ctl;
    size_t to_write;


    if (request->remaining == 0) {

        rc = -ASYNC_REQUEST_COMPLETE;

    } else {

        to_write = MIN(request->remaining, PBA_BCE_SIZE_MAX);

        // Create the address offset register.  The PowerBus offset is the
        // cache-line address of the stored offset (ex the OCI region bits).

        pbadr.value = 0;

        pbadr.fields.pb_offset =
            (request->next_bridge_address & 0x3FFFFFFF) /
            POWERBUS_CACHE_LINE_SIZE;

        pbadr.fields.extaddr =
            request->extended_address.fields.extended_address;

        // Create the setup register

        set.value = 0;

        set.fields.copy_length = to_write / POWERBUS_CACHE_LINE_SIZE;

        // Set the START bit

        ctl.value = PBA_BCDE_CTL_START;

        // Start the BCE

        engine = queue->engine;
        out64(G_bce_pbadr[engine], pbadr.value);
        out32(G_bce_ocibar[engine],      request->next_oci_address);
        out32(G_bce_set[engine],         set.words.high_order);
        out32(G_bce_ctl[engine],         ctl.words.high_order);

        // Update the work remaining to be done.

        request->remaining -= to_write;
        if (request->remaining != 0) {
            request->next_bridge_address += to_write;
            request->next_oci_address += to_write;
        }

        rc = 0;
    }

    return rc;
}


// The async error method for Block Copy Engines
//
// Collect FFDC and stop the engine.  The return of -1 will disable the
// channel associated with the request.  The application will need to decide
// whether to restart the queue.

static int
bce_async_error_method(AsyncRequest *request)
{
    BceQueue *queue = (BceQueue*)(request->queue);
    int engine = queue->engine;

    if (engine == BCE_ENGINE_BCDE) {

        bce_ffdc(&(G_pba_ffdc.bcde), engine); 
        out32(PBA_BCDE_CTL, PBA_BCDE_CTL_STOP >> 32);

    } else {

        bce_ffdc(&(G_pba_ffdc.bcue), engine); 
        out32(PBA_BCUE_CTL, PBA_BCUE_CTL_STOP >> 32);

    }

    return -1;
}


/// Create a request for a PBA Block Copy engine
///
/// \param request An uninitialized or otherwise idle BceRequest.
///
/// \param queue An initialized BceQueue.
///
/// \param bridge_address The 32-bit bridge address that is translated to
/// a PowerBus address associated with the copy.  This address must be a
/// PowerBus cache (128-byte) aligned address.
///
/// \param oci_address  The 32-bit OCI address associated with the copy.  This
/// address must be 128-byte aligned.
///
/// \param bytes The number of bytes to move.  This must be a multiple of 128,
/// and the size must not obviously overflow the OCI address or the bridge
/// address.
///
/// \param timeout If not specified as SSX_WAIT_FOREVER, then this request
/// will be governed by a private watchdog timer that will cancel a queued job
/// or kill a running job if the hardware operation does not complete before
/// it times out.
///
/// \param callback The callback to execute when the PBA copy completes, or
/// NULL (0) to indicate no callback.
///
/// \param arg The parameter to the callback routine; ignored if the \a
/// callback is NULL.
///
/// \param options Options to control request priority and callback context.
///
/// Note that the setup for the BC engines includes an extended address and a
/// PowerBus maximum priority.  This API sets these to
/// the default values in the request.  If non-default values are required,
/// they will need to be installed in the structure explicitly before
/// scheduling.
///
/// This routine has no way to know if the BceRequest structure is currently
/// in use, so this API should only be called on uninitialized or otherwise
/// idle GpeRequest structures.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_OBJECT_BCE_REQUEST The \a request was NULL (0), 
/// or the \a queue was NULL (0) or not a BceQueue.
///
/// See async_request_create() for other errors that may be returned by this
/// call.

int
bce_request_create(BceRequest *request,
                   BceQueue *queue,
                   uint32_t bridge_address,
                   uint32_t oci_address,
                   size_t bytes,
                   SsxInterval timeout,
                   AsyncRequestCallback callback,
                   void *arg,
                   int options)
{
    int rc;
    AsyncQueue *async_queue = (AsyncQueue *)queue;


    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((request == 0) ||
                     (queue == 0)   ||
                     !(async_queue->engine & ASYNC_ENGINE_BCE),
                     ASYNC_INVALID_OBJECT_BCE_REQUEST);
    }

    rc = async_request_create(&(request->request),
                              async_queue,
                              bce_async_run_method,
                              bce_async_error_method,
                              timeout,
                              callback,
                              arg,
                              options);

    request->bridge_address = bridge_address;
    request->oci_address = oci_address;
    request->bytes = bytes;
    request->extended_address.value = 0;

    return rc;
}


////////////////////////////////////////////////////////////////////////////
// BceQueue
////////////////////////////////////////////////////////////////////////////

/// Create (initialize) a BceQueue
///
/// \param queue An uninitialized of otherwise idle BceQueue
///
/// \param engine Either ASYNC_ENGINE_PBA_BCDE or ASYNC_ENGINE_PBA_BCUE
///
/// This API initializes the BceQueue structure based on the engine
/// provided. There is really no special hardware initialization required as
/// the BC-engines are pretty simple data movers.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_OBJECT_BCE_QUEUE The \a queue was NULL (0).
///
/// \retval -ASYNC_INVALID_ENGINE_BCE The \a engine is not a PBA engine.

int
bce_queue_create(BceQueue *queue,
                 int engine)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(queue == 0, ASYNC_INVALID_OBJECT_BCE_QUEUE);
    }

    switch (engine) {

    case ASYNC_ENGINE_BCDE:
        async_queue_create(&(queue->queue), engine);
        queue->engine = BCE_ENGINE_BCDE;
        break;

    case ASYNC_ENGINE_BCUE:
        async_queue_create(&(queue->queue), engine);
        queue->engine = BCE_ENGINE_BCUE;
        break;

    default:
        if (SSX_ERROR_CHECK_API) {
            SSX_ERROR(ASYNC_INVALID_ENGINE_BCE);
        }
        break;
    }

    return 0;
}


/// Schedule a request on a PBA block-copy engine
///
/// Note : As long as the BceRequest is idle, the application is free to
/// directly change the \a bridge_address, \a oci_address and \a bytes fields to
/// read/write different numbers of bytes to/from different locations the next
/// time the request is scheduled.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_ARGUMENT_BCE_SCHEDULE Either the \a bridge_address 
/// is not 128-byte aligned, or the OCI address is not 128-byte aligned, or the
/// number of bytes is not a multiple of 128.
///
/// See async_request_schedule() for documentation of other errors

int
bce_request_schedule(BceRequest *request)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((request->bridge_address % POWERBUS_CACHE_LINE_SIZE) ||
                     (request->oci_address % POWERBUS_CACHE_LINE_SIZE)    ||
                     (request->bytes % POWERBUS_CACHE_LINE_SIZE),
                     ASYNC_INVALID_ARGUMENT_BCE_SCHEDULE);
    }

    request->next_bridge_address = request->bridge_address;
    request->next_oci_address = request->oci_address;
    request->remaining = request->bytes;

    return async_request_schedule((AsyncRequest *)request);
}


// The interrupt handler for asynchronous PBA Block Copy requests
//
// BC-engine 'done' interrupts are level-sensitive active high, so they need
// to be cleared in the PBA by writing a 0 to the CTL register.  We also get
// the PBA 'done' even when jobs are killed, but the cancel/kill code is
// responsible for setting the \a completion_state of the request.
//
// We only go to the generic handler once all data has been transferred.

SSX_IRQ_FAST2FULL(bce_async_handler, bce_async_handler_full);

void
bce_async_handler_full(void *arg, SsxIrqId irq, int priority)
{
    AsyncQueue *async_queue = (AsyncQueue *)arg;
    AsyncRequest *async_current = (AsyncRequest *)async_queue->current;
    BceQueue *queue = (BceQueue *)async_queue;

    out32(G_bce_ctl[queue->engine], 0);

    if (SSX_ERROR_CHECK_KERNEL && (async_current == 0)) {
        SSX_PANIC(ASYNC_PHANTOM_INTERRUPT_BCE);
    }

    if (async_current->run_method(async_current) == -ASYNC_REQUEST_COMPLETE) {
        async_handler(async_queue);
    }
}


////////////////////////////////////////////////////////////////////////////
// PbaxRequest
////////////////////////////////////////////////////////////////////////////

/// Non-blocking read from a PBAX PUSH (read) queue
///
/// \param queue The target PbaxQueue
///
/// \param buf The caller's data buffer to receive the read data
///
/// \param bytes The maximum number of bytes to read.  This value should be an
/// even multiple of 8, as this API always reads multiples of 8 bytes.
///
/// \param read The number of bytes actually copied from the device buffer to
/// the caller's buffer.  This may be returned as any value from 0 to \a
/// bytes in multiples of 8 bytes.
///
/// pbax_read() implements a non-blocking copy of data from a PBAX read (PUSH)
/// queue data area to the caller's data area, with the side effect of
/// advancing the hardware queue pointers.  pbax_read() does not implement
/// locking, critical sections or any other type of synchronization for access
/// to the PBAX queue data.  
///
/// pbax_read() returns the error code -ASYNC_PBAX_ERROR_OLD or 
/// -ASYNC_PBAX_ERROR_NEW if PBAX receive error status is asserted.  
/// The error return code may be disjoint from the actual
/// read: If data is available it may be copied to the caller's buffer, but
/// this data should always be considered corrupted in the event of an error
/// return code.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_ARGUMENT_PBAX_READ The number of \a bytes is not 
/// an even multiple of 8.
///
/// \retval -ASYNC_PBAX_ERROR_OLD The PBA shows pre-existing error status
///
/// \retval -ASYNC_PBAX_ERROR_NEW The PBA shows current error status
///
/// \todo Once the HW design is finalized, we might consider modifying the
/// error behavior to only return an error status if the receive error was
/// associated with this particular queue.

int
pbax_read(PbaxQueue* queue, void* buf, size_t bytes, size_t* read)
{
    pba_xshcsn_t csr;
    pba_xrcvstat_t rsr;
    unsigned qlen, read_ptr, write_ptr, to_read;
    uint64_t *pcq, *pbuf;
    int rc;

    do {

        rc = 0;
        *read = 0;

        // If pre-existing errors exist then immediately abort the read.

        rsr.words.high_order = in32(PBA_XRCVSTAT);
        if (rsr.fields.rcv_error) {
            pbax_receive_ffdc(&(G_pba_ffdc.pbax_receive));
            rc = -ASYNC_PBAX_ERROR_OLD;
            break;
        }

        if (bytes % 8) {
            rc = -ASYNC_INVALID_ARGUMENT_PBAX_READ;
            break;
        }

        // Determine the number of doubleword entries remaining to be read in
        // the queue. The driver does not keep state, but instead reinterprets
        // the control/status register each time the read method is called.

        // This may be confusing - remember that 'push' is from the PowerBus
        // perspective - here we use 'read' from OCC's perspective.

        csr.words.high_order = in32(G_pba_xshcsn[queue->engine]);

        qlen = csr.fields.push_length + 1;
        read_ptr = csr.fields.push_read_ptr;

        if (csr.fields.push_empty) {
            break;

        } else if (csr.fields.push_full) {
            to_read = qlen;

        } else {
            write_ptr = csr.fields.push_write_ptr;
            if (read_ptr > write_ptr) {
                to_read = qlen - (read_ptr - write_ptr);
            } else {
                to_read = write_ptr - read_ptr;
            }
        }

        // Copy the data from the CQ memory area.  For simplicty of dealing with
        // cache management each doubleword invokes a line invalidate before
        // refetching the fresh data from memory. Alignment requirements enforced
        // on the data buffer guarantee the buffers are cache-line aligned and
        // each doubleword is fully contained in a single D-cache line.
        //
        // Here the code models the evolution of the read_ptr as each datum is
        // copied from the queue.

        pbuf = (uint64_t*) buf;
        while (bytes && to_read--) {

            read_ptr++;
            if (read_ptr == qlen) {
                read_ptr = 0;
            }
            pcq = queue->cq_base + read_ptr;

            dcache_invalidate_line(pcq);
            *pbuf++ = *pcq;
            out32(G_pba_xshincn[queue->engine], 0);        

            bytes -= 8;
            *read += 8;
        }
    } while(0);

    // Check for errors that occurred during the read

    rsr.words.high_order = in32(PBA_XRCVSTAT);
    if (rsr.fields.rcv_error) {
        pbax_receive_ffdc(&(G_pba_ffdc.pbax_receive));
        rc = -ASYNC_PBAX_ERROR_NEW;
    }

    return rc;
}


// This is the internal 'run method' for reading through a PBAX circular
// queue. The run method simply enables the IRQ.  The interrupt handler reads
// data from the queue and leaves the interrupt enabled until the read is
// satisfied.

int
pbax_read_method(AsyncRequest *async_request)
{
    PbaxRequest *request = (PbaxRequest*)async_request;
    PbaxQueue *queue = (PbaxQueue*)(async_request->queue);
    int rc;

    if (request->bytes == 0) {
        rc = -ASYNC_REQUEST_COMPLETE;
    } else {
        ssx_irq_enable(queue->irq);
        rc = 0;
    }
    return rc;
}


// The async error method for PBAX
//
// Collect FFDC.

static int
pbax_async_error_method(AsyncRequest *request)
{
    pbax_receive_ffdc(&(G_pba_ffdc.pbax_receive));
    return -1;
}
    

/// Create a request for a PBAX read queue
///
/// \param request An uninitialized or otherwise idle PbaxRequest.
///
/// \param queue An async queue for a PBAX read buffer.
///
/// \param data A pointer to the data to where the data should be placed.
///
/// \param bytes The (maximum) number of bytes of data to move.  The PBAX read
/// queues always move multiples of 8 bytes, and the number of bytes must be a
/// multiple of 8.  Higher-level abstractions will have to take care of cases
/// where the "actual" numbers of bytes are not multiples of 8.
///
/// \param timeout If not specified as SSX_WAIT_FOREVER, then this request
/// will be governed by a private watchdog timer that will cancel a queued job
/// or kill a running job if the hardware operation does not complete before
/// it times out.
///
/// \param callback The callback to execute when the read completes, or
/// NULL (0) to indicate no callback.
///
/// \param arg The parameter to the callback routine; ignored if the \a
/// callback is NULL.
///
/// \param options Options to control request priority, callback context and
/// blocking. See the documentation for the PbaxRequest object for information
/// on the special ASYNC_NONBLOCKING_READ option.
///
/// This routine has no way to know if the PbaxRequest structure is currently
/// in use, so this API should only be called on uninitialized or otherwise
/// idle PbaxRequest structures.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_OBJECT_PBAX_REQUEST The \a request or \a queue 
/// were NULL (0), or the \a queue is not an initialized PbaxQueue.
///
/// \retval -ASYNC_INVALID_ARGUMENT_PBAX_REQUEST The \a data pointer is 
/// NULL (0), or the number of bytes is not a multiple of 8.
///
/// See async_request_create() for other errors that may be returned by this
/// call.

int
pbax_request_create(PbaxRequest* request,
                    PbaxQueue* queue,
                    uint64_t* data,
                    size_t bytes,
                    SsxInterval timeout,
                    AsyncRequestCallback callback,
                    void* arg,
                    int options)
{
    int rc;
    AsyncQueue* async_queue = (AsyncQueue*)queue;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((request == 0) ||
                     (queue == 0)   ||
                     !(async_queue->engine & ASYNC_ENGINE_PBAX),
                     ASYNC_INVALID_OBJECT_PBAX_REQUEST);
        SSX_ERROR_IF((data == 0) ||
                     (bytes % 8),
                     ASYNC_INVALID_ARGUMENT_PBAX_REQUEST);
    }

    rc = async_request_create(&(request->request),
                              async_queue,
                              pbax_read_method,
                              pbax_async_error_method,
                              timeout,
                              callback,
                              arg,
                              options);

    request->data = data;
    request->bytes = bytes;

    return rc;
}


/// Schedule a request on a PBAX read queue
///
/// Note : As long as the PbaxRequest is idle, the application is free to
/// directly change the \a data and \a bytes fields to read different numbers of
/// bytes into different locations the next time the request is scheduled.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_ARGUMENT_PBAX_SCHEDULE The number of \a bytes 
/// currently requested is not a multiple of 8.
///
/// See async_request_schedule() for documentation of other errors

int
pbax_request_schedule(PbaxRequest *request)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((request->bytes % 8),ASYNC_INVALID_ARGUMENT_PBAX_SCHEDULE);
    }

    request->current = request->data;
    request->remaining = request->bytes;

    return async_request_schedule((AsyncRequest *)request);
}


////////////////////////////////////////////////////////////////////////////
// PbaxQueue
////////////////////////////////////////////////////////////////////////////

/// Disable a PBAX circular PUSH (read) queue
///
/// \param queue An initialized PbaxQueue object.
///
/// Disable the PBAX recieve mechanism for a particular PBAX receive queue.
/// Interrupts are disabled, and any data managed by the queue is effectively
/// lost. 
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_OBJECT_PBAX_DISABLE The \a queue is NULL (0) 
/// or otherwise invalid. 

int
pbax_queue_disable(PbaxQueue *queue)
{
    pba_xshcsn_t cs;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(queue == 0, ASYNC_INVALID_OBJECT_PBAX_DISABLE);
    }

    ssx_irq_disable(queue->irq);

    cs.value = 0;
    out32(G_pba_xshcsn[queue->engine], cs.value);

    return 0;
}


/// Enable a PBAX circular PUSH (read) queue
///
/// \param queue An initialized PbaxQueue object.
///
/// This API reprograms the queue hardware in accordance with the values
/// present in the \a queue structure, and enables the queue to accept data.
/// Any previous record of data present in the queue will be lost.  The queue
/// interrupt is also disabled by this API.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_OBJECT_PBAX_DISABLE The \a queue is NULL (0) 
/// or otherwise invalid. 

int
pbax_queue_enable(PbaxQueue *queue)
{
    int rc;
    pba_xshcsn_t cs;

    rc = pbax_queue_disable(queue);

    if (!rc) {

        // Reinitialize the data buffer base address register and
        // reprogram/re-enable the queue.

        out32(G_pba_xshbrn[queue->engine], (uint32_t)(queue->cq_base));

        cs.value = 0;

        if (queue->protocol == PBAX_INTERRUPT_PROTOCOL_LAZY) {
            cs.fields.push_intr_action = PBAX_INTR_ACTION_FULL;
        } else {
            cs.fields.push_intr_action = PBAX_INTR_ACTION_NOT_EMPTY;
        }

        cs.fields.push_length = queue->cq_entries - 1;
        cs.fields.push_enable = 1;

        out32(G_pba_xshcsn[queue->engine], cs.words.high_order);
    }
    return 0;
}


/// Create (initialize) a PbaxQueue abstraction
///
/// \param queue An uninitialized or otherwise idle PbaxQueue
///
/// \param engine A valid PBAX engine id
///
/// \param cq_base The base address of the circular queue data area for the
/// queue. This address must be aligned to the next higher power-of-two of the
/// buffer size, with a minimum alignment of a cache line.
///
/// \param cq_entries The number of 8-byte entries in the queue
///
/// \param protocol The interrupt protocol, either PBAX_PUSH_PROTOCOL_LAZY or
/// PBAX_PUSH_PROTOCOL_AGGRESSIVE.  Lazy means that read queues only interrupt
/// when full, and agressive means that read queues interrupt whenever they
/// are not empty.  In general the lazy read protocol will only work for 1)
/// queues of length 1, where lazy == aggressive, and 2) protocols where a
/// known fixed number of 8-byte entries is always expected to be received.
///
/// This API simply creates the PbaxQueue abstraction in the software space;
/// The API does not modify any harwdare state.  To enable/disable the PBAX
/// hardware read queue use pbax_queue_enable().
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_OBJECT_PBAX_QUEUE The \a queue was NULL (0).
///
/// \retval -ASYNC_INVALID_ARGUMENT_PBAX_QUEUE The \a cq_base is not properly 
/// aligned, or the \a cq_length is invalid, or the \a protocol is invalid.
///
/// \retval -ASYNC_INVALID_ENGINE_PBAX The \a engine is not an PBAX engine.
///
/// Other errors may be returned by async_queue_create().

int
pbax_queue_create(PbaxQueue *queue,
                  int engine,
                  uint64_t *cq_base,
                  size_t cq_entries,
                  int protocol)
{
    AsyncQueue *async_queue = (AsyncQueue *)queue;

    if (SSX_ERROR_CHECK_API) {
        uint32_t align_mask = 
            POW2_32(MAX(CEILING_LOG2(cq_entries * 8), 
                        LOG_CACHE_LINE_SIZE)) - 1; 

        SSX_ERROR_IF(queue == 0, ASYNC_INVALID_OBJECT_PBAX_QUEUE);
        SSX_ERROR_IF((((uint32_t)(cq_base) & align_mask) != 0) ||
                     (cq_entries < PBAX_PUSH_LENGTH_MIN) ||
                     (cq_entries > PBAX_PUSH_LENGTH_MAX) ||
                     ((protocol != PBAX_INTERRUPT_PROTOCOL_LAZY) &&
                      (protocol != PBAX_INTERRUPT_PROTOCOL_AGGRESSIVE)),
                     ASYNC_INVALID_ARGUMENT_PBAX_QUEUE);
    }

    queue->cq_base = cq_base;
    queue->cq_entries = cq_entries;
    queue->protocol = protocol;

    switch (engine) {

    case ASYNC_ENGINE_PBAX_PUSH0:
        queue->irq = OCCHW_IRQ_PBAX_OCC_PUSH0;
        queue->engine = PBAX_ENGINE_PUSH0;
        break;

    case ASYNC_ENGINE_PBAX_PUSH1:
        queue->irq = OCCHW_IRQ_PBAX_OCC_PUSH1;
        queue->engine = PBAX_ENGINE_PUSH1;
        break;

    default:
        if (SSX_ERROR_CHECK_API) {
            SSX_ERROR_IF(1, ASYNC_INVALID_ENGINE_PBAX);
        }
    }

    async_queue_create(async_queue, engine);

    return 0;
}


// The interrupt handler for asynchronous PBAX requests
//
// The circular buffer interupts are level sensitive, active high.  There is
// really no way to 'clear' them as they indicate a permanent status - so
// instead they need to be enabled and disabled. This is done by the read
// method. (Note that one could make them edge-triggered, but in general
// device drivers [including this one] would not work correctly if they were
// programmed that way.)
//
// This interrupt handler can process up to 256 bytes at once, plus the
// overhead of scheduling the next job when this one completes.  If interrupt
// latency becomes a problem then this process could be run with interrupt
// preemption enabled.

SSX_IRQ_FAST2FULL(pbax_async_handler, pbax_async_handler_full);

void
pbax_async_handler_full(void *arg, SsxIrqId irq, int priority)
{
    AsyncQueue* async_queue = (AsyncQueue*)arg;
    PbaxQueue* queue = (PbaxQueue*)async_queue;
    PbaxRequest* request = (PbaxRequest*)(async_queue->current);
    size_t read;
    int rc;

    if (SSX_ERROR_CHECK_KERNEL && (request == 0)) {
        SSX_PANIC(ASYNC_PHANTOM_INTERRUPT_PBAX);
    }

    rc = pbax_read(queue, request->current, request->remaining, &read);

    if (rc) {

        ssx_irq_disable(queue->irq);
        async_error_handler(async_queue, ASYNC_REQUEST_STATE_FAILED);

    } else if (read == request->remaining) {

        ssx_irq_disable(queue->irq);
        async_handler(async_queue);

    } else {

        request->current += (read / 8);
        request->remaining -= read;
    }
}


// The interrupt handler for the PBA error interrupt.
//
// There is one error interrupt that covers all PBA function - the generic
// bridge, block copy engines and PBAX buffers.  The PBA error pulses whenever
// new error bits are logged in the PBA FIR, so it is possible for OCC to log
// errors independently for the bridge, BCE and PBAX.
//
// When the PBA error interrupt fires we try to determine which unit is
// responsible for the error and take appropriate action.  The analysis is
// currently not very deep, however it is not clear whether it is worth the
// time and code space required to do more than this.
//
// - If the error appears to be associated with either a BCDE or BCUE job
// running as part of the async mechanism then we let the
// async_error_handler() mechanism operate.  As a side effect the indicated
// queue will be disabled. Note that further analysis (and storage space)
// might allow jobs to be restarted on that engine, but this is currently not
// done.
//
// - If the error appears to be associated with a PBAX send then the PBAX send
// mechanism is already effectively disabled by the fact that the send status
// register shows an error. FFDC is simply collected in this case.
//
// - If the error appears to be associated with a PBAX receive mechanism then
// both receive queues are stopped. The receive may or may not be using the
// job queues (the application may be polling using pbax_read()).
//
// If the error is due to the direct bridge we collect FFDC, but can't really
// do anything else.  Since OCC will not use the PBA bridge at run time
// (except for lab-mode applications) this error is due either to a PORE
// engine or FSP using the dedicated trusted channel to mainstore.
//
// This code is assumed to be activated as a critical interrupt.

/// \todo What to do for generic PBA errors?

SSX_IRQ_FAST2FULL(pba_error_handler, pba_error_handler_full);

void
pba_error_handler_full(void *arg, SsxIrqId irq, int priority)
{
    pba_fir_t fir;
    pba_bcde_stat_t bcde_stat;
    pba_bcue_stat_t bcue_stat;
    pba_xsndstat_t xsndstat;
    pba_xrcvstat_t xrcvstat;
    int channel;
    AsyncQueue* queue;

    ssx_irq_status_clear(irq);

    getscom(PBA_FIR, &(fir.value));
    bcde_stat.words.high_order = in32(PBA_BCDE_STAT);
    bcue_stat.words.high_order = in32(PBA_BCUE_STAT);
    xsndstat.words.high_order = in32(PBA_XSNDSTAT);
    xrcvstat.words.high_order = in32(PBA_XRCVSTAT);

    queue = (AsyncQueue*)(&G_pba_bcde_queue);
    if (bcde_stat.fields.error &&
        (queue->state == ASYNC_QUEUE_STATE_RUNNING)) {
        async_error_handler(queue, ASYNC_REQUEST_STATE_FAILED);
    }

    queue = (AsyncQueue*)(&G_pba_bcue_queue);
    if (bcue_stat.fields.error && 
        (queue->state == ASYNC_QUEUE_STATE_RUNNING)) {
        async_error_handler(queue, ASYNC_REQUEST_STATE_FAILED);
    }

    if (xsndstat.fields.snd_error) {
        pbax_send_ffdc(&G_pba_ffdc.pbax_send);
    }

    if (xrcvstat.fields.rcv_error) {
        for (channel = 0; channel < PBAX_CHANNELS; channel++) {
            queue = (AsyncQueue*)(&G_pbax_read_queue[channel]);
            if (queue->state == ASYNC_REQUEST_STATE_RUNNING) {
                async_error_handler(queue, ASYNC_REQUEST_STATE_FAILED);
            } else {
                pbax_receive_ffdc(&G_pba_ffdc.pbax_receive);
            }
        }
    }

    // Any FIR bits not already attributable to previously handled errors are
    // assumed to be due to the generic bridge.

    if (fir.value & 
        (
         PBA_FIR_OCI_APAR_ERR ||
         PBA_FIR_PB_RDADRERR_FW ||
         PBA_FIR_PB_RDDATATO_FW ||
         PBA_FIR_PB_SUE_FW ||
         PBA_FIR_PB_UE_FW ||
         PBA_FIR_PB_CE_FW ||
         PBA_FIR_OCI_SLAVE_INIT ||
         PBA_FIR_OCI_WRPAR_ERR ||
         PBA_FIR_OCI_REREQTO ||
         PBA_FIR_PB_UNEXPCRESP ||
         PBA_FIR_PB_UNEXPDATA ||
         PBA_FIR_PB_PARITY_ERR ||
         PBA_FIR_PB_WRADRERR_FW ||
         PBA_FIR_PB_BADCRESP ||
         PBA_FIR_PB_ACKDEAD_FW ||
         PBA_FIR_PB_CRESPTO ||
         // PBA_FIR_BCUE_SETUP_ERR ||
         // PBA_FIR_BCUE_PB_ACK_DEAD ||
         // PBA_FIR_BCUE_PB_ADRERR ||
         // PBA_FIR_BCUE_OCI_DATAERR ||
         // PBA_FIR_BCDE_SETUP_ERR ||
         // PBA_FIR_BCDE_PB_ACK_DEAD ||
         // PBA_FIR_BCDE_PB_ADRERR ||
         // PBA_FIR_BCDE_RDDATATO_ERR ||
         // PBA_FIR_BCDE_SUE_ERR ||
         // PBA_FIR_BCDE_UE_ERR ||
         // PBA_FIR_BCDE_CE ||
         // PBA_FIR_BCDE_OCI_DATAERR ||
         PBA_FIR_INTERNAL_ERR ||
         PBA_FIR_ILLEGAL_CACHE_OP ||
         PBA_FIR_OCI_BAD_REG_ADDR ||
         // PBA_FIR_AXPUSH_WRERR ||
         // PBA_FIR_AXRCV_DLO_ERR ||
         // PBA_FIR_AXRCV_DLO_TO ||
         // PBA_FIR_AXRCV_RSVDATA_TO ||
         // PBA_FIR_AXFLOW_ERR ||
         // PBA_FIR_AXSND_DHI_RTYTO ||
         // PBA_FIR_AXSND_DLO_RTYTO ||
         // PBA_FIR_AXSND_RSVTO ||
         // PBA_FIR_AXSND_RSVERR ||
         PBA_FIR_FIR_PARITY_ERR ||
         PBA_FIR_FIR_PARITY_ERR2
         )
        ) {

        pba_bridge_ffdc(&(G_pba_ffdc.bridge));
    }
}


////////////////////////////////////////////////////////////////////////////
// Initialization
////////////////////////////////////////////////////////////////////////////

void
async_bce_initialize(BceQueue *queue, int engine, SsxIrqId irq)
{
    bce_queue_create(queue, engine);
    async_level_handler_setup(bce_async_handler,
                              (void *)queue,
                              irq,
                              SSX_CRITICAL,
                              SSX_IRQ_POLARITY_ACTIVE_HIGH);
    ssx_irq_enable(irq);
}


void
async_pbax_initialize(PbaxQueue *queue, int engine, SsxIrqId irq,
                     uint64_t *buffer, size_t length, int protocol)
{
    pbax_queue_create(queue, engine, buffer, length, protocol);
    pbax_queue_enable(queue);
    async_level_handler_setup(pbax_async_handler,
                              (void *)queue,
                              irq,
                              SSX_NONCRITICAL,
                              SSX_IRQ_POLARITY_ACTIVE_HIGH);
    // Driver or application manages IRQ enable/disable
}


