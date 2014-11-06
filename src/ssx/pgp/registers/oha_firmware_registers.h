#ifndef __OHA_FIRMWARE_REGISTERS_H__
#define __OHA_FIRMWARE_REGISTERS_H__

// $Id: oha_firmware_registers.h,v 1.1.1.1 2013/12/11 21:03:23 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/registers/oha_firmware_registers.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file oha_firmware_registers.h
/// \brief C register structs for the OHA unit

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




typedef union oha_activity_sample_mode_reg {

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
    uint64_t enable_activity_sampling : 1;
    uint64_t enable_ppt_trace : 1;
    uint64_t l2_act_count_is_free_running : 1;
    uint64_t l3_act_count_is_free_running : 1;
    uint64_t activity_sample_l2l3_enable : 1;
    uint64_t core_activity_sample_enable : 1;
    uint64_t disable_activity_proxy_reset : 1;
    uint64_t power_proxy_activity_range_select_vcs : 5;
    uint64_t power_proxy_activity_range_select_vdd : 5;
    uint64_t memory_activity_range_select : 4;
    uint64_t avg_freq_counter_scaler : 3;
    uint64_t ppt_trace_timer_match_val : 11;
    uint64_t disable_ppt_int_timer_reset : 1;
    uint64_t ppt_int_timer_select : 2;
    uint64_t disable_ppt_cycle_counter_reset : 1;
    uint64_t ppt_cycle_counter_scaler : 3;
    uint64_t ppt_squash_timer_match_val : 6;
    uint64_t ppt_timer_timeout_enable : 1;
    uint64_t ppt_lpar_change_enable : 1;
    uint64_t ppt_global_actual_change_enable : 1;
    uint64_t ppt_local_voltage_change_enable : 1;
    uint64_t ppt_ivrm_bypass_change_enable : 1;
    uint64_t ppt_idle_entry_enable : 1;
    uint64_t ppt_idle_exit_enable : 1;
    uint64_t ppt_timer_timeout_priority : 1;
    uint64_t ppt_lpar_change_priority : 1;
    uint64_t ppt_global_actual_change_priority : 1;
    uint64_t ppt_local_voltage_change_priority : 1;
    uint64_t ppt_ivrm_bypass_change_priority : 1;
    uint64_t ppt_idle_entry_priority : 1;
    uint64_t ppt_idle_exit_priority : 1;
    uint64_t ppt_legacy_mode : 1;
    uint64_t _reserved0 : 1;
#else
    uint64_t _reserved0 : 1;
    uint64_t ppt_legacy_mode : 1;
    uint64_t ppt_idle_exit_priority : 1;
    uint64_t ppt_idle_entry_priority : 1;
    uint64_t ppt_ivrm_bypass_change_priority : 1;
    uint64_t ppt_local_voltage_change_priority : 1;
    uint64_t ppt_global_actual_change_priority : 1;
    uint64_t ppt_lpar_change_priority : 1;
    uint64_t ppt_timer_timeout_priority : 1;
    uint64_t ppt_idle_exit_enable : 1;
    uint64_t ppt_idle_entry_enable : 1;
    uint64_t ppt_ivrm_bypass_change_enable : 1;
    uint64_t ppt_local_voltage_change_enable : 1;
    uint64_t ppt_global_actual_change_enable : 1;
    uint64_t ppt_lpar_change_enable : 1;
    uint64_t ppt_timer_timeout_enable : 1;
    uint64_t ppt_squash_timer_match_val : 6;
    uint64_t ppt_cycle_counter_scaler : 3;
    uint64_t disable_ppt_cycle_counter_reset : 1;
    uint64_t ppt_int_timer_select : 2;
    uint64_t disable_ppt_int_timer_reset : 1;
    uint64_t ppt_trace_timer_match_val : 11;
    uint64_t avg_freq_counter_scaler : 3;
    uint64_t memory_activity_range_select : 4;
    uint64_t power_proxy_activity_range_select_vdd : 5;
    uint64_t power_proxy_activity_range_select_vcs : 5;
    uint64_t disable_activity_proxy_reset : 1;
    uint64_t core_activity_sample_enable : 1;
    uint64_t activity_sample_l2l3_enable : 1;
    uint64_t l3_act_count_is_free_running : 1;
    uint64_t l2_act_count_is_free_running : 1;
    uint64_t enable_ppt_trace : 1;
    uint64_t enable_activity_sampling : 1;
#endif // _BIG_ENDIAN
    } fields;
} oha_activity_sample_mode_reg_t;



typedef union oha_vcs_activity_cnt_reg {

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
    uint64_t l2_activity_count_24bit_vcs : 24;
    uint64_t l3_activity_count_24bit_vcs : 24;
    uint64_t _reserved0 : 16;
#else
    uint64_t _reserved0 : 16;
    uint64_t l3_activity_count_24bit_vcs : 24;
    uint64_t l2_activity_count_24bit_vcs : 24;
#endif // _BIG_ENDIAN
    } fields;
} oha_vcs_activity_cnt_reg_t;



typedef union oha_vdd_activity_cnt_reg {

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
    uint64_t l2_activity_count_24bit_vdd : 24;
    uint64_t l3_activity_count_24bit_vdd : 24;
    uint64_t _reserved0 : 16;
#else
    uint64_t _reserved0 : 16;
    uint64_t l3_activity_count_24bit_vdd : 24;
    uint64_t l2_activity_count_24bit_vdd : 24;
#endif // _BIG_ENDIAN
    } fields;
} oha_vdd_activity_cnt_reg_t;



typedef union oha_low_activity_detect_mode_reg {

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
    uint64_t low_activity_detect_sample_enable : 1;
    uint64_t low_activity_detect_timer_select_for_entry : 8;
    uint64_t low_activity_detect_timer_select_for_exit : 8;
    uint64_t low_activity_detect_threshold_range : 4;
    uint64_t low_activity_detect_threshold_entry : 16;
    uint64_t low_activity_detect_threshold_exit : 16;
    uint64_t _reserved0 : 11;
#else
    uint64_t _reserved0 : 11;
    uint64_t low_activity_detect_threshold_exit : 16;
    uint64_t low_activity_detect_threshold_entry : 16;
    uint64_t low_activity_detect_threshold_range : 4;
    uint64_t low_activity_detect_timer_select_for_exit : 8;
    uint64_t low_activity_detect_timer_select_for_entry : 8;
    uint64_t low_activity_detect_sample_enable : 1;
#endif // _BIG_ENDIAN
    } fields;
} oha_low_activity_detect_mode_reg_t;



typedef union oha_activity_and_frequ_reg {

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
    uint64_t low_activity_detect_engaged : 1;
    uint64_t _reserved0 : 63;
#else
    uint64_t _reserved0 : 63;
    uint64_t low_activity_detect_engaged : 1;
#endif // _BIG_ENDIAN
    } fields;
} oha_activity_and_frequ_reg_t;



typedef union oha_counter_reg {

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
    uint64_t base_counter : 16;
    uint64_t idle_detec_timer : 16;
    uint64_t tod_count_msbs : 16;
    uint64_t ppt_cycle_count_ovfl : 1;
    uint64_t ppt_parity_error : 1;
    uint64_t _reserved0 : 14;
#else
    uint64_t _reserved0 : 14;
    uint64_t ppt_parity_error : 1;
    uint64_t ppt_cycle_count_ovfl : 1;
    uint64_t tod_count_msbs : 16;
    uint64_t idle_detec_timer : 16;
    uint64_t base_counter : 16;
#endif // _BIG_ENDIAN
    } fields;
} oha_counter_reg_t;



typedef union oha_proxy_reg {

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
    uint64_t average_frequency : 32;
    uint64_t special_memory_activity_cnt : 24;
    uint64_t _reserved0 : 8;
#else
    uint64_t _reserved0 : 8;
    uint64_t special_memory_activity_cnt : 24;
    uint64_t average_frequency : 32;
#endif // _BIG_ENDIAN
    } fields;
} oha_proxy_reg_t;



typedef union oha_proxy_legacy_reg {

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
    uint64_t aproxy_vdd : 16;
    uint64_t aproxy_vcs : 16;
    uint64_t memory_activity_cnt : 16;
    uint64_t scaled_average_frequency : 16;
#else
    uint64_t scaled_average_frequency : 16;
    uint64_t memory_activity_cnt : 16;
    uint64_t aproxy_vcs : 16;
    uint64_t aproxy_vdd : 16;
#endif // _BIG_ENDIAN
    } fields;
} oha_proxy_legacy_reg_t;



typedef union oha_skitter_ctrl_mode_reg {

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
    uint64_t start_skitter_mux_sel : 3;
    uint64_t stop_skitter_mux_sel : 3;
    uint64_t skitter_timer_start_mux_sel : 3;
    uint64_t disable_skitter_qualification_mode : 1;
    uint64_t skitter_timer_enable_freerun_mode : 1;
    uint64_t skitter_timer_range_select : 4;
    uint64_t _reserved0 : 49;
#else
    uint64_t _reserved0 : 49;
    uint64_t skitter_timer_range_select : 4;
    uint64_t skitter_timer_enable_freerun_mode : 1;
    uint64_t disable_skitter_qualification_mode : 1;
    uint64_t skitter_timer_start_mux_sel : 3;
    uint64_t stop_skitter_mux_sel : 3;
    uint64_t start_skitter_mux_sel : 3;
#endif // _BIG_ENDIAN
    } fields;
} oha_skitter_ctrl_mode_reg_t;



typedef union oha_cpm_ctrl_reg {

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
    uint64_t cpm_bit_sel : 2;
    uint64_t cpm_bit_sel_trig_0 : 3;
    uint64_t cpm_bit_sel_trig_1 : 3;
    uint64_t scom_marker : 8;
    uint64_t cpm_mark_select : 2;
    uint64_t cpm_htm_mode : 1;
    uint64_t cpm_scom_mask : 8;
    uint64_t cpm_scom_mode : 2;
    uint64_t cpm_data_mode : 1;
    uint64_t _reserved0 : 34;
#else
    uint64_t _reserved0 : 34;
    uint64_t cpm_data_mode : 1;
    uint64_t cpm_scom_mode : 2;
    uint64_t cpm_scom_mask : 8;
    uint64_t cpm_htm_mode : 1;
    uint64_t cpm_mark_select : 2;
    uint64_t scom_marker : 8;
    uint64_t cpm_bit_sel_trig_1 : 3;
    uint64_t cpm_bit_sel_trig_0 : 3;
    uint64_t cpm_bit_sel : 2;
#endif // _BIG_ENDIAN
    } fields;
} oha_cpm_ctrl_reg_t;



typedef union oha_cpm_hist_reset_reg {

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
    uint64_t hist_reset : 1;
    uint64_t pconly_special_wakeup : 1;
    uint64_t _reserved0 : 62;
#else
    uint64_t _reserved0 : 62;
    uint64_t pconly_special_wakeup : 1;
    uint64_t hist_reset : 1;
#endif // _BIG_ENDIAN
    } fields;
} oha_cpm_hist_reset_reg_t;

#endif // __ASSEMBLER__
#define OHA_CPM_HIST_RESET_REG_HIST_RESET SIXTYFOUR_BIT_CONSTANT(0x8000000000000000)
#define OHA_CPM_HIST_RESET_REG_PCONLY_SPECIAL_WAKEUP SIXTYFOUR_BIT_CONSTANT(0x4000000000000000)
#ifndef __ASSEMBLER__


typedef union oha_ro_status_reg {

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
    uint64_t low_activity_detect_bit : 1;
    uint64_t special_wakeup_completed : 1;
    uint64_t architected_idle_state_from_core : 3;
    uint64_t core_access_impossible : 1;
    uint64_t eco_access_impossible : 1;
    uint64_t spare_6bit : 6;
    uint64_t current_aiss_fsm_state_vector : 7;
    uint64_t eff_idle_state : 3;
    uint64_t spare_1bit : 1;
    uint64_t pc_tc_deep_idle_thread_state : 8;
    uint64_t lpar_id : 12;
    uint64_t ppt_fsm_l : 4;
    uint64_t _reserved0 : 16;
#else
    uint64_t _reserved0 : 16;
    uint64_t ppt_fsm_l : 4;
    uint64_t lpar_id : 12;
    uint64_t pc_tc_deep_idle_thread_state : 8;
    uint64_t spare_1bit : 1;
    uint64_t eff_idle_state : 3;
    uint64_t current_aiss_fsm_state_vector : 7;
    uint64_t spare_6bit : 6;
    uint64_t eco_access_impossible : 1;
    uint64_t core_access_impossible : 1;
    uint64_t architected_idle_state_from_core : 3;
    uint64_t special_wakeup_completed : 1;
    uint64_t low_activity_detect_bit : 1;
#endif // _BIG_ENDIAN
    } fields;
} oha_ro_status_reg_t;

#endif // __ASSEMBLER__
#define OHA_RO_STATUS_REG_LOW_ACTIVITY_DETECT_BIT SIXTYFOUR_BIT_CONSTANT(0x8000000000000000)
#define OHA_RO_STATUS_REG_SPECIAL_WAKEUP_COMPLETED SIXTYFOUR_BIT_CONSTANT(0x4000000000000000)
#define OHA_RO_STATUS_REG_ARCHITECTED_IDLE_STATE_FROM_CORE_MASK SIXTYFOUR_BIT_CONSTANT(0x3800000000000000)
#define OHA_RO_STATUS_REG_CORE_ACCESS_IMPOSSIBLE SIXTYFOUR_BIT_CONSTANT(0x0400000000000000)
#define OHA_RO_STATUS_REG_ECO_ACCESS_IMPOSSIBLE SIXTYFOUR_BIT_CONSTANT(0x0200000000000000)
#define OHA_RO_STATUS_REG_SPARE_6BIT_MASK SIXTYFOUR_BIT_CONSTANT(0x01f8000000000000)
#define OHA_RO_STATUS_REG_CURRENT_AISS_FSM_STATE_VECTOR_MASK SIXTYFOUR_BIT_CONSTANT(0x0007f00000000000)
#define OHA_RO_STATUS_REG_EFF_IDLE_STATE_MASK SIXTYFOUR_BIT_CONSTANT(0x00000e0000000000)
#define OHA_RO_STATUS_REG_SPARE_1BIT SIXTYFOUR_BIT_CONSTANT(0x0000010000000000)
#define OHA_RO_STATUS_REG_PC_TC_DEEP_IDLE_THREAD_STATE_MASK SIXTYFOUR_BIT_CONSTANT(0x000000ff00000000)
#define OHA_RO_STATUS_REG_LPAR_ID_MASK SIXTYFOUR_BIT_CONSTANT(0x00000000fff00000)
#define OHA_RO_STATUS_REG_PPT_FSM_L_MASK SIXTYFOUR_BIT_CONSTANT(0x00000000000f0000)
#ifndef __ASSEMBLER__


typedef union oha_mode_reg {

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
    uint64_t enable_ignore_recov_errors : 1;
    uint64_t enable_arch_idle_mode_sequencer : 1;
    uint64_t treat_sleep_as_nap : 1;
    uint64_t treat_winkle_as_sleep : 1;
    uint64_t enable_pstate_tracing : 1;
    uint64_t enable_suppress_purges_and_pcb_fence : 1;
    uint64_t idle_state_override_en : 1;
    uint64_t idle_state_override_value : 3;
    uint64_t disable_aiss_core_handshake : 1;
    uint64_t aiss_hang_detect_timer_sel : 4;
    uint64_t enable_l2_purge_abort : 1;
    uint64_t enable_l3_purge_abort : 1;
    uint64_t tod_pulse_count_match_val : 14;
    uint64_t trace_debug_mode_select : 2;
    uint64_t lpft_mode : 1;
    uint64_t _reserved0 : 30;
#else
    uint64_t _reserved0 : 30;
    uint64_t lpft_mode : 1;
    uint64_t trace_debug_mode_select : 2;
    uint64_t tod_pulse_count_match_val : 14;
    uint64_t enable_l3_purge_abort : 1;
    uint64_t enable_l2_purge_abort : 1;
    uint64_t aiss_hang_detect_timer_sel : 4;
    uint64_t disable_aiss_core_handshake : 1;
    uint64_t idle_state_override_value : 3;
    uint64_t idle_state_override_en : 1;
    uint64_t enable_suppress_purges_and_pcb_fence : 1;
    uint64_t enable_pstate_tracing : 1;
    uint64_t treat_winkle_as_sleep : 1;
    uint64_t treat_sleep_as_nap : 1;
    uint64_t enable_arch_idle_mode_sequencer : 1;
    uint64_t enable_ignore_recov_errors : 1;
#endif // _BIG_ENDIAN
    } fields;
} oha_mode_reg_t;



typedef union oha_error_and_error_mask_reg {

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
    uint64_t oha_error_mask : 8;
    uint64_t oha_chiplet_errors : 8;
    uint64_t _reserved0 : 48;
#else
    uint64_t _reserved0 : 48;
    uint64_t oha_chiplet_errors : 8;
    uint64_t oha_error_mask : 8;
#endif // _BIG_ENDIAN
    } fields;
} oha_error_and_error_mask_reg_t;



typedef union oha_arch_idle_state_reg {

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
    uint64_t aiss_thold_sequence_select : 1;
    uint64_t disable_waiting_on_l3 : 1;
    uint64_t idle_seq_timer_select : 2;
    uint64_t allow_aiss_interrupts : 1;
    uint64_t enable_reset_of_counters_while_sleepwinkle : 1;
    uint64_t select_p7p_seq_wait_time : 1;
    uint64_t disable_auto_sleep_entry : 1;
    uint64_t disable_auto_winkle_entry : 1;
    uint64_t reset_idle_state_sequencer : 1;
    uint64_t _reserved0 : 54;
#else
    uint64_t _reserved0 : 54;
    uint64_t reset_idle_state_sequencer : 1;
    uint64_t disable_auto_winkle_entry : 1;
    uint64_t disable_auto_sleep_entry : 1;
    uint64_t select_p7p_seq_wait_time : 1;
    uint64_t enable_reset_of_counters_while_sleepwinkle : 1;
    uint64_t allow_aiss_interrupts : 1;
    uint64_t idle_seq_timer_select : 2;
    uint64_t disable_waiting_on_l3 : 1;
    uint64_t aiss_thold_sequence_select : 1;
#endif // _BIG_ENDIAN
    } fields;
} oha_arch_idle_state_reg_t;



typedef union oha_pmu_config_reg {

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
    uint64_t pmu_pstate_threshold_a : 8;
    uint64_t pmu_pstate_threshold_b : 8;
    uint64_t pmu_configuration : 3;
    uint64_t _reserved0 : 45;
#else
    uint64_t _reserved0 : 45;
    uint64_t pmu_configuration : 3;
    uint64_t pmu_pstate_threshold_b : 8;
    uint64_t pmu_pstate_threshold_a : 8;
#endif // _BIG_ENDIAN
    } fields;
} oha_pmu_config_reg_t;



typedef union oha_aiss_io_reg {

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
    uint64_t spare_2bits_b : 2;
    uint64_t tc_tp_chiplet_pm_state : 4;
    uint64_t tc_pb_sleep : 1;
    uint64_t tc_tc_pm_thold_ctrl : 3;
    uint64_t tc_l3_fence_lco : 1;
    uint64_t tc_ncu_fence : 1;
    uint64_t chksw_hw237039dis : 1;
    uint64_t tc_l3_init_dram : 1;
    uint64_t tc_pb_purge : 1;
    uint64_t tc_pc_pm_wake_up : 1;
    uint64_t spare_entry_for_config_bit : 1;
    uint64_t reset_of_counters_while_sleepwinkle : 1;
    uint64_t tc_chtm_purge : 1;
    uint64_t tc_tp_terminate_pcb : 1;
    uint64_t tc_oha_therm_purge_lvl : 1;
    uint64_t pscom_core_fence_lvl : 1;
    uint64_t pb_eco_fence_lvl : 1;
    uint64_t core2cache_fence_req : 1;
    uint64_t cache2core_fence_req : 1;
    uint64_t pervasive_eco_fence_req : 1;
    uint64_t tc_oha_pmx_fence_req_lvl_l : 1;
    uint64_t updateohafreq : 1;
    uint64_t req_idle_state_change : 1;
    uint64_t tc_l2_purge : 1;
    uint64_t tc_l3_purge : 1;
    uint64_t tc_ncu_purge : 1;
    uint64_t tc_l2_purge_abort : 1;
    uint64_t tc_l3_purge_abort : 1;
    uint64_t pc_tc_pm_state : 3;
    uint64_t l2_purge_is_done : 1;
    uint64_t l3_ncu_chtm_purge_done : 3;
    uint64_t tc_tc_xstop_err : 1;
    uint64_t tc_tc_recov_err : 1;
    uint64_t pb_tc_purge_active_lvl : 1;
    uint64_t l3_tc_dram_ready_lvl : 1;
    uint64_t core_fsm_non_idle : 1;
    uint64_t tc_pscom_core_fence_done : 1;
    uint64_t tc_pmx_oha_fence_done : 1;
    uint64_t l2_purge_abort_sticky : 1;
    uint64_t l3_purge_abort_sticky : 1;
    uint64_t _reserved0 : 14;
#else
    uint64_t _reserved0 : 14;
    uint64_t l3_purge_abort_sticky : 1;
    uint64_t l2_purge_abort_sticky : 1;
    uint64_t tc_pmx_oha_fence_done : 1;
    uint64_t tc_pscom_core_fence_done : 1;
    uint64_t core_fsm_non_idle : 1;
    uint64_t l3_tc_dram_ready_lvl : 1;
    uint64_t pb_tc_purge_active_lvl : 1;
    uint64_t tc_tc_recov_err : 1;
    uint64_t tc_tc_xstop_err : 1;
    uint64_t l3_ncu_chtm_purge_done : 3;
    uint64_t l2_purge_is_done : 1;
    uint64_t pc_tc_pm_state : 3;
    uint64_t tc_l3_purge_abort : 1;
    uint64_t tc_l2_purge_abort : 1;
    uint64_t tc_ncu_purge : 1;
    uint64_t tc_l3_purge : 1;
    uint64_t tc_l2_purge : 1;
    uint64_t req_idle_state_change : 1;
    uint64_t updateohafreq : 1;
    uint64_t tc_oha_pmx_fence_req_lvl_l : 1;
    uint64_t pervasive_eco_fence_req : 1;
    uint64_t cache2core_fence_req : 1;
    uint64_t core2cache_fence_req : 1;
    uint64_t pb_eco_fence_lvl : 1;
    uint64_t pscom_core_fence_lvl : 1;
    uint64_t tc_oha_therm_purge_lvl : 1;
    uint64_t tc_tp_terminate_pcb : 1;
    uint64_t tc_chtm_purge : 1;
    uint64_t reset_of_counters_while_sleepwinkle : 1;
    uint64_t spare_entry_for_config_bit : 1;
    uint64_t tc_pc_pm_wake_up : 1;
    uint64_t tc_pb_purge : 1;
    uint64_t tc_l3_init_dram : 1;
    uint64_t chksw_hw237039dis : 1;
    uint64_t tc_ncu_fence : 1;
    uint64_t tc_l3_fence_lco : 1;
    uint64_t tc_tc_pm_thold_ctrl : 3;
    uint64_t tc_pb_sleep : 1;
    uint64_t tc_tp_chiplet_pm_state : 4;
    uint64_t spare_2bits_b : 2;
#endif // _BIG_ENDIAN
    } fields;
} oha_aiss_io_reg_t;



typedef union oha_ppt_bar_reg {

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
    uint64_t ppt_bar : 46;
    uint64_t ppt_size_mask : 7;
    uint64_t ppt_address_scope : 3;
    uint64_t _reserved0 : 8;
#else
    uint64_t _reserved0 : 8;
    uint64_t ppt_address_scope : 3;
    uint64_t ppt_size_mask : 7;
    uint64_t ppt_bar : 46;
#endif // _BIG_ENDIAN
    } fields;
} oha_ppt_bar_reg_t;



typedef union oha_l2_vcs_directory_read_weight {

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
    uint64_t value : 6;
    uint64_t _reserved0 : 58;
#else
    uint64_t _reserved0 : 58;
    uint64_t value : 6;
#endif // _BIG_ENDIAN
    } fields;
} oha_l2_vcs_directory_read_weight_t;



typedef union oha_l2_vcs_directory_write_weight {

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
    uint64_t value : 6;
    uint64_t _reserved0 : 58;
#else
    uint64_t _reserved0 : 58;
    uint64_t value : 6;
#endif // _BIG_ENDIAN
    } fields;
} oha_l2_vcs_directory_write_weight_t;



typedef union oha_l2_vcs_cache_full_read_weight {

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
    uint64_t value : 6;
    uint64_t _reserved0 : 58;
#else
    uint64_t _reserved0 : 58;
    uint64_t value : 6;
#endif // _BIG_ENDIAN
    } fields;
} oha_l2_vcs_cache_full_read_weight_t;



typedef union oha_l2_vcs_cache_targeted_read_weight {

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
    uint64_t value : 6;
    uint64_t _reserved0 : 58;
#else
    uint64_t _reserved0 : 58;
    uint64_t value : 6;
#endif // _BIG_ENDIAN
    } fields;
} oha_l2_vcs_cache_targeted_read_weight_t;



typedef union oha_l2_vcs_cache_write_cnt_weight {

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
    uint64_t value : 6;
    uint64_t _reserved0 : 58;
#else
    uint64_t _reserved0 : 58;
    uint64_t value : 6;
#endif // _BIG_ENDIAN
    } fields;
} oha_l2_vcs_cache_write_cnt_weight_t;



typedef union oha_l3_vcs_directory_read_weight {

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
    uint64_t value : 6;
    uint64_t _reserved0 : 58;
#else
    uint64_t _reserved0 : 58;
    uint64_t value : 6;
#endif // _BIG_ENDIAN
    } fields;
} oha_l3_vcs_directory_read_weight_t;



typedef union oha_l3_vcs_directory_write_weight {

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
    uint64_t value : 6;
    uint64_t _reserved0 : 58;
#else
    uint64_t _reserved0 : 58;
    uint64_t value : 6;
#endif // _BIG_ENDIAN
    } fields;
} oha_l3_vcs_directory_write_weight_t;



typedef union oha_l3_vcs_cache_access_weight {

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
    uint64_t value : 6;
    uint64_t _reserved0 : 58;
#else
    uint64_t _reserved0 : 58;
    uint64_t value : 6;
#endif // _BIG_ENDIAN
    } fields;
} oha_l3_vcs_cache_access_weight_t;



typedef union oha_l2_vdd_directory_read_weight {

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
    uint64_t value : 6;
    uint64_t _reserved0 : 58;
#else
    uint64_t _reserved0 : 58;
    uint64_t value : 6;
#endif // _BIG_ENDIAN
    } fields;
} oha_l2_vdd_directory_read_weight_t;



typedef union oha_l2_vdd_directory_write_weight {

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
    uint64_t value : 6;
    uint64_t _reserved0 : 58;
#else
    uint64_t _reserved0 : 58;
    uint64_t value : 6;
#endif // _BIG_ENDIAN
    } fields;
} oha_l2_vdd_directory_write_weight_t;



typedef union oha_l2_vdd_cache_full_read_weight {

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
    uint64_t value : 6;
    uint64_t _reserved0 : 58;
#else
    uint64_t _reserved0 : 58;
    uint64_t value : 6;
#endif // _BIG_ENDIAN
    } fields;
} oha_l2_vdd_cache_full_read_weight_t;



typedef union oha_l2_vdd_cache_targeted_read_weight {

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
    uint64_t value : 6;
    uint64_t _reserved0 : 58;
#else
    uint64_t _reserved0 : 58;
    uint64_t value : 6;
#endif // _BIG_ENDIAN
    } fields;
} oha_l2_vdd_cache_targeted_read_weight_t;



typedef union oha_l2_vdd_cache_write_cnt_weight {

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
    uint64_t value : 6;
    uint64_t _reserved0 : 58;
#else
    uint64_t _reserved0 : 58;
    uint64_t value : 6;
#endif // _BIG_ENDIAN
    } fields;
} oha_l2_vdd_cache_write_cnt_weight_t;



typedef union oha_l3_vdd_directory_read_weight {

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
    uint64_t value : 6;
    uint64_t _reserved0 : 58;
#else
    uint64_t _reserved0 : 58;
    uint64_t value : 6;
#endif // _BIG_ENDIAN
    } fields;
} oha_l3_vdd_directory_read_weight_t;



typedef union oha_l3_vdd_directory_write_weight {

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
    uint64_t value : 6;
    uint64_t _reserved0 : 58;
#else
    uint64_t _reserved0 : 58;
    uint64_t value : 6;
#endif // _BIG_ENDIAN
    } fields;
} oha_l3_vdd_directory_write_weight_t;



typedef union oha_l3_vdd_cache_access_weight {

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
    uint64_t value : 6;
    uint64_t _reserved0 : 58;
#else
    uint64_t _reserved0 : 58;
    uint64_t value : 6;
#endif // _BIG_ENDIAN
    } fields;
} oha_l3_vdd_cache_access_weight_t;



typedef union oha_chksw_hw132623dis {

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
    uint64_t value : 1;
    uint64_t _reserved0 : 63;
#else
    uint64_t _reserved0 : 63;
    uint64_t value : 1;
#endif // _BIG_ENDIAN
    } fields;
} oha_chksw_hw132623dis_t;



typedef union oha_activity_scale_factor_array {

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
    uint64_t value : 56;
    uint64_t _reserved0 : 8;
#else
    uint64_t _reserved0 : 8;
    uint64_t value : 56;
#endif // _BIG_ENDIAN
    } fields;
} oha_activity_scale_factor_array_t;



typedef union oha_activity_scale_shift_factor_array {

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
    uint64_t value : 42;
    uint64_t _reserved0 : 22;
#else
    uint64_t _reserved0 : 22;
    uint64_t value : 42;
#endif // _BIG_ENDIAN
    } fields;
} oha_activity_scale_shift_factor_array_t;


#endif // __ASSEMBLER__
#endif // __OHA_FIRMWARE_REGISTERS_H__

