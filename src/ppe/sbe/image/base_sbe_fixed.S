/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/image/base_sbe_fixed.S $                          */
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
/// \file sbefixed.S
/// \brief code to generated fixed section in pibmem image
///
#include "sbe_xip_image.h"
#include "proc_sbe_fixed.H"

        
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
        
        

    .section .fixed, "a", @progbits
    

    // Create the .fixed section    
    .proc_sbe_fixed_proc_chip
    .proc_sbe_fixed_perv
    .proc_sbe_fixed_core
    .proc_sbe_fixed_ex
    .proc_sbe_fixed_eq
