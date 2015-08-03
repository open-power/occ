/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/image/sbe_common.H $                              */
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
#ifndef __SBE_COMMON_H
#define __SBE_COMMON_H

// $Id:  Exp $

/// \file sbe_common.H

/// \brief This header contains #include-s and macro definitions common to P8
/// and Centaur SBE code development.  It is #include-ed into p8_sbe.H and
/// cen_sbe.H which have other setup specific to each environment.


#include "sbe_link.H"
#include "sbe_xip_image.h"
//#include "sbe_xip_header.H"

/// Macros for generating the .fixed section
///
/// The CPP macros PROC_SBE_FIXED_UINT64[_VECTOR] generate equivalent code
/// depending on whether they are being called from assembler (where they
/// actually create the .fixed section data) or from C (where they specifiy a
/// C-structure form of the contents of the .fixed section.
///
/// In assembler each invocation also creates space in the .fixed_toc section
/// for a fixed TOC entry.  Unlike the normal TOC no data is generated for
/// .fixed_toc by the data declaration.  Instead, the fixed TOC table is
/// filled in during image normalization by normalizeToc (sbe_xip_image.c)
/// which requires that there be one and only one fixed TOC entery allocated
/// per TOC entry referencing the .fixed section. This means that in the
/// current implementation it is not possible to create .fixed_toc entries
/// for addresses or for data stored in sections other than .fixed.

#ifdef __ASSEMBLER__

    .macro    proc_sbe_fixed_uint8, symbol:req, tnumber=1, elements=1
    .global \symbol
\symbol\():
    .rept    ((\tnumber)*(\elements))
    .byte 0
    .endr
    
    .xip_toc \symbol, SBE_XIP_UINT8, \symbol, (\elements)

    .pushsection .fixed_toc
    .space  8
    .popsection

    .endm

    .macro    proc_sbe_fixed_int8, symbol:req, tnumber=1, elements=1
    .global \symbol
\symbol\():
    .rept    ((\tnumber)*(\elements))
    .byte 0
    .endr
    
    .xip_toc \symbol, SBE_XIP_INT8, \symbol, (\elements)

    .pushsection .fixed_toc
    .space  8
    .popsection

    .endm

    .macro    proc_sbe_fixed_uint16, symbol:req, tnumber=1, elements=1
    .balign 2
    .global \symbol
\symbol\():

    .rept    ((\tnumber)*(\elements))
    .long 0
    .endr
    
    .xip_toc \symbol, SBE_XIP_UINT16, \symbol, (\elements)

    .pushsection .fixed_toc
    .space  8
    .popsection

    .endm

    .macro    proc_sbe_fixed_int16, symbol:req, tnumber=1, elements=1
    .balign 2
    .global \symbol
\symbol\():

    .rept    ((\tnumber)*(\elements))
    .long 0
    .endr
    
    .xip_toc \symbol, SBE_XIP_INT16, \symbol, (\elements)

    .pushsection .fixed_toc
    .space  8
    .popsection

    .endm

    .macro    proc_sbe_fixed_uint32, symbol:req, tnumber=1, elements=1
    .balign 4
    .global \symbol
\symbol\():

    .rept    ((\tnumber)*(\elements))
    .long 0
    .endr
    
    .xip_toc \symbol, SBE_XIP_UINT32, \symbol, (\elements)

    .pushsection .fixed_toc
    .space  8
    .popsection

    .endm

    .macro    proc_sbe_fixed_int32, symbol:req, tnumber=1, elements=1
    .balign 4
    .global \symbol
\symbol\():

    .rept    ((\tnumber)*(\elements))
    .long 0
    .endr
    
    .xip_toc \symbol, SBE_XIP_INT32, \symbol, (\elements)

    .pushsection .fixed_toc
    .space  8
    .popsection

    .endm

    .macro    proc_sbe_fixed_uint64, symbol:req, tnumber=1, elements=1
    .balign 8
    .global \symbol
\symbol\():
    .rept    ((\tnumber)*(\elements))
    .quad 0
    .endr
    
    .xip_toc \symbol, SBE_XIP_UINT64, \symbol, (\elements)

    .pushsection .fixed_toc
    .space     8
    .popsection

    .endm

    .macro    proc_sbe_fixed_int64, symbol:req, tnumber=1, elements=1
    .balign 8
    .global \symbol
\symbol\():
    .rept    ((\tnumber)*(\elements))
    .quad 0
    .endr
    
    .xip_toc \symbol, SBE_XIP_INT64, \symbol, (\elements)

    .pushsection .fixed_toc
    .space     8
    .popsection

    .endm

        .macro    proc_sbe_fixed_quadia, symbol:req, space:req, address:req
    .global \symbol
\symbol\():
    .quadia (\space), (\address)
    
    .xip_toc \symbol, SBE_XIP_UINT64, \symbol

    .pushsection .fixed_toc
    .space  8
    .popsection

    .endm


#define PROC_SBE_FIXED_UINT8(symbol) \
            proc_sbe_fixed_uint8 symbol

#define PROC_SBE_FIXED_INT8(symbol)  \
            proc_sbe_fixed_int8 symbol

#define PROC_SBE_FIXED_UINT16(symbol) \
            proc_sbe_fixed_uint16 symbol

#define PROC_SBE_FIXED_INT16(symbol)  \
            proc_sbe_fixed_int16 symbol

#define PROC_SBE_FIXED_UINT32(symbol) \
            proc_sbe_fixed_uint32 symbol

#define PROC_SBE_FIXED_INT32(symbol)  \
            proc_sbe_fixed_int32 symbol

#define PROC_SBE_FIXED_UINT64(symbol) \
            proc_sbe_fixed_uint64 symbol

#define PROC_SBE_FIXED_INT64(symbol)  \
            proc_sbe_fixed_int64 symbol

#define PROC_SBE_FIXED_UINT8_VECTOR(symbol, elements) \
            proc_sbe_fixed_uint8 symbol,  elements

#define PROC_SBE_FIXED_INT8_VECTOR(symbol, elements) \
            proc_sbe_fixed_int8 symbol,  elements
             
#define PROC_SBE_FIXED_UINT16_VECTOR(symbol, elements) \
            proc_sbe_fixed_uint16 symbol,  elements

#define PROC_SBE_FIXED_INT16_VECTOR(symbol, elements) \
            proc_sbe_fixed_int16 symbol,  elements
             
#define PROC_SBE_FIXED_UINT32_VECTOR(symbol, elements) \
            proc_sbe_fixed_uint32 symbol,  elements

#define PROC_SBE_FIXED_INT32_VECTOR(symbol, elements) \
            proc_sbe_fixed_int32 symbol,  elements

#define PROC_SBE_FIXED_UINT64_VECTOR(symbol, elements) \
            proc_sbe_fixed_uint64 symbol,  elements

#define PROC_SBE_FIXED_INT64_VECTOR(symbol, elements) \
            proc_sbe_fixed_int64 symbol,  elements

#define PROC_SBE_FIXED_UINT8_VECTOR_2(symbol, dim1, dim2 ) \
            proc_sbe_fixed_uint8 symbol,  dim1 * dim2   

#define PROC_SBE_FIXED_INT8_VECTOR_2(symbol, dim1, dim2 ) \
            proc_sbe_fixed_int8 symbol,  dim1 * dim2                

#define PROC_SBE_FIXED_UINT16_VECTOR_2(symbol, dim1, dim2 ) \
            proc_sbe_fixed_uint16 symbol,  dim1 * dim2   

#define PROC_SBE_FIXED_INT16_VECTOR_2(symbol, dim1, dim2 ) \
            proc_sbe_fixed_int16 symbol,  dim1 * dim2                

#define PROC_SBE_FIXED_UINT32_VECTOR_2(symbol, dim1, dim2 ) \
            proc_sbe_fixed_uint32 symbol,  dim1 * dim2   

#define PROC_SBE_FIXED_INT32_VECTOR_2(symbol, dim1, dim2 ) \
            proc_sbe_fixed_int32 symbol,  dim1 * dim2                
             
#define PROC_SBE_FIXED_UINT64_VECTOR_2(symbol, dim1, dim2 ) \
            proc_sbe_fixed_uint64 symbol,  dim1 * dim2   

#define PROC_SBE_FIXED_INT64_VECTOR_2(symbol, dim1, dim2 ) \
            proc_sbe_fixed_int64 symbol,  dim1 * dim2                

#define PROC_SBE_FIXED_UINT8_VECTOR_3(symbol, dim1, dim2, dim3 ) \
            proc_sbe_fixed_uint8 symbol,  dim1 * dim2 * dim3 
             
#define PROC_SBE_FIXED_INT8_VECTOR_3(symbol, dim1, dim2, dim3 ) \
            proc_sbe_fixed_uint8 symbol,  dim1 * dim2 * dim3                 

#define PROC_SBE_FIXED_UINT16_VECTOR_3(symbol, dim1, dim2, dim3 ) \
            proc_sbe_fixed_uint16 symbol,  dim1 * dim2 * dim3 
             
#define PROC_SBE_FIXED_INT16_VECTOR_3(symbol, dim1, dim2, dim3 ) \
            proc_sbe_fixed_uint16 symbol,  dim1 * dim2 * dim3                 

#define PROC_SBE_FIXED_UINT32_VECTOR_3(symbol, dim1, dim2, dim3 ) \
            proc_sbe_fixed_uint32 symbol,  dim1 * dim2 * dim3 
             
#define PROC_SBE_FIXED_INT32_VECTOR_3(symbol, dim1, dim2, dim3 ) \
            proc_sbe_fixed_uint32 symbol,  dim1 * dim2 * dim3                 

#define PROC_SBE_FIXED_UINT64_VECTOR_3(symbol, dim1, dim2, dim3 ) \
            proc_sbe_fixed_uint64 symbol,  dim1 * dim2 * dim3 
             
#define PROC_SBE_FIXED_INT64_VECTOR_3(symbol, dim1, dim2, dim3 ) \
            proc_sbe_fixed_uint64 symbol,  dim1 * dim2 * dim3        
            
// Non-Chip Target Attributes
            
#define PROC_SBE_FIXED_TARGET_UINT8(symbol, tnumber) \
            proc_sbe_fixed_uint8 symbol, tnumber

#define PROC_SBE_FIXED_TARGET_INT8(symbol, tnumber)  \
            proc_sbe_fixed_int8 symbol, tnumber

#define PROC_SBE_FIXED_TARGET_UINT16(symbol, tnumber) \
            proc_sbe_fixed_uint16 symbol, tnumber

#define PROC_SBE_FIXED_TARGET_INT16(symbol, tnumber)  \
           proc_sbe_fixed_int16 symbol, tnumber

#define PROC_SBE_FIXED_TARGET_UINT32(symbol, tnumber) \
            proc_sbe_fixed_uint32 symbol, tnumber

#define PROC_SBE_FIXED_TARGET_INT32(symbol, tnumber)  \
            proc_sbe_fixed_int32 symbol, tnumber

#define PROC_SBE_FIXED_TARGET_UINT64(symbol, tnumber) \
            proc_sbe_fixed_uint64 symbol, tnumber

#define PROC_SBE_FIXED_TARGET_INT64(symbol, tnumber)  \
            proc_sbe_fixed_int64 symbol, tnumber

#define PROC_SBE_FIXED_TARGET_UINT8_VECTOR(symbol, tnumber, elements) \
            proc_sbe_fixed_uint8 symbol, tnumber * elements

#define PROC_SBE_FIXED_TARGET_INT8_VECTOR(symbol, tnumber, elements) \
            proc_sbe_fixed_int8 symbol, tnumber * elements
             
#define PROC_SBE_FIXED_TARGET_UINT16_VECTOR(symbol, tnumber, elements) \
            proc_sbe_fixed_uint16 symbol, tnumber * elements

#define PROC_SBE_FIXED_TARGET_INT16_VECTOR(symbol, tnumber, elements) \
            proc_sbe_fixed_int16 symbol, tnumber * elements
             
#define PROC_SBE_FIXED_TARGET_UINT32_VECTOR(symbol, tnumber, elements) \
            proc_sbe_fixed_uint32 symbol, tnumber * elements

#define PROC_SBE_FIXED_TARGET_INT32_VECTOR(symbol, tnumber, elements) \
            proc_sbe_fixed_int32 symbol, tnumber * elements

#define PROC_SBE_FIXED_TARGET_UINT64_VECTOR(symbol, tnumber, elements) \
            proc_sbe_fixed_uint64 symbol, tnumber * elements

#define PROC_SBE_FIXED_TARGET_INT64_VECTOR(symbol, tnumber, elements) \
            proc_sbe_fixed_int64 symbol, tnumber * elements

#define PROC_SBE_FIXED_TARGET_UINT8_VECTOR_2(symbol, tnumber, dim1, dim2 ) \
            proc_sbe_fixed_uint8 symbol, tnumber * dim1 * dim2   

#define PROC_SBE_FIXED_TARGET_INT8_VECTOR_2(symbol, tnumber, dim1, dim2 ) \
            proc_sbe_fixed_int8 symbol, tnumber * dim1 * dim2                

#define PROC_SBE_FIXED_TARGET_UINT16_VECTOR_2(symbol, tnumber, dim1, dim2 ) \
            proc_sbe_fixed_uint16 symbol, tnumber * dim1 * dim2   

#define PROC_SBE_FIXED_TARGET_INT16_VECTOR_2(symbol, tnumber, dim1, dim2 ) \
            proc_sbe_fixed_int16 symbol, tnumber * dim1 * dim2                

#define PROC_SBE_FIXED_TARGET_UINT32_VECTOR_2(symbol, tnumber, dim1, dim2 ) \
            proc_sbe_fixed_uint32 symbol, tnumber * dim1 * dim2   

#define PROC_SBE_FIXED_TARGET_INT32_VECTOR_2(symbol, tnumber, dim1, dim2 ) \
            proc_sbe_fixed_int32 symbol, tnumber * dim1 * dim2                
             
#define PROC_SBE_FIXED_TARGET_UINT64_VECTOR_2(symbol, tnumber, dim1, dim2 ) \
            proc_sbe_fixed_uint64 symbol, tnumber * dim1 * dim2   

#define PROC_SBE_FIXED_TARGET_INT64_VECTOR_2(symbol, tnumber, dim1, dim2 ) \
            proc_sbe_fixed_int64 symbol, tnumber * dim1 * dim2                

#define PROC_SBE_FIXED_TARGET_UINT8_VECTOR_3(symbol, tnumber, dim1, dim2, dim3 ) \
            proc_sbe_fixed_uint8 symbol, tnumber * dim1 * dim2 * dim3 
             
#define PROC_SBE_FIXED_TARGET_INT8_VECTOR_3(symbol, tnumber, dim1, dim2, dim3 ) \
            proc_sbe_fixed_uint8 symbol, tnumber * dim1 * dim2 * dim3                 

#define PROC_SBE_FIXED_TARGET_UINT16_VECTOR_3(symbol, tnumber, dim1, dim2, dim3 ) \
            proc_sbe_fixed_uint16 symbol, tnumber * dim1 * dim2 * dim3 
             
#define PROC_SBE_FIXED_TARGET_INT16_VECTOR_3(symbol, tnumber, dim1, dim2, dim3 ) \
            proc_sbe_fixed_uint16 symbol, tnumber * dim1 * dim2 * dim3                 

#define PROC_SBE_FIXED_TARGET_UINT32_VECTOR_3(symbol, tnumber, dim1, dim2, dim3 ) \
            proc_sbe_fixed_uint32 symbol, tnumber * dim1 * dim2 * dim3 
             
#define PROC_SBE_FIXED_TARGET_INT32_VECTOR_3(symbol, tnumber, dim1, dim2, dim3 ) \
            proc_sbe_fixed_uint32 symbol, tnumber * dim1 * dim2 * dim3                 

#define PROC_SBE_FIXED_TARGET_UINT64_VECTOR_3(symbol, tnumber, dim1, dim2, dim3 ) \
            proc_sbe_fixed_uint64 symbol, tnumber * dim1 * dim2 * dim3 
             
#define PROC_SBE_FIXED_TARGET_INT64_VECTOR_3(symbol, tnumber, dim1, dim2, dim3 ) \
            proc_sbe_fixed_uint64 symbol, tnumber * dim1 * dim2 * dim3                 

#define PROC_SBE_FIXED_QUADIA(symbol, space, address) \
            proc_sbe_fixed_quadia symbol, (space), (address)
             

#else  // __ASSEMBLER__  (eg the C portion)

#define PROC_SBE_FIXED_UINT8(symbol) \
        uint8_t symbol

#define PROC_SBE_FIXED_INT8(symbol) \
        int8_t symbol

#define PROC_SBE_FIXED_UINT16(symbol) \
        uint16_t symbol

#define PROC_SBE_FIXED_INT16(symbol) \
        int16_t symbol

#define PROC_SBE_FIXED_UINT32(symbol) \
        uint32_t symbol

#define PROC_SBE_FIXED_INT32(symbol) \
        int32_t symbol

#define PROC_SBE_FIXED_UINT64(symbol) \
        uint64_t symbol

#define PROC_SBE_FIXED_INT64(symbol) \
        int64_t symbol
        
#define PROC_SBE_FIXED_UINT8_VECTOR(symbol, elements) \
        uint8_t symbol[elements]

#define PROC_SBE_FIXED_INT8_VECTOR(symbol, elements) \
        int8_t symbol[elements]
        
#define PROC_SBE_FIXED_UINT16_VECTOR(symbol, elements) \
        uint16_t symbol[elements]

#define PROC_SBE_FIXED_INT16_VECTOR(symbol, elements) \
        int16_t symbol[elements]
        
#define PROC_SBE_FIXED_UINT32_VECTOR(symbol, elements) \
        uint32_t symbol[elements]

#define PROC_SBE_FIXED_INT32_VECTOR(symbol, elements) \
        int32_t symbol[elements]

#define PROC_SBE_FIXED_UINT64_VECTOR(symbol, elements) \
        uint64_t symbol[elements]

#define PROC_SBE_FIXED_INT64_VECTOR(symbol, elements) \
        int64_t symbol[elements]        
        
#define PROC_SBE_FIXED_UINT8_VECTOR_2(symbol, dim1, dim2 ) \
        uint8_t symbol[dim1][dim2]
        
#define PROC_SBE_FIXED_INT8_VECTOR_2(symbol, dim1, dim2 ) \
        int8_t symbol[dim1][dim2]

#define PROC_SBE_FIXED_UINT16_VECTOR_2(symbol, dim1, dim2 ) \
        uint16_t symbol[dim1][dim2]
        
#define PROC_SBE_FIXED_INT16_VECTOR_2(symbol, dim1, dim2 ) \
        int16_t symbol[dim1][dim2]
        
#define PROC_SBE_FIXED_UINT32_VECTOR_2(symbol, dim1, dim2 ) \
        uint32_t symbol[dim1][dim2]
        
#define PROC_SBE_FIXED_INT32_VECTOR_2(symbol, dim1, dim2 ) \
        int32_t symbol[dim1][dim2]
        
#define PROC_SBE_FIXED_UINT64_VECTOR_2(symbol, dim1, dim2 ) \
        uint64_t symbol[dim1][dim2]
        
#define PROC_SBE_FIXED_INT64_VECTOR_2(symbol, dim1, dim2 ) \
        int64_t symbol[dim1][dim2]

#define PROC_SBE_FIXED_UINT8_VECTOR_3(symbol, dim1, dim2, dim3 ) \
        uint8_t symbol[dim1][dim2][dim3]

#define PROC_SBE_FIXED_INT8_VECTOR_3(symbol, dim1, dim2, dim3 ) \
        int8_t symbol[dim1][dim2][dim3]
        
#define PROC_SBE_FIXED_UINT16_VECTOR_3(symbol, dim1, dim2, dim3 ) \
        uint16_t symbol[dim1][dim2][dim3]

#define PROC_SBE_FIXED_INT16_VECTOR_3(symbol, dim1, dim2, dim3 ) \
        int16_t symbol[dim1][dim2][dim3]

#define PROC_SBE_FIXED_UINT32_VECTOR_3(symbol, dim1, dim2, dim3 ) \
        uint32_t symbol[dim1][dim2][dim3]

#define PROC_SBE_FIXED_INT32_VECTOR_3(symbol, dim1, dim2, dim3 ) \
        int32_t symbol[dim1][dim2][dim3]

#define PROC_SBE_FIXED_UINT64_VECTOR_3(symbol, dim1, dim2, dim3 ) \
        uint64_t symbol[dim1][dim2][dim3]

#define PROC_SBE_FIXED_INT64_VECTOR_3(symbol, dim1, dim2, dim3 ) \
        int64_t symbol[dim1][dim2][dim3]

// Non-Chip Target Attributes

#define PROC_SBE_FIXED_TARGET_UINT8(symbol, tnumber) \
        uint8_t symbol[tnumber]

#define PROC_SBE_FIXED_TARGET_INT8(symbol, tnumber) \
        int8_t symbol[tnumber]

#define PROC_SBE_FIXED_TARGET_UINT16(symbol, tnumber) \
        uint16_t symbol[tnumber]

#define PROC_SBE_FIXED_TARGET_INT16(symbol, tnumber) \
        int16_t symbol[tnumber]

#define PROC_SBE_FIXED_TARGET_UINT32(symbol, tnumber) \
        uint32_t symbol[tnumber]

#define PROC_SBE_FIXED_TARGET_INT32(symbol, tnumber) \
        int32_t symbol[tnumber]

#define PROC_SBE_FIXED_TARGET_UINT64(symbol, tnumber) \
        uint64_t symbol[tnumber]

#define PROC_SBE_FIXED_TARGET_INT64(symbol, tnumber) \
        int64_t symbol[tnumber]
        
#define PROC_SBE_FIXED_TARGET_UINT8_VECTOR(symbol, tnumber, elements) \
        uint8_t symbol[tnumber][elements]

#define PROC_SBE_FIXED_TARGET_INT8_VECTOR(symbol, tnumber, elements) \
        int8_t symbol[tnumber][elements]
        
#define PROC_SBE_FIXED_TARGET_UINT16_VECTOR(symbol, tnumber, elements) \
        uint16_t symbol[tnumber][elements]

#define PROC_SBE_FIXED_TARGET_INT16_VECTOR(symbol, tnumber, elements) \
        int16_t symbol[tnumber][elements]
        
#define PROC_SBE_FIXED_TARGET_UINT32_VECTOR(symbol, tnumber, elements) \
        uint32_t symbol[tnumber][elements]

#define PROC_SBE_FIXED_TARGET_INT32_VECTOR(symbol, tnumber, elements) \
        int32_t symbol[tnumber][elements]

#define PROC_SBE_FIXED_TARGET_UINT64_VECTOR(symbol, tnumber, elements) \
        uint64_t symbol[tnumber][elements]

#define PROC_SBE_FIXED_TARGET_INT64_VECTOR(symbol, tnumber, elements) \
        int64_t symbol[tnumber][elements]        
        
#define PROC_SBE_FIXED_TARGET_UINT8_VECTOR_2(symbol, tnumber, dim1, dim2 ) \
        uint8_t symbol[tnumber][dim1][dim2]
        
#define PROC_SBE_FIXED_TARGET_INT8_VECTOR_2(symbol, tnumber, dim1, dim2 ) \
        int8_t symbol[tnumber][dim1][dim2]

#define PROC_SBE_FIXED_TARGET_UINT16_VECTOR_2(symbol, tnumber, dim1, dim2 ) \
        uint16_t symbol[tnumber][dim1][dim2]
        
#define PROC_SBE_FIXED_TARGET_INT16_VECTOR_2(symbol, tnumber, dim1, dim2 ) \
        int16_t symbol[tnumber][dim1][dim2]
        
#define PROC_SBE_FIXED_TARGET_UINT32_VECTOR_2(symbol, tnumber, dim1, dim2 ) \
        uint32_t symbol[tnumber][dim1][dim2]
        
#define PROC_SBE_FIXED_TARGET_INT32_VECTOR_2(symbol, tnumber, dim1, dim2 ) \
        int32_t symbol[tnumber][dim1][dim2]
        
#define PROC_SBE_FIXED_TARGET_UINT64_VECTOR_2(symbol, tnumber, dim1, dim2 ) \
        uint64_t symbol[tnumber][dim1][dim2]
        
#define PROC_SBE_FIXED_TARGET_INT64_VECTOR_2(symbol, tnumber, dim1, dim2 ) \
        int64_t symbol[tnumber][dim1][dim2]

#define PROC_SBE_FIXED_TARGET_UINT8_VECTOR_3(symbol, tnumber, dim1, dim2, dim3 ) \
        uint8_t symbol[tnumber][dim1][dim2][dim3]

#define PROC_SBE_FIXED_TARGET_INT8_VECTOR_3(symbol, tnumber, dim1, dim2, dim3 ) \
        int8_t symbol[tnumber][dim1][dim2][dim3]
        
#define PROC_SBE_FIXED_TARGET_UINT16_VECTOR_3(symbol, tnumber, dim1, dim2, dim3 ) \
        uint16_t symbol[tnumber][dim1][dim2][dim3]

#define PROC_SBE_FIXED_TARGET_INT16_VECTOR_3(symbol, tnumber, dim1, dim2, dim3 ) \
        int16_t symbol[tnumber][dim1][dim2][dim3]

#define PROC_SBE_FIXED_TARGET_UINT32_VECTOR_3(symbol, tnumber, dim1, dim2, dim3 ) \
        uint32_t symbol[tnumber][dim1][dim2][dim3]

#define PROC_SBE_FIXED_TARGET_INT32_VECTOR_3(symbol, tnumber, dim1, dim2, dim3 ) \
        int32_t symbol[tnumber][dim1][dim2][dim3]

#define PROC_SBE_FIXED_TARGET_UINT64_VECTOR_3(symbol, tnumber, dim1, dim2, dim3 ) \
        uint64_t symbol[tnumber][dim1][dim2][dim3]

#define PROC_SBE_FIXED_TARGET_INT64_VECTOR_3(symbol, tnumber, dim1, dim2, dim3 ) \
        int64_t symbol[tnumber][dim1][dim2][dim3]

#define PROC_SBE_FIXED_QUADIA(symbol, space, address) uint64_t symbol

#endif // __ASSEMBLER__

#endif  // __SBE_COMMON_H
