# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/tools/PowerPCtoPPE/ppe42_mulhwu.S $
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
/// \file ppe42_mulhwu.S
/// \brief PPC405 word multiplication instructions implemented by PPE ISA
///
/// This file includes implementation for the following PPC405 instructions
///     mulhwu
///
/// Note: PPE ISA specific "fused compare and branch" instructions are used
///
/// Revision History:
///     09-15-2014: Initial Version by daviddu
///

        .file "ppe42_mulhwu.S"
        .section    ".text"

        /*
        ** Code comment notation:
        **
        ** msw = most-significant (high-order) word, i.e. bits 0..31
        ** lsw = least-significant (low-order) word, i.e. bits 32..63
        ** msh = most-significant (high-order) halfword, i.e. bits 0..15
        ** lsh = least-significant (low-order) halfword, i.e. bits 16..63
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
        ** Multiplication Procedures:
        **
        ** __ppe42_mulhwu(U,V)
        ** __ppe42_mulhw(U,V)
        ** __ppe42_mullw(U,V)
        **
        ** R3:R4      = Input parameter, multipliers: U, V.
        ** R3         = Output parameter, either product.msh or product.lsh.
        ** R5-R9      = Temporary registers
        **
        ** General Algorithm
        **
        ** Using PPC405 ISA instruction 'mullhw' to emulate
        ** Note: U,V,A,B,C,D,Product are all 32-bit integers(with msh and lsh)
        **
        **             U.msh U.lsh
        **  X          V.msh V.lsh
        ** ------------------------
        **             A.msh A.lsh
        **       B.msh B.lsh
        **       C.msh C.lsh
        ** D.msh D.lsh
        ** ------------------------
        ** Product.msw Product.lsw
        **
        ** __ppe42_mulhwu: Return Product.msh (unsigned)
        ** __ppe42_mulhw:  Return Product.msh (signed)
        ** __ppe42_mullw:  Return Product.lsh
        **
        ** Precondition Check:
        **
        ** if( U == 0 || V == 0 ) return P=0;
        */

/*****************************************************************************/

        /*
        ** Multiply High Word Unsigned (__ppe42_mulhwu)
        **
        ** r5 = U[16:31] or U.lsh   |   r5 = r3 & 0xffff
        ** r3 = U[0:15]  or U.msh   |   r3 = r3 >> 16     (r3 OW)
        ** r6 = V[16:31] or V.lsh   |   r6 = r4 & 0xffff
        ** r4 = V[0:15]  or V.msh   |   r4 = r4 >> 16     (r4 OW)
        **
        ** 4th column(drop A.lsh):
        ** A = U.lsh * V.lsh [32]   |   r7 = r5 * r6
        ** A = A.msh         [16]   |   r7 = r7 >> 16     (r7 OW)
        **
        ** 3rd column(A = A.msh + B.lsh + C.lsh):
        ** B = U.msh * U.lsh [32]   |   r6 = r3 * r6      (r6 OW)
        ** T = B.lsh         [16]   |   r8 = r6 & 0xffff
        ** B = B.msh         [16]   |   r6 = r6 >> 16     (r6 OW)
        ** A = T + A         [16]   |   r7 = r8 + r7      (r7 OW, r8 FU)
        **
        ** C = U.lsh * V.msh [32]   |   r5 = r5 * r4      (r5 OW)
        ** T = C.lsh         [16]   |   r8 = r5 & 0xffff  (r8 OW)
        ** C = C.msh         [16]   |   r5 = r5 >> 16     (r5 OW)
        ** A = T + A         [16]   |   r7 = r8 + r7      (r7 OW, r8 FU)
        **
        ** 2nd column(A = 3rd_carry + B.msh + C.msh):
        ** A = A.msh         [16]   |   r7 = r7 >> 16     (r7 OW)
        ** A = A + B         [16]   |   r7 = r7 + r6      (r7 OW, r6 FU)
        ** A = A + C         [16]   |   r7 = r7 + r5      (r7 OW, r5 FU)
        **
        ** 1st column(A = D + A):
        ** D = U.msh * V.msh [32]   |   r3 = r3 * r4      (r3 OW, r4 FU)
        ** P = D + A         [32]   |   r5 = r3 + r7      (r3, r7 FU)
        **
        ** Return P(r3=r5) as Product.msw unsigned
        **
        ** Note: the implementation can be even shorter, the current
        **       implementation is ensuring the overflow is avoided
        **       by always adding 16 bits integer together.
        */

        .align  2
	.global __ppe42_mulhwu
        .type   __ppe42_mulhwu, @function

__ppe42_mulhwu:

        stwu    %r1, -0x28(%r1)                   // allocate stack frame

        stvd    %d4, 0x08(%r1)                    // save off r4 & r5 in stack
        stvd    %d6, 0x10(%r1)                    // save off r6 & r7 in stack
        stvd    %d8, 0x18(%r1)                    // save off r8 & r9 in stack

        mfcr    %r5                               // save off cr
        stw     %r5, 0x20(%r1)                    // store cr in stack

        li      %r5, 0                            // r5 = 0
        cmpwibc 1, 2, %r3, 0, __ppe42_mulhwu_ret  // U=0 -> ret
        cmpwibc 1, 2, %r4, 0, __ppe42_mulhwu_ret  // V=0 -> ret

        extrwi  %r5, %r3, 16, 16
        srwi    %r3, %r3, 16
        extrwi  %r6, %r4, 16, 16
        srwi    %r4, %r4, 16

        mullhwu %r7, %r5, %r6
        srwi    %r7, %r7, 16

        mullhwu %r6, %r3, %r6
        extrwi  %r8, %r6, 16, 16
        srwi    %r6, %r6, 16
        add     %r7, %r8, %r7

        mullhwu %r5, %r5, %r4
        extrwi  %r8, %r5, 16, 16
        srwi    %r5, %r5, 16
        add     %r7, %r8, %r7

        srwi    %r7, %r7, 16
        add     %r7, %r7, %r6
        add     %r7, %r7, %r5

        mullhwu %r3, %r3, %r4
        add     %r5, %r3, %r7

__ppe42_mulhwu_ret:

        mr      %r3, %r5                          // put return value to r3

        lwz     %r5, 0x20(%r1)                    // load cr from stack
        mtcr0   %r5                               // restore cr

        lvd     %d4, 0x08(%r1)                    // restore r4 & r5 from stack
        lvd     %d6, 0x10(%r1)                    // restore r6 & r7 from stack
        lvd     %d8, 0x18(%r1)                    // restore r8 & r9 from stack

        lwz     %r1, 0(%r1)                       // restore stack pointer

        blr

        .size __ppe42_mulhwu, .-__ppe42_mulhwu


