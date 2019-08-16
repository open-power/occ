/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/baselib/div32.S $                                     */
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
#if (!defined(__PPE_QME) && !defined(APPCFG_OCC_INSTANCE_ID))
    .nolist
#include "ppe42_asm.h"
    .list
    .section    .text.divide,"ax",@progbits
    .align 5

    ## The following code MUST be 32 byte aligned and fit in Icache
    ## for optimum performance.
udivmodsi4_loop:
    cmplwbge    %r4, %r3, .L1
    bdz         .L2
    bwltz       %r4,.L1
    rlwinm      %r4, %r4, 1, 0, 30
    rlwinm      %r9, %r9, 1, 0, 30
    b           udivmodsi4_loop
.L2:
    li          %r9,0
.L1:
    li          %r10,0
.L5:
    bwz         %r9, .L3
    cmplwblt    %r3, %r4, .L4
    subf        %r3, %r4, %r3
    or          %r10, %r10, %r9
.L4:
    rlwinm      %r9, %r9, 31, 1, 31
    rlwinm      %r4, %r4, 31, 1, 31
    b           .L5
.L3:
    bwnz        %r5, .L6
    mr          %r3,%r10
.L6:
    blr

    .section    .text.__udivsi3,"ax",@progbits
    .align      2
    .globl  __udivsi3
    .type   __udivsi3 @function
__udivsi3:
    li          %r5, 0
    li          %r10, 33
    mtctr       %r10
    li          %r9,1
    b           udivmodsi4_loop


    .section    .text.__umodsi3,"ax",@progbits
    .align      2
    .globl __umodsi3
    .type   __umodsi3, @function
__umodsi3:
    li          %r5, 1
    li          %r10, 33
    mtctr       %r10
    li          %r9,1
    b           udivmodsi4_loop
#endif

