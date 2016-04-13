/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/pgp_async_pore.c $                                */
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
/// \file pgp_async_pore.c
/// \brief PgP "async" drivers for PORE engines

#include "ssx.h"

////////////////////////////////////////////////////////////////////////////
// Global Data
////////////////////////////////////////////////////////////////////////////

// The PORE queue objects.

PoreQueue G_pore_gpe0_queue;
PoreQueue G_pore_gpe1_queue;
PoreQueue G_pore_slw_queue;


////////////////////////////////////////////////////////////////////////////
// Local Data
////////////////////////////////////////////////////////////////////////////

/// PoreFlex entry point - See G_pore_flex_table.

static uint32_t G_pore_flex_entry0 = PORE_BRAD_D0;


/// Entry 0 of the PoreFlex branch table
///
/// This variable is the only thing we represent of the branch table for PORE
/// flex requests.  PoreFlex requests are forbidden from using PORE error
/// handlers. Therefore they don't require the 60 redundant bytes of error
/// handler entry points.  They also only run trigger slot 0, and begin
/// execution with a BRAD D0, so the only thing we represent is a single BRAD
/// D0 instruction.

static uint32_t* G_pore_flex_table = &G_pore_flex_entry0 - (PORE_ERROR_SLOTS * 3);


////////////////////////////////////////////////////////////////////////////
// PoreQueue
////////////////////////////////////////////////////////////////////////////

/// Create (initialize) a PoreQueue
///
/// \param queue An uninitialized of otherwise idle PoreQueue
///
/// \param engine The identifier of a PORE engine associated with this queue.
///
/// This API initializes the PoreQueue structure and also initializes the
/// underlying PORE hardware to run in the OCC environment.  Neither the
/// branch table nor the error modes are specified here - those are considered
/// application-specific functions that are set up each time a job is run on
/// the engine.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_OBJECT_PORE_QUEUE The \a queue was NULL (0).
///
/// \retval -ASYNC_INVALID_ENGINE_PORE The \a engine is not a (valid) 
/// PORE engine.

int
pore_queue_create(PoreQueue *queue, int engine)
{
    pore_control_t control;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(queue == 0, ASYNC_INVALID_OBJECT_PORE_QUEUE);
        SSX_ERROR_IF(!(engine & ASYNC_ENGINE_PORE), ASYNC_INVALID_ENGINE_PORE);
    }

    async_queue_create(&(queue->queue), engine);

    switch (engine) {

    case ASYNC_ENGINE_PORE_GPE0:
        queue->oci_base = PORE_GPE0_OCI_BASE;
        queue->irq = PGP_IRQ_PORE_GPE0_COMPLETE;
        queue->error_irq = PGP_IRQ_PORE_GPE0_ERROR;
        queue->oci_master = OCI_MASTER_ID_PORE_GPE;
        break;

    case ASYNC_ENGINE_PORE_GPE1:
        queue->oci_base = PORE_GPE1_OCI_BASE;
        queue->irq = PGP_IRQ_PORE_GPE1_COMPLETE;
        queue->error_irq = PGP_IRQ_PORE_GPE1_ERROR;
        queue->oci_master = OCI_MASTER_ID_PORE_GPE;
        break;

    case ASYNC_ENGINE_PORE_SLW:
        queue->oci_base = PORE_SLW_OCI_BASE;
        queue->irq = PGP_IRQ_PORE_SW_COMPLETE;
        queue->error_irq = PGP_IRQ_PORE_SW_ERROR;
        queue->oci_master = OCI_MASTER_ID_PORE_SLW;
        break;

    default:
        SSX_PANIC(ASYNC_BUG_PORE_AT_CREATE);
    }

    // PORE engine setup
    //
    // Force the PORE to stop and set it up for OCC control.  Neither the
    // breakpoint address nor the trap enable setting are modified in case
    // they are being controlled from Simics or a hardware debugger ab initio.
    //
    // Register field settings:
    //
    // The scanclk ratio is not modified.
    // The EXE-Trigger register is unlocked
    // The freeze action is not modified
    // Instruction parity is ignored
    // The PIB parity checking setting is not modified
    // The TRAP enable is not modified
    // The breakpoint address is not modified

    control.value = in64(queue->oci_base + PORE_CONTROL_OFFSET);

    control.fields.start_stop = 1;
    control.fields.lock_exe_trig= 0;
    control.fields.check_parity = 0;

    out64(queue->oci_base + PORE_CONTROL_OFFSET, control.value);

    return 0;
}


// The interrupt handler for asynchronous PORE errors
//
// The PORE interrupts are disabled here, then cleared and re-enabled when the
// next job runs.  This is to protect against "phantom" interrupts caused by
// PORE freeze-on-checkstop behavior.

SSX_IRQ_FAST2FULL(pore_async_error_handler, pore_async_error_handler_full);

void
pore_async_error_handler_full(void *arg, SsxIrqId irq, int priority)
{
    PoreQueue* queue = (PoreQueue*)arg;

    ssx_irq_disable(queue->irq);
    ssx_irq_disable(queue->error_irq);

    async_error_handler((AsyncQueue *)arg, ASYNC_REQUEST_STATE_FAILED);
}


// The interrupt handler for asynchronous PORE requests
//
// The PORE interrupts are disabled here, then cleared and re-enabled when the
// next job runs.  This is to protect against "phantom" interrupts caused by
// PORE freeze-on-checkstop behavior.
//
// Note that if the system checkstops and freezes the PORE we will get a
// normal completion interrupt.  Therefore we have to check to see if the
// completion is associated with a freeze, and if so, fail the job.

SSX_IRQ_FAST2FULL(pore_async_handler, pore_async_handler_full);

void
pore_async_handler_full(void *arg, SsxIrqId irq, int priority)
{
    PoreQueue* queue = (PoreQueue*)arg;
    pore_status_t status;

    status.value = in64(queue->oci_base + PORE_STATUS_OFFSET);
    if (status.fields.freeze_action) {

        pore_async_error_handler_full(arg, irq, priority);

    } else {

        ssx_irq_disable(queue->irq);
        ssx_irq_disable(queue->error_irq);

        async_handler((AsyncQueue *)arg);
    }
}


////////////////////////////////////////////////////////////////////////////
// PoreRequest
////////////////////////////////////////////////////////////////////////////

/// Create (initialize) the PoreRequest base class
///
/// \param request An uninitialized or otherwise idle PoreRequest.
///
/// \param queue An initialized PoreQueue
///
/// \param table The PORE branch table to install prior to kicking off the
/// engine. All PoreFlex jobs use a common (stubbed) table.  PoreFixed jobs
/// must supply a fully-formed table.
///
/// \param error_mask The initial value of the PORE ERROR_MASK register to be
/// installed before kicking off the engine.
///
/// \param entry_point The entry point address of the routine.  For PoreFlex
/// this entry point will be non-0 and will be inserted into D0, as PoreFlex
/// jobs are kicked off by BRAD D0.  For PoreFixed this parameter will be zero
/// and ignored.
///
/// \param start_vector The TBAR start vector to execute. This will always be
/// 0 for PoreFlex.
///
/// \param parameter The single 32-bit parameter to the PORE program.  This
/// value is stored in the low-order part of the \c EXE_TRIGGER register
/// prior to initiating the PORE program. (This part of the \c EXE_TRIGGER
/// register is referred to as the 'Chiplet Select Mask' in PORE docs., as
/// this is the hardware usage for hardware-initiated PORE-SLW routines.)
///
/// \param timeout If not specified as SSX_WAIT_FOREVER, then this request
/// will be governed by a private watchdog timer that will cancel a queued job
/// or kill a running job if the hardware operation does not complete before
/// it times out.
///
/// \param callback The callback to execute when the PORE program completes,
/// or NULL (0) to indicate no callback.
///
/// \param arg The parameter to the callback routine; ignored if the \a
/// callback is NULL.
///
/// \param options Options to control request priority and callback context.
///
/// This routine has no way to know if the PoreRequest structure is currently
/// in use, so this API should only be called on uninitialized or otherwise
/// idle PoreRequest structures.
///
/// \retval 0 Success
///
/// \retval -ASYNC_INVALID_OBJECT_PORE_REQUEST The \a request was NULL (0) 
/// or the \a queue was NULL (0) or not a PoreQueue.
///
/// \retval -ASYNC_INVALID_ARGUMENT_PORE_REQUEST The \a start_vector is invalid or any of
/// the parameters that represent OCI addresses are not 4-byte aligned, , or
/// the \a table was null.
///
/// See async_request_create() for other errors that may be returned by this
/// call.

int
pore_request_create(PoreRequest *request,
                    PoreQueue *queue,
                    PoreBraia* table,
                    uint32_t error_mask,
                    uint32_t entry_point,
                    int start_vector,
                    uint32_t parameter,
                    SsxInterval timeout,
                    AsyncRequestCallback callback,
                    void *arg,
                    int options)
{
    AsyncQueue *async_queue = (AsyncQueue *)queue;
    int rc;
    pore_exe_trigger_t etr;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(!(async_queue->engine & ASYNC_ENGINE_PORE),
                     ASYNC_INVALID_OBJECT_PORE_REQUEST);
        SSX_ERROR_IF((start_vector < 0) ||
                     (start_vector >= PORE_TRIGGER_SLOTS) ||
                     ((uint32_t) table % 4) ||
                     (entry_point % 4) ||
                     (table == 0),
                     ASYNC_INVALID_ARGUMENT_PORE_REQUEST);
    }

    rc = async_request_create(&(request->request),
                              async_queue,
                              pore_run_method,
                              pore_error_method,
                              timeout,
                              callback,
                              arg,
                              options);

    if (!rc) {
        request->table = table;
        request->error_mask = error_mask;
        request->entry_point = entry_point;
        request->parameter = parameter;
        etr.value = 0;
        etr.fields.start_vector = start_vector;
        request->exe_trigger = etr.words.high_order;
    }

    return rc;
}


// Start a PoreRequest on a PORE
//
// \param async_request A PoreRequest upcast to an AsyncRequest.
//
// This is an internal API. At entry both the completion and error interrupts
// are disabled and may show status that needs to be cleared before they are
// re-enabled.
//
// This routine implements a simple procedure:
//
// - Check to make sure the PORE is not frozen due to a checkstop, and if so,
//   collect FFDC and immediately fail the job.
//
// Otherwise:
//
// - Reset the PORE engine to clear up any error status that may remain from
//   the last job .
// - Install the TBAR (Table Base Address Register) from the request as an OCI
//   address
// - Set the EMR (Error Mask Register) from the request
// - Install the parameter (ETR[32:63])
// - If the entry point is non-0 then this is a PoreFlex job that is kicked
//   off by a BRAD D0, and the entry point is installed in D0 as a full OCI
//   address. 
// - Clear pending interrupt status
// - Hit ETR[0:31] to start the job.
// - Enable interrupts.
//
// If the PORE is frozen due to a system checkstop we fail the job immediately
// right here.  Note that there is still a small window where the system may
// checkstop and the PORE may freeze after this check. Unfortunately the PORE
// design locks out register writes while frozen, and instead of reporting
// write access attempts as bus errors, silently ignores them and simply sets
// a FIR bit. Originally the "frozen" check was done last to shrink the
// window, however this practically guarantees these FIRs in a checkstopped
// system (which the FW team finds problematic), so the check was moved to the
// front of the procedure. (SW256621).
//
// Note that PORE interrupts remain masked unless the job starts successfully.

int
pore_run_method(AsyncRequest *async_request)
{
    PoreQueue *queue = (PoreQueue*)(async_request->queue);
    PoreRequest *request = (PoreRequest*)async_request;
    pore_status_t status;
    pore_reset_t reset;
    uint32_t oci_base;
    int rc;

    oci_base = queue->oci_base;

    status.value = in64(oci_base + PORE_STATUS_OFFSET);
    if (status.fields.freeze_action) {

        pore_error_method(async_request);
        async_request->completion_state = ASYNC_REQUEST_STATE_FAILED;
        rc = -ASYNC_REQUEST_COMPLETE;

    } else {

        reset.value = 0;
        reset.fields.fn_reset = 1;
        out32(oci_base + PORE_RESET_OFFSET, reset.value);

        out32(oci_base + PORE_TABLE_BASE_ADDR_OFFSET, PORE_ADDRESS_SPACE_OCI);
        out32(oci_base + PORE_TABLE_BASE_ADDR_OFFSET + 4,
              (uint32_t)(request->table));
        out32(oci_base + PORE_ERROR_MASK_OFFSET, request->error_mask);
        out32(oci_base + PORE_EXE_TRIGGER_OFFSET + 4, request->parameter);

        if (request->entry_point != 0) {
            out32(oci_base + PORE_SCRATCH1_OFFSET, PORE_ADDRESS_SPACE_OCI);
            out32(oci_base + PORE_SCRATCH1_OFFSET + 4, request->entry_point);
        }

        ssx_irq_status_clear(queue->irq);
        ssx_irq_status_clear(queue->error_irq);

        out32(oci_base + PORE_EXE_TRIGGER_OFFSET, request->exe_trigger);

        ssx_irq_enable(queue->irq);
        ssx_irq_enable(queue->error_irq);
        rc = 0;
    }

    return rc;
}


// PORE FFDC collection
//
// \param async_request A PoreRequest upcast to an AsyncRequest
//
// This is an internal API, called from an interrupt context when a PORE
// engine signals an error interrupt.  See the comments for PoreFfdc for a
// description of why this particular set of data is collected.
//
// PORE error handling procedure:
//
// - Collect FFDC from the PLB arbiter
//
// - Collect FFDC from the failing engine
//
// Currently all PORE errors are treated as recoverable

/// \todo Consider analyzing the errors to determine if the error should be
/// considered fatal.

int
pore_error_method(AsyncRequest *async_request)
{
    PoreQueue *queue = (PoreQueue*)(async_request->queue);
    PoreRequest *request = (PoreRequest*)async_request;
    uint32_t oci_base;
    PoreFfdc* ffdc;

    oci_base = queue->oci_base;
    ffdc = &(request->ffdc);
 
    oci_ffdc(&(ffdc->oci_ffdc), queue->oci_master);

    ffdc->debug[0] = in64(oci_base + PORE_DBG0_OFFSET);
    ffdc->debug[1] = in64(oci_base + PORE_DBG1_OFFSET);
    ffdc->address[0] = in32(oci_base + PORE_OCI_BASE_ADDRESS0_OFFSET + 4);
    ffdc->address[1] = in32(oci_base + PORE_OCI_BASE_ADDRESS1_OFFSET + 4);
    ffdc->ibuf[0] = in32(oci_base + PORE_IBUF_01_OFFSET);
    ffdc->ibuf[1] = in32(oci_base + PORE_IBUF_01_OFFSET + 4);
    ffdc->ibuf[2] = in32(oci_base + PORE_IBUF_2_OFFSET);

    return 0;
}


/// Create (initialize) a PoreBraia branch table entry
///
/// \param instr  A pointer to the BRAIA instruction to initialize. Use the
/// macros PORE_ERROR_BRANCH(table, n) and PORE_ENTRY_BRANCH(table, n) to
/// select one of 5 error branches or one of 16 entry point branches in a PORE
/// branch table.
///
/// \param address The 32-bit OCI address of the error routine or entry point.
///
/// This routine initializes the given entry of a PORE branch table with an
/// OCI-based BRAIA instruction, them flushes the entry from the D-Cache.

// Note that we don't know the alignment of the jump table, so we need to
// flush both the first and last jump address to ensure that the BRAI is
// completely flushed. This assumes (correctly) that uint32_t are at least
// 4-byte aligned.

void
pore_braia_create(PoreBraia* instr, uint32_t address) {
    instr->word[0] = PORE_BRAI;
    instr->word[1] = PORE_ADDRESS_SPACE_OCI;
    instr->word[2] = address;
    dcache_flush_line(&(instr->word[0]));
    dcache_flush_line(&(instr->word[2]));
}


////////////////////////////////////////////////////////////////////////////
// PoreFlex
////////////////////////////////////////////////////////////////////////////

/// Create (initialize) a flex-mode PORE request
///
/// \param request An uninitialized or otherwise idle PoreFlex.
///
/// \param queue A pointer to a PoreQueue
///
/// \param entry_point The entry point of the PORE program. This must be a
/// 32-bit, 4-byte aligned byte address in OCI space. The PoreEntryPoint
/// typedef is provided to declare external PORE entry points. Note that an \a
/// entry_point of 0 is considered an error - although it \e is conceivably a
/// legal OCI address in mainstore via the PBA.
///
/// \param timeout If not specified as SSX_WAIT_FOREVER, then this request
/// will be governed by a private watchdog timer that will cancel a queued job
/// or kill a running job if the hardware operation does not complete before
/// it times out.
///
/// \param parameter The single 32-bit parameter to the PORE program.  This
/// value is stored in the high-order part of the \c EXE_TRIGGER register
/// prior to initiating the PORE program. (This part of the \c EXE_TRIGGER
/// register is referred to as the 'Chiplet Select Mask' in PORE docs., as
/// this is the hardware usage for hardware-initiated PORE-SLW routines.)
///
/// \param callback The callback to execute when the PORE program completes,
/// or NULL (0) to indicate no callback.
///
/// \param arg The parameter to the callback routine; ignored if the \a
/// callback is NULL.
///
/// \param options Options to control request priority and callback context.
///
/// This routine has no way to know if the PoreFlex structure is currently
/// in use, so this API should only be called on uninitialized or
/// otherwise idle PoreFlex structures.
///
/// \retval 0 Success
///
/// See pore_request_create() for error return codes that may be returned by
/// this call.

int
pore_flex_create(PoreFlex *request,
                 PoreQueue *queue,
                 PoreEntryPoint entry_point,
                 uint32_t parameter,
                 SsxInterval timeout,
                 AsyncRequestCallback callback,
                 void *arg,
                 int options)
{
    uint32_t emr;

    // PoreFlex jobs run w/o error handlers, and ignore sleeping cores. All
    // errors are signalled on both error outputs of all PORE engines.

    emr = (PORE_ERROR_MASK_ENABLE_ERR_OUTPUT0       |
           PORE_ERROR_MASK_ENABLE_ERR_OUTPUT1       |
           PORE_ERROR_MASK_ENABLE_ERR_OUTPUT2       |
           PORE_ERROR_MASK_ENABLE_ERR_OUTPUT3       |
           PORE_ERROR_MASK_ENABLE_ERR_OUTPUT4       |
           PORE_ERROR_MASK_ENABLE_FATAL_ERR_OUTPUT0 |
           PORE_ERROR_MASK_ENABLE_FATAL_ERR_OUTPUT1 |
           PORE_ERROR_MASK_ENABLE_FATAL_ERR_OUTPUT2 |
           PORE_ERROR_MASK_ENABLE_FATAL_ERR_OUTPUT3 |
           PORE_ERROR_MASK_ENABLE_FATAL_ERR_OUTPUT4 |
           PORE_ERROR_MASK_STOP_EXE_ON_ERROR0       |
           PORE_ERROR_MASK_STOP_EXE_ON_ERROR1       |
           PORE_ERROR_MASK_STOP_EXE_ON_ERROR2       |
           PORE_ERROR_MASK_STOP_EXE_ON_ERROR3       |
           PORE_ERROR_MASK_STOP_EXE_ON_ERROR4) >> 32;

    return pore_request_create((PoreRequest*)request,
                               queue,
                               (PoreBraia*)G_pore_flex_table,
                               emr,
                               (uint32_t)entry_point,
                               0,
                               parameter,
                               timeout,
                               callback,
                               arg,
                               options);
}


////////////////////////////////////////////////////////////////////////////
// PoreFixed
////////////////////////////////////////////////////////////////////////////

/// Create (initialize) a fixed-mode PORE request
///
/// \param request An uninitialized or otherwise idle PoreFixed request.
///
/// \param queue A PoreQueue capable of running fixed requests.
///
/// \param table A PORE branch table containing all of the error handler and
/// entry point assignments required for the request.
///
/// \param error_mask A value that will be loaded into the high-order 32-bits
/// of the PORE Error Mask Register to control error behavior.
///
/// \param start_vector The branch table slot reserved for this request.
///
/// \param parameter The single 32-bit parameter to the PORE program.  This
/// value is stored in the high-order part of the \c EXE_TRIGGER register
/// prior to initiating the PORE program. (This part of the \c EXE_TRIGGER
/// register is referred to as the 'Chiplet Select Mask' in PORE docs., as
/// this is the hardware usage for hardware-initiated PORE-SLW routines.)
///
/// \param timeout If not specified as SSX_WAIT_FOREVER, then this request
/// will be governed by a private watchdog timer that will cancel a queued job
/// or kill a running job if the hardware operation does not complete before
/// it times out.
///
/// \param callback The callback to execute when the PORE program completes,
/// or NULL (0) to indicate no callback.
///
/// \param arg The parameter to the callback routine; ignored if the \a
/// callback is NULL.
///
/// \param options Options to control request priority and callback context.
///
/// This routine has no way to know if the PoreFixed structure is currently
/// in use, so this API should only be called on uninitialized or
/// otherwise idle PoreFlex structures.
///
/// \retval 0 Success
///
/// See pore_request_create() for error return codes that may be returned by
/// this call.

int
pore_fixed_create(PoreFixed *request,
                  PoreQueue *queue,
                  PoreBraia* table,
                  uint32_t error_mask,
                  int start_vector,
                  uint32_t parameter,
                  SsxInterval timeout,
                  AsyncRequestCallback callback,
                  void *arg,
                  int options)
{
    return pore_request_create((PoreRequest*)request,
                               queue,
                               table,
                               error_mask,
                               0,
                               start_vector,
                               parameter,
                               timeout,
                               callback,
                               arg,
                               options);
}


////////////////////////////////////////////////////////////////////////////
// Initialization
////////////////////////////////////////////////////////////////////////////

// Due to the fact that PORE signals a "complete" interrupt on a freeze event
// (i.e., a checkstop, even if PORE is not running), we can not enable PORE
// interrupts globally.  They need to be carefully managed to avoid "phantom
// interrupt" panics from async_handler().

void
async_pore_initialize(PoreQueue *queue,int engine)
{
    pore_queue_create(queue, engine);
    async_edge_handler_setup(pore_async_handler,
                             (void *)queue,
                             queue->irq, SSX_CRITICAL);
    async_edge_handler_setup(pore_async_error_handler,
                             (void *)queue,
                             queue->error_irq, SSX_CRITICAL);
}



