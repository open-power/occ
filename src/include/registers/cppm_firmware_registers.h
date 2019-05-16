/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/cppm_firmware_registers.h $             */
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
#ifndef __CPPM_FIRMWARE_REGISTERS_H__
#define __CPPM_FIRMWARE_REGISTERS_H__

/// \file cppm_firmware_registers.h
/// \brief C register structs for the CPPM unit

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




typedef union cppm_cpmmr
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
    uint64_t ppm_write_disable : 1;
    uint64_t ppm_write_override : 1;
        uint64_t reserved1 : 7;
    uint64_t fused_core_mode : 1;
        uint64_t stop_exit_type_sel : 1;
        uint64_t block_intr_inputs : 1;
    uint64_t cme_err_notify_dis : 1;
    uint64_t wkup_notify_select : 1;
    uint64_t enable_pece : 1;
    uint64_t cme_special_wkup_done_dis : 1;
    uint64_t reserved2 : 48;
#else
    uint64_t reserved2 : 48;
    uint64_t cme_special_wkup_done_dis : 1;
    uint64_t enable_pece : 1;
    uint64_t wkup_notify_select : 1;
    uint64_t cme_err_notify_dis : 1;
        uint64_t block_intr_inputs : 1;
        uint64_t stop_exit_type_sel : 1;
    uint64_t fused_core_mode : 1;
        uint64_t reserved1 : 7;
    uint64_t ppm_write_override : 1;
    uint64_t ppm_write_disable : 1;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cpmmr_t;



typedef union cppm_cpmmr_clr
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
    uint64_t ppm_write_disable : 1;
    uint64_t ppm_write_override : 1;
        uint64_t reserved1 : 7;
        uint64_t fused_core_mode : 1;
        uint64_t stop_exit_type_sel : 1;
        uint64_t block_intr_inputs : 1;
    uint64_t cme_err_notify_dis : 1;
    uint64_t wkup_notify_select : 1;
    uint64_t enable_pece : 1;
    uint64_t cme_special_wkup_done_dis : 1;
    uint64_t reserved2 : 48;
#else
    uint64_t reserved2 : 48;
    uint64_t cme_special_wkup_done_dis : 1;
    uint64_t enable_pece : 1;
    uint64_t wkup_notify_select : 1;
    uint64_t cme_err_notify_dis : 1;
        uint64_t block_intr_inputs : 1;
        uint64_t stop_exit_type_sel : 1;
        uint64_t fused_core_mode : 1;
        uint64_t reserved1 : 7;
    uint64_t ppm_write_override : 1;
    uint64_t ppm_write_disable : 1;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cpmmr_clr_t;



typedef union cppm_cpmmr_or
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
    uint64_t ppm_write_disable : 1;
    uint64_t ppm_write_override : 1;
        uint64_t reserved1 : 7;
        uint64_t fused_core_mode : 1;
        uint64_t stop_exit_type_sel : 1;
        uint64_t block_intr_inputs : 1;
    uint64_t cme_err_notify_dis : 1;
    uint64_t wkup_notify_select : 1;
    uint64_t enable_pece : 1;
    uint64_t cme_special_wkup_done_dis : 1;
    uint64_t reserved2 : 48;
#else
    uint64_t reserved2 : 48;
    uint64_t cme_special_wkup_done_dis : 1;
    uint64_t enable_pece : 1;
    uint64_t wkup_notify_select : 1;
    uint64_t cme_err_notify_dis : 1;
        uint64_t block_intr_inputs : 1;
        uint64_t stop_exit_type_sel : 1;
        uint64_t fused_core_mode : 1;
        uint64_t reserved1 : 7;
    uint64_t ppm_write_override : 1;
    uint64_t ppm_write_disable : 1;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cpmmr_or_t;



typedef union cppm_perrsum
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
    uint64_t cppm_error : 1;
    uint64_t reserved1 : 63;
#else
    uint64_t reserved1 : 63;
    uint64_t cppm_error : 1;
#endif // _BIG_ENDIAN
    } fields;
} cppm_perrsum_t;



typedef union cppm_err
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
    uint64_t pcb_interrupt_protocol_err : 1;
    uint64_t special_wkup_protocol_err : 1;
        uint64_t special_wkup_done_protocol_err : 1;
        uint64_t pfet_seq_program_err : 1;
    uint64_t clk_sync_err : 1;
    uint64_t pece_intr_disabled : 1;
    uint64_t deconfigured_intr : 1;
        uint64_t reserved1 : 1;
        uint64_t reserved_8_112 : 4;
        uint64_t reserved3 : 52;
#else
        uint64_t reserved3 : 52;
        uint64_t reserved_8_112 : 4;
        uint64_t reserved1 : 1;
    uint64_t deconfigured_intr : 1;
    uint64_t pece_intr_disabled : 1;
    uint64_t clk_sync_err : 1;
        uint64_t pfet_seq_program_err : 1;
        uint64_t special_wkup_done_protocol_err : 1;
    uint64_t special_wkup_protocol_err : 1;
    uint64_t pcb_interrupt_protocol_err : 1;
#endif // _BIG_ENDIAN
    } fields;
} cppm_err_t;



typedef union cppm_errmsk
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
        uint64_t reserved1 : 12;
        uint64_t reserved2 : 52;
#else
        uint64_t reserved2 : 52;
        uint64_t reserved1 : 12;
#endif // _BIG_ENDIAN
    } fields;
} cppm_errmsk_t;



typedef union cppm_nc0indir
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
    uint64_t ncn_indirect : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t ncn_indirect : 32;
#endif // _BIG_ENDIAN
    } fields;
} cppm_nc0indir_t;



typedef union cppm_nc0indir_clr
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
    uint64_t ncn_indirect : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t ncn_indirect : 32;
#endif // _BIG_ENDIAN
    } fields;
} cppm_nc0indir_clr_t;



typedef union cppm_nc0indir_or
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
    uint64_t ncn_indirect : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t ncn_indirect : 32;
#endif // _BIG_ENDIAN
    } fields;
} cppm_nc0indir_or_t;



typedef union cppm_nc1indir
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
    uint64_t ncn_indirect : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t ncn_indirect : 32;
#endif // _BIG_ENDIAN
    } fields;
} cppm_nc1indir_t;



typedef union cppm_nc1indir_clr
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
    uint64_t ncn_indirect : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t ncn_indirect : 32;
#endif // _BIG_ENDIAN
    } fields;
} cppm_nc1indir_clr_t;



typedef union cppm_nc1indir_or
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
    uint64_t ncn_indirect : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t ncn_indirect : 32;
#endif // _BIG_ENDIAN
    } fields;
} cppm_nc1indir_or_t;



typedef union cppm_csar
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
    uint64_t scratch_atomic_data : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t scratch_atomic_data : 32;
#endif // _BIG_ENDIAN
    } fields;
} cppm_csar_t;



typedef union cppm_csar_clr
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
    uint64_t scratch_atomic_data : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t scratch_atomic_data : 32;
#endif // _BIG_ENDIAN
    } fields;
} cppm_csar_clr_t;



typedef union cppm_csar_or
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
    uint64_t scratch_atomic_data : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t scratch_atomic_data : 32;
#endif // _BIG_ENDIAN
    } fields;
} cppm_csar_or_t;



typedef union cppm_caccr
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
    uint64_t clk_sb_strength : 4;
    uint64_t clk_sb_spare : 1;
    uint64_t clk_sb_pulse_mode_en : 1;
    uint64_t clk_sb_pulse_mode : 2;
    uint64_t clk_sw_resclk : 4;
    uint64_t clk_sw_spare : 1;
    uint64_t quad_clk_sb_override : 1;
    uint64_t quad_clk_sw_override : 1;
    uint64_t clk_sync_enable : 1;
    uint64_t reserved1 : 48;
#else
    uint64_t reserved1 : 48;
    uint64_t clk_sync_enable : 1;
    uint64_t quad_clk_sw_override : 1;
    uint64_t quad_clk_sb_override : 1;
    uint64_t clk_sw_spare : 1;
    uint64_t clk_sw_resclk : 4;
    uint64_t clk_sb_pulse_mode : 2;
    uint64_t clk_sb_pulse_mode_en : 1;
    uint64_t clk_sb_spare : 1;
    uint64_t clk_sb_strength : 4;
#endif // _BIG_ENDIAN
    } fields;
} cppm_caccr_t;



typedef union cppm_caccr_clr
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
    uint64_t clk_sb_strength : 4;
    uint64_t clk_sb_spare : 1;
    uint64_t clk_sb_pulse_mode_en : 1;
    uint64_t clk_sb_pulse_mode : 2;
    uint64_t clk_sw_resclk : 4;
    uint64_t clk_sw_spare : 1;
    uint64_t quad_clk_sb_override : 1;
    uint64_t quad_clk_sw_override : 1;
    uint64_t clk_sync_enable : 1;
    uint64_t reserved1 : 48;
#else
    uint64_t reserved1 : 48;
    uint64_t clk_sync_enable : 1;
    uint64_t quad_clk_sw_override : 1;
    uint64_t quad_clk_sb_override : 1;
    uint64_t clk_sw_spare : 1;
    uint64_t clk_sw_resclk : 4;
    uint64_t clk_sb_pulse_mode : 2;
    uint64_t clk_sb_pulse_mode_en : 1;
    uint64_t clk_sb_spare : 1;
    uint64_t clk_sb_strength : 4;
#endif // _BIG_ENDIAN
    } fields;
} cppm_caccr_clr_t;



typedef union cppm_caccr_or
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
    uint64_t clk_sb_strength : 4;
    uint64_t clk_sb_spare : 1;
    uint64_t clk_sb_pulse_mode_en : 1;
    uint64_t clk_sb_pulse_mode : 2;
    uint64_t clk_sw_resclk : 4;
    uint64_t clk_sw_spare : 1;
    uint64_t quad_clk_sb_override : 1;
    uint64_t quad_clk_sw_override : 1;
    uint64_t clk_sync_enable : 1;
    uint64_t reserved1 : 48;
#else
    uint64_t reserved1 : 48;
    uint64_t clk_sync_enable : 1;
    uint64_t quad_clk_sw_override : 1;
    uint64_t quad_clk_sb_override : 1;
    uint64_t clk_sw_spare : 1;
    uint64_t clk_sw_resclk : 4;
    uint64_t clk_sb_pulse_mode : 2;
    uint64_t clk_sb_pulse_mode_en : 1;
    uint64_t clk_sb_spare : 1;
    uint64_t clk_sb_strength : 4;
#endif // _BIG_ENDIAN
    } fields;
} cppm_caccr_or_t;



typedef union cppm_cacsr
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
    uint64_t actual_clk_sb_strength : 4;
    uint64_t actual_clk_sb_spare : 1;
    uint64_t actual_clk_sb_pulse_mode_en : 1;
    uint64_t actual_clk_sb_pulse_mode : 2;
    uint64_t actual_clk_sw_resclk : 4;
    uint64_t actual_clk_sw_spare : 1;
    uint64_t clk_sync_done : 1;
    uint64_t reserved1 : 50;
#else
    uint64_t reserved1 : 50;
    uint64_t clk_sync_done : 1;
    uint64_t actual_clk_sw_spare : 1;
    uint64_t actual_clk_sw_resclk : 4;
    uint64_t actual_clk_sb_pulse_mode : 2;
    uint64_t actual_clk_sb_pulse_mode_en : 1;
    uint64_t actual_clk_sb_spare : 1;
    uint64_t actual_clk_sb_strength : 4;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cacsr_t;



typedef union cppm_cmedb0
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
    uint64_t cme_message_number0 : 8;
    uint64_t cme_message_hi : 56;
#else
    uint64_t cme_message_hi : 56;
    uint64_t cme_message_number0 : 8;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cmedb0_t;



typedef union cppm_cmedb0_clr
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
    uint64_t cme_message_number0 : 8;
    uint64_t cme_message_hi : 56;
#else
    uint64_t cme_message_hi : 56;
    uint64_t cme_message_number0 : 8;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cmedb0_clr_t;



typedef union cppm_cmedb0_or
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
    uint64_t cme_message_number0 : 8;
    uint64_t cme_message_hi : 56;
#else
    uint64_t cme_message_hi : 56;
    uint64_t cme_message_number0 : 8;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cmedb0_or_t;



typedef union cppm_cmedb1
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
    uint64_t cme_message_numbern : 8;
    uint64_t reserved1 : 56;
#else
    uint64_t reserved1 : 56;
    uint64_t cme_message_numbern : 8;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cmedb1_t;



typedef union cppm_cmedb1_clr
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
    uint64_t cme_message_numbern : 8;
    uint64_t reserved1 : 56;
#else
    uint64_t reserved1 : 56;
    uint64_t cme_message_numbern : 8;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cmedb1_clr_t;



typedef union cppm_cmedb1_or
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
    uint64_t cme_message_numbern : 8;
    uint64_t reserved1 : 56;
#else
    uint64_t reserved1 : 56;
    uint64_t cme_message_numbern : 8;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cmedb1_or_t;



typedef union cppm_cmedb2
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
    uint64_t cme_message_numbern : 8;
    uint64_t reserved1 : 56;
#else
    uint64_t reserved1 : 56;
    uint64_t cme_message_numbern : 8;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cmedb2_t;



typedef union cppm_cmedb2_clr
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
    uint64_t cme_message_numbern : 8;
    uint64_t reserved1 : 56;
#else
    uint64_t reserved1 : 56;
    uint64_t cme_message_numbern : 8;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cmedb2_clr_t;



typedef union cppm_cmedb2_or
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
    uint64_t cme_message_numbern : 8;
    uint64_t reserved1 : 56;
#else
    uint64_t reserved1 : 56;
    uint64_t cme_message_numbern : 8;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cmedb2_or_t;



typedef union cppm_cmedb3
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
    uint64_t cme_message_numbern : 8;
    uint64_t reserved1 : 56;
#else
    uint64_t reserved1 : 56;
    uint64_t cme_message_numbern : 8;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cmedb3_t;



typedef union cppm_cmedb3_clr
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
    uint64_t cme_message_numbern : 8;
    uint64_t reserved1 : 56;
#else
    uint64_t reserved1 : 56;
    uint64_t cme_message_numbern : 8;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cmedb3_clr_t;



typedef union cppm_cmedb3_or
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
    uint64_t cme_message_numbern : 8;
    uint64_t reserved1 : 56;
#else
    uint64_t reserved1 : 56;
    uint64_t cme_message_numbern : 8;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cmedb3_or_t;



typedef union cppm_cmedata
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
    uint64_t data : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t data : 32;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cmedata_t;



typedef union cppm_cmedata_clr
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
    uint64_t data : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t data : 32;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cmedata_clr_t;



typedef union cppm_cmedata_or
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
    uint64_t data : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t data : 32;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cmedata_or_t;



typedef union cppm_cmemsg
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
    uint64_t cme_message : 64;
#else
    uint64_t cme_message : 64;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cmemsg_t;



typedef union cppm_ciir
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
        uint64_t reserved1 : 28;
        uint64_t msgsnd_intr_inject : 4;
        uint64_t reserved2 : 28;
        uint64_t msgsndu_intr_inject : 4;
#else
        uint64_t msgsndu_intr_inject : 4;
        uint64_t reserved2 : 28;
        uint64_t msgsnd_intr_inject : 4;
        uint64_t reserved1 : 28;
#endif // _BIG_ENDIAN
    } fields;
} cppm_ciir_t;



typedef union cppm_cisr
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
    uint64_t hyp_intr_present : 4;
    uint64_t os_intr_present : 4;
    uint64_t msgsnd_intr_present : 4;
    uint64_t ebb_intr_present : 4;
    uint64_t hyp_intr_requested : 4;
    uint64_t os_intr_requested : 4;
    uint64_t msgsnd_intr_requested : 4;
    uint64_t msgsnd_intr_sample : 4;
    uint64_t msgsnd_ack : 1;
    uint64_t malf_alert_present : 1;
    uint64_t malf_alert_requested : 1;
    uint64_t cme_special_wkup_done : 1;
        uint64_t msgsndu_intr_present : 4;
        uint64_t msgsndu_intr_requested : 4;
        uint64_t msgsndu_intr_sample : 4;
        uint64_t reserved1 : 16;
#else
        uint64_t reserved1 : 16;
        uint64_t msgsndu_intr_sample : 4;
        uint64_t msgsndu_intr_requested : 4;
        uint64_t msgsndu_intr_present : 4;
    uint64_t cme_special_wkup_done : 1;
    uint64_t malf_alert_requested : 1;
    uint64_t malf_alert_present : 1;
    uint64_t msgsnd_ack : 1;
    uint64_t msgsnd_intr_sample : 4;
    uint64_t msgsnd_intr_requested : 4;
    uint64_t os_intr_requested : 4;
    uint64_t hyp_intr_requested : 4;
    uint64_t ebb_intr_present : 4;
    uint64_t msgsnd_intr_present : 4;
    uint64_t os_intr_present : 4;
    uint64_t hyp_intr_present : 4;
#endif // _BIG_ENDIAN
    } fields;
} cppm_cisr_t;



typedef union cppm_peces
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
    uint64_t pece_t0 : 6;
    uint64_t reserved1 : 2;
    uint64_t pece_t1 : 6;
    uint64_t reserved2 : 2;
    uint64_t pece_t2 : 6;
    uint64_t reserved3 : 2;
    uint64_t pece_t3 : 6;
    uint64_t reserved4 : 2;
    uint64_t use_pece : 4;
        uint64_t spare : 1;
    uint64_t reserved5 : 27;
#else
    uint64_t reserved5 : 27;
        uint64_t spare : 1;
    uint64_t use_pece : 4;
    uint64_t reserved4 : 2;
    uint64_t pece_t3 : 6;
    uint64_t reserved3 : 2;
    uint64_t pece_t2 : 6;
    uint64_t reserved2 : 2;
    uint64_t pece_t1 : 6;
    uint64_t reserved1 : 2;
    uint64_t pece_t0 : 6;
#endif // _BIG_ENDIAN
    } fields;
} cppm_peces_t;



typedef union cppm_civrmlcr
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
    uint64_t ivrm_local_control : 1;
    uint64_t reserved_1_2 : 2;
    uint64_t ivrm_ureg_test_en : 1;
    uint64_t ivrm_ureg_test_id : 4;
    uint64_t reserved1 : 56;
#else
    uint64_t reserved1 : 56;
    uint64_t ivrm_ureg_test_id : 4;
    uint64_t ivrm_ureg_test_en : 1;
    uint64_t reserved_1_2 : 2;
    uint64_t ivrm_local_control : 1;
#endif // _BIG_ENDIAN
    } fields;
} cppm_civrmlcr_t;



typedef union cppm_ippmcmd
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
    uint64_t qppm_reg : 8;
    uint64_t qppm_rnw : 1;
    uint64_t reserved1 : 1;
    uint64_t reserved2 : 54;
#else
    uint64_t reserved2 : 54;
    uint64_t reserved1 : 1;
    uint64_t qppm_rnw : 1;
    uint64_t qppm_reg : 8;
#endif // _BIG_ENDIAN
    } fields;
} cppm_ippmcmd_t;



typedef union cppm_ippmstat
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
    uint64_t qppm_ongoing : 1;
    uint64_t qppm_status : 2;
    uint64_t reserved1 : 61;
#else
    uint64_t reserved1 : 61;
    uint64_t qppm_status : 2;
    uint64_t qppm_ongoing : 1;
#endif // _BIG_ENDIAN
    } fields;
} cppm_ippmstat_t;



typedef union cppm_ippmwdata
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
    uint64_t qppm_wdata : 64;
#else
    uint64_t qppm_wdata : 64;
#endif // _BIG_ENDIAN
    } fields;
} cppm_ippmwdata_t;



typedef union cppm_ippmrdata
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
    uint64_t qppm_rdata : 64;
#else
    uint64_t qppm_rdata : 64;
#endif // _BIG_ENDIAN
    } fields;
} cppm_ippmrdata_t;


#endif // __ASSEMBLER__
#endif // __CPPM_FIRMWARE_REGISTERS_H__

