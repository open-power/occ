#ifndef __TPC_FIRMWARE_REGISTERS_H__
#define __TPC_FIRMWARE_REGISTERS_H__

// $Id: tpc_firmware_registers.h,v 1.1.1.1 2013/12/11 21:03:25 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/registers/tpc_firmware_registers.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file tpc_firmware_registers.h
/// \brief C register structs for the TPC unit

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




typedef union tpc_perv_gp3 {

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
    uint64_t tp_chiplet_chiplet_en_dc : 1;
    uint64_t put_in_later0 : 25;
    uint64_t tp_chiplet_fence_pcb_dc : 1;
    uint64_t put_in_later1 : 37;
#else
    uint64_t put_in_later1 : 37;
    uint64_t tp_chiplet_fence_pcb_dc : 1;
    uint64_t put_in_later0 : 25;
    uint64_t tp_chiplet_chiplet_en_dc : 1;
#endif // _BIG_ENDIAN
    } fields;
} tpc_perv_gp3_t;



typedef union tpc_gp0 {

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
    uint64_t put_in_later0 : 40;
    uint64_t tc_node_id_dc : 3;
    uint64_t tc_chip_id_dc : 3;
    uint64_t put_in_later1 : 18;
#else
    uint64_t put_in_later1 : 18;
    uint64_t tc_chip_id_dc : 3;
    uint64_t tc_node_id_dc : 3;
    uint64_t put_in_later0 : 40;
#endif // _BIG_ENDIAN
    } fields;
} tpc_gp0_t;



typedef union tpc_gp0_and {

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
    uint64_t put_in_later0 : 40;
    uint64_t tc_node_id_dc : 3;
    uint64_t tc_chip_id_dc : 3;
    uint64_t put_in_later1 : 18;
#else
    uint64_t put_in_later1 : 18;
    uint64_t tc_chip_id_dc : 3;
    uint64_t tc_node_id_dc : 3;
    uint64_t put_in_later0 : 40;
#endif // _BIG_ENDIAN
    } fields;
} tpc_gp0_and_t;



typedef union tpc_gp0_or {

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
    uint64_t put_in_later0 : 40;
    uint64_t tc_node_id_dc : 3;
    uint64_t tc_chip_id_dc : 3;
    uint64_t put_in_later1 : 18;
#else
    uint64_t put_in_later1 : 18;
    uint64_t tc_chip_id_dc : 3;
    uint64_t tc_node_id_dc : 3;
    uint64_t put_in_later0 : 40;
#endif // _BIG_ENDIAN
    } fields;
} tpc_gp0_or_t;



typedef union tpc_hpr2 {

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
    uint64_t hang_pulse_reg : 6;
    uint64_t suppress_hang : 1;
    uint64_t _reserved0 : 57;
#else
    uint64_t _reserved0 : 57;
    uint64_t suppress_hang : 1;
    uint64_t hang_pulse_reg : 6;
#endif // _BIG_ENDIAN
    } fields;
} tpc_hpr2_t;



typedef union tpc_device_id {

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
    uint64_t cfam_id : 32;
    uint64_t fuse_nx_allow_crypto : 1;
    uint64_t fuse_vmx_crypto_dis : 1;
    uint64_t fuse_fp_throttle_en : 1;
    uint64_t reserved32 : 1;
    uint64_t socket_id : 3;
    uint64_t chippos_id : 1;
    uint64_t _reserved0 : 24;
#else
    uint64_t _reserved0 : 24;
    uint64_t chippos_id : 1;
    uint64_t socket_id : 3;
    uint64_t reserved32 : 1;
    uint64_t fuse_fp_throttle_en : 1;
    uint64_t fuse_vmx_crypto_dis : 1;
    uint64_t fuse_nx_allow_crypto : 1;
    uint64_t cfam_id : 32;
#endif // _BIG_ENDIAN
    } fields;
} tpc_device_id_t;


#endif // __ASSEMBLER__
#endif // __TPC_FIRMWARE_REGISTERS_H__

