/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/pss/dpss.c $                                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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

#include "ssx.h"

#include <dpss.h>
#include <trac.h>
#include <occ_common.h>
#include <comp_ids.h>
#include <occ_sys_config.h>
#include <trac_interface.h>
#include <occ_service_codes.h>
#include <pss_service_codes.h>
#include <state.h>
#include <amec_oversub.h>
#include <ppc405_irq.h>

// Macro creates a 'bridge' handler that converts the initial fast-mode to full-
// mode interrupt handler
SSX_IRQ_FAST2FULL(isr_dpss_oversubscription_handler, isr_dpss_oversubscription_handler_full);

// Increment a counter each time we see a phantom interrupt (used by health monitor thread)
uint32_t G_occ_phantom_critical_count = 0;
uint32_t G_occ_phantom_noncritical_count = 0;

// Function Specification
//
// Name:  isr_dpss_oversubscription_handler_full
//
// Description: From the flow diagram:
//              Will eventually be doing something to set the Pstates & Memory
//              Throttles when this interrupt occurs, but for now just put in this
//              comment and no code besides clearing the irq and tracing the fact that
//              we got an interrupt.
//
// End Function Specification
void isr_dpss_oversubscription_handler_full(void *private, SsxIrqId irq, int priority)
{
    SsxMachineContext ctx;

    // disable further interrupts at this level
    ssx_irq_disable(irq);

    // enter the protected context
    ssx_critical_section_enter( SSX_CRITICAL, &ctx );

    // clear this irq
    ssx_irq_status_clear(irq);

    // exit the protected context
    ssx_critical_section_exit( &ctx );

    // call oversub isr
    amec_oversub_isr();

    // re-enable interrupts at this level
    ssx_irq_enable(irq);

} // end isr_dpss_oversubscription_handler_full

// Function Specification
//
// Name:  occ_phantom_irq_handler
//
// Description:
// handler for an interrupt that fired and then went away before
// we could read what it was.
//
// End Function Specification
void occ_phantom_irq_handler(void* i_arg, SsxIrqId i_irq, int i_critical)
{
    if(i_critical == SSX_CRITICAL)
    {
        G_occ_phantom_critical_count++;
    }
    else
    {
        G_occ_phantom_noncritical_count++;
    }
}

// Function Specification
//
// Name:  dpss_oversubscription_irq_initialize
//
// Description:
// Installs the power oversubscription IRQ handler for the DPSS
//
// End Function Specification
errlHndl_t dpss_oversubscription_irq_initialize()
{
    int rc = 0;
    errlHndl_t l_err = NULL;

    // NOTE:  It is believed that the oversubscription interrupt bounces
    // on and off as power supplies are re-inserted.  If it happens quickly enough
    // it will clear the interrupt before the code has a chance to see the cause
    // of the interrupt.  The default phantom handler would then be invoked and
    // cause occ to panic.  Instead, we just increment a counter and log an info
    // error.
    __ppc405_phantom_irq.handler = occ_phantom_irq_handler;

    // Disable the IRQ while we work on it
    ssx_irq_disable(PGP_IRQ_EXTERNAL_TRAP);

    // Setup the IRQ
    rc = ssx_irq_setup(PGP_IRQ_EXTERNAL_TRAP,
                       SSX_IRQ_POLARITY_ACTIVE_LOW,
                       SSX_IRQ_TRIGGER_LEVEL_SENSITIVE);

    if( rc ) {
        TRAC_ERR("%s: Failed IRQ setup.", __FUNCTION__);

        /*@
         * @moduleid   PSS_MID_DPSS_OVS_IRQ_INIT
         * @reasonCode SSX_GENERIC_FAILURE
         * @severity   ERRL_SEV_PREDICTIVE
         * @userdata1  ssx_irq_setup return code
         * @userdata4  ERC_SSX_IRQ_SETUP_FAILURE
         * @devdesc    Firmware failure initializing DPSS IRQ
         */
        l_err = createErrl( PSS_MID_DPSS_OVS_IRQ_INIT, // i_modId
                            SSX_GENERIC_FAILURE,       // i_reasonCode
                            ERC_SSX_IRQ_SETUP_FAILURE,
                            ERRL_SEV_PREDICTIVE,
                            NULL,                      // tracDesc_t i_trace
                            0,                         // i_traceSz
                            rc,                        // i_userData1
                            0);                        // i_userData2
    }
    else {
        // Set the IRQ handler
        rc = ssx_irq_handler_set(PGP_IRQ_EXTERNAL_TRAP,
                                 isr_dpss_oversubscription_handler,
                                 NULL,
                                 SSX_NONCRITICAL);

        if( rc ) {
            TRAC_ERR("%s: Failed to set the IRQ handler.", __FUNCTION__);

            /*@
             * @moduleid   PSS_MID_DPSS_OVS_IRQ_INIT
             * @reasonCode SSX_GENERIC_FAILURE
             * @severity   ERRL_SEV_PREDICTIVE
             * @userdata1  ssx_irq_handler_set return code
             * @userdata4  ERC_SSX_IRQ_HANDLER_SET_FAILURE
             * @devdesc    Firmware failure setting up DPSS routine
             */
            l_err = createErrl( PSS_MID_DPSS_OVS_IRQ_INIT, // i_modId
                                SSX_GENERIC_FAILURE,       // i_reasonCode
                                ERC_SSX_IRQ_HANDLER_SET_FAILURE,
                                ERRL_SEV_PREDICTIVE,
                                NULL,                      // tracDesc_t i_trace
                                0,                         // i_traceSz
                                rc,                        // i_userData1
                                0);                        // i_userData2
        }
        else {
            // Enable the IRQ
            ssx_irq_status_clear(PGP_IRQ_EXTERNAL_TRAP);
            ssx_irq_enable(PGP_IRQ_EXTERNAL_TRAP);
        }
    }

    return l_err;
} // end dpss_oversubscription_irq_initialize

