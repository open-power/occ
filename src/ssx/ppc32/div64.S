# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ssx/ppc32/div64.S $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2014,2016
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
/// \file div64.S
/// \brief Unsigned 64/64 bit division
///
/// This is IBM code, originally part of OS Open.  The code has been slightly
/// modified from its original form, both to be compatible with SSX and to
/// change the function prototype slightly.
///
/// The code was provided by Matt Tyrlik in Raleigh.

/* @#START#@
**
**      PSCN (Power Service and Control Network)
**      Cage Controller OS Open Code
**
**      (C) Copyright International Business Machines Corporation 2002
**      All Rights Reserved
**      Licensed Material  -  Program Property of I B M
**      Refer to copyright instructions: Form G120-2083
**
** Module:
**      div64.s
**
** Description:
**      Divide 64 bit unsigned values on 32 bit CPU
**      div64(uint64_t dividen, uint64_t divisor, 
**            uint64_t *quotient, uint64_t *remainder)
**      
**      Original source from:
**      "The PowerPC Compiler Writer's Guide", pp62-65 by
**      Steve Hoxey, Faraydon Karim, Bill Hay, Hank Warray,
**      published by Warthman Associates, 240 Hamilton Avenue,
**      Palo Alto, CA 94301, USA, 1996 for IBM.
**      ISBN 0-9649654-0-2.
**
**      This version checks for divisor equal to zero.
**
** Environment:
**      OS Open (XCOFF)
**
** Linkage:
**      AIX 4.3.3
**
** @author
**      Thomas Richter
**
** History:
** Date      Author      Description
** -----------------------------------------------------------------------------
** 23-Sep-02 Richter     Created
**
** @#END#@*/

        .nolist
#include "ssx.h"
        .list
        
        .set    r0, 0
        .set    r1, 1
        .set    r2, 2
        .set    r3, 3
        .set    r4, 4
        .set    r5, 5
        .set    r6, 6
        .set    r7, 7
        .set    r8, 8
        .set    r9, 9
        .set    r10, 10
        .set    r11, 11
        .set    r12, 12

        .global_function __ppc32_udiv64

        /*
        ** Code comment notation: 
        ** msw = most-significant (high-order) word, i.e. bits 0..31 
        ** lsw = least-significant (low-order) word, i.e. bits 32..63
        ** LZ = Leading Zeroes 
        ** SD = Significant Digits 
        ** 
        ** R3:R4 = Input parameter, dividend.
        ** R5:R6 = Input parameter, divisor.
        ** R7 = Output parameter, pointer to quotient.
        ** R8 = Output parameter, pointer to remainder.
        **
        ** Pointer arguments point to a uint64_t.
        **
        ** Division is achieved using a shift/rotate/substract algorithsm
        ** described above.
        ** The registers are used as follows:
        ** R3:R4 = dividend (upper 32bits:lower 32bits)
        ** R5:R6 = divisor (upper 32bits:lower 32bits)
        ** 
        ** R7:R8 = temporary 64 bit register (upper 32bits:lower 32bits)
        ** count the number of leading 0s in the dividend 
        **
        ** Here is the description from the book. The dividend is placed
        ** in the low order part of a 4 (32bit) register sequence named
        ** tmp-high:tmp-low:dividend-high:dividend:low or tmp:dvd for short.
        ** 
        ** Each iteration includes the following steps:
        ** 1. Shift tmp:dvd by one bit to the left.
        ** 2. Subtract the divisor from tmp. This is a 64 bit operation.
        ** 3. If result is greater than or equal, place result in tmp and
        **    set the low order bit of dividend
        ** 4. If result is negative, do not modify tmp and
        **    clear the low order bit of dividend
        ** 5. If the number of iterations is less than the width of the
        **    dividend, goto step 1.
        **
        ** Now the algorithm can be improved by reducing the number of
        ** iterations to be executed.
        ** 1. Calculate the leading zeroes of the dividend.
        ** 2. Calculate the leading zeroes of the divisor.
        ** 3. Calculate the significant ones of the dividend.
        ** 4. Calculate the significant ones of the divisor.
        **
        ** Initial tmp := dvd >> (dvd.SD - dvs.SD)
        ** Initial dvd := dvd << (dvd.LZ + dvs.SD)
        ** Loops: dvd.SD - dvs.SD.
        **
        ** Warning: Special care must be taken if dvd.LZ == dvs.LZ. The code
        ** below does so by reducing the number of dvs.SD by one. This leads
        ** to the loop being executed 1 more time than really necessary,
        ** but avoids to check for the case when dvd.LZ == dvs.LZ.
        ** This case (dvd.LZ == dvs.LZ) only checks for the number of leading
        ** zeroes, but does not check if dividend is really greater than the
        ** divisor.
        ** Consider 16/17, both have an LZ value of 59. The code sets dvs.LZ
        ** 60. This resutls in dvs.SD to 4, thus one iteration after which
        ** tmp is the remainder 16.
        */

__ppc32_udiv64: // SSX
        /* Save result pointers on volatile spare registers */
        ori     r12, r8, 0              /* Save remainder address */
        ori     r11, r7, 0              /* Save quotient address */

        /* count the number of leading 0s in the dividend */
        cmpwi   cr0, r3, 0      /* dvd.msw == 0? */
        cntlzw  r0, r3          /* R0 = dvd.msw.LZ */
        cntlzw  r9, r4          /* R9 = dvd.lsw.LZ */
        bne     cr0, lab1       /* if(dvd.msw == 0) dvd.LZ = dvd.msw.LZ */
        addi    r0, r9, 32      /* dvd.LZ = dvd.lsw.LZ + 32 */
lab1: 
        /* count the number of leading 0s in the divisor */
        cmpwi   cr0, r5, 0      /* dvd.msw == 0? */
        cmpwi   cr1, r6, 0      /* dvd.lsw == 0? */
        cntlzw  r9, r5          /* R9 = dvs.msw.LZ */
        cntlzw  r10, r6         /* R10 = dvs.lsw.LZ */
        bne     cr0, lab2       /* if(dvs.msw == 0) dvs.LZ = dvs.msw.LZ */
        beq     cr1, lab10      /* dvs.msw == 0 */
        addi    r9, r10, 32     /* dvs.LZ = dvs.lsw.LZ + 32 */

lab2:
        /* Determine shift amounts to minimize the number of iterations  */
        cmpw    cr0, r0, r9     /* Compare dvd.LZ to dvs.LZ */
        subfic  r10, r0, 64     /* R10 = dvd.SD */
        bgt     cr0, lab9       /* if(dvs > dvd) quotient = 0 */
        addi    r9, r9, 1       /* See comment above. ++dvs.LZ (or --dvs.SD) */
        subfic  r9, r9, 64      /* R9 = dvs.SD */
        add     r0, r0, r9      /* (dvd.LZ + dvs.SD) = left shift of dvd for */
                                /* initial dvd */
        subf    r9, r9, r10     /* (dvd.SD - dvs.SD) = right shift of dvd for */
                                /* initial tmp */
        mtctr   r9              /* Number of iterations = dvd.SD - dvs.SD */
        
        /* R7:R8 = R3:R4 >> R9 */
        cmpwi   cr0, r9, 32     /* compare R9 to 32 */
        addi    r7, r9, -32
        blt     cr0, lab3       /* if(R9 < 32) jump to lab3 */
        srw     r8, r3, r7      /* tmp.lsw = dvd.msw >> (R9 - 32) */
        addi    r7, r0, 0       /* tmp.msw = 0 */
        b       lab4 

lab3: 
        srw     r8, r4, r9      /* R8 = dvd.lsw >> R9 */
        subfic  r7, r9, 32 
        slw     r7,r3,r7        /* R7 = dvd.msw << 32 - R9 */
        or      r8, r8,r7       /* tmp.lsw = R8 | R7 */
        srw     r7,r3,r9        /* tmp.msw = dvd.msw >> R9 */
lab4:
        /* R3:R4 = R3:R4 << R0 */
        cmpwi   cr0, r0, 32     /* Compare R0 to 32 */
        addic   r9, r0, -32 
        blt     cr0, lab5       /* if(R0 < 32) jump to lab5 */
        slw     r3, r4, r9      /* dvd.msw = dvd.lsw << R9 */
        addi    r4, r0, 0       /* dvd.lsw = 0 */
        b       lab6 

lab5: 
        slw     r3, r3, r0      /* r3 = dvd.msw << r0 */
        subfic  r9, r0, 32 
        srw     r9, r4, r9      /* r9 = dvd.lsw >> 32 - r0 */
        or      r3, r3, r9      /* dvd.msw = r3 | r9 */
        slw     r4, r4, r0      /* dvd.lsw = dvd.lsw << r0 */
lab6: 
        /* Restoring division shift and subtract loop */
        addi    r10, r0, -1     /* r10 = -1 */
        addic   r7, r7, 0       /* Clear carry bit before loop starts */
lab7: 
        /*
        ** tmp:dvd is considered one large register 
        ** each portion is shifted left 1 bit by adding it to itself 
        ** adde sums the carry from the previous and creates a new carry 
        */
        adde    r4, r4, r4      /* Shift dvd.lsw left 1 bit */
        adde    r3, r3, r3      /* Shift dvd.msw to left 1 bit */
        adde    r8, r8, r8      /* Shift tmp.lsw to left 1 bit */
        adde    r7, r7, r7      /* Shift tmp.msw to left 1 bit */
        subfc   r0, r6, r8      /* tmp.lsw - dvs.lsw */
        subfe.  r9, r5, r7      /* tmp.msw - dvs.msw */
        blt     cr0, lab8       /* if(result < 0) clear carry bit */
        or      r8, r0, r0      /* Move lsw */
        or      r7, r9, r9      /* Move msw */
        addic   r0, r10, 1      /* Set carry bit */

lab8: 
        bdnz    lab7 

        /* Write quotient and remainder */
        adde    r4, r4, r4      /* quo.lsw (lsb = CA) */
        adde    r3, r3, r3      /* quo.msw (lsb from lsw) */
        stw     r4, 4(r11)
        stw     r3, 0(r11)
        stw     r8, 4(r12)      /* rem.lsw */
        stw     r7, 0(r12)      /* rem.msw */
        blr

lab9:
        /* Qoutient is 0, divisor > dividend */
        addi    r0, r0, 0
        stw     r3, 0(r12)      /* Store remainder */
        stw     r4, 4(r12)
        stw     r0, 0(r11)      /* Set quotient to zero */
        stw     r0, 4(r11)
        blr

lab10:
        /* Divisor is 0 */
        addi    r0, r0, -1
        stw     r0, 0(r12)      /* Set remainder to zero */
        stw     r0, 4(r12)
        stw     r0, 0(r11)      /* Set quotient to zero */
        stw     r0, 4(r11)
        blr

        .epilogue __ppc32_udiv64 
