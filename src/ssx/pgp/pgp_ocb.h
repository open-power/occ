#ifndef __PGP_OCB_H__
#define __PGP_OCB_H__

// $Id: pgp_ocb.h,v 1.2 2014/02/03 01:30:35 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/pgp_ocb.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pgp_ocb.h
/// \brief OCB unit header.  Local and mechanically generated macros and APIs.

#include "ssx.h"
#include "ppc32.h"

#include "pgp_common.h"
#include "ocb_register_addresses.h"
#include "ocb_firmware_registers.h"

#include "ppc405_irq.h"

#define OCB_TIMER0 0
#define OCB_TIMER1 1

#define OCB_TIMERS 2

#define OCB_TIMER_ONE_SHOT    0
#define OCB_TIMER_AUTO_RELOAD 1

#define OCB_LW_LOG_SIZE_MIN 3
#define OCB_LW_LOG_SIZE_MAX 15

#define OCB_INVALID_ARGUMENT_TIMER         0x00622001
#define OCB_INVALID_ARGUMENT_LW_INIT       0x00622002
#define OCB_INVALID_ARGUMENT_LW_DISABLE    0x00622003
#define OCB_INVALID_ARGUMENT_UNTRUST       0x00622004

#ifndef __ASSEMBLER__

int 
ocb_timer_reset(int timer, 
                int auto_reload,
                int timeout_ns);

#ifdef OCC
int
ocb_timer_setup(int timer, 
                int auto_reload,
                int timeout_ns,
                SsxIrqHandler handler,
                void *arg,
                int priority) INIT_SECTION;
#else
int
ocb_timer_setup(int timer,
        int auto_reload,
        int timeout_ns,
        SsxIrqHandler handler,
        void *arg,
        int priority);
#endif

/// Clear OCB timer status based on the IRQ
///
/// This API can be called from OCB timer interrupt handlers, using the IRQ
/// provided to the handler.  No error checks are provided.

static inline void
ocb_timer_status_clear(SsxIrqId irq)
{
    out32(OCB_OTRN(irq - PGP_IRQ_OCC_TIMER0), OCB_OTRN_TIMEOUT);
}

int
ocb_linear_window_initialize(int channel, uint32_t base, int log_size);

int
ocb_linear_window_disable(int channel);

int
ocb_allow_untrusted_initialize(int channel, int allow_untrusted);
    

#endif  /* __ASSEMBLER__ */

#endif  /* __PGP_OCB_H__ */
