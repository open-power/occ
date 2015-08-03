# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/tools/PowerPCtoPPE/ppe42_divwu.S $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2015
# [+] International Business Machines Corp.
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.
#
# IBM_PROLOG_END_TAG
/// \file ppe42_divwu.S
/// \brief PPC405 word division instructions implemented by PPE ISA
///
/// This file includes implementation for the following PPC405 instructions
///     divwu RT, RA, RB
///
/// Note: PPE ISA specific "fused compare and branch" instructions are used
///
/// Revision History:
///     09-22-2014: Initial Version by daviddu
///

        .file "ppe42_divwu.S"
        .section    ".text"

        /*
        ** Code comment notation:
        **
        ** msw = most-significant (high-order) word, i.e. bits 0..31
        ** lsw = least-significant (low-order) word, i.e. bits 32..63
        ** msh = most-significant (high-order) halfword, i.e. bits 0..15
        ** lsh = least-significant (low-order) halfword, i.e. bits 16..63
        **
        ** LZ = Leading Zeroes
        ** SD = Significant Digits
        ** OW = Register is overwritten, previous value is lost,
        **      correct if previous value is no longer needed.
        ** FU = Register is not overwritten, but its value is no longer needed,
        **      in another word, the register is "free for use".
        **
        ** PPE GPR Registers are: R0-R10, R13, R28-R31
        ** Volatile Registers are: R0, R3-R10
        ** Non-volatile registers are R28-R31
        */

        /*
        ** Caling Convention
        **
        ** R2 and R13 are never saved or restored. In ABI or EABI application
        ** these registers are constant. The other touched volatile registers
        ** will be saved and restored by the subroutines. Note the caller
        ** wont be saving those registers because these subroutines will be
        ** instrumented into caller's body without compiler knowledge.
        **
        ** Note R3 is not saved and restored because it will be changed for
        ** return value anyways, the p2p script will make sure to restore it.
        ** Also CR is hanlded because of compare and branch, but XER/CTR/LR
        ** are not hanlded because they are untouched by the instructions used.
        **
        ** Stack layout:
        **
        ** 0x00 -- R1, Dedicated for Stack Pointer
        ** 0x04 -- slot reserved for LR
        ** 0x08 -- R4, Volatile, Private
        ** 0x0c -- R5, Volatile, Private
        ** 0x10 -- R6, Volatile, Private
        ** 0x14 -- R7, Volatile, Private
        ** 0x18 -- R8, Volatile, Private
        ** 0x1c -- R9, Volatile, Private
        ** 0x20 -- CR, Condition Register
        ** 0x24 --
        **
        ** 0x28 -- Stack Size, Must be 8-byte aligned
        */

        /*
        ** Division Procedures:
        **
        ** __ppe42_divwu(dividend, divisor)
        ** __ppe42_divw(dividend, divisor)
        **
        ** R3 = Input parameter,  dividend. then Return value, quotient.
        ** R4 = Input parameter,  divisor.
        ** R5 = Output parameter, quotient.
        ** R6 = Output parameter, remainder.
        ** R7 = Temporary register, counter.
        **
        ** General Algorithm
        **
        ** Using standard shift and subtract method to emulate
        ** Note: dividend,divisor,quotient,remainder are all 32-bit integers
        **
        ** Precondition Check:
        **
        ** if (divisor == dividend) {
        **     quotient = 1;
        **     remainder = 0;
        ** }
        **
        ** if (divisor == 0) {
        **     quotient = 0;
        **     remainder = 0;
        ** }
        **
        ** if (divisor > dividend) {
        **     quotient = 0;
        **     remainder = dividend;
        ** }
        */

/*****************************************************************************/

        /*
        ** Divide Word Unsigned (__ppe42_divwu)
        **
        ** The implementation uses standard shift and subtract approach.
        ** The following is an example in C. Note the implementation doesnt
        ** exactly follow the C example.
        **
        **  num_of_bits = 32;
        **  while(num_bits) {
        **    dbit = (dividend & 0x80000000) >> 31;
        **    remainder = (remainder << 1) | dbit;
        **    dividend = dividend << 1;
        **    quotient = quotient << 1;
        **    num_of_bits--;
        **    if(remainder < divisor)
        **      continue;
        **    temp = remainder - divisor;
        **    qbit = !((temp & 0x80000000) >> 31);
        **    quotient = quotient | qbit;
        **    remainder = temp;
        **  }
        */

        .align  2
        .global __ppe42_divwu
        .type   __ppe42_divwu, @function

__ppe42_divwu:

        stwu    %r1, -0x28(%r1)                   // allocate stack frame

        stvd    %d4, 0x08(%r1)                    // save off r4 & r5 in stack
        stvd    %d6, 0x10(%r1)                    // save off r6 & r7 in stack
        stvd    %d8, 0x18(%r1)                    // save off r8 & r9 in stack

        mfcr    %r5                               // save off cr
        stw     %r5, 0x20(%r1)                    // store cr in stack

        li      %r5, 1                            // quotient = 1
        li      %r6, 0                            // remainder = 0
        cmplwbc 1, 2, %r3, %r4, __ppe42_divwu_ret // ret(divisor == dividend)

        li      %r5, 0                            // quotient = 0
        li      %r6, 0                            // remainder = 0
        cmpwibc 1, 2, %r4, 0, __ppe42_divwu_ret   // ret(divisor == 0)

        li      %r5, 0                            // quotient = 0
        mr      %r6, %r3                          // remainder = dividend
        cmplwbc 1, 0, %r3, %r4, __ppe42_divwu_ret // ret(divisor > dividend)

        li      %r7, 32                           // num_of_bits = 32

__ppe42_divwu_sas:                                // <<shift and subtract loop>>

        slwi    %r6, %r6, 1                       // remainder <<= 1
        inslwi  %r6, %r3, 1, 31                   // remainder[31] = dividend[0]
        slwi    %r3, %r3, 1                       // dividend <<= 1
        slwi    %r5, %r5, 1                       // quotient <<= 1
        subi    %r7, %r7, 1                       // num_of_bits--
        cmplwbc 1, 0, %r6, %r4, __ppe42_divwu_sas // continue(remainder<divisor)

        sub     %r6, %r6, %r4                     // reminder -= divisor
        addi    %r5, %r5, 1                       // quotient++
        cmpwibc 0, 2, %r7, 0, __ppe42_divwu_sas   // while(num_of_bits)

__ppe42_divwu_ret:                                // <<return subroutine>>

        mr      %r3, %r5                          // r3 is the default return
        lwz     %r5, 0x20(%r1)                    // load cr from stack
        mtcr0   %r5                               // restore cr

        lvd     %d4, 0x08(%r1)                    // restore r4 & r5 from stack
        lvd     %d6, 0x10(%r1)                    // restore r6 & r7 from stack
        lvd     %d8, 0x18(%r1)                    // restore r8 & r9 from stack

        lwz     %r1, 0(%r1)                       // restore stack pointer

        blr                                       // branch back

        .size   __ppe42_divwu, .-__ppe42_divwu


