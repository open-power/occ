#ifndef __TOD_FIRMWARE_REGISTERS_H__
#define __TOD_FIRMWARE_REGISTERS_H__

// $Id: tod_firmware_registers.h,v 1.1.1.1 2013/12/11 21:03:23 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/registers/tod_firmware_registers.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file tod_firmware_registers.h
/// \brief C register structs for the TOD unit

// *** WARNING *** - This file is generated automatically, do not edit.

#ifndef SIXTYFOUR_BIT_CONSTANT
#ifdef __ASSEMBLER__
#define SIXTYFOUR_BIT_CONSTANT(x) x
#else
#define SIXTYFOUR_BIT_CONSTANT(x) x##ull
#endif
#endif

#ifndef __ASSEMBLER__

#include <stdint.h>




typedef union tod_value_reg {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t tod_incrementer : 60;
    uint64_t tod_wof : 4;
#else
    uint64_t tod_wof : 4;
    uint64_t tod_incrementer : 60;
#endif // _BIG_ENDIAN
    } fields;
} tod_value_reg_t;


#endif // __ASSEMBLER__
#endif // __TOD_FIRMWARE_REGISTERS_H__

