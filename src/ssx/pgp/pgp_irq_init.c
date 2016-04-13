/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/pgp_irq_init.c $                                  */
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
/// \file pgp_irq_init.c
/// \brief PGP IRQ initialization code for SSX
///
///  The entry points in this file are initialization rotines that could be
///  eliminated/deallocated by the application to free up storage if they are
///  no longer needed after initialization.

#include "ssx.h"

/// Define the polarity and trigger condition for an interrupt.
///
/// It is up to the application to take care of any side effects that may
/// occur from programming or reprogramming the interrupt controller.  For
/// example, changing edge/level sensitivity or active level may set or clear
/// interrupt status in the controller.
///
/// Note that SSX allows this API to be called from any context, and changes
/// to the interrupt controller are made from an SSX_CRITICAL critical
/// section.
///
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_ARGUMENT_IRQ_SETUP One or more arguments are invalid, 
/// including an invalid \a irq, or invalid \a polarity or \a trigger parameters.

int
ssx_irq_setup(SsxIrqId irq,
              int      polarity,
              int      trigger)
{
    SsxMachineContext ctx;
    uint32_t oitr, oeipr;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(!PGP_IRQ_VALID(irq) ||
                     !((polarity == SSX_IRQ_POLARITY_ACTIVE_HIGH) ||
                       (polarity == SSX_IRQ_POLARITY_ACTIVE_LOW)) ||
                     !((trigger == SSX_IRQ_TRIGGER_LEVEL_SENSITIVE) ||
                       (trigger == SSX_IRQ_TRIGGER_EDGE_SENSITIVE)),
                     SSX_INVALID_ARGUMENT_IRQ_SETUP);
    }

    ssx_critical_section_enter(SSX_CRITICAL, &ctx);

    oeipr = in32(OCB_OIEPR(irq));
    if (polarity == SSX_IRQ_POLARITY_ACTIVE_HIGH) {
        out32(OCB_OIEPR(irq), oeipr | PGP_IRQ_MASK32(irq));
    } else {
        out32(OCB_OIEPR(irq), oeipr & ~PGP_IRQ_MASK32(irq));
    }

    oitr = in32(OCB_OITR(irq));
    if (trigger == SSX_IRQ_TRIGGER_EDGE_SENSITIVE) {
        out32(OCB_OITR(irq), oitr | PGP_IRQ_MASK32(irq));
    } else {
        out32(OCB_OITR(irq), oitr & ~PGP_IRQ_MASK32(irq));
    }

    ssx_critical_section_exit(&ctx);

    return SSX_OK;
}


/// (Re)define the IRQ handler and priority for an interrupt.
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// Note that SSX allows this API to be called from any context, and changes
/// to the interrupt controller are made from an SSX_CRITICAL critical
/// section.
///
/// \retval 0 Successful completion
///
/// \retval -SSX_INVALID_ARGUMENT_IRQ_HANDLER One or more arguments are 
/// invalid, including an invalid \a irq, a  null (0) \a handler, 
/// or invalid \a priority.

int
ssx_irq_handler_set(SsxIrqId      irq,
                    SsxIrqHandler handler,
                    void          *arg,
                    int           priority)
{
    SsxMachineContext ctx;
    uint32_t ocir;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(!PGP_IRQ_VALID(irq) ||
                     (handler == 0) ||
                     !((priority == SSX_NONCRITICAL) ||
                       (priority == SSX_CRITICAL)),
                     SSX_INVALID_ARGUMENT_IRQ_HANDLER);
    }

    ssx_critical_section_enter(SSX_CRITICAL, &ctx);

    ocir = in32(OCB_OCIR(irq));
    if (priority == SSX_CRITICAL) {
        out32(OCB_OCIR(irq), ocir | PGP_IRQ_MASK32(irq));
    } else {
        out32(OCB_OCIR(irq), ocir & ~PGP_IRQ_MASK32(irq));
    }

    __ppc405_irq_handlers[irq].handler = handler;
    __ppc405_irq_handlers[irq].arg = arg;

    ssx_critical_section_exit(&ctx);

    return SSX_OK;
}


/// Set or clear interrupt debug mode explicitly.

void
ssx_irq_debug_set(SsxIrqId irq, int value)
{
    SsxMachineContext ctx;
    uint32_t ouder;

    ssx_critical_section_enter(SSX_CRITICAL, &ctx);

    ouder = in32(OCB_OUDER(irq));
    if (value) {
        out32(OCB_OUDER(irq), ouder | PGP_IRQ_MASK32(irq));
    } else {
        out32(OCB_OUDER(irq), ouder & ~PGP_IRQ_MASK32(irq));
    }

    ssx_critical_section_exit(&ctx);
}


    

    
        

    

    
    


    
