/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/registers/pmc_firmware_registers.h $              */
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
#ifndef __PMC_FIRMWARE_REGISTERS_H__
#define __PMC_FIRMWARE_REGISTERS_H__

/// \file pmc_firmware_registers.h
/// \brief C register structs for the PMC unit

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




typedef union pmc_mode_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t enable_hw_pstate_mode : 1;
    uint32_t enable_fw_auction_pstate_mode : 1;
    uint32_t enable_fw_pstate_mode : 1;
    uint32_t enable_pstate_voltage_changes : 1;
    uint32_t enable_global_actual_pstate_forwarding : 1;
    uint32_t halt_pstate_master_fsm : 1;
    uint32_t enable_interchip_interface : 1;
    uint32_t interchip_mode : 1;
    uint32_t enable_interchip_pstate_in_haps : 1;
    uint32_t enable_pstate_stepping : 1;
    uint32_t honor_oha_idle_state_requests : 1;
    uint32_t vid_endianess : 1;
    uint32_t reset_all_pmc_registers : 1;
    uint32_t safe_mode_without_spivid : 1;
    uint32_t halt_idle_state_master_fsm : 1;
    uint32_t interchip_halt_if : 1;
    uint32_t unfreeze_pstate_processing : 1;
    uint32_t spivid_reset_if : 1;
    uint32_t unfreeze_istate_processing : 1;
    uint32_t _reserved0 : 13;
#else
    uint32_t _reserved0 : 13;
    uint32_t unfreeze_istate_processing : 1;
    uint32_t spivid_reset_if : 1;
    uint32_t unfreeze_pstate_processing : 1;
    uint32_t interchip_halt_if : 1;
    uint32_t halt_idle_state_master_fsm : 1;
    uint32_t safe_mode_without_spivid : 1;
    uint32_t reset_all_pmc_registers : 1;
    uint32_t vid_endianess : 1;
    uint32_t honor_oha_idle_state_requests : 1;
    uint32_t enable_pstate_stepping : 1;
    uint32_t enable_interchip_pstate_in_haps : 1;
    uint32_t interchip_mode : 1;
    uint32_t enable_interchip_interface : 1;
    uint32_t halt_pstate_master_fsm : 1;
    uint32_t enable_global_actual_pstate_forwarding : 1;
    uint32_t enable_pstate_voltage_changes : 1;
    uint32_t enable_fw_pstate_mode : 1;
    uint32_t enable_fw_auction_pstate_mode : 1;
    uint32_t enable_hw_pstate_mode : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_mode_reg_t;



typedef union pmc_hardware_auction_pstate_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    int32_t haps : 8;
    uint32_t kuv_actual : 8;
    uint32_t kuv_received : 8;
    uint32_t _reserved0 : 8;
#else
    uint32_t _reserved0 : 8;
    uint32_t kuv_received : 8;
    uint32_t kuv_actual : 8;
    int32_t haps : 8;
#endif // _BIG_ENDIAN
    } fields;
} pmc_hardware_auction_pstate_reg_t;



typedef union pmc_pstate_monitor_and_ctrl_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    int32_t gpst_val : 8;
    int32_t gpsst : 8;
    int32_t gpsa : 8;
    int32_t gapr : 8;
#else
    int32_t gapr : 8;
    int32_t gpsa : 8;
    int32_t gpsst : 8;
    int32_t gpst_val : 8;
#endif // _BIG_ENDIAN
    } fields;
} pmc_pstate_monitor_and_ctrl_reg_t;



typedef union pmc_rail_bounds_register {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    int32_t pmin_rail : 8;
    int32_t pmax_rail : 8;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    int32_t pmax_rail : 8;
    int32_t pmin_rail : 8;
#endif // _BIG_ENDIAN
    } fields;
} pmc_rail_bounds_register_t;



typedef union pmc_global_pstate_bounds_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t gpsi_min : 8;
    uint32_t gpst_number_of_entries_minus_one : 7;
    uint32_t _reserved0 : 17;
#else
    uint32_t _reserved0 : 17;
    uint32_t gpst_number_of_entries_minus_one : 7;
    uint32_t gpsi_min : 8;
#endif // _BIG_ENDIAN
    } fields;
} pmc_global_pstate_bounds_reg_t;



typedef union pmc_parameter_reg0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t pstate_stepsize : 7;
    uint32_t vrm_stepdelay_range : 4;
    uint32_t vrm_stepdelay_value : 4;
    uint32_t hangpulse_predivider : 6;
    uint32_t gpsa_timeout_value : 8;
    uint32_t gpsa_timeout_value_sel : 1;
    uint32_t _reserved0 : 2;
#else
    uint32_t _reserved0 : 2;
    uint32_t gpsa_timeout_value_sel : 1;
    uint32_t gpsa_timeout_value : 8;
    uint32_t hangpulse_predivider : 6;
    uint32_t vrm_stepdelay_value : 4;
    uint32_t vrm_stepdelay_range : 4;
    uint32_t pstate_stepsize : 7;
#endif // _BIG_ENDIAN
    } fields;
} pmc_parameter_reg0_t;



typedef union pmc_parameter_reg1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t ba_sram_pstate_table : 22;
    int32_t pvsafe : 8;
    uint32_t _reserved0 : 2;
#else
    uint32_t _reserved0 : 2;
    int32_t pvsafe : 8;
    uint32_t ba_sram_pstate_table : 22;
#endif // _BIG_ENDIAN
    } fields;
} pmc_parameter_reg1_t;



typedef union pmc_eff_global_actual_voltage_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t maxreg_vdd : 8;
    uint32_t maxreg_vcs : 8;
    uint32_t eff_evid_vdd : 8;
    uint32_t eff_evid_vcs : 8;
#else
    uint32_t eff_evid_vcs : 8;
    uint32_t eff_evid_vdd : 8;
    uint32_t maxreg_vcs : 8;
    uint32_t maxreg_vdd : 8;
#endif // _BIG_ENDIAN
    } fields;
} pmc_eff_global_actual_voltage_reg_t;



typedef union pmc_global_actual_voltage_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t evid_vdd : 8;
    uint32_t evid_vcs : 8;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t evid_vcs : 8;
    uint32_t evid_vdd : 8;
#endif // _BIG_ENDIAN
    } fields;
} pmc_global_actual_voltage_reg_t;



typedef union pmc_status_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t pstate_processing_is_suspended : 1;
    uint32_t gpsa_bdcst_error : 1;
    uint32_t gpsa_bdcst_resp_info : 3;
    uint32_t gpsa_vchg_error : 1;
    uint32_t gpsa_timeout_error : 1;
    uint32_t gpsa_chg_ongoing : 1;
    uint32_t volt_chg_ongoing : 1;
    uint32_t brd_cst_ongoing : 1;
    uint32_t gps_table_error : 1;
    uint32_t pstate_interchip_error : 1;
    uint32_t istate_processing_is_suspended : 1;
    uint32_t safe_mode_engaged : 1;
    uint32_t _reserved0 : 18;
#else
    uint32_t _reserved0 : 18;
    uint32_t safe_mode_engaged : 1;
    uint32_t istate_processing_is_suspended : 1;
    uint32_t pstate_interchip_error : 1;
    uint32_t gps_table_error : 1;
    uint32_t brd_cst_ongoing : 1;
    uint32_t volt_chg_ongoing : 1;
    uint32_t gpsa_chg_ongoing : 1;
    uint32_t gpsa_timeout_error : 1;
    uint32_t gpsa_vchg_error : 1;
    uint32_t gpsa_bdcst_resp_info : 3;
    uint32_t gpsa_bdcst_error : 1;
    uint32_t pstate_processing_is_suspended : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_status_reg_t;



typedef union pmc_phase_enable_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t phase_enable : 4;
    uint32_t _reserved0 : 28;
#else
    uint32_t _reserved0 : 28;
    uint32_t phase_enable : 4;
#endif // _BIG_ENDIAN
    } fields;
} pmc_phase_enable_reg_t;



typedef union pmc_undervolting_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    int32_t puv_min : 8;
    int32_t puv_max : 8;
    uint32_t kuv_request : 8;
    uint32_t _reserved0 : 8;
#else
    uint32_t _reserved0 : 8;
    uint32_t kuv_request : 8;
    int32_t puv_max : 8;
    int32_t puv_min : 8;
#endif // _BIG_ENDIAN
    } fields;
} pmc_undervolting_reg_t;



typedef union pmc_core_deconfiguration_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t core_chiplet_deconf_vector : 16;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t core_chiplet_deconf_vector : 16;
#endif // _BIG_ENDIAN
    } fields;
} pmc_core_deconfiguration_reg_t;



typedef union pmc_intchp_ctrl_reg1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t interchip_ga_fsm_enable : 1;
    uint32_t interchip_recv_done_valid_without_if_en : 1;
    uint32_t pmcic1_reserved_2 : 1;
    uint32_t interchip_cpha : 1;
    uint32_t interchip_clock_divider : 10;
    uint32_t pmcicr1_reserved_14_17 : 4;
    uint32_t pmcicr1_reserved_18_20 : 3;
    uint32_t _reserved0 : 11;
#else
    uint32_t _reserved0 : 11;
    uint32_t pmcicr1_reserved_18_20 : 3;
    uint32_t pmcicr1_reserved_14_17 : 4;
    uint32_t interchip_clock_divider : 10;
    uint32_t interchip_cpha : 1;
    uint32_t pmcic1_reserved_2 : 1;
    uint32_t interchip_recv_done_valid_without_if_en : 1;
    uint32_t interchip_ga_fsm_enable : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_intchp_ctrl_reg1_t;



typedef union pmc_intchp_ctrl_reg2 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t interchip_ping_send : 1;
    uint32_t interchip_ping_detect_clear : 1;
    uint32_t interchip_ping_mode : 1;
    uint32_t pmcic2_reserved3 : 1;
    uint32_t pmcic2_reserved4 : 1;
    uint32_t pmcic2_reserved5_7 : 3;
    uint32_t _reserved0 : 24;
#else
    uint32_t _reserved0 : 24;
    uint32_t pmcic2_reserved5_7 : 3;
    uint32_t pmcic2_reserved4 : 1;
    uint32_t pmcic2_reserved3 : 1;
    uint32_t interchip_ping_mode : 1;
    uint32_t interchip_ping_detect_clear : 1;
    uint32_t interchip_ping_send : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_intchp_ctrl_reg2_t;



typedef union pmc_intchp_ctrl_reg4 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t interchip_ecc_gen_en : 1;
    uint32_t interchip_ecc_check_en : 1;
    uint32_t interchip_msg_rcv_overflow_check_en : 1;
    uint32_t interchip_ecc_ue_block_en : 1;
    uint32_t chksw_hw221732 : 1;
    uint32_t slave_occ_timeout_forces_safe_mode_disable : 1;
    uint32_t pmcic4_reserved6_7 : 2;
    uint32_t _reserved0 : 24;
#else
    uint32_t _reserved0 : 24;
    uint32_t pmcic4_reserved6_7 : 2;
    uint32_t slave_occ_timeout_forces_safe_mode_disable : 1;
    uint32_t chksw_hw221732 : 1;
    uint32_t interchip_ecc_ue_block_en : 1;
    uint32_t interchip_msg_rcv_overflow_check_en : 1;
    uint32_t interchip_ecc_check_en : 1;
    uint32_t interchip_ecc_gen_en : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_intchp_ctrl_reg4_t;



typedef union pmc_intchp_status_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t interchip_ga_ongoing : 1;
    uint32_t interchip_ecc_ue : 1;
    uint32_t interchip_ecc_ce : 1;
    uint32_t interchip_ping_detected : 1;
    uint32_t interchip_ping_ack_detected : 1;
    uint32_t interchip_msg_send_ongoing : 1;
    uint32_t interchip_msg_recv_detected : 1;
    uint32_t interchip_fsm_err : 1;
    uint32_t interchip_ping_detect_count : 8;
    uint32_t interchip_slave_error_code : 4;
    uint32_t interchip_msg_snd_overflow_detected : 1;
    uint32_t interchip_msg_rcv_overflow_detected : 1;
    uint32_t interchip_ecc_ue_err : 1;
    uint32_t _reserved0 : 9;
#else
    uint32_t _reserved0 : 9;
    uint32_t interchip_ecc_ue_err : 1;
    uint32_t interchip_msg_rcv_overflow_detected : 1;
    uint32_t interchip_msg_snd_overflow_detected : 1;
    uint32_t interchip_slave_error_code : 4;
    uint32_t interchip_ping_detect_count : 8;
    uint32_t interchip_fsm_err : 1;
    uint32_t interchip_msg_recv_detected : 1;
    uint32_t interchip_msg_send_ongoing : 1;
    uint32_t interchip_ping_ack_detected : 1;
    uint32_t interchip_ping_detected : 1;
    uint32_t interchip_ecc_ce : 1;
    uint32_t interchip_ecc_ue : 1;
    uint32_t interchip_ga_ongoing : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_intchp_status_reg_t;



typedef union pmc_intchp_command_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t interchip_reset_if : 1;
    uint32_t interchip_halt_msg_fsm : 1;
    uint32_t interchip_clear_sticky_bits : 1;
    uint32_t _reserved0 : 29;
#else
    uint32_t _reserved0 : 29;
    uint32_t interchip_clear_sticky_bits : 1;
    uint32_t interchip_halt_msg_fsm : 1;
    uint32_t interchip_reset_if : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_intchp_command_reg_t;



typedef union pmc_intchp_msg_wdata {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t interchip_msg_wdata : 32;
#else
    uint32_t interchip_msg_wdata : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_intchp_msg_wdata_t;



typedef union pmc_intchp_msg_rdata {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t interchip_msg_rdata : 32;
#else
    uint32_t interchip_msg_rdata : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_intchp_msg_rdata_t;



typedef union pmc_intchp_pstate_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    int32_t pstate_interchip : 8;
    uint32_t _reserved0 : 24;
#else
    uint32_t _reserved0 : 24;
    int32_t pstate_interchip : 8;
#endif // _BIG_ENDIAN
    } fields;
} pmc_intchp_pstate_reg_t;



typedef union pmc_intchp_globack_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t gaack_interchip : 1;
    int32_t gaack_interchip_pstate : 8;
    uint32_t _reserved0 : 23;
#else
    uint32_t _reserved0 : 23;
    int32_t gaack_interchip_pstate : 8;
    uint32_t gaack_interchip : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_intchp_globack_reg_t;



typedef union pmc_fsmstate_status_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t mis_fsm_state : 3;
    uint32_t mps_fsm_state : 5;
    uint32_t svs_fsm_state : 4;
    uint32_t o2s_fsm_state : 4;
    uint32_t m2p_fsm_state : 4;
    uint32_t o2p_fsm_state : 4;
    uint32_t icp_msg_fsm_state : 5;
    uint32_t _reserved0 : 3;
#else
    uint32_t _reserved0 : 3;
    uint32_t icp_msg_fsm_state : 5;
    uint32_t o2p_fsm_state : 4;
    uint32_t m2p_fsm_state : 4;
    uint32_t o2s_fsm_state : 4;
    uint32_t svs_fsm_state : 4;
    uint32_t mps_fsm_state : 5;
    uint32_t mis_fsm_state : 3;
#endif // _BIG_ENDIAN
    } fields;
} pmc_fsmstate_status_reg_t;



typedef union pmc_trace_mode_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t pmc_trace_mode : 4;
    uint32_t trace_sel_data : 2;
    uint32_t _reserved0 : 26;
#else
    uint32_t _reserved0 : 26;
    uint32_t trace_sel_data : 2;
    uint32_t pmc_trace_mode : 4;
#endif // _BIG_ENDIAN
    } fields;
} pmc_trace_mode_reg_t;



typedef union pmc_spiv_ctrl_reg0a {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t spivid_frame_size : 6;
    uint32_t spivid_out_count1 : 6;
    uint32_t spivid_in_delay1 : 6;
    uint32_t spivid_in_count1 : 6;
    uint32_t _reserved0 : 8;
#else
    uint32_t _reserved0 : 8;
    uint32_t spivid_in_count1 : 6;
    uint32_t spivid_in_delay1 : 6;
    uint32_t spivid_out_count1 : 6;
    uint32_t spivid_frame_size : 6;
#endif // _BIG_ENDIAN
    } fields;
} pmc_spiv_ctrl_reg0a_t;



typedef union pmc_spiv_ctrl_reg0b {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t spivid_out_count2 : 6;
    uint32_t spivid_in_delay2 : 6;
    uint32_t spivid_in_count2 : 6;
    uint32_t _reserved0 : 14;
#else
    uint32_t _reserved0 : 14;
    uint32_t spivid_in_count2 : 6;
    uint32_t spivid_in_delay2 : 6;
    uint32_t spivid_out_count2 : 6;
#endif // _BIG_ENDIAN
    } fields;
} pmc_spiv_ctrl_reg0b_t;



typedef union pmc_spiv_ctrl_reg1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t spivid_fsm_enable : 1;
    uint32_t pmcscr1_reserved_1 : 1;
    uint32_t spivid_cpol : 1;
    uint32_t spivid_cpha : 1;
    uint32_t spivid_clock_divider : 10;
    uint32_t pmcscr1_reserved_2 : 4;
    uint32_t spivid_port_enable : 3;
    uint32_t _reserved0 : 11;
#else
    uint32_t _reserved0 : 11;
    uint32_t spivid_port_enable : 3;
    uint32_t pmcscr1_reserved_2 : 4;
    uint32_t spivid_clock_divider : 10;
    uint32_t spivid_cpha : 1;
    uint32_t spivid_cpol : 1;
    uint32_t pmcscr1_reserved_1 : 1;
    uint32_t spivid_fsm_enable : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_spiv_ctrl_reg1_t;



typedef union pmc_spiv_ctrl_reg2 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t spivid_inter_frame_delay_write_status : 17;
    uint32_t _reserved0 : 15;
#else
    uint32_t _reserved0 : 15;
    uint32_t spivid_inter_frame_delay_write_status : 17;
#endif // _BIG_ENDIAN
    } fields;
} pmc_spiv_ctrl_reg2_t;



typedef union pmc_spiv_ctrl_reg3 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t spivid_inter_retry_delay : 17;
    uint32_t pmc_100ns_pls_range : 6;
    uint32_t _reserved0 : 9;
#else
    uint32_t _reserved0 : 9;
    uint32_t pmc_100ns_pls_range : 6;
    uint32_t spivid_inter_retry_delay : 17;
#endif // _BIG_ENDIAN
    } fields;
} pmc_spiv_ctrl_reg3_t;



typedef union pmc_spiv_ctrl_reg4 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t spivid_crc_gen_en : 1;
    uint32_t spivid_crc_check_en : 1;
    uint32_t spivid_majority_vote_en : 1;
    uint32_t spivid_max_retries : 5;
    uint32_t spivid_crc_polynomial_enables : 8;
    uint32_t spivid_crc_const_gen_enable : 1;
    uint32_t spivid_crc_const_check_enable : 1;
    uint32_t spivid_frame_sync_wrong_enable : 1;
    uint32_t _reserved0 : 13;
#else
    uint32_t _reserved0 : 13;
    uint32_t spivid_frame_sync_wrong_enable : 1;
    uint32_t spivid_crc_const_check_enable : 1;
    uint32_t spivid_crc_const_gen_enable : 1;
    uint32_t spivid_crc_polynomial_enables : 8;
    uint32_t spivid_max_retries : 5;
    uint32_t spivid_majority_vote_en : 1;
    uint32_t spivid_crc_check_en : 1;
    uint32_t spivid_crc_gen_en : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_spiv_ctrl_reg4_t;



typedef union pmc_spiv_status_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t spivid_ongoing : 1;
    uint32_t spivid_crc_error0 : 1;
    uint32_t spivid_crc_error1 : 1;
    uint32_t spivid_crc_error2 : 1;
    uint32_t spivid_retry_timeout : 1;
    uint32_t pmcssr_reserved_1 : 2;
    uint32_t spivid_fsm_err : 1;
    uint32_t spivid_majority_detected_a_minority0 : 1;
    uint32_t spivid_majority_detected_a_minority1 : 1;
    uint32_t spivid_majority_detected_a_minority2 : 1;
    uint32_t spivid_majority_nr_of_minorities0 : 4;
    uint32_t spivid_majority_nr_of_minorities1 : 4;
    uint32_t spivid_majority_nr_of_minorities2 : 4;
    uint32_t _reserved0 : 9;
#else
    uint32_t _reserved0 : 9;
    uint32_t spivid_majority_nr_of_minorities2 : 4;
    uint32_t spivid_majority_nr_of_minorities1 : 4;
    uint32_t spivid_majority_nr_of_minorities0 : 4;
    uint32_t spivid_majority_detected_a_minority2 : 1;
    uint32_t spivid_majority_detected_a_minority1 : 1;
    uint32_t spivid_majority_detected_a_minority0 : 1;
    uint32_t spivid_fsm_err : 1;
    uint32_t pmcssr_reserved_1 : 2;
    uint32_t spivid_retry_timeout : 1;
    uint32_t spivid_crc_error2 : 1;
    uint32_t spivid_crc_error1 : 1;
    uint32_t spivid_crc_error0 : 1;
    uint32_t spivid_ongoing : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_spiv_status_reg_t;



typedef union pmc_spiv_command_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t spivid_halt_fsm : 1;
    uint32_t _reserved0 : 31;
#else
    uint32_t _reserved0 : 31;
    uint32_t spivid_halt_fsm : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_spiv_command_reg_t;



typedef union pmc_o2s_ctrl_reg0a {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t o2s_frame_size : 6;
    uint32_t o2s_out_count1 : 6;
    uint32_t o2s_in_delay1 : 6;
    uint32_t o2s_in_count1 : 6;
    uint32_t _reserved0 : 8;
#else
    uint32_t _reserved0 : 8;
    uint32_t o2s_in_count1 : 6;
    uint32_t o2s_in_delay1 : 6;
    uint32_t o2s_out_count1 : 6;
    uint32_t o2s_frame_size : 6;
#endif // _BIG_ENDIAN
    } fields;
} pmc_o2s_ctrl_reg0a_t;



typedef union pmc_o2s_ctrl_reg0b {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t o2s_out_count2 : 6;
    uint32_t o2s_in_delay2 : 6;
    uint32_t o2s_in_count2 : 6;
    uint32_t _reserved0 : 14;
#else
    uint32_t _reserved0 : 14;
    uint32_t o2s_in_count2 : 6;
    uint32_t o2s_in_delay2 : 6;
    uint32_t o2s_out_count2 : 6;
#endif // _BIG_ENDIAN
    } fields;
} pmc_o2s_ctrl_reg0b_t;



typedef union pmc_o2s_ctrl_reg1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t o2s_bridge_enable : 1;
    uint32_t pmcocr1_reserved_1 : 1;
    uint32_t o2s_cpol : 1;
    uint32_t o2s_cpha : 1;
    uint32_t o2s_clock_divider : 10;
    uint32_t pmcocr1_reserved_2 : 3;
    uint32_t o2s_nr_of_frames : 1;
    uint32_t o2s_port_enable : 3;
    uint32_t _reserved0 : 11;
#else
    uint32_t _reserved0 : 11;
    uint32_t o2s_port_enable : 3;
    uint32_t o2s_nr_of_frames : 1;
    uint32_t pmcocr1_reserved_2 : 3;
    uint32_t o2s_clock_divider : 10;
    uint32_t o2s_cpha : 1;
    uint32_t o2s_cpol : 1;
    uint32_t pmcocr1_reserved_1 : 1;
    uint32_t o2s_bridge_enable : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_o2s_ctrl_reg1_t;



typedef union pmc_o2s_ctrl_reg2 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t o2s_inter_frame_delay : 17;
    uint32_t _reserved0 : 15;
#else
    uint32_t _reserved0 : 15;
    uint32_t o2s_inter_frame_delay : 17;
#endif // _BIG_ENDIAN
    } fields;
} pmc_o2s_ctrl_reg2_t;



typedef union pmc_o2s_ctrl_reg4 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t o2s_crc_gen_en : 1;
    uint32_t o2s_crc_check_en : 1;
    uint32_t o2s_majority_vote_en : 1;
    uint32_t o2s_max_retries : 5;
    uint32_t pmcocr4_reserved8_15 : 8;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t pmcocr4_reserved8_15 : 8;
    uint32_t o2s_max_retries : 5;
    uint32_t o2s_majority_vote_en : 1;
    uint32_t o2s_crc_check_en : 1;
    uint32_t o2s_crc_gen_en : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_o2s_ctrl_reg4_t;



typedef union pmc_o2s_status_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t o2s_ongoing : 1;
    uint32_t o2s_crc_error0 : 1;
    uint32_t o2s_crc_error1 : 1;
    uint32_t o2s_crc_error2 : 1;
    uint32_t o2s_retry_timeout : 1;
    uint32_t o2s_write_while_bridge_busy_err : 1;
    uint32_t pmcosr_reserved_6 : 1;
    uint32_t o2s_fsm_err : 1;
    uint32_t o2s_majority_detected_a_minority0 : 1;
    uint32_t o2s_majority_detected_a_minority1 : 1;
    uint32_t o2s_majority_detected_a_minority2 : 1;
    uint32_t o2s_majority_nr_of_minorities0 : 4;
    uint32_t o2s_majority_nr_of_minorities1 : 4;
    uint32_t o2s_majority_nr_of_minorities2 : 4;
    uint32_t _reserved0 : 9;
#else
    uint32_t _reserved0 : 9;
    uint32_t o2s_majority_nr_of_minorities2 : 4;
    uint32_t o2s_majority_nr_of_minorities1 : 4;
    uint32_t o2s_majority_nr_of_minorities0 : 4;
    uint32_t o2s_majority_detected_a_minority2 : 1;
    uint32_t o2s_majority_detected_a_minority1 : 1;
    uint32_t o2s_majority_detected_a_minority0 : 1;
    uint32_t o2s_fsm_err : 1;
    uint32_t pmcosr_reserved_6 : 1;
    uint32_t o2s_write_while_bridge_busy_err : 1;
    uint32_t o2s_retry_timeout : 1;
    uint32_t o2s_crc_error2 : 1;
    uint32_t o2s_crc_error1 : 1;
    uint32_t o2s_crc_error0 : 1;
    uint32_t o2s_ongoing : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_o2s_status_reg_t;



typedef union pmc_o2s_command_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t o2s_halt_retries : 1;
    uint32_t o2s_clear_sticky_bits : 1;
    uint32_t _reserved0 : 30;
#else
    uint32_t _reserved0 : 30;
    uint32_t o2s_clear_sticky_bits : 1;
    uint32_t o2s_halt_retries : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_o2s_command_reg_t;



typedef union pmc_o2s_wdata_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t o2s_wdata : 32;
#else
    uint32_t o2s_wdata : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_o2s_wdata_reg_t;



typedef union pmc_o2s_rdata_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t o2s_rdata : 32;
#else
    uint32_t o2s_rdata : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_o2s_rdata_reg_t;



typedef union pmc_o2p_addr_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t o2p_write_plus_read : 1;
    uint32_t o2p_mc : 1;
    uint32_t o2p_slave_addr : 6;
    uint32_t o2p_read_not_write : 1;
    uint32_t reserved_bit_pmco2par2 : 3;
    uint32_t o2p_pcb_port : 4;
    uint32_t o2p_pcb_reg_addr : 16;
#else
    uint32_t o2p_pcb_reg_addr : 16;
    uint32_t o2p_pcb_port : 4;
    uint32_t reserved_bit_pmco2par2 : 3;
    uint32_t o2p_read_not_write : 1;
    uint32_t o2p_slave_addr : 6;
    uint32_t o2p_mc : 1;
    uint32_t o2p_write_plus_read : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_o2p_addr_reg_t;



typedef union pmc_o2p_ctrl_status_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t o2p_ongoing : 1;
    uint32_t o2p_scresp : 3;
    uint32_t o2p_write_while_bridge_busy_err : 1;
    uint32_t o2p_fsm_err : 1;
    uint32_t o2p_abort : 1;
    uint32_t o2p_parity_error : 1;
    uint32_t o2p_clear_sticky_bits : 1;
    uint32_t _reserved0 : 23;
#else
    uint32_t _reserved0 : 23;
    uint32_t o2p_clear_sticky_bits : 1;
    uint32_t o2p_parity_error : 1;
    uint32_t o2p_abort : 1;
    uint32_t o2p_fsm_err : 1;
    uint32_t o2p_write_while_bridge_busy_err : 1;
    uint32_t o2p_scresp : 3;
    uint32_t o2p_ongoing : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_o2p_ctrl_status_reg_t;



typedef union pmc_o2p_send_data_hi_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t o2p_send_data_hi : 32;
#else
    uint32_t o2p_send_data_hi : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_o2p_send_data_hi_reg_t;



typedef union pmc_o2p_send_data_lo_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t o2p_send_data_lo : 32;
#else
    uint32_t o2p_send_data_lo : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_o2p_send_data_lo_reg_t;



typedef union pmc_o2p_recv_data_hi_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t o2p_receive_data_hi : 32;
#else
    uint32_t o2p_receive_data_hi : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_o2p_recv_data_hi_reg_t;



typedef union pmc_o2p_recv_data_lo_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t o2p_receive_data_lo : 32;
#else
    uint32_t o2p_receive_data_lo : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_o2p_recv_data_lo_reg_t;



typedef union pmc_occ_heartbeat_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t pmc_occ_heartbeat_time : 16;
    uint32_t pmc_occ_heartbeat_en : 1;
    uint32_t _reserved0 : 15;
#else
    uint32_t _reserved0 : 15;
    uint32_t pmc_occ_heartbeat_en : 1;
    uint32_t pmc_occ_heartbeat_time : 16;
#endif // _BIG_ENDIAN
    } fields;
} pmc_occ_heartbeat_reg_t;



typedef union pmc_error_int_mask_hi_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t pmc_error_int_mask_hi : 32;
#else
    uint32_t pmc_error_int_mask_hi : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_error_int_mask_hi_reg_t;



typedef union pmc_error_int_mask_lo_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t pmc_error_int_mask_lo : 32;
#else
    uint32_t pmc_error_int_mask_lo : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_error_int_mask_lo_reg_t;



typedef union pmc_idle_suspend_mask_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t pmc_idle_suspend_mask : 16;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t pmc_idle_suspend_mask : 16;
#endif // _BIG_ENDIAN
    } fields;
} pmc_idle_suspend_mask_reg_t;



typedef union pmc_pend_idle_req_reg0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t idle_pending_0 : 1;
    uint32_t idle_op_0 : 2;
    uint32_t idle_type_0 : 1;
    uint32_t idle_scope_0 : 1;
    uint32_t assist_mode_0 : 1;
    uint32_t reserved_pirr_0 : 2;
    uint32_t idle_pending_1 : 1;
    uint32_t idle_op_1 : 2;
    uint32_t idle_type_1 : 1;
    uint32_t idle_scope_1 : 1;
    uint32_t assist_mode_1 : 1;
    uint32_t reserved_pirr_1 : 2;
    uint32_t idle_pending_2 : 1;
    uint32_t idle_op_2 : 2;
    uint32_t idle_type_2 : 1;
    uint32_t idle_scope_2 : 1;
    uint32_t assist_mode_2 : 1;
    uint32_t reserved_pirr_2 : 2;
    uint32_t idle_pending_3 : 1;
    uint32_t idle_op_3 : 2;
    uint32_t idle_type_3 : 1;
    uint32_t idle_scope_3 : 1;
    uint32_t assist_mode_3 : 1;
    uint32_t reserved_pirr_3 : 2;
#else
    uint32_t reserved_pirr_3 : 2;
    uint32_t assist_mode_3 : 1;
    uint32_t idle_scope_3 : 1;
    uint32_t idle_type_3 : 1;
    uint32_t idle_op_3 : 2;
    uint32_t idle_pending_3 : 1;
    uint32_t reserved_pirr_2 : 2;
    uint32_t assist_mode_2 : 1;
    uint32_t idle_scope_2 : 1;
    uint32_t idle_type_2 : 1;
    uint32_t idle_op_2 : 2;
    uint32_t idle_pending_2 : 1;
    uint32_t reserved_pirr_1 : 2;
    uint32_t assist_mode_1 : 1;
    uint32_t idle_scope_1 : 1;
    uint32_t idle_type_1 : 1;
    uint32_t idle_op_1 : 2;
    uint32_t idle_pending_1 : 1;
    uint32_t reserved_pirr_0 : 2;
    uint32_t assist_mode_0 : 1;
    uint32_t idle_scope_0 : 1;
    uint32_t idle_type_0 : 1;
    uint32_t idle_op_0 : 2;
    uint32_t idle_pending_0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_pend_idle_req_reg0_t;



typedef union pmc_pend_idle_req_reg1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t idle_pending_4 : 1;
    uint32_t idle_op_4 : 2;
    uint32_t idle_type_4 : 1;
    uint32_t idle_scope_4 : 1;
    uint32_t assist_mode_4 : 1;
    uint32_t reserved_pirr_4 : 2;
    uint32_t idle_pending_5 : 1;
    uint32_t idle_op_5 : 2;
    uint32_t idle_type_5 : 1;
    uint32_t idle_scope_5 : 1;
    uint32_t assist_mode_5 : 1;
    uint32_t reserved_pirr_5 : 2;
    uint32_t idle_pending_6 : 1;
    uint32_t idle_op_6 : 2;
    uint32_t idle_type_6 : 1;
    uint32_t idle_scope_6 : 1;
    uint32_t assist_mode_6 : 1;
    uint32_t reserved_pirr_6 : 2;
    uint32_t idle_pending_7 : 1;
    uint32_t idle_op_7 : 2;
    uint32_t idle_type_7 : 1;
    uint32_t idle_scope_7 : 1;
    uint32_t assist_mode_7 : 1;
    uint32_t reserved_pirr_7 : 2;
#else
    uint32_t reserved_pirr_7 : 2;
    uint32_t assist_mode_7 : 1;
    uint32_t idle_scope_7 : 1;
    uint32_t idle_type_7 : 1;
    uint32_t idle_op_7 : 2;
    uint32_t idle_pending_7 : 1;
    uint32_t reserved_pirr_6 : 2;
    uint32_t assist_mode_6 : 1;
    uint32_t idle_scope_6 : 1;
    uint32_t idle_type_6 : 1;
    uint32_t idle_op_6 : 2;
    uint32_t idle_pending_6 : 1;
    uint32_t reserved_pirr_5 : 2;
    uint32_t assist_mode_5 : 1;
    uint32_t idle_scope_5 : 1;
    uint32_t idle_type_5 : 1;
    uint32_t idle_op_5 : 2;
    uint32_t idle_pending_5 : 1;
    uint32_t reserved_pirr_4 : 2;
    uint32_t assist_mode_4 : 1;
    uint32_t idle_scope_4 : 1;
    uint32_t idle_type_4 : 1;
    uint32_t idle_op_4 : 2;
    uint32_t idle_pending_4 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_pend_idle_req_reg1_t;



typedef union pmc_pend_idle_req_reg2 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t idle_pending_8 : 1;
    uint32_t idle_op_8 : 2;
    uint32_t idle_type_8 : 1;
    uint32_t idle_scope_8 : 1;
    uint32_t assist_mode_8 : 1;
    uint32_t reserved_pirr_8 : 2;
    uint32_t idle_pending_9 : 1;
    uint32_t idle_op_9 : 2;
    uint32_t idle_type_9 : 1;
    uint32_t idle_scope_9 : 1;
    uint32_t assist_mode_9 : 1;
    uint32_t reserved_pirr_9 : 2;
    uint32_t idle_pending_10 : 1;
    uint32_t idle_op_10 : 2;
    uint32_t idle_type_10 : 1;
    uint32_t idle_scope_10 : 1;
    uint32_t assist_mode_10 : 1;
    uint32_t reserved_pirr_10 : 2;
    uint32_t idle_pending_11 : 1;
    uint32_t idle_op_11 : 2;
    uint32_t idle_type_11 : 1;
    uint32_t idle_scope_11 : 1;
    uint32_t assist_mode_11 : 1;
    uint32_t reserved_pirr_11 : 2;
#else
    uint32_t reserved_pirr_11 : 2;
    uint32_t assist_mode_11 : 1;
    uint32_t idle_scope_11 : 1;
    uint32_t idle_type_11 : 1;
    uint32_t idle_op_11 : 2;
    uint32_t idle_pending_11 : 1;
    uint32_t reserved_pirr_10 : 2;
    uint32_t assist_mode_10 : 1;
    uint32_t idle_scope_10 : 1;
    uint32_t idle_type_10 : 1;
    uint32_t idle_op_10 : 2;
    uint32_t idle_pending_10 : 1;
    uint32_t reserved_pirr_9 : 2;
    uint32_t assist_mode_9 : 1;
    uint32_t idle_scope_9 : 1;
    uint32_t idle_type_9 : 1;
    uint32_t idle_op_9 : 2;
    uint32_t idle_pending_9 : 1;
    uint32_t reserved_pirr_8 : 2;
    uint32_t assist_mode_8 : 1;
    uint32_t idle_scope_8 : 1;
    uint32_t idle_type_8 : 1;
    uint32_t idle_op_8 : 2;
    uint32_t idle_pending_8 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_pend_idle_req_reg2_t;



typedef union pmc_pend_idle_req_reg3 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t idle_pending_12 : 1;
    uint32_t idle_op_12 : 2;
    uint32_t idle_type_12 : 1;
    uint32_t idle_scope_12 : 1;
    uint32_t assist_mode_12 : 1;
    uint32_t reserved_pirr_12 : 2;
    uint32_t idle_pending_13 : 1;
    uint32_t idle_op_13 : 2;
    uint32_t idle_type_13 : 1;
    uint32_t idle_scope_13 : 1;
    uint32_t assist_mode_13 : 1;
    uint32_t reserved_pirr_13 : 2;
    uint32_t idle_pending_14 : 1;
    uint32_t idle_op_14 : 2;
    uint32_t idle_type_14 : 1;
    uint32_t idle_scope_14 : 1;
    uint32_t assist_mode_14 : 1;
    uint32_t reserved_pirr_14 : 2;
    uint32_t idle_pending_15 : 1;
    uint32_t idle_op_15 : 2;
    uint32_t idle_type_15 : 1;
    uint32_t idle_scope_15 : 1;
    uint32_t assist_mode_15 : 1;
    uint32_t reserved_pirr_15 : 2;
#else
    uint32_t reserved_pirr_15 : 2;
    uint32_t assist_mode_15 : 1;
    uint32_t idle_scope_15 : 1;
    uint32_t idle_type_15 : 1;
    uint32_t idle_op_15 : 2;
    uint32_t idle_pending_15 : 1;
    uint32_t reserved_pirr_14 : 2;
    uint32_t assist_mode_14 : 1;
    uint32_t idle_scope_14 : 1;
    uint32_t idle_type_14 : 1;
    uint32_t idle_op_14 : 2;
    uint32_t idle_pending_14 : 1;
    uint32_t reserved_pirr_13 : 2;
    uint32_t assist_mode_13 : 1;
    uint32_t idle_scope_13 : 1;
    uint32_t idle_type_13 : 1;
    uint32_t idle_op_13 : 2;
    uint32_t idle_pending_13 : 1;
    uint32_t reserved_pirr_12 : 2;
    uint32_t assist_mode_12 : 1;
    uint32_t idle_scope_12 : 1;
    uint32_t idle_type_12 : 1;
    uint32_t idle_op_12 : 2;
    uint32_t idle_pending_12 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_pend_idle_req_reg3_t;



typedef union pmc_sleep_int_req_vec_reg0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t fastsleepentry_int_req_vec : 32;
#else
    uint32_t fastsleepentry_int_req_vec : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_sleep_int_req_vec_reg0_t;



typedef union pmc_sleep_int_req_vec_reg1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t deepsleepentry_int_req_vec : 32;
#else
    uint32_t deepsleepentry_int_req_vec : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_sleep_int_req_vec_reg1_t;



typedef union pmc_sleep_int_req_vec_reg2 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t fastsleepexit_int_req_vec : 32;
#else
    uint32_t fastsleepexit_int_req_vec : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_sleep_int_req_vec_reg2_t;



typedef union pmc_sleep_int_req_vec_reg3 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t deepsleepexit_int_req_vec : 32;
#else
    uint32_t deepsleepexit_int_req_vec : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_sleep_int_req_vec_reg3_t;



typedef union pmc_winkle_int_req_vec_reg0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t fastwinkleentry_int_req_vec : 32;
#else
    uint32_t fastwinkleentry_int_req_vec : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_winkle_int_req_vec_reg0_t;



typedef union pmc_winkle_int_req_vec_reg1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t deepwinkleentry_int_req_vec : 32;
#else
    uint32_t deepwinkleentry_int_req_vec : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_winkle_int_req_vec_reg1_t;



typedef union pmc_winkle_int_req_vec_reg2 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t fastwinkleexit_int_req_vec : 32;
#else
    uint32_t fastwinkleexit_int_req_vec : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_winkle_int_req_vec_reg2_t;



typedef union pmc_winkle_int_req_vec_reg3 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t deepwinkleexit_int_req_vec : 32;
#else
    uint32_t deepwinkleexit_int_req_vec : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_winkle_int_req_vec_reg3_t;



typedef union pmc_nap_int_req_vec_reg0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t napentry_int_req_vec : 16;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t napentry_int_req_vec : 16;
#endif // _BIG_ENDIAN
    } fields;
} pmc_nap_int_req_vec_reg0_t;



typedef union pmc_nap_int_req_vec_reg1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t napexit_int_req_vec : 25;
    uint32_t _reserved0 : 7;
#else
    uint32_t _reserved0 : 7;
    uint32_t napexit_int_req_vec : 25;
#endif // _BIG_ENDIAN
    } fields;
} pmc_nap_int_req_vec_reg1_t;



typedef union pmc_pore_req_reg0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t porrr_reserved0 : 8;
    uint32_t porrr_start_vector : 4;
    uint32_t porrr_reserved1 : 8;
    uint32_t porrr_pore_busy : 1;
    uint32_t porrr_pore_suspended : 1;
    uint32_t porrr_porrtc_busy : 1;
    uint32_t _reserved0 : 9;
#else
    uint32_t _reserved0 : 9;
    uint32_t porrr_porrtc_busy : 1;
    uint32_t porrr_pore_suspended : 1;
    uint32_t porrr_pore_busy : 1;
    uint32_t porrr_reserved1 : 8;
    uint32_t porrr_start_vector : 4;
    uint32_t porrr_reserved0 : 8;
#endif // _BIG_ENDIAN
    } fields;
} pmc_pore_req_reg0_t;



typedef union pmc_pore_req_reg1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t porrr_chiplet_enable_0 : 1;
    uint32_t porrr_chiplet_enable_1 : 1;
    uint32_t porrr_chiplet_enable_2 : 1;
    uint32_t porrr_chiplet_enable_3 : 1;
    uint32_t porrr_chiplet_enable_4 : 1;
    uint32_t porrr_chiplet_enable_5 : 1;
    uint32_t porrr_chiplet_enable_6 : 1;
    uint32_t porrr_chiplet_enable_7 : 1;
    uint32_t porrr_chiplet_enable_8 : 1;
    uint32_t porrr_chiplet_enable_9 : 1;
    uint32_t porrr_chiplet_enable_10 : 1;
    uint32_t porrr_chiplet_enable_11 : 1;
    uint32_t porrr_chiplet_enable_12 : 1;
    uint32_t porrr_chiplet_enable_13 : 1;
    uint32_t porrr_chiplet_enable_14 : 1;
    uint32_t porrr_chiplet_enable_15 : 1;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t porrr_chiplet_enable_15 : 1;
    uint32_t porrr_chiplet_enable_14 : 1;
    uint32_t porrr_chiplet_enable_13 : 1;
    uint32_t porrr_chiplet_enable_12 : 1;
    uint32_t porrr_chiplet_enable_11 : 1;
    uint32_t porrr_chiplet_enable_10 : 1;
    uint32_t porrr_chiplet_enable_9 : 1;
    uint32_t porrr_chiplet_enable_8 : 1;
    uint32_t porrr_chiplet_enable_7 : 1;
    uint32_t porrr_chiplet_enable_6 : 1;
    uint32_t porrr_chiplet_enable_5 : 1;
    uint32_t porrr_chiplet_enable_4 : 1;
    uint32_t porrr_chiplet_enable_3 : 1;
    uint32_t porrr_chiplet_enable_2 : 1;
    uint32_t porrr_chiplet_enable_1 : 1;
    uint32_t porrr_chiplet_enable_0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_pore_req_reg1_t;



typedef union pmc_pore_req_stat_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t porrs_reserved0 : 8;
    uint32_t porrs_start_vector : 4;
    uint32_t pore_rc : 8;
    uint32_t porrs_reserved1 : 1;
    uint32_t porrs_recovery_write : 1;
    uint32_t _reserved0 : 10;
#else
    uint32_t _reserved0 : 10;
    uint32_t porrs_recovery_write : 1;
    uint32_t porrs_reserved1 : 1;
    uint32_t pore_rc : 8;
    uint32_t porrs_start_vector : 4;
    uint32_t porrs_reserved0 : 8;
#endif // _BIG_ENDIAN
    } fields;
} pmc_pore_req_stat_reg_t;



typedef union pmc_pore_req_tout_th_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t porrtt_timeout_threshold : 8;
    uint32_t porrtc_no_predivide : 1;
    uint32_t _reserved0 : 23;
#else
    uint32_t _reserved0 : 23;
    uint32_t porrtc_no_predivide : 1;
    uint32_t porrtt_timeout_threshold : 8;
#endif // _BIG_ENDIAN
    } fields;
} pmc_pore_req_tout_th_reg_t;



typedef union pmc_deep_exit_mask_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t chiplet_deep_exit_mask0 : 1;
    uint32_t chiplet_deep_exit_mask1 : 1;
    uint32_t chiplet_deep_exit_mask2 : 1;
    uint32_t chiplet_deep_exit_mask3 : 1;
    uint32_t chiplet_deep_exit_mask4 : 1;
    uint32_t chiplet_deep_exit_mask5 : 1;
    uint32_t chiplet_deep_exit_mask6 : 1;
    uint32_t chiplet_deep_exit_mask7 : 1;
    uint32_t chiplet_deep_exit_mask8 : 1;
    uint32_t chiplet_deep_exit_mask9 : 1;
    uint32_t chiplet_deep_exit_mask10 : 1;
    uint32_t chiplet_deep_exit_mask11 : 1;
    uint32_t chiplet_deep_exit_mask12 : 1;
    uint32_t chiplet_deep_exit_mask13 : 1;
    uint32_t chiplet_deep_exit_mask14 : 1;
    uint32_t chiplet_deep_exit_mask15 : 1;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t chiplet_deep_exit_mask15 : 1;
    uint32_t chiplet_deep_exit_mask14 : 1;
    uint32_t chiplet_deep_exit_mask13 : 1;
    uint32_t chiplet_deep_exit_mask12 : 1;
    uint32_t chiplet_deep_exit_mask11 : 1;
    uint32_t chiplet_deep_exit_mask10 : 1;
    uint32_t chiplet_deep_exit_mask9 : 1;
    uint32_t chiplet_deep_exit_mask8 : 1;
    uint32_t chiplet_deep_exit_mask7 : 1;
    uint32_t chiplet_deep_exit_mask6 : 1;
    uint32_t chiplet_deep_exit_mask5 : 1;
    uint32_t chiplet_deep_exit_mask4 : 1;
    uint32_t chiplet_deep_exit_mask3 : 1;
    uint32_t chiplet_deep_exit_mask2 : 1;
    uint32_t chiplet_deep_exit_mask1 : 1;
    uint32_t chiplet_deep_exit_mask0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_deep_exit_mask_reg_t;



typedef union pmc_deep_exit_mask_reg_and {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t chiplet_deep_exit_mask0 : 1;
    uint32_t chiplet_deep_exit_mask1 : 1;
    uint32_t chiplet_deep_exit_mask2 : 1;
    uint32_t chiplet_deep_exit_mask3 : 1;
    uint32_t chiplet_deep_exit_mask4 : 1;
    uint32_t chiplet_deep_exit_mask5 : 1;
    uint32_t chiplet_deep_exit_mask6 : 1;
    uint32_t chiplet_deep_exit_mask7 : 1;
    uint32_t chiplet_deep_exit_mask8 : 1;
    uint32_t chiplet_deep_exit_mask9 : 1;
    uint32_t chiplet_deep_exit_mask10 : 1;
    uint32_t chiplet_deep_exit_mask11 : 1;
    uint32_t chiplet_deep_exit_mask12 : 1;
    uint32_t chiplet_deep_exit_mask13 : 1;
    uint32_t chiplet_deep_exit_mask14 : 1;
    uint32_t chiplet_deep_exit_mask15 : 1;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t chiplet_deep_exit_mask15 : 1;
    uint32_t chiplet_deep_exit_mask14 : 1;
    uint32_t chiplet_deep_exit_mask13 : 1;
    uint32_t chiplet_deep_exit_mask12 : 1;
    uint32_t chiplet_deep_exit_mask11 : 1;
    uint32_t chiplet_deep_exit_mask10 : 1;
    uint32_t chiplet_deep_exit_mask9 : 1;
    uint32_t chiplet_deep_exit_mask8 : 1;
    uint32_t chiplet_deep_exit_mask7 : 1;
    uint32_t chiplet_deep_exit_mask6 : 1;
    uint32_t chiplet_deep_exit_mask5 : 1;
    uint32_t chiplet_deep_exit_mask4 : 1;
    uint32_t chiplet_deep_exit_mask3 : 1;
    uint32_t chiplet_deep_exit_mask2 : 1;
    uint32_t chiplet_deep_exit_mask1 : 1;
    uint32_t chiplet_deep_exit_mask0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_deep_exit_mask_reg_and_t;



typedef union pmc_deep_exit_mask_reg_or {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t chiplet_deep_exit_mask0 : 1;
    uint32_t chiplet_deep_exit_mask1 : 1;
    uint32_t chiplet_deep_exit_mask2 : 1;
    uint32_t chiplet_deep_exit_mask3 : 1;
    uint32_t chiplet_deep_exit_mask4 : 1;
    uint32_t chiplet_deep_exit_mask5 : 1;
    uint32_t chiplet_deep_exit_mask6 : 1;
    uint32_t chiplet_deep_exit_mask7 : 1;
    uint32_t chiplet_deep_exit_mask8 : 1;
    uint32_t chiplet_deep_exit_mask9 : 1;
    uint32_t chiplet_deep_exit_mask10 : 1;
    uint32_t chiplet_deep_exit_mask11 : 1;
    uint32_t chiplet_deep_exit_mask12 : 1;
    uint32_t chiplet_deep_exit_mask13 : 1;
    uint32_t chiplet_deep_exit_mask14 : 1;
    uint32_t chiplet_deep_exit_mask15 : 1;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t chiplet_deep_exit_mask15 : 1;
    uint32_t chiplet_deep_exit_mask14 : 1;
    uint32_t chiplet_deep_exit_mask13 : 1;
    uint32_t chiplet_deep_exit_mask12 : 1;
    uint32_t chiplet_deep_exit_mask11 : 1;
    uint32_t chiplet_deep_exit_mask10 : 1;
    uint32_t chiplet_deep_exit_mask9 : 1;
    uint32_t chiplet_deep_exit_mask8 : 1;
    uint32_t chiplet_deep_exit_mask7 : 1;
    uint32_t chiplet_deep_exit_mask6 : 1;
    uint32_t chiplet_deep_exit_mask5 : 1;
    uint32_t chiplet_deep_exit_mask4 : 1;
    uint32_t chiplet_deep_exit_mask3 : 1;
    uint32_t chiplet_deep_exit_mask2 : 1;
    uint32_t chiplet_deep_exit_mask1 : 1;
    uint32_t chiplet_deep_exit_mask0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_deep_exit_mask_reg_or_t;



typedef union pmc_core_pstate_reg0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    int32_t pstate_core0 : 8;
    int32_t pstate_core1 : 8;
    int32_t pstate_core2 : 8;
    int32_t pstate_core3 : 8;
#else
    int32_t pstate_core3 : 8;
    int32_t pstate_core2 : 8;
    int32_t pstate_core1 : 8;
    int32_t pstate_core0 : 8;
#endif // _BIG_ENDIAN
    } fields;
} pmc_core_pstate_reg0_t;



typedef union pmc_core_pstate_reg1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    int32_t pstate_core4 : 8;
    int32_t pstate_core5 : 8;
    int32_t pstate_core6 : 8;
    int32_t pstate_core7 : 8;
#else
    int32_t pstate_core7 : 8;
    int32_t pstate_core6 : 8;
    int32_t pstate_core5 : 8;
    int32_t pstate_core4 : 8;
#endif // _BIG_ENDIAN
    } fields;
} pmc_core_pstate_reg1_t;



typedef union pmc_core_pstate_reg2 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    int32_t pstate_core8 : 8;
    int32_t pstate_core9 : 8;
    int32_t pstate_core10 : 8;
    int32_t pstate_core11 : 8;
#else
    int32_t pstate_core11 : 8;
    int32_t pstate_core10 : 8;
    int32_t pstate_core9 : 8;
    int32_t pstate_core8 : 8;
#endif // _BIG_ENDIAN
    } fields;
} pmc_core_pstate_reg2_t;



typedef union pmc_core_pstate_reg3 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    int32_t pstate_core12 : 8;
    int32_t pstate_core13 : 8;
    int32_t pstate_core14 : 8;
    int32_t pstate_core15 : 8;
#else
    int32_t pstate_core15 : 8;
    int32_t pstate_core14 : 8;
    int32_t pstate_core13 : 8;
    int32_t pstate_core12 : 8;
#endif // _BIG_ENDIAN
    } fields;
} pmc_core_pstate_reg3_t;



typedef union pmc_core_power_donation_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t power_donation_core0 : 1;
    uint32_t power_donation_core1 : 1;
    uint32_t power_donation_core2 : 1;
    uint32_t power_donation_core3 : 1;
    uint32_t power_donation_core4 : 1;
    uint32_t power_donation_core5 : 1;
    uint32_t power_donation_core6 : 1;
    uint32_t power_donation_core7 : 1;
    uint32_t power_donation_core8 : 1;
    uint32_t power_donation_core9 : 1;
    uint32_t power_donation_core10 : 1;
    uint32_t power_donation_core11 : 1;
    uint32_t power_donation_core12 : 1;
    uint32_t power_donation_core13 : 1;
    uint32_t power_donation_core14 : 1;
    uint32_t power_donation_core15 : 1;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t power_donation_core15 : 1;
    uint32_t power_donation_core14 : 1;
    uint32_t power_donation_core13 : 1;
    uint32_t power_donation_core12 : 1;
    uint32_t power_donation_core11 : 1;
    uint32_t power_donation_core10 : 1;
    uint32_t power_donation_core9 : 1;
    uint32_t power_donation_core8 : 1;
    uint32_t power_donation_core7 : 1;
    uint32_t power_donation_core6 : 1;
    uint32_t power_donation_core5 : 1;
    uint32_t power_donation_core4 : 1;
    uint32_t power_donation_core3 : 1;
    uint32_t power_donation_core2 : 1;
    uint32_t power_donation_core1 : 1;
    uint32_t power_donation_core0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_core_power_donation_reg_t;



typedef union pmc_pmax_sync_collection_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t pmax_sync0 : 1;
    uint32_t pmax_sync1 : 1;
    uint32_t pmax_sync2 : 1;
    uint32_t pmax_sync3 : 1;
    uint32_t pmax_sync4 : 1;
    uint32_t pmax_sync5 : 1;
    uint32_t pmax_sync6 : 1;
    uint32_t pmax_sync7 : 1;
    uint32_t pmax_sync8 : 1;
    uint32_t pmax_sync9 : 1;
    uint32_t pmax_sync10 : 1;
    uint32_t pmax_sync11 : 1;
    uint32_t pmax_sync12 : 1;
    uint32_t pmax_sync13 : 1;
    uint32_t pmax_sync14 : 1;
    uint32_t pmax_sync15 : 1;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t pmax_sync15 : 1;
    uint32_t pmax_sync14 : 1;
    uint32_t pmax_sync13 : 1;
    uint32_t pmax_sync12 : 1;
    uint32_t pmax_sync11 : 1;
    uint32_t pmax_sync10 : 1;
    uint32_t pmax_sync9 : 1;
    uint32_t pmax_sync8 : 1;
    uint32_t pmax_sync7 : 1;
    uint32_t pmax_sync6 : 1;
    uint32_t pmax_sync5 : 1;
    uint32_t pmax_sync4 : 1;
    uint32_t pmax_sync3 : 1;
    uint32_t pmax_sync2 : 1;
    uint32_t pmax_sync1 : 1;
    uint32_t pmax_sync0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_pmax_sync_collection_reg_t;



typedef union pmc_pmax_sync_collection_mask_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t pmax_sync_mask0 : 1;
    uint32_t pmax_sync_mask1 : 1;
    uint32_t pmax_sync_mask2 : 1;
    uint32_t pmax_sync_mask3 : 1;
    uint32_t pmax_sync_mask4 : 1;
    uint32_t pmax_sync_mask5 : 1;
    uint32_t pmax_sync_mask6 : 1;
    uint32_t pmax_sync_mask7 : 1;
    uint32_t pmax_sync_mask8 : 1;
    uint32_t pmax_sync_mask9 : 1;
    uint32_t pmax_sync_mask10 : 1;
    uint32_t pmax_sync_mask11 : 1;
    uint32_t pmax_sync_mask12 : 1;
    uint32_t pmax_sync_mask13 : 1;
    uint32_t pmax_sync_mask14 : 1;
    uint32_t pmax_sync_mask15 : 1;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t pmax_sync_mask15 : 1;
    uint32_t pmax_sync_mask14 : 1;
    uint32_t pmax_sync_mask13 : 1;
    uint32_t pmax_sync_mask12 : 1;
    uint32_t pmax_sync_mask11 : 1;
    uint32_t pmax_sync_mask10 : 1;
    uint32_t pmax_sync_mask9 : 1;
    uint32_t pmax_sync_mask8 : 1;
    uint32_t pmax_sync_mask7 : 1;
    uint32_t pmax_sync_mask6 : 1;
    uint32_t pmax_sync_mask5 : 1;
    uint32_t pmax_sync_mask4 : 1;
    uint32_t pmax_sync_mask3 : 1;
    uint32_t pmax_sync_mask2 : 1;
    uint32_t pmax_sync_mask1 : 1;
    uint32_t pmax_sync_mask0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_pmax_sync_collection_mask_reg_t;



typedef union pmc_gpsa_ack_collection_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t gpsa_ack0 : 1;
    uint32_t gpsa_ack1 : 1;
    uint32_t gpsa_ack2 : 1;
    uint32_t gpsa_ack3 : 1;
    uint32_t gpsa_ack4 : 1;
    uint32_t gpsa_ack5 : 1;
    uint32_t gpsa_ack6 : 1;
    uint32_t gpsa_ack7 : 1;
    uint32_t gpsa_ack8 : 1;
    uint32_t gpsa_ack9 : 1;
    uint32_t gpsa_ack10 : 1;
    uint32_t gpsa_ack11 : 1;
    uint32_t gpsa_ack12 : 1;
    uint32_t gpsa_ack13 : 1;
    uint32_t gpsa_ack14 : 1;
    uint32_t gpsa_ack15 : 1;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t gpsa_ack15 : 1;
    uint32_t gpsa_ack14 : 1;
    uint32_t gpsa_ack13 : 1;
    uint32_t gpsa_ack12 : 1;
    uint32_t gpsa_ack11 : 1;
    uint32_t gpsa_ack10 : 1;
    uint32_t gpsa_ack9 : 1;
    uint32_t gpsa_ack8 : 1;
    uint32_t gpsa_ack7 : 1;
    uint32_t gpsa_ack6 : 1;
    uint32_t gpsa_ack5 : 1;
    uint32_t gpsa_ack4 : 1;
    uint32_t gpsa_ack3 : 1;
    uint32_t gpsa_ack2 : 1;
    uint32_t gpsa_ack1 : 1;
    uint32_t gpsa_ack0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_gpsa_ack_collection_reg_t;



typedef union pmc_gpsa_ack_collection_mask_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t gpsa_ack_mask0 : 1;
    uint32_t gpsa_ack_mask1 : 1;
    uint32_t gpsa_ack_mask2 : 1;
    uint32_t gpsa_ack_mask3 : 1;
    uint32_t gpsa_ack_mask4 : 1;
    uint32_t gpsa_ack_mask5 : 1;
    uint32_t gpsa_ack_mask6 : 1;
    uint32_t gpsa_ack_mask7 : 1;
    uint32_t gpsa_ack_mask8 : 1;
    uint32_t gpsa_ack_mask9 : 1;
    uint32_t gpsa_ack_mask10 : 1;
    uint32_t gpsa_ack_mask11 : 1;
    uint32_t gpsa_ack_mask12 : 1;
    uint32_t gpsa_ack_mask13 : 1;
    uint32_t gpsa_ack_mask14 : 1;
    uint32_t gpsa_ack_mask15 : 1;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t gpsa_ack_mask15 : 1;
    uint32_t gpsa_ack_mask14 : 1;
    uint32_t gpsa_ack_mask13 : 1;
    uint32_t gpsa_ack_mask12 : 1;
    uint32_t gpsa_ack_mask11 : 1;
    uint32_t gpsa_ack_mask10 : 1;
    uint32_t gpsa_ack_mask9 : 1;
    uint32_t gpsa_ack_mask8 : 1;
    uint32_t gpsa_ack_mask7 : 1;
    uint32_t gpsa_ack_mask6 : 1;
    uint32_t gpsa_ack_mask5 : 1;
    uint32_t gpsa_ack_mask4 : 1;
    uint32_t gpsa_ack_mask3 : 1;
    uint32_t gpsa_ack_mask2 : 1;
    uint32_t gpsa_ack_mask1 : 1;
    uint32_t gpsa_ack_mask0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_gpsa_ack_collection_mask_reg_t;



typedef union pmc_pore_scratch_reg0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t porscr_scratch0 : 32;
#else
    uint32_t porscr_scratch0 : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_pore_scratch_reg0_t;



typedef union pmc_pore_scratch_reg1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t porscr_scratch1 : 32;
#else
    uint32_t porscr_scratch1 : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_pore_scratch_reg1_t;



typedef union pmc_deep_idle_exit_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t deep_exit_pending_and_masked_0 : 1;
    uint32_t deep_exit_pending_and_masked_1 : 1;
    uint32_t deep_exit_pending_and_masked_2 : 1;
    uint32_t deep_exit_pending_and_masked_3 : 1;
    uint32_t deep_exit_pending_and_masked_4 : 1;
    uint32_t deep_exit_pending_and_masked_5 : 1;
    uint32_t deep_exit_pending_and_masked_6 : 1;
    uint32_t deep_exit_pending_and_masked_7 : 1;
    uint32_t deep_exit_pending_and_masked_8 : 1;
    uint32_t deep_exit_pending_and_masked_9 : 1;
    uint32_t deep_exit_pending_and_masked_10 : 1;
    uint32_t deep_exit_pending_and_masked_11 : 1;
    uint32_t deep_exit_pending_and_masked_12 : 1;
    uint32_t deep_exit_pending_and_masked_13 : 1;
    uint32_t deep_exit_pending_and_masked_14 : 1;
    uint32_t deep_exit_pending_and_masked_15 : 1;
    uint32_t _reserved0 : 16;
#else
    uint32_t _reserved0 : 16;
    uint32_t deep_exit_pending_and_masked_15 : 1;
    uint32_t deep_exit_pending_and_masked_14 : 1;
    uint32_t deep_exit_pending_and_masked_13 : 1;
    uint32_t deep_exit_pending_and_masked_12 : 1;
    uint32_t deep_exit_pending_and_masked_11 : 1;
    uint32_t deep_exit_pending_and_masked_10 : 1;
    uint32_t deep_exit_pending_and_masked_9 : 1;
    uint32_t deep_exit_pending_and_masked_8 : 1;
    uint32_t deep_exit_pending_and_masked_7 : 1;
    uint32_t deep_exit_pending_and_masked_6 : 1;
    uint32_t deep_exit_pending_and_masked_5 : 1;
    uint32_t deep_exit_pending_and_masked_4 : 1;
    uint32_t deep_exit_pending_and_masked_3 : 1;
    uint32_t deep_exit_pending_and_masked_2 : 1;
    uint32_t deep_exit_pending_and_masked_1 : 1;
    uint32_t deep_exit_pending_and_masked_0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_deep_idle_exit_reg_t;



typedef union pmc_deep_status_reg {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t deep_idle_state_core0 : 1;
    uint32_t deep_idle_state_core1 : 1;
    uint32_t deep_idle_state_core2 : 1;
    uint32_t deep_idle_state_core3 : 1;
    uint32_t deep_idle_state_core4 : 1;
    uint32_t deep_idle_state_core5 : 1;
    uint32_t deep_idle_state_core6 : 1;
    uint32_t deep_idle_state_core7 : 1;
    uint32_t deep_idle_state_core8 : 1;
    uint32_t deep_idle_state_core9 : 1;
    uint32_t deep_idle_state_core10 : 1;
    uint32_t deep_idle_state_core11 : 1;
    uint32_t deep_idle_state_core12 : 1;
    uint32_t deep_idle_state_core13 : 1;
    uint32_t deep_idle_state_core14 : 1;
    uint32_t deep_idle_state_core15 : 1;
    uint32_t winkle_state_core0 : 1;
    uint32_t winkle_state_core1 : 1;
    uint32_t winkle_state_core2 : 1;
    uint32_t winkle_state_core3 : 1;
    uint32_t winkle_state_core4 : 1;
    uint32_t winkle_state_core5 : 1;
    uint32_t winkle_state_core6 : 1;
    uint32_t winkle_state_core7 : 1;
    uint32_t winkle_state_core8 : 1;
    uint32_t winkle_state_core9 : 1;
    uint32_t winkle_state_core10 : 1;
    uint32_t winkle_state_core11 : 1;
    uint32_t winkle_state_core12 : 1;
    uint32_t winkle_state_core13 : 1;
    uint32_t winkle_state_core14 : 1;
    uint32_t winkle_state_core15 : 1;
#else
    uint32_t winkle_state_core15 : 1;
    uint32_t winkle_state_core14 : 1;
    uint32_t winkle_state_core13 : 1;
    uint32_t winkle_state_core12 : 1;
    uint32_t winkle_state_core11 : 1;
    uint32_t winkle_state_core10 : 1;
    uint32_t winkle_state_core9 : 1;
    uint32_t winkle_state_core8 : 1;
    uint32_t winkle_state_core7 : 1;
    uint32_t winkle_state_core6 : 1;
    uint32_t winkle_state_core5 : 1;
    uint32_t winkle_state_core4 : 1;
    uint32_t winkle_state_core3 : 1;
    uint32_t winkle_state_core2 : 1;
    uint32_t winkle_state_core1 : 1;
    uint32_t winkle_state_core0 : 1;
    uint32_t deep_idle_state_core15 : 1;
    uint32_t deep_idle_state_core14 : 1;
    uint32_t deep_idle_state_core13 : 1;
    uint32_t deep_idle_state_core12 : 1;
    uint32_t deep_idle_state_core11 : 1;
    uint32_t deep_idle_state_core10 : 1;
    uint32_t deep_idle_state_core9 : 1;
    uint32_t deep_idle_state_core8 : 1;
    uint32_t deep_idle_state_core7 : 1;
    uint32_t deep_idle_state_core6 : 1;
    uint32_t deep_idle_state_core5 : 1;
    uint32_t deep_idle_state_core4 : 1;
    uint32_t deep_idle_state_core3 : 1;
    uint32_t deep_idle_state_core2 : 1;
    uint32_t deep_idle_state_core1 : 1;
    uint32_t deep_idle_state_core0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_deep_status_reg_t;



typedef union pmc_ba_pore_exe_trigger_reg {

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
    uint64_t value : 32;
    uint64_t _reserved0 : 32;
#else
    uint64_t _reserved0 : 32;
    uint64_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_ba_pore_exe_trigger_reg_t;



typedef union pmc_pcbs_gaps_brdcast_addr {

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
    uint64_t value : 32;
    uint64_t _reserved0 : 32;
#else
    uint64_t _reserved0 : 32;
    uint64_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} pmc_pcbs_gaps_brdcast_addr_t;



typedef union pmc_lfir_err_reg {

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
    uint64_t lfir_pstate_oci_master_rderr : 1;
    uint64_t lfir_pstate_oci_master_rddata_parity_err : 1;
    uint64_t lfir_pstate_gpst_checkbyte_err : 1;
    uint64_t lfir_pstate_gack_to_err : 1;
    uint64_t lfir_pstate_pib_master_nonoffline_err : 1;
    uint64_t lfir_pstate_pib_master_offline_err : 1;
    uint64_t lfir_pstate_oci_master_to_err : 1;
    uint64_t lfir_pstate_interchip_ue_err : 1;
    uint64_t lfir_pstate_interchip_errorframe_err : 1;
    uint64_t lfir_pstate_ms_fsm_err : 1;
    uint64_t lfir_ms_comp_parity_err : 1;
    uint64_t lfir_idle_poresw_fatal_err : 1;
    uint64_t lfir_idle_poresw_status_rc_err : 1;
    uint64_t lfir_idle_poresw_status_value_err : 1;
    uint64_t lfir_idle_poresw_write_while_inactive_err : 1;
    uint64_t lfir_idle_poresw_timeout_err : 1;
    uint64_t lfir_idle_oci_master_write_timeout_err : 1;
    uint64_t lfir_idle_internal_err : 1;
    uint64_t lfir_int_comp_parity_err : 1;
    uint64_t lfir_pmc_occ_heartbeat_timeout : 1;
    uint64_t lfir_spivid_crc_error0 : 1;
    uint64_t lfir_spivid_crc_error1 : 1;
    uint64_t lfir_spivid_crc_error2 : 1;
    uint64_t lfir_spivid_retry_timeout : 1;
    uint64_t lfir_spivid_fsm_err : 1;
    uint64_t lfir_spivid_majority_detected_a_minority : 1;
    uint64_t lfir_o2s_crc_error0 : 1;
    uint64_t lfir_o2s_crc_error1 : 1;
    uint64_t lfir_o2s_crc_error2 : 1;
    uint64_t lfir_o2s_retry_timeout : 1;
    uint64_t lfir_o2s_write_while_bridge_busy_err : 1;
    uint64_t lfir_o2s_fsm_err : 1;
    uint64_t lfir_o2s_majority_detected_a_minority : 1;
    uint64_t lfir_o2p_write_while_bridge_busy_err : 1;
    uint64_t lfir_o2p_fsm_err : 1;
    uint64_t lfir_oci_slave_err : 1;
    uint64_t lfir_if_comp_parity_error : 1;
    uint64_t spare_fir : 10;
    uint64_t fir_parity_err_dup : 1;
    uint64_t fir_parity_err : 1;
    uint64_t _reserved0 : 15;
#else
    uint64_t _reserved0 : 15;
    uint64_t fir_parity_err : 1;
    uint64_t fir_parity_err_dup : 1;
    uint64_t spare_fir : 10;
    uint64_t lfir_if_comp_parity_error : 1;
    uint64_t lfir_oci_slave_err : 1;
    uint64_t lfir_o2p_fsm_err : 1;
    uint64_t lfir_o2p_write_while_bridge_busy_err : 1;
    uint64_t lfir_o2s_majority_detected_a_minority : 1;
    uint64_t lfir_o2s_fsm_err : 1;
    uint64_t lfir_o2s_write_while_bridge_busy_err : 1;
    uint64_t lfir_o2s_retry_timeout : 1;
    uint64_t lfir_o2s_crc_error2 : 1;
    uint64_t lfir_o2s_crc_error1 : 1;
    uint64_t lfir_o2s_crc_error0 : 1;
    uint64_t lfir_spivid_majority_detected_a_minority : 1;
    uint64_t lfir_spivid_fsm_err : 1;
    uint64_t lfir_spivid_retry_timeout : 1;
    uint64_t lfir_spivid_crc_error2 : 1;
    uint64_t lfir_spivid_crc_error1 : 1;
    uint64_t lfir_spivid_crc_error0 : 1;
    uint64_t lfir_pmc_occ_heartbeat_timeout : 1;
    uint64_t lfir_int_comp_parity_err : 1;
    uint64_t lfir_idle_internal_err : 1;
    uint64_t lfir_idle_oci_master_write_timeout_err : 1;
    uint64_t lfir_idle_poresw_timeout_err : 1;
    uint64_t lfir_idle_poresw_write_while_inactive_err : 1;
    uint64_t lfir_idle_poresw_status_value_err : 1;
    uint64_t lfir_idle_poresw_status_rc_err : 1;
    uint64_t lfir_idle_poresw_fatal_err : 1;
    uint64_t lfir_ms_comp_parity_err : 1;
    uint64_t lfir_pstate_ms_fsm_err : 1;
    uint64_t lfir_pstate_interchip_errorframe_err : 1;
    uint64_t lfir_pstate_interchip_ue_err : 1;
    uint64_t lfir_pstate_oci_master_to_err : 1;
    uint64_t lfir_pstate_pib_master_offline_err : 1;
    uint64_t lfir_pstate_pib_master_nonoffline_err : 1;
    uint64_t lfir_pstate_gack_to_err : 1;
    uint64_t lfir_pstate_gpst_checkbyte_err : 1;
    uint64_t lfir_pstate_oci_master_rddata_parity_err : 1;
    uint64_t lfir_pstate_oci_master_rderr : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_lfir_err_reg_t;



typedef union pmc_lfir_err_reg_and {

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
    uint64_t lfir_pstate_oci_master_rderr : 1;
    uint64_t lfir_pstate_oci_master_rddata_parity_err : 1;
    uint64_t lfir_pstate_gpst_checkbyte_err : 1;
    uint64_t lfir_pstate_gack_to_err : 1;
    uint64_t lfir_pstate_pib_master_nonoffline_err : 1;
    uint64_t lfir_pstate_pib_master_offline_err : 1;
    uint64_t lfir_pstate_oci_master_to_err : 1;
    uint64_t lfir_pstate_interchip_ue_err : 1;
    uint64_t lfir_pstate_interchip_errorframe_err : 1;
    uint64_t lfir_pstate_ms_fsm_err : 1;
    uint64_t lfir_ms_comp_parity_err : 1;
    uint64_t lfir_idle_poresw_fatal_err : 1;
    uint64_t lfir_idle_poresw_status_rc_err : 1;
    uint64_t lfir_idle_poresw_status_value_err : 1;
    uint64_t lfir_idle_poresw_write_while_inactive_err : 1;
    uint64_t lfir_idle_poresw_timeout_err : 1;
    uint64_t lfir_idle_oci_master_write_timeout_err : 1;
    uint64_t lfir_idle_internal_err : 1;
    uint64_t lfir_int_comp_parity_err : 1;
    uint64_t lfir_pmc_occ_heartbeat_timeout : 1;
    uint64_t lfir_spivid_crc_error0 : 1;
    uint64_t lfir_spivid_crc_error1 : 1;
    uint64_t lfir_spivid_crc_error2 : 1;
    uint64_t lfir_spivid_retry_timeout : 1;
    uint64_t lfir_spivid_fsm_err : 1;
    uint64_t lfir_spivid_majority_detected_a_minority : 1;
    uint64_t lfir_o2s_crc_error0 : 1;
    uint64_t lfir_o2s_crc_error1 : 1;
    uint64_t lfir_o2s_crc_error2 : 1;
    uint64_t lfir_o2s_retry_timeout : 1;
    uint64_t lfir_o2s_write_while_bridge_busy_err : 1;
    uint64_t lfir_o2s_fsm_err : 1;
    uint64_t lfir_o2s_majority_detected_a_minority : 1;
    uint64_t lfir_o2p_write_while_bridge_busy_err : 1;
    uint64_t lfir_o2p_fsm_err : 1;
    uint64_t lfir_oci_slave_err : 1;
    uint64_t lfir_if_comp_parity_error : 1;
    uint64_t spare_fir : 10;
    uint64_t fir_parity_err_dup : 1;
    uint64_t fir_parity_err : 1;
    uint64_t _reserved0 : 15;
#else
    uint64_t _reserved0 : 15;
    uint64_t fir_parity_err : 1;
    uint64_t fir_parity_err_dup : 1;
    uint64_t spare_fir : 10;
    uint64_t lfir_if_comp_parity_error : 1;
    uint64_t lfir_oci_slave_err : 1;
    uint64_t lfir_o2p_fsm_err : 1;
    uint64_t lfir_o2p_write_while_bridge_busy_err : 1;
    uint64_t lfir_o2s_majority_detected_a_minority : 1;
    uint64_t lfir_o2s_fsm_err : 1;
    uint64_t lfir_o2s_write_while_bridge_busy_err : 1;
    uint64_t lfir_o2s_retry_timeout : 1;
    uint64_t lfir_o2s_crc_error2 : 1;
    uint64_t lfir_o2s_crc_error1 : 1;
    uint64_t lfir_o2s_crc_error0 : 1;
    uint64_t lfir_spivid_majority_detected_a_minority : 1;
    uint64_t lfir_spivid_fsm_err : 1;
    uint64_t lfir_spivid_retry_timeout : 1;
    uint64_t lfir_spivid_crc_error2 : 1;
    uint64_t lfir_spivid_crc_error1 : 1;
    uint64_t lfir_spivid_crc_error0 : 1;
    uint64_t lfir_pmc_occ_heartbeat_timeout : 1;
    uint64_t lfir_int_comp_parity_err : 1;
    uint64_t lfir_idle_internal_err : 1;
    uint64_t lfir_idle_oci_master_write_timeout_err : 1;
    uint64_t lfir_idle_poresw_timeout_err : 1;
    uint64_t lfir_idle_poresw_write_while_inactive_err : 1;
    uint64_t lfir_idle_poresw_status_value_err : 1;
    uint64_t lfir_idle_poresw_status_rc_err : 1;
    uint64_t lfir_idle_poresw_fatal_err : 1;
    uint64_t lfir_ms_comp_parity_err : 1;
    uint64_t lfir_pstate_ms_fsm_err : 1;
    uint64_t lfir_pstate_interchip_errorframe_err : 1;
    uint64_t lfir_pstate_interchip_ue_err : 1;
    uint64_t lfir_pstate_oci_master_to_err : 1;
    uint64_t lfir_pstate_pib_master_offline_err : 1;
    uint64_t lfir_pstate_pib_master_nonoffline_err : 1;
    uint64_t lfir_pstate_gack_to_err : 1;
    uint64_t lfir_pstate_gpst_checkbyte_err : 1;
    uint64_t lfir_pstate_oci_master_rddata_parity_err : 1;
    uint64_t lfir_pstate_oci_master_rderr : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_lfir_err_reg_and_t;



typedef union pmc_lfir_err_reg_or {

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
    uint64_t lfir_pstate_oci_master_rderr : 1;
    uint64_t lfir_pstate_oci_master_rddata_parity_err : 1;
    uint64_t lfir_pstate_gpst_checkbyte_err : 1;
    uint64_t lfir_pstate_gack_to_err : 1;
    uint64_t lfir_pstate_pib_master_nonoffline_err : 1;
    uint64_t lfir_pstate_pib_master_offline_err : 1;
    uint64_t lfir_pstate_oci_master_to_err : 1;
    uint64_t lfir_pstate_interchip_ue_err : 1;
    uint64_t lfir_pstate_interchip_errorframe_err : 1;
    uint64_t lfir_pstate_ms_fsm_err : 1;
    uint64_t lfir_ms_comp_parity_err : 1;
    uint64_t lfir_idle_poresw_fatal_err : 1;
    uint64_t lfir_idle_poresw_status_rc_err : 1;
    uint64_t lfir_idle_poresw_status_value_err : 1;
    uint64_t lfir_idle_poresw_write_while_inactive_err : 1;
    uint64_t lfir_idle_poresw_timeout_err : 1;
    uint64_t lfir_idle_oci_master_write_timeout_err : 1;
    uint64_t lfir_idle_internal_err : 1;
    uint64_t lfir_int_comp_parity_err : 1;
    uint64_t lfir_pmc_occ_heartbeat_timeout : 1;
    uint64_t lfir_spivid_crc_error0 : 1;
    uint64_t lfir_spivid_crc_error1 : 1;
    uint64_t lfir_spivid_crc_error2 : 1;
    uint64_t lfir_spivid_retry_timeout : 1;
    uint64_t lfir_spivid_fsm_err : 1;
    uint64_t lfir_spivid_majority_detected_a_minority : 1;
    uint64_t lfir_o2s_crc_error0 : 1;
    uint64_t lfir_o2s_crc_error1 : 1;
    uint64_t lfir_o2s_crc_error2 : 1;
    uint64_t lfir_o2s_retry_timeout : 1;
    uint64_t lfir_o2s_write_while_bridge_busy_err : 1;
    uint64_t lfir_o2s_fsm_err : 1;
    uint64_t lfir_o2s_majority_detected_a_minority : 1;
    uint64_t lfir_o2p_write_while_bridge_busy_err : 1;
    uint64_t lfir_o2p_fsm_err : 1;
    uint64_t lfir_oci_slave_err : 1;
    uint64_t lfir_if_comp_parity_error : 1;
    uint64_t spare_fir : 10;
    uint64_t fir_parity_err_dup : 1;
    uint64_t fir_parity_err : 1;
    uint64_t _reserved0 : 15;
#else
    uint64_t _reserved0 : 15;
    uint64_t fir_parity_err : 1;
    uint64_t fir_parity_err_dup : 1;
    uint64_t spare_fir : 10;
    uint64_t lfir_if_comp_parity_error : 1;
    uint64_t lfir_oci_slave_err : 1;
    uint64_t lfir_o2p_fsm_err : 1;
    uint64_t lfir_o2p_write_while_bridge_busy_err : 1;
    uint64_t lfir_o2s_majority_detected_a_minority : 1;
    uint64_t lfir_o2s_fsm_err : 1;
    uint64_t lfir_o2s_write_while_bridge_busy_err : 1;
    uint64_t lfir_o2s_retry_timeout : 1;
    uint64_t lfir_o2s_crc_error2 : 1;
    uint64_t lfir_o2s_crc_error1 : 1;
    uint64_t lfir_o2s_crc_error0 : 1;
    uint64_t lfir_spivid_majority_detected_a_minority : 1;
    uint64_t lfir_spivid_fsm_err : 1;
    uint64_t lfir_spivid_retry_timeout : 1;
    uint64_t lfir_spivid_crc_error2 : 1;
    uint64_t lfir_spivid_crc_error1 : 1;
    uint64_t lfir_spivid_crc_error0 : 1;
    uint64_t lfir_pmc_occ_heartbeat_timeout : 1;
    uint64_t lfir_int_comp_parity_err : 1;
    uint64_t lfir_idle_internal_err : 1;
    uint64_t lfir_idle_oci_master_write_timeout_err : 1;
    uint64_t lfir_idle_poresw_timeout_err : 1;
    uint64_t lfir_idle_poresw_write_while_inactive_err : 1;
    uint64_t lfir_idle_poresw_status_value_err : 1;
    uint64_t lfir_idle_poresw_status_rc_err : 1;
    uint64_t lfir_idle_poresw_fatal_err : 1;
    uint64_t lfir_ms_comp_parity_err : 1;
    uint64_t lfir_pstate_ms_fsm_err : 1;
    uint64_t lfir_pstate_interchip_errorframe_err : 1;
    uint64_t lfir_pstate_interchip_ue_err : 1;
    uint64_t lfir_pstate_oci_master_to_err : 1;
    uint64_t lfir_pstate_pib_master_offline_err : 1;
    uint64_t lfir_pstate_pib_master_nonoffline_err : 1;
    uint64_t lfir_pstate_gack_to_err : 1;
    uint64_t lfir_pstate_gpst_checkbyte_err : 1;
    uint64_t lfir_pstate_oci_master_rddata_parity_err : 1;
    uint64_t lfir_pstate_oci_master_rderr : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_lfir_err_reg_or_t;



typedef union pmc_lfir_err_mask_reg {

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
    uint64_t pmc_lfir_mask_0 : 1;
    uint64_t pmc_lfir_mask_1 : 1;
    uint64_t pmc_lfir_mask_2 : 1;
    uint64_t pmc_lfir_mask_3 : 1;
    uint64_t pmc_lfir_mask_4 : 1;
    uint64_t pmc_lfir_mask_5 : 1;
    uint64_t pmc_lfir_mask_6 : 1;
    uint64_t pmc_lfir_mask_7 : 1;
    uint64_t pmc_lfir_mask_8 : 1;
    uint64_t pmc_lfir_mask_9 : 1;
    uint64_t pmc_lfir_mask_10 : 1;
    uint64_t pmc_lfir_mask_11 : 1;
    uint64_t pmc_lfir_mask_12 : 1;
    uint64_t pmc_lfir_mask_13 : 1;
    uint64_t pmc_lfir_mask_14 : 1;
    uint64_t pmc_lfir_mask_15 : 1;
    uint64_t pmc_lfir_mask_16 : 1;
    uint64_t pmc_lfir_mask_17 : 1;
    uint64_t pmc_lfir_mask_18 : 1;
    uint64_t pmc_lfir_mask_19 : 1;
    uint64_t pmc_lfir_mask_20 : 1;
    uint64_t pmc_lfir_mask_21 : 1;
    uint64_t pmc_lfir_mask_22 : 1;
    uint64_t pmc_lfir_mask_23 : 1;
    uint64_t pmc_lfir_mask_24 : 1;
    uint64_t pmc_lfir_mask_25 : 1;
    uint64_t pmc_lfir_mask_26 : 1;
    uint64_t pmc_lfir_mask_27 : 1;
    uint64_t pmc_lfir_mask_28 : 1;
    uint64_t pmc_lfir_mask_29 : 1;
    uint64_t pmc_lfir_mask_30 : 1;
    uint64_t pmc_lfir_mask_31 : 1;
    uint64_t pmc_lfir_mask_32 : 1;
    uint64_t pmc_lfir_mask_33 : 1;
    uint64_t pmc_lfir_mask_34 : 1;
    uint64_t pmc_lfir_mask_35 : 1;
    uint64_t pmc_lfir_mask_36 : 1;
    uint64_t pmc_lfir_mask1_37_46 : 10;
    uint64_t pmc_lfir_mask1_47 : 1;
    uint64_t pmc_lfir_mask1_48 : 1;
    uint64_t _reserved0 : 15;
#else
    uint64_t _reserved0 : 15;
    uint64_t pmc_lfir_mask1_48 : 1;
    uint64_t pmc_lfir_mask1_47 : 1;
    uint64_t pmc_lfir_mask1_37_46 : 10;
    uint64_t pmc_lfir_mask_36 : 1;
    uint64_t pmc_lfir_mask_35 : 1;
    uint64_t pmc_lfir_mask_34 : 1;
    uint64_t pmc_lfir_mask_33 : 1;
    uint64_t pmc_lfir_mask_32 : 1;
    uint64_t pmc_lfir_mask_31 : 1;
    uint64_t pmc_lfir_mask_30 : 1;
    uint64_t pmc_lfir_mask_29 : 1;
    uint64_t pmc_lfir_mask_28 : 1;
    uint64_t pmc_lfir_mask_27 : 1;
    uint64_t pmc_lfir_mask_26 : 1;
    uint64_t pmc_lfir_mask_25 : 1;
    uint64_t pmc_lfir_mask_24 : 1;
    uint64_t pmc_lfir_mask_23 : 1;
    uint64_t pmc_lfir_mask_22 : 1;
    uint64_t pmc_lfir_mask_21 : 1;
    uint64_t pmc_lfir_mask_20 : 1;
    uint64_t pmc_lfir_mask_19 : 1;
    uint64_t pmc_lfir_mask_18 : 1;
    uint64_t pmc_lfir_mask_17 : 1;
    uint64_t pmc_lfir_mask_16 : 1;
    uint64_t pmc_lfir_mask_15 : 1;
    uint64_t pmc_lfir_mask_14 : 1;
    uint64_t pmc_lfir_mask_13 : 1;
    uint64_t pmc_lfir_mask_12 : 1;
    uint64_t pmc_lfir_mask_11 : 1;
    uint64_t pmc_lfir_mask_10 : 1;
    uint64_t pmc_lfir_mask_9 : 1;
    uint64_t pmc_lfir_mask_8 : 1;
    uint64_t pmc_lfir_mask_7 : 1;
    uint64_t pmc_lfir_mask_6 : 1;
    uint64_t pmc_lfir_mask_5 : 1;
    uint64_t pmc_lfir_mask_4 : 1;
    uint64_t pmc_lfir_mask_3 : 1;
    uint64_t pmc_lfir_mask_2 : 1;
    uint64_t pmc_lfir_mask_1 : 1;
    uint64_t pmc_lfir_mask_0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_lfir_err_mask_reg_t;



typedef union pmc_lfir_err_mask_reg_and {

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
    uint64_t pmc_lfir_mask_0 : 1;
    uint64_t pmc_lfir_mask_1 : 1;
    uint64_t pmc_lfir_mask_2 : 1;
    uint64_t pmc_lfir_mask_3 : 1;
    uint64_t pmc_lfir_mask_4 : 1;
    uint64_t pmc_lfir_mask_5 : 1;
    uint64_t pmc_lfir_mask_6 : 1;
    uint64_t pmc_lfir_mask_7 : 1;
    uint64_t pmc_lfir_mask_8 : 1;
    uint64_t pmc_lfir_mask_9 : 1;
    uint64_t pmc_lfir_mask_10 : 1;
    uint64_t pmc_lfir_mask_11 : 1;
    uint64_t pmc_lfir_mask_12 : 1;
    uint64_t pmc_lfir_mask_13 : 1;
    uint64_t pmc_lfir_mask_14 : 1;
    uint64_t pmc_lfir_mask_15 : 1;
    uint64_t pmc_lfir_mask_16 : 1;
    uint64_t pmc_lfir_mask_17 : 1;
    uint64_t pmc_lfir_mask_18 : 1;
    uint64_t pmc_lfir_mask_19 : 1;
    uint64_t pmc_lfir_mask_20 : 1;
    uint64_t pmc_lfir_mask_21 : 1;
    uint64_t pmc_lfir_mask_22 : 1;
    uint64_t pmc_lfir_mask_23 : 1;
    uint64_t pmc_lfir_mask_24 : 1;
    uint64_t pmc_lfir_mask_25 : 1;
    uint64_t pmc_lfir_mask_26 : 1;
    uint64_t pmc_lfir_mask_27 : 1;
    uint64_t pmc_lfir_mask_28 : 1;
    uint64_t pmc_lfir_mask_29 : 1;
    uint64_t pmc_lfir_mask_30 : 1;
    uint64_t pmc_lfir_mask_31 : 1;
    uint64_t pmc_lfir_mask_32 : 1;
    uint64_t pmc_lfir_mask_33 : 1;
    uint64_t pmc_lfir_mask_34 : 1;
    uint64_t pmc_lfir_mask_35 : 1;
    uint64_t pmc_lfir_mask_36 : 1;
    uint64_t pmc_lfir_mask1_37_46 : 10;
    uint64_t pmc_lfir_mask1_47 : 1;
    uint64_t pmc_lfir_mask1_48 : 1;
    uint64_t _reserved0 : 15;
#else
    uint64_t _reserved0 : 15;
    uint64_t pmc_lfir_mask1_48 : 1;
    uint64_t pmc_lfir_mask1_47 : 1;
    uint64_t pmc_lfir_mask1_37_46 : 10;
    uint64_t pmc_lfir_mask_36 : 1;
    uint64_t pmc_lfir_mask_35 : 1;
    uint64_t pmc_lfir_mask_34 : 1;
    uint64_t pmc_lfir_mask_33 : 1;
    uint64_t pmc_lfir_mask_32 : 1;
    uint64_t pmc_lfir_mask_31 : 1;
    uint64_t pmc_lfir_mask_30 : 1;
    uint64_t pmc_lfir_mask_29 : 1;
    uint64_t pmc_lfir_mask_28 : 1;
    uint64_t pmc_lfir_mask_27 : 1;
    uint64_t pmc_lfir_mask_26 : 1;
    uint64_t pmc_lfir_mask_25 : 1;
    uint64_t pmc_lfir_mask_24 : 1;
    uint64_t pmc_lfir_mask_23 : 1;
    uint64_t pmc_lfir_mask_22 : 1;
    uint64_t pmc_lfir_mask_21 : 1;
    uint64_t pmc_lfir_mask_20 : 1;
    uint64_t pmc_lfir_mask_19 : 1;
    uint64_t pmc_lfir_mask_18 : 1;
    uint64_t pmc_lfir_mask_17 : 1;
    uint64_t pmc_lfir_mask_16 : 1;
    uint64_t pmc_lfir_mask_15 : 1;
    uint64_t pmc_lfir_mask_14 : 1;
    uint64_t pmc_lfir_mask_13 : 1;
    uint64_t pmc_lfir_mask_12 : 1;
    uint64_t pmc_lfir_mask_11 : 1;
    uint64_t pmc_lfir_mask_10 : 1;
    uint64_t pmc_lfir_mask_9 : 1;
    uint64_t pmc_lfir_mask_8 : 1;
    uint64_t pmc_lfir_mask_7 : 1;
    uint64_t pmc_lfir_mask_6 : 1;
    uint64_t pmc_lfir_mask_5 : 1;
    uint64_t pmc_lfir_mask_4 : 1;
    uint64_t pmc_lfir_mask_3 : 1;
    uint64_t pmc_lfir_mask_2 : 1;
    uint64_t pmc_lfir_mask_1 : 1;
    uint64_t pmc_lfir_mask_0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_lfir_err_mask_reg_and_t;



typedef union pmc_lfir_err_mask_reg_or {

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
    uint64_t pmc_lfir_mask_0 : 1;
    uint64_t pmc_lfir_mask_1 : 1;
    uint64_t pmc_lfir_mask_2 : 1;
    uint64_t pmc_lfir_mask_3 : 1;
    uint64_t pmc_lfir_mask_4 : 1;
    uint64_t pmc_lfir_mask_5 : 1;
    uint64_t pmc_lfir_mask_6 : 1;
    uint64_t pmc_lfir_mask_7 : 1;
    uint64_t pmc_lfir_mask_8 : 1;
    uint64_t pmc_lfir_mask_9 : 1;
    uint64_t pmc_lfir_mask_10 : 1;
    uint64_t pmc_lfir_mask_11 : 1;
    uint64_t pmc_lfir_mask_12 : 1;
    uint64_t pmc_lfir_mask_13 : 1;
    uint64_t pmc_lfir_mask_14 : 1;
    uint64_t pmc_lfir_mask_15 : 1;
    uint64_t pmc_lfir_mask_16 : 1;
    uint64_t pmc_lfir_mask_17 : 1;
    uint64_t pmc_lfir_mask_18 : 1;
    uint64_t pmc_lfir_mask_19 : 1;
    uint64_t pmc_lfir_mask_20 : 1;
    uint64_t pmc_lfir_mask_21 : 1;
    uint64_t pmc_lfir_mask_22 : 1;
    uint64_t pmc_lfir_mask_23 : 1;
    uint64_t pmc_lfir_mask_24 : 1;
    uint64_t pmc_lfir_mask_25 : 1;
    uint64_t pmc_lfir_mask_26 : 1;
    uint64_t pmc_lfir_mask_27 : 1;
    uint64_t pmc_lfir_mask_28 : 1;
    uint64_t pmc_lfir_mask_29 : 1;
    uint64_t pmc_lfir_mask_30 : 1;
    uint64_t pmc_lfir_mask_31 : 1;
    uint64_t pmc_lfir_mask_32 : 1;
    uint64_t pmc_lfir_mask_33 : 1;
    uint64_t pmc_lfir_mask_34 : 1;
    uint64_t pmc_lfir_mask_35 : 1;
    uint64_t pmc_lfir_mask_36 : 1;
    uint64_t pmc_lfir_mask1_37_46 : 10;
    uint64_t pmc_lfir_mask1_47 : 1;
    uint64_t pmc_lfir_mask1_48 : 1;
    uint64_t _reserved0 : 15;
#else
    uint64_t _reserved0 : 15;
    uint64_t pmc_lfir_mask1_48 : 1;
    uint64_t pmc_lfir_mask1_47 : 1;
    uint64_t pmc_lfir_mask1_37_46 : 10;
    uint64_t pmc_lfir_mask_36 : 1;
    uint64_t pmc_lfir_mask_35 : 1;
    uint64_t pmc_lfir_mask_34 : 1;
    uint64_t pmc_lfir_mask_33 : 1;
    uint64_t pmc_lfir_mask_32 : 1;
    uint64_t pmc_lfir_mask_31 : 1;
    uint64_t pmc_lfir_mask_30 : 1;
    uint64_t pmc_lfir_mask_29 : 1;
    uint64_t pmc_lfir_mask_28 : 1;
    uint64_t pmc_lfir_mask_27 : 1;
    uint64_t pmc_lfir_mask_26 : 1;
    uint64_t pmc_lfir_mask_25 : 1;
    uint64_t pmc_lfir_mask_24 : 1;
    uint64_t pmc_lfir_mask_23 : 1;
    uint64_t pmc_lfir_mask_22 : 1;
    uint64_t pmc_lfir_mask_21 : 1;
    uint64_t pmc_lfir_mask_20 : 1;
    uint64_t pmc_lfir_mask_19 : 1;
    uint64_t pmc_lfir_mask_18 : 1;
    uint64_t pmc_lfir_mask_17 : 1;
    uint64_t pmc_lfir_mask_16 : 1;
    uint64_t pmc_lfir_mask_15 : 1;
    uint64_t pmc_lfir_mask_14 : 1;
    uint64_t pmc_lfir_mask_13 : 1;
    uint64_t pmc_lfir_mask_12 : 1;
    uint64_t pmc_lfir_mask_11 : 1;
    uint64_t pmc_lfir_mask_10 : 1;
    uint64_t pmc_lfir_mask_9 : 1;
    uint64_t pmc_lfir_mask_8 : 1;
    uint64_t pmc_lfir_mask_7 : 1;
    uint64_t pmc_lfir_mask_6 : 1;
    uint64_t pmc_lfir_mask_5 : 1;
    uint64_t pmc_lfir_mask_4 : 1;
    uint64_t pmc_lfir_mask_3 : 1;
    uint64_t pmc_lfir_mask_2 : 1;
    uint64_t pmc_lfir_mask_1 : 1;
    uint64_t pmc_lfir_mask_0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_lfir_err_mask_reg_or_t;



typedef union pmc_lfir_action0_reg {

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
    uint64_t pmc_lfir_action0_0 : 1;
    uint64_t pmc_lfir_action0_1 : 1;
    uint64_t pmc_lfir_action0_2 : 1;
    uint64_t pmc_lfir_action0_3 : 1;
    uint64_t pmc_lfir_action0_4 : 1;
    uint64_t pmc_lfir_action0_5 : 1;
    uint64_t pmc_lfir_action0_6 : 1;
    uint64_t pmc_lfir_action0_7 : 1;
    uint64_t pmc_lfir_action0_8 : 1;
    uint64_t pmc_lfir_action0_9 : 1;
    uint64_t pmc_lfir_action0_10 : 1;
    uint64_t pmc_lfir_action0_11 : 1;
    uint64_t pmc_lfir_action0_12 : 1;
    uint64_t pmc_lfir_action0_13 : 1;
    uint64_t pmc_lfir_action0_14 : 1;
    uint64_t pmc_lfir_action0_15 : 1;
    uint64_t pmc_lfir_action0_16 : 1;
    uint64_t pmc_lfir_action0_17 : 1;
    uint64_t pmc_lfir_action0_18 : 1;
    uint64_t pmc_lfir_action0_19 : 1;
    uint64_t pmc_lfir_action0_20 : 1;
    uint64_t pmc_lfir_action0_21 : 1;
    uint64_t pmc_lfir_action0_22 : 1;
    uint64_t pmc_lfir_action0_23 : 1;
    uint64_t pmc_lfir_action0_24 : 1;
    uint64_t pmc_lfir_action0_25 : 1;
    uint64_t pmc_lfir_action0_26 : 1;
    uint64_t pmc_lfir_action0_27 : 1;
    uint64_t pmc_lfir_action0_28 : 1;
    uint64_t pmc_lfir_action0_29 : 1;
    uint64_t pmc_lfir_action0_30 : 1;
    uint64_t pmc_lfir_action0_31 : 1;
    uint64_t pmc_lfir_action0_32 : 1;
    uint64_t pmc_lfir_action0_33 : 1;
    uint64_t pmc_lfir_action0_34 : 1;
    uint64_t pmc_lfir_action0_35 : 1;
    uint64_t pmc_lfir_action0_36 : 1;
    uint64_t pmc_lfir_action0_37_46 : 10;
    uint64_t pmc_lfir_action0_47 : 1;
    uint64_t pmc_lfir_action0_48 : 1;
    uint64_t _reserved0 : 15;
#else
    uint64_t _reserved0 : 15;
    uint64_t pmc_lfir_action0_48 : 1;
    uint64_t pmc_lfir_action0_47 : 1;
    uint64_t pmc_lfir_action0_37_46 : 10;
    uint64_t pmc_lfir_action0_36 : 1;
    uint64_t pmc_lfir_action0_35 : 1;
    uint64_t pmc_lfir_action0_34 : 1;
    uint64_t pmc_lfir_action0_33 : 1;
    uint64_t pmc_lfir_action0_32 : 1;
    uint64_t pmc_lfir_action0_31 : 1;
    uint64_t pmc_lfir_action0_30 : 1;
    uint64_t pmc_lfir_action0_29 : 1;
    uint64_t pmc_lfir_action0_28 : 1;
    uint64_t pmc_lfir_action0_27 : 1;
    uint64_t pmc_lfir_action0_26 : 1;
    uint64_t pmc_lfir_action0_25 : 1;
    uint64_t pmc_lfir_action0_24 : 1;
    uint64_t pmc_lfir_action0_23 : 1;
    uint64_t pmc_lfir_action0_22 : 1;
    uint64_t pmc_lfir_action0_21 : 1;
    uint64_t pmc_lfir_action0_20 : 1;
    uint64_t pmc_lfir_action0_19 : 1;
    uint64_t pmc_lfir_action0_18 : 1;
    uint64_t pmc_lfir_action0_17 : 1;
    uint64_t pmc_lfir_action0_16 : 1;
    uint64_t pmc_lfir_action0_15 : 1;
    uint64_t pmc_lfir_action0_14 : 1;
    uint64_t pmc_lfir_action0_13 : 1;
    uint64_t pmc_lfir_action0_12 : 1;
    uint64_t pmc_lfir_action0_11 : 1;
    uint64_t pmc_lfir_action0_10 : 1;
    uint64_t pmc_lfir_action0_9 : 1;
    uint64_t pmc_lfir_action0_8 : 1;
    uint64_t pmc_lfir_action0_7 : 1;
    uint64_t pmc_lfir_action0_6 : 1;
    uint64_t pmc_lfir_action0_5 : 1;
    uint64_t pmc_lfir_action0_4 : 1;
    uint64_t pmc_lfir_action0_3 : 1;
    uint64_t pmc_lfir_action0_2 : 1;
    uint64_t pmc_lfir_action0_1 : 1;
    uint64_t pmc_lfir_action0_0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_lfir_action0_reg_t;



typedef union pmc_lfir_action1_reg {

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
    uint64_t pmc_lfir_action1_0 : 1;
    uint64_t pmc_lfir_action1_1 : 1;
    uint64_t pmc_lfir_action1_2 : 1;
    uint64_t pmc_lfir_action1_3 : 1;
    uint64_t pmc_lfir_action1_4 : 1;
    uint64_t pmc_lfir_action1_5 : 1;
    uint64_t pmc_lfir_action1_6 : 1;
    uint64_t pmc_lfir_action1_7 : 1;
    uint64_t pmc_lfir_action1_8 : 1;
    uint64_t pmc_lfir_action1_9 : 1;
    uint64_t pmc_lfir_action1_10 : 1;
    uint64_t pmc_lfir_action1_11 : 1;
    uint64_t pmc_lfir_action1_12 : 1;
    uint64_t pmc_lfir_action1_13 : 1;
    uint64_t pmc_lfir_action1_14 : 1;
    uint64_t pmc_lfir_action1_15 : 1;
    uint64_t pmc_lfir_action1_16 : 1;
    uint64_t pmc_lfir_action1_17 : 1;
    uint64_t pmc_lfir_action1_18 : 1;
    uint64_t pmc_lfir_action1_19 : 1;
    uint64_t pmc_lfir_action1_20 : 1;
    uint64_t pmc_lfir_action1_21 : 1;
    uint64_t pmc_lfir_action1_22 : 1;
    uint64_t pmc_lfir_action1_23 : 1;
    uint64_t pmc_lfir_action1_24 : 1;
    uint64_t pmc_lfir_action1_25 : 1;
    uint64_t pmc_lfir_action1_26 : 1;
    uint64_t pmc_lfir_action1_27 : 1;
    uint64_t pmc_lfir_action1_28 : 1;
    uint64_t pmc_lfir_action1_29 : 1;
    uint64_t pmc_lfir_action1_30 : 1;
    uint64_t pmc_lfir_action1_31 : 1;
    uint64_t pmc_lfir_action1_32 : 1;
    uint64_t pmc_lfir_action1_33 : 1;
    uint64_t pmc_lfir_action1_34 : 1;
    uint64_t pmc_lfir_action1_35 : 1;
    uint64_t pmc_lfir_action1_36 : 1;
    uint64_t pmc_lfir_action1_37_46 : 10;
    uint64_t pmc_lfir_action1_47 : 1;
    uint64_t pmc_lfir_action1_48 : 1;
    uint64_t _reserved0 : 15;
#else
    uint64_t _reserved0 : 15;
    uint64_t pmc_lfir_action1_48 : 1;
    uint64_t pmc_lfir_action1_47 : 1;
    uint64_t pmc_lfir_action1_37_46 : 10;
    uint64_t pmc_lfir_action1_36 : 1;
    uint64_t pmc_lfir_action1_35 : 1;
    uint64_t pmc_lfir_action1_34 : 1;
    uint64_t pmc_lfir_action1_33 : 1;
    uint64_t pmc_lfir_action1_32 : 1;
    uint64_t pmc_lfir_action1_31 : 1;
    uint64_t pmc_lfir_action1_30 : 1;
    uint64_t pmc_lfir_action1_29 : 1;
    uint64_t pmc_lfir_action1_28 : 1;
    uint64_t pmc_lfir_action1_27 : 1;
    uint64_t pmc_lfir_action1_26 : 1;
    uint64_t pmc_lfir_action1_25 : 1;
    uint64_t pmc_lfir_action1_24 : 1;
    uint64_t pmc_lfir_action1_23 : 1;
    uint64_t pmc_lfir_action1_22 : 1;
    uint64_t pmc_lfir_action1_21 : 1;
    uint64_t pmc_lfir_action1_20 : 1;
    uint64_t pmc_lfir_action1_19 : 1;
    uint64_t pmc_lfir_action1_18 : 1;
    uint64_t pmc_lfir_action1_17 : 1;
    uint64_t pmc_lfir_action1_16 : 1;
    uint64_t pmc_lfir_action1_15 : 1;
    uint64_t pmc_lfir_action1_14 : 1;
    uint64_t pmc_lfir_action1_13 : 1;
    uint64_t pmc_lfir_action1_12 : 1;
    uint64_t pmc_lfir_action1_11 : 1;
    uint64_t pmc_lfir_action1_10 : 1;
    uint64_t pmc_lfir_action1_9 : 1;
    uint64_t pmc_lfir_action1_8 : 1;
    uint64_t pmc_lfir_action1_7 : 1;
    uint64_t pmc_lfir_action1_6 : 1;
    uint64_t pmc_lfir_action1_5 : 1;
    uint64_t pmc_lfir_action1_4 : 1;
    uint64_t pmc_lfir_action1_3 : 1;
    uint64_t pmc_lfir_action1_2 : 1;
    uint64_t pmc_lfir_action1_1 : 1;
    uint64_t pmc_lfir_action1_0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pmc_lfir_action1_reg_t;



typedef union pmc_lfir_wof_reg {

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
    uint64_t pmc_lfir_wof : 49;
    uint64_t _reserved0 : 15;
#else
    uint64_t _reserved0 : 15;
    uint64_t pmc_lfir_wof : 49;
#endif // _BIG_ENDIAN
    } fields;
} pmc_lfir_wof_reg_t;


#endif // __ASSEMBLER__
#endif // __PMC_FIRMWARE_REGISTERS_H__

