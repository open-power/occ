/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/occ_gpe1_machine_check_handler.c $               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2018                        */
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
#include "pk_panic_codes.h"
#include "gpe_membuf.h"
#include "pk.h"

#define OCI_ADDR_BAR_MASK 0xf0000000
#define OCI_ADDR_BAR1 0x90000000

extern uint32_t gpe1_machine_check_handler(uint32_t srr0,
                                           uint32_t srr1,
                                           uint32_t edr,
                                           uint32_t stack_ptr);
extern uint32_t g_inband_access_state;

uint32_t gpe1_machine_check_handler(uint32_t srr0,
                                    uint32_t srr1,
                                    uint32_t edr,
                                    uint32_t stack_ptr)
{
    PK_TRACE("GPE1 Machine check! SRR0:%08x SRR1: %08x EDR:%08x StackPtr:%08x",
             srr0,
             srr1,
             edr,
             stack_ptr);

    // It's possible to get back-to-back machine checks for the same condition
    // so MEMBUF_CHANNEL_CHECKSTOP may already be set. Also check that the
    // machine check was due to a MEMBUF PBA Access (PBABAR1)
    if((g_inband_access_state == INBAND_ACCESS_IN_PROGRESS ||
       g_inband_access_state == MEMBUF_CHANNEL_CHECKSTOP) &&
       ((edr & OCI_ADDR_BAR_MASK) == OCI_ADDR_BAR1))
    {
        // Returning this to OCC405 will cause sensor to be removed from
        // active list
        g_inband_access_state = MEMBUF_CHANNEL_CHECKSTOP;

        // The instruction that caused the machine check should
        // be a double word load or store.
        // move the IAR to the instruction after the one that caused
        // the machine check.
        srr0 += 4;
    }
    else
    {
        PK_PANIC( PPE42_MACHINE_CHECK_PANIC );
    }
    return srr0;
}
