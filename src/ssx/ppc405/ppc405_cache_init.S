// $Id: ppc405_cache_init.S,v 1.3 2014/06/26 12:59:35 cmolsen Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ppc405/ppc405_cache_init.S,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------
        
/// \file ppc405_cache_init.S
/// \brief PPC405-specific procedures required for cache initialization
///
/// The routines in this file are not part of SSX per se, but are included
/// with SSX because they are used during initialialization and boot.  
/// 
/// These are 'initialization' routines used by the SSX bootloader and other
/// application initialization code and may not be needed after
/// boot/initialization of the SSX application.
///
/// See the documents './doc/PowerPC 405-S Core Errata' and './doc/PowerPC
/// 405-S Core Errata Impact on PGP OCC Firmware' for information on the
/// CPU_xxx errata.             

        .nolist
#include "ssx.h"
#include "ppc405_context.h"
        .list

/// \fn void icache_invalidate_all()
/// \brief Invalidate the entire I-Cache
///
/// In the PPC405 a single instruction invalidates the entire I-Cache,
/// therefore this routine need not be executed from a critical section.
///
///
/// This API always issues an isync() after the invalidation.
#ifdef DOXYGEN_ONLY
void icache_invalidate_all();
#endif
/// \cond

        .global_function icache_invalidate_all

icache_invalidate_all:  

        _ssx_critical_section_enter SSX_SUPERCRITICAL, %r5, %r6

        # Workaround for CPU_121:  The iccci instruction may errantly
        # cause a Data TLB exception. Workaround avoids this erratum by
        # temporarily disabling data address translation.

        mfmsr   %r3
        andi.   %r4, %r3, 0xffef
        mtmsr   %r4
        isync

        ## On the 405, iccci invalidates the entire I-Cache.
        
        iccci   %r0,%r0     
       
        # Restore MSR/DR bit 
        mtmsr   %r3
        isync          

        _ssx_critical_section_exit %r5
       
        blr                          
        
        .epilogue icache_invalidate_all

/// \endcond


/// \fn void dcache_invalidate_all()
/// \brief Invalidate the entire D-Cache
///
/// This API operates in an SSX_SUPERCRITICAL critical section.  This API always
/// issues a sync() after the invalidation. 
#ifdef DOXYGEN_ONLY
void dcache_invalidate_all();
#endif
/// \cond

        .global_function dcache_invalidate_all
        
dcache_invalidate_all:  

        _ssx_critical_section_enter SSX_SUPERCRITICAL, %r5, %r6

        ## We loop through addresses 0 ... (DCACHE_SIZE / DCACHE_WAYS) - 1
        ## invalidating the D-Cache.  The dccci instruction on the 405
        ## invalidates both ways.

        li      %r3, 0
        _liwa   %r4, (DCACHE_LINES / DCACHE_WAYS)
        mtctr   %r4

1:      
        dccci   %r0, %r3
        addi    %r3, %r3, CACHE_LINE_SIZE
        bdnz    1b
        
        sync

        _ssx_critical_section_exit %r5
        
        blr
                                
        .epilogue dcache_invalidate_all

/// \endcond


#ifndef USE_GENERIC_DCACHE_FLUSH_ALL
#define USE_GENERIC_DCACHE_FLUSH_ALL 1
#endif

/// \fn void dcache_flush_all()
/// \brief Flush the entire D-Cache (Generic)
///
/// This API is necessary whenever it is required to change data cacheability
/// after boot. The D-cache is scanned, and any lines present in the cache are
/// flushed. This API operates in an SSX_SUPERCRITICAL critical section.  This
/// API always issues a sync() after the flush.  
///
/// This is a generic implementation that should work on all PPC405
/// systems. If something is known about the memory configuration then SSX
/// ports can implement better/more efficient flush-all algorithms.
///
/// This API runs with data translation disabled.  This is necessary for
/// correctness, and also obviates the need to check whether a cache entry is
/// valid before flushing the entry.
///
/// This API is currently a NOP in Simics simulations which complain that the
/// dcread instruction is implemented as a NOP.
#ifdef DOXYGEN_ONLY
void dcache_flush_all();
#endif
/// \cond

#if USE_GENERIC_DCACHE_FLUSH_ALL

                .global_function dcache_flush_all
dcache_flush_all:
        
#if !SIMICS_ENVIRONMENT

        ## %r3 used as temp throughout
        ## %r4 holds the original MSR throughout
        ## %r8 used as a temp throughout
        ## %r9 used as a temp throughout
        
        _ssx_critical_section_enter SSX_SUPERCRITICAL, %r4, %r3

        mfmsr   %r3
        _clrbit %r3, %r3, MSR_DR_BIT
        mtmsr   %r3
        isync
        
        ## %r5 counts the congruence class address
        ## CTR counts the number of congruence classes

        li      %r5, 0
        _liwa   %r9, (DCACHE_LINES / DCACHE_WAYS)
        mtctr   %r9

        ## %r6 has the CCR0 value for reading DCACHE tag on way A
        ## %r7 has the CCR0 value for reading DCACHE tag on way B
        ## %r8 has the cache tag mask
        
        mfccr0  %r6
        _liwa   %r9, 0x00000011
        andc    %r6, %r6, %r9   # Clear DCREAD control bits
        ori     %r6, %r6, 0x0010
        ori     %r7, %r6, 0x0001
        _liwa   %r8, DCACHE_TAG_MASK    # Cache tag mask

dflush_loop:

        ## Way A

        mtccr0  %r6             # Set CCR0 for DCREAD
        dcread  %r9, %r0, %r5
        and     %r9, %r9, %r8   # Mask out cache tag
        or      %r9, %r9, %r5   # OR in the congruence class address
        dcbf    %r0, %r9        # Flush the line

        ## Way B

        mtccr0  %r7             # Set CCR0 for DCREAD
        dcread  %r9, %r0, %r5
        and     %r9, %r9, %r8   # Mask out cache tag
        or      %r9, %r9, %r5   # OR in the congruence class address
        dcbf    %r0, %r9        # Flush the line

        addi    %r5, %r5, CACHE_LINE_SIZE
        bdnz    dflush_loop
        
        ## Restore the MSR and return

        _ssx_critical_section_exit %r4

#endif // SIMICS_ENVIRONMENT

        blr
                                
        .epilogue dcache_flush_all

#endif // USE_GENERIC_DCACHE_FLUSH_ALL

/// \endcond

/// \fn int ppc405_ccr0_modify(uint32_t bits_to_clear, uint32_t bits_to_set)
/// \brief A special code sequence to modify the Cache Control Register 0 (CCR0)
/// 
/// \param bits_to_clear A positive mask of bits in CCR0 that will be cleared
/// by this call.
///
/// \param bits_to_set A positive mask of bits in CCR0 that will be set by
/// this call.
///
/// The code that implements this procedure \e must be in I-cacheable memory.
///
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Successful completion.
///
/// \retval -SSX_ILLEGAL_CONTEXT_PPC405_CACHE The code implementing the API 
/// is not cacheable. 
///
#ifdef DOXYGEN_ONLY
int ppc405_ccr0_modify(uint32_t bits_to_clear, uint32_t bits_to_set);
#endif
/// \cond

        ## The implementation is based on an example from the 405GP manual.

        .global_function ppc405_ccr0_modify

ppc405_ccr0_modify:

        ## %r3 is parameter bits_to_clear
        ## %r4 is parameter bits_to_set
        ## %r5 will hold the original MSR
        ## %r6 will hold the address of the update code.
        ## %r7, %r8, %r9 are used for computation.

        ## Disable external and critical interrupts

        mfmsr   %r5
        _liwa   %r6, (MSR_CE | MSR_EE)
        andc    %r6, %r5, %r6
        mtmsr   %r6             # Interrupts disabled
        sync

        .if     SSX_ERROR_CHECK_API

        ## Check for cacheability of the code sequence

        _liw    %r6, ccr0_modify_doit
        extrwi  %r7, %r6, 5, 0  # Upper 5 bits are address range
        mficcr  %r8
        _liwa   %r9, 0x80000000
        srw     %r9, %r9, %r7   # %r9 holds bit to check in %r8
        and.    %r9, %r9, %r8
        bne     ccr0_modify_continue
        
        ##  Whoops! Code not cacheable; restore MSR and return error code.
        
        .if     SSX_ERROR_PANIC
        
        _ssx_panic SSX_ILLEGAL_CONTEXT_PPC405_CACHE

        .else
        
        _liwa   %r3, -SSX_ILLEGAL_CONTEXT_PPC405_CACHE
        mtmsr   %r5
        isync

        .endif

        blr

        .endif

        ## Touch the code sequence into the I-Cache and do it.

ccr0_modify_continue:   
        icbt    %r0, %r6
        isync                   # Erratum CPU_208
        b       ccr0_modify_doit

        ## The ICBT is repeated to insure that the code is really in
        ## the cache.  The bits are cleared, set, updated and we exit.

        .cache_align            # Must be cache-line aligned
ccr0_modify_doit:
        icbt    %r0, %r6
        isync
        mfccr0  %r7
        andc    %r7, %r7, %r3   # Clear some bits
        or      %r7, %r7, %r4   # Set some bits
        mtccr0  %r7
        isync

        ## Restore MSR and return 0.

        li      %r3, 0
        mtmsr   %r5
        isync
        blr

        .epilogue ppc405_ccr0_modify

/// \endcond
