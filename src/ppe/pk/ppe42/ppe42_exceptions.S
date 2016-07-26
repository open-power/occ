/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/ppe42_exceptions.S $                         */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------
        
/// \file ppe42_exceptions.S
/// \brief PPE42 exception vector area.
///
/// \cond

        .nolist
#include "pk.h"
        .list

## declare and initializes global variables that hold external irq config data
## Each PPE macro type (GPE, CME, and SBE) will have it's own implementation of this macro
## defined in (gpe, cme, sbe)_common.h
        .hwmacro_irq_cfg_bitmaps

### ****************************************************************************
### .vectors - This section contains all ppe42 exception vectors
### 
### ****************************************************************************

        .section .vectors, "ax", @progbits

        .global __vectors

__vectors:

        ############################################################
        # 0x0000 : Machine Check
        ############################################################

        ### Unmaskable interrupts (including program interrupts) are promoted
        ### to machine check interrupts if MSR[UIE] = 0 and MSR[ME] = 1.
        ### If the machine check was caused by a program interrupt it
        ### will be forwarded to the program exception handler.
__machine_check:

        PPE42_MACHINE_CHECK_HANDLER

        ############################################################
        # 0x0040 : System Reset
        ############################################################
        .global __system_reset
        .org __vectors + 0x0040
__system_reset:
        b   __pk_boot

        ############################################################
        # 0x0060 : Data Storage Interrupt
        ############################################################

        .org __vectors + 0x0060
__data_storage:

        PPE42_DATA_STORAGE_HANDLER

        ############################################################
        # 0x0080 : Instruction Storage Interrupt
        ############################################################

        .org __vectors + 0x0080
__instruction_storage:

        PPE42_INSTRUCTION_STORAGE_HANDLER


        ############################################################
        # 0x00A0 : External Interrupt
        ############################################################

        .org __vectors + 0x00A0
__external_interrupt_vector:
        _pk_ctx_push_as_needed __get_ext_irq

        ############################################################
        # 0x00C0 : Alignment Exception
        ############################################################

        .org __vectors + 0x00C0
__alignment_exception:

        PPE42_ALIGNMENT_HANDLER


        ############################################################
        # 0x00E0 : Program Interrupt
        ############################################################

        .org __vectors + 0x00E0

        ### Program exceptions are utilized for emulating the system call
        ### instruction (0x44000002) which is used for doing context
        ### switches between threads.  They can also be used by the code
        ### to signal an exception in an error scenario.
__program_exception:
        _pk_ctx_push_as_needed  program_exception_handler 


        ############################################################
        # 0x0100 : DEC Interrupts
        ############################################################

        .org __vectors + 0x0100
__dec_interrupt:
        _pk_ctx_push_as_needed  dec_handler

        ############################################################
        # 0x0120 : FIT Interrupts
        ############################################################
        .org __vectors + 0x0120
__fit_interrupt:        

        _pk_ctx_push_as_needed  fit_handler

        ############################################################
        # 0x0140 : Watchdog Interrupts
        ############################################################
        .org __vectors + 0x0140
__watchdog_interrupt:   

        _pk_ctx_push_as_needed  watchdog_handler


### ****************************************************************************
### The rest of the code in this file doesn't have to be placed anywhere
### special, so just place it in the .text section.
### ****************************************************************************

        .section .text, "ax", @progbits


        ## The idle thread has no permanent register context.  The idle thread
        ## entry point is re-entered whenever the idle thread is scheduled.

        .global __pk_idle_thread
        .global __pk_idle_thread_from_bootloader

__pk_idle_thread:

        ## The idle thread 'uses' the kernel stack.  Any register context
        ## pushed here is redundant and is wiped out/ignored every time the
        ## idle thread is re-scheduled. 
        
        ## The idle thread simply establishes a default machine context and
        ## enters the wait-enable state.  The idle thread is always entered
        ## with interrupts disabled.  
        ##
        ## The kernel context is initialized to indicate that the idle thread
        ## is running - the idle thread priority is PK_THREADS, the
        ## 'thread-mode' bit is asserted and so is the 'discard-ctx" bit.
        ## In addition, the previous kernel context is stored in the lower
        ## 16 bits.
        ##
        ## This loop can also be called from the PK bootloader if main()
        ## returns - in which case we don't muck with the SPRG0 or the stack
        ## pointer. 
        mfsprg0 %r3         
        srwi    %r3, %r3, 16
        oris    %r3, %r3, (PK_THREADS << 8) | PPE42_THREAD_MODE | PPE42_DISCARD_CTX
        mtsprg0        %r3
        _lwzsd  %r1, __pk_kernel_stack
                
__pk_idle_thread_from_bootloader:

        PK_KERN_TRACE_ASM16("ENTER_IDLE_STATE")

        _lwzsd  %r3, __pk_thread_machine_context_default
        _oriwa  %r3, %r3, MSR_WE
        mtmsr   %r3
        b       .

        ## pk_halt() is implemented on the ppe42 by writing a value of 0x3 to
        ## the RST field of the DBCR.
        .global pk_halt
pk_halt:
        lis     %r31, 0x3000
        mtdbcr  %r31
        .long   0


dec_handler:            

        ## The portable timer handler of PK is a full-mode handler with the prototype:
        ## void (*pk_timer_handler)(void).
        ##
        ## To support the portable specification, the kernel clears the
        ## interrupt by writing the DIS back into the TSR before calling the
        ## handler.  The timer handler does not take any arguments.

        li      %r4, PPE42_IRQ_DEC
        _update_kernel_context %r4, %r3
        
        _liwa   %r3, TSR_DIS
        mttsr   %r3

        bl      __pk_timer_handler
        b       check_for_ext_interrupt

program_exception_handler:
        _pk_panic   PPE42_ILLEGAL_INSTRUCTION

        .global __pk_next_thread_resume
__pk_next_thread_resume:
        
        _lwzsd  %r3, __pk_next_thread
        _stwsd  %r3, __pk_current_thread

        ## Enter the wait enabled state if the thread pointer is null
        bwz     %r3, __pk_idle_thread
        
        ## switch to the new thread stack 
        lwz     %r1, PK_THREAD_OFFSET_SAVED_STACK_POINTER(%r3)

        ## load sprg0 from the stack and update the thread priority
        ## in case it changed.
restore_and_update_sprg0:
        _lbzsd  %r31, __pk_next_priority

        PK_KERN_TRACE_ASM16("RESUME_THREAD(%d)", %r31)

        lwz     %r3, PK_CTX_KERNEL_CTX(%r1)
        rlwimi  %r3, %r31, 24, 2, 7
        mtsprg0 %r3

        b       ctx_pop
 
fit_handler:
        
        ## The FIT handler is user defined. By
        ## convention the kernel clears the interrupt by writing the FIS back
        ## into the TSR.
        
        li      %r4, PPE42_IRQ_FIT
                
        _update_kernel_context %r4, %r3

        _lwzsd  %r3, __ppe42_fit_arg

        _liwa   %r6, TSR_FIS
        mttsr   %r6

        _lwzsd  %r6, __ppe42_fit_routine
        mtlr    %r6
        blrl
        
        b       check_for_ext_interrupt

watchdog_handler:
        ## Watchdog setup is described in the PK Specification. 
        ## The kernel clears TSR[WIS] prior to calling the handler.  

        li      %r4, PPE42_IRQ_WATCHDOG
        
        _update_kernel_context %r4, %r3

        _liwa   %r6, TSR_WIS
        mttsr   %r6

        _lwzsd  %r6, __ppe42_watchdog_routine
        mtlr    %r6
        blrl

        b       check_for_ext_interrupt


        ## Check if we can disard the interrupted context.
        ## This routine expects r3, r4, lr, and cr to already be pushed.
        ## It also expects r3 to hold the address of the function to jump
        ## to after the interrupted context has been pushed (if necessary).

        .align 5
ctx_check_discard:

        ## Prepare to jump to the branch function that was passed in
        mtlr    %r3 

        ## Check if the DISCARD_CTX bit is set in the kernel context
        mfsprg0 %r3
        bb0wi   %r3, PPE42_DISCARD_CTX_BIT, ctx_continue_push

ctx_discard:        
        ## DISCARD_CTX bit was set.  Discard stack and branch to interrupt
        ## handler code
        addi    %r1, %r1, PK_CTX_SIZE
        blr

        ## DISCARD_CTX bit was not set.  Continue saving full context.
        ## (r3, r4, lr, and cr have already been saved for us) and
        ## r3 contains the interrupted kernel context

        .global __ctx_switch
__ctx_switch:
        stwu    %r1,    -PK_CTX_SIZE(%r1)
        stvd    %d3,    PK_CTX_GPR3(%r1)
        mfcr    %r3
        mflr    %r4
        stvd    %d3,    PK_CTX_CR(%r1)
        _liw    %r3     __pk_next_thread_resume
        mtlr    %r3
        ## emulate what interrupt would do
        mtsrr0  %r4
        mfmsr   %r3
        mtsrr1  %r3

        ## ctx_continue_push expects r3 to be value of sprg0
        mfsprg0 %r3
        
ctx_continue_push:

        stvd    %d5,    PK_CTX_GPR5(%r1)
        stvd    %d7,    PK_CTX_GPR7(%r1)
        stvd    %d9,    PK_CTX_GPR9(%r1)
        stvd    %d28,   PK_CTX_GPR28(%r1)
        stvd    %d30,   PK_CTX_GPR30(%r1)
        mfxer   %r5
        mfctr   %r6
        stvd    %d5,    PK_CTX_XER(%r1)
        mfsrr0  %r7
        mfsrr1  %r8
        stvd    %d7,    PK_CTX_SRR0(%r1)
        stw     %r0,    PK_CTX_GPR0(%r1)
        stw     %r3,    PK_CTX_KERNEL_CTX(%r1)
        
        ## If the 'processing interrupt' bit is set then we were already
        ## using the kernel stack and don't need to modify or save the current
        ## stack pointer.
        bb1wi   %r3, PPE42_PROC_IRQ_BIT, ctx_push_completed

        ## load the pointer to the current thread control block
        _lwzsd  %r4, __pk_current_thread
        
        ## don't save the stack pointer in the thread control block
        ## if the current thread was the idle thread (null pointer)
        bwz     %r4, switch_to_kernel_stack

        ## we interrupted a bonafide thread, so save off the stack
        ## pointer
        stw     %r1, PK_THREAD_OFFSET_SAVED_STACK_POINTER(%r4)

switch_to_kernel_stack:
        _stwsd  %r1, __pk_saved_sp
        _lwzsd  %r1, __pk_kernel_stack
       
ctx_push_completed:      
        blr

__get_ext_irq:   
        
        ## Entry invariants:
        ## 1. external interupts are disabled;
        ## 2. previous context has ben saved off
        ## 3. r3 contains the kernel context
        ## 4. r1 points to the kernel stack

        ## This is HW Macro specific code that is responsible for finding the
        ## IRQ # and storing it in r4 (phantom IRQ's are assigned a value of EXTERNAL_IRQS).
        
        hwmacro_get_ext_irq        

        ## An active or phantom IRQ was found. 
        ## R3 has the context of the interrupted thread or bottom half
        ## R4 has the IRQ number.
        ## The IRQ is converted into a pointer to an 8-byte handler
        ## structure, and the handler is dispatched. The call is made with the
        ## parameters:

        ## R3 = private data ptr
        ## R4 = irq

call_external_irq_handler:  

        _update_kernel_context %r4, %r3
        slwi    %r3, %r4, 3                 //multiply the irq# by 8
        _liw    %r6, __ppe42_irq_handlers
        lwzx    %r5, %r6, %r3
        addi    %r3, %r3, 4
        lwzx    %r3, %r6, %r3
        mtlr    %r5
        blrl
                
        ## Once the interrupt handler returns, check if any interrupts are
        ## waiting and handle them now.

check_for_ext_interrupt:

        ## Set the CTX_DISCARD bit in the kernel context so that if there is
        ## an interrupt it will not bother saving the full context.
        mfsprg0 %r31
        oris    %r31, %r31, PPE42_DISCARD_CTX
        mtsprg0 %r31

        ###### Enable/Disable External Interrupts #####
        wrteei  1
        wrteei  0

        ## If we made it this far, there must not be any interrupts pending.
        ## If bottom half processing was interrupted we need to restore it
check_interrupted_bh:

        ## If the thread ID is 33 then the bottom half handler was interrupted
        ## and needs to be restored.
        extrwi      %r4, %r31, 6, 2
        cmpwi       %r4, 33
        beq         ctx_pop_with_sprg0

check_for_bh:
        ## if the bottom half queue is pointing to itself then the queue is
        ## empty and there are no bottom halves that need processing.
        _lwzsd      %r4, _pk_bh_queue
        lwz         %r5, 0(%r4)
        cmplwbeq    %r4, %r5, restore_interrupted_sp

process_bottom_halves:
        ## Clear the CTX_DISCARD bit so that interrupted bottom half context
        ## will be saved in case an interrupt occurs after this point.  Also
        ## set the thread ID to 33 so that we know to restore the bottom half
        ## context that was interrupted.
        rlwinm  %r3, %r31, 0, 9, 1   //clear thread id + discard bit
        oris    %r3, %r3, 0x2100    //set thread id to 33
        mtsprg0 %r3                 //set bottom half context
        
        ## branch to a C function that processes bottom halves
        wrteei  1
        bl      _pk_process_bh
        wrteei  0

        ## restore the previous kernel context (with discard bit set)
        mtsprg0 %r31

restore_interrupted_sp:
        ## restore the interrupted thread stack pointer
        _lwzsd  %r1,   __pk_saved_sp

        ## If we are not in thread mode (i.e., we took an interrupt in an
        ## interupt-only configuration of PK or after pk_initialize() but
        ## before pk_start_threads) simply pop the context and RFI - in this
        ## case we'll most likely be returning to main() or the non-thread-mode
        ## idle thread.

check_thread_mode:
        bb0wi   %r31, PPE42_THREAD_MODE_BIT, ctx_pop_with_sprg0

        ## Check if external interrupt activated a delayed context switch.  The
        ## C-level code has taken care of the scheduling decisions - we simply
        ## need to implement them here.
check_for_ctx_switch:

        _lwzsd  %r3, __pk_delayed_switch
        bwz     %r3, check_for_idle_thread

        ## Clear the delayed switch flag and go to the context switch code to
        ## finish the switch. 

        li      %r3, 0
        _stwsd  %r3, __pk_delayed_switch
        
        b       __pk_next_thread_resume

        ## check if we should switch to the wait enabled state (idle)
check_for_idle_thread:
        _lwzsd  %r3, __pk_current_thread
        bwz     %r3, __pk_idle_thread

ctx_pop_with_sprg0:
        ## we must ensure that interrupts are disabled while restoring context
        ##
        ## restore sprg0 from the saved context
        lwz     %r0,    PK_CTX_KERNEL_CTX(%r1)
        mtsprg0 %r0

#if PK_KERNEL_TRACE_ENABLE
        srwi    %r0, %r0, 16
        PK_KERN_TRACE_ASM16("RESUME_CONTEXT(0x%04x)", %r0)
#endif

ctx_pop:
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
        lvd     %d3,    PK_CTX_CR(%r1)
        mtlr    %r4
        mtcr0   %r3
        lvd     %d3,    PK_CTX_GPR3(%r1)
        addi    %r1,    %r1, PK_CTX_SIZE

        rfi
        
/// \endcond
