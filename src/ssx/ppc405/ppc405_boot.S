/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ppc405/ppc405_boot.S $                                */
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
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------
        
/// \file ppc405_boot.S
/// \brief SSX bootloader for PPC405

        .nolist
#include "ssx.h"
        .list
        
### SSX Bootloader for PPC405
###
### This is the basic restart initialization of the processor.  
### Parts of this code were derived from examples in the IBM OSopen
### OpenBIOS for the 405GP written by James Burke.
###
### This code does not really do very much, just makes sure that there will
### be a reasonable state in the machine when control is turned over to
### the SSX application.  Any core setup that requires SPR access will be done
### here. All other setup is expected to take place in system-specific
### routines. 
###
### From the PowerPC 405-S Embedded Core User's manual:
###
### "In general, the contents of SPRs are undefined after a core, chip or
### system reset. Some SPRs retain the content they had before the reset
### occurred."
###
### Registers fully reset:
###     DBCR1 - Data compares disabled
###     DCCR  - Data cache disabled
###     DCWR  - Data cache write-through disabled
###     ESR   - No exception syndromes
###     ICCR  - Instruction cache disabled
###     MSR   - No exceptions/interrupts are allowed
###     SLER  - Storage is big-endian
###     SU0R  - Storage is uncompressed
### 
### Registers partially reset:
###     CCR0        = 0x00700000 - Sets ICU and DCU PLB Priority
###     DBCR0 [EDM] = 0          - External debug mode disabled
###           [RST] = 0          - No reset action
###     DBSR  [MRR] = x          - x indicates most recent reset action
###     SGR         = 0xffffffff - Storage is guarded
###     TCR   [WRC] = 0          - Watchdog timer reset disabled
###     TSR   [WRS] = x          - x is a copy of TCR[WRC] Watchdog reset status
###           [PIS] = x          - undefined

        .global_function __ssx_boot
        .global __reset_trap
        
__ssx_boot:
        
        ## Trap the reset for the debugger.  Set R0 to a non-zero value in the
        ## debugger to continue.

        .if     PPC405_RESET_TRAP
        li      %r0, 0
__reset_trap:   
        cmpwi   %r0, 0          
        beq     __reset_trap
        .endif
        
        ## First the real-mode memory parameters are set up as configured
        ## for the system and/or application. There are no defaults for these 6
        ## configuration options - they must always be supplied.

        _liwa   %r3, PPC405_ICCR_INITIAL
        mticcr  %r3
        _liwa   %r3, PPC405_DCCR_INITIAL
        mtdccr  %r3
        _liwa   %r3, PPC405_DCWR_INITIAL
        mtdcwr  %r3
        _liwa   %r3, PPC405_SGR_INITIAL
        mtsgr   %r3
        _liwa   %r3, PPC405_SU0R_INITIAL
        mtsu0r  %r3
        _liwa   %r3, PPC405_SLER_INITIAL
        mtsler  %r3
        
        ## Next the I and D caches are invalidated.
        ## NB:  The only reason we can do a BL at this point is because we
        ## know that these routines do not try to save the LR on the stack.
        
        bl      icache_invalidate_all
        bl      dcache_invalidate_all

        ## Set up PowerPC EABI constant registers.  These registers are never
        ## again touched by the SSX kernel or the application (if they are
        ## behaving).

        _liw    %r2, _SDA2_BASE_
        _liw    %r13, _SDA_BASE_

        ## Clear the timer control register.  This masks all timer interrupts.

        mttcr   %r3

        ## Several options are available for dynamic memory ...
        ## If none of these options are defined, then it is assumed that 
        ## the memory area is already set up.

#if defined(SSX_RUN_FROM_DCACHE)
        
        ## SSX can run from the DCACHE. In order for this to work, the
        ## the linker script must set the symbol __ssx_ram_vma to the
        ## beginning of a 128MB memory region marked as cacheable in the
        ## application-defined PPC405_DCCR_INITIAL.
        ##
        ## Note that there is typically no backing store for this data. Before
        ## going further, all data cache blocks must be allocated with 'dcbz'. 
        ## Should a subsequent programming error cause the eviction of
        ## a dirty line, a machine check exception will result.

        _liw    %r3, __ssx_ram_vma
        _liwa   %r4, DCACHE_LINES
        mtctr   %r4
        
dcbz_loop:      
        dcbz    %r0, %r3
        addi    %r3, %r3, CACHE_LINE_SIZE
        bdnz    dcbz_loop                       

#elif defined(SSX_RUN_FROM_MEMORY)

        ## Nothing to do here...

#else

#error "One of SSX_RUN_FROM_DCACHE or SSX_RUN_FROM_MEMORY must be defined"

#endif /* SSX_RUN_FROM_xxx */

        ## The stack pointer is initialized for use by the remainder of the 
        ## initialization, including the application main(). The linker script
        ## defines the initial stack area.
        ## 
        ## Stacks are always 8-byte aligned.  A '0' is stored at the
        ## stack pointer to indicate the end of the stack chain. Stack frames
        ## always consist of at least 8 bytes - the backchain pointer and the
        ## slot above the backchain pointer for the callee's LR.
        
        _liw    %r1, _SSX_INITIAL_STACK
        _clrfield %r1, %r1, 3, 29 # 8-byte align
        li      %r3, 0
        stwu    %r3, -8(%r1)
        
        ## USPRG0 (__SsxKernelContext) is initialized to 0
        ## indicating that the SSX kernel is not in thread mode, and no
        ## interrupts are active.

        li      %r3, 0
        mtusprg0 %r3

        ## Set up the initial value of Debug Control Register 0.  Note that
        ## DBCR1 is specified to be cleared at reset.  VBU simulation requested
        ## an option that this register not be modified so that they could
        ## completely control debug behavior from reset of the 405.

#ifndef NO_INIT_DBCR0
        _liwa   %r3, PPC405_DBCR0_INITIAL
        mtdbcr0 %r3
#endif

        ## The exception vector prefix is set - it must be 64KB aligned.
        
        _liw    %r3, __vectors
        andi.   %r4, %r3, 0xffff
        beq     1f
        _ssx_panic PPC405_BOOT_VECTORS_NOT_ALIGNED
1:      
        mtevpr  %r3
        isync

        ## The MSR to be used during the rest of intialization is
        ## established.  This MSR should NOT enable critical or non-critical
        ## interrupts, but could enable machine check exceptions.

        _liwa   %r3, PPC405_MSR_INITIAL
        mtmsr   %r3
        isync

        ## Initialize the CCR0.  If it returns non-zero, panic.

        _liwa   %r4, PPC405_CCR0_INITIAL # bits_to_set
        not     %r3, %r4        # bits_to_clear
        bl      ppc405_ccr0_modify
        cmpwi   %r3, 0
        beq     1f
        _ssx_panic PPC405_BOOT_CCR0_MODIFY_FAILED
1:      

#ifdef SSX_BOOT_FROM_ROM

        ## NB:  I don't think the old linker scripts were necessarily the most
        ## optimal.  We need to revisit this if we actually do ROM boots in SSX
        ## Version 2. Not sure the comments are correct.

        ## Data is copied from the initial ROM image to the RAM.  The 
        ## address symbols are defined in the linker command file. The linker 
        ## will have zeroed this area in the ROM image.

        liw     %r3, __ssx_ram_lma - 4 # src
        liw     %r4, __ssx_ram_vma - 4 # dest
        liw     %r5, __ssx_ram_size
        liw     %r6, 2
        srw     %r5, %r5, %r6   # Number of word transfers
        mtctr   %r5

copy_loop:
        lwzu    %r5, 4(%r3)
        stwu    %r5, 4(%r4)
        bdnz    copy_loop

#endif /* SSX_BOOT_FROM_ROM */
        

        ## Call the system setup code.
        
        bl      __ppc405_system_setup
        
        ## Call the application. If for some reason we return from
        ## the call of the application we call an alternate entry point of the
        ## idle thread.
        ##      
        ## An initial argc/argv can be passed into main(). argc is expected to
        ## be a 32-bit immediate integer, and argv is expected to be a 32-bit
        ## absolute or relocatable expression. 

        _liwa   %r3, PPC405_ARGC_INITIAL
        _liw    %r4, PPC405_ARGV_INITIAL
        bl      __ssx_main

        b       __ssx_idle_thread_from_bootloader

        .epilogue __ssx_boot

