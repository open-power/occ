/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/gpe/gpe_init.c $                                   */
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

/// \file gpe_init.c
/// \brief PK initialization for GPE
///
/// The entry points in this routine are used during initialization.  This
/// code space can be deallocated and reassigned after application
/// initialization if required.

#include "pk.h"
#include "ocb_register_addresses.h"

/// GPE environment initial setup.
///
/// This is setup common to all GPE HW Macro applications.  This setup takes place
/// during boot, before main() is called.

void
__hwmacro_setup(void)
{
    uint64_t oirrA;
    uint64_t oirrB;
    uint64_t oirrC;
    uint64_t owned_actual;
    uint64_t reverse_polarity;

    //verify that this code is running on the correct GPE instance (one time check)
    if((mfspr(SPRN_PIR) & PIR_PPE_INSTANCE_MASK) != APPCFG_OCC_INSTANCE_ID)
    {
        //APPCFG_OCC_INSTANCE_ID does not match actual instance ID!
        PK_PANIC(OCCHW_INSTANCE_MISMATCH);
    }

#if (APPCFG_OCC_INSTANCE_ID == OCCHW_IRQ_ROUTE_OWNER)
    //If this instance is the owner of the interrupt routting registers
    //then write the routing registers for all OCC interrupts.
    //This instance must be the first instance to run within the OCC
    //This will be done while all external interrupts are masked.
    PKTRACE("Initializing External Interrupt Routing Registers");
    out32(OCB_OIMR0_OR, 0xffffffff);
    out32(OCB_OIMR1_OR, 0xffffffff);
    out32(OCB_OIRR0A, (uint32_t)(g_ext_irqs_routeA >> 32));
    out32(OCB_OIRR1A, (uint32_t)g_ext_irqs_routeA);
    out32(OCB_OIRR0B, (uint32_t)(g_ext_irqs_routeB >> 32));
    out32(OCB_OIRR1B, (uint32_t)g_ext_irqs_routeB);
    out32(OCB_OIRR0C, (uint32_t)(g_ext_irqs_routeC >> 32));
    out32(OCB_OIRR1C, (uint32_t)g_ext_irqs_routeC);
#endif

    //Determine from the routing registers which irqs are owned by this instance
    //NOTE: If a bit is not set in the routeA register, it is not owned by a GPE

    oirrA = ((uint64_t)in32(OCB_OIRR0A)) << 32;
    oirrA |= in32(OCB_OIRR1A);
    oirrB = ((uint64_t)in32(OCB_OIRR0B)) << 32;
    oirrB |= in32(OCB_OIRR1B);
    oirrC = ((uint64_t)in32(OCB_OIRR0C)) << 32;
    oirrC |= in32(OCB_OIRR1C);

    //All interrupts routed to a GPE will have a bit set in routeA
    owned_actual = oirrA;

    //wittle it down by bits in the routeB register
#if APPCFG_OCC_INSTANCE_ID & 0x2
    owned_actual &= oirrB;
#else
    owned_actual &= ~oirrB;
#endif

    //wittle it down further by bits in the routeC register
#if APPCFG_OCC_INSTANCE_ID & 0x1
    owned_actual &= oirrC;
#else
    owned_actual &= ~oirrC;
#endif

    //Panic if we don't own the irqs we were expecting
    //NOTE: we don't panic if we are given more IRQ's than expected
    if((owned_actual & g_ext_irqs_owned) != g_ext_irqs_owned)
    {
        //IRQ's were not routed to us correctly.
        PK_PANIC(OCC_IRQ_ROUTING_ERROR);
    }

    //Mask all external interrupts owned by this instance
    //(even the ones given to us that we weren't expecting)
    out32(OCB_OIMR0_OR, (uint32_t)(owned_actual >> 32));
    out32(OCB_OIMR1_OR, (uint32_t)owned_actual);

    //Set the interrupt type for all interrupts owned by this instance
    out32(OCB_OITR0_CLR, (uint32_t)(g_ext_irqs_owned >> 32));
    out32(OCB_OITR1_CLR, (uint32_t)g_ext_irqs_owned);
    out32(OCB_OITR0_OR, (uint32_t)(g_ext_irqs_type >> 32));
    out32(OCB_OITR1_OR, (uint32_t)g_ext_irqs_type);

    //Set the interrupt polarity for all interrupts owned by this instance
    out32(OCB_OIEPR0_CLR, (uint32_t)(g_ext_irqs_owned >> 32));
    out32(OCB_OIEPR1_CLR, (uint32_t)g_ext_irqs_owned);
    out32(OCB_OIEPR0_OR, (uint32_t)(g_ext_irqs_polarity >> 32));
    out32(OCB_OIEPR1_OR, (uint32_t)g_ext_irqs_polarity);

    //clear the status of all external interrupts owned by this instance
    out32(OCB_OISR0_CLR, ((uint32_t)(g_ext_irqs_owned >> 32)));
    out32(OCB_OISR1_CLR, ((uint32_t)g_ext_irqs_owned));

    //set the status for interrupts that have reverse polarity
    reverse_polarity = ~g_ext_irqs_polarity & g_ext_irqs_owned;
    out32(OCB_OISR0_OR, ((uint32_t)(reverse_polarity >> 32)));
    out32(OCB_OISR1_OR, ((uint32_t)reverse_polarity));

    //Unmask the interrupts owned by this instance that are to be enabled by default
    out32(OCB_OIMR0_CLR, (uint32_t)(g_ext_irqs_enable >> 32));
    out32(OCB_OIMR1_CLR, (uint32_t)g_ext_irqs_enable);

    //Wait for the last out32 operation to complete
    sync();

}
