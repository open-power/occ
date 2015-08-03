/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/ppe42_boot.S $                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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
        
/// \file ppe42_boot.S
/// \brief PK bootloader for PPE42

        .nolist
#include "pk.h"
        .list
        
### PK Bootloader for PPE42
###
### This is the basic restart initialization of the processor.  
### Parts of this code were derived from examples in the IBM OSopen
### OpenBIOS for the 405GP written by James Burke.
###
### This code does not really do very much, just makes sure that there will
### be a reasonable state in the machine when control is turned over to
### the PK application.  Any core setup that requires SPR access will be done
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
###     DCWR  - Data cache write-through disabled
###     ESR   - No exception syndromes
###     MSR   - No exceptions/interrupts are allowed
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

        .global_function __pk_boot
        .global __reset_trap
        
__pk_boot:
        
        ## Trap the reset for the debugger.  Set R0 to a non-zero value in the
        ## debugger to continue.

        .if     PPE42_RESET_TRAP
        li      %r0, 0
__reset_trap:   
        cmpwi   %r0, 0          
        beq     __reset_trap
        .endif
        
        ## Set up PowerPC EABI constant registers.  These registers are never
        ## again touched by the PK kernel or the application (if they are
        ## behaving).

        _liw    %r2, _SDA2_BASE_
        _liw    %r13, _SDA_BASE_

        ## Clear the timer control register.  This masks all timer interrupts.

        li      %r3, 0
        mttcr   %r3

        ## The stack pointer is initialized for use by the remainder of the 
        ## initialization, including the application main(). The linker script
        ## defines the initial stack area.
        ## 
        ## Stacks are always 8-byte aligned.  A '0' is stored at the
        ## stack pointer to indicate the end of the stack chain. Stack frames
        ## always consist of at least 8 bytes - the backchain pointer and the
        ## slot above the backchain pointer for the callee's LR.
        
        _liw    %r1, _PK_INITIAL_STACK
        _clrfield %r1, %r1, 3, 29 # 8-byte align
        li      %r3, 0
        stwu    %r3, -8(%r1)
        
        ## SPRG0 (__PkKernelContext) is initialized to 0
        ## indicating that the PK kernel is not in thread mode, and no
        ## interrupts are active.

        li      %r3, 0
        mtsprg0 %r3

        ## Set up the initial value of Debug Control Register 0.  Note that
        ## DBCR1 is specified to be cleared at reset.  VBU simulation requested
        ## an option that this register not be modified so that they could
        ## completely control debug behavior from reset of the PPE42.

#ifndef NO_INIT_DBCR0
        _liwa   %r3, PPE42_DBCR_INITIAL
        mtdbcr  %r3
#endif

        ## The exception vector prefix is set - it must be 512 byte aligned.
        ## NOTE: for PPE42, the IVPR is read only, but can be changed through scoms
        
        #_liw    %r3, __vectors
        #andi.   %r4, %r3, 0x01ff
        #beq     1f
        #_pk_panic PPE42_BOOT_VECTORS_NOT_ALIGNED
#1:      
        #mtivpr  %r3
        #sync

        ## The MSR to be used during the rest of intialization is
        ## established.  This MSR should NOT enable 
        ## interrupts, but could enable machine check exceptions.

        _liwa   %r3, PPE42_MSR_INITIAL
        mtmsr   %r3
        sync

#ifdef PK_BOOT_FROM_ROM

        ## NB:  I don't think the old linker scripts were necessarily the most
        ## optimal.  We need to revisit this if we actually do ROM boots in PK
        ## Version 2. Not sure the comments are correct.

        ## Data is copied from the initial ROM image to the RAM.  The 
        ## address symbols are defined in the linker command file. The linker 
        ## will have zeroed this area in the ROM image.

        liw     %r3, __pk_ram_lma - 4 # src
        liw     %r4, __pk_ram_vma - 4 # dest
        liw     %r5, __pk_ram_size
        liw     %r6, 2
        srw     %r5, %r5, %r6   # Number of word transfers
        mtctr   %r5

copy_loop:
        lwzu    %r5, 4(%r3)
        stwu    %r5, 4(%r4)
        bdnz    copy_loop

#endif /* PK_BOOT_FROM_ROM */
        

        ## Call the system setup code.
        
        bl      __ppe42_system_setup
        
        ## Call the application. If for some reason we return from
        ## the call of the application we call an alternate entry point of the
        ## idle thread.
        ##      
        ## An initial argc/argv can be passed into main(). argc is expected to
        ## be a 32-bit immediate integer, and argv is expected to be a 32-bit
        ## absolute or relocatable expression. 

        _liwa   %r3, PPE42_ARGC_INITIAL
        _liw    %r4, PPE42_ARGV_INITIAL
        bl      __pk_main

        b       __pk_idle_thread_from_bootloader

        .epilogue __pk_boot

