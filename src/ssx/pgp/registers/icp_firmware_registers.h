/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/registers/icp_firmware_registers.h $              */
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
#ifndef __ICP_FIRMWARE_REGISTERS_H__
#define __ICP_FIRMWARE_REGISTERS_H__

/// \file icp_firmware_registers.h
/// \brief C register structs for the ICP unit

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




typedef union icp_bar {

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
    uint64_t icp_bar : 30;
    uint64_t icp_bar_en : 1;
    uint64_t _reserved0 : 33;
#else
    uint64_t _reserved0 : 33;
    uint64_t icp_bar_en : 1;
    uint64_t icp_bar : 30;
#endif // _BIG_ENDIAN
    } fields;
} icp_bar_t;

#endif // __ASSEMBLER__
#define ICP_BAR_ICP_BAR_MASK SIXTYFOUR_BIT_CONSTANT(0xfffffffc00000000)
#define ICP_BAR_ICP_BAR_EN SIXTYFOUR_BIT_CONSTANT(0x0000000200000000)
#ifndef __ASSEMBLER__


typedef union icp_mode0 {

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
    uint64_t priority : 8;
    uint64_t reserved0 : 1;
    uint64_t scope_initial : 3;
    uint64_t reserved1 : 1;
    uint64_t no_hang2status : 1;
    uint64_t oper_disable_hang : 1;
    uint64_t oper_hang_div : 5;
    uint64_t reserved2 : 2;
    uint64_t data_disable_hang : 1;
    uint64_t data_hang_div : 5;
    uint64_t backoff_disable : 1;
    uint64_t fwd_que_fwd_conv_disable : 1;
    uint64_t disa_wait4cresp_mode4ris : 1;
    uint64_t disa_auto_no_retry4ris : 1;
    uint64_t disa_retry_mode4ris : 1;
    uint64_t hang_on_addr_error : 1;
    uint64_t eoi_correction : 2;
    uint64_t max_load_count : 4;
    uint64_t max_store_count : 4;
    uint64_t reserved3 : 3;
    uint64_t enable_inject : 1;
    uint64_t _reserved0 : 16;
#else
    uint64_t _reserved0 : 16;
    uint64_t enable_inject : 1;
    uint64_t reserved3 : 3;
    uint64_t max_store_count : 4;
    uint64_t max_load_count : 4;
    uint64_t eoi_correction : 2;
    uint64_t hang_on_addr_error : 1;
    uint64_t disa_retry_mode4ris : 1;
    uint64_t disa_auto_no_retry4ris : 1;
    uint64_t disa_wait4cresp_mode4ris : 1;
    uint64_t fwd_que_fwd_conv_disable : 1;
    uint64_t backoff_disable : 1;
    uint64_t data_hang_div : 5;
    uint64_t data_disable_hang : 1;
    uint64_t reserved2 : 2;
    uint64_t oper_hang_div : 5;
    uint64_t oper_disable_hang : 1;
    uint64_t no_hang2status : 1;
    uint64_t reserved1 : 1;
    uint64_t scope_initial : 3;
    uint64_t reserved0 : 1;
    uint64_t priority : 8;
#endif // _BIG_ENDIAN
    } fields;
} icp_mode0_t;

#endif // __ASSEMBLER__
#define ICP_MODE0_PRIORITY_MASK SIXTYFOUR_BIT_CONSTANT(0xff00000000000000)
#define ICP_MODE0_SCOPE_INITIAL_MASK SIXTYFOUR_BIT_CONSTANT(0x0070000000000000)
#define ICP_MODE0_NO_HANG2STATUS SIXTYFOUR_BIT_CONSTANT(0x0004000000000000)
#define ICP_MODE0_OPER_DISABLE_HANG SIXTYFOUR_BIT_CONSTANT(0x0002000000000000)
#define ICP_MODE0_OPER_HANG_DIV_MASK SIXTYFOUR_BIT_CONSTANT(0x0001f00000000000)
#define ICP_MODE0_DATA_DISABLE_HANG SIXTYFOUR_BIT_CONSTANT(0x0000020000000000)
#define ICP_MODE0_DATA_HANG_DIV_MASK SIXTYFOUR_BIT_CONSTANT(0x000001f000000000)
#define ICP_MODE0_BACKOFF_DISABLE SIXTYFOUR_BIT_CONSTANT(0x0000000800000000)
#define ICP_MODE0_FWD_QUE_FWD_CONV_DISABLE SIXTYFOUR_BIT_CONSTANT(0x0000000400000000)
#define ICP_MODE0_DISA_WAIT4CRESP_MODE4RIS SIXTYFOUR_BIT_CONSTANT(0x0000000200000000)
#define ICP_MODE0_DISA_AUTO_NO_RETRY4RIS SIXTYFOUR_BIT_CONSTANT(0x0000000100000000)
#define ICP_MODE0_DISA_RETRY_MODE4RIS SIXTYFOUR_BIT_CONSTANT(0x0000000080000000)
#define ICP_MODE0_HANG_ON_ADDR_ERROR SIXTYFOUR_BIT_CONSTANT(0x0000000040000000)
#define ICP_MODE0_EOI_CORRECTION_MASK SIXTYFOUR_BIT_CONSTANT(0x0000000030000000)
#define ICP_MODE0_MAX_LOAD_COUNT_MASK SIXTYFOUR_BIT_CONSTANT(0x000000000f000000)
#define ICP_MODE0_MAX_STORE_COUNT_MASK SIXTYFOUR_BIT_CONSTANT(0x0000000000f00000)
#define ICP_MODE0_ENABLE_INJECT SIXTYFOUR_BIT_CONSTANT(0x0000000000010000)
#ifndef __ASSEMBLER__


typedef union icp_iir {

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
    uint64_t inject_target_core : 16;
    uint64_t inject_target_thread : 8;
    uint64_t reserved0 : 8;
    uint64_t inject_level : 4;
    uint64_t reserved1 : 4;
    uint64_t inject_priority : 8;
    uint64_t _reserved0 : 16;
#else
    uint64_t _reserved0 : 16;
    uint64_t inject_priority : 8;
    uint64_t reserved1 : 4;
    uint64_t inject_level : 4;
    uint64_t reserved0 : 8;
    uint64_t inject_target_thread : 8;
    uint64_t inject_target_core : 16;
#endif // _BIG_ENDIAN
    } fields;
} icp_iir_t;

#endif // __ASSEMBLER__
#define ICP_IIR_INJECT_TARGET_CORE_MASK SIXTYFOUR_BIT_CONSTANT(0xffff000000000000)
#define ICP_IIR_INJECT_TARGET_THREAD_MASK SIXTYFOUR_BIT_CONSTANT(0x0000ff0000000000)
#define ICP_IIR_INJECT_LEVEL_MASK SIXTYFOUR_BIT_CONSTANT(0x00000000f0000000)
#define ICP_IIR_INJECT_PRIORITY_MASK SIXTYFOUR_BIT_CONSTANT(0x0000000000ff0000)
#ifndef __ASSEMBLER__

#endif // __ASSEMBLER__
#endif // __ICP_FIRMWARE_REGISTERS_H__

