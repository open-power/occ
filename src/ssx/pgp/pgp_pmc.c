// $Id: pgp_pmc.c,v 1.2 2014/02/03 01:30:35 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/pgp_pmc.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pgp_pmc.c
/// \brief PgP procedures and support for PMC operations
///
/// <b> SCOM Operations </b>
///
/// The PMC provides an indirect bridge from the OCI to the PIB/PCB.  OCC
/// firmware therefore has the ability to do immediate putscom()/getscom()
/// operations in addition to the capabilities provided by the PORE-GPE
/// engines. In PgP, SCOM latency from OCC is expected to be in the range of
/// 150 - 1000 ns.  The maximum latency of a PIB operation has a hard upper
/// bound derived from the hardware implementation.  The putscom()/getscom()
/// drivers here take advantage of this upper bound and implement tight
/// timeouts, enforced by polling the timebase while waiting for the SCOM
/// operations to complete.
///
/// The latencies are small enough and so well understood that the
/// getscom()/putscom() procedures operate in SSX_CRITICAL critical
/// sections. There should be no problem for thread-based procedures to be
/// written using getscom()/putscom() directly - in fact for short procedures
/// it may be less overhead to use getscom()/putscom() than queuing a PORE-GPE
/// program.  All mainline procedures used by hard real-time code should
/// remain as PORE-GPE programs however.
///
/// SCOM operations return non-zero error codes that may or may not indicate
/// an actual error, depending on which SCOM is being accessed.  This error
/// code (or 0 for success) is returned as the value of getscom()/putscom().
/// The error severity increases with the severity of the error:
/// \code
///
/// #define PCB_ERROR_NONE              0
/// #define PCB_ERROR_RESOURCE_OCCUPIED 1
/// #define PCB_ERROR_CHIPLET_OFFLINE   2
/// #define PCB_ERROR_PARTIAL_GOOD      3
/// #define PCB_ERROR_ADDRESS_ERROR     4
/// #define PCB_ERROR_CLOCK_ERROR       5
/// #define PCB_ERROR_PACKET_ERROR      6
/// #define PCB_ERROR_TIMEOUT           7
/// \endcode
///
/// The default configuration variable SCOM_ERROR_LIMIT defines the maximum
/// error code that will be returned - error codes above the limit (plus hard
/// timeouts and other protocol errors) cause an immediate kernel panic. In
/// the event of a non-0 error code, getscom() always sets the returned data
/// to 0.
///
/// In addition to getscom()/putscom() that implement the above defined error
/// protocols, the raw APIs _getscom()/_putscom() are also available and
/// allow the application full control over timeouts and error handling on a
/// SCOM-by-SCOM basis.
///
/// \bug Modify getscom/putscom to return the SSX error codes rather than
/// 1-7. 
///
/// \bug Implement and use a generic poll_with_timeout(f, arg, t)

#include "ssx.h"
#include "pgp_pmc.h"


////////////////////////////////////////////////////////////////////////////
// SCOM
////////////////////////////////////////////////////////////////////////////

// Common SCOM polling loop with software timeout.  The PMC is always polled
// at least twice to guarantee that we always poll once after a timeout.

static int 
poll_scom(SsxInterval timeout, pmc_o2p_ctrl_status_reg_t *cs)
{
    SsxTimebase start;
    int timed_out;

    start = ssx_timebase_get();
    timed_out = 0;
    do {
        cs->value = in32(PMC_O2P_CTRL_STATUS_REG);
        if (!(cs->fields.o2p_ongoing)) {
            break;
        }
        if (timed_out) {
            return -SCOM_TIMEOUT_ERROR;
        }
        timed_out = 
            ((timeout != SSX_WAIT_FOREVER) &&
             ((ssx_timebase_get() - start) > timeout));
    } while (1);

    return 0;
}


/// A raw getscom() through the PMC OCI/PIB bridge
///
/// \param address A standard 32-bit SCOM address, including multicast
/// addresses.
///
/// \param data Points to a container for the returned data.
///
/// \param timeout The software timeout as an SSX interval (timebase ticks),
/// or the special value SSX_WAIT_FOREVER to indicate no software timeout.
///
/// This routine executes in an SSX_CRITICAL critical section.
///
/// Unlike most other APIs, this API returns both positive and negative error
/// codes, as well as the 0 code for success. In the event of PCB errors, the
/// returned \a data is obtained from the PMC O2P data registers.  In the
/// event of non-PCB errors, the caller \a data is not modified.
///
/// If the transaction experiences a software timeout (controlled by the \a
/// timeout parameter) or a protocol error, the PMC PIB master will be left in
/// a state in which it is illegal to perform further SCOM access through the
/// PMC until the ongoing transaction is finished.
///
/// \retval 0 Success
///
///\ retval 1-7 A PCB error code.  See \c pcb_common.h
///
/// \retval -SCOM_TIMEOUT_ERROR The software timeout specified by the \a
/// timeout parameter expired before the transaction completed.
///
/// retval -SCOM_PROTOCOL_ERROR_GETSCOM_BUSY The PMC SCOM engine was busy when 
/// the call was made. 

int
_getscom(uint32_t address, uint64_t *data, SsxInterval timeout)
{
    pmc_o2p_addr_reg_t addr;
    pmc_o2p_ctrl_status_reg_t cs;
    SsxMachineContext ctx;
    Uint64 data64;
    int rc;

    ssx_critical_section_enter(SSX_CRITICAL, &ctx);

    // Check for a transaction already ongoing

    cs.value = in32(PMC_O2P_CTRL_STATUS_REG);
    if (cs.fields.o2p_ongoing) {
        ssx_critical_section_exit(&ctx);
        return -SCOM_PROTOCOL_ERROR_GETSCOM_BUSY;
    }

    // Start the read.  The 'read' bit is forced into the address.  Writing
    // the PMC_O2P_ADDR_REG starts the read.

    addr.value = address;
    addr.fields.o2p_read_not_write = 1;
    out32(PMC_O2P_ADDR_REG, addr.value);

    // Polling and return.

    rc = poll_scom(timeout, &cs);

    data64.word[0] = in32(PMC_O2P_RECV_DATA_HI_REG);
    data64.word[1] = in32(PMC_O2P_RECV_DATA_LO_REG);
    *data = data64.value;

    ssx_critical_section_exit(&ctx);

    if (rc) {
        return rc;
    } else {
        return cs.fields.o2p_scresp;
    }
}


/// getscom() through the PMC OCI/PIB bridge
///
/// \param address A standard 32-bit SCOM address, including multicast
/// addresses.
///
/// \param data Points to a container for the returned data.
///
/// This routine executes in an SSX_CRITICAL critical section.
///
/// Unlike most other APIs, this API returns positive error
/// codes, as well as the 0 code for success. In the event of PCB errors, the
/// returned \a data is set to 0.
///
/// If the transaction experiences a software timeout (controlled by the \a
/// timeout parameter), a protocol error, or a PCB error greater than the
/// configuration constant SCOM_ERROR_LIMIT this routine causes a kernel
/// panic. This may leave the PMC PIB master in a state in which it is illegal
/// to perform further SCOM access through the PMC (until the ongoing
/// transaction is finished.)
///
/// \retval 0 Success
///
///\ retval 1-7 A PCB error code.  See \c pcb_common.h

int
getscom(uint32_t address, uint64_t *data)
{
    int rc;

    rc = _getscom(address, data, SCOM_TIMEOUT);
    if (rc == 0) {
        return 0;
    }

    if ((rc > 0) && (rc <= SCOM_ERROR_LIMIT)) {
        *data = 0;
    } else {

        printk("getscom(0x%08x, %p) : Failed with error %d\n",
               address, data, rc);

        if (rc > 0) {
            switch (rc) {
            case 1: SSX_PANIC(SCOM_PCB_ERROR_1_GETSCOM); break;
            case 2: SSX_PANIC(SCOM_PCB_ERROR_2_GETSCOM); break;
            case 3: SSX_PANIC(SCOM_PCB_ERROR_3_GETSCOM); break;
            case 4: SSX_PANIC(SCOM_PCB_ERROR_4_GETSCOM); break;
            case 5: SSX_PANIC(SCOM_PCB_ERROR_5_GETSCOM); break;
            case 6: SSX_PANIC(SCOM_PCB_ERROR_6_GETSCOM); break;
            default: SSX_PANIC(SCOM_PCB_ERROR_7_GETSCOM); break;
            } 
        } else if (rc == -SCOM_TIMEOUT_ERROR) {
            SSX_PANIC(SCOM_TIMEOUT_ERROR_GETSCOM);
        } else {
            SSX_PANIC(SCOM_PROTOCOL_ERROR_GETSCOM);
        }
    }

    return rc;
}


/// A raw putscom() through the PMC OCI/PIB bridge
///
/// \param address A standard 32-bit SCOM address, including multicast
/// addresses.
///
/// \param data The SCOM write data
///
/// \param timeout The software timeout as an SSX interval (timebase ticks),
/// or the special value SSX_WAIT_FOREVER to indicate no timeout.
///
/// This routine executes in an SSX_CRITICAL critical section.
///
/// Unlike most other APIs, this API returns both positive and negative error
/// codes, as well as the 0 code for success. 
///
/// If the transaction experiences a software timeout (controlled by the \a
/// timeout parameter) or a protocol error, the PMC PIB master will be left in
/// a state in which it is illegal to perform further SCOM access through the
/// PMC until the ongoing transaction is finished.
///
/// \retval 0 Success
///
/// \retval 1-7 A PCB error code.  See \c pcb_common.h
///
/// \retval -SCOM_TIMEOUT The software timeout specified by the \a timeout
/// parameter expired before the transaction completed.
///
/// \retval -SCOM_PROTOCOL_ERROR_PUTSCOM_BUSY The PMC SCOM engine was busy when 
/// the call was made. 

int
_putscom(uint32_t address, uint64_t data, SsxInterval timeout)
{
    pmc_o2p_addr_reg_t addr;
    pmc_o2p_ctrl_status_reg_t cs;
    SsxMachineContext ctx;
    Uint64 data64;
    int rc;

    ssx_critical_section_enter(SSX_CRITICAL, &ctx);

    // Check for a transaction already ongoing

    cs.value = in32(PMC_O2P_CTRL_STATUS_REG);
    if (cs.fields.o2p_ongoing) {
        ssx_critical_section_exit(&ctx);
        return -SCOM_PROTOCOL_ERROR_PUTSCOM_BUSY;
    }

    // Start the write.  The 'write' bit is cleared in the address. Here the
    // PIB write starts when the PMC_O2P_SEND_DATA_LO_REG is written.

    addr.value = address;
    addr.fields.o2p_read_not_write = 0;
    out32(PMC_O2P_ADDR_REG, addr.value);

    data64.value = data;
    out32(PMC_O2P_SEND_DATA_HI_REG, data64.word[0]);
    out32(PMC_O2P_SEND_DATA_LO_REG, data64.word[1]);
    
    // Poll and return.

    rc = poll_scom(timeout, &cs);

    ssx_critical_section_exit(&ctx);

    if (rc) {
        return rc;
    } else {
        return cs.fields.o2p_scresp;
    }
}


/// putscom() through the PMC OCI/PIB bridge
///
/// \param address A standard 32-bit SCOM address, including multicast
/// addresses.
///
/// \param data The SCOM write data.
///
/// This routine executes in an SSX_CRITICAL critical section.
///
/// Unlike most other APIs, this API returns positive error
/// codes, as well as the 0 code for success. 
///
/// If the transaction experiences a software timeout (controlled by the \a
/// timeout parameter), a protocol error, or a PCB error greater than the
/// configuration constant SCOM_ERROR_LIMIT this routine causes a kernel
/// panic. This may leave the PMC PIB master in a state in which it is illegal
/// to perform further SCOM access through the PMC (until the ongoing
/// transaction is finished.)
///
/// \retval 0 Success
///
/// \retval 1-7 A PCB error code.  See \c pcb_common.h

int
putscom(uint32_t address, uint64_t data)
{
    int rc;

    rc = _putscom(address, data, SCOM_TIMEOUT);

    if ((rc == 0) || ((rc > 0) && (rc <= SCOM_ERROR_LIMIT))) {
        return rc;
    }

    printk("putscom(0x%08x, 0x%016llx) : Failed with error %d\n",
           address, data, rc);

    if (rc > 0) {
        switch (rc) {
        case 1: SSX_PANIC(SCOM_PCB_ERROR_1_PUTSCOM); break;
        case 2: SSX_PANIC(SCOM_PCB_ERROR_2_PUTSCOM); break;
        case 3: SSX_PANIC(SCOM_PCB_ERROR_3_PUTSCOM); break;
        case 4: SSX_PANIC(SCOM_PCB_ERROR_4_PUTSCOM); break;
        case 5: SSX_PANIC(SCOM_PCB_ERROR_5_PUTSCOM); break;
        case 6: SSX_PANIC(SCOM_PCB_ERROR_6_PUTSCOM); break;
        default: SSX_PANIC(SCOM_PCB_ERROR_7_PUTSCOM); break;
        }
    } else if (rc == -SCOM_TIMEOUT_ERROR) {
        SSX_PANIC(SCOM_TIMEOUT_ERROR_PUTSCOM);
    } else {
        SSX_PANIC(SCOM_PROTOCOL_ERROR_PUTSCOM);
    }

    return rc;
}
