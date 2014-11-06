#ifndef __GPE_H__
#define __GPE_H__

// $Id: gpe.h,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/gpe.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

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
