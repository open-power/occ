/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/sbefw/sbeirq.C $                                  */
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
/*
 * @file: ppe/sbe/sbefw/sbeirq.C
 *
 * @brief This sets up and registers SBE ISRs
 *
 */

#include "sbeexeintf.H"
#include "sbeirq.H"
#include "sbetrace.H"


////////////////////////////////////////////////////////////////
// @brief:     SBE control loop ISR:
//               - FIFO new data available
//               - FIFO reset request
//
// @param[in]  i_pArg - Unused
// @param[in]  i_irq  - IRQ number as defined in sbeirq.h
//
////////////////////////////////////////////////////////////////
void sbe_fifo_interrupt_handler (void *i_pArg, PkIrqId i_irq)
{
    #define SBE_FUNC " sbe_fifo_interrupt_handler "
    SBE_ENTER(SBE_FUNC"i_irq=[0x%02X]",i_irq);

    int l_rc = 0;
    switch (i_irq)
    {
        case SBE_IRQ_SBEFIFO_DATA:
        case SBE_IRQ_SBEFIFO_RESET:
            // Mask the interrupt
            pk_irq_disable(i_irq);

            // Unblock the command receiver thread
            l_rc = pk_semaphore_post(&g_sbeSemCmdRecv);
            if (l_rc)
            {
                // If we received an error while posting the semaphore,
                // unmask the interrupt back and assert
                // @TODO via RTC : 129166
                //       Add support for ASSERT here
                SBE_ERROR(SBE_FUNC"pk_semaphore_post failed, rc=[%d]", l_rc);
                pk_irq_enable(i_irq);
            }
            break;

        default:
            SBE_ERROR(SBE_FUNC"Unknown IRQ, assert");
            // @TODO via RTC : 129166
            //       Add support for ASSERT here
            break;
    }
    #undef SBE_FUNC
}

////////////////////////////////////////////////////////////////
// See sbeexeintf.h for more details
////////////////////////////////////////////////////////////////
int sbeIRQSetup (void)
{
    #define SBE_FUNC " sbeIRQSetup "
    int l_rc = 0;

    // Disable the relevant IRQs while we set them up
    pk_irq_disable(SBE_IRQ_SBEFIFO_DATA);
    pk_irq_disable(SBE_IRQ_SBEFIFO_RESET);

    do
    {
        // Register the IRQ handler with PK

        // FIFO New data available interrupt
        l_rc = pk_irq_handler_set(SBE_IRQ_SBEFIFO_DATA,
                              sbe_fifo_interrupt_handler,
                              NULL);

        if(l_rc)
        {
            SBE_ERROR (SBE_FUNC"pk_irq_handler_set failed, IRQ=[0x%02X], "
                   "rc=[%d]", SBE_IRQ_SBEFIFO_DATA, l_rc);
            break;
        }

        // FIFO Reset request
        l_rc = pk_irq_handler_set(SBE_IRQ_SBEFIFO_RESET,
                              sbe_fifo_interrupt_handler,
                              NULL);

        if(l_rc)
        {
            SBE_ERROR (SBE_FUNC"pk_irq_handler_set failed, IRQ=[0x%02X], "
                  "rc=[%d]", SBE_IRQ_SBEFIFO_RESET, l_rc);
            break;
        }

        // Enable the IRQ
        pk_irq_enable(SBE_IRQ_SBEFIFO_RESET);
        pk_irq_enable(SBE_IRQ_SBEFIFO_DATA);
    } while(false);

    return l_rc;
    #undef SBE_FUNC
}
