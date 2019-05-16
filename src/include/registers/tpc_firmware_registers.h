/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/tpc_firmware_registers.h $              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2019                        */
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
#ifndef __TPC_FIRMWARE_REGISTERS_H__
#define __TPC_FIRMWARE_REGISTERS_H__

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




typedef union tpc_perv_ctrl0
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t tp_chiplet_chiplet_en_dc : 1;
        uint64_t put_in_later0 : 24;
    uint64_t tp_chiplet_fence_pcb_dc : 1;
        uint64_t put_in_later1 : 6;
        uint64_t reserved : 32;
#else
        uint64_t reserved : 32;
        uint64_t put_in_later1 : 6;
    uint64_t tp_chiplet_fence_pcb_dc : 1;
        uint64_t put_in_later0 : 24;
    uint64_t tp_chiplet_chiplet_en_dc : 1;
#endif // _BIG_ENDIAN
    } fields;
} tpc_perv_ctrl0_t;



typedef union tpc_cplt_conf0
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t put_in_later0 : 48;
        uint64_t tc_unit_group_id_dc : 4;
        uint64_t tc_unit_chip_id_dc : 3;
        uint64_t put_in_later1 : 9;
#else
        uint64_t put_in_later1 : 9;
        uint64_t tc_unit_chip_id_dc : 3;
        uint64_t tc_unit_group_id_dc : 4;
        uint64_t put_in_later0 : 48;
#endif // _BIG_ENDIAN
    } fields;
} tpc_cplt_conf0_t;



typedef union tpc_cplt_conf0_and
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t put_in_later0 : 48;
        uint64_t tc_unit_group_id_dc : 4;
        uint64_t tc_unit_chip_id_dc : 3;
        uint64_t put_in_later1 : 9;
#else
        uint64_t put_in_later1 : 9;
        uint64_t tc_unit_chip_id_dc : 3;
        uint64_t tc_unit_group_id_dc : 4;
        uint64_t put_in_later0 : 48;
#endif // _BIG_ENDIAN
    } fields;
} tpc_cplt_conf0_and_t;



typedef union tpc_cplt_conf0_or
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t put_in_later0 : 48;
        uint64_t tc_unit_group_id_dc : 4;
        uint64_t tc_unit_chip_id_dc : 3;
        uint64_t put_in_later1 : 9;
#else
        uint64_t put_in_later1 : 9;
        uint64_t tc_unit_chip_id_dc : 3;
        uint64_t tc_unit_group_id_dc : 4;
        uint64_t put_in_later0 : 48;
#endif // _BIG_ENDIAN
    } fields;
} tpc_cplt_conf0_or_t;



typedef union tpc_hpr2
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
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



typedef union tpc_device_id
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t cfam_id : 32;
        uint64_t constant : 4;
        uint64_t socket_id : 3;
        uint64_t chippos_id : 1;
        uint64_t io_tp_vsb_chip_pos : 1;
        uint64_t constant1 : 7;
    uint64_t fuse_nx_allow_crypto : 1;
    uint64_t fuse_vmx_crypto_dis : 1;
    uint64_t fuse_fp_throttle_en : 1;
        uint64_t tp_np_nvlink_disable : 1;
        uint64_t fuse_topology_2chip : 1;
        uint64_t fuse_topology_group : 2;
        uint64_t constant2 : 9;
#else
        uint64_t constant2 : 9;
        uint64_t fuse_topology_group : 2;
        uint64_t fuse_topology_2chip : 1;
        uint64_t tp_np_nvlink_disable : 1;
    uint64_t fuse_fp_throttle_en : 1;
    uint64_t fuse_vmx_crypto_dis : 1;
    uint64_t fuse_nx_allow_crypto : 1;
        uint64_t constant1 : 7;
        uint64_t io_tp_vsb_chip_pos : 1;
        uint64_t chippos_id : 1;
        uint64_t socket_id : 3;
        uint64_t constant : 4;
    uint64_t cfam_id : 32;
#endif // _BIG_ENDIAN
    } fields;
} tpc_device_id_t;


#endif // __ASSEMBLER__
#endif // __TPC_FIRMWARE_REGISTERS_H__

