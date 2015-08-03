/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe/ppe_common.h $                                 */
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
#ifndef __PPE_COMMON_H__
#define __PPE_COMMON_H__

//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppe_common.h
/// \brief Common header for PPE
///
/// This header is maintained as part of the PK port for PPE, but needs to be
/// physically present in the PMX area to allow dropping PMX code as a whole
/// to other teams.

// -*- WARNING: This file is maintained as part of PK.  Do not edit in -*-
// -*- the PMX area as your edits will be lost.                         -*-

#ifndef __ASSEMBLER__
#include <stdint.h>
#endif

/// This constant is used to define the size of the table of interrupt handler
/// structures as well as a limit for error checking. 
/// NOTE: This can be specific to each PPE type (SBE, PPE, GPE)
#define EXTERNAL_IRQS 64

#ifdef __ASSEMBLER__
/// This macro contains PPE specific code.
/// Since standalone models of the PPE do not support external interrupts
/// we just set the code to 64 (phantom interrupt)
    .macro hwmacro_get_ext_irq
        
        li  %r4, 64

    .endm

/// Redirect the .hwmacro_irq_cfg_bitmaps macro to call our ppe specific implementation
/// This is called from the ppe42_exceptions.S file.
/// NOTE: The standalone version of PPE doesn't support external interrupts so this
///       does nothing.
    .macro .hwmacro_irq_cfg_bitmaps
    .endm


#endif /* __ASSEMBLER__ */

#endif  /* __PPE_COMMON_H__ */
