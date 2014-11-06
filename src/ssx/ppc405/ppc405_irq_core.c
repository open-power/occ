// $Id: ppc405_irq_core.c,v 1.1.1.1 2013/12/11 21:03:26 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ppc405/ppc405_irq_core.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppc405_irq_core.c
/// \brief Core IRQ routines required of any PPC405 configuration of SSX
///
///  This file is mostly only a placeholder - where 'extern inline' API
///  functions and 'extern' variables are realized.  A couple of default
///  handlers are also installed here.  The entry points in this file are
///  considered 'core' routines that will always be present at runtime in any
///  SSX application.

#define __PPC405_IRQ_CORE_C__

#include "ssx.h"

/// This function is installed by default for interrupts not explicitly set up
/// by the application.  These interrupts should never fire.

void 
__ppc405_default_irq_handler(void     *arg,
                             SsxIrqId irq,
                             int      critical)
{
    SSX_PANIC(PPC405_DEFAULT_IRQ_HANDLER);
}


/// This function is installed by default to handle the case that the
/// interrupt dispatch code is entered in response to an external critical or
/// non-critical interrupt, but no interrupt is found pending in the interrupt
/// controller.  This should never happen, as it would indicate that a
/// 'glitch' occurred on the external noncritical or critical interrupt input
/// to the PPC405 core.

void __ppc405_phantom_irq_handler(void     *arg,
                                  SsxIrqId irq,
                                  int      critical)
{
    SSX_PANIC(PPC405_PHANTOM_INTERRUPT);
}
    

#undef __PPC405_IRQ_CORE_C__
