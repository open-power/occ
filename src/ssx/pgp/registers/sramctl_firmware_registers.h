#ifndef __SRAMCTL_FIRMWARE_REGISTERS_H__
#define __SRAMCTL_FIRMWARE_REGISTERS_H__

// $Id: sramctl_firmware_registers.h,v 1.1.1.1 2013/12/11 21:03:24 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/registers/sramctl_firmware_registers.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file sramctl_firmware_registers.h
/// \brief C register structs for the SRAMCTL unit

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




typedef union sramctl_srbar {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t sram_region_qualifier : 2;
    uint32_t reserved : 3;
    uint32_t sram_bar_region : 8;
    uint32_t _reserved0 : 19;
#else
    uint32_t _reserved0 : 19;
    uint32_t sram_bar_region : 8;
    uint32_t reserved : 3;
    uint32_t sram_region_qualifier : 2;
#endif // _BIG_ENDIAN
    } fields;
} sramctl_srbar_t;



typedef union sramctl_srmr {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t sram_enable_remap : 1;
    uint32_t sram_arb_en_send_all_writes : 1;
    uint32_t sram_disable_lfsr : 1;
    uint32_t sram_lfsr_fairness_mask : 5;
    uint32_t sram_error_inject_enable : 1;
    uint32_t sram_ctl_trace_en : 1;
    uint32_t sram_ctl_trace_sel : 1;
    uint32_t reserved : 5;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t reserved : 5;
    uint32_t sram_ctl_trace_sel : 1;
    uint32_t sram_ctl_trace_en : 1;
    uint32_t sram_error_inject_enable : 1;
    uint32_t sram_lfsr_fairness_mask : 5;
    uint32_t sram_disable_lfsr : 1;
    uint32_t sram_arb_en_send_all_writes : 1;
    uint32_t sram_enable_remap : 1;
#endif // _BIG_ENDIAN
    } fields;
} sramctl_srmr_t;



typedef union sramctl_srmap {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t reserved : 1;
    uint32_t sram_remap_source : 12;
    uint32_t _reserved0 : 1;
    uint32_t reserved1 : 3;
    uint32_t sram_remap_dest : 13;
    uint32_t reserved2 : 2;
#else
    uint32_t reserved2 : 2;
    uint32_t sram_remap_dest : 13;
    uint32_t reserved1 : 3;
    uint32_t _reserved0 : 1;
    uint32_t sram_remap_source : 12;
    uint32_t reserved : 1;
#endif // _BIG_ENDIAN
    } fields;
} sramctl_srmap_t;



typedef union sramctl_srear {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t sram_error_address : 16;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t sram_error_address : 16;
#endif // _BIG_ENDIAN
    } fields;
} sramctl_srear_t;



typedef union sramctl_srbv0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t boot_vector_word0 : 32;
#else
    uint32_t boot_vector_word0 : 32;
#endif // _BIG_ENDIAN
    } fields;
} sramctl_srbv0_t;



typedef union sramctl_srbv1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t boot_vector_word1 : 32;
#else
    uint32_t boot_vector_word1 : 32;
#endif // _BIG_ENDIAN
    } fields;
} sramctl_srbv1_t;



typedef union sramctl_srbv2 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t boot_vector_word2 : 32;
#else
    uint32_t boot_vector_word2 : 32;
#endif // _BIG_ENDIAN
    } fields;
} sramctl_srbv2_t;



typedef union sramctl_srbv3 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t boot_vector_word3 : 32;
#else
    uint32_t boot_vector_word3 : 32;
#endif // _BIG_ENDIAN
    } fields;
} sramctl_srbv3_t;



typedef union sramctl_srchsw {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t chksw_wrfsm_dly_dis : 1;
    uint32_t chksw_allow1_rd : 1;
    uint32_t chksw_allow1_wr : 1;
    uint32_t chksw_allow1_rdwr : 1;
    uint32_t chksw_oci_parchk_dis : 1;
    uint32_t chksw_tank_rddata_parchk_dis : 1;
    uint32_t chksw_tank_sr_rderr_dis : 1;
    uint32_t chksw_val_be_addr_chk_dis : 1;
    uint32_t chksw_so_spare : 2;
    uint32_t _reserved0 : 22;
#else
    uint32_t _reserved0 : 22;
    uint32_t chksw_so_spare : 2;
    uint32_t chksw_val_be_addr_chk_dis : 1;
    uint32_t chksw_tank_sr_rderr_dis : 1;
    uint32_t chksw_tank_rddata_parchk_dis : 1;
    uint32_t chksw_oci_parchk_dis : 1;
    uint32_t chksw_allow1_rdwr : 1;
    uint32_t chksw_allow1_wr : 1;
    uint32_t chksw_allow1_rd : 1;
    uint32_t chksw_wrfsm_dly_dis : 1;
#endif // _BIG_ENDIAN
    } fields;
} sramctl_srchsw_t;


#endif // __ASSEMBLER__
#endif // __SRAMCTL_FIRMWARE_REGISTERS_H__

