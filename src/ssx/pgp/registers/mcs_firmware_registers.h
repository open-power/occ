/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/registers/mcs_firmware_registers.h $              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2015                        */
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
#ifndef __MCS_FIRMWARE_REGISTERS_H__
#define __MCS_FIRMWARE_REGISTERS_H__

// $Id: mcs_firmware_registers.h,v 1.4 2015/01/27 17:56:30 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/registers/mcs_firmware_registers.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file mcs_firmware_registers.h
/// \brief C register structs for the MCS unit

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




typedef union mcfgpr {

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
    uint64_t mcfgprq_valid : 1;
    uint64_t reserved0 : 5;
    uint64_t mcfgprq_base_address : 14;
    uint64_t _reserved0 : 44;
#else
    uint64_t _reserved0 : 44;
    uint64_t mcfgprq_base_address : 14;
    uint64_t reserved0 : 5;
    uint64_t mcfgprq_valid : 1;
#endif // _BIG_ENDIAN
    } fields;
} mcfgpr_t;

#endif // __ASSEMBLER__
#define MCFGPR_MCFGPRQ_VALID SIXTYFOUR_BIT_CONSTANT(0x8000000000000000)
#define MCFGPR_MCFGPRQ_BASE_ADDRESS_MASK SIXTYFOUR_BIT_CONSTANT(0x03fff00000000000)
#ifndef __ASSEMBLER__


typedef union mcsmode0 {

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
    uint64_t enable_cmd_byp_stutter : 1;
    uint64_t reserved1 : 1;
    uint64_t enable_ns_rd_ao_sfu_for_dcbz : 1;
    uint64_t enable_centaur_local_checkstop_command : 1;
    uint64_t l3_prefetch_retry_threshold : 4;
    uint64_t number_of_cl_entries_reserved_for_read : 4;
    uint64_t number_of_cl_entries_reserved_for_mirrored_ops : 4;
    uint64_t number_of_cl_entries_reserved_for_writes : 4;
    uint64_t number_of_cl_entries_reserved_for_cp_writes : 4;
    uint64_t number_of_cl_entries_reserved_for_cp_ig : 4;
    uint64_t number_of_cl_entries_reserved_for_htm_ops : 4;
    uint64_t number_of_cl_entries_reserved_for_ha_assist : 4;
    uint64_t mcfgrp_19_is_ho_bit : 1;
    uint64_t cl_channel_timeout_forces_channel_fail : 1;
    uint64_t enable_fault_line_for_global_checkstop : 1;
    uint64_t reserved39 : 5;
    uint64_t address_collision_modes : 9;
    uint64_t include_cp_ig_in_cp_write_fullness_group : 1;
    uint64_t enable_dmawr_cmd_bit : 1;
    uint64_t enable_read_lsfr_data : 1;
    uint64_t force_channel_fail : 1;
    uint64_t disable_read_crc_ecc_bypass_taken : 1;
    uint64_t disable_cl_ao_queueus : 1;
    uint64_t address_select_lfsr_value : 2;
    uint64_t enable_centaur_sync : 1;
    uint64_t write_data_buffer_ecc_check_disable : 1;
    uint64_t write_data_buffer_ecc_correct_disable : 1;
#else
    uint64_t write_data_buffer_ecc_correct_disable : 1;
    uint64_t write_data_buffer_ecc_check_disable : 1;
    uint64_t enable_centaur_sync : 1;
    uint64_t address_select_lfsr_value : 2;
    uint64_t disable_cl_ao_queueus : 1;
    uint64_t disable_read_crc_ecc_bypass_taken : 1;
    uint64_t force_channel_fail : 1;
    uint64_t enable_read_lsfr_data : 1;
    uint64_t enable_dmawr_cmd_bit : 1;
    uint64_t include_cp_ig_in_cp_write_fullness_group : 1;
    uint64_t address_collision_modes : 9;
    uint64_t reserved39 : 5;
    uint64_t enable_fault_line_for_global_checkstop : 1;
    uint64_t cl_channel_timeout_forces_channel_fail : 1;
    uint64_t mcfgrp_19_is_ho_bit : 1;
    uint64_t number_of_cl_entries_reserved_for_ha_assist : 4;
    uint64_t number_of_cl_entries_reserved_for_htm_ops : 4;
    uint64_t number_of_cl_entries_reserved_for_cp_ig : 4;
    uint64_t number_of_cl_entries_reserved_for_cp_writes : 4;
    uint64_t number_of_cl_entries_reserved_for_writes : 4;
    uint64_t number_of_cl_entries_reserved_for_mirrored_ops : 4;
    uint64_t number_of_cl_entries_reserved_for_read : 4;
    uint64_t l3_prefetch_retry_threshold : 4;
    uint64_t enable_centaur_local_checkstop_command : 1;
    uint64_t enable_ns_rd_ao_sfu_for_dcbz : 1;
    uint64_t reserved1 : 1;
    uint64_t enable_cmd_byp_stutter : 1;
#endif // _BIG_ENDIAN
    } fields;
} mcsmode0_t;

#endif // __ASSEMBLER__
#define MCSMODE0_ENABLE_CMD_BYP_STUTTER SIXTYFOUR_BIT_CONSTANT(0x8000000000000000)
#define MCSMODE0_ENABLE_NS_RD_AO_SFU_FOR_DCBZ SIXTYFOUR_BIT_CONSTANT(0x2000000000000000)
#define MCSMODE0_ENABLE_CENTAUR_LOCAL_CHECKSTOP_COMMAND SIXTYFOUR_BIT_CONSTANT(0x1000000000000000)
#define MCSMODE0_L3_PREFETCH_RETRY_THRESHOLD_MASK SIXTYFOUR_BIT_CONSTANT(0x0f00000000000000)
#define MCSMODE0_MCFGRP_19_IS_HO_BIT SIXTYFOUR_BIT_CONSTANT(0x0000000008000000)
#define MCSMODE0_CL_CHANNEL_TIMEOUT_FORCES_CHANNEL_FAIL SIXTYFOUR_BIT_CONSTANT(0x0000000004000000)
#define MCSMODE0_ENABLE_FAULT_LINE_FOR_GLOBAL_CHECKSTOP SIXTYFOUR_BIT_CONSTANT(0x0000000002000000)
#define MCSMODE0_ADDRESS_COLLISION_MODES_MASK SIXTYFOUR_BIT_CONSTANT(0x00000000000ff800)
#define MCSMODE0_INCLUDE_CP_IG_IN_CP_WRITE_FULLNESS_GROUP SIXTYFOUR_BIT_CONSTANT(0x0000000000000400)
#define MCSMODE0_ENABLE_DMAWR_CMD_BIT SIXTYFOUR_BIT_CONSTANT(0x0000000000000200)
#define MCSMODE0_ENABLE_READ_LSFR_DATA SIXTYFOUR_BIT_CONSTANT(0x0000000000000100)
#define MCSMODE0_FORCE_CHANNEL_FAIL SIXTYFOUR_BIT_CONSTANT(0x0000000000000080)
#define MCSMODE0_DISABLE_READ_CRC_ECC_BYPASS_TAKEN SIXTYFOUR_BIT_CONSTANT(0x0000000000000040)
#define MCSMODE0_DISABLE_CL_AO_QUEUEUS SIXTYFOUR_BIT_CONSTANT(0x0000000000000020)
#define MCSMODE0_ADDRESS_SELECT_LFSR_VALUE_MASK SIXTYFOUR_BIT_CONSTANT(0x0000000000000018)
#define MCSMODE0_ENABLE_CENTAUR_SYNC SIXTYFOUR_BIT_CONSTANT(0x0000000000000004)
#define MCSMODE0_WRITE_DATA_BUFFER_ECC_CHECK_DISABLE SIXTYFOUR_BIT_CONSTANT(0x0000000000000002)
#define MCSMODE0_WRITE_DATA_BUFFER_ECC_CORRECT_DISABLE SIXTYFOUR_BIT_CONSTANT(0x0000000000000001)
#ifndef __ASSEMBLER__

typedef union mcifir {

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
    uint64_t _reserved0 : 30;
    uint64_t channel_fail_signal_active : 1;
    uint64_t _reserved1 : 33;
#else
    uint64_t _reserved1 : 33;
    uint64_t channel_fail_signal_active : 1;
    uint64_t _reserved0 : 30;
#endif // _BIG_ENDIAN
    } fields;
} mcifir_t;

#endif // __ASSEMBLER__
#endif // __MCS_FIRMWARE_REGISTERS_H__

