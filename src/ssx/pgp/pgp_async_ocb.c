// $Id: pgp_async_ocb.c,v 1.2 2014/02/03 01:30:34 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/pgp_async_ocb.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pgp_async_ocb.c
/// \brief Async driver code for OCB

#include "ssx.h"


////////////////////////////////////////////////////////////////////////////
// Global Data
////////////////////////////////////////////////////////////////////////////

OcbUnitFfdc G_ocb_ffdc = {{{{{0}}}}};

OcbQueue G_ocb_read_queue[OCB_INDIRECT_CHANNELS];
OcbQueue G_ocb_write_queue[OCB_INDIRECT_CHANNELS];


#if OCB_READ0_LENGTH % CACHE_LINE_SIZE
#error "OCB read buffer 0 alignment error"
#endif
#if OCB_READ1_LENGTH % CACHE_LINE_SIZE
#error "OCB read buffer 1 alignment error"
#endif
#if OCB_READ2_LENGTH % CACHE_LINE_SIZE
#error "OCB read buffer 2 alignment error"
#endif
#if OCB_READ3_LENGTH % CACHE_LINE_SIZE
#error "OCB read buffer 3 alignment error"
#endif

// OCB circular queue write buffers must be 8-byte aligned per hardware
// restrictions, whereas read buffers are cache-line aligned and must be an
// even multiple of the cache line size since they must be invalidated. Some
// minor efficiencies could probably be obtained by a policy that CQ buffers
// be held in non-cacheable storage (and subsequent modifications to the
// device driver code).
//
// Due to linker restrictions, only initialized data areas can be aligned.

#define OCB_CQ_WRITE_BUFFER(buffer, length)                           \
    uint64_t buffer[length] __attribute__ ((aligned (8))) = {0}

#define OCB_CQ_READ_BUFFER(buffer, length) \
    uint64_t buffer[length] __attribute__ ((aligned (CACHE_LINE_SIZE))) = {0}


OCB_CQ_READ_BUFFER(G_ocb_read0_buffer, OCB_READ0_LENGTH);
OCB_CQ_READ_BUFFER(G_ocb_read1_buffer, OCB_READ1_LENGTH);
OCB_CQ_READ_BUFFER(G_ocb_read2_buffer, OCB_READ2_LENGTH);

OCB_CQ_WRITE_BUFFER(G_ocb_write0_buffer, OCB_WRITE0_LENGTH);
OCB_CQ_WRITE_BUFFER(G_ocb_write1_buffer, OCB_WRITE1_LENGTH);
OCB_CQ_WRITE_BUFFER(G_ocb_write2_buffer, OCB_WRITE2_LENGTH);


////////////////////////////////////////////////////////////////////////////
// Local Data
////////////////////////////////////////////////////////////////////////////

/// \todo These addresses could/should simply be stored with the queue objects
/// to avoid these static data declarations.

/// OCB Stream Push/Pull Control/Status Register addresses

static const SsxAddress G_ocb_ocbsxcsn[OCB_ENGINES] =
    {OCB_OCBSHCS0, OCB_OCBSLCS0, 
     OCB_OCBSHCS1, OCB_OCBSLCS1, 
     OCB_OCBSHCS2, OCB_OCBSLCS2};

/// OCB Stream Push/Pull Base Register addresses

static const SsxAddress G_ocb_ocbsxbrn[OCB_ENGINES] =
    {OCB_OCBSHBR0, OCB_OCBSLBR0, 
     OCB_OCBSHBR1, OCB_OCBSLBR1,
     OCB_OCBSHBR2, OCB_OCBSLBR2};


/// OCB Stream Push/Pull Increment Register addresses

static const SsxAddress G_ocb_ocbsxin[OCB_ENGINES] =
    {OCB_OCBSHI0, OCB_OCBSLI0, 
     OCB_OCBSHI1, OCB_OCBSLI1,
     OCB_OCBSHI2, OCB_OCBSLI2};


/// OCB Stream Error Status; There is only one register per OCB channel

const SsxAddress G_ocb_ocbsesn[OCB_ENGINES / 2] =
    {OCB_OCBSES0, OCB_OCBSES1, OCB_OCBSES2};


////////////////////////////////////////////////////////////////////////////
// OcbRequest
////////////////////////////////////////////////////////////////////////////

// Collect FFDC for an OCB channel
//
// This is an internal API, called either due to an OCB error interrupt or a
// read/write error detected during the operation.  See the comments for
// OcbFfdc for a description of why this particular set of data is collected.
// The special channel number -1 is used to denote the direct bridge.
//
// OCB FFDC collection procedure:
//
// - Collect the OCBSCR<n> for indirect channels
//
// - Collect PUSH/PULL queue setup for indirect channels and disable the queues.
//
// - Collect the OCB FIR
//
// - Collect FFDC from the PLB (OCI) arbiter
//
// FFDC is only collected for the first error, until the error flag is reset.

static void
ocb_ffdc(int channel)
{
    OcbFfdc* ffdc;

    if (channel < 0) {
        ffdc = &(G_ocb_ffdc.bridge);
    } else {
        ffdc = &(G_ocb_ffdc.channel[channel]);
    }

    if (ffdc->error == 0) {
        
        if (channel < 0) {

            memset(ffdc, 0, sizeof(OcbFfdc));

        } else {

            getscom(OCB_OCBCSRN(channel), &(ffdc->csr.value));

            ffdc->shbr.value = in32(OCB_OCBSHBRN(channel));
            ffdc->shcs.value = in32(OCB_OCBSHCSN(channel));
            ffdc->slbr.value = in32(OCB_OCBSLBRN(channel));
            ffdc->slcs.value = in32(OCB_OCBSLCSN(channel));

            out32(OCB_OCBSHCSN(channel), 
                  ffdc->shcs.value & ~OCB_OCBSHCSN_PUSH_ENABLE);
            out32(OCB_OCBSLCSN(channel), 
                  ffdc->slcs.value & ~OCB_OCBSLCSN_PULL_ENABLE);

        }

        getscom(OCB_OCCLFIR, &(ffdc->fir.value));

        oci_ffdc(&(ffdc->oci_ffdc), OCI_MASTER_ID_OCB);

        ffdc->error = 1;
    }
}


/// Non-blocking read from an OCB PUSH (read) queue
///
/// \param queue The target OcbQueue
///
/// \param buf The caller's data buffer to receive the read data
///
/// \param bytes The maximum number of bytes to read.  This value must be an
/// even multiple of 8, as this API always reads multiples of 8 bytes. 
///
/// \param read The number of bytes actually copied from the device buffer to
/// the caller's buffer.  This may be returned as any value from 0 to \a
/// bytes in multiples of 8 bytes.
///
/// ocb_read() implements a non-blocking copy of data from an OCB read (PUSH)
/// queue data area to the caller's data area, with the side effect of
/// advancing the hardware queue pointers.  ocb_read() does not implement
/// locking, critical sections or any other type of synchronization for access
/// to the OCB queue data - that is the responsibility of the caller. 
///
/// ocb_read() may return an error code.  This may indicate a preexisting
/// error in the queue, or may be an error resulting from the current read.
/// In either event any read data should be considered corrupted.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_ARGUMENT_OCB_READ The number of \a bytes is not 
/// an even multiple of 8.
///
/// \retval -ASYNC_OCB_ERROR_READ_OLD or -ASYNC_OCB_ERROR_READ_NEW This code 
/// indicates an error associated with the OCB channel represented by the queue.

int
ocb_read(OcbQueue *queue, void* buf, size_t bytes, size_t* read)
{
    ocb_ocbshcsn_t csr;
    ocb_ocbsesn_t ses;
    unsigned qlen, read_ptr, write_ptr, to_read;
    uint64_t *pcq, *pbuf;
    int rc;

    do {

        rc = 0;
        *read = 0;

        // If pre-existing errors exist then immediately abort the read.

        if (G_ocb_ffdc.channel[queue->engine / 2].error) {
            rc = -ASYNC_OCB_ERROR_READ_OLD;
            break;
        }

        if (bytes % 8) {
            rc = -ASYNC_INVALID_ARGUMENT_OCB_READ;
            break;
        }

        // Determine the number of doubleword entries remaining to be read in
        // the queue. The driver does not keep state, but instead reinterprets
        // the control/status register each time ocb_read() is called.

        // This may be confusing - remember that 'push' and 'pull' are from
        // the PIB perspective - here we use 'read' and 'write' from OCC's
        // perspective.

        csr.value = in32(G_ocb_ocbsxcsn[queue->engine]);

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

        // Copy the data from the CQ memory area to the user's buffer.  For
        // simplicty of dealing with cache management each doubleword invokes
        // a line invalidate before refetching the fresh data from
        // memory. Alignment requirements enforced on the data buffer
        // guarantee the buffers are cache-line aligned and each doubleword is
        // fully contained in a single D-cache line.
        //
        // Here the code models the evolution of the read_ptr as each datum is
        // copied from the queue.

        pbuf = (uint64_t*)buf;
        while (bytes && to_read--) {

            read_ptr++;
            if (read_ptr == qlen) {
                read_ptr = 0;
            }
            pcq = queue->cq_base + read_ptr;

            dcache_invalidate_line(pcq);
            *pbuf++ = *pcq;
            out32(G_ocb_ocbsxin[queue->engine], 0);

            bytes -= 8;
            *read += 8;
        }
    } while (0);

    // Check for underflow errors. If found, collect FFDC.

    ses.value = in32(G_ocb_ocbsesn[queue->engine / 2]);
    if (ses.fields.push_read_underflow) {
        ocb_ffdc(queue->engine / 2);
        rc = -ASYNC_OCB_ERROR_READ_NEW;
    }

    return rc;
}


/// Non-blocking write to an OCB PULL (write) queue
///
/// \param queue The target OcbQueue
///
/// \param buf The caller's data buffer containing the write data
///
/// \param bytes The maximum number of bytes to write.  This value must be an
/// even multiple of 8, as this API always writes multiples of 8 bytes. 
///
/// \param written The number of bytes actually copied from the caller's buffer to
/// the device buffer.  This may be returned as any value from 0 to \a
/// bytes in multiples of 8 bytes.
///
/// ocb_write() implements a non-blocking copy of data to an OCB write (PULL)
/// queue, with the side effect of advancing the hardware queue pointers.
/// ocb_write() does not implement locking, critical sections or any other
/// type of synchronization for access to the OCB queue data - that is the
/// responsibility of the caller.
///
/// ocb_write() may return an error code.  This may indicate a preexisting
/// error in the queue, or may be an error resulting from the current write.
/// In either event any write data should be considered corrupted/nondelivered.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_ARGUMENT_OCB_WRITE The number of \a bytes is not 
/// an even multiple of 8.
///
/// \retval -ASYNC_OCB_ERROR_WRITE_OLD or -ASYNC_OCB_ERROR_WRITE_NEW This code 
/// indicates an error associated with the OCB channel represented by the queue.

int
ocb_write(OcbQueue *queue, void* buf, size_t bytes, size_t* written)
{
    ocb_ocbslcsn_t csr;
    ocb_ocbsesn_t ses;
    unsigned qlen, read_ptr, write_ptr, free;
    uint64_t *pcq, *pbuf;
    int rc;

    do {

        rc = 0;
        *written = 0;

        // Pre-existing errors immediately abort the read.

        if (G_ocb_ffdc.channel[queue->engine / 2].error) {
            rc = -ASYNC_OCB_ERROR_WRITE_OLD;
            break;
        }

        if (bytes % 8) {
            rc = -ASYNC_INVALID_ARGUMENT_OCB_WRITE;
            break;
        }

        // Determine the number of free doubleword entries remaining in the
        // queue. The driver does not keep state, but instead reinterprets the
        // control/status register each time the write method is called.

        // This is confusing - remember that 'push' and 'pull' are from the PIB
        // perspective - here we use 'read' and 'write' from OCC's perspective.

        csr.value = in32(G_ocb_ocbsxcsn[queue->engine]);

        qlen = csr.fields.pull_length + 1;
        write_ptr = csr.fields.pull_write_ptr;

        if (csr.fields.pull_full) {
            break;
        }

        if (csr.fields.pull_empty) {
            free = qlen;
        } else {
            read_ptr = csr.fields.pull_read_ptr;
            if (write_ptr > read_ptr) {
                free = qlen - (write_ptr - read_ptr);
            } else {
                free = read_ptr - write_ptr;
            }
        }

        // Copy the data into the CQ memory area.  For simplicty of dealing
        // with cache management each doubleword invokes a line flush before
        // incrementing the hardware pointer. Alignment requirements enforced
        // on the data buffer guarantee each doubleword is fully contained in
        // a single D-cache line.
        //
        // Here the code models the evolution of the write_ptr as each datum is
        // copied into the queue.

        pbuf = (uint64_t*)buf;
        while (bytes && free--) {

            write_ptr++;
            if (write_ptr == qlen) {
                write_ptr = 0;
            }
            pcq = queue->cq_base + write_ptr;

            *pcq = *pbuf++;
            dcache_flush_line(pcq);
            in32(G_ocb_ocbsxin[queue->engine]);

            bytes -= 8;
            *written += 8;
        }
    } while (0);

    // Check for overflow. If found, collect FFDC.

    ses.value = in32(G_ocb_ocbsesn[queue->engine / 2]);
    if (ses.fields.pull_write_overflow) {
        ocb_ffdc(queue->engine / 2);
        rc = -ASYNC_OCB_ERROR_WRITE_NEW;
    }

    return rc;
}


// This is the internal 'run method' for reading through an OCB circular
// queue.  The run method simply enables the IRQ.  The interrupt handler reads
// data from the queue and leaves the interrupt enabled until the read is
// satisfied. 

static int
ocb_read_method(AsyncRequest *async_request)
{
    OcbRequest *request = (OcbRequest*)async_request;
    OcbQueue *queue = (OcbQueue*)(async_request->queue);
    int rc;

    if (request->bytes == 0) {
        rc = -ASYNC_REQUEST_COMPLETE;
    } else {
        ssx_irq_enable(queue->irq);
        rc = 0;
    }
    return rc;
}


// This is the internal 'run method' for writing through an OCB circular
// queue. The run method simply enables the IRQ.  The interrupt handler writes
// data from the queue and leaves the interrupt enabled until the write is
// satisfied.

static int
ocb_write_method(AsyncRequest *async_request)
{
    OcbRequest *request = (OcbRequest *)async_request;
    OcbQueue *queue = (OcbQueue*)(async_request->queue);
    int rc;

    if (request->bytes == 0) {
        rc = -ASYNC_REQUEST_COMPLETE;
    } else {
        ssx_irq_enable(queue->irq);
        rc = 0;
    }
    return rc;
}


// The async error method for OCB
//
// Collect FFDC.  The return of -1 will disable the channel associated with
// the request.

static int
ocb_async_error_method(AsyncRequest *request)
{
    OcbQueue *queue = (OcbQueue*)(request->queue);
    ocb_ffdc(queue->engine / 2);
    return -1;
}
    

/// Create a request for an OCB circular queue
///
/// \param request An uninitialized or otherwise idle OcbRequest.
///
/// \param queue An async queue for an OCB circular buffer.  The queue \a
/// engine determines whether this is a read or write request.
///
/// \param data A pointer to the data to be moved (for write) or where the
/// data should be placed (for read).
///
/// \param bytes The number of bytes of data to move.  The OCB
/// circular buffers always move multiples of 8 bytes, and the number of bytes
/// must be a multiple of 8.  Higher-level abstractions will have to take care
/// of cases where the numbers of bytes are not multiples of 8.
///
/// \param timeout If not specified as SSX_WAIT_FOREVER, then this request
/// will be governed by a private watchdog timer that will cancel a queued job
/// or kill a running job if the hardware operation does not complete before
/// it times out.
///
/// \param callback The callback to execute when the data move completes, or
/// NULL (0) to indicate no callback.
///
/// \param arg The parameter to the callback routine; ignored if the \a
/// callback is NULL.
///
/// \param options Options to control request priority, callback context and
/// blocking.
///
/// This routine has no way to know if the OcbRequest structure is currently
/// in use, so this API should only be called on uninitialized or otherwise
/// idle OcbRequest structures.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_OBJECT_OCB_REQUEST The \a request or \a queue were NULL (0), or
/// the \a queue is not an initialized OcbQueue.
///
/// \retval -ASYNC_INVALID_ARGUMENT_OCB_REQUEST The \a data pointer is 
/// NULL (0), or the number of bytes is not a multiple of 8.
///
/// See async_request_create() for other errors that may be returned by this
/// call.

int
ocb_request_create(OcbRequest *request,
                   OcbQueue *queue,
                   uint64_t *data,
                   size_t bytes,
                   SsxInterval timeout,
                   AsyncRequestCallback callback,
                   void *arg,
                   int options)
{
    int rc;
    AsyncRunMethod run_method = 0; // Make GCC Happy
    AsyncQueue *async_queue = (AsyncQueue *)queue;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((request == 0) ||
                     (queue == 0)   ||
                     !(async_queue->engine & ASYNC_ENGINE_OCB),
                     ASYNC_INVALID_OBJECT_OCB_REQUEST);
        SSX_ERROR_IF((data == 0)  ||
                     (bytes % 8),
                     ASYNC_INVALID_ARGUMENT_OCB_REQUEST);
    }

    switch (async_queue->engine) {

    case ASYNC_ENGINE_OCB_PULL0:
    case ASYNC_ENGINE_OCB_PULL1:
    case ASYNC_ENGINE_OCB_PULL2:
        run_method = ocb_write_method;
        break;

    case ASYNC_ENGINE_OCB_PUSH0:
    case ASYNC_ENGINE_OCB_PUSH1:
    case ASYNC_ENGINE_OCB_PUSH2:
        run_method = ocb_read_method;
        break;
    }

    rc = async_request_create(&(request->request),
                              async_queue,
                              run_method,
                              ocb_async_error_method,
                              timeout,
                              callback,
                              arg,
                              options);

    request->data = data;
    request->bytes = bytes;

    return rc;
}


/// Schedule a request on an OCB circular queue.
///
/// \param request An initialized OcbRequest
///
/// Note : As long as the OcbRequest is idle, the application is free to
/// directly change the \a data and \a bytes fields to read/write different
/// numbers of bytes to/from different locations the next time the request is
/// scheduled.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_ARGUMENT_OCB_SCHEDULE The number of \a bytes 
/// currently requested is not a multiple of 8.
///
/// See async_request_schedule() for documentation of other errors

int
ocb_request_schedule(OcbRequest *request)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((request->bytes % 8), ASYNC_INVALID_ARGUMENT_OCB_SCHEDULE);
    }

    request->current = request->data;
    request->remaining = request->bytes;

    return async_request_schedule((AsyncRequest *)request);
}


////////////////////////////////////////////////////////////////////////////
// OcbQueue
////////////////////////////////////////////////////////////////////////////

/// Reset an OCB circular PUSH (read) queue
///
/// This API is normally used during initialization, and assumes that all of
/// the parameters are valid.  It resets and reprograms the hardware
/// associated with an OCB circular buffer to be consistent with the queue
/// engine, base address, length and interrupt protocol.  The queue is enabled
/// and its interrupts are disabled.  Any data in the queue will be silently
/// lost.
///
/// Note that this API \e does \e not put the OCB channel into circular mode -
/// the communication method is controlled by the communication partner.
///
/// The function of this routine is to write a new value into the OCB Stream
/// Push Control/Status register, which as a side effect resets the circular
/// buffer. The base register is also set up.

void
ocb_read_engine_reset(OcbQueue *queue, size_t cq_length, int protocol)
{
    ocb_ocbshcsn_t cs;

    // Disable interrupts and disable and clear the queue.  The queue length
    // field is updated (for informational purposes). Interrupts will be
    // re-enabled when requests are made for the queue.

    ssx_irq_disable(queue->irq);

    queue->cq_length = cq_length;

    cs.value = 0;
    out32(G_ocb_ocbsxcsn[queue->engine], cs.value);


    // Reinitialize the data buffer base address register

    out32(G_ocb_ocbsxbrn[queue->engine], (uint32_t)(queue->cq_base));


    // Reprogram and reenable/reset the queue

    if (protocol == OCB_INTERRUPT_PROTOCOL_LAZY) {
        cs.fields.push_intr_action = OCB_INTR_ACTION_FULL;
    } else {
        cs.fields.push_intr_action = OCB_INTR_ACTION_NOT_EMPTY;
    }

    cs.fields.push_length = cq_length - 1;
    cs.fields.push_enable = 1;

    out32(G_ocb_ocbsxcsn[queue->engine], cs.value);
}


/// Reset an OCB circular PULL (write) queue
///
/// This API is normally used during initialization, and assumes that all of
/// the parameters are valid.  It resets and reprograms the hardware
/// associated with an OCB circular buffer to be consistent with the queue
/// engine, base address, length and interrupt protocol.  The queue is enabled
/// and its interrupts are disabled.  Any data in the queue will be silently
/// lost.
///
/// Note that this API \e does \e not put the OCB channel into circular mode -
/// the communication method is controlled by the communication partner.
///
/// The function of this routine is to write a new value into the OCB Stream
/// Pull Control/Status register, which as a side effect resets the circular
/// buffer. The base register is also set up.

void
ocb_write_engine_reset(OcbQueue *queue, size_t cq_length, int protocol)
{
    ocb_ocbslcsn_t cs;

    // Disable interrupts and disable and clear the queue.  The queue length
    // field is updated (for informational purposes). Interrupts will be
    // re-enabled when requests are made for the queue.

    ssx_irq_disable(queue->irq);

    queue->cq_length = cq_length;

    cs.value = 0;
    out32(G_ocb_ocbsxcsn[queue->engine], cs.value);


    // Reinitialize the data buffer base address register

    out32(G_ocb_ocbsxbrn[queue->engine], (uint32_t)(queue->cq_base));


    // Reprogram and reenable/reset the queue

    if (protocol == OCB_INTERRUPT_PROTOCOL_LAZY) {
        cs.fields.pull_intr_action = OCB_INTR_ACTION_EMPTY;
    } else {
        cs.fields.pull_intr_action = OCB_INTR_ACTION_NOT_FULL;
    }

    cs.fields.pull_length = cq_length - 1;
    cs.fields.pull_enable = 1;

    out32(G_ocb_ocbsxcsn[queue->engine], cs.value);
}


/// Create (initialize) an OcbQueue
///
/// \param queue An uninitialized or otherwise idle OcbQueue
///
/// \param engine A valid OCB engine id
///
/// \param cq_base The base address of the circular queue data area for the
/// queue. This address must be 8-byte aligned for write queues. Read queues
/// must be cache-line aligned and a multiple of the cache line in size.
///
/// \param cq_length The length of the circular queue measured in 8-byte
/// increments.
///
/// \param protocol The interrupt protocol, either OCB_PUSH_PULL_PROTOCOL_LAZY
/// or OCB_PUSH_PULL_PROTOCOL_AGGRESSIVE.  Lazy means that read queues only
/// interrupt when empty, and write queues only interrupt when full.
/// Agressive means that read queues interrupt when not empty and write queues
/// interrupt when not full.  In general the lazy read protocol will only work
/// for 1) queues of length 1, where lazy == aggressive, and 2) protocols
/// where a known fixed number of 8-byte entries is always expected to be
/// received.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_OBJECT_OCB_QUEUE The \a queue was NULL (0).
///
/// \retval -ASYNC_INVALID_ARGUMENT_OCB_QUEUE The \a cq_base is not properly 
/// aligned, or the \a cq_length is invalid, or the \a protocol is invalid.
///
/// \retval -ASYNC_INVALID_ENGINE_OCB The \a engine is not an OCB engine.
///
/// Other errors may be returned by async_queue_create().

int
ocb_queue_create(OcbQueue *queue,
                 int engine,
                 uint64_t *cq_base,
                 size_t cq_length,
                 int protocol)
{
    AsyncQueue *async_queue = (AsyncQueue *)queue;
    int align_mask = 0;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(queue == 0, ASYNC_INVALID_OBJECT_OCB_QUEUE);
        SSX_ERROR_IF((cq_length < OCB_PUSH_PULL_LENGTH_MIN) ||
                     (cq_length > OCB_PUSH_PULL_LENGTH_MAX) ||
                     ((protocol != OCB_INTERRUPT_PROTOCOL_LAZY) &&
                      (protocol != OCB_INTERRUPT_PROTOCOL_AGGRESSIVE)),
                     ASYNC_INVALID_ARGUMENT_OCB_QUEUE);
    }

    queue->cq_base = cq_base;
    queue->cq_length = cq_length;

    switch (engine) {

        // These are the read engines from OCC's perspective.

    case ASYNC_ENGINE_OCB_PUSH0:
        queue->irq = PGP_IRQ_STRM0_PUSH;
        queue->engine = OCB_ENGINE_PUSH0;
        goto read_engine;

    case ASYNC_ENGINE_OCB_PUSH1:
        queue->irq = PGP_IRQ_STRM1_PUSH;
        queue->engine = OCB_ENGINE_PUSH1;
        goto read_engine;

    case ASYNC_ENGINE_OCB_PUSH2:
        queue->irq = PGP_IRQ_STRM2_PUSH;
        queue->engine = OCB_ENGINE_PUSH2;
        goto read_engine;

    read_engine:
        align_mask = CACHE_LINE_SIZE - 1;
        async_queue_create(async_queue, engine);
        ocb_read_engine_reset(queue, cq_length, protocol);
        break;

        // These are the write engines from OCC's perspective.

    case ASYNC_ENGINE_OCB_PULL0:
        queue->irq = PGP_IRQ_STRM0_PULL;
        queue->engine = OCB_ENGINE_PULL0;
        goto write_engine;

    case ASYNC_ENGINE_OCB_PULL1:
        queue->irq = PGP_IRQ_STRM1_PULL;
        queue->engine = OCB_ENGINE_PULL1;
        goto write_engine;

    case ASYNC_ENGINE_OCB_PULL2:
        queue->irq = PGP_IRQ_STRM2_PULL;
        queue->engine = OCB_ENGINE_PULL2;
        goto write_engine;

    write_engine:
        align_mask = 8 - 1;
        async_queue_create(async_queue, engine);
        ocb_write_engine_reset(queue, cq_length, protocol);
        break;

    default:
        if (SSX_ERROR_CHECK_API) {
            SSX_ERROR_IF(1, ASYNC_INVALID_ENGINE_OCB);
        }
    }

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(((uint32_t)cq_base & align_mask) != 0, 
                     ASYNC_INVALID_ARGUMENT_OCB_QUEUE2);
    }

    return 0;
}


// The interrupt handler for asynchronous OCB CQ requests
//
// The circular buffer interupts are level sensitive, active high.  There is
// really no way to 'clear' them as they indicate a permanent status - so
// instead they need to be enabled and disabled. Interrupts are enabled by the
// run methods and disabled by the interrupt handlers once all data has been
// transferred (or in the event of an error).
//
// This interrupt handler can process up to 256 bytes at once, plus the
// overhead of scheduling the next job when this one completes.  If interrupt
// latency becomes a problem then this process could be run with interrupt
// preemption enabled.

SSX_IRQ_FAST2FULL(ocb_async_handler, ocb_async_handler_full);

void
ocb_async_handler_full(void *arg, SsxIrqId irq, int priority)
{
    AsyncQueue *async_queue = (AsyncQueue*)arg;
    OcbQueue *queue = (OcbQueue*)async_queue;
    OcbRequest *request = (OcbRequest*)(async_queue->current);
    size_t processed;
    int rc;

    if (SSX_ERROR_CHECK_KERNEL && (request == 0)) {
        SSX_PANIC(ASYNC_PHANTOM_INTERRUPT_OCB);
    }

    if (queue->engine % 2) {
        rc = ocb_write(queue, request->current, request->remaining, &processed);
    } else {
        rc = ocb_read(queue, request->current, request->remaining, &processed);
    }

    if (rc) {

        ssx_irq_disable(queue->irq);
        async_error_handler(async_queue, ASYNC_REQUEST_STATE_FAILED);

    } else if (processed == request->remaining) {

        ssx_irq_disable(queue->irq);
        async_handler(async_queue);

    } else {

        request->current += (processed / 8);
        request->remaining -= processed;
    }
}


// The interrupt handler for the OCB error interrupt.
//
// There is one interrupt that covers all OCB indirect channels as well as the
// direct bridge.  When this interrupt fires we try to determine which unit is
// responsible. If the error appears to be associated with a job running as
// part of the async mechanism then we let the async_error_handler() mechanism
// operate, otherwise simply collect FFDC.  The \a error field of the FFDC
// structure stops non-queued read/write requests. Note that we kill both read
// and write jobs without regard to the error.
//
// If the error is due to the direct bridge we collect FFDC, but can't really
// do anything else.

/// \todo What action to take for bridge errors?


SSX_IRQ_FAST2FULL(ocb_error_handler, ocb_error_handler_full);

void
ocb_error_handler_full(void *arg, SsxIrqId irq, int priority)
{
    ocb_occlfir_t fir;
    int channel;
    AsyncQueue* queue;

    ssx_irq_status_clear(irq);

    getscom(OCB_OCCLFIR, &(fir.value));
    
    for (channel = 0; channel < OCB_INDIRECT_CHANNELS; channel++) {
        if (fir.value & (OCB_OCCLFIR_OCB_IDC0_ERROR >> channel)) {

            queue = (AsyncQueue*)(&(G_ocb_read_queue[channel]));
            if (queue->state == ASYNC_QUEUE_STATE_RUNNING) {
                async_error_handler(queue, ASYNC_REQUEST_STATE_FAILED);
            } else {
                ocb_ffdc(channel);
            }
                
            queue = (AsyncQueue*)(&(G_ocb_write_queue[channel]));
            if (queue->state == ASYNC_QUEUE_STATE_RUNNING) {
                async_error_handler(queue, ASYNC_REQUEST_STATE_FAILED);
            } else {
                ocb_ffdc(channel);
            }
        }
    }

    if (fir.fields.ocb_db_oci_timeout |
        fir.fields.ocb_db_oci_read_data_parity |
        fir.fields.ocb_db_oci_slave_error |
        fir.fields.ocb_pib_addr_parity_err |
        fir.fields.ocb_db_pib_data_parity_err) {

        ocb_ffdc(-1);
    }
}


////////////////////////////////////////////////////////////////////////////
// Initialization
////////////////////////////////////////////////////////////////////////////

void
async_ocb_initialize(OcbQueue *queue, int engine,
                     uint64_t *buffer, size_t length, int protocol)
{
    ocb_queue_create(queue, engine, buffer, length, protocol);
    async_level_handler_setup(ocb_async_handler,
                              (void *)queue,
                              queue->irq,
                              SSX_NONCRITICAL,
                              SSX_IRQ_POLARITY_ACTIVE_HIGH);
    // Driver manages IRQ enable/disable
}
