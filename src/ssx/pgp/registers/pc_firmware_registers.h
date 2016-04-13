/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/registers/pc_firmware_registers.h $               */
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
#ifndef __PC_FIRMWARE_REGISTERS_H__
#define __PC_FIRMWARE_REGISTERS_H__

/// \file pc_firmware_registers.h
/// \brief C register structs for the PC unit

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




typedef union pc_pfth_modereg {

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
    uint64_t pfth_cntr_dis : 1;
    uint64_t pfth_charac_mode : 1;
    uint64_t pfth_cntr_run_latch_gate_dis : 1;
    uint64_t sprd_pfth_tx_run_latches : 8;
    uint64_t tx_threads_stopped : 8;
    uint64_t _reserved0 : 45;
#else
    uint64_t _reserved0 : 45;
    uint64_t tx_threads_stopped : 8;
    uint64_t sprd_pfth_tx_run_latches : 8;
    uint64_t pfth_cntr_run_latch_gate_dis : 1;
    uint64_t pfth_charac_mode : 1;
    uint64_t pfth_cntr_dis : 1;
#endif // _BIG_ENDIAN
    } fields;
} pc_pfth_modereg_t;



typedef union pc_occ_sprc {

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
    uint64_t reserved0 : 53;
    uint64_t autoinc : 1;
    uint64_t sprn : 7;
    uint64_t reserved1 : 3;
#else
    uint64_t reserved1 : 3;
    uint64_t sprn : 7;
    uint64_t autoinc : 1;
    uint64_t reserved0 : 53;
#endif // _BIG_ENDIAN
    } fields;
} pc_occ_sprc_t;



typedef union pc_occ_sprd {

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
} pc_occ_sprd_t;



typedef union pc_pfth_oha_instr_cnt_sel {

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
    uint64_t _reserved0 : 62;
    uint64_t value : 2;
#else
    uint64_t value : 2;
    uint64_t _reserved0 : 62;
#endif // _BIG_ENDIAN
    } fields;
} pc_pfth_oha_instr_cnt_sel_t;



typedef union pc_pfth_throt_reg {

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
    uint64_t didt_trigger_enable : 1;
    uint64_t isu_trigger_enable : 1;
    uint64_t didt_throttle : 2;
    uint64_t uthrottle : 2;
    uint64_t force_suppress_speedup : 1;
    uint64_t suppress_speedup_on_throttle : 1;
    uint64_t core_slowdown : 1;
    uint64_t suppress_on_slowdown : 1;
    uint64_t isu_only_count_mode : 1;
    uint64_t spare : 5;
    uint64_t reserved : 48;
#else
    uint64_t reserved : 48;
    uint64_t spare : 5;
    uint64_t isu_only_count_mode : 1;
    uint64_t suppress_on_slowdown : 1;
    uint64_t core_slowdown : 1;
    uint64_t suppress_speedup_on_throttle : 1;
    uint64_t force_suppress_speedup : 1;
    uint64_t uthrottle : 2;
    uint64_t didt_throttle : 2;
    uint64_t isu_trigger_enable : 1;
    uint64_t didt_trigger_enable : 1;
#endif // _BIG_ENDIAN
    } fields;
} pc_pfth_throt_reg_t;



typedef union pc_direct_controln {

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
    uint64_t reserved0 : 42;
    uint64_t dc_prestart_sleep : 1;
    uint64_t dc_prestart_winkle : 1;
    uint64_t dc_clear_maint : 1;
    uint64_t dc_ntc_flush : 1;
    uint64_t reserved46 : 1;
    uint64_t dc_prestart_nap : 1;
    uint64_t dc_cancel_lost : 1;
    uint64_t dc_reset_maint : 1;
    uint64_t reserved50 : 1;
    uint64_t dc_set_maint : 1;
    uint64_t dc_goto_quiesce_state : 1;
    uint64_t reserved53 : 1;
    uint64_t dc_inj_test_hang : 2;
    uint64_t dc_core_running : 1;
    uint64_t dc_hang_inject : 1;
    uint64_t dc_smt_start_suppress : 1;
    uint64_t reserved59 : 1;
    uint64_t dc_sreset_request : 1;
    uint64_t dc_core_step : 1;
    uint64_t dc_core_start : 1;
    uint64_t dc_core_stop : 1;
#else
    uint64_t dc_core_stop : 1;
    uint64_t dc_core_start : 1;
    uint64_t dc_core_step : 1;
    uint64_t dc_sreset_request : 1;
    uint64_t reserved59 : 1;
    uint64_t dc_smt_start_suppress : 1;
    uint64_t dc_hang_inject : 1;
    uint64_t dc_core_running : 1;
    uint64_t dc_inj_test_hang : 2;
    uint64_t reserved53 : 1;
    uint64_t dc_goto_quiesce_state : 1;
    uint64_t dc_set_maint : 1;
    uint64_t reserved50 : 1;
    uint64_t dc_reset_maint : 1;
    uint64_t dc_cancel_lost : 1;
    uint64_t dc_prestart_nap : 1;
    uint64_t reserved46 : 1;
    uint64_t dc_ntc_flush : 1;
    uint64_t dc_clear_maint : 1;
    uint64_t dc_prestart_winkle : 1;
    uint64_t dc_prestart_sleep : 1;
    uint64_t reserved0 : 42;
#endif // _BIG_ENDIAN
    } fields;
} pc_direct_controln_t;



typedef union pc_ras_moderegn {

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
    uint64_t reserved0 : 43;
    uint64_t mr_thread_in_debug_mode : 1;
    uint64_t mr_pmon_inhibit : 1;
    uint64_t mr_fence_interrupts : 1;
    uint64_t mr_stop_fetch : 1;
    uint64_t mr_stop_prefetch : 1;
    uint64_t mr_stop_dispatch : 1;
    uint64_t mr_single_decode : 1;
    uint64_t mr_do_single_mode : 1;
    uint64_t mr_one_ppc_mode : 1;
    uint64_t mr_hang_test_ctrl : 2;
    uint64_t mr_attempt_gps_hr : 3;
    uint64_t mr_hang_dis : 1;
    uint64_t mr_on_corehng : 1;
    uint64_t mr_on_ambihng : 1;
    uint64_t mr_on_nesthng : 1;
    uint64_t mr_recov_enable : 1;
    uint64_t mr_block_hmi_on_maint : 1;
    uint64_t mr_fence_intr_on_checkstop : 1;
#else
    uint64_t mr_fence_intr_on_checkstop : 1;
    uint64_t mr_block_hmi_on_maint : 1;
    uint64_t mr_recov_enable : 1;
    uint64_t mr_on_nesthng : 1;
    uint64_t mr_on_ambihng : 1;
    uint64_t mr_on_corehng : 1;
    uint64_t mr_hang_dis : 1;
    uint64_t mr_attempt_gps_hr : 3;
    uint64_t mr_hang_test_ctrl : 2;
    uint64_t mr_one_ppc_mode : 1;
    uint64_t mr_do_single_mode : 1;
    uint64_t mr_single_decode : 1;
    uint64_t mr_stop_dispatch : 1;
    uint64_t mr_stop_prefetch : 1;
    uint64_t mr_stop_fetch : 1;
    uint64_t mr_fence_interrupts : 1;
    uint64_t mr_pmon_inhibit : 1;
    uint64_t mr_thread_in_debug_mode : 1;
    uint64_t reserved0 : 43;
#endif // _BIG_ENDIAN
    } fields;
} pc_ras_moderegn_t;



typedef union pc_ras_statusn {

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
    uint64_t quiesce_status : 20;
    uint64_t reserved20 : 1;
    uint64_t reserved21 : 1;
    uint64_t reserved22 : 1;
    uint64_t other_thread_active : 1;
    uint64_t hang_fsm : 3;
    uint64_t reserved27 : 1;
    uint64_t hang_hist0 : 1;
    uint64_t hang_hist1 : 1;
    uint64_t hang_hist2 : 1;
    uint64_t hang_hist3 : 1;
    uint64_t reserved32 : 1;
    uint64_t hr_comp_cnt : 8;
    uint64_t smt_dead_stop : 1;
    uint64_t stop_fetch : 1;
    uint64_t stop_dispatch : 1;
    uint64_t stop_completion : 1;
    uint64_t hold_decode : 1;
    uint64_t reserved46 : 1;
    uint64_t reserved47 : 1;
    uint64_t thread_enabled : 1;
    uint64_t pow_status_thread_state : 4;
    uint64_t reserved53 : 1;
    uint64_t maint_single_mode : 1;
    uint64_t reserved55 : 1;
    uint64_t reserved56 : 1;
    uint64_t reserved57 : 7;
#else
    uint64_t reserved57 : 7;
    uint64_t reserved56 : 1;
    uint64_t reserved55 : 1;
    uint64_t maint_single_mode : 1;
    uint64_t reserved53 : 1;
    uint64_t pow_status_thread_state : 4;
    uint64_t thread_enabled : 1;
    uint64_t reserved47 : 1;
    uint64_t reserved46 : 1;
    uint64_t hold_decode : 1;
    uint64_t stop_completion : 1;
    uint64_t stop_dispatch : 1;
    uint64_t stop_fetch : 1;
    uint64_t smt_dead_stop : 1;
    uint64_t hr_comp_cnt : 8;
    uint64_t reserved32 : 1;
    uint64_t hang_hist3 : 1;
    uint64_t hang_hist2 : 1;
    uint64_t hang_hist1 : 1;
    uint64_t hang_hist0 : 1;
    uint64_t reserved27 : 1;
    uint64_t hang_fsm : 3;
    uint64_t other_thread_active : 1;
    uint64_t reserved22 : 1;
    uint64_t reserved21 : 1;
    uint64_t reserved20 : 1;
    uint64_t quiesce_status : 20;
#endif // _BIG_ENDIAN
    } fields;
} pc_ras_statusn_t;



typedef union pc_pow_statusn {

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
    uint64_t thread_state : 4;
    uint64_t thread_pow_state : 2;
    uint64_t smt_mode : 3;
    uint64_t hmi_intr : 1;
    uint64_t maybe_ext_intr : 1;
    uint64_t decr_intr : 1;
    uint64_t maybe_debug_intr : 1;
    uint64_t hdec_intr : 1;
    uint64_t maybe_pmu_intr : 1;
    uint64_t sp_attn_intr : 1;
    uint64_t sreset_type : 3;
    uint64_t reserved19 : 1;
    uint64_t sreset_pending : 1;
    uint64_t debug_fetch_stop : 1;
    uint64_t async_pending : 1;
    uint64_t core_pow_state : 3;
    uint64_t reserved26 : 3;
    uint64_t _reserved0 : 35;
#else
    uint64_t _reserved0 : 35;
    uint64_t reserved26 : 3;
    uint64_t core_pow_state : 3;
    uint64_t async_pending : 1;
    uint64_t debug_fetch_stop : 1;
    uint64_t sreset_pending : 1;
    uint64_t reserved19 : 1;
    uint64_t sreset_type : 3;
    uint64_t sp_attn_intr : 1;
    uint64_t maybe_pmu_intr : 1;
    uint64_t hdec_intr : 1;
    uint64_t maybe_debug_intr : 1;
    uint64_t decr_intr : 1;
    uint64_t maybe_ext_intr : 1;
    uint64_t hmi_intr : 1;
    uint64_t smt_mode : 3;
    uint64_t thread_pow_state : 2;
    uint64_t thread_state : 4;
#endif // _BIG_ENDIAN
    } fields;
} pc_pow_statusn_t;


#endif // __ASSEMBLER__
#endif // __PC_FIRMWARE_REGISTERS_H__

