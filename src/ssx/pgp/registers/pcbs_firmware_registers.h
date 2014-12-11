#ifndef __PCBS_FIRMWARE_REGISTERS_H__
#define __PCBS_FIRMWARE_REGISTERS_H__

// $Id: pcbs_firmware_registers.h,v 1.1.1.1 2013/12/11 21:03:25 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/registers/pcbs_firmware_registers.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pcbs_firmware_registers.h
/// \brief C register structs for the PCBS unit

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




typedef union pcbs_pmgp0_reg {

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
    uint64_t pm_disable : 1;
    uint64_t pmgp0_spare_bit1 : 1;
    uint64_t tp_tc_restart_core_domain : 1;
    uint64_t dpll_thold : 1;
    uint64_t perv_thold : 1;
    uint64_t cpm_cal_set_override_en : 1;
    uint64_t cpm_cal_set_val : 1;
    uint64_t pm_dpll_timer_ena : 1;
    uint64_t dpll_lock_sense : 1;
    uint64_t pmgp0_spare2 : 1;
    uint64_t dpll_reset : 1;
    uint64_t pmgp0_spare_bit11 : 1;
    uint64_t dpll_testout_ctl : 8;
    uint64_t tp_tc_cache2core_fence : 1;
    uint64_t tp_tc_core2cache_fence : 1;
    uint64_t tp_tc_pervasive_eco_fence : 1;
    uint64_t chksw_hw257424_disable : 1;
    uint64_t tp_clk_async_reset_dc : 3;
    uint64_t tp_clkglm_sel_dc : 3;
    uint64_t tp_clkglm_eco_sel_dc : 1;
    uint64_t special_wkup_done : 1;
    uint64_t tp_clkglm_core_sel_dc : 2;
    uint64_t tp_clkglm_const_dc : 1;
    uint64_t thold_timer_sel : 2;
    uint64_t block_all_wakeup_sources : 1;
    uint64_t tp_tc_dpll_testmode_dc : 1;
    uint64_t pm_slv_winkle_fence : 1;
    uint64_t l3_enable_switch : 1;
    uint64_t tp_cplt_ivrm_refbypass_dc : 1;
    uint64_t chksw_hw241939_disable : 1;
    uint64_t chksw_hw273115_disable : 1;
    uint64_t chksw_hw245103_disable : 1;
    uint64_t chksw_hw257534_disable : 1;
    uint64_t chksw_hw259509_enable : 1;
    uint64_t pmgp0_spare3 : 1;
    uint64_t wakeup_int_type : 2;
    uint64_t dpll_lock : 1;
    uint64_t special_wkup_all_sources_ored : 1;
    uint64_t regular_wkup_available : 1;
    uint64_t block_reg_wkup_sources : 1;
    uint64_t _reserved0 : 10;
#else
    uint64_t _reserved0 : 10;
    uint64_t block_reg_wkup_sources : 1;
    uint64_t regular_wkup_available : 1;
    uint64_t special_wkup_all_sources_ored : 1;
    uint64_t dpll_lock : 1;
    uint64_t wakeup_int_type : 2;
    uint64_t pmgp0_spare3 : 1;
    uint64_t chksw_hw259509_enable : 1;
    uint64_t chksw_hw257534_disable : 1;
    uint64_t chksw_hw245103_disable : 1;
    uint64_t chksw_hw273115_disable : 1;
    uint64_t chksw_hw241939_disable : 1;
    uint64_t tp_cplt_ivrm_refbypass_dc : 1;
    uint64_t l3_enable_switch : 1;
    uint64_t pm_slv_winkle_fence : 1;
    uint64_t tp_tc_dpll_testmode_dc : 1;
    uint64_t block_all_wakeup_sources : 1;
    uint64_t thold_timer_sel : 2;
    uint64_t tp_clkglm_const_dc : 1;
    uint64_t tp_clkglm_core_sel_dc : 2;
    uint64_t special_wkup_done : 1;
    uint64_t tp_clkglm_eco_sel_dc : 1;
    uint64_t tp_clkglm_sel_dc : 3;
    uint64_t tp_clk_async_reset_dc : 3;
    uint64_t chksw_hw257424_disable : 1;
    uint64_t tp_tc_pervasive_eco_fence : 1;
    uint64_t tp_tc_core2cache_fence : 1;
    uint64_t tp_tc_cache2core_fence : 1;
    uint64_t dpll_testout_ctl : 8;
    uint64_t pmgp0_spare_bit11 : 1;
    uint64_t dpll_reset : 1;
    uint64_t pmgp0_spare2 : 1;
    uint64_t dpll_lock_sense : 1;
    uint64_t pm_dpll_timer_ena : 1;
    uint64_t cpm_cal_set_val : 1;
    uint64_t cpm_cal_set_override_en : 1;
    uint64_t perv_thold : 1;
    uint64_t dpll_thold : 1;
    uint64_t tp_tc_restart_core_domain : 1;
    uint64_t pmgp0_spare_bit1 : 1;
    uint64_t pm_disable : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmgp0_reg_t;



typedef union pcbs_pmgp0_reg_and {

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
    uint64_t pm_disable : 1;
    uint64_t pmgp0_spare_bit1 : 1;
    uint64_t tp_tc_restart_core_domain : 1;
    uint64_t dpll_thold : 1;
    uint64_t perv_thold : 1;
    uint64_t cpm_cal_set_override_en : 1;
    uint64_t cpm_cal_set_val : 1;
    uint64_t pm_dpll_timer_ena : 1;
    uint64_t dpll_lock_sense : 1;
    uint64_t pmgp0_spare2 : 1;
    uint64_t dpll_reset : 1;
    uint64_t pmgp0_spare_bit11 : 1;
    uint64_t dpll_testout_ctl : 8;
    uint64_t tp_tc_cache2core_fence : 1;
    uint64_t tp_tc_core2cache_fence : 1;
    uint64_t tp_tc_pervasive_eco_fence : 1;
    uint64_t chksw_hw257424_disable : 1;
    uint64_t tp_clk_async_reset_dc : 3;
    uint64_t tp_clkglm_sel_dc : 3;
    uint64_t tp_clkglm_eco_sel_dc : 1;
    uint64_t special_wkup_done : 1;
    uint64_t tp_clkglm_core_sel_dc : 2;
    uint64_t tp_clkglm_const_dc : 1;
    uint64_t thold_timer_sel : 2;
    uint64_t block_all_wakeup_sources : 1;
    uint64_t tp_tc_dpll_testmode_dc : 1;
    uint64_t pm_slv_winkle_fence : 1;
    uint64_t l3_enable_switch : 1;
    uint64_t tp_cplt_ivrm_refbypass_dc : 1;
    uint64_t chksw_hw241939_disable : 1;
    uint64_t chksw_hw273115_disable : 1;
    uint64_t chksw_hw245103_disable : 1;
    uint64_t chksw_hw257534_disable : 1;
    uint64_t chksw_hw259509_enable : 1;
    uint64_t pmgp0_spare3 : 1;
    uint64_t wakeup_int_type : 2;
    uint64_t dpll_lock : 1;
    uint64_t special_wkup_all_sources_ored : 1;
    uint64_t regular_wkup_available : 1;
    uint64_t block_reg_wkup_sources : 1;
    uint64_t _reserved0 : 10;
#else
    uint64_t _reserved0 : 10;
    uint64_t block_reg_wkup_sources : 1;
    uint64_t regular_wkup_available : 1;
    uint64_t special_wkup_all_sources_ored : 1;
    uint64_t dpll_lock : 1;
    uint64_t wakeup_int_type : 2;
    uint64_t pmgp0_spare3 : 1;
    uint64_t chksw_hw259509_enable : 1;
    uint64_t chksw_hw257534_disable : 1;
    uint64_t chksw_hw245103_disable : 1;
    uint64_t chksw_hw273115_disable : 1;
    uint64_t chksw_hw241939_disable : 1;
    uint64_t tp_cplt_ivrm_refbypass_dc : 1;
    uint64_t l3_enable_switch : 1;
    uint64_t pm_slv_winkle_fence : 1;
    uint64_t tp_tc_dpll_testmode_dc : 1;
    uint64_t block_all_wakeup_sources : 1;
    uint64_t thold_timer_sel : 2;
    uint64_t tp_clkglm_const_dc : 1;
    uint64_t tp_clkglm_core_sel_dc : 2;
    uint64_t special_wkup_done : 1;
    uint64_t tp_clkglm_eco_sel_dc : 1;
    uint64_t tp_clkglm_sel_dc : 3;
    uint64_t tp_clk_async_reset_dc : 3;
    uint64_t chksw_hw257424_disable : 1;
    uint64_t tp_tc_pervasive_eco_fence : 1;
    uint64_t tp_tc_core2cache_fence : 1;
    uint64_t tp_tc_cache2core_fence : 1;
    uint64_t dpll_testout_ctl : 8;
    uint64_t pmgp0_spare_bit11 : 1;
    uint64_t dpll_reset : 1;
    uint64_t pmgp0_spare2 : 1;
    uint64_t dpll_lock_sense : 1;
    uint64_t pm_dpll_timer_ena : 1;
    uint64_t cpm_cal_set_val : 1;
    uint64_t cpm_cal_set_override_en : 1;
    uint64_t perv_thold : 1;
    uint64_t dpll_thold : 1;
    uint64_t tp_tc_restart_core_domain : 1;
    uint64_t pmgp0_spare_bit1 : 1;
    uint64_t pm_disable : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmgp0_reg_and_t;



typedef union pcbs_pmgp0_reg_or {

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
    uint64_t pm_disable : 1;
    uint64_t pmgp0_spare_bit1 : 1;
    uint64_t tp_tc_restart_core_domain : 1;
    uint64_t dpll_thold : 1;
    uint64_t perv_thold : 1;
    uint64_t cpm_cal_set_override_en : 1;
    uint64_t cpm_cal_set_val : 1;
    uint64_t pm_dpll_timer_ena : 1;
    uint64_t dpll_lock_sense : 1;
    uint64_t pmgp0_spare2 : 1;
    uint64_t dpll_reset : 1;
    uint64_t pmgp0_spare_bit11 : 1;
    uint64_t dpll_testout_ctl : 8;
    uint64_t tp_tc_cache2core_fence : 1;
    uint64_t tp_tc_core2cache_fence : 1;
    uint64_t tp_tc_pervasive_eco_fence : 1;
    uint64_t chksw_hw257424_disable : 1;
    uint64_t tp_clk_async_reset_dc : 3;
    uint64_t tp_clkglm_sel_dc : 3;
    uint64_t tp_clkglm_eco_sel_dc : 1;
    uint64_t special_wkup_done : 1;
    uint64_t tp_clkglm_core_sel_dc : 2;
    uint64_t tp_clkglm_const_dc : 1;
    uint64_t thold_timer_sel : 2;
    uint64_t block_all_wakeup_sources : 1;
    uint64_t tp_tc_dpll_testmode_dc : 1;
    uint64_t pm_slv_winkle_fence : 1;
    uint64_t l3_enable_switch : 1;
    uint64_t tp_cplt_ivrm_refbypass_dc : 1;
    uint64_t chksw_hw241939_disable : 1;
    uint64_t chksw_hw273115_disable : 1;
    uint64_t chksw_hw245103_disable : 1;
    uint64_t chksw_hw257534_disable : 1;
    uint64_t chksw_hw259509_enable : 1;
    uint64_t pmgp0_spare3 : 1;
    uint64_t wakeup_int_type : 2;
    uint64_t dpll_lock : 1;
    uint64_t special_wkup_all_sources_ored : 1;
    uint64_t regular_wkup_available : 1;
    uint64_t block_reg_wkup_sources : 1;
    uint64_t _reserved0 : 10;
#else
    uint64_t _reserved0 : 10;
    uint64_t block_reg_wkup_sources : 1;
    uint64_t regular_wkup_available : 1;
    uint64_t special_wkup_all_sources_ored : 1;
    uint64_t dpll_lock : 1;
    uint64_t wakeup_int_type : 2;
    uint64_t pmgp0_spare3 : 1;
    uint64_t chksw_hw259509_enable : 1;
    uint64_t chksw_hw257534_disable : 1;
    uint64_t chksw_hw245103_disable : 1;
    uint64_t chksw_hw273115_disable : 1;
    uint64_t chksw_hw241939_disable : 1;
    uint64_t tp_cplt_ivrm_refbypass_dc : 1;
    uint64_t l3_enable_switch : 1;
    uint64_t pm_slv_winkle_fence : 1;
    uint64_t tp_tc_dpll_testmode_dc : 1;
    uint64_t block_all_wakeup_sources : 1;
    uint64_t thold_timer_sel : 2;
    uint64_t tp_clkglm_const_dc : 1;
    uint64_t tp_clkglm_core_sel_dc : 2;
    uint64_t special_wkup_done : 1;
    uint64_t tp_clkglm_eco_sel_dc : 1;
    uint64_t tp_clkglm_sel_dc : 3;
    uint64_t tp_clk_async_reset_dc : 3;
    uint64_t chksw_hw257424_disable : 1;
    uint64_t tp_tc_pervasive_eco_fence : 1;
    uint64_t tp_tc_core2cache_fence : 1;
    uint64_t tp_tc_cache2core_fence : 1;
    uint64_t dpll_testout_ctl : 8;
    uint64_t pmgp0_spare_bit11 : 1;
    uint64_t dpll_reset : 1;
    uint64_t pmgp0_spare2 : 1;
    uint64_t dpll_lock_sense : 1;
    uint64_t pm_dpll_timer_ena : 1;
    uint64_t cpm_cal_set_val : 1;
    uint64_t cpm_cal_set_override_en : 1;
    uint64_t perv_thold : 1;
    uint64_t dpll_thold : 1;
    uint64_t tp_tc_restart_core_domain : 1;
    uint64_t pmgp0_spare_bit1 : 1;
    uint64_t pm_disable : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmgp0_reg_or_t;



typedef union pcbs_pmgp1_reg {

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
    uint64_t sleep_power_down_en : 1;
    uint64_t sleep_power_up_en : 1;
    uint64_t sleep_power_off_sel : 1;
    uint64_t winkle_power_down_en : 1;
    uint64_t winkle_power_up_en : 1;
    uint64_t winkle_power_off_sel : 1;
    uint64_t oha_wkup_override_en : 1;
    uint64_t oha_pm_wkup_override : 1;
    uint64_t oha_spc_wkup_override : 1;
    uint64_t endp_reset_pm_only : 1;
    uint64_t dpll_freq_override_enable : 1;
    uint64_t pm_spr_override_en : 1;
    uint64_t force_safe_mode : 1;
    uint64_t ivrm_safe_mode_en : 1;
    uint64_t ivrm_safe_mode_force_active : 1;
    uint64_t pmicr_latency_en : 1;
    uint64_t enable_occ_ctrl_for_local_pstate_eff_req : 1;
    uint64_t serialize_pfet_powerdown : 1;
    uint64_t serialize_pfet_powerup : 1;
    uint64_t disable_force_deep_to_fast_sleep : 1;
    uint64_t disable_force_deep_to_fast_winkle : 1;
    uint64_t _reserved0 : 43;
#else
    uint64_t _reserved0 : 43;
    uint64_t disable_force_deep_to_fast_winkle : 1;
    uint64_t disable_force_deep_to_fast_sleep : 1;
    uint64_t serialize_pfet_powerup : 1;
    uint64_t serialize_pfet_powerdown : 1;
    uint64_t enable_occ_ctrl_for_local_pstate_eff_req : 1;
    uint64_t pmicr_latency_en : 1;
    uint64_t ivrm_safe_mode_force_active : 1;
    uint64_t ivrm_safe_mode_en : 1;
    uint64_t force_safe_mode : 1;
    uint64_t pm_spr_override_en : 1;
    uint64_t dpll_freq_override_enable : 1;
    uint64_t endp_reset_pm_only : 1;
    uint64_t oha_spc_wkup_override : 1;
    uint64_t oha_pm_wkup_override : 1;
    uint64_t oha_wkup_override_en : 1;
    uint64_t winkle_power_off_sel : 1;
    uint64_t winkle_power_up_en : 1;
    uint64_t winkle_power_down_en : 1;
    uint64_t sleep_power_off_sel : 1;
    uint64_t sleep_power_up_en : 1;
    uint64_t sleep_power_down_en : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmgp1_reg_t;



typedef union pcbs_pmgp1_reg_and {

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
    uint64_t sleep_power_down_en : 1;
    uint64_t sleep_power_up_en : 1;
    uint64_t sleep_power_off_sel : 1;
    uint64_t winkle_power_down_en : 1;
    uint64_t winkle_power_up_en : 1;
    uint64_t winkle_power_off_sel : 1;
    uint64_t oha_wkup_override_en : 1;
    uint64_t oha_pm_wkup_override : 1;
    uint64_t oha_spc_wkup_override : 1;
    uint64_t endp_reset_pm_only : 1;
    uint64_t dpll_freq_override_enable : 1;
    uint64_t pm_spr_override_en : 1;
    uint64_t force_safe_mode : 1;
    uint64_t ivrm_safe_mode_en : 1;
    uint64_t ivrm_safe_mode_force_active : 1;
    uint64_t pmicr_latency_en : 1;
    uint64_t enable_occ_ctrl_for_local_pstate_eff_req : 1;
    uint64_t serialize_pfet_powerdown : 1;
    uint64_t serialize_pfet_powerup : 1;
    uint64_t disable_force_deep_to_fast_sleep : 1;
    uint64_t disable_force_deep_to_fast_winkle : 1;
    uint64_t _reserved0 : 43;
#else
    uint64_t _reserved0 : 43;
    uint64_t disable_force_deep_to_fast_winkle : 1;
    uint64_t disable_force_deep_to_fast_sleep : 1;
    uint64_t serialize_pfet_powerup : 1;
    uint64_t serialize_pfet_powerdown : 1;
    uint64_t enable_occ_ctrl_for_local_pstate_eff_req : 1;
    uint64_t pmicr_latency_en : 1;
    uint64_t ivrm_safe_mode_force_active : 1;
    uint64_t ivrm_safe_mode_en : 1;
    uint64_t force_safe_mode : 1;
    uint64_t pm_spr_override_en : 1;
    uint64_t dpll_freq_override_enable : 1;
    uint64_t endp_reset_pm_only : 1;
    uint64_t oha_spc_wkup_override : 1;
    uint64_t oha_pm_wkup_override : 1;
    uint64_t oha_wkup_override_en : 1;
    uint64_t winkle_power_off_sel : 1;
    uint64_t winkle_power_up_en : 1;
    uint64_t winkle_power_down_en : 1;
    uint64_t sleep_power_off_sel : 1;
    uint64_t sleep_power_up_en : 1;
    uint64_t sleep_power_down_en : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmgp1_reg_and_t;



typedef union pcbs_pmgp1_reg_or {

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
    uint64_t sleep_power_down_en : 1;
    uint64_t sleep_power_up_en : 1;
    uint64_t sleep_power_off_sel : 1;
    uint64_t winkle_power_down_en : 1;
    uint64_t winkle_power_up_en : 1;
    uint64_t winkle_power_off_sel : 1;
    uint64_t oha_wkup_override_en : 1;
    uint64_t oha_pm_wkup_override : 1;
    uint64_t oha_spc_wkup_override : 1;
    uint64_t endp_reset_pm_only : 1;
    uint64_t dpll_freq_override_enable : 1;
    uint64_t pm_spr_override_en : 1;
    uint64_t force_safe_mode : 1;
    uint64_t ivrm_safe_mode_en : 1;
    uint64_t ivrm_safe_mode_force_active : 1;
    uint64_t pmicr_latency_en : 1;
    uint64_t enable_occ_ctrl_for_local_pstate_eff_req : 1;
    uint64_t serialize_pfet_powerdown : 1;
    uint64_t serialize_pfet_powerup : 1;
    uint64_t disable_force_deep_to_fast_sleep : 1;
    uint64_t disable_force_deep_to_fast_winkle : 1;
    uint64_t _reserved0 : 43;
#else
    uint64_t _reserved0 : 43;
    uint64_t disable_force_deep_to_fast_winkle : 1;
    uint64_t disable_force_deep_to_fast_sleep : 1;
    uint64_t serialize_pfet_powerup : 1;
    uint64_t serialize_pfet_powerdown : 1;
    uint64_t enable_occ_ctrl_for_local_pstate_eff_req : 1;
    uint64_t pmicr_latency_en : 1;
    uint64_t ivrm_safe_mode_force_active : 1;
    uint64_t ivrm_safe_mode_en : 1;
    uint64_t force_safe_mode : 1;
    uint64_t pm_spr_override_en : 1;
    uint64_t dpll_freq_override_enable : 1;
    uint64_t endp_reset_pm_only : 1;
    uint64_t oha_spc_wkup_override : 1;
    uint64_t oha_pm_wkup_override : 1;
    uint64_t oha_wkup_override_en : 1;
    uint64_t winkle_power_off_sel : 1;
    uint64_t winkle_power_up_en : 1;
    uint64_t winkle_power_down_en : 1;
    uint64_t sleep_power_off_sel : 1;
    uint64_t sleep_power_up_en : 1;
    uint64_t sleep_power_down_en : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmgp1_reg_or_t;



typedef union pcbs_pfvddcntlstat_reg {

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
    uint64_t core_vdd_pfet_force_state : 2;
    uint64_t eco_vdd_pfet_force_state : 2;
    uint64_t core_vdd_pfet_val_override : 1;
    uint64_t core_vdd_pfet_sel_override : 1;
    uint64_t eco_vdd_pfet_val_override : 1;
    uint64_t eco_vdd_pfet_sel_override : 1;
    uint64_t core_vdd_pfet_enable_regulation_finger : 1;
    uint64_t eco_vdd_pfet_enable_regulation_finger : 1;
    uint64_t core_vdd_pfet_enable_value : 12;
    uint64_t core_vdd_pfet_sel_value : 4;
    uint64_t eco_vdd_pfet_enable_value : 12;
    uint64_t eco_vdd_pfet_sel_value : 4;
    uint64_t core_vdd_pg_state : 4;
    uint64_t core_vdd_pg_sel : 4;
    uint64_t eco_vdd_pg_state : 4;
    uint64_t eco_vdd_pg_sel : 4;
    uint64_t _reserved0 : 6;
#else
    uint64_t _reserved0 : 6;
    uint64_t eco_vdd_pg_sel : 4;
    uint64_t eco_vdd_pg_state : 4;
    uint64_t core_vdd_pg_sel : 4;
    uint64_t core_vdd_pg_state : 4;
    uint64_t eco_vdd_pfet_sel_value : 4;
    uint64_t eco_vdd_pfet_enable_value : 12;
    uint64_t core_vdd_pfet_sel_value : 4;
    uint64_t core_vdd_pfet_enable_value : 12;
    uint64_t eco_vdd_pfet_enable_regulation_finger : 1;
    uint64_t core_vdd_pfet_enable_regulation_finger : 1;
    uint64_t eco_vdd_pfet_sel_override : 1;
    uint64_t eco_vdd_pfet_val_override : 1;
    uint64_t core_vdd_pfet_sel_override : 1;
    uint64_t core_vdd_pfet_val_override : 1;
    uint64_t eco_vdd_pfet_force_state : 2;
    uint64_t core_vdd_pfet_force_state : 2;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pfvddcntlstat_reg_t;



typedef union pcbs_pfvcscntlstat_reg {

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
    uint64_t core_vcs_pfet_force_state : 2;
    uint64_t eco_vcs_pfet_force_state : 2;
    uint64_t core_vcs_pfet_val_override : 1;
    uint64_t core_vcs_pfet_sel_override : 1;
    uint64_t eco_vcs_pfet_val_override : 1;
    uint64_t eco_vcs_pfet_sel_override : 1;
    uint64_t core_vcs_pfet_enable_regulation_finger : 1;
    uint64_t eco_vcs_pfet_enable_regulation_finger : 1;
    uint64_t core_vcs_pfet_enable_value : 12;
    uint64_t core_vcs_pfet_sel_value : 4;
    uint64_t eco_vcs_pfet_enable_value : 12;
    uint64_t eco_vcs_pfet_sel_value : 4;
    uint64_t core_vcs_pg_state : 4;
    uint64_t core_vcs_pg_sel : 4;
    uint64_t eco_vcs_pg_state : 4;
    uint64_t eco_vcs_pg_sel : 4;
    uint64_t _reserved0 : 6;
#else
    uint64_t _reserved0 : 6;
    uint64_t eco_vcs_pg_sel : 4;
    uint64_t eco_vcs_pg_state : 4;
    uint64_t core_vcs_pg_sel : 4;
    uint64_t core_vcs_pg_state : 4;
    uint64_t eco_vcs_pfet_sel_value : 4;
    uint64_t eco_vcs_pfet_enable_value : 12;
    uint64_t core_vcs_pfet_sel_value : 4;
    uint64_t core_vcs_pfet_enable_value : 12;
    uint64_t eco_vcs_pfet_enable_regulation_finger : 1;
    uint64_t core_vcs_pfet_enable_regulation_finger : 1;
    uint64_t eco_vcs_pfet_sel_override : 1;
    uint64_t eco_vcs_pfet_val_override : 1;
    uint64_t core_vcs_pfet_sel_override : 1;
    uint64_t core_vcs_pfet_val_override : 1;
    uint64_t eco_vcs_pfet_force_state : 2;
    uint64_t core_vcs_pfet_force_state : 2;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pfvcscntlstat_reg_t;



typedef union pcbs_pfsense_reg {

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
    uint64_t tp_core_vdd_pfet_enable_sense : 12;
    uint64_t tp_eco_vdd_pfet_enable_sense : 12;
    uint64_t tp_core_vcs_pfet_enable_sense : 12;
    uint64_t tp_eco_vcs_pfet_enable_sense : 12;
    uint64_t _reserved0 : 16;
#else
    uint64_t _reserved0 : 16;
    uint64_t tp_eco_vcs_pfet_enable_sense : 12;
    uint64_t tp_core_vcs_pfet_enable_sense : 12;
    uint64_t tp_eco_vdd_pfet_enable_sense : 12;
    uint64_t tp_core_vdd_pfet_enable_sense : 12;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pfsense_reg_t;



typedef union pcbs_pmerrsum_reg {

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
    uint64_t _reserved0 : 63;
#else
    uint64_t _reserved0 : 63;
    uint64_t pm_error : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmerrsum_reg_t;



typedef union pcbs_pmerr_reg {

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
    uint64_t pcbs_sleep_entry_notify_pmc_hang_err : 1;
    uint64_t pcbs_sleep_entry_notify_pmc_assist_hang_err : 1;
    uint64_t pcbs_sleep_entry_notify_pmc_err : 1;
    uint64_t pcbs_sleep_exit_invoke_pore_err : 1;
    uint64_t pcbs_winkle_entry_notify_pmc_err : 1;
    uint64_t pcbs_winkle_entry_send_int_assist_err : 1;
    uint64_t pcbs_winkle_exit_notify_pmc_err : 1;
    uint64_t pcbs_wait_dpll_lock_err : 1;
    uint64_t pcbs_spare8_err : 1;
    uint64_t pcbs_winkle_exit_send_int_assist_err : 1;
    uint64_t pcbs_winkle_exit_send_int_powup_assist_err : 1;
    uint64_t pcbs_write_fsm_goto_reg_in_invalid_state_err : 1;
    uint64_t pcbs_write_pmgp0_in_invalid_state_err : 1;
    uint64_t pcbs_freq_overflow_in_pstate_mode_err : 1;
    uint64_t pcbs_eco_rs_bypass_confusion_err : 1;
    uint64_t pcbs_core_rs_bypass_confusion_err : 1;
    uint64_t pcbs_read_lpst_in_pstate_mode_err : 1;
    uint64_t pcbs_lpst_read_corr_err : 1;
    uint64_t pcbs_lpst_read_uncorr_err : 1;
    uint64_t pcbs_pfet_strength_overflow_err : 1;
    uint64_t pcbs_vds_lookup_err : 1;
    uint64_t pcbs_idle_interrupt_timeout_err : 1;
    uint64_t pcbs_pstate_interrupt_timeout_err : 1;
    uint64_t pcbs_global_actual_sync_interrupt_timeout_err : 1;
    uint64_t pcbs_pmax_sync_interrupt_timeout_err : 1;
    uint64_t pcbs_global_actual_pstate_protocol_err : 1;
    uint64_t pcbs_pmax_protocol_err : 1;
    uint64_t pcbs_ivrm_gross_or_fine_err : 1;
    uint64_t pcbs_ivrm_range_err : 1;
    uint64_t pcbs_dpll_cpm_fmin_err : 1;
    uint64_t pcbs_dpll_dco_full_err : 1;
    uint64_t pcbs_dpll_dco_empty_err : 1;
    uint64_t pcbs_dpll_int_err : 1;
    uint64_t pcbs_fmin_and_not_cpmbit_err : 1;
    uint64_t pcbs_dpll_faster_than_fmax_plus_delta1_err : 1;
    uint64_t pcbs_dpll_slower_than_fmin_minus_delta2_err : 1;
    uint64_t pcbs_resclk_csb_instr_vector_chg_in_invalid_state_err : 1;
    uint64_t pcbs_reslkc_band_boundary_chg_in_invalid_state_err : 1;
    uint64_t pcbs_occ_heartbeat_loss_err : 1;
    uint64_t pcbs_spare39_err : 1;
    uint64_t pcbs_spare40_err : 1;
    uint64_t pcbs_spare41_err : 1;
    uint64_t pcbs_spare42_err : 1;
    uint64_t _reserved0 : 21;
#else
    uint64_t _reserved0 : 21;
    uint64_t pcbs_spare42_err : 1;
    uint64_t pcbs_spare41_err : 1;
    uint64_t pcbs_spare40_err : 1;
    uint64_t pcbs_spare39_err : 1;
    uint64_t pcbs_occ_heartbeat_loss_err : 1;
    uint64_t pcbs_reslkc_band_boundary_chg_in_invalid_state_err : 1;
    uint64_t pcbs_resclk_csb_instr_vector_chg_in_invalid_state_err : 1;
    uint64_t pcbs_dpll_slower_than_fmin_minus_delta2_err : 1;
    uint64_t pcbs_dpll_faster_than_fmax_plus_delta1_err : 1;
    uint64_t pcbs_fmin_and_not_cpmbit_err : 1;
    uint64_t pcbs_dpll_int_err : 1;
    uint64_t pcbs_dpll_dco_empty_err : 1;
    uint64_t pcbs_dpll_dco_full_err : 1;
    uint64_t pcbs_dpll_cpm_fmin_err : 1;
    uint64_t pcbs_ivrm_range_err : 1;
    uint64_t pcbs_ivrm_gross_or_fine_err : 1;
    uint64_t pcbs_pmax_protocol_err : 1;
    uint64_t pcbs_global_actual_pstate_protocol_err : 1;
    uint64_t pcbs_pmax_sync_interrupt_timeout_err : 1;
    uint64_t pcbs_global_actual_sync_interrupt_timeout_err : 1;
    uint64_t pcbs_pstate_interrupt_timeout_err : 1;
    uint64_t pcbs_idle_interrupt_timeout_err : 1;
    uint64_t pcbs_vds_lookup_err : 1;
    uint64_t pcbs_pfet_strength_overflow_err : 1;
    uint64_t pcbs_lpst_read_uncorr_err : 1;
    uint64_t pcbs_lpst_read_corr_err : 1;
    uint64_t pcbs_read_lpst_in_pstate_mode_err : 1;
    uint64_t pcbs_core_rs_bypass_confusion_err : 1;
    uint64_t pcbs_eco_rs_bypass_confusion_err : 1;
    uint64_t pcbs_freq_overflow_in_pstate_mode_err : 1;
    uint64_t pcbs_write_pmgp0_in_invalid_state_err : 1;
    uint64_t pcbs_write_fsm_goto_reg_in_invalid_state_err : 1;
    uint64_t pcbs_winkle_exit_send_int_powup_assist_err : 1;
    uint64_t pcbs_winkle_exit_send_int_assist_err : 1;
    uint64_t pcbs_spare8_err : 1;
    uint64_t pcbs_wait_dpll_lock_err : 1;
    uint64_t pcbs_winkle_exit_notify_pmc_err : 1;
    uint64_t pcbs_winkle_entry_send_int_assist_err : 1;
    uint64_t pcbs_winkle_entry_notify_pmc_err : 1;
    uint64_t pcbs_sleep_exit_invoke_pore_err : 1;
    uint64_t pcbs_sleep_entry_notify_pmc_err : 1;
    uint64_t pcbs_sleep_entry_notify_pmc_assist_hang_err : 1;
    uint64_t pcbs_sleep_entry_notify_pmc_hang_err : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmerr_reg_t;



typedef union pcbs_pmerrmask_reg {

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
    uint64_t pcbs_sleep_entry_notify_pmc_hang_err_mask : 1;
    uint64_t pcbs_sleep_entry_notify_pmc_assist_hang_err_mask : 1;
    uint64_t pcbs_sleep_entry_notify_pmc_err_mask : 1;
    uint64_t pcbs_sleep_exit_invoke_pore_err_mask : 1;
    uint64_t pcbs_winkle_entry_notify_pmc_err_mask : 1;
    uint64_t pcbs_winkle_entry_send_int_assist_err_mask : 1;
    uint64_t pcbs_winkle_exit_notify_pmc_err_mask : 1;
    uint64_t pcbs_wait_dpll_lock_err_mask : 1;
    uint64_t pcbs_spare8_err_mask : 1;
    uint64_t pcbs_winkle_exit_send_int_assist_err_mask : 1;
    uint64_t pcbs_winkle_exit_send_int_powup_assist_err_mask : 1;
    uint64_t pcbs_write_fsm_goto_reg_in_invalid_state_err_mask : 1;
    uint64_t pcbs_write_pmgp0_in_invalid_state_err_mask : 1;
    uint64_t pcbs_freq_overflow_in_pstate_mode_err_mask : 1;
    uint64_t pcbs_eco_rs_bypass_confusion_err_mask : 1;
    uint64_t pcbs_core_rs_bypass_confusion_err_mask : 1;
    uint64_t pcbs_read_lpst_in_pstate_mode_err_mask : 1;
    uint64_t pcbs_lpst_read_corr_err_mask : 1;
    uint64_t pcbs_lpst_read_uncorr_err_mask : 1;
    uint64_t pcbs_pfet_strength_overflow_err_mask : 1;
    uint64_t pcbs_vds_lookup_err_mask : 1;
    uint64_t pcbs_idle_interrupt_timeout_err_mask : 1;
    uint64_t pcbs_pstate_interrupt_timeout_err_mask : 1;
    uint64_t pcbs_global_actual_sync_interrupt_timeout_err_mask : 1;
    uint64_t pcbs_pmax_sync_interrupt_timeout_err_mask : 1;
    uint64_t pcbs_global_actual_pstate_protocol_err_mask : 1;
    uint64_t pcbs_pmax_protocol_err_mask : 1;
    uint64_t pcbs_ivrm_gross_or_fine_err_mask : 1;
    uint64_t pcbs_ivrm_range_err_mask : 1;
    uint64_t pcbs_dpll_cpm_fmin_err_mask : 1;
    uint64_t pcbs_dpll_dco_full_err_mask : 1;
    uint64_t pcbs_dpll_dco_empty_err_mask : 1;
    uint64_t pcbs_dpll_int_err_mask : 1;
    uint64_t pcbs_fmin_and_not_cpmbit_err_mask : 1;
    uint64_t pcbs_dpll_faster_than_fmax_plus_delta1_err_mask : 1;
    uint64_t pcbs_dpll_slower_than_fmin_minus_delta2_err_mask : 1;
    uint64_t pcbs_resclk_csb_instr_vector_chg_in_invalid_state_err_mask : 1;
    uint64_t pcbs_reslkc_band_boundary_chg_in_invalid_state_err_mask : 1;
    uint64_t pcbs_occ_heartbeat_loss_err_mask : 1;
    uint64_t pcbs_spare39_err_mask : 1;
    uint64_t pcbs_spare40_err_mask : 1;
    uint64_t pcbs_spare41_err_mask : 1;
    uint64_t pcbs_spare42_err_mask : 1;
    uint64_t _reserved0 : 21;
#else
    uint64_t _reserved0 : 21;
    uint64_t pcbs_spare42_err_mask : 1;
    uint64_t pcbs_spare41_err_mask : 1;
    uint64_t pcbs_spare40_err_mask : 1;
    uint64_t pcbs_spare39_err_mask : 1;
    uint64_t pcbs_occ_heartbeat_loss_err_mask : 1;
    uint64_t pcbs_reslkc_band_boundary_chg_in_invalid_state_err_mask : 1;
    uint64_t pcbs_resclk_csb_instr_vector_chg_in_invalid_state_err_mask : 1;
    uint64_t pcbs_dpll_slower_than_fmin_minus_delta2_err_mask : 1;
    uint64_t pcbs_dpll_faster_than_fmax_plus_delta1_err_mask : 1;
    uint64_t pcbs_fmin_and_not_cpmbit_err_mask : 1;
    uint64_t pcbs_dpll_int_err_mask : 1;
    uint64_t pcbs_dpll_dco_empty_err_mask : 1;
    uint64_t pcbs_dpll_dco_full_err_mask : 1;
    uint64_t pcbs_dpll_cpm_fmin_err_mask : 1;
    uint64_t pcbs_ivrm_range_err_mask : 1;
    uint64_t pcbs_ivrm_gross_or_fine_err_mask : 1;
    uint64_t pcbs_pmax_protocol_err_mask : 1;
    uint64_t pcbs_global_actual_pstate_protocol_err_mask : 1;
    uint64_t pcbs_pmax_sync_interrupt_timeout_err_mask : 1;
    uint64_t pcbs_global_actual_sync_interrupt_timeout_err_mask : 1;
    uint64_t pcbs_pstate_interrupt_timeout_err_mask : 1;
    uint64_t pcbs_idle_interrupt_timeout_err_mask : 1;
    uint64_t pcbs_vds_lookup_err_mask : 1;
    uint64_t pcbs_pfet_strength_overflow_err_mask : 1;
    uint64_t pcbs_lpst_read_uncorr_err_mask : 1;
    uint64_t pcbs_lpst_read_corr_err_mask : 1;
    uint64_t pcbs_read_lpst_in_pstate_mode_err_mask : 1;
    uint64_t pcbs_core_rs_bypass_confusion_err_mask : 1;
    uint64_t pcbs_eco_rs_bypass_confusion_err_mask : 1;
    uint64_t pcbs_freq_overflow_in_pstate_mode_err_mask : 1;
    uint64_t pcbs_write_pmgp0_in_invalid_state_err_mask : 1;
    uint64_t pcbs_write_fsm_goto_reg_in_invalid_state_err_mask : 1;
    uint64_t pcbs_winkle_exit_send_int_powup_assist_err_mask : 1;
    uint64_t pcbs_winkle_exit_send_int_assist_err_mask : 1;
    uint64_t pcbs_spare8_err_mask : 1;
    uint64_t pcbs_wait_dpll_lock_err_mask : 1;
    uint64_t pcbs_winkle_exit_notify_pmc_err_mask : 1;
    uint64_t pcbs_winkle_entry_send_int_assist_err_mask : 1;
    uint64_t pcbs_winkle_entry_notify_pmc_err_mask : 1;
    uint64_t pcbs_sleep_exit_invoke_pore_err_mask : 1;
    uint64_t pcbs_sleep_entry_notify_pmc_err_mask : 1;
    uint64_t pcbs_sleep_entry_notify_pmc_assist_hang_err_mask : 1;
    uint64_t pcbs_sleep_entry_notify_pmc_hang_err_mask : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmerrmask_reg_t;



typedef union pcbs_pmspcwkupfsp_reg {

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
    uint64_t fsp_special_wakeup : 1;
    uint64_t _reserved0 : 63;
#else
    uint64_t _reserved0 : 63;
    uint64_t fsp_special_wakeup : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmspcwkupfsp_reg_t;



typedef union pcbs_pmspcwkupocc_reg {

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
    uint64_t occ_special_wakeup : 1;
    uint64_t _reserved0 : 63;
#else
    uint64_t _reserved0 : 63;
    uint64_t occ_special_wakeup : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmspcwkupocc_reg_t;



typedef union pcbs_pmspcwkupphyp_reg {

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
    uint64_t phyp_special_wakeup : 1;
    uint64_t _reserved0 : 63;
#else
    uint64_t _reserved0 : 63;
    uint64_t phyp_special_wakeup : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmspcwkupphyp_reg_t;



typedef union pcbs_pmstatehistphyp_reg {

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
    uint64_t phyp_pm_state : 3;
    uint64_t phyp_past_core_instruct_stop : 1;
    uint64_t phyp_past_core_clk_stop : 1;
    uint64_t phyp_past_core_pwr_off : 1;
    uint64_t phyp_past_eco_clk_stop : 1;
    uint64_t phyp_past_eco_pwr_off : 1;
    uint64_t _reserved0 : 56;
#else
    uint64_t _reserved0 : 56;
    uint64_t phyp_past_eco_pwr_off : 1;
    uint64_t phyp_past_eco_clk_stop : 1;
    uint64_t phyp_past_core_pwr_off : 1;
    uint64_t phyp_past_core_clk_stop : 1;
    uint64_t phyp_past_core_instruct_stop : 1;
    uint64_t phyp_pm_state : 3;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmstatehistphyp_reg_t;



typedef union pcbs_pmstatehistfsp_reg {

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
    uint64_t fsp_pm_state : 3;
    uint64_t fsp_past_core_instruct_stop : 1;
    uint64_t fsp_past_core_clk_stop : 1;
    uint64_t fsp_past_core_pwr_off : 1;
    uint64_t fsp_past_eco_clk_stop : 1;
    uint64_t fsp_past_eco_pwr_off : 1;
    uint64_t _reserved0 : 56;
#else
    uint64_t _reserved0 : 56;
    uint64_t fsp_past_eco_pwr_off : 1;
    uint64_t fsp_past_eco_clk_stop : 1;
    uint64_t fsp_past_core_pwr_off : 1;
    uint64_t fsp_past_core_clk_stop : 1;
    uint64_t fsp_past_core_instruct_stop : 1;
    uint64_t fsp_pm_state : 3;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmstatehistfsp_reg_t;



typedef union pcbs_pmstatehistocc_reg {

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
    uint64_t occ_pm_state : 3;
    uint64_t occ_past_core_instruct_stop : 1;
    uint64_t occ_past_core_clk_stop : 1;
    uint64_t occ_past_core_pwr_off : 1;
    uint64_t occ_past_eco_clk_stop : 1;
    uint64_t occ_past_eco_pwr_off : 1;
    uint64_t _reserved0 : 56;
#else
    uint64_t _reserved0 : 56;
    uint64_t occ_past_eco_pwr_off : 1;
    uint64_t occ_past_eco_clk_stop : 1;
    uint64_t occ_past_core_pwr_off : 1;
    uint64_t occ_past_core_clk_stop : 1;
    uint64_t occ_past_core_instruct_stop : 1;
    uint64_t occ_pm_state : 3;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmstatehistocc_reg_t;



typedef union pcbs_pmstatehistperf_reg {

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
    uint64_t perf_pm_state : 3;
    uint64_t perf_past_core_instruct_stop : 1;
    uint64_t perf_past_core_clk_stop : 1;
    uint64_t perf_past_core_pwr_off : 1;
    uint64_t perf_past_eco_clk_stop : 1;
    uint64_t perf_past_eco_pwr_off : 1;
    uint64_t _reserved0 : 56;
#else
    uint64_t _reserved0 : 56;
    uint64_t perf_past_eco_pwr_off : 1;
    uint64_t perf_past_eco_clk_stop : 1;
    uint64_t perf_past_core_pwr_off : 1;
    uint64_t perf_past_core_clk_stop : 1;
    uint64_t perf_past_core_instruct_stop : 1;
    uint64_t perf_pm_state : 3;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmstatehistperf_reg_t;



typedef union pcbs_idlefsmgotocmd_reg {

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
    uint64_t idle_fsm_goto_cmd : 2;
    uint64_t babystp_trigger_sleep_entry : 1;
    uint64_t babystp_trigger_winkle_entry : 1;
    uint64_t babystp_trigger_wakeup : 1;
    uint64_t _reserved0 : 59;
#else
    uint64_t _reserved0 : 59;
    uint64_t babystp_trigger_wakeup : 1;
    uint64_t babystp_trigger_winkle_entry : 1;
    uint64_t babystp_trigger_sleep_entry : 1;
    uint64_t idle_fsm_goto_cmd : 2;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_idlefsmgotocmd_reg_t;



typedef union pcbs_corepfpudly_reg {

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
    uint64_t core_powup_dly0 : 4;
    uint64_t core_powup_dly1 : 4;
    uint64_t core_power_up_delay_sel : 12;
    uint64_t _reserved0 : 44;
#else
    uint64_t _reserved0 : 44;
    uint64_t core_power_up_delay_sel : 12;
    uint64_t core_powup_dly1 : 4;
    uint64_t core_powup_dly0 : 4;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_corepfpudly_reg_t;



typedef union pcbs_corepfpddly_reg {

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
    uint64_t core_powdn_dly0 : 4;
    uint64_t core_powdn_dly1 : 4;
    uint64_t core_power_dn_delay_sel : 12;
    uint64_t _reserved0 : 44;
#else
    uint64_t _reserved0 : 44;
    uint64_t core_power_dn_delay_sel : 12;
    uint64_t core_powdn_dly1 : 4;
    uint64_t core_powdn_dly0 : 4;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_corepfpddly_reg_t;



typedef union pcbs_corepfvret_reg {

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
    uint64_t core_vret_sel : 4;
    uint64_t core_voff_sel : 4;
    uint64_t _reserved0 : 56;
#else
    uint64_t _reserved0 : 56;
    uint64_t core_voff_sel : 4;
    uint64_t core_vret_sel : 4;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_corepfvret_reg_t;



typedef union pcbs_ecopfpudly_reg {

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
    uint64_t eco_powup_dly0 : 4;
    uint64_t eco_powup_dly1 : 4;
    uint64_t eco_power_up_delay_sel : 12;
    uint64_t _reserved0 : 44;
#else
    uint64_t _reserved0 : 44;
    uint64_t eco_power_up_delay_sel : 12;
    uint64_t eco_powup_dly1 : 4;
    uint64_t eco_powup_dly0 : 4;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_ecopfpudly_reg_t;



typedef union pcbs_ecopfpddly_reg {

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
    uint64_t eco_powdn_dly0 : 4;
    uint64_t eco_powdn_dly1 : 4;
    uint64_t eco_power_dn_delay_sel : 12;
    uint64_t _reserved0 : 44;
#else
    uint64_t _reserved0 : 44;
    uint64_t eco_power_dn_delay_sel : 12;
    uint64_t eco_powdn_dly1 : 4;
    uint64_t eco_powdn_dly0 : 4;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_ecopfpddly_reg_t;



typedef union pcbs_ecopfvret_reg {

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
    uint64_t eco_vret_sel : 4;
    uint64_t eco_voff_sel : 4;
    uint64_t _reserved0 : 56;
#else
    uint64_t _reserved0 : 56;
    uint64_t eco_voff_sel : 4;
    uint64_t eco_vret_sel : 4;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_ecopfvret_reg_t;



typedef union pcbs_freq_ctrl_reg {

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
    uint64_t dpll_fmin : 9;
    uint64_t dpll_fmax : 9;
    uint64_t dpll_fmax_bias : 4;
    uint64_t frequ_at_pstate0 : 9;
    uint64_t _reserved0 : 33;
#else
    uint64_t _reserved0 : 33;
    uint64_t frequ_at_pstate0 : 9;
    uint64_t dpll_fmax_bias : 4;
    uint64_t dpll_fmax : 9;
    uint64_t dpll_fmin : 9;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_freq_ctrl_reg_t;



typedef union pcbs_dpll_cpm_parm_reg {

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
    uint64_t lf_slewratexpi : 8;
    uint64_t lf_use_cpmxpi : 1;
    uint64_t ff_use_cpmxpi : 1;
    uint64_t cpm_filter_enable : 1;
    uint64_t ff_bypassxpi : 1;
    uint64_t dco_override : 1;
    uint64_t dco_incr : 1;
    uint64_t dco_decr : 1;
    uint64_t dpll_lock_timer_replacement_value : 9;
    uint64_t pre_vret_pstate : 8;
    uint64_t override_pcbs_dpll_synchronizer : 1;
    uint64_t dpll_char_delta1 : 4;
    uint64_t dpll_char_delta2 : 4;
    uint64_t _reserved0 : 23;
#else
    uint64_t _reserved0 : 23;
    uint64_t dpll_char_delta2 : 4;
    uint64_t dpll_char_delta1 : 4;
    uint64_t override_pcbs_dpll_synchronizer : 1;
    uint64_t pre_vret_pstate : 8;
    uint64_t dpll_lock_timer_replacement_value : 9;
    uint64_t dco_decr : 1;
    uint64_t dco_incr : 1;
    uint64_t dco_override : 1;
    uint64_t ff_bypassxpi : 1;
    uint64_t cpm_filter_enable : 1;
    uint64_t ff_use_cpmxpi : 1;
    uint64_t lf_use_cpmxpi : 1;
    uint64_t lf_slewratexpi : 8;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_dpll_cpm_parm_reg_t;



typedef union pcbs_power_management_status_reg {

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
    int64_t global_pstate_actual : 8;
    int64_t local_pstate_actual : 8;
    int64_t pv_min : 8;
    int64_t pvf_max : 8;
    uint64_t spr_em_disabled : 1;
    uint64_t psafe_mode_active : 1;
    uint64_t ivrm_safe_mode_active : 1;
    uint64_t ivrm_enable : 1;
    uint64_t all_fsms_in_safe_state : 1;
    uint64_t pmsr_spares : 4;
    uint64_t _reserved0 : 23;
#else
    uint64_t _reserved0 : 23;
    uint64_t pmsr_spares : 4;
    uint64_t all_fsms_in_safe_state : 1;
    uint64_t ivrm_enable : 1;
    uint64_t ivrm_safe_mode_active : 1;
    uint64_t psafe_mode_active : 1;
    uint64_t spr_em_disabled : 1;
    int64_t pvf_max : 8;
    int64_t pv_min : 8;
    int64_t local_pstate_actual : 8;
    int64_t global_pstate_actual : 8;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_power_management_status_reg_t;



typedef union pcbs_ivrm_control_status_reg {

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
    uint64_t ivrm_fsm_enable : 1;
    uint64_t use_ivrm_for_vret : 1;
    uint64_t binsearch_cal_ena : 1;
    uint64_t pvref_en : 1;
    uint64_t ivrm_core_vdd_bypass_b : 1;
    uint64_t ivrm_core_vdd_poweron : 1;
    uint64_t ivrm_core_vcs_bypass_b : 1;
    uint64_t ivrm_core_vcs_poweron : 1;
    uint64_t ivrm_eco_vdd_bypass_b : 1;
    uint64_t ivrm_eco_vdd_poweron : 1;
    uint64_t ivrm_eco_vcs_bypass_b : 1;
    uint64_t ivrm_eco_vcs_poweron : 1;
    uint64_t ivrm_vret_vdd : 7;
    uint64_t ivrm_vret_vcs : 7;
    uint64_t ivrm_vret_core_vdd_pfet_strength : 5;
    uint64_t ivrm_vret_core_vcs_pfet_strength : 5;
    uint64_t ivrm_vret_eco_vdd_pfet_strength : 5;
    uint64_t ivrm_vret_eco_vcs_pfet_strength : 5;
    uint64_t pvref_fail : 1;
    uint64_t ivrm_pref_error_gross : 1;
    uint64_t ivrm_pref_error_fine : 1;
    uint64_t ivrm_core_vdd_range_hi : 1;
    uint64_t ivrm_core_vdd_range_lo : 1;
    uint64_t ivrm_eco_vdd_range_hi : 1;
    uint64_t ivrm_eco_vdd_range_lo : 1;
    uint64_t ivrm_core_vcs_range_hi : 1;
    uint64_t ivrm_core_vcs_range_lo : 1;
    uint64_t ivrm_eco_vcs_range_hi : 1;
    uint64_t ivrm_eco_vcs_range_lo : 1;
    uint64_t binsearch_cal_done : 1;
    uint64_t ivrm_core_vdd_pfet_low_vout : 1;
    uint64_t ivrm_core_vcs_pfet_low_vout : 1;
    uint64_t ivrm_eco_vdd_pfet_low_vout : 1;
    uint64_t ivrm_eco_vcs_pfet_low_vout : 1;
    uint64_t ivrm_power_down_disable : 1;
    uint64_t _reserved0 : 1;
#else
    uint64_t _reserved0 : 1;
    uint64_t ivrm_power_down_disable : 1;
    uint64_t ivrm_eco_vcs_pfet_low_vout : 1;
    uint64_t ivrm_eco_vdd_pfet_low_vout : 1;
    uint64_t ivrm_core_vcs_pfet_low_vout : 1;
    uint64_t ivrm_core_vdd_pfet_low_vout : 1;
    uint64_t binsearch_cal_done : 1;
    uint64_t ivrm_eco_vcs_range_lo : 1;
    uint64_t ivrm_eco_vcs_range_hi : 1;
    uint64_t ivrm_core_vcs_range_lo : 1;
    uint64_t ivrm_core_vcs_range_hi : 1;
    uint64_t ivrm_eco_vdd_range_lo : 1;
    uint64_t ivrm_eco_vdd_range_hi : 1;
    uint64_t ivrm_core_vdd_range_lo : 1;
    uint64_t ivrm_core_vdd_range_hi : 1;
    uint64_t ivrm_pref_error_fine : 1;
    uint64_t ivrm_pref_error_gross : 1;
    uint64_t pvref_fail : 1;
    uint64_t ivrm_vret_eco_vcs_pfet_strength : 5;
    uint64_t ivrm_vret_eco_vdd_pfet_strength : 5;
    uint64_t ivrm_vret_core_vcs_pfet_strength : 5;
    uint64_t ivrm_vret_core_vdd_pfet_strength : 5;
    uint64_t ivrm_vret_vcs : 7;
    uint64_t ivrm_vret_vdd : 7;
    uint64_t ivrm_eco_vcs_poweron : 1;
    uint64_t ivrm_eco_vcs_bypass_b : 1;
    uint64_t ivrm_eco_vdd_poweron : 1;
    uint64_t ivrm_eco_vdd_bypass_b : 1;
    uint64_t ivrm_core_vcs_poweron : 1;
    uint64_t ivrm_core_vcs_bypass_b : 1;
    uint64_t ivrm_core_vdd_poweron : 1;
    uint64_t ivrm_core_vdd_bypass_b : 1;
    uint64_t pvref_en : 1;
    uint64_t binsearch_cal_ena : 1;
    uint64_t use_ivrm_for_vret : 1;
    uint64_t ivrm_fsm_enable : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_ivrm_control_status_reg_t;



typedef union pcbs_ivrm_value_setting_reg {

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
    uint64_t ivrm_core_vdd_ivid : 8;
    uint64_t ivrm_core_vcs_ivid : 8;
    uint64_t ivrm_eco_vdd_ivid : 8;
    uint64_t ivrm_eco_vcs_ivid : 8;
    uint64_t ivrm_core_vdd_pfet_strength : 5;
    uint64_t ivrm_core_vcs_pfet_strength : 5;
    uint64_t ivrm_eco_vdd_pfet_strength : 5;
    uint64_t ivrm_eco_vcs_pfet_strength : 5;
    uint64_t ivrm_vdd_core_pfetstr_valid : 1;
    uint64_t ivrm_vcs_core_pfetstr_valid : 1;
    uint64_t ivrm_vdd_eco_pfetstr_valid : 1;
    uint64_t ivrm_vcs_eco_pfetstr_valid : 1;
    uint64_t core_vdd_vpump_en : 1;
    uint64_t core_vcs_vpump_en : 1;
    uint64_t eco_vdd_vpump_en : 1;
    uint64_t eco_vcs_vpump_en : 1;
    uint64_t _reserved0 : 4;
#else
    uint64_t _reserved0 : 4;
    uint64_t eco_vcs_vpump_en : 1;
    uint64_t eco_vdd_vpump_en : 1;
    uint64_t core_vcs_vpump_en : 1;
    uint64_t core_vdd_vpump_en : 1;
    uint64_t ivrm_vcs_eco_pfetstr_valid : 1;
    uint64_t ivrm_vdd_eco_pfetstr_valid : 1;
    uint64_t ivrm_vcs_core_pfetstr_valid : 1;
    uint64_t ivrm_vdd_core_pfetstr_valid : 1;
    uint64_t ivrm_eco_vcs_pfet_strength : 5;
    uint64_t ivrm_eco_vdd_pfet_strength : 5;
    uint64_t ivrm_core_vcs_pfet_strength : 5;
    uint64_t ivrm_core_vdd_pfet_strength : 5;
    uint64_t ivrm_eco_vcs_ivid : 8;
    uint64_t ivrm_eco_vdd_ivid : 8;
    uint64_t ivrm_core_vcs_ivid : 8;
    uint64_t ivrm_core_vdd_ivid : 8;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_ivrm_value_setting_reg_t;



typedef union pcbs_pcbspm_mode_reg {

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
    uint64_t enable_pstate_mode : 1;
    uint64_t global_pstate_change_for_idle_state_enabled : 1;
    uint64_t enable_global_pstate_req : 1;
    uint64_t enable_winkle_with_cpm_mode : 1;
    uint64_t enable_clipping_of_global_pstate_req : 1;
    uint64_t chksw_hw214553 : 1;
    uint64_t enable_pmc_pmax_sync_notification : 1;
    uint64_t dpll_lock_replacement_timer_mode_en : 1;
    uint64_t dpll_freqout_mode_en : 1;
    uint64_t dpll_flock_mode_en : 1;
    uint64_t enable_sense_delay_characterization : 1;
    uint64_t sense_delay_timer_val : 7;
    uint64_t cpm_fmin_clip_error_sel : 2;
    uint64_t dbg_trace_sel : 4;
    uint64_t trace_data_sel : 2;
    uint64_t tp_cplt_ivrm_vpp_tune : 4;
    uint64_t _reserved0 : 34;
#else
    uint64_t _reserved0 : 34;
    uint64_t tp_cplt_ivrm_vpp_tune : 4;
    uint64_t trace_data_sel : 2;
    uint64_t dbg_trace_sel : 4;
    uint64_t cpm_fmin_clip_error_sel : 2;
    uint64_t sense_delay_timer_val : 7;
    uint64_t enable_sense_delay_characterization : 1;
    uint64_t dpll_flock_mode_en : 1;
    uint64_t dpll_freqout_mode_en : 1;
    uint64_t dpll_lock_replacement_timer_mode_en : 1;
    uint64_t enable_pmc_pmax_sync_notification : 1;
    uint64_t chksw_hw214553 : 1;
    uint64_t enable_clipping_of_global_pstate_req : 1;
    uint64_t enable_winkle_with_cpm_mode : 1;
    uint64_t enable_global_pstate_req : 1;
    uint64_t global_pstate_change_for_idle_state_enabled : 1;
    uint64_t enable_pstate_mode : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pcbspm_mode_reg_t;

#endif // __ASSEMBLER__
#define PCBS_PCBSPM_MODE_REG_ENABLE_PSTATE_MODE SIXTYFOUR_BIT_CONSTANT(0x8000000000000000)
#define PCBS_PCBSPM_MODE_REG_GLOBAL_PSTATE_CHANGE_FOR_IDLE_STATE_ENABLED SIXTYFOUR_BIT_CONSTANT(0x4000000000000000)
#define PCBS_PCBSPM_MODE_REG_ENABLE_GLOBAL_PSTATE_REQ SIXTYFOUR_BIT_CONSTANT(0x2000000000000000)
#define PCBS_PCBSPM_MODE_REG_ENABLE_WINKLE_WITH_CPM_MODE SIXTYFOUR_BIT_CONSTANT(0x1000000000000000)
#define PCBS_PCBSPM_MODE_REG_ENABLE_CLIPPING_OF_GLOBAL_PSTATE_REQ SIXTYFOUR_BIT_CONSTANT(0x0800000000000000)
#define PCBS_PCBSPM_MODE_REG_CHKSW_HW214553 SIXTYFOUR_BIT_CONSTANT(0x0400000000000000)
#define PCBS_PCBSPM_MODE_REG_ENABLE_PMC_PMAX_SYNC_NOTIFICATION SIXTYFOUR_BIT_CONSTANT(0x0200000000000000)
#define PCBS_PCBSPM_MODE_REG_DPLL_LOCK_REPLACEMENT_TIMER_MODE_EN SIXTYFOUR_BIT_CONSTANT(0x0100000000000000)
#define PCBS_PCBSPM_MODE_REG_DPLL_FREQOUT_MODE_EN SIXTYFOUR_BIT_CONSTANT(0x0080000000000000)
#define PCBS_PCBSPM_MODE_REG_DPLL_FLOCK_MODE_EN SIXTYFOUR_BIT_CONSTANT(0x0040000000000000)
#define PCBS_PCBSPM_MODE_REG_ENABLE_SENSE_DELAY_CHARACTERIZATION SIXTYFOUR_BIT_CONSTANT(0x0020000000000000)
#define PCBS_PCBSPM_MODE_REG_SENSE_DELAY_TIMER_VAL_MASK SIXTYFOUR_BIT_CONSTANT(0x001fc00000000000)
#define PCBS_PCBSPM_MODE_REG_CPM_FMIN_CLIP_ERROR_SEL_MASK SIXTYFOUR_BIT_CONSTANT(0x0000300000000000)
#define PCBS_PCBSPM_MODE_REG_DBG_TRACE_SEL_MASK SIXTYFOUR_BIT_CONSTANT(0x00000f0000000000)
#define PCBS_PCBSPM_MODE_REG_TRACE_DATA_SEL_MASK SIXTYFOUR_BIT_CONSTANT(0x000000c000000000)
#define PCBS_PCBSPM_MODE_REG_TP_CPLT_IVRM_VPP_TUNE_MASK SIXTYFOUR_BIT_CONSTANT(0x0000003c00000000)
#ifndef __ASSEMBLER__


typedef union pcbs_ivrm_pfetstr_sense_reg {

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
    uint64_t ivrm_core_vdd_pfetstr_sns : 5;
    uint64_t ivrm_core_vcs_pfetstr_sns : 5;
    uint64_t ivrm_eco_vdd_pfetstr_sns : 5;
    uint64_t ivrm_eco_vcs_pfetstr_sns : 5;
    uint64_t ivrm_vdd_core_pfetstr_valid_sns : 1;
    uint64_t ivrm_vcs_core_pfetstr_valid_sns : 1;
    uint64_t ivrm_vdd_eco_pfetstr_valid_sns : 1;
    uint64_t ivrm_vcs_eco_pfetstr_valid_sns : 1;
    uint64_t core_vdd_bypass_b_sense : 1;
    uint64_t core_vcs_bypass_b_sense : 1;
    uint64_t eco_vdd_bypass_b_sense : 1;
    uint64_t eco_vcs_bypass_b_sense : 1;
    uint64_t core_vdd_poweron_sense : 1;
    uint64_t core_vcs_poweron_sense : 1;
    uint64_t eco_vdd_poweron_sense : 1;
    uint64_t eco_vcs_poweron_sense : 1;
    uint64_t core_vdd_vpump_en_sense : 1;
    uint64_t core_vcs_vpump_en_sense : 1;
    uint64_t eco_vdd_vpump_en_sense : 1;
    uint64_t eco_vcs_vpump_en_sense : 1;
    uint64_t core_vdd_pfet_low_vout_sns : 1;
    uint64_t core_vcs_pfet_low_vout_sns : 1;
    uint64_t eco_vdd_pfet_low_vout_sns : 1;
    uint64_t eco_vcs_pfet_low_vout_sns : 1;
    uint64_t _reserved0 : 24;
#else
    uint64_t _reserved0 : 24;
    uint64_t eco_vcs_pfet_low_vout_sns : 1;
    uint64_t eco_vdd_pfet_low_vout_sns : 1;
    uint64_t core_vcs_pfet_low_vout_sns : 1;
    uint64_t core_vdd_pfet_low_vout_sns : 1;
    uint64_t eco_vcs_vpump_en_sense : 1;
    uint64_t eco_vdd_vpump_en_sense : 1;
    uint64_t core_vcs_vpump_en_sense : 1;
    uint64_t core_vdd_vpump_en_sense : 1;
    uint64_t eco_vcs_poweron_sense : 1;
    uint64_t eco_vdd_poweron_sense : 1;
    uint64_t core_vcs_poweron_sense : 1;
    uint64_t core_vdd_poweron_sense : 1;
    uint64_t eco_vcs_bypass_b_sense : 1;
    uint64_t eco_vdd_bypass_b_sense : 1;
    uint64_t core_vcs_bypass_b_sense : 1;
    uint64_t core_vdd_bypass_b_sense : 1;
    uint64_t ivrm_vcs_eco_pfetstr_valid_sns : 1;
    uint64_t ivrm_vdd_eco_pfetstr_valid_sns : 1;
    uint64_t ivrm_vcs_core_pfetstr_valid_sns : 1;
    uint64_t ivrm_vdd_core_pfetstr_valid_sns : 1;
    uint64_t ivrm_eco_vcs_pfetstr_sns : 5;
    uint64_t ivrm_eco_vdd_pfetstr_sns : 5;
    uint64_t ivrm_core_vcs_pfetstr_sns : 5;
    uint64_t ivrm_core_vdd_pfetstr_sns : 5;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_ivrm_pfetstr_sense_reg_t;



typedef union pcbs_power_management_idle_control_reg {

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
    int64_t nap_pstate_req : 8;
    uint64_t nap_pstate_en : 1;
    uint64_t nap_global_en : 1;
    uint64_t nap_latency : 2;
    uint64_t reserved_ppmicr_0 : 4;
    int64_t sleep_pstate_req : 8;
    uint64_t sleep_pstate_en : 1;
    uint64_t sleep_global_en : 1;
    uint64_t sleep_latency : 2;
    uint64_t reserved_ppmicr_1 : 4;
    int64_t winkle_pstate_req : 8;
    uint64_t winkle_pstate_en : 1;
    uint64_t winkle_global_en : 1;
    uint64_t winkle_latency : 2;
    uint64_t reserved_ppmicr_2 : 4;
    uint64_t _reserved0 : 16;
#else
    uint64_t _reserved0 : 16;
    uint64_t reserved_ppmicr_2 : 4;
    uint64_t winkle_latency : 2;
    uint64_t winkle_global_en : 1;
    uint64_t winkle_pstate_en : 1;
    int64_t winkle_pstate_req : 8;
    uint64_t reserved_ppmicr_1 : 4;
    uint64_t sleep_latency : 2;
    uint64_t sleep_global_en : 1;
    uint64_t sleep_pstate_en : 1;
    int64_t sleep_pstate_req : 8;
    uint64_t reserved_ppmicr_0 : 4;
    uint64_t nap_latency : 2;
    uint64_t nap_global_en : 1;
    uint64_t nap_pstate_en : 1;
    int64_t nap_pstate_req : 8;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_power_management_idle_control_reg_t;



typedef union pcbs_power_management_control_reg {

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
    int64_t global_pstate_req : 8;
    int64_t local_pstate_req : 8;
    uint64_t auto_override0_pstate_limit_en : 1;
    uint64_t auto_override1_pstate_limit_en : 1;
    uint64_t reserved_ppmcr : 6;
    int64_t auto_override_pstate0 : 8;
    int64_t auto_override_pstate1 : 8;
    uint64_t _reserved0 : 24;
#else
    uint64_t _reserved0 : 24;
    int64_t auto_override_pstate1 : 8;
    int64_t auto_override_pstate0 : 8;
    uint64_t reserved_ppmcr : 6;
    uint64_t auto_override1_pstate_limit_en : 1;
    uint64_t auto_override0_pstate_limit_en : 1;
    int64_t local_pstate_req : 8;
    int64_t global_pstate_req : 8;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_power_management_control_reg_t;



typedef union pcbs_pmc_vf_ctrl_reg {

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
    int64_t pglobal_actual : 8;
    uint64_t maxregvcs : 8;
    uint64_t maxregvdd : 8;
    uint64_t evidvcs_eff : 8;
    uint64_t evidvdd_eff : 8;
    uint64_t _reserved0 : 24;
#else
    uint64_t _reserved0 : 24;
    uint64_t evidvdd_eff : 8;
    uint64_t evidvcs_eff : 8;
    uint64_t maxregvdd : 8;
    uint64_t maxregvcs : 8;
    int64_t pglobal_actual : 8;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pmc_vf_ctrl_reg_t;



typedef union pcbs_undervolting_reg {

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
    int64_t puv_min : 8;
    int64_t puv_max : 8;
    uint64_t kuv : 6;
    uint64_t _reserved0 : 42;
#else
    uint64_t _reserved0 : 42;
    uint64_t kuv : 6;
    int64_t puv_max : 8;
    int64_t puv_min : 8;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_undervolting_reg_t;



typedef union pcbs_pstate_index_bound_reg {

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
    uint64_t lpsi_min : 8;
    uint64_t lpsi_entries_minus_1 : 7;
    uint64_t _reserved0 : 49;
#else
    uint64_t _reserved0 : 49;
    uint64_t lpsi_entries_minus_1 : 7;
    uint64_t lpsi_min : 8;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pstate_index_bound_reg_t;



typedef union pcbs_power_management_bounds_reg {

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
    int64_t pmin_clip : 8;
    int64_t pmax_clip : 8;
    uint64_t _reserved0 : 48;
#else
    uint64_t _reserved0 : 48;
    int64_t pmax_clip : 8;
    int64_t pmin_clip : 8;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_power_management_bounds_reg_t;



typedef union pcbs_pstate_table_ctrl_reg {

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
    uint64_t pstate_table_address : 7;
    uint64_t _reserved0 : 57;
#else
    uint64_t _reserved0 : 57;
    uint64_t pstate_table_address : 7;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pstate_table_ctrl_reg_t;



typedef union pcbs_pstate_table_reg {

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
    uint64_t pstate_data : 64;
#else
    uint64_t pstate_data : 64;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pstate_table_reg_t;



typedef union pcbs_pstate_step_target_reg {

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
    int64_t local_pstate_eff_req : 8;
    int64_t local_pstate_target : 8;
    int64_t local_core_pstate_step_target : 8;
    int64_t local_eco_pstate_step_target : 8;
    uint64_t _reserved0 : 32;
#else
    uint64_t _reserved0 : 32;
    int64_t local_eco_pstate_step_target : 8;
    int64_t local_core_pstate_step_target : 8;
    int64_t local_pstate_target : 8;
    int64_t local_pstate_eff_req : 8;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_pstate_step_target_reg_t;



typedef union pcbs_dpll_status_reg {

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
    uint64_t dpll_ff_freqout : 15;
    uint64_t dpll_frequ_change : 1;
    uint64_t dpll_status_spare_bit1 : 1;
    uint64_t pmax_sync_pending : 1;
    uint64_t ga_ack_pending : 1;
    int64_t capped_global_pstate_req : 8;
    uint64_t dpll_fmax_and_cpmbit2 : 1;
    uint64_t dpll_fmax_and_cpmbit3 : 1;
    uint64_t dpll_fmax_and_cpmbit4 : 1;
    uint64_t dpll_fmin_and_not_cpmbit2 : 1;
    uint64_t dpll_fmin_and_not_cpmbit1 : 1;
    uint64_t dpll_fmin_and_not_cpmbit0 : 1;
    uint64_t dpll_faster_than_fmax_plus_delta1 : 1;
    uint64_t dpll_slower_than_fmin_minus_delta2 : 1;
    uint64_t dpll_max_freqout_after_last_read : 14;
    uint64_t dpll_min_freqout_after_last_read : 14;
    uint64_t _reserved0 : 1;
#else
    uint64_t _reserved0 : 1;
    uint64_t dpll_min_freqout_after_last_read : 14;
    uint64_t dpll_max_freqout_after_last_read : 14;
    uint64_t dpll_slower_than_fmin_minus_delta2 : 1;
    uint64_t dpll_faster_than_fmax_plus_delta1 : 1;
    uint64_t dpll_fmin_and_not_cpmbit0 : 1;
    uint64_t dpll_fmin_and_not_cpmbit1 : 1;
    uint64_t dpll_fmin_and_not_cpmbit2 : 1;
    uint64_t dpll_fmax_and_cpmbit4 : 1;
    uint64_t dpll_fmax_and_cpmbit3 : 1;
    uint64_t dpll_fmax_and_cpmbit2 : 1;
    int64_t capped_global_pstate_req : 8;
    uint64_t ga_ack_pending : 1;
    uint64_t pmax_sync_pending : 1;
    uint64_t dpll_status_spare_bit1 : 1;
    uint64_t dpll_frequ_change : 1;
    uint64_t dpll_ff_freqout : 15;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_dpll_status_reg_t;



typedef union pcbs_ivrm_vid_control_reg0 {

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
    uint64_t ivrm_req_pstate_stepdelay_rising : 8;
    uint64_t ivrm_req_pstate_stepdelay_lowering : 8;
    uint64_t _reserved0 : 48;
#else
    uint64_t _reserved0 : 48;
    uint64_t ivrm_req_pstate_stepdelay_lowering : 8;
    uint64_t ivrm_req_pstate_stepdelay_rising : 8;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_ivrm_vid_control_reg0_t;



typedef union pcbs_ivrm_vid_control_reg1 {

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
    uint64_t ivrm_stabilize_delay_run : 8;
    uint64_t ivrm_stabilize_delay_idle : 8;
    uint64_t ivrm_pfstr_prop_delay : 8;
    uint64_t ivrm_pfstrvalid_prop_delay : 8;
    uint64_t ivrm_vpump_poweron_time : 8;
    uint64_t ivrm_bypass_delay : 8;
    uint64_t pfet_vpump_enable_delay : 8;
    uint64_t ivrm_vid_vout_threshold : 7;
    uint64_t _reserved0 : 1;
#else
    uint64_t _reserved0 : 1;
    uint64_t ivrm_vid_vout_threshold : 7;
    uint64_t pfet_vpump_enable_delay : 8;
    uint64_t ivrm_bypass_delay : 8;
    uint64_t ivrm_vpump_poweron_time : 8;
    uint64_t ivrm_pfstrvalid_prop_delay : 8;
    uint64_t ivrm_pfstr_prop_delay : 8;
    uint64_t ivrm_stabilize_delay_idle : 8;
    uint64_t ivrm_stabilize_delay_run : 8;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_ivrm_vid_control_reg1_t;



typedef union pcbs_occ_heartbeat_reg {

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
    uint64_t occ_heartbeat_time : 8;
    uint64_t occ_heartbeat_enable : 1;
    uint64_t occ_heartbeat_reg_addr_offset : 8;
    int64_t psafe : 8;
    uint64_t _reserved0 : 39;
#else
    uint64_t _reserved0 : 39;
    int64_t psafe : 8;
    uint64_t occ_heartbeat_reg_addr_offset : 8;
    uint64_t occ_heartbeat_enable : 1;
    uint64_t occ_heartbeat_time : 8;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_occ_heartbeat_reg_t;



typedef union pcbs_resonant_clock_control_reg0 {

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
    uint64_t resclk_dis : 1;
    uint64_t resclk_control_mode : 1;
    uint64_t resclk_sync_pw : 3;
    uint64_t res_sync_delay_cnt : 7;
    uint64_t res_csb_str_instr_lo : 15;
    uint64_t res_csb_str_instr_hi : 15;
    uint64_t _reserved0 : 22;
#else
    uint64_t _reserved0 : 22;
    uint64_t res_csb_str_instr_hi : 15;
    uint64_t res_csb_str_instr_lo : 15;
    uint64_t res_sync_delay_cnt : 7;
    uint64_t resclk_sync_pw : 3;
    uint64_t resclk_control_mode : 1;
    uint64_t resclk_dis : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_resonant_clock_control_reg0_t;



typedef union pcbs_resonant_clock_control_reg1 {

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
    int64_t full_csb_ps : 8;
    int64_t res_low_lower_ps : 8;
    int64_t res_low_upper_ps : 8;
    int64_t res_high_lower_ps : 8;
    int64_t res_high_upper_ps : 8;
    uint64_t nonres_csb_value_ti : 4;
    uint64_t full_csb_value_ti : 4;
    uint64_t resclk_value : 9;
    uint64_t resclk_core_sync_value : 1;
    uint64_t csb_eco_sync_value : 1;
    uint64_t _reserved0 : 5;
#else
    uint64_t _reserved0 : 5;
    uint64_t csb_eco_sync_value : 1;
    uint64_t resclk_core_sync_value : 1;
    uint64_t resclk_value : 9;
    uint64_t full_csb_value_ti : 4;
    uint64_t nonres_csb_value_ti : 4;
    int64_t res_high_upper_ps : 8;
    int64_t res_high_lower_ps : 8;
    int64_t res_low_upper_ps : 8;
    int64_t res_low_lower_ps : 8;
    int64_t full_csb_ps : 8;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_resonant_clock_control_reg1_t;



typedef union pcbs_resonant_clock_status_reg {

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
    uint64_t resclk_state : 1;
    uint64_t res_hi_induct_en : 1;
    uint64_t resclk_inprogress : 1;
    uint64_t resclk_full_csb : 1;
    uint64_t _reserved0 : 60;
#else
    uint64_t _reserved0 : 60;
    uint64_t resclk_full_csb : 1;
    uint64_t resclk_inprogress : 1;
    uint64_t res_hi_induct_en : 1;
    uint64_t resclk_state : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_resonant_clock_status_reg_t;



typedef union pcbs_local_pstate_frequency_target_control_reg {

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
    uint64_t delay_time : 3;
    uint64_t record_transitions : 1;
    uint64_t multiplier : 15;
    uint64_t enable_lpft_function : 1;
    uint64_t _reserved0 : 44;
#else
    uint64_t _reserved0 : 44;
    uint64_t enable_lpft_function : 1;
    uint64_t multiplier : 15;
    uint64_t record_transitions : 1;
    uint64_t delay_time : 3;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_local_pstate_frequency_target_control_reg_t;



typedef union pcbs_local_pstate_frequency_target_status_reg {

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
    uint64_t valid : 1;
    uint64_t cpm_dpll : 1;
    uint64_t ivrm : 1;
    uint64_t transition : 1;
    uint64_t stable : 1;
    uint64_t delta : 24;
    uint64_t cumulative : 24;
    uint64_t pstate : 8;
    uint64_t _reserved0 : 3;
#else
    uint64_t _reserved0 : 3;
    uint64_t pstate : 8;
    uint64_t cumulative : 24;
    uint64_t delta : 24;
    uint64_t stable : 1;
    uint64_t transition : 1;
    uint64_t ivrm : 1;
    uint64_t cpm_dpll : 1;
    uint64_t valid : 1;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_local_pstate_frequency_target_status_reg_t;



typedef union pcbs_fsm_monitor1_reg {

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
    uint64_t babystep_main_fsm : 7;
    uint64_t babystep_slave_fsm : 5;
    uint64_t core_railstepper_main_fsm : 5;
    uint64_t eco_railstepper_main_fsm : 5;
    uint64_t core_railstepper_sub_fsm : 4;
    uint64_t eco_railstepper_sub_fsm : 4;
    uint64_t core_railstepper_byp_fsm : 5;
    uint64_t eco_railstepper_byp_fsm : 5;
    uint64_t ivrm_core_vdd_sequencer_fsm : 6;
    uint64_t ivrm_core_vcs_sequencer_fsm : 6;
    uint64_t ivrm_eco_vdd_sequencer_fsm : 6;
    uint64_t ivrm_eco_vcs_sequencer_fsm : 6;
#else
    uint64_t ivrm_eco_vcs_sequencer_fsm : 6;
    uint64_t ivrm_eco_vdd_sequencer_fsm : 6;
    uint64_t ivrm_core_vcs_sequencer_fsm : 6;
    uint64_t ivrm_core_vdd_sequencer_fsm : 6;
    uint64_t eco_railstepper_byp_fsm : 5;
    uint64_t core_railstepper_byp_fsm : 5;
    uint64_t eco_railstepper_sub_fsm : 4;
    uint64_t core_railstepper_sub_fsm : 4;
    uint64_t eco_railstepper_main_fsm : 5;
    uint64_t core_railstepper_main_fsm : 5;
    uint64_t babystep_slave_fsm : 5;
    uint64_t babystep_main_fsm : 7;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_fsm_monitor1_reg_t;



typedef union pcbs_fsm_monitor2_reg {

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
    uint64_t resclk_band_fsm : 7;
    uint64_t resclk_lowres_fsm : 4;
    uint64_t resclk_highres_fsm : 4;
    uint64_t resclk_fullcsb_fsm : 4;
    uint64_t resclk_update_fsm : 4;
    uint64_t idle_transition_fsm : 7;
    uint64_t peco_step_target_uv : 8;
    uint64_t pcore_step_target_uv : 8;
    uint64_t _reserved0 : 18;
#else
    uint64_t _reserved0 : 18;
    uint64_t pcore_step_target_uv : 8;
    uint64_t peco_step_target_uv : 8;
    uint64_t idle_transition_fsm : 7;
    uint64_t resclk_update_fsm : 4;
    uint64_t resclk_fullcsb_fsm : 4;
    uint64_t resclk_highres_fsm : 4;
    uint64_t resclk_lowres_fsm : 4;
    uint64_t resclk_band_fsm : 7;
#endif // _BIG_ENDIAN
    } fields;
} pcbs_fsm_monitor2_reg_t;



typedef union pcbs_chksw_unassisted_interrupts {

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
} pcbs_chksw_unassisted_interrupts_t;


#endif // __ASSEMBLER__
#endif // __PCBS_FIRMWARE_REGISTERS_H__

