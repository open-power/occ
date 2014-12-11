#ifndef __PGP_ASYNC_H__
#define __PGP_ASYNC_H__

// $Id: pgp_async.h,v 1.2 2014/02/03 01:30:34 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/pgp_async.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pgp_async.h
/// \brief Support for asynchronous request/callback mechanisms
///
/// The data structures defined here provide a 'C' implementation of multiple
/// single-inheritance class hierarchies. The 'subclasses' always include the
/// 'superclass' as the initial element of the structure, allowing subclass
/// pointers to be safely cast to the superclass, and vice-versa (assuming
/// that the subclass is known). One benefit of this approach is that it
/// allows code sharing between requests targeting PORE-GPE, PORE-SW,
/// PBA-BCDE, PBA-BCUE and the deferred callback queue.
///
/// The 'class hierarchy' :
///
/// SsxDeque
///        AsyncRequest
///              PoreRequest
///                      PoreFixed
///                      PoreFlex
///              BceRequest
///              OcbRequest
///              Pbaxrequest
///
/// AsyncQueue
///        PoreQueue
///        BceQueue
///        OcbQueue
///        PbaxQueue
///
/// \bug We may need to redo how we start jobs since writing the EXE-TRIGGER
/// does not guarantee a restart from a good state, although if we get our CCB
/// request through it will.


// PgP Execution engines for the purposes of the generic request mechanism.

#define ASYNC_ENGINE_ANONYMOUS 0x00

#define ASYNC_ENGINE_PORE      0x10
#define ASYNC_ENGINE_PORE_GPE0 0x10
#define ASYNC_ENGINE_PORE_GPE1 0x11
#define ASYNC_ENGINE_PORE_SLW  0x12

#define ASYNC_ENGINE_BCE       0x20
#define ASYNC_ENGINE_BCDE      0x20
#define ASYNC_ENGINE_BCUE      0x21

// Indirect channel 3 no longer supports push/pull queues and they have been
// removed.
#define ASYNC_ENGINE_OCB       0x40
#define ASYNC_ENGINE_OCB_PUSH0 0x41
#define ASYNC_ENGINE_OCB_PUSH1 0x42
#define ASYNC_ENGINE_OCB_PUSH2 0x43
#define ASYNC_ENGINE_OCB_PULL0 0x45
#define ASYNC_ENGINE_OCB_PULL1 0x46
#define ASYNC_ENGINE_OCB_PULL2 0x47

#define ASYNC_ENGINE_PBAX       0x80
#define ASYNC_ENGINE_PBAX_PUSH0 0x81
#define ASYNC_ENGINE_PBAX_PUSH1 0x82


#ifndef __ASSEMBLER__

typedef uint8_t AsyncEngine;

#endif  /* __ASSEMBLER__ */


////////////////////////////////////////////////////////////////////////////
// FFDC Structures
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

/// FFDC from the PLB (OCI) arbiter
///
/// The PLB arbiter records errors from all OCI masters, including the address
/// of the transaction that caused the error.  The PLB arbiter is set up to
/// 'lock' the first error data until the lock is reset.  This structure is
/// included in all of the unit FFDC structures for peripherals that can
/// master on the OCI (SLW/GPE/OCB/PBA).  Note that OCI errors from the 405
/// core will cause an immediate machine check exception.

typedef struct {

    /// PLB arbiter Error Address Register Low
    ///
    /// This is the address of the last PLB timeout or other error recorded in
    /// the PEARL.  This is an error for the unit in question only if the
    /// \a mine data member is non-zero.
    plb_pearl_t pearl;

    /// PLB arbiter Error Status Register
    ///
    /// The PESR at the time of the error.
    plb_pesr_t pesr;

    /// Is the unit in question responsible for the error address recorded in
    /// the PEARL?
    int mine;

} OciFfdc;

void
oci_ffdc(OciFfdc* ffdc, int master_id);

#endif //  __ASSEMBLER__


////////////////////////////////////////////////////////////////////////////
// AsyncRequest
////////////////////////////////////////////////////////////////////////////

/// \defgroup async_request_states ASYNC Request States
///
/// Request states for async requests are group into 5 logical states for
/// error handling and timeout handling purposes.
///
/// - Queued Group : Queued requests are still waiting to execute in the device queue,
/// so these requests can be cancelled simply by removing them from the queue.
///
/// - Running Group : Running requests are running on the hardware, so killing
/// these jobs may require a forced stop and restart of the device.
///
/// - Callback Group : By specification callbacks are always run if provided,
/// even if a job dies or is timed out.  This includes post-processing only
/// work like unblocking blocked threads. These processes are not interrupted.
///
/// - Idle Group : These jobs are no longer in process, and their states are
/// final.
///
/// Only idle requests can be (re)scheduled. Only jobs in the Queued or
/// Running states can time out.  Once the job has moved to the Callback group
/// it is considered complete as far as timeout processing is concerned.
///
/// @{

/// The request is waiting in the queue
#define ASYNC_REQUEST_STATE_QUEUED            0x10


/// The request is running on the hardware
#define ASYNC_REQUEST_STATE_RUNNING           0x20


/// The request is complete but a deferred callback has yet to run
#define ASYNC_REQUEST_STATE_CALLBACK_QUEUED   0x40

/// The request callback is running
#define ASYNC_REQUEST_STATE_CALLBACK_RUNNING  0x41

/// Thread unblocking and/or timeout cancellation has been deferred to a
/// noncritical interrupt context.
#define ASYNC_REQUEST_STATE_POSTPROCESSING    0x42


/// The request has been initialized but never run
#define ASYNC_REQUEST_STATE_INITIALIZED       0x80

/// The request failed due to an error signalled by hardware
#define ASYNC_REQUEST_STATE_FAILED            0x81

/// The request completed normally
#define ASYNC_REQUEST_STATE_COMPLETE          0x82

/// The application cancelled the request
#define ASYNC_REQUEST_STATE_CANCELLED         0x83

/// The request timed out
#define ASYNC_REQUEST_STATE_TIMED_OUT         0x84


#define ASYNC_REQUEST_QUEUED_GROUP            0x10
#define ASYNC_REQUEST_RUNNING_GROUP           0x20
#define ASYNC_REQUEST_CALLBACK_GROUP          0x40
#define ASYNC_REQUEST_IDLE_GROUP              0x80

/// @}


/// \defgroup async_request_options ASYNC Request Options
///
/// These are the option flags for the \a options field of the AsyncRequest.
/// These are generic options applicable to all requests.
///
/// @{

#define ASYNC_CALLBACK_IMMEDIATE   0x0001
#define ASYNC_CALLBACK_DEFERRED    0x0002
#define ASYNC_CALLBACK_NONCRITICAL 0x0004
#define ASYNC_CALLBACK_OPTIONS     0x0007

#define ASYNC_REQUEST_PRIORITY     0x0008
#define ASYNC_CALLBACK_PRIORITY    0x0010

#define ASYNC_REQUEST_BLOCKING     0x0020

#define ASYNC_GENERIC_OPTIONS      0x003f

/// @}

#ifndef __ASSEMBLER__

struct AsyncRequest;
struct AsyncQueue;

typedef int (*AsyncRunMethod)(struct AsyncRequest *request);

typedef int (*AsyncErrorMethod)(struct AsyncRequest *request);

typedef int (*AsyncRequestCallback)(void *);

/// A generic request for the asynchronous request drivers
///
/// Note: Normally the application will not explicitly manipulate this
/// structure, but will instead manipulate derived structures.
///
/// A request is queued for a particular engine, with a callback to be
/// executed once the request has been processed by the engine.  The \a
/// callback may be NULL, but at a minimum the requestor can observe the \a
/// state of the request to determine the state of request processing.  If a
/// deferred callback is requested, then once processing is done the generic
/// handler will requeue the request in a callback queue.
///
/// The \a run_method is a "pure virtual" function of this class.  The \a
/// run_method encapsulates the procedure for starting a job on the
/// engine. The \a run_method may make assumptions about and use data
/// contained in the AsyncQueue class used to queue the requests.
///
/// The \a error_method is also "pure virtual" function of this class.  The \a
/// error_method encapsulates the procedure for collecting FFDC and preparing
/// the engine to run the next job.  The \a error_method may make assumptions
/// about and use data contained in the AsyncQueue class used to queue the
/// requests.  If the \a error_method returns a non-0 code then the error is
/// considered fatal and the queue stops.

typedef struct AsyncRequest {

    /// Generic queue management - the "base class"
    SsxDeque deque;

    /// The time the job was started on the hardware.
    ///
    /// For jobs requiring multiple passes on the hardware, e.g., BCE jobs
    /// that move more data than the PBA supports with a single pass, this
    /// time is the time the first hardware pass started. In this case the
    /// interval time computed as the difference with \a start_time includes
    /// interrupt handling overhead required to process the completion/restart
    /// of intermediate passes.
    ///
    /// This timestamp is inserted into the request by the generic routine
    /// async_request_run().
    SsxTimebase start_time;

    /// The time the job finished on the hardware.
    ///
    /// This timestamp is inserted into the request by the generic routine
    /// async_handler().
    SsxTimebase end_time;

    /// A semaphore for thread-mode requests to block on if desired.
    SsxSemaphore sem;

    /// A timer used for timeout management
    SsxTimer timer;

    /// The engine queue the request is/was scheduled on
    struct AsyncQueue *queue;

    /// The "virtual" run method for the class
    AsyncRunMethod run_method;

    /// The "virtual" error handler method for the class
    AsyncErrorMethod error_method;

    /// The routine Called (or deferred) with the \a arg parameter once
    /// the engine has completed the request.
    ///
    /// The callback may be NULL (0), in which case the \a arg parameter
    /// is ignored, and the only status available to the requestor is the
    /// request \a state.
    AsyncRequestCallback callback;

    /// The argument of the callback
    void *arg;

    /// The timeout value
    ///
    /// AsyncRequest objects with \a timeout other than SSX_WAIT_FOREVER are
    /// governed by a private watchdog timer that will cancel a queued job or
    /// kill a running job if the hardware operation does not complete before
    /// it times out.
    SsxInterval timeout;

    /// The return value of the callback (if any) is stored here
    ///
    /// The success or failure of the callback is recorded here; it is not
    /// recorded in the \a state variable.  If there is no callback this field
    /// will always read as 0 at job completion.
    int callback_rc;

    /// Options controlling request processing
    uint8_t options;

    /// The current state of the request
    ///
    /// This field is declared volatile because applications may poll this
    /// field which is set asynchronously by the async interrupt handlers, and
    /// optimizing compilers transform polling a variable into an infinite
    /// loop if the variable is not set on the initial test!
    volatile uint8_t state;

    /// The state of the request when the request was aborted
    ///
    /// This field is valid when the state of the request is read as
    /// ASYNC_REQUEST_STATE_FAILED, ASYNC_REQUEST_STATE_CANCELLED, or
    /// ASYNC_REQUEST_STATE_TIMED_OUT.  This field records the state of the
    /// job when it was forceably cancelled or killed either due to the
    /// application's request or due to a hardware error or timeout.
    uint8_t abort_state;

    /// The completion state of the request.
    ///
    /// This is the state that will be reported when the request completes and
    /// the callback has been run. Normally this is
    /// ASYNC_REQUEST_STATE_COMPLETE. However the specification requires that
    /// even jobs that terminate due to an errors, timeouts or being cancelled
    /// or killed must run their callbacks. In this case this variable will be
    /// set to ASYNC_REQUEST_STATE_FAILED, ASYNC_REQUEST_STATE_TIMED_OUT,
    /// ASYNC_REQUEST_STATE_CANCELLED or ASYNC_REQUEST_STATE_KILLED
    /// respectively.
    uint8_t completion_state;

} AsyncRequest;


int
async_request_create(AsyncRequest *request,
                     struct AsyncQueue *queue,
                     AsyncRunMethod run_method,
                     AsyncErrorMethod error_method,
                     SsxInterval timeout,
                     AsyncRequestCallback callback,
                     void *arg,
                     int options);

void
async_request_finalize(AsyncRequest* request);


/// Check that an asynchronous request is idle
///
/// \param request An initialized request
///
/// A request is considered idle if it is not attached to any of the
/// asynchronous request queues, \e or the request has terminated with an
/// error.  This includes requests that have never been scheduled, have
/// completed or been cancelled or killed. Only idle requests can be
/// rescheduled.
///
/// \retval 0 The request is not idle
///
/// \retval 1 The request is idle

static inline int
async_request_is_idle(AsyncRequest *request)
{
    return (request->state & ASYNC_REQUEST_IDLE_GROUP) != 0;
}


/// Check an asynchronous request for successful completion
///
/// \param request A request that had been previosuly scheduled
///
/// Note that a request is not considered complete until both the engine job
/// has finshed without error and any callback has run to completion. Thus
/// jobs that have error-ed out, been cancelled or killed will be idle (and
/// rescheduleable), but not complete.
///
/// \retval 0 The request has not yet completed successfully
///
/// \retval 1 The request has completed successfully.

static inline int
async_request_completed(AsyncRequest *request)
{
    return (request->state == ASYNC_REQUEST_STATE_COMPLETE);
}


int
async_request_timestamps_get(AsyncRequest* request,
                             SsxTimebase* start_time,
                             SsxTimebase* end_time);

int
async_request_latency(AsyncRequest* request, SsxTimebase* latency);


void
async_request_printk(AsyncRequest *request);


#endif  /* __ASSEMBLER__ */


////////////////////////////////////////////////////////////////////////////
// AsyncQueue
////////////////////////////////////////////////////////////////////////////

#define ASYNC_QUEUE_STATE_IDLE    0x00
#define ASYNC_QUEUE_STATE_RUNNING 0x01
#define ASYNC_QUEUE_STATE_ERROR   0x02

#ifndef __ASSEMBLER__

/// A generic asynchronous request queue
///
/// Note: This structure is normally not manipulated directly by application
/// code, but only by the device drivers.
///
/// Request queues support 2 priorities - Normal and High. Normal priority
/// requests are queued FIFO order, while high-priority requests are queued
/// LIFO order. Requests queued with high priority always execute before any
/// queued with normal priority, however there is no concept of preemption of
/// a currently running request.
///
/// Because high-priority requests are queued in LIFO order, multiple
/// high-priority jobs will run in the reverse order from which they were
/// enqueued.

typedef struct AsyncQueue {

    /// The sentinel of the request queue
    SsxDeque deque;

    /// The currently running request, or NULL (0)
    AsyncRequest* current;

    /// The engine associated with the queue.
    AsyncEngine engine;

    /// The state of the queue
    uint8_t state;

} AsyncQueue;


int
async_queue_create(AsyncQueue* queue, AsyncEngine engine);

void
async_handler(AsyncQueue* queue);

int
async_request_schedule(AsyncRequest* request);

void
async_error_handler(AsyncQueue* queue, uint8_t completion_state);

#endif  /* __ASSEMBLER__ */


////////////////////////////////////////////////////////////////////////////
// Async Initialization
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

void
async_edge_handler_setup(SsxIrqHandler handler,
                         void *arg,
                         SsxIrqId irq,
                         int priority);

void
async_level_handler_setup(SsxIrqHandler handler,
                          void *arg,
                          SsxIrqId irq,
                          int priority,
                          int polarity);

void
async_callbacks_initialize(SsxDeque *queue, SsxIrqId irq);


#endif // __ASSEMBLER__


////////////////////////////////////////////////////////////////////////////
// PoreRequest
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

struct PoreQueue;

/// PORE FFDC
///
/// The PORE engine has 208 bytes of programmer-visible state - too much to
/// allocate in every PoreRequest on the off-chance that a request may fail.
/// This PoreFfdc structure is designed to capture a reasonble amount of data
/// in the case of failure of a PoreFlex request, which does not include any
/// PORE error handlers.  This structure is currently 48 bytes.
///
/// The most common recoverable errors are expected to be erroneous PIB
/// responses from deconfigured (garded) cores.  These will show up as Error
/// event 0 and can be debugged from the debug registers.  We also include the
/// instruction buffer registers to help debug error 2 - instruction
/// fetch/decode errors.
///
/// To get a full picture in the event of OCI execution phase errors (error
/// 1), the PORE memory-space address registers are captured.  We also capture
/// FFDC from the PLB arbiter, which can be used to debug illegal address-type
/// problems. 

typedef struct {

    /// FFDC from the PLB (OCI) arbiter
    OciFfdc oci_ffdc;

    /// PORE Debug Registers
    /// 
    /// - [0] Contains PIB address and PIB return code
    /// - [1] Contains failing PC and error status bits
    uint64_t debug[2];

    /// PORE Memory-space address registers.  
    ///
    /// We only save the low-order 32 bits of each - the high-order bits are
    /// implied/ignored for OCI-attached engines.  If OCC managed the SBE then
    /// we would require the high-order bits in an FFDC dump.
    uint32_t address[2];

    /// PORE Instruction buffer 0-2
    ///
    /// - [0] Contains the opcode and register/short operands
    /// - [1:2] Contain the 64-bit immediate
    uint32_t ibuf[3];

} PoreFfdc;


/// A PORE branch immediate address instruction, used in PORE jump tables.
typedef struct {
    uint32_t word[3];
} PoreBraia;


void
pore_braia_create(PoreBraia* instr, uint32_t address);


/// A request to run a PORE program
///
/// A PORE request extends the generic AsyncRequest request by the addition of
/// several fields required to be set up in the engine before the job is run,
/// including the program parameter for the routine.  The PoreRequest is an
/// internal class that is re-typed to create the PoreFixed and PoreFlex
/// request classes, which differ only slightly in their behavior.
///
/// As long as the request is known to be idle the application is free to
/// change the \a parameter value between executions of the PORE program,
/// e.g., to do ping-pong buffer management.  None of the other fields should
/// be directly modified by the application.

typedef struct {

    /// The generic request
    AsyncRequest request;

    /// Error information
    PoreFfdc ffdc;

    /// The PORE Jump Table
    PoreBraia* table;

    /// The initial value of the high-order 32 bits of the ERROR_MASK register
    uint32_t error_mask;

    /// The entry point address of the routine.
    ///
    /// For PoreFlex this entry point will be non-0 and will be inserted into
    /// D0, as PoreFlex jobs are kicked off by BRAD.  For PoreFixed this
    /// parameter will be zero and ignored.
    uint32_t entry_point;

    /// The single parameter of the PORE program - EXE-Trigger[32:63]
    uint32_t parameter;

    /// The high-order 32 bits of EXE-Trigger used to kick off the program.
    ///
    /// For PoreFlex this field is always 0, as PoreFlex only uses table entry
    /// 0 which contains a BRAD.  For PoreFixed this encodes the address of
    /// the table entry to kick off.
    uint32_t exe_trigger;

} PoreRequest;


typedef PoreRequest PoreFlex;

typedef PoreRequest PoreFixed;

int
pore_run_method(AsyncRequest* request);

int
pore_error_method(AsyncRequest* request);

#endif  /* __ASSEMBLER__ */

/// The (fixed) number of PORE jump table error handler slots
#define PORE_ERROR_SLOTS 5

/// The maximum number of PORE jump table EXE_TRIGGER slots
#define PORE_TRIGGER_SLOTS 16

/// Compute the address of error handler BRAIA entry n of a PORE branch table
#define PORE_ERROR_BRANCH(table, n) ((table) + ((n) * 12))

/// Compute the address of entry point BRAIA entry n of a PORE branch table
#define PORE_ENTRY_BRANCH(table, n) \
    ((table) + (((PORE_ERROR_SLOTS) + (n)) * 12))


////////////////////////////////////////////////////////////////////////////
// PoreFlex
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

int
pore_flex_create(PoreFlex* request,
                 struct PoreQueue* queue,
                 PoreEntryPoint entry_point,
                 uint32_t parameter,
                 SsxInterval timeout,
                 AsyncRequestCallback callback,
                 void *arg,
                 int options);


/// See async_request_schedule() for documentation.
static inline int
pore_flex_schedule(PoreFlex* request)
{
    return async_request_schedule((AsyncRequest *)request);
}

#endif  /* __ASSEMBLER__ */


////////////////////////////////////////////////////////////////////////////
// PoreFixed
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

int
pore_fixed_create(PoreFixed *request,
                  struct PoreQueue *queue,
                  PoreBraia* table,
                  uint32_t error_mask,
                  int start_vector,
                  uint32_t parameter,
                  SsxInterval timeout,
                  AsyncRequestCallback callback,
                  void *arg,
                  int options);

/// See async_request_schedule() for documentation.
static inline int
pore_fixed_schedule(PoreFixed *request)
{
    return async_request_schedule((AsyncRequest *)request);
}

#endif  /* __ASSEMBLER__ */


////////////////////////////////////////////////////////////////////////////
// PoreQueue
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

/// A PORE engine queue
///
/// A PORE queue consists of a generic AsyncQueue to manage jobs on the
/// engine, the OCI base address of the PORE control register space of the
/// engine, and interrupt information.

typedef struct PoreQueue {

    /// The generic request queue - the "base class"
    AsyncQueue queue;

    /// The base address of the OCI control register space for this engine.
    uint32_t oci_base;

    /// The IRQ associated with normal completion on the engine
    ///
    /// \todo Due to header reference ordering we can't define this as SsxIrqId
    uint8_t irq;

    /// The IRQ associated with error completion on the engine
    ///
    /// \todo Due to header reference ordering we can't define this as SsxIrqId
    uint8_t error_irq;

    /// The OCI master number of this engine; See pore_error_method()
    uint8_t oci_master;

} PoreQueue;


int
pore_queue_create(PoreQueue *queue,
                  int engine);

#endif  /* ASSEMBLER */


////////////////////////////////////////////////////////////////////////////
// PBA FFDC Structures
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

/// Common FFDC collected for each PBA subunit in the event that a particular
/// subunit is implicated in an error.  This structure contains generic
/// data that may be useful in diagnosing any PBA error.  Instances of this
/// structure are embedded in higher-level structures for the PBA bridge, BCE
/// engines, and PBAX mechanism.

typedef struct {

    /// FFDC from the PLB (OCI) arbiter
    OciFfdc oci_ffdc;

    /// The PBA MODE register
    pba_mode_t mode;

    /// The PBA FIR
    pba_fir_t fir;

    /// PBA Error Report 0
    pba_errpt0_t errpt0;

    /// PBA Error Report 1
    pba_errpt1_t errpt1;

    /// PBA Error Report 2
    pba_errpt2_t errpt2;

    /// PBA Read Buffer Valid Status
    pba_rbufvaln_t rbufval[PBA_READ_BUFFERS];

    /// PBA Write Buffer Valid Status
    pba_wbufvaln_t wbufval[PBA_WRITE_BUFFERS];

    /// PBA BARs
    pba_barn_t bar[PBA_BARS];

    /// PBA BAR masks
    pba_barmskn_t barmsk[PBA_BARS];

    /// Error status - non-0 if this structure contains error data
    int error;

} PbaCommonFfdc;


/// FFDC collected for generic PBA bridge errors
///
/// These types of errors will almost certainly be attributable to the
/// PORE-SLW or PORE-GPE, since the OCC is normally not allowed to perform
/// direct bridge operations after OCC initialization.  This structure extends
/// the common PBA FFDC with information on how the slaves were programmed.

typedef struct {

    /// Common FFDC
    PbaCommonFfdc common;

    /// PBA Slave reset register
    pba_slvrst_t slvrst;

    /// PBA Slave control registers
    pba_slvctln_t slvctl[PBA_SLAVES];

} PbaBridgeFfdc;


/// FFDC collected for the PBA BCUE/BCDE
///
/// The BCDE/BCUE control and status registers have identical layouts.
/// Similar to the way the drivers are coded, the BCDE register forms are used
/// to declare the structures.

typedef struct {

    /// Common FFDC
    PbaCommonFfdc common;

    /// BCE control register
    pba_bcde_ctl_t ctl;

    /// BCE setup register
    pba_bcde_set_t set;

    /// BCE PowerBus setup register
    pba_bcde_pbadr_t pbadr;

    /// BCE status register
    pba_bcde_stat_t stat;

} BceFfdc;


/// FFDC collected for PBAX send errors

typedef struct {

    /// Common FFDC
    PbaCommonFfdc common;

    /// PBAX configuration register
    pba_xcfg_t xcfg;

    /// PBAX send transaction register
    pba_xsndtx_t xsndtx;

    /// PBAX send status register
    pba_xsndstat_t xsndstat;

} PbaxSendFfdc;


/// FFDC collected for PBAX receive errors

typedef struct {

    /// Common FFDC
    PbaCommonFfdc common;

    /// PBAX configuration register
    pba_xcfg_t xcfg;

    /// PBAX receive status register
    pba_xrcvstat_t xrcvstat;

    /// PBAX push base registers
    pba_xshbrn_t xshbrn[PBAX_QUEUES];

    /// PBAX push control/status registers
    pba_xshcsn_t xshcsn[PBAX_QUEUES];

} PbaxReceiveFfdc;


/// A single global structure containing FFDC for all PBA functions

typedef struct {

    /// FFDC for the generic bridge
    PbaBridgeFfdc bridge;

    /// FFDC for the BCDE
    BceFfdc bcde;

    /// FFDC for the BCUE
    BceFfdc bcue;

    /// FFDC for PBAX send
    PbaxSendFfdc pbax_send;
    
    /// FFDC for PBAX receive
    PbaxReceiveFfdc pbax_receive;
    
} PbaUnitFfdc;

#endif  // __ASSEMBLER__

////////////////////////////////////////////////////////////////////////////
// BceRequest
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

struct BceQueue;

/// A request to move data through the PBA BCUE or BCDE
///
/// From a programming and control perspective the Block Copy Download Engine
/// (BCDE) and Upload Engine (BCUE) are identical - they simply move a given
/// number of 128-byte cache lines from/to system memory to/from SRAM.
///
/// Although the PBA BCE hardware can only move a maximum of 4KB at a time
/// through the channel, the software layer allows any amount of data to be
/// moved as a single request.

typedef struct {

    /// The generic request
    AsyncRequest request;

    /// FFDC collected in the event of an error in the request
    BceFfdc ffdc;

    /// Initial bridge address for read (BCDE) or write (BCUE) data.  This
    /// field is not modified by the drivers.
    uint32_t bridge_address;

    /// Initial OCI address for read (BCUE) or write (BCDE) data.  This
    /// field is not modified by the drivers.
    uint32_t oci_address;

    /// Number of bytes to move. This field is not modified by the drivers.
    ///
    /// Note that the PBA moves data in sets of 128 bytes (the PowerBus
    /// cache line size).  This field will always be a multiple of 128.
    size_t bytes;

    /// The next bridge address for read (BCDE) or write (BCUE) data.  This
    /// field is modified by the drivers as each maximum 4K block is copied.
    uint32_t next_bridge_address;

    /// Initial OCI address for read (BCUE) or write (BCDE) data.  This
    /// field is modified by the drivers as each maximum 4K block is copied.
    uint32_t next_oci_address;

    /// The number of bytes remaining to be moved.
    size_t remaining;

    /// The extended address, as a 64-bit PowerBus address
    ///
    /// Bits 23:36 of the field define bits 23:36 of the final PowerBus
    /// address, subject to the final mask selection. This field is cleared by
    /// the default constructor, and must be explicitly set if an extended
    /// address is needed.
    pba_extended_address_t extended_address;

} BceRequest;

int
bce_request_create(BceRequest *request,
                   struct BceQueue *queue,
                   uint32_t bridge_address,
                   uint32_t oci_address,
                   size_t bytes,
                   SsxInterval timeout,
                   AsyncRequestCallback callback,
                   void *arg,
                   int options);

#endif  /* __ASSEMBLER__ */


////////////////////////////////////////////////////////////////////////////
// BceQueue
////////////////////////////////////////////////////////////////////////////

// NB: This assignment ordering is assumed by static initialization code.
// These constants are used as array indices.

#define BCE_ENGINE_BCDE 0
#define BCE_ENGINE_BCUE 1

#define BCE_ENGINES 2

#ifndef __ASSEMBLER__


/// A PBA Block Copy Engine queue
///
/// A PBA block copy request queue consists of a generic AsyncQueue to manage
/// jobs on the engine and the PBA engine id. The BCDE and BCUE can use the
/// same driver because their control interfaces are identical.

typedef struct BceQueue {

    /// The generic request queue
    AsyncQueue queue;

    /// The engine id of the BCE engine for control register address lookup
    int engine;

} BceQueue;


int
bce_queue_create(BceQueue *queue,
                 int engine);


int
bce_request_schedule(BceRequest *request);

#endif  /* __ASSEMBLER__ */


////////////////////////////////////////////////////////////////////////////
// PbaxRequest
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

struct PbaxQueue;

/// A request to move data through a PBAX circular queue
///
/// The PBAX circular queues are modeled after the OCB circular queues.  The
/// queues are named and documented in the hardware specifications from the
/// perspective of the PowerBus as "PUSH" queues.  Here we name and document
/// the queues from the perspective of OCC firmware as "READ" queues. Note
/// that there are no PBAX "WRITE" queues. The PBAX write mechanism handles
/// only a single datum at a time and has no interrupt.  PBAX writes are
/// handled by the pbax_send() and _pbax_send() APIs.
///
/// The PBAX queues support data buffers of from 8 to 256 bytes with an 8-byte
/// granularity.  The PBAX hardware requires that the data buffers be aligned
/// to a power-of-2 boundary greater than or equal to the buffer size.
/// The async drivers allow specification of any multiple of 8 bytes to read
/// and handle all of the queue and split-request management.
///
/// Note : As long as the PbaxRequest is idle, the application is free to
/// directly change the \a data and \a bytes fields to read/write different
/// numbers of bytes to/from different locations the next time the request is
/// scheduled.
///
/// The PBAX queues support both 'lazy' and 'aggressive' interrupt protocols.
/// The aggressive protocols generate interrupts whenever read data is
/// available.  The lazy protocols require a full read buffer to trigger an
/// interrupt. A lazy read protocol with a buffer size > 1 demands that
/// communcations always consist of a known number of doublewords equal to the
/// buffer size. For free-form communications an aggressive read protocol must
/// be used.

typedef struct {

    /// The generic request
    AsyncRequest request;

    /// Initial pointer to the data area to hold read data.
    ///
    /// This field is not modified by the drivers.  The application may modify
    /// this field any time the PbaxRequest is idle.
    uint64_t *data;

    /// Number of bytes to move, which must be a multiple of 8.
    ///
    /// This field is not modified by the drivers.  The application may modify
    /// this field any time the PbaxRequest is idle.
    size_t bytes;

    /// Pointer to where to put the next read data.
    ///
    /// The application should never modify this field.
    void* current;

    /// Number of bytes remaining to be moved.
    ///
    /// The application should never modify this field.
    size_t remaining;

} PbaxRequest;


int
pbax_request_create(PbaxRequest *request,
                    struct PbaxQueue *queue,
                    uint64_t *data,
                    size_t bytes,
                    SsxInterval timeout,
                    AsyncRequestCallback callback,
                    void *arg,
                    int options);


int
pbax_request_schedule(PbaxRequest *request);

#endif  /* __ASSEMBLER__ */


////////////////////////////////////////////////////////////////////////////
// PbaxQueue
////////////////////////////////////////////////////////////////////////////

// NB: This assignment ordering is assumed by static initialization code in
// pgp_async.c - these constants are used as array indices.

#define PBAX_ENGINE_PUSH0 0
#define PBAX_ENGINE_PUSH1 1

#define PBAX_ENGINES 2

#ifndef __ASSEMBLER__

extern const SsxAddress pba_xshcsn[];
extern const SsxAddress pba_xshbrn[];
extern const SsxAddress pba_xshincn[];


/// A PBAX Circular buffer queue
///
/// A PBAX circular buffer queue consists of a generic AsyncQueue to manage
/// jobs on the engine, a pointer to the data area and the PBAX engine id.

typedef struct PbaxQueue {

    /// The generic request queue
    AsyncQueue queue;

    /// The base of the circular queue data area
    ///
    /// This data area must satisfy stringent alignment constraints; See the
    /// documentation for PbaxRequest.
    uint64_t *cq_base;

    /// The number of 8-byte entries in the queue.
    ///
    /// This is included here to simplify APIs; The length is also encoded in
    /// the PBAXSHCSn register for the queue.
    size_t cq_entries;

    /// The interrupt protocol implemented by the queue.
    ///
    /// This will either be PBAX_INTERRUPT_PROTOCOL_LAZY or
    /// PBAX_INTERRUPT_PROTOCOL_AGGRESSIVE.
    int protocol;

    /// The engine id for lookup of OCI control register addresses.
    int engine;

    /// The IRQ associated with normal completion on the engine
    ///
    /// \todo Due to header reference ordering we can't define this as SsxIrqId
    uint8_t irq;

} PbaxQueue;


int
pbax_queue_create(PbaxQueue *queue,
                  int engine,
                  uint64_t *cq_base,
                  size_t cq_entries,
                  int protocol);

int
pbax_queue_disable(PbaxQueue *queue);

int
pbax_queue_enable(PbaxQueue *queue);

int
pbax_read(PbaxQueue* queue, void* buf, size_t bytes, size_t* read);

#endif  /* __ASSEMBLER__ */


////////////////////////////////////////////////////////////////////////////
// OcbRequest
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

/// OCB FFDC
///
/// This structure is used to store FFDC in two cases. 1) When an error can be
/// reasonably attributed to an OCB channel under the control of OCC, and 2)
/// general errors signalled by the OCB direct bridge.  Due to the myriad ways
/// that OCB channels can be used the FFDC for OCB direct bridge errors is
/// stored globally rather than with particular requests or queues.  
///
/// The OcbFfdc includes the PLB arbiter PEARL and PESR register state.
/// This state is only collected if the error status indicates an OCI timeout
/// for this channel.
///
/// Any error reasonably attributable to a channel will capture the FFDC and
/// then disable the channel since any further communication through the
/// channel must be considered compromised.

typedef struct {

    /// FFDC from the OCI (PLB) arbiter
    OciFfdc oci_ffdc;

    /// A copy of the OCB OCC_LFIR register at the time of the error 
    ocb_occlfir_t fir;

    /// A copy of the OCB Control/Status register for the channel at the time
    /// of the error.
    ///
    /// This field will be set to 0 for generic OCB bridge errors
    ocb_ocbcsrn_t csr;

    /// A copy of the OCB Stream Push Base [n] Register at the time of the
    /// failure
    ///
    /// This field will be set to 0 for generic OCB bridge errors
    ocb_ocbshbrn_t shbr;

    /// A copy of the OCB Stream Push Control/Status [n] Register at the time
    /// of the failure
    ///
    /// This field will be set to 0 for generic OCB bridge errors
    ocb_ocbshcsn_t shcs;

    /// A copy of the OCB Stream Pull Base [n] Register at the time of the
    /// failure
    ///
    /// This field will be set to 0 for generic OCB bridge errors
    ocb_ocbslbrn_t slbr;

    /// A copy of the OCB Stream Pull Control/Status [n] Register at the time
    /// of the failure
    ///
    /// This field will be set to 0 for generic OCB bridge errors
    ocb_ocbslcsn_t slcs;

    /// Is this record valid (does it contain captured error information)?
    int error;

} OcbFfdc;


/// OCB Unit FFDC
///
/// Contains FFDC structures for each channel and the direct bridge

typedef struct {

    OcbFfdc channel[OCB_INDIRECT_CHANNELS];
    OcbFfdc bridge;

} OcbUnitFfdc;


/// Global FFDC for OCB

extern OcbUnitFfdc G_ocb_ffdc;


struct OcbQueue;

/// A request to move data through an OCB circular queue
///
/// The OCB circular queues are named and documented in the hardware specs
/// from the perspective of the PIB - "PUSH" and "PULL" queues.  Here we name
/// and document the queues from the perspective of OCC firmware - "READ" and
/// "WRITE" queues. The request is generic and becomes either a read or write
/// request simply by which queue it is scheduled on.
///
/// The circular queues only support 8-byte granularity, require that the data
/// areas be 8- or 32-byte aligned, and only support up to 256 byte
/// queues. However the async drivers allow specification of any multiple of 8
/// bytes to read or write, and handle all of the queue and split-request
/// management.
///
/// Note : As long as the OcbRequest is idle, the application is free to
/// directly change the \a data and \a bytes fields to read/write different
/// numbers of bytes to/from different locations the next time the request is
/// scheduled.
///
/// OCB requests always complete in an interrupt context.  The OCB queues
/// support both 'lazy' and 'aggressive' interrupt protocols.  The aggressive
/// protocols generate interrupts whenever read data is available or write
/// space is available.  The lazy protocols require a full read buffer or
/// empty write buffer to trigger an interrupt. A lazy read protocol with a
/// buffer size > 1 demands that communcations always consist of a known
/// number of doublewords. For free-form communications an aggressive read
/// protocol must be used. On average, an aggressive write protocol will
/// shorten blocking periods of writers, and can guarantee that data is in the
/// queue whenever data is avilable. Lazy write protocols will be preferred
/// when reducing interrupt overhead is more important than reducing blocking
/// time.
///
/// Completion of write requests does not guarantee that the communication
/// partner has received the data, but simply that the data has been queued
/// for reception and the caller may reuse/refill the data buffer if required.

typedef struct {

    /// The generic request
    AsyncRequest request;

    /// Initial pointer to the data to be moved for write data, or the
    /// data area to hold read data. This field is not modified by the drivers.
    uint64_t *data;

    /// Number of bytes to move. This field is not modified by the drivers.
    size_t bytes;

    /// Pointer to data yet to be moved for write, or where to put the next
    /// read data.
    uint64_t* current;

    /// Number of bytes remaining to be moved.
    size_t remaining;

} OcbRequest;


int
ocb_request_create(OcbRequest *request,
                   struct OcbQueue *queue,
                   uint64_t *data,
                   size_t bytes,
                   SsxInterval timeout,
                   AsyncRequestCallback callback,
                   void *arg,
                   int options);


int
ocb_request_schedule(OcbRequest *request);

#endif  /* __ASSEMBLER__ */


////////////////////////////////////////////////////////////////////////////
// OcbQueue
////////////////////////////////////////////////////////////////////////////

// NB: This assignment ordering is assumed by static initialization code in
// pgp_async.c - these constants are used as array indices.  The code also
// assumes this ordering for the access of G_ocb_ocbsesn[], and for
// determining whether the engine is a PUSH or PULL queue.
// Note:  push/pull queues for channel 3 have been deleted 

#define OCB_ENGINE_PUSH0 0
#define OCB_ENGINE_PULL0 1
#define OCB_ENGINE_PUSH1 2
#define OCB_ENGINE_PULL1 3
#define OCB_ENGINE_PUSH2 4
#define OCB_ENGINE_PULL2 5

#define OCB_ENGINES 6

#ifndef __ASSEMBLER__

/// An OCB Circular buffer queue
///
/// A OCB circular buffer queue consists of a generic AsyncQueue to manage
/// jobs on the engine, a pointer to the data area and the OCB engine id.

typedef struct OcbQueue {

    /// The generic request queue
    AsyncQueue queue;

    /// The base of the circular queue data area - must be 8-byte aligned
    uint64_t *cq_base;

    /// The length of the queue in terms of the number of 8-byte entries in
    /// the queue.
    ///
    /// This is for informational purposes only.  The length is also encoded
    /// in the OCBSxCSn register for the queue.
    size_t cq_length;

    /// The engine id for lookup of OCI control register addresses.
    int engine;

    /// The IRQ associated with normal completion on the engine
    ///
    /// \todo Due to header reference ordering we can't define this as SsxIrqId
    uint8_t irq;

} OcbQueue;


int
ocb_queue_create(OcbQueue *queue,
                 int engine,
                 uint64_t *cq_base,
                 size_t cq_length,
                 int protocol);

#endif  /* __ASSEMBLER__ */


////////////////////////////////////////////////////////////////////////////
// Miscellaneous/Initialization
////////////////////////////////////////////////////////////////////////////

// Error codes

#define ASYNC_INVALID_OBJECT_REQUEST         0x00279600
#define ASYNC_INVALID_OBJECT_SCHEDULE        0x00279601
#define ASYNC_INVALID_OBJECT_OCB_REQUEST     0x00279602
#define ASYNC_INVALID_OBJECT_OCB_QUEUE       0x00279603
#define ASYNC_INVALID_OBJECT_PBAX_REQUEST    0x00279604
#define ASYNC_INVALID_OBJECT_PBAX_DISABLE    0x00279605
#define ASYNC_INVALID_OBJECT_PBAX_QUEUE      0x00279606
#define ASYNC_INVALID_OBJECT_BCE_REQUEST     0x00279607
#define ASYNC_INVALID_OBJECT_BCE_QUEUE       0x00279608
#define ASYNC_INVALID_OBJECT_PORE_REQUEST    0x00279609
#define ASYNC_INVALID_OBJECT_PORE_QUEUE      0x0027960a
#define ASYNC_INVALID_OBJECT_PTS_REQUEST     0x0027960b
#define ASYNC_INVALID_OBJECT_PTS_THREAD      0x0027960c
#define ASYNC_INVALID_OBJECT_PTS_QUEUE       0x0027960d
#define ASYNC_INVALID_OBJECT_PTS_START       0x0027960e
#define ASYNC_INVALID_OBJECT_PTS_SCHEDULE    0x0027960f
#define ASYNC_INVALID_ARGUMENT               0x00279610
#define ASYNC_INVALID_ARGUMENT_OCB_READ      0x00279611
#define ASYNC_INVALID_ARGUMENT_OCB_WRITE     0x00279612
#define ASYNC_INVALID_ARGUMENT_OCB_QUEUE     0x00279613
#define ASYNC_INVALID_ARGUMENT_OCB_QUEUE2    0x00279614
#define ASYNC_INVALID_ARGUMENT_OCB_REQUEST   0x00279615
#define ASYNC_INVALID_ARGUMENT_OCB_SCHEDULE  0x00279616
#define ASYNC_INVALID_ARGUMENT_BCE_SCHEDULE  0x00279617
#define ASYNC_INVALID_ARGUMENT_PBAX_READ     0x00279618
#define ASYNC_INVALID_ARGUMENT_PBAX_REQUEST  0x00279619
#define ASYNC_INVALID_ARGUMENT_PBAX_SCHEDULE 0x0027961a
#define ASYNC_INVALID_ARGUMENT_PBAX_QUEUE    0x0027961b
#define ASYNC_INVALID_ARGUMENT_PORE_REQUEST  0x0027961c
#define ASYNC_INVALID_ARGUMENT_PTS_THREAD    0x0027961d
#define ASYNC_INVALID_ARGUMENT_PTS_REQUEST   0x0027961e
#define ASYNC_INVALID_ENGINE_OCB             0x0027961f
#define ASYNC_INVALID_ENGINE_PBAX            0x00279620
#define ASYNC_INVALID_ENGINE_BCE             0x00279621
#define ASYNC_INVALID_ENGINE_PORE            0x00279622
#define ASYNC_INVALID_ENGINE_PTS             0x00279623
#define ASYNC_INVALID_OPTIONS                0x00279624
#define ASYNC_INVALID_ASSIGNMENT             0x00279625
#define ASYNC_CALLBACK_PROTOCOL_UNSPECIFIED  0x00279626
#define ASYNC_REQUEST_NOT_IDLE               0x00279627
#define ASYNC_REQUEST_NOT_IDLE_PTS           0x00279628
#define ASYNC_REQUEST_COMPLETE               0x00279629
#define ASYNC_INVALID_TIMESTAMPS             0x0027962a
#define ASYNC_OCB_ERROR_READ_OLD             0x0027962b
#define ASYNC_OCB_ERROR_READ_NEW             0x0027962c
#define ASYNC_OCB_ERROR_WRITE_OLD            0x0027962d
#define ASYNC_OCB_ERROR_WRITE_NEW            0x0027962e
#define ASYNC_PBAX_ERROR_OLD                 0x0027962f
#define ASYNC_PBAX_ERROR_NEW                 0x00279630
#define ASYNC_REQUEST_NOT_COMPLETE           0x00279631
#define ASYNC_REQUEST_NOT_COMPLETE_PTS       0x00279632

// Panic codes

#define ASYNC_PORE_FIXED_INVARIANT           0x00279633
#define ASYNC_PHANTOM_INTERRUPT              0x00279634
#define ASYNC_PHANTOM_INTERRUPT_OCB          0x00279635
#define ASYNC_PHANTOM_INTERRUPT_BCE          0x00279636
#define ASYNC_PHANTOM_INTERRUPT_PBAX         0x00279637
#define ASYNC_SCOM_ERROR                     0x00279638
#define ASYNC_TIMEOUT_BUG                    0x00279639
#define ASYNC_INVALID_STATE                  0x0027963a
#define ASYNC_PHANTOM_ERROR                  0x0027963b
#define ASYNC_BUG_PORE_AT_CREATE             0x0027963c
#define ASYNC_BUG_PTS_AT_CREATE              0x0027963d
#define ASYNC_BUG_PTS_AT_RUN                 0x0027963e

////////////////////////////////////////////////////////////////////////////
// Global Data and Constants
////////////////////////////////////////////////////////////////////////////

#ifndef __ASSEMBLER__

// PORE Queues

extern PoreQueue G_pore_gpe0_queue;
extern PoreQueue G_pore_gpe1_queue;
extern PoreQueue G_pore_slw_queue;

/// Define a PORE branch table.  All error slots are always defined, but space
/// can be saved if not all of the entry points are required.
#define PORE_TABLE(var, slots) PoreBraia var[PORE_ERROR_SLOTS + (slots)]


// PTS Queues
//
// These queues are only defined if CONFIGURE_PTS is non-zero. CONFIGURE_PTS
// is 0 by default.

#ifndef CONFIGURE_PTS
#define CONFIGURE_PTS 0
#endif

#if CONFIGURE_PTS
#include "pgp_async_pts.h"
extern PtsQueue G_pts_gpe0_queue;
extern PtsQueue G_pts_gpe1_queue;
#endif


// OCB Queues and FFDC

// OCB circular queue naming is confusing, because PUSH/PULL are defined from
// the perspective of the PIB.  This driver uses the terms read/write
// respectively and represents the hardware from the perspective of code
// running on OCC.

// OCB read/write queue lengths are defined in terms of doublewords, and must
// be in a small defined range.  Two meaningful interrupt protocols are
// possible for a queue: 'LAZY' allows read/write queues to fill/empty
// respectively before an interrupt is signalled.  'AGRESSIVE' signals an
// interrupt whenever there is data to be read or a free slot to put write
// data.

// In a production system the lengths and protocols will likely vary based on
// the particular application and the characteristics of the communication
// partner. By default the queues take on their maximum lengths.  Read queues
// use the aggressive protocol which allows free-form communication.  Write
// queues use lazy protoclols which reduces interrupt overhead.  Different
// effective lengths and the interrupt protocols can be changed later if
// desired - assuming the queues are idle when the changes are made.

#define OCB_PUSH_PULL_LENGTH_MIN 1
#define OCB_PUSH_PULL_LENGTH_MAX 32

#define OCB_INTERRUPT_PROTOCOL_LAZY       0
#define OCB_INTERRUPT_PROTOCOL_AGGRESSIVE 1

#define OCB_READ0_LENGTH OCB_PUSH_PULL_LENGTH_MAX
#define OCB_READ1_LENGTH OCB_PUSH_PULL_LENGTH_MAX
#define OCB_READ2_LENGTH OCB_PUSH_PULL_LENGTH_MAX

#define OCB_WRITE0_LENGTH OCB_PUSH_PULL_LENGTH_MAX
#define OCB_WRITE1_LENGTH OCB_PUSH_PULL_LENGTH_MAX
#define OCB_WRITE2_LENGTH OCB_PUSH_PULL_LENGTH_MAX

#define OCB_READ0_PROTOCOL OCB_INTERRUPT_PROTOCOL_AGGRESSIVE
#define OCB_READ1_PROTOCOL OCB_INTERRUPT_PROTOCOL_AGGRESSIVE
#define OCB_READ2_PROTOCOL OCB_INTERRUPT_PROTOCOL_AGGRESSIVE

#define OCB_WRITE0_PROTOCOL OCB_INTERRUPT_PROTOCOL_LAZY
#define OCB_WRITE1_PROTOCOL OCB_INTERRUPT_PROTOCOL_LAZY
#define OCB_WRITE2_PROTOCOL OCB_INTERRUPT_PROTOCOL_LAZY

extern OcbUnitFfdc G_ocb_ffdc;

extern OcbQueue G_ocb_read_queue[];
extern OcbQueue G_ocb_write_queue[];

extern uint64_t G_ocb_read0_buffer[];
extern uint64_t G_ocb_read1_buffer[];
extern uint64_t G_ocb_read2_buffer[];

extern uint64_t G_ocb_write0_buffer[];
extern uint64_t G_ocb_write1_buffer[];
extern uint64_t G_ocb_write2_buffer[];


// PBA Queues

/// Block Copy Upload (OCI -> PowerBus) Engine job queue;
extern BceQueue G_pba_bcue_queue;

/// Block Copy Download Engine (PowerBus -> OCI) job queue;
extern BceQueue G_pba_bcde_queue;


// PBAX Queues

// PBAX circular queue naming is confusing, because "PUSH" is defined from the
// perspective of the PowerBus.  This driver uses the term "READ" and
// represents the hardware from the perspective of code running on OCC.

// PBAX read queue lengths are defined in terms of 8-byte doublewords, and
// must be in a small defined range.  Two meaningful interrupt protocols are
// possible for a queue: 'LAZY' allows read queues to fill before an interrupt
// is signalled.  'AGRESSIVE' signals an interrupt whenever there is data to
// be read.

// In a production system the lengths and protocols will likely vary based on
// the particular application and the characteristics of the communication
// partner. By default the queues take on their maximum lengths.  Read queues
// use the aggressive protocol which allows free-form communication. Different
// effective lengths and the interrupt protocols can be changed later if
// desired - assuming the queues are idle when the changes are made.

#define PBAX_PUSH_LENGTH_MIN 1
#define PBAX_PUSH_LENGTH_MAX 32

#define PBAX_INTERRUPT_PROTOCOL_LAZY       0
#define PBAX_INTERRUPT_PROTOCOL_AGGRESSIVE 1

#define PBAX_READ0_LENGTH PBAX_PUSH_LENGTH_MAX
#define PBAX_READ1_LENGTH PBAX_PUSH_LENGTH_MAX

#define PBAX_READ0_PROTOCOL PBAX_INTERRUPT_PROTOCOL_AGGRESSIVE
#define PBAX_READ1_PROTOCOL PBAX_INTERRUPT_PROTOCOL_AGGRESSIVE

/// Job queues for the PBAX circular buffers
extern PbaxQueue G_pbax_read_queue[];


// PBAX read buffers must be cache-line aligned since they are invalidated,
// and furthermore must be aligned to the next higher power-of-two of their
// lengths.  Some minor efficiencies could probably be obtained by a policy
// that buffers be held in non-cacheable storage (and subsequent modifications
// to the device driver code).
//
// Due to linker restrictions, only initialized data areas can be aligned.


#define _PBAX_ALIGN_(length)                    \
    (((length) <= 4) ? 32 :                     \
     (((length) <= 8) ? 64 :                    \
      (((length) <= 16) ? 128 : 256)))     

#define PBAX_CQ_READ_BUFFER(buffer, length) \
    uint64_t buffer[length] \
    __attribute__ ((aligned (_PBAX_ALIGN_(length)))) = {0}


extern uint64_t G_pbax_read0_buffer[];
extern uint64_t G_pbax_read1_buffer[];


// Initialization APIs

void
async_pore_initialize(PoreQueue *queue, int engine);

void
async_bce_initialize(BceQueue *queue, int engine, SsxIrqId irq);


void
async_ocb_initialize(OcbQueue *queue, int engine,
                     uint64_t *buffer, size_t length, int protocol);


void
async_pbax_initialize(PbaxQueue *queue, int engine, SsxIrqId irq,
                      uint64_t *buffer, size_t length, int protocol);


void
async_initialize();

#endif  /* __ASSEMBLER__ */

#endif  /* __PGP_ASYNC_H__ */
