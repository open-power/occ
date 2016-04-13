/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ppc405/ppc405_context.h $                             */
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
#ifndef __PPC405_CONTEXT_H__
#define __PPC405_CONTEXT_H__

/// \file ppc405_context.h
/// \brief PPC405 Machine and Thread context for SSX

/// \page ppc405_machine_context PPC405 Assembler Macros for SSX Machine
/// Context (Critical Sections)
///
/// \section _ssx_enter_critical \b _ssx_critical_section_enter/exit
///
/// These macro encapsulates the instruction sequences required to enter and
/// exit critical sections, along with the machine context save for later
/// exiting the critical section.
///
/// \arg \c priority Either \c SSX_CRITICAL, \c SSX_NON_CRITICAL or
/// SSX_SUPERCRITICAL (for \c ssx_critical_section_enter). 
///
/// \arg \c ctxreg A register that will hold (holds) the machine context (MSR)
/// prior to entering the critical section (to be restored) for \c
/// _ssx_critical_section_enter (\c _ssx_critical_section_exit).
///
/// \arg \c scrreg A scratch register required for the computation of
/// \c _ssx_critical_section_enter. 
///
/// Forms:
/// 
/// \b _ssx_critical_section_enter \a priority, \a ctxreg, \a scrreg - Enter a
/// critical section \n
/// \b _ssx_critical_section_exit \a ctxreg - Exit a critical section

#ifdef __ASSEMBLER__

        .set    _msr_ee_bit, MSR_EE_BIT
        .set    _msr_ce_bit, MSR_CE_BIT

        .macro  _ssx_critical_section_enter priority, ctxreg, scrreg
        mfmsr   \ctxreg
        .if     ((\priority) == SSX_CRITICAL)
        _clrbit \scrreg, \ctxreg, _msr_ee_bit
        _clrbit \scrreg, \scrreg, _msr_ce_bit
        mtmsr   \scrreg
        .elseif ((\priority) == SSX_SUPERCRITICAL)
        _liwa   \scrreg, (MSR_APE | MSR_WE | MSR_CE | MSR_EE | MSR_ME | MSR_DWE | MSR_DE)
        andc    \scrreg, \ctxreg, \scrreg
        mtmsr   \scrreg
        .elseif ((\priority) == SSX_NONCRITICAL)
        wrteei  0
        .else
        .error  "'priority' was not one of SSX_CRITICAL, SSX_NONCRITICAL or SSX_SUPERCRITICAL"
        .endif
        isync
        .endm

        .macro  _ssx_critical_section_exit ctxreg
        mtmsr   \ctxreg
        isync
        .endm
        
// ****************************************************************************
// SSX context save/restore macros for 32-bit Embedded PowerPC
// ****************************************************************************

// All stack frames are 8-byte aligned in conformance with the EABI.  SSX
// never saves or restores GPR2 or GPR13.  GPR13 is constant in (E)ABI
// applications - the base of the read-write small data area.  GPR2 is
// system-reserved in ABI applications, and is the base for read-only small data
// in EABI applications.

// A fair amount of complexity is involved in handling the non-critical and
// critical interrupt levels, and the emphasis on performance of fast-mode
// interrupt handlers. Several different approaches and philosophies could
// have been implemented - this is only one.  In this implementation
// critical/non-critical interrupt levels are treated more or less the same,
// and the interrupt priority is just that - a kind of preemption priority.
// Critical interrupt handling does have a little less overhead because it
// does not have a thread scheduling step at the end.

// A full context save takes place in 3 or 4 steps.  Thread switches always do
// steps 1, 2 and 3.
// 1. The fast context that is always saved in response to every interrupt;
// 1a. The optional save/update of the kernel context for interrupts. 
// 2. The (volatile - fast) context that is saved if an interrupt handler
//    switches from fast-mode to full-mode.
// 3. The non-volatile context that is saved when a thread is switched out.

// USPRG0 holds the __SsxKernelContext structure (defined in ppc405.h) that
// represents the current kernel context.  The layout is as follows:
//
// Bits   Meaning
// ==============
// 0:7   The critical interrupt count
// 8:15  The non-critical interrupt count
// 16:23 The IRQ currently being processed
// 24    The 'thread_mode' flag
// 25:31 The thread priority of the running thread
//
// When SSX is initialized USPRG0 is initialized to 0.  When thread-mode is
// entered (by ssx_start_threads()) bit 24 is set to 1.  In order to support
// PgP/OCC firmware, once initialized (with ssx_initialize()) SSX can simply
// handle interrupts, reverting back to the non-thread-mode idle loop when
// there's nothing to do.
//      
// Note that it would require a serious error for the interrupt counts to ever
// equal or exceed 2**8 as this would imply runaway reentrancy and stack
// overflow. In fact it is most likely an error if an interrupt handler is
// ever re-entered while active.

// Registers SRR2 and SRR3 are always saved in IRQ context because
// __ssx_irq_fast2full must save the (volatile - fast) context to provide
// working registers before it can look at USPRG0 to determine critical
// vs. non-critical context.  However, when restoring a non-critical interrupt
// or thread these registers need not be restored. SRR2 and SRR3 are never
// saved or restored for thread context switches, because threads always
// operate at noncritical level.

// When MMU protection is enabled, relocation/protection is re-established
// immediately upon entry to the interrupt handler, before any memory
// operations (load/store) take place.  This requires using SPRG0 and SPGR4
// for temporary storage for noncritical/critical handlers respectively in
// accordance with the SSX conventions for SPRGn usage by fast-mode
// interrupts. 

        ## ------------------------------------------------------------
        ## Unused registers for embedded PowerPC
        ## ------------------------------------------------------------

        ## Registers GPR2 and GPR13 are never saved or restored.  In ABI and 
        ## EABI applications these registers are constant.

        .set    UNUSED_GPR2,  0x2 # Dedicated; EABI read-only small data area
        .set    UNUSED_GPR13, 0xd # Dedicated; (E)ABI read-write small data area

        ## ------------------------------------------------------------
        ## Flags for context push/pop
        ## ------------------------------------------------------------

        .set    SSX_THREAD_CONTEXT, 0
        .set    SSX_IRQ_CONTEXT, 1

        ## ------------------------------------------------------------
        ## The SSX fast context layout for Embedded PowerPC
        ## ------------------------------------------------------------

        .set    SSX_FAST_CTX_GPR1,       0x00 # Dedicated; Stack pointer
        .set    SSX_FAST_CTX_HANDLER_LR, 0x04 # Slot for handler to store LR 
        .set    SSX_FAST_CTX_GPR3,       0x08 # Volatile;  Parameter; Return Value
        .set    SSX_FAST_CTX_GPR4,       0x0c # Volatile;  Parameter
        .set    SSX_FAST_CTX_GPR5,       0x10 # Volatile;  Parameter
        .set    SSX_FAST_CTX_GPR6,       0x14 # Volatile;  Parameter
        .set    SSX_FAST_CTX_GPR7,       0x18 # Volatile; Parameter
        .set    SSX_FAST_CTX_CR,         0x1c # Condition register 
        .set    SSX_FAST_CTX_LR,         0x20 # Link register      SPRN 0x008 
        .set    SSX_FAST_CTX_KERNEL_CTX, 0x24 # Saved __SsxKernelContext for IRQ

        .set    SSX_FAST_CTX_SIZE,  0x28  # Must be 8-byte aligned

        ## ------------------------------------------------------------
        ## The SSX (volatile - fast) context layout for Embedded PowerPC
        ## ------------------------------------------------------------

        .set    SSX_VOL_FAST_CTX_GPR1,       0x00 # Dedicated; Stack pointer
        .set    SSX_VOL_FAST_CTX_HANDLER_LR, 0x04 # Slot for handler to store LR
        .set    SSX_VOL_FAST_CTX_GPR0,       0x08 # Volatile;  Language specific
        .set    SSX_VOL_FAST_CTX_GPR8,       0x0c # Volatile;  Parameter
        .set    SSX_VOL_FAST_CTX_GPR9,       0x10 # Volatile;  Parameter
        .set    SSX_VOL_FAST_CTX_GPR10,      0x14 # Volatile;  Parameter
        .set    SSX_VOL_FAST_CTX_GPR11,      0x18 # Volatile
        .set    SSX_VOL_FAST_CTX_GPR12,      0x1c # Volatile
        .set    SSX_VOL_FAST_CTX_XER,        0x20 # Fixed-point exception register  SPRN 0x001
        .set    SSX_VOL_FAST_CTX_CTR,        0x24 # Count register                  SPRN 0x009
        .set    SSX_VOL_FAST_CTX_SRR0,       0x28 # Save/restore register 0         SPRN 0x01a
        .set    SSX_VOL_FAST_CTX_SRR1,       0x2c # Save/restore register 1         SPRN 0x01b
        .set    SSX_VOL_FAST_CTX_SRR2,       0x30 # Save/restore register 2         SPRN 0x3de
        .set    SSX_VOL_FAST_CTX_SRR3,       0x34 # Save/restore register 3         SPRN 0x3df

        .set    SSX_VOL_FAST_CTX_SIZE,  0x38  # Must be 8-byte aligned

        ## ------------------------------------------------------------
        ## The SSX non-volatile context layout for Embedded PowerPC
        ## ------------------------------------------------------------

        ## The 'preferred form' for stmw is for the LSB of R31 to fall into the
        ## end of a 16-byte aligned block.

        .set    SSX_NON_VOL_CTX_GPR1,       0x0   # Dedicated; Stack Pointer
        .set    SSX_NON_VOL_CTX_HANDLER_LR, 0x4   # Slot for handler to store LR
        .set    SSX_NON_VOL_CTX_GPR14,      0x8   # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR15,      0xc   # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR16,      0x10  # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR17,      0x14  # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR18,      0x18  # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR19,      0x1c  # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR20,      0x20  # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR21,      0x24  # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR22,      0x28  # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR23,      0x2c  # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR24,      0x30  # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR25,      0x34  # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR26,      0x38  # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR27,      0x3c  # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR28,      0x40  # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR29,      0x44  # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR30,      0x48  # Non-volatile
        .set    SSX_NON_VOL_CTX_GPR31,      0x4c  # Non-volatile

        .set    SSX_NON_VOL_CTX_SIZE,  0x50 # Must be 8-byte aligned

        ## ------------------------------------------------------------
        ## Save/restore the fast context
        ## 
        ## 11 Instructions, 8 Loads/Stores : If MMU is disabled
        ## 17 Instructions, 8 Loads/Stores : If MMU is enabled
        ## ------------------------------------------------------------
        ##
        ## Without MMU support, an EIEIO is always executed at the entry point
        ## to gauarantee that all memory operations (especially MMIO
        ## operations) have completed prior to execution of the interrupt
        ## handler.
        ##
        ## If MMU support is enabled, address translation is re-established
        ## immediately at the entry of each interrupt, prior to performing any
        ## loads or stores. SSX currently only supports using the MMU for
        ## protection, not for address translation.  Therfore it is 'legal'
        ## to change translation modes a with an MTMSR followed by an
        ## ISYNC. This is much simpler then the complex instruction sequence
        ## that would be required if we had to set up RFI/RFCI sequences to
        ## change the execution context at this point.
        ##
        ## Note that since we are not really doing address translation, it
        ## would also be in keeping with the 'fast interrupt' idea to defer
        ## reenabling translation (protection) until the fast-to-full sequence
        ## was executed for full-mode interrupts, and run fast-mode interrupts
        ## unprotected. However here we chose to run all interrupts with MMU
        ## protection. 
        ##
        ## Unfortunately the simple MTMSR;ISYNC sequence exposes a serious bug
        ## in the 405-S core that causes the stack-pointer store instruction
        ## to generate a seemingly random, *real-mode* address in certain cases
        ## when this instruction in a noncritical interrupt prologue is
        ## interrupted by a critical interrupt. This bug is described in
        ## HW239446. The workaround is to follow the ISYNC sith a SYNC - which
        ## eliminates the problem for reasons still unknown. On the bright side
        ## this SYNC might also serve the same purpose as the EIEIO in the
        ## non-MMU case, guaranteeing that all MMIO has completed prior to the
        ## interrupt handler. However without the initial EIEIO we still
        ## experience failures, so this seemingly redundant instruction also
        ## remains in place. This requirement is assumed to be related to the
        ## HW239446 issue.       

        .macro  _ssx_fast_ctx_push, critical

        .if     !PPC405_MMU_SUPPORT

                eieio

        .elseif \critical

                eieio           # HW239446?
                mtsprg4 %r3
                mfmsr   %r3
                ori     %r3, %r3, PPC405_RELOCATION_MODE
                mtmsr   %r3
                isync
#ifndef ALLOW_HW239446
                sync            # HW239446!
#endif
                mfsprg4 %r3

        .else

                eieio           # HW239446?
                mtsprg0 %r3
                mfmsr   %r3
                ori     %r3, %r3, PPC405_RELOCATION_MODE
                mtmsr   %r3
                isync
#ifndef ALLOW_HW239446
                sync            # HW239446!
#endif
                mfsprg0 %r3
        
        .endif                             

        stwu    %r1, -SSX_FAST_CTX_SIZE(%r1) # May be corrupted w/o HW239446

        stw     %r3,  SSX_FAST_CTX_GPR3(%r1)
        stw     %r4,  SSX_FAST_CTX_GPR4(%r1)
        stw     %r5,  SSX_FAST_CTX_GPR5(%r1)
        stw     %r6,  SSX_FAST_CTX_GPR6(%r1)
        stw     %r7,  SSX_FAST_CTX_GPR7(%r1)

        mfcr    %r3
        mflr    %r4

        stw     %r3,  SSX_FAST_CTX_CR(%r1)
        stw     %r4,  SSX_FAST_CTX_LR(%r1)

        .endm


        .macro  _ssx_fast_ctx_pop
                
        lwz     %r3, SSX_FAST_CTX_CR(%r1)
        lwz     %r4, SSX_FAST_CTX_LR(%r1)

        mtcr    %r3
        mtlr    %r4

        lwz     %r3,  SSX_FAST_CTX_GPR3(%r1)
        lwz     %r4,  SSX_FAST_CTX_GPR4(%r1)
        lwz     %r5,  SSX_FAST_CTX_GPR5(%r1)
        lwz     %r6,  SSX_FAST_CTX_GPR6(%r1)
        lwz     %r7,  SSX_FAST_CTX_GPR7(%r1)

        lwz     %r1, 0(%r1)

        .endm

        ## ------------------------------------------------------------
        ## Save/update the kernel context in response to an interrupt.  This is
        ## not part of the fast context save because for external interupts the
        ## IRQ is not determined until later.
        ## ------------------------------------------------------------

        ## The kernel context is saved, then updated with the currently active
        ## IRQ in bits 16:23.  The correct interrupt count is incremented and
        ## the context is returned to USPRG0.

        .macro  _save_update_kernel_context critical, irqreg, ctxreg

        .if     \critical
        SSX_TRACE_CRITICAL_IRQ_ENTRY \irqreg, \ctxreg
        .else
        SSX_TRACE_NONCRITICAL_IRQ_ENTRY \irqreg, \ctxreg
        .endif          

        mfusprg0 \ctxreg
        stw     \ctxreg, SSX_FAST_CTX_KERNEL_CTX(%r1)
        rlwimi  \ctxreg, \irqreg, 8, 16, 23
        .if     \critical
        addis   \ctxreg, \ctxreg, 0x0100
        .else
        addis   \ctxreg, \ctxreg, 0x0001
        .endif
        mtusprg0 \ctxreg

        .endm

        ## ------------------------------------------------------------
        ## Fast-mode context pop and RF(C)I.  This is only used by
        ## interrupt handlers - the thread context switch has its own
        ## code to handle updating USPRG0 for thread mode.
        ## ------------------------------------------------------------

        .macro  _ssx_fast_ctx_pop_exit critical

        .if     SSX_KERNEL_TRACE_ENABLE
        .if     \critical
        bl      __ssx_trace_critical_irq_exit
        .else
        bl      __ssx_trace_noncritical_irq_exit
        .endif
        .endif

        lwz     %r3, SSX_FAST_CTX_KERNEL_CTX(%r1)
        mtusprg0 %r3
        _ssx_fast_ctx_pop
        .if     \critical
        rfci
        .else
        rfi
        .endif

        .endm

        ## ------------------------------------------------------------
        ## Save/restore the (volatile - fast) context
        ## 
        ## Thread - 15     Instructions, 11 Loads/Stores
        ## IRQ    - 19(15) Instructions, 13(11) Loads/Stores
        ## ------------------------------------------------------------

        .macro  _ssx_vol_fast_ctx_push, irq_context, critical=1

        stwu    %r1, -SSX_VOL_FAST_CTX_SIZE(%r1)

        stw     %r0,  SSX_VOL_FAST_CTX_GPR0(%r1)
        stw     %r8,  SSX_VOL_FAST_CTX_GPR8(%r1)
        stw     %r9,  SSX_VOL_FAST_CTX_GPR9(%r1)
        stw     %r10, SSX_VOL_FAST_CTX_GPR10(%r1)
        stw     %r11, SSX_VOL_FAST_CTX_GPR11(%r1)
        stw     %r12, SSX_VOL_FAST_CTX_GPR12(%r1)

        mfxer   %r8
        mfctr   %r9
        mfsrr0  %r10
        mfsrr1  %r11

        stw     %r8,  SSX_VOL_FAST_CTX_XER(%r1)
        stw     %r9,  SSX_VOL_FAST_CTX_CTR(%r1)
        stw     %r10, SSX_VOL_FAST_CTX_SRR0(%r1)
        stw     %r11, SSX_VOL_FAST_CTX_SRR1(%r1)

        .if     (\irq_context & \critical)
        mfsrr2  %r8
        mfsrr3  %r9

        stw     %r8, SSX_VOL_FAST_CTX_SRR2(%r1)
        stw     %r9, SSX_VOL_FAST_CTX_SRR3(%r1)
        .endif

        .endm


        .macro  _ssx_vol_fast_ctx_pop, irq_context, critical

        .if     (\irq_context & \critical)
        lwz     %r8, SSX_VOL_FAST_CTX_SRR2(%r1)
        lwz     %r9, SSX_VOL_FAST_CTX_SRR3(%r1)

        mtsrr2  %r8
        mtsrr3  %r9
        .endif  

        lwz     %r8,  SSX_VOL_FAST_CTX_XER(%r1)
        lwz     %r9,  SSX_VOL_FAST_CTX_CTR(%r1)
        lwz     %r10, SSX_VOL_FAST_CTX_SRR0(%r1)
        lwz     %r11, SSX_VOL_FAST_CTX_SRR1(%r1)

        mtxer   %r8
        mtctr   %r9
        mtsrr0  %r10
        mtsrr1  %r11

        lwz     %r0,  SSX_VOL_FAST_CTX_GPR0(%r1)
        lwz     %r8,  SSX_VOL_FAST_CTX_GPR8(%r1)
        lwz     %r9,  SSX_VOL_FAST_CTX_GPR9(%r1)
        lwz     %r10, SSX_VOL_FAST_CTX_GPR10(%r1)
        lwz     %r11, SSX_VOL_FAST_CTX_GPR11(%r1)
        lwz     %r12, SSX_VOL_FAST_CTX_GPR12(%r1)

        lwz     %r1, 0(%r1)

        .endm

        ## ------------------------------------------------------------
        ## Save/restore the non-volatile context on the stack
        ##
        ## 2 Instructions, 19 Loads/Stores
        ## ------------------------------------------------------------

        .macro  _ssx_non_vol_ctx_push

        stwu    %r1, -SSX_NON_VOL_CTX_SIZE(%r1)
        stmw    %r14, SSX_NON_VOL_CTX_GPR14(%r1)
        
        .endm


        .macro  _ssx_non_vol_ctx_pop

        lmw     %r14, SSX_NON_VOL_CTX_GPR14(%r1)
        lwz     %r1, 0(%r1)

        .endm

#else /* __ASSEMBLER__ */

/// SSX thread context layout as a C structure.
///
/// This is the structure of the stack area pointed to by
/// thread->saved_stack_pointer when a thread is fully context-switched out.

typedef struct {

    uint32_t r1_nv;
    uint32_t link_nv;
    uint32_t r14;
    uint32_t r15;
    uint32_t r16;
    uint32_t r17;
    uint32_t r18;
    uint32_t r19;
    uint32_t r20;
    uint32_t r21;
    uint32_t r22;
    uint32_t r23;
    uint32_t r24;
    uint32_t r25;
    uint32_t r26;
    uint32_t r27;
    uint32_t r28;
    uint32_t r29;
    uint32_t r30;
    uint32_t r31;
    uint32_t r1_vf;
    uint32_t link_vf;
    uint32_t r0;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;
    uint32_t xer;
    uint32_t ctr;
    uint32_t srr0;
    uint32_t srr1;
    uint32_t srr2;
    uint32_t srr3;
    uint32_t r1;
    uint32_t link_fast;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t cr;
    uint32_t lr;
    uint32_t usprg0;

} SsxThreadContext;


/// SSX thread context of an interrupted thread (full-mode handler)
///
/// When a thread is interrupted by a full-mode interrupt handler, this is the
/// layout of the stack area pointed to by either __ssx_saved_sp_noncritical
/// or __ssx_saved_sp_critical.

typedef struct {

    uint32_t r1_vf;
    uint32_t link_vf;
    uint32_t r0;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;
    uint32_t xer;
    uint32_t ctr;
    uint32_t srr0;
    uint32_t srr1;
    uint32_t srr2;
    uint32_t srr3;
    uint32_t r1;
    uint32_t link_fast;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t cr;
    uint32_t lr;
    uint32_t usprg0;

} SsxThreadContextFullIrq;


/// SSX thread context of an interrupted thread (fast-mode handler)
///
/// When a thread is interrupted by a fast-mode interrupt handler, this is the
/// layout of the stack area pointed to by R1 - unless the fast-mode interrupt
/// handler extends the stack.

typedef struct {

    uint32_t r1;
    uint32_t link_fast;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t cr;
    uint32_t lr;
    uint32_t usprg0;

} SsxThreadContextFastIrq;

#endif /* __ASSEMBLER__ */

#endif /* __PPC405_CONTEXT_H__ */


