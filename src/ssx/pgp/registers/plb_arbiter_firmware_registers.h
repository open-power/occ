#ifndef __PLB_ARBITER_FIRMWARE_REGISTERS_H__
#define __PLB_ARBITER_FIRMWARE_REGISTERS_H__

// $Id: plb_arbiter_firmware_registers.h,v 1.1.1.1 2013/12/11 21:03:25 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/registers/plb_arbiter_firmware_registers.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file plb_arbiter_firmware_registers.h
/// \brief C register structs for the PLB_ARBITER unit

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




typedef union plb_prev {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} plb_prev_t;



typedef union plb_pacr {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t ppm : 1;
    uint32_t ppo : 3;
    uint32_t hbu : 1;
    uint32_t rdp : 2;
    uint32_t wrp : 1;
    uint32_t _reserved0 : 24;
#else
    uint32_t _reserved0 : 24;
    uint32_t wrp : 1;
    uint32_t rdp : 2;
    uint32_t hbu : 1;
    uint32_t ppo : 3;
    uint32_t ppm : 1;
#endif // _BIG_ENDIAN
    } fields;
} plb_pacr_t;



typedef union plb_pesr {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t pte0 : 1;
    uint32_t rw0 : 1;
    uint32_t flk0 : 1;
    uint32_t alk0 : 1;
    uint32_t pte1 : 1;
    uint32_t rw1 : 1;
    uint32_t flk1 : 1;
    uint32_t alk1 : 1;
    uint32_t pte2 : 1;
    uint32_t rw2 : 1;
    uint32_t flk2 : 1;
    uint32_t alk2 : 1;
    uint32_t pte3 : 1;
    uint32_t rw3 : 1;
    uint32_t flk3 : 1;
    uint32_t alk3 : 1;
    uint32_t pte4 : 1;
    uint32_t rw4 : 1;
    uint32_t flk4 : 1;
    uint32_t alk4 : 1;
    uint32_t pte5 : 1;
    uint32_t rw5 : 1;
    uint32_t flk5 : 1;
    uint32_t alk5 : 1;
    uint32_t pte6 : 1;
    uint32_t rw6 : 1;
    uint32_t flk6 : 1;
    uint32_t alk6 : 1;
    uint32_t pte7 : 1;
    uint32_t rw7 : 1;
    uint32_t flk7 : 1;
    uint32_t alk7 : 1;
#else
    uint32_t alk7 : 1;
    uint32_t flk7 : 1;
    uint32_t rw7 : 1;
    uint32_t pte7 : 1;
    uint32_t alk6 : 1;
    uint32_t flk6 : 1;
    uint32_t rw6 : 1;
    uint32_t pte6 : 1;
    uint32_t alk5 : 1;
    uint32_t flk5 : 1;
    uint32_t rw5 : 1;
    uint32_t pte5 : 1;
    uint32_t alk4 : 1;
    uint32_t flk4 : 1;
    uint32_t rw4 : 1;
    uint32_t pte4 : 1;
    uint32_t alk3 : 1;
    uint32_t flk3 : 1;
    uint32_t rw3 : 1;
    uint32_t pte3 : 1;
    uint32_t alk2 : 1;
    uint32_t flk2 : 1;
    uint32_t rw2 : 1;
    uint32_t pte2 : 1;
    uint32_t alk1 : 1;
    uint32_t flk1 : 1;
    uint32_t rw1 : 1;
    uint32_t pte1 : 1;
    uint32_t alk0 : 1;
    uint32_t flk0 : 1;
    uint32_t rw0 : 1;
    uint32_t pte0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} plb_pesr_t;



typedef union plb_pearl {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} plb_pearl_t;



typedef union plb_pearh {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} plb_pearh_t;



typedef union plb_sto_pesr {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t icu_te : 1;
    uint32_t icu_rw : 1;
    uint32_t reserved2 : 2;
    uint32_t dcu_te : 1;
    uint32_t dcu_rw : 1;
    uint32_t reserved6 : 2;
    uint32_t _reserved0 : 24;
#else
    uint32_t _reserved0 : 24;
    uint32_t reserved6 : 2;
    uint32_t dcu_rw : 1;
    uint32_t dcu_te : 1;
    uint32_t reserved2 : 2;
    uint32_t icu_rw : 1;
    uint32_t icu_te : 1;
#endif // _BIG_ENDIAN
    } fields;
} plb_sto_pesr_t;



typedef union plb_sto_pear {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} plb_sto_pear_t;


#endif // __ASSEMBLER__
#endif // __PLB_ARBITER_FIRMWARE_REGISTERS_H__

