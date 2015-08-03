/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ppc405/ppc405_breakpoint.S $                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2015                        */
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
        
/// \file breakpoint.S
/// \brief Implementation of code breakpoints for VBU/VPO
///
/// The operation of these short assembler routines is introduced as part of
/// the documentation for the CODE_BREAKPOINT macro.

        .nolist
#include "ssx.h"
        .list
        
#ifdef DOXYGEN_ONLY
void _code_breakpoint_prologue(void);
#endif

/// \cond

        _sprinstrs iac4, SPRN_IAC4

        .global_function _code_breakpoint_prologue
_code_breakpoint_prologue:

        // Save the DBCR0/1 and IAC4

        mfdbcr0 %r3
        mfdbcr1 %r4
        mfiac4  %r5
        _stwsd  %r3, _saved_dbcr0
        _stwsd  %r4, _saved_dbcr1
        _stwsd  %r5, _saved_iac4

        // Set IAC4 to the contents of LR, which will cause a break in the
        // caller's context.

        mflr    %r3
        mtiac4  %r3

        // Reprogram to external debug mode, trigger on IAC4 compare, freeze
        // timers. Clear data compare setup for good measure.

        _liwa   %r3, DBCR0_EDM | DBCR0_IA4 | DBCR0_FT
        mtdbcr0 %r3
        _liwa   %r3, 0
        mtdbcr1 %r3

        isync
        blr

        .epilogue _code_breakpoint_prologue

        .section .sdata
        .balign 4
        
        .global _code_breakpoint_enable
_code_breakpoint_enable:
        .long   1
        
_saved_dbcr0:
        .long   0
_saved_dbcr1:
        .long   0
_saved_iac4:
        .long   0
        
/// \endcond


#ifdef DOXYGEN_ONLY
void _code_breakpoint_epilogue(void);
#endif

/// \cond

        .global_function _code_breakpoint_epilogue
_code_breakpoint_epilogue:

        // Restore the DBCR0/1 and IAC4

        _lwzsd   %r3, _saved_dbcr0
        _lwzsd   %r4, _saved_dbcr1
        _lwzsd   %r5, _saved_iac4
        mtdbcr0 %r3
        mtdbcr1 %r4
        mtiac4  %r5

        // Clear IA4 status

        _liwa   %r3, DBSR_IA4
        mtdbsr  %r3

        isync
        blr
        
        .epilogue _code_breakpoint_epilogue
        
/// \endcond
