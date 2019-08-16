/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/iota/test/iota_test_irq.h $                           */
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
#if !defined(__IOTA_TEST_IRQ_H__)
    #define __IOTA_TEST_IRQ_H__
#endif

// Group0: hi-prty IRQs
#define IRQ_VEC_PRTY0_GPE 0xffffffef00000000ull
// Group1: IPC
#define IRQ_VEC_PRTY1_GPE 0x0000001000000000ull
// Group2: low-prty IRQs
#define IRQ_VEC_PRTY2_GPE 0x00000000ffffffffull

//Make sure all ext interrupts have been accounted for
// This should be 0xFFFFFFFFFFFFFFFF
#define IRQ_VEC_PRTY_ALL_CHECK   (IRQ_VEC_PRTY0_GPE | \
                                  IRQ_VEC_PRTY1_GPE | \
                                  IRQ_VEC_PRTY2_GPE )

#define IRQ_VEC_PRTY_XOR_CHECK   (IRQ_VEC_PRTY0_GPE ^ \
                                  IRQ_VEC_PRTY1_GPE ^ \
                                  IRQ_VEC_PRTY2_GPE )

// Include all but the last priority in ALL_OUR_IRQS
#define IRQ_VEC_ALL_OUR_IRQS (IRQ_VEC_PRTY0_GPE | \
                              IRQ_VEC_PRTY1_GPE)

#define IDX_PRTY_VEC 0
#define IDX_MASK_VEC 1
extern const uint64_t ext_irq_vectors_gpe[IOTA_NUM_EXT_IRQ_PRIORITIES][2];
extern uint32_t G_OCB_OIMR0_CLR;
extern uint32_t G_OCB_OIMR1_CLR;
extern uint32_t G_OCB_OIMR0_OR;
extern uint32_t G_OCB_OIMR1_OR;

