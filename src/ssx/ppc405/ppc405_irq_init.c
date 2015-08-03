/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ppc405/ppc405_irq_init.c $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2015                        */
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

/// \file ppc405_irq_init.c
/// \brief PPC405 IRQ initialization routines
///
/// The entry points in this file are routines that are typically used during
/// initialization, and their code space could be deallocated and recovered if
/// no longer needed by the application after initialization.

#include "ssx.h"

/// Set up a PPC405 Fixed Interval Timer (FIT) handler
///
/// See the SSX specification for full details on setting up a FIT handler.
///
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_ARGUMENT_PPC405_FIT The \a tcr_fp argument was 
/// invalid when called with a non-null (non-0) \a handler.

// Since the SSX_CRITICAL Watchdog interrupt is also controlled by the TCR, we
// need to enter an SSX_CRITICAL critical section to manipulate the TCR.

int
ppc405_fit_setup(int tcr_fp, SsxIrqHandler handler, void* arg)
{
    SsxMachineContext ctx;
    Ppc405TCR tcr;

    if (SSX_ERROR_CHECK_API && handler) {
        SSX_ERROR_IF((tcr_fp < 0) ||
                     (tcr_fp > 3),
                     SSX_INVALID_ARGUMENT_PPC405_FIT);
    }

    ssx_critical_section_enter(SSX_CRITICAL, &ctx);

    tcr.value = mfspr(SPRN_TCR);

    if (handler) {

        tcr.fields.fp  = tcr_fp;
        tcr.fields.fie = 1;

        __ppc405_fit_routine = handler;
        __ppc405_fit_arg = arg;

    } else {

        tcr.fields.fie = 0;
    }

    mtspr(SPRN_TCR, tcr.value);
    isync();

    ssx_critical_section_exit(&ctx);

    return SSX_OK;
}
    

/// Set up a PPC405 Watchdog interrupt handler
///
/// See the SSX specification for full details on setting up a watchdog
/// handler.
///
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_ARGUMENT_PPC405_WATCHDOG One or more of the \a tcr_wp 
/// or \a tcr_wrc arguments were invalid.

int
ppc405_watchdog_setup(int tcr_wp, int tcr_wrc, 
                      SsxIrqHandler handler, void* arg)
{
    SsxMachineContext ctx;
    Ppc405TCR tcr;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((tcr_wp < 0)  ||
                     (tcr_wp > 3)  ||
                     (tcr_wrc < 0) ||
                     (tcr_wrc > 3),
                     SSX_INVALID_ARGUMENT_PPC405_WATCHDOG);
    }

    ssx_critical_section_enter(SSX_CRITICAL, &ctx);

    mtspr(SPRN_TSR, TSR_ENW | TSR_WIS);

    tcr.value = mfspr(SPRN_TCR);

    tcr.fields.wp  = tcr_wp;
    tcr.fields.wrc = tcr_wrc;

    if (handler == 0) {

        // Reinstall the default handler and clear the interrupt enable.  Then
        // clear any pending interrupt status.
        // WIS. 

        __ppc405_watchdog_routine = __ppc405_default_irq_handler;
        __ppc405_watchdog_arg = 0;

        tcr.fields.wie = 0;
        mtspr(SPRN_TCR, tcr.value);
        isync();

        mtspr(SPRN_TSR, TSR_WIS);
        isync();

    } else {

        // Install the new handler and enable the watchdog interrup.

        __ppc405_watchdog_routine = handler;
        __ppc405_watchdog_arg = arg;

        tcr.fields.wie = 1;
        mtspr(SPRN_TCR, tcr.value);
        isync();
    }

    ssx_critical_section_exit(&ctx);

    return SSX_OK;
}
    

/// Set up a PPC405 Debug interrupt handler
///
/// See the SSX specification for full details on setting up a debug handler.
///
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_ARGUMENT_PPC405_DEBUG The \a handler argument 
/// is null (0).

// The debug handler is installed in an SSX_CRITICAL critical section with all
// debug interrupts disabled as well.

int
ppc405_debug_setup(SsxIrqHandler handler, void* arg)
{
    SsxMachineContext ctx;

    ssx_critical_section_enter(SSX_CRITICAL, &ctx);
    andc_msr(MSR_DE | MSR_DWE);

    __ppc405_debug_routine = handler;
    __ppc405_debug_arg = arg;
    isync();

    ssx_critical_section_exit(&ctx);

    return SSX_OK;
}
