/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ppc405/ppc405_msr.h $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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
#ifndef __PPC405_MSR_H__
#define __PPC405_MSR_H__

//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppc405_msr.h
/// \brief Everything related to the PPC405 Machine State Register
///
/// All of the macros defined here that \e modify the MSR create a compiler
/// memory barrier that will cause GCC to flush/invalidate all memory data
/// held in registers before the macro. This is consistent with other systems,
/// e.g., the PowerPC Linux kernel, and is the safest way to define these
/// macros as it guarantess for example that kernel data structure updates
/// have completed before exiting a critical section.

#define MSR_AP  0x02000000      /* APU Available */
#define MSR_APE 0x00080000      /* APU Exception Enable */
#define MSR_WE  0x00040000      /* Wait State Enable */
#define MSR_CE  0x00020000      /* Critical Interrupt Enable */
#define MSR_EE  0x00008000      /* External Interrupt Enable */
#define MSR_PR  0x00004000      /* Problem State */
#define MSR_ME  0x00001000      /* Machine Check Exception Enable */
#define MSR_FE0 0x00000800      /* Floating-Point Exception Mode 0 */
#define MSR_DWE 0x00000400      /* Debug Wait Enable */
#define MSR_DE  0x00000200      /* Debug Interrupt Enable */
#define MSR_IR  0x00000020      /* Instruction Relocation */
#define MSR_DR  0x00000010      /* Data Relocation */

#define MSR_CE_BIT 14
#define MSR_EE_BIT 16
#define MSR_IR_BIT 26
#define MSR_DR_BIT 27

#ifndef __ASSEMBLER__

/// Move From MSR

#define mfmsr()                               \
    ({uint32_t __msr;                          \
        asm volatile ("mfmsr %0" : "=r" (__msr)); \
        __msr;})


/// Move to MSR

#define mtmsr(value) \
    asm volatile ("mtmsr %0; isync" : : "r" (value) : "memory")


/// Read-Modify-Write the MSR with OR (Set MSR bits).  This operation is only
/// guaranteed atomic in a critical section.

#define or_msr(x) \
    mtmsr(mfmsr() | (x))


/// Read-Modify-Write the MSR with AND complement (Clear MSR bits). This
/// operation is only guaranteed atomic in a critical section.

#define andc_msr(x) \
    mtmsr(mfmsr() & ~(x))


/// Write MSR[EE] with an immediate value (0/1)
///
/// Note that the immediate value \a i must be a compile-time constant.

#define wrteei(i) \
    asm volatile ("wrteei %0; isync" : : "i" (i) : "memory")


/// Write MSR[EE] from the EE bit of another MSR

#define wrtee(other_msr) \
    asm volatile ("wrtee %0; isync" : : "r" (other_msr) : "memory")

#endif /* __ASSEMBLER__ */

#endif /* __PPC405_MSR_H__ */
