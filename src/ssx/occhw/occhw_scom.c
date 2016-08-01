/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/occhw/occhw_scom.c $                                  */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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

/// \file occhw_scom.c
/// \brief procedures and support for scom operations
///
/// <b> SCOM Operations </b>
///
/// The maximum latency of a PIB operation has a hard upper
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
#include "occhw_scom.h"
#include "occhw_shared_data.h"

////////////////////////////////////////////////////////////////////////////
// SCOM
////////////////////////////////////////////////////////////////////////////

// Common SCOM polling loop with software timeout.  The PMC is always polled
// at least twice to guarantee that we always poll once after a timeout.

static int
poll_scom(SsxInterval timeout)
{
    SsxTimebase start;
    int timed_out;
    int rc;

    start = ssx_timebase_get();
    timed_out = 0;

    do
    {
        rc = ssx_irq_status_get(OCCHW_IRQ_IPI_SCOM);

        if (!rc)
        {
            break;
        }

        if (timed_out)
        {
            rc = -SCOM_TIMEOUT_ERROR;
            break;
        }

        timed_out =
            ((timeout != SSX_WAIT_FOREVER) &&
             ((ssx_timebase_get() - start) > timeout));
    }
    while (1);

    return rc;
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
_getscom(uint32_t address, uint64_t* data, SsxInterval timeout)
{
    SsxMachineContext   ctx;
    int                 rc;
    occhw_scom_cmd_t*    scom_cmd = &OSD_PTR->scom_cmd;
    occhw_scom_status_t scom_status;

    do
    {
        if(address & OCCHW_SCOM_READ_MASK)
        {
            rc = -SCOM_INVALID_ADDRESS;
            break;
        }

        ssx_critical_section_enter(SSX_CRITICAL, &ctx);

        // Check for a transaction already ongoing
        rc = ssx_irq_status_get(OCCHW_IRQ_IPI_SCOM);

        if (rc)
        {
            ssx_critical_section_exit(&ctx);
            rc = -SCOM_PROTOCOL_ERROR_GETSCOM_BUSY;
            break;
        }

        // Setup the write.  The 'read' bit is set in the address.
        scom_cmd->scom_status.status32 = OCCHW_SCOM_PENDING;
        scom_cmd->scom_addr = address | OCCHW_SCOM_READ_MASK;

        // Notify the GPE (by raising an interrupt) that a request is pending
        ssx_irq_status_set(OCCHW_IRQ_IPI_SCOM, 1);

        // Poll until completed or timed out
        rc = poll_scom(timeout);

        // Extract the data and status out of the scom command block
        *data = scom_cmd->scom_data;
        scom_status.status32 = scom_cmd->scom_status.status32;

        ssx_critical_section_exit(&ctx);

        if(!rc)
        {
            //check that the GPE updated the scom status.  Normally,
            //the gpe won't clear the interrupt until it has updated
            //the status field.  The exception is if the GPE gets
            //reset.
            if(scom_status.status32 == OCCHW_SCOM_PENDING)
            {
                rc = -SCOM_PROTOCOL_ERROR_GETSCOM_RST;
            }
            else
            {
                //The SIBRC field of the MSR is where we get the status for
                //the last scom operation.
                rc = scom_status.sibrc;
            }
        }

    }
    while(0);

    return rc;
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
getscom(uint32_t address, uint64_t* data)
{
    int rc;

    rc = _getscom(address, data, SCOM_TIMEOUT);

    if (rc == 0)
    {
        return 0;
    }

    if ((rc > 0) && (rc <= SCOM_ERROR_LIMIT))
    {
        *data = 0;
    }
    else
    {

        //printk("getscom(0x%08x, %p) : Failed with error %d\n",
        //       address, data, rc);

        if (rc > 0)
        {
            switch (rc)
            {
                case 1:
                    SSX_PANIC(SCOM_PCB_ERROR_1_GETSCOM);
                    break;

                case 2:
                    SSX_PANIC(SCOM_PCB_ERROR_2_GETSCOM);
                    break;

                case 3:
                    SSX_PANIC(SCOM_PCB_ERROR_3_GETSCOM);
                    break;

                case 4:
                    SSX_PANIC(SCOM_PCB_ERROR_4_GETSCOM);
                    break;

                case 5:
                    SSX_PANIC(SCOM_PCB_ERROR_5_GETSCOM);
                    break;

                case 6:
                    SSX_PANIC(SCOM_PCB_ERROR_6_GETSCOM);
                    break;

                default:
                    SSX_PANIC(SCOM_PCB_ERROR_7_GETSCOM);
                    break;
            }
        }
        else if (rc == -SCOM_TIMEOUT_ERROR)
        {
            SSX_PANIC(SCOM_TIMEOUT_ERROR_GETSCOM);
        }
        else
        {
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
    SsxMachineContext   ctx;
    int                 rc;
    occhw_scom_cmd_t*    scom_cmd = &OSD_PTR->scom_cmd;
    occhw_scom_status_t scom_status;

    do
    {
        if(address & OCCHW_SCOM_READ_MASK)
        {
            rc = -SCOM_INVALID_ADDRESS;
            break;
        }

        ssx_critical_section_enter(SSX_CRITICAL, &ctx);

        // Check for a transaction already ongoing
        rc = ssx_irq_status_get(OCCHW_IRQ_IPI_SCOM);

        if (rc)
        {
            ssx_critical_section_exit(&ctx);
            rc = -SCOM_PROTOCOL_ERROR_PUTSCOM_BUSY;
            break;
        }

        // Setup the write.  The 'read' bit is cleared in the address.
        scom_cmd->scom_status.status32 = OCCHW_SCOM_PENDING;
        scom_cmd->scom_addr = address;
        scom_cmd->scom_data = data;

        // Notify the GPE (by raising an interrupt) that a request is pending
        ssx_irq_status_set(OCCHW_IRQ_IPI_SCOM, 1);

        // Poll until completed or timed out
        rc = poll_scom(timeout);
        scom_status.status32 = scom_cmd->scom_status.status32;

        ssx_critical_section_exit(&ctx);

        if(!rc)
        {
            //check that the GPE updated the scom status.  Normally,
            //the gpe won't clear the interrupt until it has updated
            //the status field.  The exception is if the GPE gets
            //reset.
            if(scom_status.status32 == OCCHW_SCOM_PENDING)
            {
                rc = -SCOM_PROTOCOL_ERROR_PUTSCOM_RST;
            }
            else
            {
                //The SIBRC field of the MSR is where we get the status for
                //the last scom operation.
                rc = scom_status.sibrc;
            }
        }
    }
    while(0);

    return rc;
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

    if ((rc == 0) || ((rc > 0) && (rc <= SCOM_ERROR_LIMIT)))
    {
        return rc;
    }

    //printk("putscom(0x%08x, 0x%016llx) : Failed with error %d\n",
    //       address, data, rc);

    if (rc > 0)
    {
        switch (rc)
        {
            case 1:
                SSX_PANIC(SCOM_PCB_ERROR_1_PUTSCOM);
                break;

            case 2:
                SSX_PANIC(SCOM_PCB_ERROR_2_PUTSCOM);
                break;

            case 3:
                SSX_PANIC(SCOM_PCB_ERROR_3_PUTSCOM);
                break;

            case 4:
                SSX_PANIC(SCOM_PCB_ERROR_4_PUTSCOM);
                break;

            case 5:
                SSX_PANIC(SCOM_PCB_ERROR_5_PUTSCOM);
                break;

            case 6:
                SSX_PANIC(SCOM_PCB_ERROR_6_PUTSCOM);
                break;

            default:
                SSX_PANIC(SCOM_PCB_ERROR_7_PUTSCOM);
                break;
        }
    }
    else if (rc == -SCOM_TIMEOUT_ERROR)
    {
        SSX_PANIC(SCOM_TIMEOUT_ERROR_PUTSCOM);
    }
    else
    {
        SSX_PANIC(SCOM_PROTOCOL_ERROR_PUTSCOM);
    }

    return rc;
}
