/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/iota/test/iota_test_irq_priority_table.c $            */
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
#include <stdint.h>
#include "iota.h"
#include "iota_test_irq.h"


const uint64_t ext_irq_vectors_gpe[IOTA_NUM_EXT_IRQ_PRIORITIES][2] =
{
    // IDX_PRTY_VEC    , MASK
    {IRQ_VEC_PRTY0_GPE, 0xFFFFFFFFFFFFFFFFul},
    {IRQ_VEC_PRTY1_GPE, IRQ_VEC_PRTY1_GPE | IRQ_VEC_PRTY2_GPE},
    // last entry is for irqs not handled by this engine.
    {IRQ_VEC_PRTY2_GPE, IRQ_VEC_PRTY2_GPE}
};

compile_assert(ALL_CHECK, IRQ_VEC_PRTY_ALL_CHECK == 0xfffffffffffffffful);
compile_assert(XOR_CHECK, IRQ_VEC_PRTY_XOR_CHECK == 0xfffffffffffffffful);
