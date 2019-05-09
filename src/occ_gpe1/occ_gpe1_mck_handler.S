/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/occ_gpe1_mck_handler.S $                         */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2018,2019                        */
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

.nolist
#include "pk.h"
.list

.section .text, "ax", @progbits
.global __gpe1_machine_check_handler
.global __pk_ctx_pop

__gpe1_machine_check_handler:
#if !defined(__PPE42X__)
    stwu    %r1,    -PK_CTX_SIZE(%r1)
    stw     %r0,    PK_CTX_GPR0(%r1)
    stvd    %d3,    PK_CTX_GPR3(%r1)
    stvd    %d5,    PK_CTX_GPR5(%r1)
    stvd    %d7,    PK_CTX_GPR7(%r1)
    stvd    %d9,    PK_CTX_GPR9(%r1)
    stvd    %d28,   PK_CTX_GPR28(%r1)
    stvd    %d30,   PK_CTX_GPR30(%r1)
    mflr    %r3
    stw     %r3,    PK_CTX_LR(%r1)
    mfcr    %r3
    mfsprg0 %r4
    stvd    %d3,    PK_CTX_CR(%r1)
    mfxer   %r3
    mfctr   %r4
    stvd    %d3,    PK_CTX_XER(%r1)
    mfsrr0  %r3
    mfsrr1  %r4
    stvd    %d3,    PK_CTX_SRR0(%r1)
#else
    stcxu   %r1,    -PK_CTX_SIZE(%r1)
    mfsrr0  %r3
    mfsrr1  %r4
#endif

    mfedr   %r5

    bl  gpe1_machine_check_handler
    stw %r3,    PK_CTX_SRR0(%r1)

#if !defined(__PPE42X__)
    lwz     %r0,    PK_CTX_GPR0(%r1)
    lvd     %d7,    PK_CTX_SRR0(%r1)
    mtsrr1  %r8
    mtsrr0  %r7
    lvd     %d5,    PK_CTX_XER(%r1)
    mtctr   %r6
    mtxer   %r5
    lvd     %d30,   PK_CTX_GPR30(%r1)
    lvd     %d28,   PK_CTX_GPR28(%r1)
    lvd     %d9,    PK_CTX_GPR9(%r1)
    lvd     %d7,    PK_CTX_GPR7(%r1)
    lvd     %d5,    PK_CTX_GPR5(%r1)
    lvd     %d3,    PK_CTX_CR(%r1)  ## CR,SPRG0
    mtcr0   %r3
    lwz     %r4,    PK_CTX_LR(%r1)
    mtlr    %r4
    lvd     %d3,    PK_CTX_GPR3(%r1)
    addi    %r1,    %r1, PK_CTX_SIZE
#else
    lcxu    %r1, PK_CTX_SIZE(%r1)
#endif
    rfi

