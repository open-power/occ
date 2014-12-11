// $Id: ppc405_init.c,v 1.1.1.1 2013/12/11 21:03:26 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ppc405/ppc405_init.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppc405_init.c
/// \brief PPC405 initialization routines
///
/// The entry points in this file are routines that are typically used during
/// initialization, and their code space could be deallocated and recovered if
/// no longer needed by the application after initialization.

#include "ssx.h"

// Note that __ppc405_system_setup() is called from the SSX bootloader early
// in the initialization, at a point before the aplication has enabled
// critical or external interruts.

void
__ppc405_system_setup()
{
    SsxIrqId irq;

    // Initialize the interrupt vectors.

    for (irq = 0; irq < PPC405_IRQS; irq++) {
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

    //  Call system-specific setup

#ifdef CHIP_PGP
    void __pgp_setup();
    __pgp_setup();
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


