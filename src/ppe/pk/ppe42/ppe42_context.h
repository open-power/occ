/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/ppe42_context.h $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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
#ifndef __PPE42_CONTEXT_H__
#define __PPE42_CONTEXT_H__

/// \file ppe42_context.h
/// \brief PPE42 Machine and Thread context for PK

/// \page ppe42_machine_context PPE42 Assembler Macros for PK Machine
/// Context (Critical Sections)
///
/// \section _pk_enter_critical \b _pk_critical_section_enter/exit
///
/// These macro encapsulates the instruction sequences required to enter and
/// exit critical sections, along with the machine context save for later
/// exiting the critical section.
///
/// \arg \c ctxreg A register that will hold (holds) the machine context (MSR)
/// prior to entering the critical section (to be restored) for \c
/// _pk_critical_section_enter (\c _pk_critical_section_exit).
///
/// \arg \c scrreg A scratch register required for the computation of
/// \c _pk_critical_section_enter.
///
/// Forms:
///
/// \b _pk_critical_section_enter \a priority, \a ctxreg, \a scrreg - Enter a
/// critical section \n
/// \b _pk_critical_section_exit \a ctxreg - Exit a critical section

#ifdef __ASSEMBLER__
// *INDENT-OFF*

        .set    _msr_ee_bit, MSR_EE_BIT

        .macro  _pk_critical_section_enter ctxreg, scrreg
        mfmsr   \ctxreg
        wrteei  0
        .endm

        .macro  _pk_critical_section_exit ctxreg
        mtmsr   \ctxreg
        .endm

// ****************************************************************************
// PK context save/restore macros for 32-bit Embedded PowerPC
// ****************************************************************************

// All stack frames are 8-byte aligned in conformance with the EABI.  PK
// never saves or restores GPR2 or GPR13.  GPR13 is constant in (E)ABI
// applications - the base of the read-write small data area.  GPR2 is
// system-reserved in ABI applications, and is the base for read-only small data
// in EABI applications.

// USPRG0 holds the __PkKernelContext structure (defined in ppe42.h) that
// represents the current kernel context.  The layout is as follows:
//
// Bits   Meaning
// ==============
// 0     The 'thread_mode' flag
// 1     The 'processing_interrupt" flag
// 2:7   The thread priority of the running thread
// 8     The 'discard_ctx' flag
// 9:15  The IRQ currently being processed
// 16:31 The application specific data
//
// When PK is initialized USPRG0 is initialized to 0.  When thread-mode is
// entered (by pk_start_threads()) bit 0 is set to 1.  If desired,
// once initialized (with pk_initialize()) PK can simply
// handle interrupts, reverting back to the non-thread-mode idle loop when
// there's nothing to do.
//      

        ## ------------------------------------------------------------
        ## Unused registers for embedded PPE42`
        ## ------------------------------------------------------------

        ## Registers GPR2 and GPR13 are never saved or restored.  In ABI and 
        ## EABI applications these registers are constant.

        .set    UNUSED_GPR2,  0x2 # Dedicated; EABI read-only small data area
        .set    UNUSED_GPR13, 0xd # Dedicated; (E)ABI read-write small data area

        ## ------------------------------------------------------------
        ## The PK context layout for Embedded PPE42
        ## ------------------------------------------------------------

        .set    PK_CTX_GPR1,        0x00 # Dedicated; Stack pointer
        .set    PK_CTX_LINKAGE,     0x04 # Slot for handler to store LR 
        .set    PK_CTX_GPR3,        0x08 # Volatile;  Parameter; Return Value
        .set    PK_CTX_GPR4,        0x0c # Volatile;  Parameter
        .set    PK_CTX_GPR5,        0x10 # Volatile;  Parameter
        .set    PK_CTX_GPR6,        0x14 # Volatile;  Parameter
        .set    PK_CTX_CR,          0x18 # Condition register 
        .set    PK_CTX_LR,          0x1c # Link register

        .set    PK_CTX_GPR7,        0x20 # Volatile;  Parameter
        .set    PK_CTX_GPR8,        0x24 # Volatile;  Parameter
        .set    PK_CTX_GPR9,        0x28 # Volatile;  Parameter
        .set    PK_CTX_GPR10,       0x2c # Volatile;  Parameter
        .set    PK_CTX_GPR28,       0x30 # Non-volatile
        .set    PK_CTX_GPR29,       0x34 # Non-volatile
        .set    PK_CTX_GPR30,       0x38 # Non-volatile
        .set    PK_CTX_GPR31,       0x3c # Non-volatile

        .set    PK_CTX_XER,         0x40 # Fixed-point exception register
        .set    PK_CTX_CTR,         0x44 # Count register
        .set    PK_CTX_SRR0,        0x48 # Save/restore register 0
        .set    PK_CTX_SRR1,        0x4c # Save/restore register 1
        .set    PK_CTX_GPR0,        0x50 # Volatile;  Language specific
        .set    PK_CTX_KERNEL_CTX,  0x54 # Saved __PkKernelContext for IRQ

#ifdef HWMACRO_GPE
        .set    PK_CTX_PBASLVCTLV,  0x58 # PBA slave controller value(8 bytes)
        .set    PK_CTX_SIZE,        0x60 # Must be 8-byte aligned
#else
        .set    PK_CTX_SIZE,        0x58
#endif

        ## ------------------------------------------------------------
        ## Push the interrupted context if necessary
        ##
        ## This macro saves off some context in preparation for calling
        ## the pk_ctx_check_discard routine.  This is an attempt to use
        ## the 32 byte cache more efficiently.
        ## 
        ## 8 Instructions
        ## ------------------------------------------------------------
        ##

        .macro _pk_ctx_push_as_needed branch_addr:req

        stwu    %r1,    -PK_CTX_SIZE(%r1)
        stvd    %d3,    PK_CTX_GPR3(%r1)
        mfcr    %r3
        mflr    %r4
        stvd    %d3,    PK_CTX_CR(%r1)
        _liw    %r3,    \branch_addr
        b       ctx_check_discard
        .endm


        ## ------------------------------------------------------------
        ## update the kernel context in response to an interrupt.
        ## ------------------------------------------------------------

        ## The kernel context is updated with the currently active
        ## IRQ in bits 9:15.  

        .macro  _update_kernel_context irqreg, ctxreg
        rlwimi  \ctxreg, \irqreg, 16, 9, 15 //set the irq #
        oris    \ctxreg, \ctxreg, 0x4000 //set the 'processing_interrupt' flag
        mtsprg0 \ctxreg

#if PK_KERNEL_TRACE_ENABLE
        mr      %r31, \irqreg
        srwi    \ctxreg, \ctxreg, 16
        PK_KERN_TRACE_ASM16("INTERRUPT_CONTEXT(0x%04x)", \ctxreg)
        mr      \irqreg, %r31
#endif        

        .endm
// *INDENT-ON*

#else /* __ASSEMBLER__ */

/// PK thread context layout as a C structure.
///
/// This is the structure of the stack area pointed to by
/// thread->saved_stack_pointer when a thread is fully context-switched out.

typedef struct
{
    uint32_t r1;
    uint32_t linkage;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t cr;
    uint32_t lr;

    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r28;
    uint32_t r29;
    uint32_t r30;
    uint32_t r31;

    uint32_t xer;
    uint32_t ctr;
    uint32_t srr0;
    uint32_t srr1;
    uint32_t r0;
    uint32_t sprg0;
#ifdef HWMACRO_GPE
    uint64_t pbaslvctlv;
#endif
} PkThreadContext;


#endif /* __ASSEMBLER__ */

#endif /* __PPE42_CONTEXT_H__ */


