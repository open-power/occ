/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/ppe42_init.c $                               */
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

/// \file ppe42_init.c
/// \brief PPE42 initialization routines
///
/// The entry points in this file are routines that are typically used during
/// initialization, and their code space could be deallocated and recovered if
/// no longer needed by the application after initialization.

#include "pk.h"
#include "pk_trace.h"

// Note that __ppe42_system_setup() is called from the PK bootloader early
// in the initialization, at a point before the aplication has enabled
// interrupts.

// This function is expected to be defined by the macro specific code (GPE, CME, SBE)
void __hwmacro_setup(void);


void
__ppe42_system_setup()
{
    //Only do this if the application hasn't provided a static table definition
#ifndef STATIC_IRQ_TABLE
    PkIrqId irq;

    // Initialize the interrupt vectors.
    for (irq = 0; irq < EXTERNAL_IRQS; irq++) {
        __ppe42_irq_handlers[irq].handler = __ppe42_default_irq_handler;      
    }

    //NOTE: EXTERNAL_IRQS is the phantom interrupt assigned irq
    __ppe42_irq_handlers[irq].handler = __ppe42_phantom_irq_handler;

    // Initialize special interrupt handlers

    __ppe42_fit_routine = __ppe42_default_irq_handler;
    __ppe42_fit_arg = 0;

    __ppe42_watchdog_routine = __ppe42_default_irq_handler;
    __ppe42_watchdog_arg = 0;

/*
    __ppe42_debug_routine = __ppe42_default_irq_handler;
    __ppe42_debug_arg = 0;
*/
#endif /*STATIC_IRQ_TABLE*/

    //Clear all status bits in the TSR
    mtspr(SPRN_TSR, TSR_ENW | TSR_WIS | TSR_DIS | TSR_FIS);

#ifdef APPCFG_USE_EXT_TIMEBASE
    //Enable the DEC interrupt and configure it to use the external dec_timer signal
    mtspr(SPRN_TCR, TCR_DIE | TCR_DS);
#else
    //Enable the DEC interrupt and configure it to use the internal clock signal
    mtspr(SPRN_TCR, TCR_DIE);
#endif /* APPCFG_USE_EXT_TIMEBASE */

#if PK_TIMER_SUPPORT
#if PK_TRACE_SUPPORT
extern PkTraceBuffer g_pk_trace_buf;
    //set the ppe instance id
    g_pk_trace_buf.instance_id = (uint16_t)(mfspr(SPRN_PIR) & PIR_PPE_INSTANCE_MASK);
#endif  /* PK_TRACE_SUPPORT */
#endif  /* PK_TIMER_SUPPORT */

    //call macro-specific setup
    __hwmacro_setup();
}


