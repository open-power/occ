/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/registers/pibmem_firmware_registers.h $           */
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
#ifndef __PIBMEM_FIRMWARE_REGISTERS_H__
#define __PIBMEM_FIRMWARE_REGISTERS_H__

/// \file pibmem_firmware_registers.h
/// \brief C register structs for the PIBMEM unit

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




typedef union pibmem_data0 {

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
    uint64_t value : 64;
#else
    uint64_t value : 64;
#endif // _BIG_ENDIAN
    } fields;
} pibmem_data0_t;



typedef union pibmem_control {

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
    uint64_t auto_pre_increment : 1;
    uint64_t auto_post_decrement : 1;
    uint64_t _reserved0 : 62;
#else
    uint64_t _reserved0 : 62;
    uint64_t auto_post_decrement : 1;
    uint64_t auto_pre_increment : 1;
#endif // _BIG_ENDIAN
    } fields;
} pibmem_control_t;



typedef union pibmem_address {

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
    uint64_t reserved0 : 48;
    uint64_t address : 16;
#else
    uint64_t address : 16;
    uint64_t reserved0 : 48;
#endif // _BIG_ENDIAN
    } fields;
} pibmem_address_t;



typedef union pibmem_data {

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
    uint64_t value : 64;
#else
    uint64_t value : 64;
#endif // _BIG_ENDIAN
    } fields;
} pibmem_data_t;



typedef union pibmem_data_inc {

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
    uint64_t value : 64;
#else
    uint64_t value : 64;
#endif // _BIG_ENDIAN
    } fields;
} pibmem_data_inc_t;



typedef union pibmem_data_dec {

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
    uint64_t value : 64;
#else
    uint64_t value : 64;
#endif // _BIG_ENDIAN
    } fields;
} pibmem_data_dec_t;



typedef union pibmem_status {

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
    uint64_t addr_invalid : 1;
    uint64_t write_invalid : 1;
    uint64_t read_invalid : 1;
    uint64_t ecc_uncorrected_error : 1;
    uint64_t ecc_corrected_error : 1;
    uint64_t bad_array_address : 1;
    uint64_t reserved6 : 5;
    uint64_t fsm_present_state : 7;
    uint64_t _reserved0 : 46;
#else
    uint64_t _reserved0 : 46;
    uint64_t fsm_present_state : 7;
    uint64_t reserved6 : 5;
    uint64_t bad_array_address : 1;
    uint64_t ecc_corrected_error : 1;
    uint64_t ecc_uncorrected_error : 1;
    uint64_t read_invalid : 1;
    uint64_t write_invalid : 1;
    uint64_t addr_invalid : 1;
#endif // _BIG_ENDIAN
    } fields;
} pibmem_status_t;



typedef union pibmem_reset {

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
    uint64_t reset_code : 2;
    uint64_t _reserved0 : 62;
#else
    uint64_t _reserved0 : 62;
    uint64_t reset_code : 2;
#endif // _BIG_ENDIAN
    } fields;
} pibmem_reset_t;



typedef union pibmem_repair {

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
    uint64_t value : 64;
#else
    uint64_t value : 64;
#endif // _BIG_ENDIAN
    } fields;
} pibmem_repair_t;


#endif // __ASSEMBLER__
#endif // __PIBMEM_FIRMWARE_REGISTERS_H__

