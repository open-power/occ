/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ppc405/ppc405_init.c $                                */
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

/// \file ppc405_init.c
/// \brief PPC405 initialization routines
///
/// The entry points in this file are routines that are typically used during
/// initialization, and their code space could be deallocated and recovered if
/// no longer needed by the application after initialization.

#include "ssx.h"
#include "ssx_trace.h"

// Note that __ppc405_system_setup() is called from the SSX bootloader early
// in the initialization, at a point before the aplication has enabled
// critical or external interruts.

void
__ppc405_system_setup()
{
    SsxIrqId irq;

    // Initialize the interrupt vectors.

    for (irq = 0; irq < EXTERNAL_IRQS; irq++)
    {
        __ppc405_irq_handlers[irq].handler = __ppc405_default_irq_handler;
        __ppc405_irq_handlers[irq].arg = 0;
    }

    __ppc405_phantom_irq.handler = __ppc405_phantom_irq_handler;
    __ppc405_phantom_irq.arg = 0;

    // Initialize special interrupt handlers

    __ppc405_fit_routine = __ppc405_default_irq_handler;
    __ppc405_fit_arg = 0;

    __ppc405_watchdog_routine = __ppc405_default_irq_handler;
    __ppc405_watchdog_arg = 0;

    __ppc405_debug_routine = __ppc405_default_irq_handler;
    __ppc405_debug_arg = 0;

    // Enable the PIT interrupt, but not auto-reload mode. Clear the status
    // of all timers for good measure.

    andc_spr(SPRN_TCR, TCR_ARE);
    or_spr(SPRN_TCR, TCR_PIE);

    or_spr(SPRN_TSR, TSR_ENW | TSR_WIS | TSR_PIS | TSR_FIS);

#if SSX_TIMER_SUPPORT
#if SSX_TRACE_SUPPORT
    extern SsxTraceBuffer g_ssx_trace_buf;
    //set the instance id
    g_ssx_trace_buf.instance_id = OCCHW_INST_ID_PPC;
#endif  /* SSX_TRACE_SUPPORT */
#endif  /* SSX_TIMER_SUPPORT */

#ifdef HWMACRO_OCC
    //  Call system-specific setup
    void __occhw_setup();
    __occhw_setup();
#endif

}


// Set the timebase using the PowerPC protocol.

void
__ssx_timebase_set(SsxTimebase t)
{
    Uint64 tb;

    tb.value = t;

    mttbl(0);
    mttbu(tb.word[0]);
    mttbl(tb.word[1]);
}


