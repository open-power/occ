/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/occhw/occhw_cache.S $                                 */
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
        
/// \file occhw_cache.S
/// \brief Cache-management specific to OCCHW

#include "ssx.h"

/// \fn void dcache_flush_all()
/// \brief Flush the entire D-Cache by 0-filling and invalidating
///
/// This API is necessary whenever it is required to change data cacheability
/// after boot.  This API operates in an SSX_SUPERCRITICAL critical section.
/// This API always issues a sync() after the flush.
///
/// This API runs with data translation disabled.  This is necessary for
/// correctness, and also obviates the need to check whether a cache entry is
/// valid before flushing the entry.
///
/// This algorithm works by filling the cache with 0s to displace any dirty
/// lines.  Then the cache is invalidated.  In OCCHW the first 16 KB of the
/// 0x80000000 address range are used as the zero-fill range.  This memory is
/// not mapped on the OCI so these lines must never escape the D-cache.
///
/// Note: Our Simics model includes this 16K memory area since Simics does not
/// default to having a cache.  Since we run OCCHW with the MMU enabled and we
/// don't MMU-map this area, memory addressing bugs should not be able to slip
/// through. 
#ifdef DOXYGEN_ONLY
void dcache_flush_all();
#endif
/// \cond

        .global_function dcache_flush_all
dcache_flush_all:
        
        ## %r3 used as scratch throughout
        
        ## %r11 holds the original DCCR throughout
        ## %r12 holds the original MSR throughout

        ##  Enter a super-critical section and go to real mode

        _ssx_critical_section_enter SSX_SUPERCRITICAL, %r12, %r3

        mfmsr   %r3    
        _clrbit %r3, %r3, MSR_DR_BIT
        mtmsr   %r3
        isync
        
        ## Save the DCCR, and make 0x80000000 cacheable.  This is necessary for
        ## DCBZ to work.

        mfdccr  %r11
        _liwa   %r3, OCCHW_FLUSH_ZERO_DCCR
        or      %r3, %r3, %r11
        mtdccr  %r3
        isync

        ## Fill the cache with 0, displacing any dirty lines

        li      %r3, DCACHE_LINES
        mtctr   %r3
        _liwa   %r3, OCCHW_FLUSH_ZERO_ADDRESS
1:      
        dcbz    %r0, %r3
        addi    %r3, %r3, CACHE_LINE_SIZE
        bdnz    1b

        sync

        ## Now invalidate the cache

        li      %r3, DCACHE_LINES
        mtctr   %r3
        _liwa   %r3, OCCHW_FLUSH_ZERO_ADDRESS
1:      
        dcbi    %r0, %r3
        addi    %r3, %r3, CACHE_LINE_SIZE
        bdnz    1b
        
        ## Restore the DCCR and MSR and return

        mtdccr  %r11
        isync
        
        _ssx_critical_section_exit %r12

        blr
                                
        .epilogue dcache_flush_all

/// \endcond
        
