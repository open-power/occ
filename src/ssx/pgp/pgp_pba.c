// $Id: pgp_pba.c,v 1.2 2014/02/03 01:30:35 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/pgp_pba.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pgp_pba.c
/// \brief procedures for pba setup and operation.

#include "ssx.h"
#include "pgp_pba.h"
#include "pgp_pmc.h"
#include "pgp_common.h"
#include "polling.h"


// Internal API to set up a PBA BAR

static int 
pba_bar_set(int idx, uint64_t pb_base, uint8_t pb_scope)
{
    int rc ;
    pba_barn_t bar ;

    bar.fields.cmd_scope = pb_scope ;
    bar.fields.addr = pb_base >> PBA_LOG_SIZE_MIN ; 

    rc = putscom(PBA_BARN(idx), bar.value);

    return rc ;
}


// Internal API to set up a PBA BAR Mask.  The mask covers bits 23:43, address
// bits 44:63 are always allowed.

static int
pba_barmask_set(int idx, uint64_t mask)
{
    int rc ;
    pba_barmskn_t barmask ;

    barmask.value = mask & 0x000001FFFFF00000ull;

    rc = putscom(PBA_BARMSKN(idx), barmask.value);

    return rc ;
}


/// Procedure to allocate a PowerBus address block using the bridge.
///
/// \param idx The BAR set to use, in the range 0..3.
///
/// \param base The 50-bit PowerBus base address where the block starts.  This
/// address must be aligned to the \a log_size.
///
/// \param log_size The base 2 logarithm of the block size, in bytes. The
/// minimum size is 1MB (2**20), the maximum size is 2TB (2**41)
///
/// This is a validation/test procedure only, since setting up the PBA BARs is
/// normally reserved to pHyp and FSP.  If the MMU is enabled and the PBA
/// mapping will be referenced by the 405, then an MMU mapping will also have
/// to be created (separately) for the block.
///
/// This procedure is not the complete setup for large memory areas.  Memory
/// areas that require the extended address will have to set that up
/// separately.
///
/// \retval 0 Success
///
/// \retval PBA_INVALID_ARGUMENT_BARSET One or more of the parameter 
/// restrictions were violated.
///
/// \retval PBA_SCOM_ERROR1 or PBA_SCOM_ERROR2 An attempt to write a PBA SCOM 
/// register to set up the BARs produced a non-zero return code.

int
pba_barset_initialize(int idx, uint64_t base, int log_size)
{
    uint64_t mask ;

    mask = (0x1ull << log_size) - 1;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((idx < 0) || 
                     (idx > 3) ||
                     (log_size < PBA_LOG_SIZE_MIN) || 
                     (log_size > PBA_LOG_SIZE_MAX) ||
                     ((base & mask) != 0),
                     PBA_INVALID_ARGUMENT_BARSET);
    }

    if (pba_bar_set(idx, base, PBA_POWERBUS_COMMAND_SCOPE_DEFAULT)) {
        SSX_ERROR(PBA_SCOM_ERROR1);
    }
    if (pba_barmask_set(idx, mask)) {
        SSX_ERROR(PBA_SCOM_ERROR2);
    }   

    return 0 ;
}


// polling() function for PBA Slave reset
//
// Slave reset for PBA is a complex issue, especially in cases where the
// entity requesting the reset may be executing from main memory, i.e.
// continuing to read to or write from the slave being reset.  To work around
// potential issues if the 405 is attempting to reset its own slave, the code
// that polls for reset is PowerBus cache-line aligned, and we re-hit the
// reset button each time we get an unsuccessful poll for the reset being
// done.  This should guarantee that the slave will go to reset status as soon
// as any PowerBus blockages (if any) clear and the master stops either
// reading or writing the slave port. For details see HW228485.
int
_pba_slave_reset_poll(void* arg, int* done) ALIGNED_ATTRIBUTE(128);

int
_pba_slave_reset_poll(void* arg, int* done)
{
    int id;
    pba_slvrst_t psr;

    id = (int)arg;

    psr.value = 0;
    psr.fields.set = PBA_SLVRST_SET(id);
    out64(PBA_SLVRST, psr.value);

    psr.value = in64(PBA_SLVRST);
    *done = !(psr.fields.in_prog & PBA_SLVRST_IN_PROG(id));

    return 0;
}


/// Reset a PBA slave with explicit timeout.  
///
/// \param id A PBA slave id in the range 0..3
///
/// \param timeout A value of SsxInterval type.  The special value
/// SSX_WAIT_FOREVER indicates no timeout.
///
/// \param sleep A value of SsxInterval type. Callers using the explicit
/// timeout form can request that the thread sleeps between polls; See
/// documentation for the polling() API.
///
/// This form of pba_slave_reset() gives the caller control over timeouts,
/// sleeping and error handling.
///
/// \retval 0 Success
///
/// \retval -PBA_INVALID_ARGUMENT_RESET The slave \a id parameter 
/// is invalid.
///
/// \retval -PBA_SLVRST_TIMED_OUT1 The procedure timed out waiting for the PBA
/// to reset the slave.


int
_pba_slave_reset(int id, SsxInterval timeout, SsxInterval sleep)
{
    int rc, closureRc;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((id < 0) || (id >= PBA_SLAVES),
                     PBA_INVALID_ARGUMENT_RESET);
    }
    
    rc = polling(&closureRc, _pba_slave_reset_poll, (void*)id, timeout, sleep);
    if (rc == POLLING_TIMEOUT) {
        rc = -PBA_SLVRST_TIMED_OUT1;
    }

    return rc;
}


/// Reset a PBA slave with a default timeout
///
/// \param id A PBA slave id in the range 0..3
///
/// PBA slaves must be reset before being reprogrammed. Resetting a slave
/// also flushes any write buffers and invalidates any read buffers associated
/// with the slave.  This procedure is for initialization/bringup/test only;
/// it has a non-deterministic run time due to the PowerBus so should not be
/// run as part of a hard real-time loop.
///
/// \retval 0 Success
///
/// \retval -SSX_INVALID_ARGUMENT_PBA_RESET The slave \a id parameter is 
/// invalid.
///
/// \retval -PBA_SLVRST_TIMED_OUT2 The procedure timed out waiting for the PBA
/// to reset the slave.

int
pba_slave_reset(int id)
{
    int rc;

    rc = _pba_slave_reset(id, PBA_SLAVE_RESET_TIMEOUT, 0);
    if (rc) {
        SSX_ERROR(PBA_SLVRST_TIMED_OUT2);
    }

    return rc;
}


/// Configure the PBAX mechanism
///
/// \param master If non-0, then this OCC will assume the role of the PBAX
/// master processor in the power domain.  To avoid PBAX livelock there can
/// only be a single master in any power domain, and only the master is
/// allowed to issue PBAX broadcast transactions.
///
/// \param node The PBAX Node Id of this OCC
///
/// \param chip The PBAX Chip Id of this OCC
///
/// \param group_mask A bit mask indicating the broadcast group(s) (power
/// domain(s)) that this OCC belongs to.
///
/// This API sets up certain fields of the PBA_XCFG register according to the
/// parameters. Other fields in the register controlling hardware timeouts and
/// performance monitoring are unchanged.
///
/// \retval 0 Success
///
/// \retval -PBAX_INVALID_ARGUMENT_CONFIG One of the arguments is 
/// not valid for some reason. 

int
pbax_configure(int master, int node, int chip, int group_mask)
{
    pba_xcfg_t pxc;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((node < 0) ||
                     (node >= PBAX_NODES) ||
                     (chip < 0) ||
                     (chip >= PBAX_CHIPS) ||
                     (group_mask < 0) ||
                     (group_mask > PBAX_GROUP_MASK_MAX),
                     PBAX_INVALID_ARGUMENT_CONFIG);
    }
    pxc.value = in64(PBA_XCFG);
    pxc.fields.reservation_en = (master != 0);
    pxc.fields.rcv_nodeid = node;
    pxc.fields.rcv_chipid = chip;
    pxc.fields.rcv_brdcst_group = group_mask;
    out64(PBA_XCFG, pxc.value);
    return 0;
}


/// Create a PBAX abstract target
///
/// \param target An uninitialized or otherwise idle PbaxTarget object
///
/// \param type The transmission type, either PBAX_UNICAST or PBAX_BROADCAST
///
/// \param scope The PowerBus scope, either PBAX_GROUP or PBAX_SYSTEM
///
/// \param queue The receive queue index on the target, either 0 or 1
///
/// \param node The PBAX Node Id of the target
///
/// \param chip_or_group Either the PBAX Chip Id of the target (for unicast),
/// or the PBAX Group Id of the target (for broadcast)
///
/// Create an abstraction of a communication target for PBAX send operations,
/// for use with the _pbax_send() and _pbax_send() APIs.  This API has no
/// knowledge of how the PBAX is configured, and therefore accepts all node,
/// chip and broadcast group ids as valid.  However this API does assume that
/// all broadcast transactions are "real-time" and require a reservation.
///
/// \retval 0 Success
///
/// \retval -PBAX_INVALID_OBJECT The \a target parameter is NULL (0) or
/// otherwise invalid. 
///
/// \retval -PBAX_INVALID_ARGUMENT_TARGET One or more of the arguments 
/// is invalid.

int
pbax_target_create(PbaxTarget* target,
                   int type, int scope, int queue, 
                   int node, int chip_or_group)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(target == 0, PBAX_INVALID_OBJECT);
        SSX_ERROR_IF(((type != PBAX_UNICAST) && (type != PBAX_BROADCAST)) ||
                     ((scope != PBAX_GROUP) && (scope != PBAX_SYSTEM)) ||
                     ((queue < 0) || (queue >= PBAX_QUEUES)),
                     PBAX_INVALID_ARGUMENT_TARGET);
    }

    target->target.value = 0;
    target->target.fields.snd_scope = scope;
    target->target.fields.snd_qid = queue;
    target->target.fields.snd_type = type;
    target->target.fields.snd_reservation = (type == PBAX_BROADCAST);
    target->target.fields.snd_nodeid = node;
    target->target.fields.snd_chipid = chip_or_group;

    return 0;
}



/// Use PBAX to send 64 bits to a target with a caller-specified timeout
///
/// \param target An abstract PBAX target object
///
/// \param data The data to send
///
/// \param timeout The caller's timeout represented as an
/// SsxInterval. Use SSX_WAIT_FOREVER to indicate the caller is willing to
/// wait forever.  A \a timeout of 0 indicates that the caller is not
/// willing to wait at all, and the call will either succeed immediately or
/// immediately return -PBAX_TIMEOUT.
///
/// The PBAX mechanism has a single outgoing channel that must be shared by
/// all processes that need to send messages using PBAX.  Since messages sent
/// over PBAX may require an unknown amount of time to complete (due to
/// PowerBus traffic and blockages), this driver implements an agressive
/// policy that allows a sender to initiate a send transaction without waiting
/// for the final completion status. However this opens a window where a
/// subsequent writer may find the PBAX send mechanism already in use.  Here
/// the new sender is obligated to busy-wait until the send mechanism is free,
/// as the PBAX send does not include an interrupt notification.
///
/// This form of the PBAX send operation accepts an explicit \a timeout value
/// as previously described.  The timeout represents the amount of time that
/// the caller is willing to wait for a busy PBAX send mechanism to become
/// free. The PBAX hardware also includes mechanisms to time out the hardware
/// and modeably interrupt OCC in the event of lack of progress.
///
/// <em> This API does not operate in a critical section or enforce any
/// synchronization protocol. Synchronization and software timeout management
/// in the case of multiple senders is the responsibility of the
/// application. </em>
///
/// Unlike most other driver APIs this API can not be configured to "panic",
/// but instead always terminates with a 0 return code or an error status.
///
/// \retval 0 Success.  Success only means that a transaction was sucessfully
/// initiated on an idle PBAX send machine.
///
/// \retval -PBAX_TIMEOUT The caller-specified timeout expired before the PBAX
/// send machine became free, but the PBAX send machine does not show error
/// status. 
///
/// \retval -PBAX_SEND_ERROR The PBAXSNDSTAT.snd_error bit is asserted.  It
/// is expected that this error will cause the PBA error interrupt to fire -
/// FFDC is collected by the interrupt handler.

int
_pbax_send(PbaxTarget* target, uint64_t data, SsxInterval timeout)
{
    pba_xsndstat_t pss;
    SsxTimebase start;
    int rc, timed_out;

    // The PBAX is always polled at least twice to guarantee that we always
    // poll once after a timeout - unless the caller explicitly requested a 0
    // timeout. 
    
    start = 0;     
    timed_out = 0; 
    do {
        pss.words.high_order = in32(PBA_XSNDSTAT);
        if (pss.fields.snd_error) {
            rc = -PBAX_SEND_ERROR;
            break;
        }
        if (!pss.fields.snd_in_progress) {
            rc = 0;
            break;
        }
        if (start == 0) {
            start = ssx_timebase_get();
        }
        if ((timeout == 0) || timed_out) {
            rc = -PBAX_SEND_TIMEOUT;
            break;
        }
        timed_out = 
            ((timeout != SSX_WAIT_FOREVER) &&
             ((ssx_timebase_get() - start) > timeout));
    } while (1);

    //  Configure the send engine and initiate the write, which is kicked off
    //  by writing the high-order word of the send data register.

    if (!rc) {
        out32(PBA_XSNDTX, target->target.words.high_order);
        out32(PBA_XSNDDAT + 4, data & 0xffffffff);
        out32(PBA_XSNDDAT, data >> 32);
    }

    return rc;
}


/// Use PBAX to send 64 bits to a target with a default timeout
///
/// \param target An abstract PBAX target object
///
/// \param data The data to send
///
/// This form of the PBAX send operation uses a default timeout, and may be
/// configured to panic in the event of timeouts or send errors.  See
/// _pbax_send() for the specification of the underlying API.
///
/// <em> This API does not operate in a critical section or enforce any
/// synchronization protocol. Synchronization and software timeout management
/// in the case of multiple senders is the responsibility of the
/// application. </em>
///
/// \retval 0 Success.  Success only means that a transaction was sucessfully
/// initiated on an idle PBAX send machine.
///
/// \retval -PBAX_TIMEOUT The caller-specified timeout expired before the PBAX
/// send machine became free.
///
/// \retval -PBAX_SEND_ERROR The PBAXSNDSTAT.snd_error bit is asserted.

int
pbax_send(PbaxTarget* target, uint64_t data)
{
    int rc;

    rc = _pbax_send(target, data, PBAX_SEND_DEFAULT_TIMEOUT);
    if (rc) {
        if (rc == -PBAX_SEND_TIMEOUT) {
            SSX_ERROR(PBAX_SEND_TIMEOUT);
        } else {
            SSX_ERROR(PBAX_SEND_ERROR);
        }
    }
    return rc;
}


    
    
    
                     






