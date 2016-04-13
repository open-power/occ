# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ssx/ppc405/ppc405_exceptions.S $
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
/// \file ppc405_exceptions.S
/// \brief PPC405 exception vector area.
///
/// The PowerPC exception vector area has many small and large 'holes' in the
/// SSX implementation.  These are due to numerous unhandled exceptions and
/// unimplemented exceptions in the exception vector area that comprises 8KB
/// in the 405. SSX interrupt handling and other code is 'packed' into these
/// holes to reduce the effective code footprint of SSX.  The packing is done
/// (hopefully) on a reasonable basis - we haven't tried to squeeze every last
/// byte by chopping up routines willy-nilly and stitching them together with
/// random branches - but some fragmentation has occurred in the interrupt
/// handling code.
///
/// To facilitate the packing, the exception vector area is divided into 5 ELF
/// sections (addresses are offsets into the exception area)
///
/// .vectors_0000 - Empty section for adding image header
///
/// .vectors_0100 - From 0x0100 to 0x0bff.  The beginning of the table through
///                 the large space prior to the system call vector.
/// 
/// .vectors_0c00 - From 0x0c00 to 0x0eff.  This is a moderately large area
///                 after the system call vector.
///
/// .vectors_0f00 - From 0x0f00 to 0x1fff.  From the APU Unavailable vector
///                 through the major 3.5K hole above the Debug vector.
///
/// .vectors_2000 - From 0x2000 to 0x2003 - branch to the Debug handler.
///
/// The exception vector area must be aligned on a 64KB boundary.
///
/// Note that PgP mainstore boot and interrupt controller handling is
/// currently hard-coded into this file - but it can easily be generalized if
/// a port to another environment is required, assuming the new environment
/// has something similar to a PgP or 405 ASIC interrupt controller.
///
/// \cond

        .nolist
#include "ssx.h"
        .list
        
### ****************************************************************************
### .vectors_0000 - Empty section ( Image header will be placed in this section
###                 from the linker command file )
### ****************************************************************************

        .section .vectors_0000, "a", @progbits
        .global __vectors
        .global __vectors_0000
__vectors:
__vectors_0000:

### ****************************************************************************
### .vectors_0100
### ****************************************************************************
        .section .vectors_0100, "ax", @progbits

        .global __vectors_0100

__vectors_0100:

        ############################################################
        # 0x0100 : Critical Interrupt
        ############################################################

__critical_interrupt:

        ## The critical interrupt handler entry point is re-entrant - A handler
        ## may allow preemption, which could cause another entry here.  
        
        ## Entry invariants:
        ## 1. Critical interupts are disabled;
        ## 2. The SP points to a thread stack, the non-critical stack or
        ##    critical stack

        ## Since fast-mode handlers can not use SSX services or alter the
        ## machine context, the exit of a fast mode handler is a simple RF(C)I.
        
        ## Begin by pushing the fast context on the stack.

        _ssx_fast_ctx_push SSX_CRITICAL

        ## Load critical status 0 and the handler array base address.  Check
        ## for interrupts pending in status register 0 while the IRQ is
        ## computed and R5 is loaded with the critical flag.
        
        _lwzi   %r3, %r3, OCB_OCISR0
        _liw    %r6, __ppc405_irq_handlers
        cmpwi   %r3, 0
        cntlzw  %r4, %r3
        li      %r5, SSX_CRITICAL
        bne+    critical_irq_found
        
        ## No IRQ pending in interrupt set 0.  Try set 1.
        
        _lwzi   %r3, %r3, OCB_OCISR1
        cmpwi   %r3, 0
        cntlzw  %r4, %r3
        addi    %r4, %r4, 32
        beq-    critical_phantom

        ## An active IRQ was found.  At entry here R6 has the handler table
        ## base address, R4 has the IRQ number, and R5 has the critical
        ## flag. The IRQ is converted into a pointer to an 8-byte handler
        ## structure, and the handler is dispatched. The call is made with the
        ## parameters:

        ## R3 = private
        ## R4 = irq
        ## R5 = SSX_CRITICAL

critical_irq_found:     

        _save_update_kernel_context SSX_CRITICAL, %r4, %r7
        slwi    %r3, %r4, 3
        lwzux   %r7, %r6, %r3
        lwz     %r3, 4(%r6)
        mtlr    %r7
        blrl
                
        ## Pop the stack/RFCI when (if) it returns here. 

fast_exit_critical:
        
        _ssx_fast_ctx_pop_exit SSX_CRITICAL

        ## This is a phantom interrupt - we got interrupted but no status bits
        ## are set. The interrupt is marked as #64. The register used for the
        ## handler table address (R6) is set to the special structure for the
        ## phantom interrupt, with it's address adjusted to make it appear to
        ## be the 64th entry in the table. 

critical_phantom:       

        _liw    %r6, __ppc405_phantom_irq
        subi    %r6, %r6, (64 * 8)
        b       critical_irq_found


        ############################################################
        # 0x0200 : Machine Check, Data or Instruction
        ############################################################

        .org __vectors_0100 + 0x0100
__machine_check:

        PPC405_MACHINE_CHECK_HANDLER

        .org    __machine_check + 0x20

        .global __ssx_irq_fast2full
__ssx_irq_fast2full:    
        
        ## Convert a fast-mode to a full-mode interrupt by saving the
        ## (volatile - fast) context, and switching to the appropriate system
        ## stack. 

        ## Entry invariants:
        ## 1. The SP/stack must be exactly as it was when the fast-mode
        ##    handler was entered.
        ## 2. No changes have been made to the MSR - the interrupt level must
        ##    remain disabled.
        ## 3. The handler owns the fast context and has not modified the other
        ##    register context.  This routine can only use the (volatile -
        ##    fast) register context.

        ## 41 (linear) instructions plus alignmenmt

        ## Start by pushing the (volatile - fast) context. Technically we also
        ## need to save the CR as our contract with the handler is not to
        ## disturb any of its register state.
        
        _ssx_vol_fast_ctx_push  SSX_IRQ_CONTEXT
        mfcr    %r12

        ## USPRG0 tells whether this is a critical or non-critical interrupt.
        ## The high-order 8 bits of USPRG0 counts critical interrupt nesting,
        ## and the SSX preemption rules guarantee that if the count is > 0 then
        ## we are in a critical handler.

        mfusprg0        %r8
        extrwi. %r9, %r8, 8, 0
        beq     fast2full_noncritical

        ## If the critical interrupt count is > 1, we are already in a
        ## nested critical interrupt, so we're already on the critical stack
        ## and there's nothing left to do.

        cmpwi   %r9, 1
        bne     1f      

        ## Otherwise, save the current stack pointer and switch to the critical
        ## stack. 
        
        _stwsd  %r1, __ssx_saved_sp_critical
        _lwzsd  %r1, __ssx_critical_stack

        ## Restore the CR and return to the now full-mode handler.

1:      
        mtcr    %r12
        blr

        ## Non-critical interrupts are handled analogously to the above,
        ## except that bits 8:15 of R7 are the non-critical
        ## count. At entry here the (volatile - fast) context has been pushed,
        ## R8 has USPRG0 and R12 contains the saved CR.

        ## Note that it would violate a kernel/API invariant if this routine
        ## were entered from outside an interrupt context.

        .cache_align
fast2full_noncritical:
        
        extrwi  %r9, %r8, 8, 8
        cmpwi   %r9, 1
        bne     1f

        _stwsd  %r1, __ssx_saved_sp_noncritical
        _lwzsd  %r1, __ssx_noncritical_stack

1:      

        .if     (SSX_ERROR_CHECK_KERNEL | SSX_ERROR_CHECK_API)
        cmpwi   %r9, 0
        bne     2f
        _ssx_panic PPC405_IRQ_FAST2FULL_INVARIANT
2:      
        .endif  

        mtcr    %r12    
        blr


        ############################################################
        # 0x0300 : Data Storage Interrupt
        ############################################################

        .org __vectors_0100 + 0x0200
__data_storage:

        PPC405_DATA_STORAGE_HANDLER

        .org    __data_storage + 0x20

        .global __ssx_irq_full_mode_exit
__ssx_irq_full_mode_exit:       

        ## Exit a full-mode handler.

        ## Entry invariants:
        ## 1. The SP/stack must be in exactly the same state it was left in at
        ##    the  exit of __ssx_irq_fast2full.
        ## 2. It is assumed the the preemption rules of SSX have been followed
        ##    - in particular that critical handlers have not enabled
        ##    non-critical interrupts.

        ## We can freely modify the volatile context here - the handler is done
        ## and we will restore the interrupted volatile context.

        ## 22 linear instructions

        ## If the critical count is non-zero, then the SSX preemption rules
        ## guarantee that we are exiting from a critical interrupt
        ## handler. This test is safe to make even if critical interrupts are
        ## enabled, because the variable is set exactly once in a critical
        ## section. 

        mfusprg0        %r3
        extrwi. %r4, %r3, 8, 0
        beq     full_exit_noncritical

        ## The context restore must be done from a critical section, in case
        ## the handler enabled preemption.

        _ssx_critical_section_enter     SSX_CRITICAL, %r5, %r6

        ## If the critical count (R4) is > 1 then this is a nested interrupt
        ## and we can simply pop the context and RFCI.

        cmpwi   %r4, 1
        bne     full_exit_critical

        ## Otherwise, restore the saved stack pointer before popping and RFCI.

        _lwzsd  %r1, __ssx_saved_sp_critical

full_exit_critical:     
        _ssx_vol_fast_ctx_pop   SSX_IRQ_CONTEXT, SSX_CRITICAL
        b       fast_exit_critical


        ############################################################
        # 0x0400 : Instruction Storage Interrupt
        ############################################################

        .org __vectors_0100 + 0x0300
__instruction_storage:

        PPC405_INSTRUCTION_STORAGE_HANDLER

        .org    __instruction_storage + 0x20

        ## The idle thread has no permanent register context.  The idle thread
        ## entry point is re-entered whenever the idle thread is scheduled.

        .global __ssx_idle_thread
        .global __ssx_idle_thread_from_bootloader

__ssx_idle_thread:

        ## The idle thread 'uses' the non-critical stack.  Any register context
        ## pushed here is redundant and is wiped out/ignored every time the
        ## idle thread is re-scheduled. 
        
        ## The idle thread simply establishes a default machine context and
        ## enters the wait-enable state.  The idle thread is always entered
        ## with non-critical interrupts disabled.  
        ##
        ## The kernel context is initialized to indicate that the idle thread
        ## is running - the idle thread priority is SSX_THREADS, and the
        ## 'thread-mode' bit is asserted as well.
        ##
        ## This loop can also be called from the SSX bootloader if main()
        ## returns - in which case we don't muck with the USPRG0 or the stack
        ## pointer. 

        li      %r3, (SSX_THREADS | PPC405_THREAD_MODE)
        mtusprg0        %r3
        _lwzsd  %r1, __ssx_noncritical_stack
                
__ssx_idle_thread_from_bootloader:      

        li      %r3, SSX_THREADS
        SSX_TRACE_THREAD_SWITCH %r3, %r4
        _lwzsd  %r3, __ssx_thread_machine_context_default
        _oriwa  %r3, %r3, MSR_WE
        mtmsr   %r3
        b       .

        ## ssx_halt() is implemented on the PPC405 by disabling all
        ## interrupts, forcing external debug mode, and executing a trap.  A
        ## 0x0 word appears after the trap instruction similar to the default
        ## SSX_PANIC macro. The caller may also call ssx_halt() with
        ## parameters which will appear in R3, R4, etc. In the Simics
        ## environment we use the Simics 'trap' since Simics does not handle
        ## the PPC405 TRAP instruction correctly.

        .global ssx_halt
ssx_halt:
        li      %r31, 0
        mtmsr   %r31
        isync
        _liwa   %r31, (DBCR0_EDM | DBCR0_TDE)
        mtdbcr0 %r31
        isync
#if SIMICS_ENVIRONMENT
        rlwimi  1, 1, 0, 0, 0
#else
        trap
#endif
        .long   0

        ############################################################
        # 0x0500 : External Interrupt
        ############################################################

        .org __vectors_0100 + 0x0400
__external_interrupt:   

        ## The non-critical interrupt handler entry point is re-entrant - A
        ## handler may allow preemption, which could cause another entry here.
        
        ## Entry invariants:
        ## 1. Non-critical interupts are disabled;
        ## 2. The SP points to a thread stack or the non-critical stack.

        ## Since fast-mode handlers can not use SSX services or alter the
        ## machine context, the exit of a fast mode handler is a simple RF(C)I.
        
        ## Begin by pushing the fast context on the current stack.

        _ssx_fast_ctx_push SSX_NONCRITICAL

        ## Load noncritical status 0 and the handler array base address.  Check
        ## for interrupts pending in status register 0 while the IRQ is
        ## computed and R5 is loaded with the noncritical flag.
        
        _lwzi   %r3, %r3, OCB_ONISR0
        _liw    %r6, __ppc405_irq_handlers
        cmpwi   %r3, 0
        cntlzw  %r4, %r3
        li      %r5, SSX_NONCRITICAL
        bne+    noncritical_irq_found
        
        ## No IRQ pending in interrupt set 0.  Try set 1.
        
        _lwzi   %r3, %r3, OCB_ONISR1
        cmpwi   %r3, 0
        cntlzw  %r4, %r3
        addi    %r4, %r4, 32
        beq-    noncritical_phantom

        ## An active IRQ was found.  At entry here R6 has the handler table
        ## base address, R4 has the IRQ number, and R5 has the noncritical
        ## flag. The IRQ is converted into a pointer to an 8-byte handler
        ## structure, and the handler is dispatched. The call is made with the
        ## parameters:

        ## R3 = private
        ## R4 = irq
        ## R5 = SSX_NONCRITICAL

noncritical_irq_found:  

        _save_update_kernel_context SSX_NONCRITICAL, %r4, %r7
        slwi    %r3, %r4, 3
        lwzux   %r7, %r6, %r3
        lwz     %r3, 4(%r6)
        mtlr    %r7
        blrl
                
        ## Pop the stack/RFI when (if) it returns here. 

fast_exit_noncritical:
        
        _ssx_fast_ctx_pop_exit SSX_NONCRITICAL

        ## This is a phantom interrupt - we got interrupted but no status bits
        ## are set. The interrupt is marked as #64. The register used for the
        ## handler table address (R6) is set to the special structure for the
        ## phantom interrupt, with it's address adjusted to make it appear to
        ## be the 64th entry in the table. 

noncritical_phantom:    

        _liw    %r6, __ppc405_phantom_irq
        subi    %r6, %r6, (64 * 8)
        b       noncritical_irq_found

        ############################################################
        # 0x0600 : Alignment Exception
        ############################################################

        .org __vectors_0100 + 0x0500
__alignment_exception:

        PPC405_ALIGNMENT_HANDLER

        .org    __alignment_exception + 0x20
pit_handler:            

        ## The portable timer handler of SSX a full-mode handler with the prototype:
        ## void (*ssx_timer_handler)(void).
        ##
        ## To support the portable specification, the kernel clears the
        ## interrupt by writing the PIS back into the TSR before calling the
        ## handler. SSX does not use the PIT in auto-reload mode - it is
        ## tickless - so the interrupt will not fire again until reprogrammed
        ## by the timer handler. The timer handler does not take any arguments.

        ## 21 instructions

        _ssx_fast_ctx_push SSX_NONCRITICAL
        li      %r3, PPC405_IRQ_PIT
        _save_update_kernel_context SSX_NONCRITICAL, %r3, %r4
        
        _liwa   %r3, TSR_PIS
        mttsr   %r3
        isync

        _ssx_irq_fast2full      __ssx_timer_handler


        ############################################################
        # 0x0700 : Program Interrupt
        ############################################################

        .org __vectors_0100 + 0x0600
__program_interrupt:

        PPC405_PROGRAM_HANDLER

        .org    __program_interrupt + 0x20
        
        ## Exiting a full-mode non-critical handler is more complex than the
        ## critical case, because the handler may have made a new
        ## highest-priority thread runnable and we may need to go through a
        ## delayed scheduling step.

        ## Note that the idle thread is treated as a special case.  The idle
        ## thread has no permanent register context. To avoid having to
        ## allocate a stack area for the idle thread, the idle thread 
        ## 'uses' the non-critical stack.  When the idle thread is interrupted
        ## the (redundant) context is pushed, but is then effectively lost.
        ## Whenever we restore the idle thread we simply reenter the idle
        ## thread entry point.

        ## At entry:    
        ## 1. R3 holds the value of USPRG0 (__SsxKernelContext)

        ## 33 linear instructions.

full_exit_noncritical:  
        
        ## Enter a critical section for the return from interrupt, in the event
        ## that the handler enabled preemption.

        _ssx_critical_section_enter     SSX_NONCRITICAL, %r4, %r5

        ## If the non-critical count is > 1 then this is a nested interrupt 
        ## and we can simply pop the context and RFI.  Note that it would
        ## violate a kernel/API invariant if this routine were entered from
        ## outside an interrupt context (interrupt level == 0). 

        extrwi. %r4, %r3, 8, 8
        
        .if     (SSX_ERROR_CHECK_KERNEL | SSX_ERROR_CHECK_API)
        bne     1f
        _ssx_panic PPC405_IRQ_FULL_EXIT_INVARIANT
1:      
        .endif
        
        cmpwi   %r4, 1
        bne     exit_noncritical_without_switch
                
        ## Otherwise, restore the saved stack pointer and continue.
        
        _lwzsd  %r1, __ssx_saved_sp_noncritical

        ## If we are not in thread mode (i.e., we took an interrupt in an
        ## interupt-only configuration of SSX or after ssx_initialize() but
        ## before ssx_start_threads) simply pop the context and RFI - in this
        ## case we'll most likely be returning to main() or the non-thread-mode
        ## idle thread.

        andi.   %r4, %r3, PPC405_THREAD_MODE
        beq     exit_noncritical_without_switch

        ## Now, check for a delayed context switch.  If none is pending, we can
        ## exit (after a check for the idle thread special case).

        _lwzsd  %r3, __ssx_delayed_switch
        cmpwi   %r3, 0
        bne     noncritical_switch

        _lwzsd  %r3, __ssx_current_thread
        cmpwi   %r3, 0
        beq     __ssx_idle_thread

exit_noncritical_without_switch:
        _ssx_vol_fast_ctx_pop   SSX_IRQ_CONTEXT, SSX_NONCRITICAL
        b       fast_exit_noncritical

        ## The non-critical interrupt activated a delayed context switch.  The
        ## C-level code has taken care of the scheduling decisions - we simply
        ## need to implement them here.

noncritical_switch:
                
        ## Clear the delayed switch flag and go to the context switch code to
        ## finish the switch. 

        li      %r3, 0
        _stwsd  %r3, __ssx_delayed_switch
        
        b       thread_save_non_volatile_and_switch


        ############################################################
        # 0x0800 : FPU Unavailable
        ############################################################

        .org __vectors_0100 + 0x0700
__fpu_unavailable:

        PPC405_FPU_UNAVAILABLE_HANDLER

        .org    __fpu_unavailable + 0x20


        ## Exit traces are moved here because the code area (0x100 bytes)
        ## reserved for individual interrupts is overflowing when tracing is
        ## enabled.  This is kind of a hack: We know that this trace only
        ## occurs when we're about to exit the fast context, at a place
        ## where we can use any of the fast registers.

__ssx_trace_critical_irq_exit:  
        SSX_TRACE_CRITICAL_IRQ_EXIT %r3, %r4
        blr
        
__ssx_trace_noncritical_irq_exit:       
        SSX_TRACE_NONCRITICAL_IRQ_EXIT %r3, %r4
        blr
        
        ## >>>>>>>>>> Pack .vectors_0100 here. Room for ~900 bytes. <<<<<<<<<<

### ****************************************************************************
### .vectors_0c00
### ****************************************************************************

        .section .vectors_0c00, "ax", @progbits
        .global __vectors_0c00
__vectors_0c00:         

        ############################################################
        # 0x0c00 : System Call
        ############################################################

        .org __vectors_0c00 + 0x0
        .global __ssx_next_thread_resume
        
__system_call:  

        ## The system call exception is used by SSX as a handy way to start a
        ## context switch, as the continuation address and MSR of the thread to
        ## be swapped out are saved in SRR0 and SRR1.

        ## Non-critical interrupts are disabled at entry.

        ## Note that the system call exception begins a large free area
        ## so there is plenty of room for the context switch code.

        ## Begin by saving the volatile context of the current thread.

        _ssx_fast_ctx_push SSX_NONCRITICAL      
        _ssx_vol_fast_ctx_push  SSX_THREAD_CONTEXT

thread_save_non_volatile_and_switch:    

        ## Finish the thread context save by pushing the non-volatile context
        ## and saving the resulting stack pointer in the thread structure.  If
        ## the current thread is the idle thread this step is bypassed.
        
        ## This symbol is also used as an entry point by the non-critical
        ## interrupt handler - non-critical interrupts are disabled here.
        
        _lwzsd  %r3, __ssx_current_thread
        cmpwi   %r3, 0
        beq     __ssx_next_thread_resume
        
        _ssx_non_vol_ctx_push
        stw     %r1, SSX_THREAD_OFFSET_SAVED_STACK_POINTER(%r3)

        ## The next thread becomes the current thread, and we switch to its
        ## stack - unless the new thread is the idle thread, in which case it
        ## (the idle thread) is simply resumed.  

__ssx_next_thread_resume:
        
        _lwzsd  %r3, __ssx_next_thread
        _stwsd  %r3, __ssx_current_thread

        cmpwi   %r3, 0
        beq     __ssx_idle_thread
        
        lwz     %r1, SSX_THREAD_OFFSET_SAVED_STACK_POINTER(%r3)

        ## Restore the thread context and resume the new thread.  The kernel
        ## context in thread mode is simply the thread priority OR'ed with the
        ## thread-mode flag. All other fields are cleared.

        _ssx_non_vol_ctx_pop
        _ssx_vol_fast_ctx_pop   SSX_THREAD_CONTEXT, SSX_NONCRITICAL
        
        _lbzsd  %r3, __ssx_next_priority
        SSX_TRACE_THREAD_SWITCH %r3, %r4
        ori     %r3, %r3, PPC405_THREAD_MODE            
        mtusprg0 %r3

        _ssx_fast_ctx_pop
        rfi

        ## >>>>>>>> Pack .vectors_0c00 here - room for ~500 bytes <<<<<<<
        
### ****************************************************************************
### .vectors_0f00
### ****************************************************************************
        
        .section .vectors_0f00, "ax", @progbits
        .global __vectors_0f00
__vectors_0f00: 
        
        ############################################################
        # 0x0f20 : APU Unavailable
        ############################################################

        .org __vectors_0f00 + 0x20 # 0x0f20
__apu_unavailable:

        PPC405_APU_UNAVAILABLE_HANDLER
        
        .org    __vectors_0f00 + 0x40 # 0x0f40

fit_handler:
        
        ## The FIT handler is user defined, and is a fast-mode handler. By
        ## convention the kernel clears the interrupt by writing the FIS back
        ## into the TSR.
        
        _ssx_fast_ctx_push SSX_NONCRITICAL
        
        _lwzsd  %r3, __ppc405_fit_arg
        li      %r4, PPC405_IRQ_FIT
        li      %r5, SSX_NONCRITICAL
                
        _save_update_kernel_context SSX_NONCRITICAL, %r4, %r6

        _liwa   %r6, TSR_FIS
        mttsr   %r6
        isync

        _lwzsd  %r6, __ppc405_fit_routine
        mtlr    %r6
        blrl
        
        b       fast_exit_noncritical


        ############################################################
        # 0x10x0 : PIT, FIT and Watchdog Interrupts
        ############################################################

        .org    __vectors_0f00 + 0x100 # 0x1000
__pit_interrupt:
        
        b       pit_handler

        .org    __vectors_0f00 + 0x110 # 0x1010
__fit_interrupt:        

        b       fit_handler

        .org    __vectors_0f00 + 0x120 # 0x1020
__watchdog_interrupt:   

        ## Watchdog setup is described in the SSX Specification. 
        ## The kernel clears TSR[WIS] prior to calling the handler.  
        ## The watchdog handler is a critical, fast-mode handler.

        _ssx_fast_ctx_push SSX_CRITICAL
        
        _lwzsd  %r3, __ppc405_watchdog_arg
        li      %r4, PPC405_IRQ_WATCHDOG
        li      %r5, SSX_CRITICAL
        
        _save_update_kernel_context SSX_CRITICAL, %r4, %r6

        _liwa   %r6, TSR_WIS
        mttsr   %r6
        isync

        _lwzsd  %r6, __ppc405_watchdog_routine
        mtlr    %r6
        blrl

        b       fast_exit_critical


        ############################################################
        # 0x1100 : Data TLB Miss
        ############################################################

        .org __vectors_0f00 + 0x200 # 0x1100
__data_tlb_miss:

        PPC405_DATA_TLB_MISS_HANDLER

        .org    __data_tlb_miss + 0x20
debug_handler:          

        ## SSX does nothing upon reception of the debug interrupt other
        ## than calling the handler (if non-0). The debug handler is a
        ## fast-mode handler.

        _ssx_fast_ctx_push SSX_CRITICAL

        _lwzsd  %r3, __ppc405_debug_arg
        li      %r4, PPC405_IRQ_DEBUG
        li      %r5, SSX_CRITICAL
        
        _save_update_kernel_context SSX_CRITICAL, %r4, %r6

        _lwzsd  %r6, __ppc405_debug_routine
        cmpwi   %r6, 0
        mtlr    %r6
        beq     debug_exit
        blrl
        
debug_exit:     
        b       fast_exit_critical
        

        ############################################################
        # 0x1200 : Instruction TLB Miss
        ############################################################

        .org __vectors_0f00 + 0x300 # 0x1200
__instruction_tlb_miss:

        PPC405_INSTRUCTION_TLB_MISS_HANDLER

        .org    __instruction_tlb_miss + 0x20

        ## >>>>>> Pack .vectors_0f00 A huge hole here - ~3.5KB <<<<<<

### ****************************************************************************
### .vectors_2000
### ****************************************************************************

        .section .vectors_2000, "ax", @progbits
        
        .global __vectors_2000
__vectors_2000:

        ############################################################
        # 0x2000 : Debug Interrupt
        ############################################################

__debug_interrupt:      
        b       debug_handler   

/// \endcond
