/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/baselib/ppe42_msr.h $                                 */
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
#ifndef __PPE42_MSR_H__
#define __PPE42_MSR_H__

/// \file ppe42_msr.h
/// \brief Everything related to the PPE42 Machine State Register
///
/// All of the macros defined here that \e modify the MSR create a compiler
/// memory barrier that will cause GCC to flush/invalidate all memory data
/// held in registers before the macro. This is consistent with other systems,
/// e.g., the PowerPC Linux kernel, and is the safest way to define these
/// macros as it guarantess for example that kernel data structure updates
/// have completed before exiting a critical section.

#define MSR_SEM     0x7f000000      /* SIB Error Mask */
#define MSR_IS0     0x00800000      /* Instance-Specific Field 0 */
#define MSR_SIBRC   0x00700000      /* Last SIB return code */
#define MSR_LP      0x00080000      /* Low Priority */
#define MSR_WE      0x00040000      /* Wait State Enable */
#define MSR_IS1     0x00020000      /* Instance-Specific Field 1 */
#define MSR_UIE     0x00010000      /* Unmaskable Interrupt Enable */
#define MSR_EE      0x00008000      /* External Interrupt Enable */
#define MSR_ME      0x00001000      /* Machine Check Exception Enable */
#define MSR_IS2     0x00000800      /* Instance-Specific field 2 */
#define MSR_IS3     0x00000400      /* Instance-Specific field 3 */
#define MSR_IPE     0x00000100      /* Imprecise Mode Enable */
#define MSR_SIBRCA  0x000000ff      /* SIB Return Code Accumulator */

//#define MSR_CE_BIT 14
#define MSR_EE_BIT 16
//#define MSR_IR_BIT 26
//#define MSR_DR_BIT 27


#define MSR_SEM_START_BIT       1
#define MSR_SEM_LEN             7
#define MSR_SEM1                0x40000000
#define MSR_SEM2                0x20000000
#define MSR_SEM3                0x10000000
#define MSR_SEM4                0x08000000
#define MSR_SEM5                0x04000000
#define MSR_SEM6                0x02000000
#define MSR_SEM7                0x01000000

#define MSR_SIBRC_START_BIT     9
#define MSR_SIBRC_LEN           3


#ifndef __ASSEMBLER__

/// Move From MSR

#define mfmsr()                               \
    ({uint32_t __msr;                          \
        asm volatile ("mfmsr %0" : "=r" (__msr) : : "memory"); \
        __msr;})


/// Move to MSR

#define mtmsr(value) \
    asm volatile ("mtmsr %0" : : "r" (value) : "memory")


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
    asm volatile ("wrteei %0" : : "i" (i) : "memory")


/// Write MSR[EE] from the EE bit of another MSR

#define wrtee(other_msr) \
    asm volatile ("wrtee %0" : : "r" (other_msr) : "memory")

extern void __set_msr(unsigned int i_msr_value);

#define ppe_idle() __set_msr(mfmsr() | MSR_WE | MSR_EE);

#endif /* __ASSEMBLER__ */

#endif /* __PPE42_MSR_H__ */
