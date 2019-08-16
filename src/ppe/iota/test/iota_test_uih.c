/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/iota/test/iota_test_uih.c $                           */
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
#include "iota.h"
#include "iota_trace.h"
#include "iota_test_irq.h"
#include "ocb_register_addresses.h"

uint8_t   g_oimr_stack[IOTA_NUM_EXT_IRQ_PRIORITIES];
int       g_oimr_stack_ctr = -1;
int       g_current_prty_level = IOTA_NUM_EXT_IRQ_PRIORITIES - 1;
uint64_t  g_oimr_override = 0x0000000000000000ull;
uint64_t  g_oimr_override_stack[IOTA_NUM_EXT_IRQ_PRIORITIES];

uint32_t G_OCB_OIMR0_CLR = OCB_OIMR0_CLR;
uint32_t G_OCB_OIMR1_CLR = OCB_OIMR1_CLR;
uint32_t G_OCB_OIMR0_OR = OCB_OIMR0_OR;
uint32_t G_OCB_OIMR1_OR = OCB_OIMR1_OR;

uint32_t __ext_irq_handler(void)
{
    int       l_priority_level;
    int       bFound;
    uint64_t  ext_irq_vector_pk;

    // 1. Identify the priority level of the interrupt.
    ext_irq_vector_pk = ((uint64_t)in32(OCB_G0ISR0 + APPCFG_OCC_INSTANCE_ID * 8)) << 32 |
                        (uint64_t)in32(OCB_G0ISR1 + APPCFG_OCC_INSTANCE_ID * 8);
    //CMO-swap above lines comments when we've defined the HI PRTY interrupts correctly, i.e.
    //    right now they fire all the time because by default they are defined as LEVEL
    //    SENSITIVE and ACTIVE LOW.
    bFound = 0;
    l_priority_level = 0;

    do
    {
        if ( ext_irq_vectors_gpe[l_priority_level][IDX_PRTY_VEC] & ext_irq_vector_pk )
        {
            bFound = 1;
            break;
        }
    }
    while (++l_priority_level < (IOTA_NUM_EXT_IRQ_PRIORITIES - 1)); // No need to check DISABLED.

    // 2. Only manipulate OIMR masks for task level prty levels. Let shared non-task
    // IRQs (l_priority_level=0) be processed by the PK kernel in usual fashion.
    if (bFound)
    {
        if(++g_oimr_stack_ctr < IOTA_NUM_EXT_IRQ_PRIORITIES)
        {
            g_oimr_stack[g_oimr_stack_ctr] = g_current_prty_level;
            g_current_prty_level = l_priority_level; // Update prty level tracker.
            g_oimr_override_stack[g_oimr_stack_ctr] = g_oimr_override;
        }
        else
        {
            PK_TRACE("Code bug: OIMR S/R stack counter=%d  >=  max=%d.",
                     g_oimr_stack_ctr, IOTA_NUM_EXT_IRQ_PRIORITIES);
            IOTA_PANIC(UIH_OIMR_STACK_OVERFLOW);
        }

        // Write the new mask for this priority level.
        // First, clear all those IRQs that could possibly interrupt this instance.
        // This includes all those IRQs which belong to this instance as well as
        // those high-prty IRQs shared with the other instances.
        //
        out32(G_OCB_OIMR0_CLR, (uint32_t)(IRQ_VEC_ALL_OUR_IRQS >> 32));
        out32(G_OCB_OIMR1_CLR, (uint32_t)IRQ_VEC_ALL_OUR_IRQS);

        // Second, mask IRQs belonging to this task and lower prty tasks.
        // Note, that we do not modify the permanently disabled IRQs, such as the
        //  _RESERVED_ ones. Nor do we touch other instances' IRQs. Iow, the
        //  IDX_PRTY_LVL_DISABLED mask is  NOT  part of the mask we apply below.
        out32(G_OCB_OIMR0_OR, (uint32_t)((ext_irq_vectors_gpe[l_priority_level][IDX_MASK_VEC] |
                                          g_oimr_override) >> 32) );
        out32(G_OCB_OIMR1_OR, (uint32_t)(ext_irq_vectors_gpe[l_priority_level][IDX_MASK_VEC] |
                                         g_oimr_override) );

    }
    else
    {
        PK_TRACE("A Phantom IRQ fired, ext_irq_vector_pk=0x%08x%08x",
                 (uint32_t)(ext_irq_vector_pk >> 32), (uint32_t)(ext_irq_vector_pk));
        IOTA_PANIC(UIH_OIMR_STACK_OVERFLOW);
    }

    PK_TRACE("IRQ SET: prty_lvl=%d,  g_oimr_stack_ctr=0x%x", g_current_prty_level, g_oimr_stack_ctr);

    return l_priority_level;
}

void __ext_irq_resume()
{
    if (g_oimr_stack_ctr >= 0)
    {
        out32( G_OCB_OIMR0_CLR, (uint32_t)((IRQ_VEC_ALL_OUR_IRQS |
                                            g_oimr_override_stack[g_oimr_stack_ctr]) >> 32));
        out32( G_OCB_OIMR1_CLR, (uint32_t)(IRQ_VEC_ALL_OUR_IRQS |
                                           g_oimr_override_stack[g_oimr_stack_ctr]));
        out32( G_OCB_OIMR0_OR,
               (uint32_t)((ext_irq_vectors_gpe[g_oimr_stack[g_oimr_stack_ctr]][IDX_MASK_VEC] |
                           g_oimr_override) >> 32));
        out32( G_OCB_OIMR1_OR,
               (uint32_t)(ext_irq_vectors_gpe[g_oimr_stack[g_oimr_stack_ctr]][IDX_MASK_VEC] |
                          g_oimr_override));
        // Restore the prty level tracker to the task that was interrupted, if any.
        g_current_prty_level = g_oimr_stack[g_oimr_stack_ctr];
        --g_oimr_stack_ctr;
        // PK_TRACE("IRQ RS: prty_lvl=%d,  g_oimr_stack_ctr=0x%x", g_current_prty_level, g_oimr_stack_ctr);
    }
    else
    {
        PK_TRACE("ERR: Code bug: Messed up OIMR book keeping: g_oimr_stack_ctr=%d",
                 g_oimr_stack_ctr);
        IOTA_PANIC(UIH_OIMR_STACK_UNDERFLOW);
    }
}
