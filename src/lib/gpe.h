/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/gpe.h $                                               */
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
#ifndef __GPE_H__
#define __GPE_H__
/// \file gpe.h
/// \brief Useful PGAS macros for PORE-GPE procedures

#include "pgas.h"

#ifdef __ASSEMBLER__
#ifdef __PGAS__

// Required to guarantee that the .purgem below always works.  .purgem on
// undefined macros causes an error; There appears to be no way to determine
// if a macro is defined.
#include "ppc32_asm.h"


         // All GPE code should be assembled in the .text.pore section, and
         // all GPE data should be assembled in the .data.pore section.

        .macro  .text.pore
        .section .text.pore, "ax", @progbits
        .balign 4
        .endm


        .macro  .data.pore
        .section .data.pore, "a", @progbits
        .balign 8
        .endm    

        .purgem .function
        .macro  .function symbol
        .text.pore
        .align  2
        .endm

        .purgem .global_function
        .macro  .global_function symbol
        .text.pore
        .align  2
        .global \symbol
        .endm


        // Get the CFAM Id right-justified in a Dx register, scratching a Px
        // register in the process.

        .macro  cfam_id, Dx:req, Px:req
        ..data  (\Dx)
        ..pervasive_chiplet_id (\Px)
        lpcs    (\Px), 0x000f000f
        ldandi  (\Dx), 0x000f000f, (\Px), 0xffffffff00000000
        rols    (\Dx), (\Dx), 32
        .endm


        // This macro defines structure offsets for PORE assembler-versions of
        // structures.

        .macro  .gpeStructField, field:req, size=8
\field\():
        .struct \field + (\size)
        .endm


#endif  // __PGAS__ 
#endif  // __ASSEMBLER__

#endif  // __GPE_H__ 
