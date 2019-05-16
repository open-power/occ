/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/qppm_firmware_registers.h $             */
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
#ifndef __QPPM_FIRMWARE_REGISTERS_H__
#define __QPPM_FIRMWARE_REGISTERS_H__

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




typedef union qppm_qpmmr
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
        uint64_t force_fsafe : 1;
    uint64_t fsafe : 11;
    uint64_t enable_fsafe_upon_heartbeat_loss : 1;
        uint64_t enable_droop_protect_upon_heartbeat_loss : 1;
        uint64_t enable_pfets_upon_ivrm_dropout : 1;
        uint64_t reserved1 : 1;
        uint64_t enable_pcb_intr_upon_heartbeat_loss : 1;
        uint64_t enable_pcb_intr_upon_ivrm_dropout : 1;
        uint64_t enable_pcb_intr_upon_large_droop : 1;
        uint64_t enable_pcb_intr_upon_extreme_droop : 1;
    uint64_t cme_interppm_ivrm_enable : 1;
    uint64_t cme_interppm_ivrm_sel : 1;
    uint64_t cme_interppm_aclk_enable : 1;
    uint64_t cme_interppm_aclk_sel : 1;
        uint64_t cme_interppm_vdata_enable : 1;
        uint64_t cme_interppm_vdata_sel : 1;
    uint64_t cme_interppm_dpll_enable : 1;
    uint64_t cme_interppm_dpll_sel : 1;
        uint64_t reserved28_312 : 4;
        uint64_t reserved3 : 32;
#else
        uint64_t reserved3 : 32;
        uint64_t reserved28_312 : 4;
    uint64_t cme_interppm_dpll_sel : 1;
    uint64_t cme_interppm_dpll_enable : 1;
        uint64_t cme_interppm_vdata_sel : 1;
        uint64_t cme_interppm_vdata_enable : 1;
    uint64_t cme_interppm_aclk_sel : 1;
    uint64_t cme_interppm_aclk_enable : 1;
    uint64_t cme_interppm_ivrm_sel : 1;
    uint64_t cme_interppm_ivrm_enable : 1;
        uint64_t enable_pcb_intr_upon_extreme_droop : 1;
        uint64_t enable_pcb_intr_upon_large_droop : 1;
        uint64_t enable_pcb_intr_upon_ivrm_dropout : 1;
        uint64_t enable_pcb_intr_upon_heartbeat_loss : 1;
        uint64_t reserved1 : 1;
        uint64_t enable_pfets_upon_ivrm_dropout : 1;
        uint64_t enable_droop_protect_upon_heartbeat_loss : 1;
    uint64_t enable_fsafe_upon_heartbeat_loss : 1;
    uint64_t fsafe : 11;
        uint64_t force_fsafe : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qpmmr_t;



typedef union qppm_qpmmr_clr
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
        uint64_t force_fsafe : 1;
    uint64_t fsafe : 11;
    uint64_t enable_fsafe_upon_heartbeat_loss : 1;
        uint64_t enable_droop_protect_upon_heartbeat_loss : 1;
        uint64_t enable_pfets_upon_ivrm_dropout : 1;
        uint64_t reserved1 : 1;
        uint64_t enable_pcb_intr_upon_heartbeat_loss : 1;
        uint64_t enable_pcb_intr_upon_ivrm_dropout : 1;
        uint64_t enable_pcb_intr_upon_large_droop : 1;
        uint64_t enable_pcb_intr_upon_extreme_droop : 1;
    uint64_t cme_interppm_ivrm_enable : 1;
    uint64_t cme_interppm_ivrm_sel : 1;
    uint64_t cme_interppm_aclk_enable : 1;
    uint64_t cme_interppm_aclk_sel : 1;
        uint64_t cme_interppm_vdata_enable : 1;
        uint64_t cme_interppm_vdata_sel : 1;
    uint64_t cme_interppm_dpll_enable : 1;
    uint64_t cme_interppm_dpll_sel : 1;
        uint64_t reserved28_312 : 4;
        uint64_t reserved3 : 32;
#else
        uint64_t reserved3 : 32;
        uint64_t reserved28_312 : 4;
    uint64_t cme_interppm_dpll_sel : 1;
    uint64_t cme_interppm_dpll_enable : 1;
        uint64_t cme_interppm_vdata_sel : 1;
        uint64_t cme_interppm_vdata_enable : 1;
    uint64_t cme_interppm_aclk_sel : 1;
    uint64_t cme_interppm_aclk_enable : 1;
    uint64_t cme_interppm_ivrm_sel : 1;
    uint64_t cme_interppm_ivrm_enable : 1;
        uint64_t enable_pcb_intr_upon_extreme_droop : 1;
        uint64_t enable_pcb_intr_upon_large_droop : 1;
        uint64_t enable_pcb_intr_upon_ivrm_dropout : 1;
        uint64_t enable_pcb_intr_upon_heartbeat_loss : 1;
        uint64_t reserved1 : 1;
        uint64_t enable_pfets_upon_ivrm_dropout : 1;
        uint64_t enable_droop_protect_upon_heartbeat_loss : 1;
    uint64_t enable_fsafe_upon_heartbeat_loss : 1;
    uint64_t fsafe : 11;
        uint64_t force_fsafe : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qpmmr_clr_t;



typedef union qppm_qpmmr_or
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
        uint64_t force_fsafe : 1;
    uint64_t fsafe : 11;
    uint64_t enable_fsafe_upon_heartbeat_loss : 1;
        uint64_t enable_droop_protect_upon_heartbeat_loss : 1;
        uint64_t enable_pfets_upon_ivrm_dropout : 1;
        uint64_t reserved1 : 1;
        uint64_t enable_pcb_intr_upon_heartbeat_loss : 1;
        uint64_t enable_pcb_intr_upon_ivrm_dropout : 1;
        uint64_t enable_pcb_intr_upon_large_droop : 1;
        uint64_t enable_pcb_intr_upon_extreme_droop : 1;
    uint64_t cme_interppm_ivrm_enable : 1;
    uint64_t cme_interppm_ivrm_sel : 1;
    uint64_t cme_interppm_aclk_enable : 1;
    uint64_t cme_interppm_aclk_sel : 1;
        uint64_t cme_interppm_vdata_enable : 1;
        uint64_t cme_interppm_vdata_sel : 1;
    uint64_t cme_interppm_dpll_enable : 1;
    uint64_t cme_interppm_dpll_sel : 1;
        uint64_t reserved28_312 : 4;
        uint64_t reserved3 : 32;
#else
        uint64_t reserved3 : 32;
        uint64_t reserved28_312 : 4;
    uint64_t cme_interppm_dpll_sel : 1;
    uint64_t cme_interppm_dpll_enable : 1;
        uint64_t cme_interppm_vdata_sel : 1;
        uint64_t cme_interppm_vdata_enable : 1;
    uint64_t cme_interppm_aclk_sel : 1;
    uint64_t cme_interppm_aclk_enable : 1;
    uint64_t cme_interppm_ivrm_sel : 1;
    uint64_t cme_interppm_ivrm_enable : 1;
        uint64_t enable_pcb_intr_upon_extreme_droop : 1;
        uint64_t enable_pcb_intr_upon_large_droop : 1;
        uint64_t enable_pcb_intr_upon_ivrm_dropout : 1;
        uint64_t enable_pcb_intr_upon_heartbeat_loss : 1;
        uint64_t reserved1 : 1;
        uint64_t enable_pfets_upon_ivrm_dropout : 1;
        uint64_t enable_droop_protect_upon_heartbeat_loss : 1;
    uint64_t enable_fsafe_upon_heartbeat_loss : 1;
    uint64_t fsafe : 11;
        uint64_t force_fsafe : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qpmmr_or_t;



typedef union qppm_errsum
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
    uint64_t pm_error : 1;
    uint64_t reserved1 : 63;
#else
    uint64_t reserved1 : 63;
    uint64_t pm_error : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_errsum_t;



typedef union qppm_err
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
        uint64_t occ_heartbeat_loss : 1;
    uint64_t l2_ex0_clk_sync_err : 1;
    uint64_t l2_ex1_clk_sync_err : 1;
        uint64_t edram_sequence_err : 1;
        uint64_t edram_pgate_err : 1;
        uint64_t dpll_int_err : 1;
    uint64_t dpll_dyn_fmin_err : 1;
    uint64_t dpll_dco_full_err : 1;
    uint64_t dpll_dco_empty_err : 1;
        uint64_t inverted_vdm_data : 4;
        uint64_t cme0_ivrm_dropout_err : 1;
        uint64_t cme1_ivrm_dropout_err : 1;
        uint64_t reserved_19_23 : 5;
        uint64_t reserved1 : 40;
#else
        uint64_t reserved1 : 40;
        uint64_t reserved_19_23 : 5;
        uint64_t cme1_ivrm_dropout_err : 1;
        uint64_t cme0_ivrm_dropout_err : 1;
        uint64_t inverted_vdm_data : 4;
    uint64_t dpll_dco_empty_err : 1;
    uint64_t dpll_dco_full_err : 1;
    uint64_t dpll_dyn_fmin_err : 1;
        uint64_t dpll_int_err : 1;
        uint64_t edram_pgate_err : 1;
        uint64_t edram_sequence_err : 1;
    uint64_t l2_ex1_clk_sync_err : 1;
    uint64_t l2_ex0_clk_sync_err : 1;
        uint64_t occ_heartbeat_loss : 1;
        uint64_t pfet_seq_program_err : 1;
        uint64_t special_wkup_done_protocol_err : 1;
    uint64_t special_wkup_protocol_err : 1;
    uint64_t pcb_interrupt_protocol_err : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_err_t;



typedef union qppm_errmsk
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
        uint64_t reserved1 : 24;
        uint64_t reserved2 : 40;
#else
        uint64_t reserved2 : 40;
        uint64_t reserved1 : 24;
#endif // _BIG_ENDIAN
    } fields;
} qppm_errmsk_t;



typedef union qppm_dpll_freq
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
    uint64_t reserved1 : 1;
    uint64_t fmax : 11;
    uint64_t hires_fmax : 4;
    uint64_t reserved2 : 1;
        uint64_t fmult : 11;
        uint64_t hires_fmult : 4;
    uint64_t reserved3 : 1;
    uint64_t fmin : 11;
    uint64_t hires_fmin : 4;
    uint64_t reserved4 : 16;
#else
    uint64_t reserved4 : 16;
    uint64_t hires_fmin : 4;
    uint64_t fmin : 11;
    uint64_t reserved3 : 1;
        uint64_t hires_fmult : 4;
        uint64_t fmult : 11;
    uint64_t reserved2 : 1;
    uint64_t hires_fmax : 4;
    uint64_t fmax : 11;
    uint64_t reserved1 : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_dpll_freq_t;



typedef union qppm_dpll_ctrl
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
    uint64_t dpll_lock_sel : 1;
        uint64_t enable_jump_protect : 1;
    uint64_t ff_bypass : 1;
    uint64_t dco_override : 1;
    uint64_t dco_incr : 1;
    uint64_t dco_decr : 1;
    uint64_t ff_slewrate : 10;
    uint64_t ss_enable : 1;
    uint64_t reserved_17_19 : 3;
        uint64_t slew_dn_sel : 1;
        uint64_t enable_jump_target_update : 1;
        uint64_t enable_fmin_target : 1;
        uint64_t enable_fmax_target : 1;
        uint64_t reserved1 : 40;
#else
        uint64_t reserved1 : 40;
        uint64_t enable_fmax_target : 1;
        uint64_t enable_fmin_target : 1;
        uint64_t enable_jump_target_update : 1;
        uint64_t slew_dn_sel : 1;
    uint64_t reserved_17_19 : 3;
    uint64_t ss_enable : 1;
    uint64_t ff_slewrate : 10;
    uint64_t dco_decr : 1;
    uint64_t dco_incr : 1;
    uint64_t dco_override : 1;
    uint64_t ff_bypass : 1;
        uint64_t enable_jump_protect : 1;
    uint64_t dpll_lock_sel : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_dpll_ctrl_t;



typedef union qppm_dpll_ctrl_clr
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
    uint64_t dpll_lock_sel : 1;
        uint64_t enable_jump_protect : 1;
    uint64_t ff_bypass : 1;
    uint64_t dco_override : 1;
    uint64_t dco_incr : 1;
    uint64_t dco_decr : 1;
    uint64_t ff_slewrate : 10;
    uint64_t ss_enable : 1;
    uint64_t reserved_17_19 : 3;
        uint64_t slew_dn_sel : 1;
        uint64_t enable_jump_target_update : 1;
        uint64_t enable_fmin_target : 1;
        uint64_t enable_fmax_target : 1;
        uint64_t reserved1 : 40;
#else
        uint64_t reserved1 : 40;
        uint64_t enable_fmax_target : 1;
        uint64_t enable_fmin_target : 1;
        uint64_t enable_jump_target_update : 1;
        uint64_t slew_dn_sel : 1;
    uint64_t reserved_17_19 : 3;
    uint64_t ss_enable : 1;
    uint64_t ff_slewrate : 10;
    uint64_t dco_decr : 1;
    uint64_t dco_incr : 1;
    uint64_t dco_override : 1;
    uint64_t ff_bypass : 1;
        uint64_t enable_jump_protect : 1;
    uint64_t dpll_lock_sel : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_dpll_ctrl_clr_t;



typedef union qppm_dpll_ctrl_or
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
    uint64_t dpll_lock_sel : 1;
        uint64_t enable_jump_protect : 1;
    uint64_t ff_bypass : 1;
    uint64_t dco_override : 1;
    uint64_t dco_incr : 1;
    uint64_t dco_decr : 1;
    uint64_t ff_slewrate : 10;
    uint64_t ss_enable : 1;
    uint64_t reserved_17_19 : 3;
        uint64_t slew_dn_sel : 1;
        uint64_t enable_jump_target_update : 1;
        uint64_t enable_fmin_target : 1;
        uint64_t enable_fmax_target : 1;
        uint64_t reserved1 : 40;
#else
        uint64_t reserved1 : 40;
        uint64_t enable_fmax_target : 1;
        uint64_t enable_fmin_target : 1;
        uint64_t enable_jump_target_update : 1;
        uint64_t slew_dn_sel : 1;
    uint64_t reserved_17_19 : 3;
    uint64_t ss_enable : 1;
    uint64_t ff_slewrate : 10;
    uint64_t dco_decr : 1;
    uint64_t dco_incr : 1;
    uint64_t dco_override : 1;
    uint64_t ff_bypass : 1;
        uint64_t enable_jump_protect : 1;
    uint64_t dpll_lock_sel : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_dpll_ctrl_or_t;



typedef union qppm_dpll_stat
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
    uint64_t reserved1 : 1;
    uint64_t freqout : 11;
    uint64_t hires_freqout : 5;
    uint64_t reserved2 : 40;
        uint64_t reserved_57_59 : 2;
    uint64_t fsafe_active : 1;
        uint64_t update_complete : 1;
    uint64_t freq_change : 1;
        uint64_t block_active : 1;
    uint64_t lock : 1;
#else
    uint64_t lock : 1;
        uint64_t block_active : 1;
    uint64_t freq_change : 1;
        uint64_t update_complete : 1;
    uint64_t fsafe_active : 1;
        uint64_t reserved_57_59 : 2;
    uint64_t reserved2 : 40;
    uint64_t hires_freqout : 5;
    uint64_t freqout : 11;
    uint64_t reserved1 : 1;
#endif // _BIG_ENDIAN
    } fields;
} qppm_dpll_stat_t;



typedef union qppm_dpll_ochar
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



typedef union qppm_dpll_ichar
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



typedef union qppm_occhb
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



typedef union qppm_qaccr
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
        uint64_t common_clk_sb_strength : 4;
        uint64_t common_clk_sb_spare : 1;
        uint64_t common_clk_sb_pulse_mode_en : 1;
        uint64_t common_clk_sb_pulse_mode : 2;
        uint64_t common_clk_sw_resclk : 4;
        uint64_t common_clk_sw_spare : 1;
        uint64_t reserved_13_15 : 3;
        uint64_t l3_clk_sb_strength : 4;
        uint64_t l3_clk_sb_spare0 : 1;
        uint64_t l3_clk_sb_pulse_mode_en : 1;
        uint64_t l3_clk_sb_pulse_mode : 2;
        uint64_t reserved1 : 40;
#else
        uint64_t reserved1 : 40;
        uint64_t l3_clk_sb_pulse_mode : 2;
        uint64_t l3_clk_sb_pulse_mode_en : 1;
        uint64_t l3_clk_sb_spare0 : 1;
        uint64_t l3_clk_sb_strength : 4;
        uint64_t reserved_13_15 : 3;
        uint64_t common_clk_sw_spare : 1;
        uint64_t common_clk_sw_resclk : 4;
        uint64_t common_clk_sb_pulse_mode : 2;
        uint64_t common_clk_sb_pulse_mode_en : 1;
        uint64_t common_clk_sb_spare : 1;
        uint64_t common_clk_sb_strength : 4;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qaccr_t;



typedef union qppm_qaccr_clr
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
        uint64_t common_clk_sb_strength : 4;
        uint64_t common_clk_sb_spare : 1;
        uint64_t common_clk_sb_pulse_mode_en : 1;
        uint64_t common_clk_sb_pulse_mode : 2;
        uint64_t common_clk_sw_resclk : 4;
        uint64_t common_clk_sw_spare : 1;
        uint64_t reserved_13_15 : 3;
        uint64_t l3_clk_sb_strength : 4;
        uint64_t l3_clk_sb_spare0 : 1;
        uint64_t l3_clk_sb_pulse_mode_en : 1;
        uint64_t l3_clk_sb_pulse_mode : 2;
        uint64_t reserved1 : 40;
#else
        uint64_t reserved1 : 40;
        uint64_t l3_clk_sb_pulse_mode : 2;
        uint64_t l3_clk_sb_pulse_mode_en : 1;
        uint64_t l3_clk_sb_spare0 : 1;
        uint64_t l3_clk_sb_strength : 4;
        uint64_t reserved_13_15 : 3;
        uint64_t common_clk_sw_spare : 1;
        uint64_t common_clk_sw_resclk : 4;
        uint64_t common_clk_sb_pulse_mode : 2;
        uint64_t common_clk_sb_pulse_mode_en : 1;
        uint64_t common_clk_sb_spare : 1;
        uint64_t common_clk_sb_strength : 4;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qaccr_clr_t;



typedef union qppm_qaccr_or
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
        uint64_t common_clk_sb_strength : 4;
        uint64_t common_clk_sb_spare : 1;
        uint64_t common_clk_sb_pulse_mode_en : 1;
        uint64_t common_clk_sb_pulse_mode : 2;
        uint64_t common_clk_sw_resclk : 4;
        uint64_t common_clk_sw_spare : 1;
        uint64_t reserved_13_15 : 3;
        uint64_t l3_clk_sb_strength : 4;
        uint64_t l3_clk_sb_spare0 : 1;
        uint64_t l3_clk_sb_pulse_mode_en : 1;
        uint64_t l3_clk_sb_pulse_mode : 2;
        uint64_t reserved1 : 40;
#else
        uint64_t reserved1 : 40;
        uint64_t l3_clk_sb_pulse_mode : 2;
        uint64_t l3_clk_sb_pulse_mode_en : 1;
        uint64_t l3_clk_sb_spare0 : 1;
        uint64_t l3_clk_sb_strength : 4;
        uint64_t reserved_13_15 : 3;
        uint64_t common_clk_sw_spare : 1;
        uint64_t common_clk_sw_resclk : 4;
        uint64_t common_clk_sb_pulse_mode : 2;
        uint64_t common_clk_sb_pulse_mode_en : 1;
        uint64_t common_clk_sb_spare : 1;
        uint64_t common_clk_sb_strength : 4;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qaccr_or_t;



typedef union qppm_qacsr
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
        uint64_t actual_l2_ex0_clk_sb_strength : 4;
        uint64_t actual_l2_ex0_clk_sb_spare0 : 1;
        uint64_t actual_l2_ex0_clk_sb_pulse_mode_en : 1;
        uint64_t actual_l2_ex0_clk_sb_pulse_mode : 2;
        uint64_t actual_l2_ex0_clk_sw_resclk : 4;
        uint64_t actual_l2_ex0_clk_sw_spare1 : 1;
        uint64_t reserved1 : 3;
        uint64_t actual_l2_ex1_clk_sb_strength : 4;
        uint64_t actual_l2_ex1_clk_sb_spare0 : 1;
        uint64_t actual_l2_ex1_clk_sb_pulse_mode_en : 1;
        uint64_t actual_l2_ex1_clk_sb_pulse_mode : 2;
        uint64_t actual_l2_ex1_clk_sw_resclk : 4;
        uint64_t actual_l2_ex1_clk_sw_spare1 : 1;
        uint64_t reserved2 : 7;
        uint64_t l2_ex0_clk_sync_done : 1;
        uint64_t l2_ex1_clk_sync_done : 1;
        uint64_t reserved3 : 26;
#else
        uint64_t reserved3 : 26;
        uint64_t l2_ex1_clk_sync_done : 1;
        uint64_t l2_ex0_clk_sync_done : 1;
        uint64_t reserved2 : 7;
        uint64_t actual_l2_ex1_clk_sw_spare1 : 1;
        uint64_t actual_l2_ex1_clk_sw_resclk : 4;
        uint64_t actual_l2_ex1_clk_sb_pulse_mode : 2;
        uint64_t actual_l2_ex1_clk_sb_pulse_mode_en : 1;
        uint64_t actual_l2_ex1_clk_sb_spare0 : 1;
        uint64_t actual_l2_ex1_clk_sb_strength : 4;
        uint64_t reserved1 : 3;
        uint64_t actual_l2_ex0_clk_sw_spare1 : 1;
        uint64_t actual_l2_ex0_clk_sw_resclk : 4;
        uint64_t actual_l2_ex0_clk_sb_pulse_mode : 2;
        uint64_t actual_l2_ex0_clk_sb_pulse_mode_en : 1;
        uint64_t actual_l2_ex0_clk_sb_spare0 : 1;
        uint64_t actual_l2_ex0_clk_sb_strength : 4;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qacsr_t;



typedef union qppm_excgcr
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
    uint64_t l2_ex0_clk_sb_strength : 4;
    uint64_t l2_ex0_clk_sb_spare0 : 1;
    uint64_t l2_ex0_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex0_clk_sb_pulse_mode : 2;
    uint64_t l2_ex0_clk_sw_resclk : 4;
    uint64_t l2_ex0_clk_sw_spare1 : 1;
        uint64_t reserved_13_151 : 3;
    uint64_t l2_ex1_clk_sb_strength : 4;
    uint64_t l2_ex1_clk_sb_spare0 : 1;
    uint64_t l2_ex1_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex1_clk_sb_pulse_mode : 2;
    uint64_t l2_ex1_clk_sw_resclk : 4;
    uint64_t l2_ex1_clk_sw_spare1 : 1;
        uint64_t reserved_29312 : 3;
        uint64_t l2_ex0_clkglm_async_reset : 1;
        uint64_t l2_ex1_clkglm_async_reset : 1;
        uint64_t l2_ex0_clkglm_sel : 1;
        uint64_t l2_ex1_clkglm_sel : 1;
        uint64_t l2_ex0_clk_sync_enable : 1;
        uint64_t l2_ex1_clk_sync_enable : 1;
        uint64_t l2_ex0_clk_sb_override : 1;
        uint64_t l2_ex1_clk_sb_override : 1;
        uint64_t l2_ex0_clk_sw_override : 1;
        uint64_t l2_ex1_clk_sw_override : 1;
        uint64_t reserved3 : 22;
#else
        uint64_t reserved3 : 22;
        uint64_t l2_ex1_clk_sw_override : 1;
        uint64_t l2_ex0_clk_sw_override : 1;
        uint64_t l2_ex1_clk_sb_override : 1;
        uint64_t l2_ex0_clk_sb_override : 1;
        uint64_t l2_ex1_clk_sync_enable : 1;
        uint64_t l2_ex0_clk_sync_enable : 1;
        uint64_t l2_ex1_clkglm_sel : 1;
        uint64_t l2_ex0_clkglm_sel : 1;
        uint64_t l2_ex1_clkglm_async_reset : 1;
        uint64_t l2_ex0_clkglm_async_reset : 1;
        uint64_t reserved_29312 : 3;
    uint64_t l2_ex1_clk_sw_spare1 : 1;
    uint64_t l2_ex1_clk_sw_resclk : 4;
    uint64_t l2_ex1_clk_sb_pulse_mode : 2;
    uint64_t l2_ex1_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex1_clk_sb_spare0 : 1;
    uint64_t l2_ex1_clk_sb_strength : 4;
        uint64_t reserved_13_151 : 3;
    uint64_t l2_ex0_clk_sw_spare1 : 1;
    uint64_t l2_ex0_clk_sw_resclk : 4;
    uint64_t l2_ex0_clk_sb_pulse_mode : 2;
    uint64_t l2_ex0_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex0_clk_sb_spare0 : 1;
    uint64_t l2_ex0_clk_sb_strength : 4;
#endif // _BIG_ENDIAN
    } fields;
} qppm_excgcr_t;



typedef union qppm_excgcr_clr
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
    uint64_t l2_ex0_clk_sb_strength : 4;
    uint64_t l2_ex0_clk_sb_spare0 : 1;
    uint64_t l2_ex0_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex0_clk_sb_pulse_mode : 2;
    uint64_t l2_ex0_clk_sw_resclk : 4;
    uint64_t l2_ex0_clk_sw_spare1 : 1;
        uint64_t reserved_13_151 : 3;
    uint64_t l2_ex1_clk_sb_strength : 4;
    uint64_t l2_ex1_clk_sb_spare0 : 1;
    uint64_t l2_ex1_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex1_clk_sb_pulse_mode : 2;
    uint64_t l2_ex1_clk_sw_resclk : 4;
    uint64_t l2_ex1_clk_sw_spare1 : 1;
        uint64_t reserved_29312 : 3;
        uint64_t l2_ex0_clkglm_async_reset : 1;
        uint64_t l2_ex1_clkglm_async_reset : 1;
        uint64_t l2_ex0_clkglm_sel : 1;
        uint64_t l2_ex1_clkglm_sel : 1;
        uint64_t l2_ex0_clk_sync_enable : 1;
        uint64_t l2_ex1_clk_sync_enable : 1;
        uint64_t l2_ex0_clk_sb_override : 1;
        uint64_t l2_ex1_clk_sb_override : 1;
        uint64_t l2_ex0_clk_sw_override : 1;
        uint64_t l2_ex1_clk_sw_override : 1;
        uint64_t reserved3 : 22;
#else
        uint64_t reserved3 : 22;
        uint64_t l2_ex1_clk_sw_override : 1;
        uint64_t l2_ex0_clk_sw_override : 1;
        uint64_t l2_ex1_clk_sb_override : 1;
        uint64_t l2_ex0_clk_sb_override : 1;
        uint64_t l2_ex1_clk_sync_enable : 1;
        uint64_t l2_ex0_clk_sync_enable : 1;
        uint64_t l2_ex1_clkglm_sel : 1;
        uint64_t l2_ex0_clkglm_sel : 1;
        uint64_t l2_ex1_clkglm_async_reset : 1;
        uint64_t l2_ex0_clkglm_async_reset : 1;
        uint64_t reserved_29312 : 3;
    uint64_t l2_ex1_clk_sw_spare1 : 1;
    uint64_t l2_ex1_clk_sw_resclk : 4;
    uint64_t l2_ex1_clk_sb_pulse_mode : 2;
    uint64_t l2_ex1_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex1_clk_sb_spare0 : 1;
    uint64_t l2_ex1_clk_sb_strength : 4;
        uint64_t reserved_13_151 : 3;
    uint64_t l2_ex0_clk_sw_spare1 : 1;
    uint64_t l2_ex0_clk_sw_resclk : 4;
    uint64_t l2_ex0_clk_sb_pulse_mode : 2;
    uint64_t l2_ex0_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex0_clk_sb_spare0 : 1;
    uint64_t l2_ex0_clk_sb_strength : 4;
#endif // _BIG_ENDIAN
    } fields;
} qppm_excgcr_clr_t;



typedef union qppm_excgcr_or
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
    uint64_t l2_ex0_clk_sb_strength : 4;
    uint64_t l2_ex0_clk_sb_spare0 : 1;
    uint64_t l2_ex0_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex0_clk_sb_pulse_mode : 2;
    uint64_t l2_ex0_clk_sw_resclk : 4;
    uint64_t l2_ex0_clk_sw_spare1 : 1;
        uint64_t reserved_13_151 : 3;
    uint64_t l2_ex1_clk_sb_strength : 4;
    uint64_t l2_ex1_clk_sb_spare0 : 1;
    uint64_t l2_ex1_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex1_clk_sb_pulse_mode : 2;
    uint64_t l2_ex1_clk_sw_resclk : 4;
    uint64_t l2_ex1_clk_sw_spare1 : 1;
        uint64_t reserved_29312 : 3;
        uint64_t l2_ex0_clkglm_async_reset : 1;
        uint64_t l2_ex1_clkglm_async_reset : 1;
        uint64_t l2_ex0_clkglm_sel : 1;
        uint64_t l2_ex1_clkglm_sel : 1;
        uint64_t l2_ex0_clk_sync_enable : 1;
        uint64_t l2_ex1_clk_sync_enable : 1;
        uint64_t l2_ex0_clk_sb_override : 1;
        uint64_t l2_ex1_clk_sb_override : 1;
        uint64_t l2_ex0_clk_sw_override : 1;
        uint64_t l2_ex1_clk_sw_override : 1;
        uint64_t reserved3 : 22;
#else
        uint64_t reserved3 : 22;
        uint64_t l2_ex1_clk_sw_override : 1;
        uint64_t l2_ex0_clk_sw_override : 1;
        uint64_t l2_ex1_clk_sb_override : 1;
        uint64_t l2_ex0_clk_sb_override : 1;
        uint64_t l2_ex1_clk_sync_enable : 1;
        uint64_t l2_ex0_clk_sync_enable : 1;
        uint64_t l2_ex1_clkglm_sel : 1;
        uint64_t l2_ex0_clkglm_sel : 1;
        uint64_t l2_ex1_clkglm_async_reset : 1;
        uint64_t l2_ex0_clkglm_async_reset : 1;
        uint64_t reserved_29312 : 3;
    uint64_t l2_ex1_clk_sw_spare1 : 1;
    uint64_t l2_ex1_clk_sw_resclk : 4;
    uint64_t l2_ex1_clk_sb_pulse_mode : 2;
    uint64_t l2_ex1_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex1_clk_sb_spare0 : 1;
    uint64_t l2_ex1_clk_sb_strength : 4;
        uint64_t reserved_13_151 : 3;
    uint64_t l2_ex0_clk_sw_spare1 : 1;
    uint64_t l2_ex0_clk_sw_resclk : 4;
    uint64_t l2_ex0_clk_sb_pulse_mode : 2;
    uint64_t l2_ex0_clk_sb_pulse_mode_en : 1;
    uint64_t l2_ex0_clk_sb_spare0 : 1;
    uint64_t l2_ex0_clk_sb_strength : 4;
#endif // _BIG_ENDIAN
    } fields;
} qppm_excgcr_or_t;



typedef union qppm_vdmcfgr
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
        uint64_t vdm_vid_compare : 8;
        uint64_t vdm_overvolt : 4;
        uint64_t vdm_droop_small : 4;
        uint64_t vdm_droop_large : 4;
        uint64_t vdm_droop_xtreme : 4;
        uint64_t reserved1 : 8;
        uint64_t reserved2 : 32;
#else
        uint64_t reserved2 : 32;
        uint64_t reserved1 : 8;
        uint64_t vdm_droop_xtreme : 4;
        uint64_t vdm_droop_large : 4;
        uint64_t vdm_droop_small : 4;
        uint64_t vdm_overvolt : 4;
        uint64_t vdm_vid_compare : 8;
#endif // _BIG_ENDIAN
    } fields;
} qppm_vdmcfgr_t;



typedef union qppm_volt_char
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
    uint64_t vid_compare_max : 8;
    uint64_t vid_compare_min : 8;
        uint64_t ivrm_enabled_history : 1;
        uint64_t reserved1 : 47;
#else
        uint64_t reserved1 : 47;
        uint64_t ivrm_enabled_history : 1;
    uint64_t vid_compare_min : 8;
    uint64_t vid_compare_max : 8;
#endif // _BIG_ENDIAN
    } fields;
} qppm_volt_char_t;



typedef union qppm_qccr
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
        uint64_t l3_ex0_edram_enable_encode : 4;
        uint64_t l3_ex1_edram_enable_encode : 4;
        uint64_t spare_8_11 : 4;
        uint64_t droop_protect_data : 4;
        uint64_t force_droop_data : 4;
        uint64_t pulse_droop_data : 4;
        uint64_t pulse_droop_enable : 1;
        uint64_t reserved1 : 5;
        uint64_t pb_purge_req : 1;
        uint64_t pb_purge_done_lvl : 1;
        uint64_t l3_ex0_edram_enable_actual : 4;
        uint64_t l3_ex1_edram_enable_actual : 4;
        uint64_t l3_edram_seq_err : 1;
        uint64_t l3_edram_pgate_err : 1;
        uint64_t l3_ex0_edram_unlocked : 1;
        uint64_t l3_ex1_edram_unlocked : 1;
        uint64_t reserved2 : 20;
#else
        uint64_t reserved2 : 20;
        uint64_t l3_ex1_edram_unlocked : 1;
        uint64_t l3_ex0_edram_unlocked : 1;
        uint64_t l3_edram_pgate_err : 1;
        uint64_t l3_edram_seq_err : 1;
        uint64_t l3_ex1_edram_enable_actual : 4;
        uint64_t l3_ex0_edram_enable_actual : 4;
        uint64_t pb_purge_done_lvl : 1;
        uint64_t pb_purge_req : 1;
        uint64_t reserved1 : 5;
        uint64_t pulse_droop_enable : 1;
        uint64_t pulse_droop_data : 4;
        uint64_t force_droop_data : 4;
        uint64_t droop_protect_data : 4;
        uint64_t spare_8_11 : 4;
        uint64_t l3_ex1_edram_enable_encode : 4;
        uint64_t l3_ex0_edram_enable_encode : 4;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qccr_t;



typedef union qppm_qccr_clr
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
        uint64_t l3_ex0_edram_enable_encode : 4;
        uint64_t l3_ex1_edram_enable_encode : 4;
        uint64_t spare_8_11 : 4;
        uint64_t droop_protect_data : 4;
        uint64_t force_droop_data : 4;
        uint64_t pulse_droop_data : 4;
        uint64_t pulse_droop_enable : 1;
        uint64_t reserved1 : 5;
        uint64_t pb_purge_req : 1;
        uint64_t pb_purge_done_lvl : 1;
        uint64_t l3_ex0_edram_enable_actual : 4;
        uint64_t l3_ex1_edram_enable_actual : 4;
        uint64_t l3_edram_seq_err : 1;
        uint64_t l3_edram_pgate_err : 1;
        uint64_t l3_ex0_edram_unlocked : 1;
        uint64_t l3_ex1_edram_unlocked : 1;
        uint64_t reserved2 : 20;
#else
        uint64_t reserved2 : 20;
        uint64_t l3_ex1_edram_unlocked : 1;
        uint64_t l3_ex0_edram_unlocked : 1;
        uint64_t l3_edram_pgate_err : 1;
        uint64_t l3_edram_seq_err : 1;
        uint64_t l3_ex1_edram_enable_actual : 4;
        uint64_t l3_ex0_edram_enable_actual : 4;
        uint64_t pb_purge_done_lvl : 1;
        uint64_t pb_purge_req : 1;
        uint64_t reserved1 : 5;
        uint64_t pulse_droop_enable : 1;
        uint64_t pulse_droop_data : 4;
        uint64_t force_droop_data : 4;
        uint64_t droop_protect_data : 4;
        uint64_t spare_8_11 : 4;
        uint64_t l3_ex1_edram_enable_encode : 4;
        uint64_t l3_ex0_edram_enable_encode : 4;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qccr_clr_t;



typedef union qppm_qccr_or
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
        uint64_t l3_ex0_edram_enable_encode : 4;
        uint64_t l3_ex1_edram_enable_encode : 4;
        uint64_t spare_8_11 : 4;
        uint64_t droop_protect_data : 4;
        uint64_t force_droop_data : 4;
        uint64_t pulse_droop_data : 4;
        uint64_t pulse_droop_enable : 1;
        uint64_t reserved1 : 5;
        uint64_t pb_purge_req : 1;
        uint64_t pb_purge_done_lvl : 1;
        uint64_t l3_ex0_edram_enable_actual : 4;
        uint64_t l3_ex1_edram_enable_actual : 4;
        uint64_t l3_edram_seq_err : 1;
        uint64_t l3_edram_pgate_err : 1;
        uint64_t l3_ex0_edram_unlocked : 1;
        uint64_t l3_ex1_edram_unlocked : 1;
        uint64_t reserved2 : 20;
#else
        uint64_t reserved2 : 20;
        uint64_t l3_ex1_edram_unlocked : 1;
        uint64_t l3_ex0_edram_unlocked : 1;
        uint64_t l3_edram_pgate_err : 1;
        uint64_t l3_edram_seq_err : 1;
        uint64_t l3_ex1_edram_enable_actual : 4;
        uint64_t l3_ex0_edram_enable_actual : 4;
        uint64_t pb_purge_done_lvl : 1;
        uint64_t pb_purge_req : 1;
        uint64_t reserved1 : 5;
        uint64_t pulse_droop_enable : 1;
        uint64_t pulse_droop_data : 4;
        uint64_t force_droop_data : 4;
        uint64_t droop_protect_data : 4;
        uint64_t spare_8_11 : 4;
        uint64_t l3_ex1_edram_enable_encode : 4;
        uint64_t l3_ex0_edram_enable_encode : 4;
#endif // _BIG_ENDIAN
    } fields;
} qppm_qccr_or_t;


#endif // __ASSEMBLER__
#endif // __QPPM_FIRMWARE_REGISTERS_H__

