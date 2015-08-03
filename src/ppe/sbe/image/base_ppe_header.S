/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/image/base_ppe_header.S $                         */
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

/// \file base_ppe_header.S
/// \brief code to create header, toc, strings, fixed and fixed_toc sections
///
/// function SbeXipHeader creates header section, function .proc_sbe_fixed
/// creates fixed, fixed_toc sections
#include "sbe_xip_image.h"
#include "proc_sbe_fixed.H"

        
#define PORE_SPACE_UNDEFINED 0xffff
#define PORE_SPACE_OCI       0x8000
#define PORE_SPACE_PNOR      0x800b
#define PORE_SPACE_OTPROM    0x0001
#define PORE_SPACE_SEEPROM   0x800c
#define PORE_SPACE_PIBMEM    0x0008 



	.macro	..checku, x:req, bits:req, err="Unsigned value too large"

	.if	(((\bits) <= 0) || ((\bits) > 63))
	.error	"The number of bits must be in the range 0 < bits < 64"
	.endif

	.iflt	(\x)
	.error  "An unsigned value is required here"
        .endif
	
	.ifgt	((\x) - (0xffffffffffffffff >> (64 - (\bits))))
	.error	"\err"
	.endif

	.endm        

    .macro  ..check_u16, u16
	..checku (\u16), 16, "Unsigned immediate is larger than 16 bits"
    .endm
        
     
    .macro  ..set_default_space, s
	..check_u16 (\s)
    .set    _PGAS_DEFAULT_SPACE, (\s)
    .endm
        
	.macro	..check_default_space
	.if	(_PGAS_DEFAULT_SPACE == PORE_SPACE_UNDEFINED)
	.error	"The PGAS default address space has not been defined"
	.endif
	.endm	
        

	.macro	.quada, offset:req
	..check_default_space
	.long   _PGAS_DEFAULT_SPACE
	.long	(\offset)
	.endm        

        
//    .macro	SbeXipHeader, magic, link_address, entry_point, image_size

	.macro	SbeXipSection, s:req, alignment=1, empty=0
__\s\()_section:
	.if	\empty
		.long	0
		.long	0
	.else
		.long	_\s\()_offset
		.long	_\s\()_size
	.endif
		.byte	(\alignment)
        .byte	0, 0, 0
	.endm


	.macro	SbeXipHeader, magic, link_address, entry_point, image_size
	
	.section .header, "a", @progbits


	//////////////////////////////////////////////////////////////////////
	// Identification - 8-byte aligned; 8 Entries; TOC-Indexed
	//////////////////////////////////////////////////////////////////////

__magic:	
	.quad	(\magic)
__entry_offset:	
	.quad	((\entry_point) - (\link_address))
__link_address:
	.quada	(\link_address)
__header_64_reserved:
	.quad	0, 0, 0, 0, 0

	.xip_toc magic, SBE_XIP_UINT64, __magic
	.xip_toc entry_offset, SBE_XIP_UINT64, __entry_offset
	.xip_toc link_address, SBE_XIP_UINT64, __link_address	


        //////////////////////////////////////////////////////////////////////
        // Section Table - 8-byte aligned; 16 entries; Not TOC-Indexed
        //////////////////////////////////////////////////////////////////////

        SbeXipSection header
        SbeXipSection fixed,                8
        SbeXipSection fixed_toc,            8
        SbeXipSection ipl_text,             4
        SbeXipSection ipl_data,             8
        SbeXipSection text,                 4
        SbeXipSection data,                 8
        SbeXipSection toc,                  4
        SbeXipSection strings
        SbeXipSection halt,                 4, empty=1
        SbeXipSection pibmem0,              8, empty=1
        SbeXipSection dcrings,              8, empty=1
        SbeXipSection rings,                8, empty=1
        SbeXipSection slw,                128, empty=1
        SbeXipSection fit,                     empty=1
        SbeXipSection ffdc,                 8, empty=1

        
        //////////////////////////////////////////////////////////////////////
        // Other Information - 4-byte aligned; 8 entries; TOC-Indexed
        //////////////////////////////////////////////////////////////////////

__image_size:
	.long	(\image_size)
__build_date:
	.long	0
__build_time:
	.long	0
__header_32_reserved:
	.long	0, 0, 0, 0, 0

	.xip_toc image_size, SBE_XIP_UINT32, __image_size
	.xip_toc build_date, SBE_XIP_UINT32, __build_date
	.xip_toc build_time, SBE_XIP_UINT32, __build_time


        //////////////////////////////////////////////////////////////////////
        // Other Information - 1-byte aligned; 8 entries; TOC-Indexed
        //////////////////////////////////////////////////////////////////////

__header_version:
	.byte	SBE_XIP_HEADER_VERSION
__toc_normalized:
	.byte	0
__toc_sorted:
	.byte	0
__header_8_reserved:
	.byte	0, 0, 0, 0, 0

	.xip_toc header_version, SBE_XIP_UINT8, __header_version
	.xip_toc toc_normalized, SBE_XIP_UINT8, __toc_normalized
	.xip_toc toc_sorted, SBE_XIP_UINT8, __toc_sorted


        //////////////////////////////////////////////////////////////////////
        // Strings; 64 characters allocated; TOC-Indexed
        //////////////////////////////////////////////////////////////////////

__build_user:
	.asciz	"unknown        "         # 16 Characters allocated
__build_host:
	.asciz	"unknown                " # 24 characters allocated
__header_string_reserved:
	.space	24, 0

	.xip_toc build_user, SBE_XIP_STRING, __build_user
	.xip_toc build_host, SBE_XIP_STRING, __build_host


	.endm
        

    .section .fixed, "a", @progbits
    .section .fixed_toc, "a", @progbits        
    .section .ipl_data, "a", @progbits
    .section .ipl_text, "a", @progbits        
        
    .section .toc, "a", @progbits
    .section .strings, "aS", @progbits
    
    ..set_default_space PORE_SPACE_SEEPROM
    SbeXipHeader SBE_SEEPROM_MAGIC, 0xFFF00000, 0xFFF00C78, _sbe_base_size 

    // Create the .fixed section    
    .proc_sbe_fixed_proc_chip
    .proc_sbe_fixed_perv
    .proc_sbe_fixed_core
    .proc_sbe_fixed_ex
    .proc_sbe_fixed_eq
