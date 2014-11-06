// $Id: ppc405_mmu_asm.S,v 1.1.1.1 2013/12/11 21:03:27 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ppc405/ppc405_mmu_asm.S,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------
        
/// \file ppc405_mmu_asm.S
/// \brief Assembler support for PPC405 MMU operations
///
/// Currently the only API in this file - ppc405_mmu_start() - is likely an
/// initialization-only API that could be removed from the run-time image.
        
        .nolist
#include "ssx.h"
        .list

/// Start MMU mode on the PPC405
///
/// This API enables PPC405 address translation of the type supported by SSX -
/// simple 1 to 1 effective - real translation for the purpose of
/// protection. It is coded in assembler to ensure that this transition is
/// done as cache-safely as generically possible.  The API enters a critical
/// section and flushes the data cache and invalidates the I-cache, then
/// enables the translation modes specifed by PPC405_RELOCATION_MODE.
/// Following is a final invalidation of the I-cache.
///
/// \cond

        .global_function ppc405_mmu_start
ppc405_mmu_start:

        ## Create and link a stack frame

        stwu    %r1, -16(%r1)
        mflr    %r0
        stw     %r0, 20(%r1)
        
        ## Enter critical section, save original MSR in R31
        
        stw     %r31, 8(%r1)
        _ssx_critical_section_enter SSX_CRITICAL, %r31, %r4

        ## Flush D-cache, Invalidate I-Cache

        bl      dcache_flush_all
        bl      icache_invalidate_all

        ## Enter translation mode on original MSR (removing critical section) 

        _liw    %r3, 1f
        mtsrr0  %r3
        _liwa   %r3, PPC405_RELOCATION_MODE
        or      %r3, %r31, %r3
        mtsrr1  %r3
        rfi
1:
        ## Invalidate I-cache again

        bl      icache_invalidate_all

        ## Restore R31, Unwind stack and return

        lwz     %r31, 8(%r1)
        lwz     %r0, 20(%r1)
        mtlr    %r0
        addi    %r1, %r1, 16
        blr

        .epilogue ppc405_mmu_start

/// \endcond
