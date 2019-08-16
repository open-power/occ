/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/boltonlib/gpe/gpe_irq_init.c $                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2019                             */
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

/// \file gpe_irq_init.c
/// \brief OCCHW IRQ initialization code for PK
///
///  The entry points in this file are initialization rotines that could be
///  eliminated/deallocated by the application to free up storage if they are
///  no longer needed after initialization.

#if defined(__PK__)
#include "pk.h"

/// Define the polarity and trigger condition for an interrupt.
///
/// It is up to the application to take care of any side effects that may
/// occur from programming or reprogramming the interrupt controller.  For
/// example, changing edge/level sensitivity or active level may set or clear
/// interrupt status in the controller.
///
/// Note that PK allows this API to be called from any context, and changes
/// to the interrupt controller are made from a critical section.
///
/// Return values other then PK_OK (0) are errors; see \ref pk_errors
///
/// \retval 0 Successful completion
///
/// \retval -PK_INVALID_ARGUMENT_IRQ_SETUP One or more arguments are invalid,
/// including an invalid \a irq, or invalid \a polarity or \a trigger parameters.

int
pk_irq_setup(PkIrqId irq,
             int      polarity,
             int      trigger)
{
    PkMachineContext ctx;

    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF(!OCCHW_IRQ_VALID(irq) ||
                    !OCCHW_IRQ_OWNED(irq) ||
                    !((polarity == PK_IRQ_POLARITY_ACTIVE_HIGH) ||
                      (polarity == PK_IRQ_POLARITY_ACTIVE_LOW)) ||
                    !((trigger == PK_IRQ_TRIGGER_LEVEL_SENSITIVE) ||
                      (trigger == PK_IRQ_TRIGGER_EDGE_SENSITIVE)),
                    PK_INVALID_ARGUMENT_IRQ_SETUP);
    }

    pk_critical_section_enter(&ctx);

    if (polarity == PK_IRQ_POLARITY_ACTIVE_HIGH)
    {
        out32(OCCHW_OIEPR_OR(irq), OCCHW_IRQ_MASK32(irq));
    }
    else
    {
        out32(OCCHW_OIEPR_CLR(irq), OCCHW_IRQ_MASK32(irq));
    }

    if (trigger == PK_IRQ_TRIGGER_EDGE_SENSITIVE)
    {
        out32(OCCHW_OITR_OR(irq), OCCHW_IRQ_MASK32(irq));
    }
    else
    {
        out32(OCCHW_OITR_CLR(irq), OCCHW_IRQ_MASK32(irq));
    }

    pk_critical_section_exit(&ctx);

    return PK_OK;
}


/// (Re)define the IRQ handler and priority for an interrupt.
/// Return values other then PK_OK (0) are errors; see \ref pk_errors
///
/// Note that PK allows this API to be called from any context, and changes
/// to the interrupt controller are made from a critical section.
///
/// \retval 0 Successful completion
///
/// \retval -PK_INVALID_ARGUMENT_IRQ_HANDLER One or more arguments are
/// invalid, including an invalid \a irq, a  null (0) \a handler,
/// or invalid \a priority.

int
pk_irq_handler_set(PkIrqId      irq,
                   PkIrqHandler handler,
                   void*          arg)
{
    PkMachineContext ctx;

    if (PK_ERROR_CHECK_API)
    {
        PK_ERROR_IF(!OCCHW_IRQ_VALID(irq) ||
                    (handler == 0),
                    PK_INVALID_ARGUMENT_IRQ_HANDLER);
    }

    pk_critical_section_enter(&ctx);

    __ppe42_irq_handlers[irq].handler = handler;
    __ppe42_irq_handlers[irq].arg = arg;

    pk_critical_section_exit(&ctx);

    return PK_OK;
}

#endif /* __PK__ */

