/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/qppm_firmware_registers.h $             */
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
#ifndef __QPPM_FIRMWARE_REGISTERS_H__
#define __QPPM_FIRMWARE_REGISTERS_H__

// $Id$
// $Source$
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2015
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file qppm_firmware_registers.h
/// \brief C register structs for the QPPM unit

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




typedef union qppm_qpmmr {

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
    uint64_t reserved1 : 1;
    uint64_t fsafe : 11;
    uint64_t enable_fsafe_upon_heartbeat_loss : 1;
    uint64_t reserved13_15 : 3;
    uint64_t reserved2 : 4;
    uint64_t cme_interppm_ivrm_enable : 1;
    uint64_t cme_interppm_ivrm_sel : 1;
    uint64_t cme_interppm_aclk_enable : 1;
    uint64_t cme_interppm_aclk_sel : 1;
    uint64_t cme_interppm_vdm_enable : 1;
    uint64_t cme_interppm_vdm_sel : 1;
    uint64_t cme_interppm_dpll_enable : 1;
    uint64_t cme_interppm_dpll_sel : 1;
    uint64_t reserved28_293 : 2;
    uint64_t pb_purge_pls : 1;
    uint64_t pb_purge_done_lvl : 1;
    uint64_t reserved4 : 32;
#else
    uint64_t reserved4 : 32;
    uint64_t pb_purge_done_lvl : 1;
    uint64_t pb_purge_pls : 1;
    uint64_t reserved28_293 : 2;
    uint64_t cme_interppm_dpll_sel : 1;
    uint64_t cme_interppm_dpll_enable : 1;
    uint64_t cme_interppm_vdm_sel : 1;
    uint64_t cme_interppm_vdm_enable : 1;
    uint64_t cme_interppm_aclk_sel : 1;
    uint64_t cme_interppm_aclk_enable : 1;
    uint64_t cme_interppm_ivrm_sel : 1;
    uint64_t cme_interppm_ivrm_enable : 1;
    uint64_t reserved2 : 4;
    uint64_t reserved13_15 : 3;
    uint64_t enable_fsafe_upon_heartbeat_loss : 1;
    uint64_t fsafe : 11;
    uint64_t reserved1 : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qpmmr_t;



typedef union qppm_qpmmr_clr {

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
    uint64_t reserved1 : 1;
    uint64_t fsafe : 11;
    uint64_t enable_fsafe_upon_heartbeat_loss : 1;
    uint64_t reserved13_15 : 3;
    uint64_t reserved2 : 4;
    uint64_t cme_interppm_ivrm_enable : 1;
    uint64_t cme_interppm_ivrm_sel : 1;
    uint64_t cme_interppm_aclk_enable : 1;
    uint64_t cme_interppm_aclk_sel : 1;
    uint64_t cme_interppm_vdm_enable : 1;
    uint64_t cme_interppm_vdm_sel : 1;
    uint64_t cme_interppm_dpll_enable : 1;
    uint64_t cme_interppm_dpll_sel : 1;
    uint64_t reserved28_293 : 2;
    uint64_t pb_purge_pls : 1;
    uint64_t pb_purge_done_lvl : 1;
    uint64_t reserved4 : 32;
#else
    uint64_t reserved4 : 32;
    uint64_t pb_purge_done_lvl : 1;
    uint64_t pb_purge_pls : 1;
    uint64_t reserved28_293 : 2;
    uint64_t cme_interppm_dpll_sel : 1;
    uint64_t cme_interppm_dpll_enable : 1;
    uint64_t cme_interppm_vdm_sel : 1;
    uint64_t cme_interppm_vdm_enable : 1;
    uint64_t cme_interppm_aclk_sel : 1;
    uint64_t cme_interppm_aclk_enable : 1;
    uint64_t cme_interppm_ivrm_sel : 1;
    uint64_t cme_interppm_ivrm_enable : 1;
    uint64_t reserved2 : 4;
    uint64_t reserved13_15 : 3;
    uint64_t enable_fsafe_upon_heartbeat_loss : 1;
    uint64_t fsafe : 11;
    uint64_t reserved1 : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qpmmr_clr_t;



typedef union qppm_qpmmr_or {

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
    uint64_t reserved1 : 1;
    uint64_t fsafe : 11;
    uint64_t enable_fsafe_upon_heartbeat_loss : 1;
    uint64_t reserved13_15 : 3;
    uint64_t reserved2 : 4;
    uint64_t cme_interppm_ivrm_enable : 1;
    uint64_t cme_interppm_ivrm_sel : 1;
    uint64_t cme_interppm_aclk_enable : 1;
    uint64_t cme_interppm_aclk_sel : 1;
    uint64_t cme_interppm_vdm_enable : 1;
    uint64_t cme_interppm_vdm_sel : 1;
    uint64_t cme_interppm_dpll_enable : 1;
    uint64_t cme_interppm_dpll_sel : 1;
    uint64_t reserved28_293 : 2;
    uint64_t pb_purge_pls : 1;
    uint64_t pb_purge_done_lvl : 1;
    uint64_t reserved4 : 32;
#else
    uint64_t reserved4 : 32;
    uint64_t pb_purge_done_lvl : 1;
    uint64_t pb_purge_pls : 1;
    uint64_t reserved28_293 : 2;
    uint64_t cme_interppm_dpll_sel : 1;
    uint64_t cme_interppm_dpll_enable : 1;
    uint64_t cme_interppm_vdm_sel : 1;
    uint64_t cme_interppm_vdm_enable : 1;
    uint64_t cme_interppm_aclk_sel : 1;
    uint64_t cme_interppm_aclk_enable : 1;
    uint64_t cme_interppm_ivrm_sel : 1;
    uint64_t cme_interppm_ivrm_enable : 1;
    uint64_t reserved2 : 4;
    uint64_t reserved13_15 : 3;
    uint64_t enable_fsafe_upon_heartbeat_loss : 1;
    uint64_t fsafe : 11;
    uint64_t reserved1 : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qpmmr_or_t;



typedef union qppm_errsum {

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
    uint64_t pm_error : 1;
    uint64_t reserved1 : 63;
#else
    uint64_t reserved1 : 63;
    uint64_t pm_error : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_errsum_t;



typedef union qppm_err {

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
    uint64_t pcb_interrupt_protocol_err : 1;
    uint64_t special_wkup_protocol_err : 1;
    uint64_t l2_ex0_clk_sync_err : 1;
    uint64_t l2_ex1_clk_sync_err : 1;
    uint64_t dpll_dyn_fmin_err : 1;
    uint64_t dpll_dco_full_err : 1;
    uint64_t dpll_dco_empty_err : 1;
    uint64_t dpll_int_err : 1;
    uint64_t occ_heartbeat_loss : 1;
    uint64_t spare_8_11 : 3;
    uint64_t reserved1 : 52;
#else
    uint64_t reserved1 : 52;
    uint64_t spare_8_11 : 3;
    uint64_t occ_heartbeat_loss : 1;
    uint64_t dpll_int_err : 1;
    uint64_t dpll_dco_empty_err : 1;
    uint64_t dpll_dco_full_err : 1;
    uint64_t dpll_dyn_fmin_err : 1;
    uint64_t l2_ex1_clk_sync_err : 1;
    uint64_t l2_ex0_clk_sync_err : 1;
    uint64_t special_wkup_protocol_err : 1;
    uint64_t pcb_interrupt_protocol_err : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_err_t;



typedef union qppm_errmsk {

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
    uint64_t reserved1 : 12;
    uint64_t reserved2 : 52;
#else
    uint64_t reserved2 : 52;
    uint64_t reserved1 : 12;
#endif // _BIG_ENDIAN
    } fields;
} qppm_errmsk_t;



typedef union qppm_dpll_freq {

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
    uint64_t reserved1 : 1;
    uint64_t fmax : 11;
    uint64_t hires_fmax : 4;
    uint64_t reserved2 : 1;
    uint64_t freq_mult : 11;
    uint64_t hires_freq_mult : 4;
    uint64_t reserved3 : 1;
    uint64_t fmin : 11;
    uint64_t hires_fmin : 4;
    uint64_t reserved4 : 16;
#else
    uint64_t reserved4 : 16;
    uint64_t hires_fmin : 4;
    uint64_t fmin : 11;
    uint64_t reserved3 : 1;
    uint64_t hires_freq_mult : 4;
    uint64_t freq_mult : 11;
    uint64_t reserved2 : 1;
    uint64_t hires_fmax : 4;
    uint64_t fmax : 11;
    uint64_t reserved1 : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_dpll_freq_t;



typedef union qppm_dpll_ctrl {

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
    uint64_t dpll_lock_sel : 1;
    uint64_t dynamic_filter_enable : 1;
    uint64_t ff_bypass : 1;
    uint64_t dco_override : 1;
    uint64_t dco_incr : 1;
    uint64_t dco_decr : 1;
    uint64_t ff_slewrate : 10;
    uint64_t ss_enable : 1;
    uint64_t reserved_17_19 : 3;
    uint64_t reserved1 : 44;
#else
    uint64_t reserved1 : 44;
    uint64_t reserved_17_19 : 3;
    uint64_t ss_enable : 1;
    uint64_t ff_slewrate : 10;
    uint64_t dco_decr : 1;
    uint64_t dco_incr : 1;
    uint64_t dco_override : 1;
    uint64_t ff_bypass : 1;
    uint64_t dynamic_filter_enable : 1;
    uint64_t dpll_lock_sel : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_dpll_ctrl_t;



typedef union qppm_dpll_ctrl_clr {

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
    uint64_t dpll_lock_sel : 1;
    uint64_t dynamic_filter_enable : 1;
    uint64_t ff_bypass : 1;
    uint64_t dco_override : 1;
    uint64_t dco_incr : 1;
    uint64_t dco_decr : 1;
    uint64_t ff_slewrate : 10;
    uint64_t ss_enable : 1;
    uint64_t reserved_17_19 : 3;
    uint64_t reserved1 : 44;
#else
    uint64_t reserved1 : 44;
    uint64_t reserved_17_19 : 3;
    uint64_t ss_enable : 1;
    uint64_t ff_slewrate : 10;
    uint64_t dco_decr : 1;
    uint64_t dco_incr : 1;
    uint64_t dco_override : 1;
    uint64_t ff_bypass : 1;
    uint64_t dynamic_filter_enable : 1;
    uint64_t dpll_lock_sel : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_dpll_ctrl_clr_t;



typedef union qppm_dpll_ctrl_or {

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
    uint64_t dpll_lock_sel : 1;
    uint64_t dynamic_filter_enable : 1;
    uint64_t ff_bypass : 1;
    uint64_t dco_override : 1;
    uint64_t dco_incr : 1;
    uint64_t dco_decr : 1;
    uint64_t ff_slewrate : 10;
    uint64_t ss_enable : 1;
    uint64_t reserved_17_19 : 3;
    uint64_t reserved1 : 44;
#else
    uint64_t reserved1 : 44;
    uint64_t reserved_17_19 : 3;
    uint64_t ss_enable : 1;
    uint64_t ff_slewrate : 10;
    uint64_t dco_decr : 1;
    uint64_t dco_incr : 1;
    uint64_t dco_override : 1;
    uint64_t ff_bypass : 1;
    uint64_t dynamic_filter_enable : 1;
    uint64_t dpll_lock_sel : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_dpll_ctrl_or_t;



typedef union qppm_dpll_stat {

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
    uint64_t reserved1 : 1;
    uint64_t freqout : 11;
    uint64_t hires_freqout : 5;
    uint64_t reserved2 : 40;
    uint64_t reserved_57_60 : 4;
    uint64_t fsafe_active : 1;
    uint64_t freq_change : 1;
    uint64_t lock : 1;
#else
    uint64_t lock : 1;
    uint64_t freq_change : 1;
    uint64_t fsafe_active : 1;
    uint64_t reserved_57_60 : 4;
    uint64_t reserved2 : 40;
    uint64_t hires_freqout : 5;
    uint64_t freqout : 11;
    uint64_t reserved1 : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_dpll_stat_t;



typedef union qppm_dpll_ochar {

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
    uint64_t reserved1 : 1;
    uint64_t freqout_max : 11;
    uint64_t hires_freqout_max : 5;
    uint64_t reserved2 : 4;
    uint64_t freqout_avg : 11;
    uint64_t hires_freqout_avg : 5;
    uint64_t reserved3 : 4;
    uint64_t freqout_min : 11;
    uint64_t hires_freqout_min : 5;
    uint64_t reserved4 : 7;
#else
    uint64_t reserved4 : 7;
    uint64_t hires_freqout_min : 5;
    uint64_t freqout_min : 11;
    uint64_t reserved3 : 4;
    uint64_t hires_freqout_avg : 5;
    uint64_t freqout_avg : 11;
    uint64_t reserved2 : 4;
    uint64_t hires_freqout_max : 5;
    uint64_t freqout_max : 11;
    uint64_t reserved1 : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_dpll_ochar_t;



typedef union qppm_dpll_ichar {

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
    uint64_t reserved1 : 1;
    uint64_t freqin_avg : 11;
    uint64_t hires_freqin_avg : 5;
    uint64_t reserved2 : 4;
    uint64_t freqin_max : 11;
    uint64_t hires_freqin_max : 5;
    uint64_t reserved3 : 4;
    uint64_t freqin_min : 11;
    uint64_t hires_freqin_min : 5;
    uint64_t reserved4 : 7;
#else
    uint64_t reserved4 : 7;
    uint64_t hires_freqin_min : 5;
    uint64_t freqin_min : 11;
    uint64_t reserved3 : 4;
    uint64_t hires_freqin_max : 5;
    uint64_t freqin_max : 11;
    uint64_t reserved2 : 4;
    uint64_t hires_freqin_avg : 5;
    uint64_t freqin_avg : 11;
    uint64_t reserved1 : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_dpll_ichar_t;



typedef union qppm_occhb {

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
    uint64_t occ_heartbeat_count : 16;
    uint64_t occ_heartbeat_enable : 1;
    uint64_t reserved1 : 47;
#else
    uint64_t reserved1 : 47;
    uint64_t occ_heartbeat_enable : 1;
    uint64_t occ_heartbeat_count : 16;
#endif // _BIG_ENDIAN
    } fields;
} qppm_occhb_t;



typedef union qppm_qaccr {

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
    uint64_t core_clk_sb_strength : 4;
    uint64_t core_clk_sb_spare : 1;
    uint64_t core_clk_sb_pulse_mode_en : 1;
    uint64_t core_clk_sb_pulse_mode : 2;
    uint64_t core_clk_sw_resclk : 4;
    uint64_t core_clk_sw_spare : 1;
    uint64_t l2_ex0_clk_sync_enable : 1;
    uint64_t reserved_14_151 : 2;
    uint64_t l2_ex0_clkglm_async_reset : 1;
    uint64_t reserved_17_182 : 2;
    uint64_t l2_ex0_clkglm_sel : 1;
    uint64_t l2_ex0_clk_sb_strength : 4;
    uint64_t l2_ex0_clk_sb_spare0 : 1;
    uint64_t l2_ex0_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex0_clk_sb_pulse_mode : 2;
    uint64_t l2_ex0_clk_sw_resclk : 4;
    uint64_t l2_ex0_clk_sw_spare1 : 1;
    uint64_t l2_ex1_clk_sync_enable : 1;
    uint64_t reserved_34_353 : 2;
    uint64_t l2_ex1_clkglm_async_reset : 1;
    uint64_t reserved_37_384 : 2;
    uint64_t l2_ex1_clkglm_sel : 1;
    uint64_t l2_ex1_clk_sb_strength : 4;
    uint64_t l2_ex1_clk_sb_spare0 : 1;
    uint64_t l2_ex1_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex1_clk_sb_pulse_mode : 2;
    uint64_t l2_ex1_clk_sw_resclk : 4;
    uint64_t l2_ex1_clk_sw_spare1 : 1;
    uint64_t reserved_53_55 : 3;
    uint64_t l3_clk_sb_strength : 4;
    uint64_t l3_clk_sb_spare0 : 1;
    uint64_t l3_clk_sb_pulse_mode_en : 1;
    uint64_t l3_clk_sb_pulse_mode : 2;
#else
    uint64_t l3_clk_sb_pulse_mode : 2;
    uint64_t l3_clk_sb_pulse_mode_en : 1;
    uint64_t l3_clk_sb_spare0 : 1;
    uint64_t l3_clk_sb_strength : 4;
    uint64_t reserved_53_55 : 3;
    uint64_t l2_ex1_clk_sw_spare1 : 1;
    uint64_t l2_ex1_clk_sw_resclk : 4;
    uint64_t l2_ex1_clk_sb_pulse_mode : 2;
    uint64_t l2_ex1_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex1_clk_sb_spare0 : 1;
    uint64_t l2_ex1_clk_sb_strength : 4;
    uint64_t l2_ex1_clkglm_sel : 1;
    uint64_t reserved_37_384 : 2;
    uint64_t l2_ex1_clkglm_async_reset : 1;
    uint64_t reserved_34_353 : 2;
    uint64_t l2_ex1_clk_sync_enable : 1;
    uint64_t l2_ex0_clk_sw_spare1 : 1;
    uint64_t l2_ex0_clk_sw_resclk : 4;
    uint64_t l2_ex0_clk_sb_pulse_mode : 2;
    uint64_t l2_ex0_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex0_clk_sb_spare0 : 1;
    uint64_t l2_ex0_clk_sb_strength : 4;
    uint64_t l2_ex0_clkglm_sel : 1;
    uint64_t reserved_17_182 : 2;
    uint64_t l2_ex0_clkglm_async_reset : 1;
    uint64_t reserved_14_151 : 2;
    uint64_t l2_ex0_clk_sync_enable : 1;
    uint64_t core_clk_sw_spare : 1;
    uint64_t core_clk_sw_resclk : 4;
    uint64_t core_clk_sb_pulse_mode : 2;
    uint64_t core_clk_sb_pulse_mode_en : 1;
    uint64_t core_clk_sb_spare : 1;
    uint64_t core_clk_sb_strength : 4;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qaccr_t;



typedef union qppm_qaccr_clr {

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
    uint64_t core_clk_sb_strength : 4;
    uint64_t core_clk_sb_spare : 1;
    uint64_t core_clk_sb_pulse_mode_en : 1;
    uint64_t core_clk_sb_pulse_mode : 2;
    uint64_t core_clk_sw_resclk : 4;
    uint64_t core_clk_sw_spare : 1;
    uint64_t l2_ex0_clk_sync_enable : 1;
    uint64_t reserved_14_151 : 2;
    uint64_t l2_ex0_clkglm_async_reset : 1;
    uint64_t reserved_17_182 : 2;
    uint64_t l2_ex0_clkglm_sel : 1;
    uint64_t l2_ex0_clk_sb_strength : 4;
    uint64_t l2_ex0_clk_sb_spare0 : 1;
    uint64_t l2_ex0_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex0_clk_sb_pulse_mode : 2;
    uint64_t l2_ex0_clk_sw_resclk : 4;
    uint64_t l2_ex0_clk_sw_spare1 : 1;
    uint64_t l2_ex1_clk_sync_enable : 1;
    uint64_t reserved_34_353 : 2;
    uint64_t l2_ex1_clkglm_async_reset : 1;
    uint64_t reserved_37_384 : 2;
    uint64_t l2_ex1_clkglm_sel : 1;
    uint64_t l2_ex1_clk_sb_strength : 4;
    uint64_t l2_ex1_clk_sb_spare0 : 1;
    uint64_t l2_ex1_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex1_clk_sb_pulse_mode : 2;
    uint64_t l2_ex1_clk_sw_resclk : 4;
    uint64_t l2_ex1_clk_sw_spare1 : 1;
    uint64_t reserved_53_55 : 3;
    uint64_t l3_clk_sb_strength : 4;
    uint64_t l3_clk_sb_spare0 : 1;
    uint64_t l3_clk_sb_pulse_mode_en : 1;
    uint64_t l3_clk_sb_pulse_mode : 2;
#else
    uint64_t l3_clk_sb_pulse_mode : 2;
    uint64_t l3_clk_sb_pulse_mode_en : 1;
    uint64_t l3_clk_sb_spare0 : 1;
    uint64_t l3_clk_sb_strength : 4;
    uint64_t reserved_53_55 : 3;
    uint64_t l2_ex1_clk_sw_spare1 : 1;
    uint64_t l2_ex1_clk_sw_resclk : 4;
    uint64_t l2_ex1_clk_sb_pulse_mode : 2;
    uint64_t l2_ex1_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex1_clk_sb_spare0 : 1;
    uint64_t l2_ex1_clk_sb_strength : 4;
    uint64_t l2_ex1_clkglm_sel : 1;
    uint64_t reserved_37_384 : 2;
    uint64_t l2_ex1_clkglm_async_reset : 1;
    uint64_t reserved_34_353 : 2;
    uint64_t l2_ex1_clk_sync_enable : 1;
    uint64_t l2_ex0_clk_sw_spare1 : 1;
    uint64_t l2_ex0_clk_sw_resclk : 4;
    uint64_t l2_ex0_clk_sb_pulse_mode : 2;
    uint64_t l2_ex0_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex0_clk_sb_spare0 : 1;
    uint64_t l2_ex0_clk_sb_strength : 4;
    uint64_t l2_ex0_clkglm_sel : 1;
    uint64_t reserved_17_182 : 2;
    uint64_t l2_ex0_clkglm_async_reset : 1;
    uint64_t reserved_14_151 : 2;
    uint64_t l2_ex0_clk_sync_enable : 1;
    uint64_t core_clk_sw_spare : 1;
    uint64_t core_clk_sw_resclk : 4;
    uint64_t core_clk_sb_pulse_mode : 2;
    uint64_t core_clk_sb_pulse_mode_en : 1;
    uint64_t core_clk_sb_spare : 1;
    uint64_t core_clk_sb_strength : 4;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qaccr_clr_t;



typedef union qppm_qaccr_or {

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
    uint64_t core_clk_sb_strength : 4;
    uint64_t core_clk_sb_spare : 1;
    uint64_t core_clk_sb_pulse_mode_en : 1;
    uint64_t core_clk_sb_pulse_mode : 2;
    uint64_t core_clk_sw_resclk : 4;
    uint64_t core_clk_sw_spare : 1;
    uint64_t l2_ex0_clk_sync_enable : 1;
    uint64_t reserved_14_151 : 2;
    uint64_t l2_ex0_clkglm_async_reset : 1;
    uint64_t reserved_17_182 : 2;
    uint64_t l2_ex0_clkglm_sel : 1;
    uint64_t l2_ex0_clk_sb_strength : 4;
    uint64_t l2_ex0_clk_sb_spare0 : 1;
    uint64_t l2_ex0_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex0_clk_sb_pulse_mode : 2;
    uint64_t l2_ex0_clk_sw_resclk : 4;
    uint64_t l2_ex0_clk_sw_spare1 : 1;
    uint64_t l2_ex1_clk_sync_enable : 1;
    uint64_t reserved_34_353 : 2;
    uint64_t l2_ex1_clkglm_async_reset : 1;
    uint64_t reserved_37_384 : 2;
    uint64_t l2_ex1_clkglm_sel : 1;
    uint64_t l2_ex1_clk_sb_strength : 4;
    uint64_t l2_ex1_clk_sb_spare0 : 1;
    uint64_t l2_ex1_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex1_clk_sb_pulse_mode : 2;
    uint64_t l2_ex1_clk_sw_resclk : 4;
    uint64_t l2_ex1_clk_sw_spare1 : 1;
    uint64_t reserved_53_55 : 3;
    uint64_t l3_clk_sb_strength : 4;
    uint64_t l3_clk_sb_spare0 : 1;
    uint64_t l3_clk_sb_pulse_mode_en : 1;
    uint64_t l3_clk_sb_pulse_mode : 2;
#else
    uint64_t l3_clk_sb_pulse_mode : 2;
    uint64_t l3_clk_sb_pulse_mode_en : 1;
    uint64_t l3_clk_sb_spare0 : 1;
    uint64_t l3_clk_sb_strength : 4;
    uint64_t reserved_53_55 : 3;
    uint64_t l2_ex1_clk_sw_spare1 : 1;
    uint64_t l2_ex1_clk_sw_resclk : 4;
    uint64_t l2_ex1_clk_sb_pulse_mode : 2;
    uint64_t l2_ex1_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex1_clk_sb_spare0 : 1;
    uint64_t l2_ex1_clk_sb_strength : 4;
    uint64_t l2_ex1_clkglm_sel : 1;
    uint64_t reserved_37_384 : 2;
    uint64_t l2_ex1_clkglm_async_reset : 1;
    uint64_t reserved_34_353 : 2;
    uint64_t l2_ex1_clk_sync_enable : 1;
    uint64_t l2_ex0_clk_sw_spare1 : 1;
    uint64_t l2_ex0_clk_sw_resclk : 4;
    uint64_t l2_ex0_clk_sb_pulse_mode : 2;
    uint64_t l2_ex0_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex0_clk_sb_spare0 : 1;
    uint64_t l2_ex0_clk_sb_strength : 4;
    uint64_t l2_ex0_clkglm_sel : 1;
    uint64_t reserved_17_182 : 2;
    uint64_t l2_ex0_clkglm_async_reset : 1;
    uint64_t reserved_14_151 : 2;
    uint64_t l2_ex0_clk_sync_enable : 1;
    uint64_t core_clk_sw_spare : 1;
    uint64_t core_clk_sw_resclk : 4;
    uint64_t core_clk_sb_pulse_mode : 2;
    uint64_t core_clk_sb_pulse_mode_en : 1;
    uint64_t core_clk_sb_spare : 1;
    uint64_t core_clk_sb_strength : 4;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qaccr_or_t;



typedef union qppm_qacsr {

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
    uint64_t reserved1 : 62;
    uint64_t l2_ex0_clk_sync_done : 1;
    uint64_t l2_ex1_clk_sync_done : 1;
#else
    uint64_t l2_ex1_clk_sync_done : 1;
    uint64_t l2_ex0_clk_sync_done : 1;
    uint64_t reserved1 : 62;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qacsr_t;



typedef union qppm_vdmcfgr {

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
    uint64_t vdm_vid_compare : 8;
    uint64_t vdm_overvolt : 4;
    uint64_t vdm_droop_small : 4;
    uint64_t vdm_droop_large : 4;
    uint64_t vdm_droop_xtreme : 4;
    uint64_t reserved1 : 8;
    uint64_t vid_compare_max : 8;
    uint64_t vid_compare_min : 8;
    uint64_t reserved2 : 16;
#else
    uint64_t reserved2 : 16;
    uint64_t vid_compare_min : 8;
    uint64_t vid_compare_max : 8;
    uint64_t reserved1 : 8;
    uint64_t vdm_droop_xtreme : 4;
    uint64_t vdm_droop_large : 4;
    uint64_t vdm_droop_small : 4;
    uint64_t vdm_overvolt : 4;
    uint64_t vdm_vid_compare : 8;
#endif // _BIG_ENDIAN
    } fields;
} qppm_vdmcfgr_t;



typedef union qppm_edram_ctrl {

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
    uint64_t l3_ex0_edram_enable : 1;
    uint64_t l3_ex0_edram_vwl_enable : 1;
    uint64_t l3_ex0_edram_vrow_vblh_enable : 1;
    uint64_t l3_ex0_edram_vpp_enable : 1;
    uint64_t l3_ex1_edram_enable : 1;
    uint64_t l3_ex1_edram_vwl_enable : 1;
    uint64_t l3_ex1_edram_vrow_vblh_enable : 1;
    uint64_t l3_ex1_edram_vpp_enable : 1;
    uint64_t reserved1 : 56;
#else
    uint64_t reserved1 : 56;
    uint64_t l3_ex1_edram_vpp_enable : 1;
    uint64_t l3_ex1_edram_vrow_vblh_enable : 1;
    uint64_t l3_ex1_edram_vwl_enable : 1;
    uint64_t l3_ex1_edram_enable : 1;
    uint64_t l3_ex0_edram_vpp_enable : 1;
    uint64_t l3_ex0_edram_vrow_vblh_enable : 1;
    uint64_t l3_ex0_edram_vwl_enable : 1;
    uint64_t l3_ex0_edram_enable : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_edram_ctrl_t;



typedef union qppm_edram_ctrl_clr {

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
    uint64_t l3_ex0_edram_enable : 1;
    uint64_t l3_ex0_edram_vwl_enable : 1;
    uint64_t l3_ex0_edram_vrow_vblh_enable : 1;
    uint64_t l3_ex0_edram_vpp_enable : 1;
    uint64_t l3_ex1_edram_enable : 1;
    uint64_t l3_ex1_edram_vwl_enable : 1;
    uint64_t l3_ex1_edram_vrow_vblh_enable : 1;
    uint64_t l3_ex1_edram_vpp_enable : 1;
    uint64_t reserved1 : 56;
#else
    uint64_t reserved1 : 56;
    uint64_t l3_ex1_edram_vpp_enable : 1;
    uint64_t l3_ex1_edram_vrow_vblh_enable : 1;
    uint64_t l3_ex1_edram_vwl_enable : 1;
    uint64_t l3_ex1_edram_enable : 1;
    uint64_t l3_ex0_edram_vpp_enable : 1;
    uint64_t l3_ex0_edram_vrow_vblh_enable : 1;
    uint64_t l3_ex0_edram_vwl_enable : 1;
    uint64_t l3_ex0_edram_enable : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_edram_ctrl_clr_t;



typedef union qppm_edram_ctrl_or {

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
    uint64_t l3_ex0_edram_enable : 1;
    uint64_t l3_ex0_edram_vwl_enable : 1;
    uint64_t l3_ex0_edram_vrow_vblh_enable : 1;
    uint64_t l3_ex0_edram_vpp_enable : 1;
    uint64_t l3_ex1_edram_enable : 1;
    uint64_t l3_ex1_edram_vwl_enable : 1;
    uint64_t l3_ex1_edram_vrow_vblh_enable : 1;
    uint64_t l3_ex1_edram_vpp_enable : 1;
    uint64_t reserved1 : 56;
#else
    uint64_t reserved1 : 56;
    uint64_t l3_ex1_edram_vpp_enable : 1;
    uint64_t l3_ex1_edram_vrow_vblh_enable : 1;
    uint64_t l3_ex1_edram_vwl_enable : 1;
    uint64_t l3_ex1_edram_enable : 1;
    uint64_t l3_ex0_edram_vpp_enable : 1;
    uint64_t l3_ex0_edram_vrow_vblh_enable : 1;
    uint64_t l3_ex0_edram_vwl_enable : 1;
    uint64_t l3_ex0_edram_enable : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_edram_ctrl_or_t;


#endif // __ASSEMBLER__
#endif // __QPPM_FIRMWARE_REGISTERS_H__

