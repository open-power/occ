/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/image/proc_sbe_fixed.H $                          */
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

/// \file proc_sbe_fixed.H
/// \brief Define the layout of fixed-position data in the P9 SBE reference
/// and IPL images
///
/// Contains struct ProcSbeFixed which contains functions, rings and
/// attributes whose pointers are stored in the fixed and fixed_toc section
///
/// This file defines the layout of the special .fixed data section of P9 SBE
/// reference and IPL images.  The .fixed section is guaranteed to appear at a
/// fixed offset from the beginning of the image, containing data required to
/// be manipulated when images are resident in non-volatile memories, thereby
/// avoiding having to search the TOC. Data manipulated during in-memory image
/// processing can always be easily located using the SBE-XIP TOC APIs.  All
/// of the data stored in .fixed can also still be referenced via the SBE-XIP
/// TOC.
///
/// This header file can be #include-ed into either C or SBE assembly language
/// source code.  In C, it creates simple structures 'ProcChipAttributes', 
/// 'PervAttributes', 'CoreAttributes', 'EQAttributes' and 'EXAttributes' that 
/// contain attribute information that are sized per XML definition per entry.
///
/// PPE image data is always stored big-endian, so applications on little-endian 
/// hosts will need to perform the appropriate endian converison when reading or
/// writing images via this header.
///
/// In assembler code, this header creates macros '.proc_sbe_fixed_proc_chip',
/// '.proc_sbe_fixed_perv', '.proc_sbe_fixed_core', '.proc_sbe_fixed_ex', 
/// '.proc_sbe_fixed_eq', that is expanded exactly once in the file 
/// 'sbe_base_header.S', creating the actual data allocation of the data 
/// equivalent to the C structure.  Assembler code references the data symbols 
/// as normal.
///
/// To simplify programming a 'mini-TOC' is also provided for this data in
/// the .fixed_toc section.  This section is comprised of SbeXipHashedToc
/// structures. When a symbol is indedxed in .fixed, a blank SbeXipHashedToc
/// stucture is added to .fixed_toc.  During image normalization the array of
/// SbeXipHashedToc is filled in as each symbol in .fixed is re-indexed into
/// .fixed_toc.

#ifndef __PROC_SBE_FIXED_H__
#define __PROC_SBE_FIXED_H__

#include "p9_sbe.H"
#include "plat_target_parms.H"
#include "fapi2AttributeIds.H"


#ifdef __ASSEMBLER__
         .macro     .proc_sbe_fixed_proc_chip
	     .section   .fixed, "a", @progbits
	     .balign    8
         .global    G_proc_chip_attributes
G_proc_chip_attributes:         
#else
extern "C" {
namespace fapi2attr {
typedef struct ProcChipAttributes_t {
#endif

#include "proc_sbe_fixed_proc_chip.H"

#ifdef __ASSEMBLER__
	.endm
#else
} ProcChipAttributes;
} // fapi2
} // C
#endif

///
/// Pervasive Target Attributes
#ifdef __ASSEMBLER__
         .macro	    .proc_sbe_fixed_perv
	     .section   .fixed, "a", @progbits
	     .balign    8
         .global    G_perv_attributes
G_perv_attributes:     
#else
namespace fapi2attr {
typedef struct PervAttributes_t {
#endif

#include "proc_sbe_fixed_perv.H"

#ifdef __ASSEMBLER__
	.endm
#else
} PervAttributes;
} // fapi2
#endif

///
/// Core (EC) Target Attributes
#ifdef __ASSEMBLER__
        .macro     .proc_sbe_fixed_core
        .section   .fixed, "a", @progbits
	    .balign    8
        .global     G_core_attributes
G_core_attributes:     
#else
namespace fapi2attr {
typedef struct CoreAttributes_t {
#endif

#include "proc_sbe_fixed_core.H"
    
#ifdef __ASSEMBLER__
	.endm
#else
} CoreAttributes;
} // fapi2
#endif

///
/// EX Target Attributes
#ifdef __ASSEMBLER__
         .macro	    .proc_sbe_fixed_ex
	     .section   .fixed, "a", @progbits
	     .balign    8
         .global    G_ex_attributes
G_ex_attributes: 
#else
namespace fapi2attr {
typedef struct EXAttributes_t {
#endif

#include "proc_sbe_fixed_ex.H"
    
#ifdef __ASSEMBLER__
	.endm
#else
} EXAttributes;
} // fapi2
#endif

///
/// EQ Target Attributes
#ifdef __ASSEMBLER__
         .macro	    .proc_sbe_fixed_eq
	     .section   .fixed, "a", @progbits
	     .balign    8
         .global    G_eq_attributes
G_eq_attributes:
#else
namespace fapi2attr {
typedef struct EQAttributes_t {
#endif

#include "proc_sbe_fixed_eq.H"
    
#ifdef __ASSEMBLER__
	.endm
#else
} EQAttributes;
} // fapi2
#endif


#endif  // __PROC_SBE_FIXED_H__
