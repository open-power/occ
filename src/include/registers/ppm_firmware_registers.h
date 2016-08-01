/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/ppm_firmware_registers.h $              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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
#ifndef __PPM_FIRMWARE_REGISTERS_H__
#define __PPM_FIRMWARE_REGISTERS_H__

/// \file ppm_firmware_registers.h
/// \brief C register structs for the PPM unit

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




typedef union ppm_gpmmr {

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
    uint64_t special_wkup_done : 1;
    uint64_t special_wkup_active : 1;
    uint64_t regular_wkup_active : 1;
    uint64_t special_wkup_requested : 1;
    uint64_t regular_wkup_requested : 1;
    uint64_t regular_wkup_present : 1;
    uint64_t block_reg_wkup_events : 1;
    uint64_t block_all_wkup_events : 1;
    uint64_t wkup_override_en : 1;
    uint64_t spc_wkup_override : 1;
    uint64_t reg_wkup_override : 1;
    uint64_t chiplet_enable : 1;
    uint64_t reserved_12_141 : 3;
    uint64_t reset_state_indicator : 1;
    uint64_t reserved2 : 48;
#else
    uint64_t reserved2 : 48;
    uint64_t reset_state_indicator : 1;
    uint64_t reserved_12_141 : 3;
    uint64_t chiplet_enable : 1;
    uint64_t reg_wkup_override : 1;
    uint64_t spc_wkup_override : 1;
    uint64_t wkup_override_en : 1;
    uint64_t block_all_wkup_events : 1;
    uint64_t block_reg_wkup_events : 1;
    uint64_t regular_wkup_present : 1;
    uint64_t regular_wkup_requested : 1;
    uint64_t special_wkup_requested : 1;
    uint64_t regular_wkup_active : 1;
    uint64_t special_wkup_active : 1;
    uint64_t special_wkup_done : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_gpmmr_t;



typedef union ppm_gpmmr_clr {

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
    uint64_t special_wkup_done : 1;
    uint64_t special_wkup_active : 1;
    uint64_t regular_wkup_active : 1;
    uint64_t special_wkup_requested : 1;
    uint64_t regular_wkup_requested : 1;
    uint64_t regular_wkup_present : 1;
    uint64_t block_reg_wkup_events : 1;
    uint64_t block_all_wkup_events : 1;
    uint64_t wkup_override_en : 1;
    uint64_t spc_wkup_override : 1;
    uint64_t reg_wkup_override : 1;
    uint64_t chiplet_enable : 1;
    uint64_t reserved_12_141 : 3;
    uint64_t reset_state_indicator : 1;
    uint64_t reserved2 : 48;
#else
    uint64_t reserved2 : 48;
    uint64_t reset_state_indicator : 1;
    uint64_t reserved_12_141 : 3;
    uint64_t chiplet_enable : 1;
    uint64_t reg_wkup_override : 1;
    uint64_t spc_wkup_override : 1;
    uint64_t wkup_override_en : 1;
    uint64_t block_all_wkup_events : 1;
    uint64_t block_reg_wkup_events : 1;
    uint64_t regular_wkup_present : 1;
    uint64_t regular_wkup_requested : 1;
    uint64_t special_wkup_requested : 1;
    uint64_t regular_wkup_active : 1;
    uint64_t special_wkup_active : 1;
    uint64_t special_wkup_done : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_gpmmr_clr_t;



typedef union ppm_gpmmr_or {

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
    uint64_t special_wkup_done : 1;
    uint64_t special_wkup_active : 1;
    uint64_t regular_wkup_active : 1;
    uint64_t special_wkup_requested : 1;
    uint64_t regular_wkup_requested : 1;
    uint64_t regular_wkup_present : 1;
    uint64_t block_reg_wkup_events : 1;
    uint64_t block_all_wkup_events : 1;
    uint64_t wkup_override_en : 1;
    uint64_t spc_wkup_override : 1;
    uint64_t reg_wkup_override : 1;
    uint64_t chiplet_enable : 1;
    uint64_t reserved_12_141 : 3;
    uint64_t reset_state_indicator : 1;
    uint64_t reserved2 : 48;
#else
    uint64_t reserved2 : 48;
    uint64_t reset_state_indicator : 1;
    uint64_t reserved_12_141 : 3;
    uint64_t chiplet_enable : 1;
    uint64_t reg_wkup_override : 1;
    uint64_t spc_wkup_override : 1;
    uint64_t wkup_override_en : 1;
    uint64_t block_all_wkup_events : 1;
    uint64_t block_reg_wkup_events : 1;
    uint64_t regular_wkup_present : 1;
    uint64_t regular_wkup_requested : 1;
    uint64_t special_wkup_requested : 1;
    uint64_t regular_wkup_active : 1;
    uint64_t special_wkup_active : 1;
    uint64_t special_wkup_done : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_gpmmr_or_t;



typedef union ppm_spwkup_otr {

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
    uint64_t otr_special_wkup : 1;
    uint64_t reserved1 : 63;
#else
    uint64_t reserved1 : 63;
    uint64_t otr_special_wkup : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_spwkup_otr_t;



typedef union ppm_spwkup_fsp {

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
    uint64_t fsp_special_wkup : 1;
    uint64_t reserved1 : 63;
#else
    uint64_t reserved1 : 63;
    uint64_t fsp_special_wkup : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_spwkup_fsp_t;



typedef union ppm_spwkup_occ {

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
    uint64_t occ_special_wkup : 1;
    uint64_t reserved1 : 63;
#else
    uint64_t reserved1 : 63;
    uint64_t occ_special_wkup : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_spwkup_occ_t;



typedef union ppm_spwkup_hyp {

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
    uint64_t hyp_special_wkup : 1;
    uint64_t reserved1 : 63;
#else
    uint64_t reserved1 : 63;
    uint64_t hyp_special_wkup : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_spwkup_hyp_t;



typedef union ppm_sshsrc {

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
    uint64_t stop_gated : 1;
    uint64_t stop_transition : 2;
    uint64_t special_wkup_done : 1;
    uint64_t req_stop_level : 4;
    uint64_t act_stop_level : 4;
    uint64_t reserved1 : 52;
#else
    uint64_t reserved1 : 52;
    uint64_t act_stop_level : 4;
    uint64_t req_stop_level : 4;
    uint64_t special_wkup_done : 1;
    uint64_t stop_transition : 2;
    uint64_t stop_gated : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_sshsrc_t;



typedef union ppm_sshfsp {

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
    uint64_t stop_gated_fsp : 1;
    uint64_t special_wkup_active_fsp : 1;
    uint64_t stop_transition__fsp : 2;
    uint64_t req_stop_level_fsp : 4;
    uint64_t act_stop_level_fsp : 4;
    uint64_t deepest_req_stop_level_fsp : 4;
    uint64_t deepest_act_stop_level_fsp : 4;
    uint64_t reserved1 : 44;
#else
    uint64_t reserved1 : 44;
    uint64_t deepest_act_stop_level_fsp : 4;
    uint64_t deepest_req_stop_level_fsp : 4;
    uint64_t act_stop_level_fsp : 4;
    uint64_t req_stop_level_fsp : 4;
    uint64_t stop_transition__fsp : 2;
    uint64_t special_wkup_active_fsp : 1;
    uint64_t stop_gated_fsp : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_sshfsp_t;



typedef union ppm_sshocc {

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
    uint64_t run_stop_occ : 1;
    uint64_t special_wkup_active_occ : 1;
    uint64_t stop_transition_occ : 2;
    uint64_t req_stop_level_occ : 4;
    uint64_t act_stop_level_occ : 4;
    uint64_t deepest_req_stop_level_occ : 4;
    uint64_t deepest_act_stop_level_occ : 4;
    uint64_t reserved1 : 44;
#else
    uint64_t reserved1 : 44;
    uint64_t deepest_act_stop_level_occ : 4;
    uint64_t deepest_req_stop_level_occ : 4;
    uint64_t act_stop_level_occ : 4;
    uint64_t req_stop_level_occ : 4;
    uint64_t stop_transition_occ : 2;
    uint64_t special_wkup_active_occ : 1;
    uint64_t run_stop_occ : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_sshocc_t;



typedef union ppm_sshotr {

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
    uint64_t run_stop_otr : 1;
    uint64_t special_wkup_active_otr : 1;
    uint64_t stop_transition_otr : 2;
    uint64_t req_stop_level_otr : 4;
    uint64_t act_stop_level_otr : 4;
    uint64_t deepest_req_stop_level_otr : 4;
    uint64_t deepest_act_stop_level_otr : 4;
    uint64_t reserved1 : 44;
#else
    uint64_t reserved1 : 44;
    uint64_t deepest_act_stop_level_otr : 4;
    uint64_t deepest_req_stop_level_otr : 4;
    uint64_t act_stop_level_otr : 4;
    uint64_t req_stop_level_otr : 4;
    uint64_t stop_transition_otr : 2;
    uint64_t special_wkup_active_otr : 1;
    uint64_t run_stop_otr : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_sshotr_t;



typedef union ppm_sshhyp {

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
    uint64_t run_stop_hyp : 1;
    uint64_t special_wkup_active_hyp : 1;
    uint64_t stop_transition_hyp : 2;
    uint64_t req_stop_level_hyp : 4;
    uint64_t act_stop_level_hyp : 4;
    uint64_t deepest_req_stop_level_hyp : 4;
    uint64_t deepest_act_stop_level_hyp : 4;
    uint64_t reserved1 : 44;
#else
    uint64_t reserved1 : 44;
    uint64_t deepest_act_stop_level_hyp : 4;
    uint64_t deepest_req_stop_level_hyp : 4;
    uint64_t act_stop_level_hyp : 4;
    uint64_t req_stop_level_hyp : 4;
    uint64_t stop_transition_hyp : 2;
    uint64_t special_wkup_active_hyp : 1;
    uint64_t run_stop_hyp : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_sshhyp_t;



typedef union ppm_pfcs {

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
    uint64_t vdd_pfet_force_state : 2;
    uint64_t vcs_pfet_force_state : 2;
    uint64_t vdd_pfet_val_override : 1;
    uint64_t vdd_pfet_sel_override : 1;
    uint64_t vcs_pfet_val_override : 1;
    uint64_t vcs_pfet_sel_override : 1;
    uint64_t vdd_pfet_regulation_finger_en : 1;
    uint64_t vdd_pfet_regulation_finger_value : 1;
    uint64_t reserved1 : 2;
    uint64_t vdd_pfet_enable_value : 8;
    uint64_t vdd_pfet_sel_value : 4;
    uint64_t vcs_pfet_enable_value : 8;
    uint64_t vcs_pfet_sel_value : 4;
    uint64_t reserved2 : 6;
    uint64_t vdd_pg_state : 4;
    uint64_t vdd_pg_sel : 4;
    uint64_t vcs_pg_state : 4;
    uint64_t vcs_pg_sel : 4;
    uint64_t reserved3 : 6;
#else
    uint64_t reserved3 : 6;
    uint64_t vcs_pg_sel : 4;
    uint64_t vcs_pg_state : 4;
    uint64_t vdd_pg_sel : 4;
    uint64_t vdd_pg_state : 4;
    uint64_t reserved2 : 6;
    uint64_t vcs_pfet_sel_value : 4;
    uint64_t vcs_pfet_enable_value : 8;
    uint64_t vdd_pfet_sel_value : 4;
    uint64_t vdd_pfet_enable_value : 8;
    uint64_t reserved1 : 2;
    uint64_t vdd_pfet_regulation_finger_value : 1;
    uint64_t vdd_pfet_regulation_finger_en : 1;
    uint64_t vcs_pfet_sel_override : 1;
    uint64_t vcs_pfet_val_override : 1;
    uint64_t vdd_pfet_sel_override : 1;
    uint64_t vdd_pfet_val_override : 1;
    uint64_t vcs_pfet_force_state : 2;
    uint64_t vdd_pfet_force_state : 2;
#endif // _BIG_ENDIAN
    } fields;
} ppm_pfcs_t;



typedef union ppm_pfcs_clr {

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
    uint64_t vdd_pfet_force_state : 2;
    uint64_t vcs_pfet_force_state : 2;
    uint64_t vdd_pfet_val_override : 1;
    uint64_t vdd_pfet_sel_override : 1;
    uint64_t vcs_pfet_val_override : 1;
    uint64_t vcs_pfet_sel_override : 1;
    uint64_t vdd_pfet_regulation_finger_en : 1;
    uint64_t vdd_pfet_regulation_finger_value : 1;
    uint64_t reserved1 : 2;
    uint64_t vdd_pfet_enable_value : 8;
    uint64_t vdd_pfet_sel_value : 4;
    uint64_t vcs_pfet_enable_value : 8;
    uint64_t vcs_pfet_sel_value : 4;
    uint64_t reserved2 : 6;
    uint64_t vdd_pg_state : 4;
    uint64_t vdd_pg_sel : 4;
    uint64_t vcs_pg_state : 4;
    uint64_t vcs_pg_sel : 4;
    uint64_t reserved3 : 6;
#else
    uint64_t reserved3 : 6;
    uint64_t vcs_pg_sel : 4;
    uint64_t vcs_pg_state : 4;
    uint64_t vdd_pg_sel : 4;
    uint64_t vdd_pg_state : 4;
    uint64_t reserved2 : 6;
    uint64_t vcs_pfet_sel_value : 4;
    uint64_t vcs_pfet_enable_value : 8;
    uint64_t vdd_pfet_sel_value : 4;
    uint64_t vdd_pfet_enable_value : 8;
    uint64_t reserved1 : 2;
    uint64_t vdd_pfet_regulation_finger_value : 1;
    uint64_t vdd_pfet_regulation_finger_en : 1;
    uint64_t vcs_pfet_sel_override : 1;
    uint64_t vcs_pfet_val_override : 1;
    uint64_t vdd_pfet_sel_override : 1;
    uint64_t vdd_pfet_val_override : 1;
    uint64_t vcs_pfet_force_state : 2;
    uint64_t vdd_pfet_force_state : 2;
#endif // _BIG_ENDIAN
    } fields;
} ppm_pfcs_clr_t;



typedef union ppm_pfcs_or {

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
    uint64_t vdd_pfet_force_state : 2;
    uint64_t vcs_pfet_force_state : 2;
    uint64_t vdd_pfet_val_override : 1;
    uint64_t vdd_pfet_sel_override : 1;
    uint64_t vcs_pfet_val_override : 1;
    uint64_t vcs_pfet_sel_override : 1;
    uint64_t vdd_pfet_regulation_finger_en : 1;
    uint64_t vdd_pfet_regulation_finger_value : 1;
    uint64_t reserved1 : 2;
    uint64_t vdd_pfet_enable_value : 8;
    uint64_t vdd_pfet_sel_value : 4;
    uint64_t vcs_pfet_enable_value : 8;
    uint64_t vcs_pfet_sel_value : 4;
    uint64_t reserved2 : 6;
    uint64_t vdd_pg_state : 4;
    uint64_t vdd_pg_sel : 4;
    uint64_t vcs_pg_state : 4;
    uint64_t vcs_pg_sel : 4;
    uint64_t reserved3 : 6;
#else
    uint64_t reserved3 : 6;
    uint64_t vcs_pg_sel : 4;
    uint64_t vcs_pg_state : 4;
    uint64_t vdd_pg_sel : 4;
    uint64_t vdd_pg_state : 4;
    uint64_t reserved2 : 6;
    uint64_t vcs_pfet_sel_value : 4;
    uint64_t vcs_pfet_enable_value : 8;
    uint64_t vdd_pfet_sel_value : 4;
    uint64_t vdd_pfet_enable_value : 8;
    uint64_t reserved1 : 2;
    uint64_t vdd_pfet_regulation_finger_value : 1;
    uint64_t vdd_pfet_regulation_finger_en : 1;
    uint64_t vcs_pfet_sel_override : 1;
    uint64_t vcs_pfet_val_override : 1;
    uint64_t vdd_pfet_sel_override : 1;
    uint64_t vdd_pfet_val_override : 1;
    uint64_t vcs_pfet_force_state : 2;
    uint64_t vdd_pfet_force_state : 2;
#endif // _BIG_ENDIAN
    } fields;
} ppm_pfcs_or_t;



typedef union ppm_pfdly {

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
    uint64_t powdn_dly : 4;
    uint64_t powup_dly : 4;
    uint64_t reserved1 : 56;
#else
    uint64_t reserved1 : 56;
    uint64_t powup_dly : 4;
    uint64_t powdn_dly : 4;
#endif // _BIG_ENDIAN
    } fields;
} ppm_pfdly_t;



typedef union ppm_pfsns {

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
    uint64_t vdd_pfets_enabled_sense : 1;
    uint64_t vdd_pfets_disabled_sense : 1;
    uint64_t vcs_pfets_enabled_sense : 1;
    uint64_t vcs_pfets_disabled_sense : 1;
    uint64_t reserved1 : 12;
    uint64_t tp_vdd_pfet_enable_actual : 8;
    uint64_t tp_vcs_pfet_enable_actual : 8;
    uint64_t reserved2 : 32;
#else
    uint64_t reserved2 : 32;
    uint64_t tp_vcs_pfet_enable_actual : 8;
    uint64_t tp_vdd_pfet_enable_actual : 8;
    uint64_t reserved1 : 12;
    uint64_t vcs_pfets_disabled_sense : 1;
    uint64_t vcs_pfets_enabled_sense : 1;
    uint64_t vdd_pfets_disabled_sense : 1;
    uint64_t vdd_pfets_enabled_sense : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_pfsns_t;



typedef union ppm_pfoff {

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
    uint64_t vdd_voff_sel : 4;
    uint64_t vcs_voff_sel : 4;
    uint64_t reserved1 : 56;
#else
    uint64_t reserved1 : 56;
    uint64_t vcs_voff_sel : 4;
    uint64_t vdd_voff_sel : 4;
#endif // _BIG_ENDIAN
    } fields;
} ppm_pfoff_t;



typedef union ppm_scratch0 {

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
    uint64_t data : 64;
#else
    uint64_t data : 64;
#endif // _BIG_ENDIAN
    } fields;
} ppm_scratch0_t;



typedef union ppm_scratch1 {

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
    uint64_t data : 64;
#else
    uint64_t data : 64;
#endif // _BIG_ENDIAN
    } fields;
} ppm_scratch1_t;



typedef union ppm_cgcr {

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
    uint64_t clkglm_async_reset : 1;
    uint64_t reserved_1_21 : 2;
    uint64_t clkglm_sel : 1;
    uint64_t reserved2 : 60;
#else
    uint64_t reserved2 : 60;
    uint64_t clkglm_sel : 1;
    uint64_t reserved_1_21 : 2;
    uint64_t clkglm_async_reset : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_cgcr_t;



typedef union ppm_cgcr_clr {

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
    uint64_t clkglm_async_reset : 1;
    uint64_t reserved_1_21 : 2;
    uint64_t clkglm_sel : 1;
    uint64_t reserved2 : 60;
#else
    uint64_t reserved2 : 60;
    uint64_t clkglm_sel : 1;
    uint64_t reserved_1_21 : 2;
    uint64_t clkglm_async_reset : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_cgcr_clr_t;



typedef union ppm_cgcr_or {

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
    uint64_t clkglm_async_reset : 1;
    uint64_t reserved_1_21 : 2;
    uint64_t clkglm_sel : 1;
    uint64_t reserved2 : 60;
#else
    uint64_t reserved2 : 60;
    uint64_t clkglm_sel : 1;
    uint64_t reserved_1_21 : 2;
    uint64_t clkglm_async_reset : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_cgcr_or_t;



typedef union ppm_pig {

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
    uint64_t req_intr_type : 3;
    uint64_t req_intr_payload : 12;
    uint64_t granted_packet : 16;
    uint64_t intr_granted : 1;
    uint64_t reserved2 : 1;
    uint64_t granted__source : 2;
    uint64_t reserved3 : 1;
    uint64_t pending_source : 3;
    uint64_t reserved4 : 24;
#else
    uint64_t reserved4 : 24;
    uint64_t pending_source : 3;
    uint64_t reserved3 : 1;
    uint64_t granted__source : 2;
    uint64_t reserved2 : 1;
    uint64_t intr_granted : 1;
    uint64_t granted_packet : 16;
    uint64_t req_intr_payload : 12;
    uint64_t req_intr_type : 3;
    uint64_t reserved1 : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_pig_t;



typedef union ppm_ivrmcr {

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
    uint64_t ivrm_vid_valid : 1;
    uint64_t ivrm_bypass_b : 1;
    uint64_t ivrm_poweron : 1;
    uint64_t ivrm_vreg_slow_dc : 1;
    uint64_t reserved1 : 4;
    uint64_t reserved2 : 56;
#else
    uint64_t reserved2 : 56;
    uint64_t reserved1 : 4;
    uint64_t ivrm_vreg_slow_dc : 1;
    uint64_t ivrm_poweron : 1;
    uint64_t ivrm_bypass_b : 1;
    uint64_t ivrm_vid_valid : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_ivrmcr_t;



typedef union ppm_ivrmcr_clr {

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
    uint64_t ivrm_vid_valid : 1;
    uint64_t ivrm_bypass_b : 1;
    uint64_t ivrm_poweron : 1;
    uint64_t ivrm_vreg_slow_dc : 1;
    uint64_t reserved1 : 4;
    uint64_t reserved2 : 56;
#else
    uint64_t reserved2 : 56;
    uint64_t reserved1 : 4;
    uint64_t ivrm_vreg_slow_dc : 1;
    uint64_t ivrm_poweron : 1;
    uint64_t ivrm_bypass_b : 1;
    uint64_t ivrm_vid_valid : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_ivrmcr_clr_t;



typedef union ppm_ivrmcr_or {

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
    uint64_t ivrm_vid_valid : 1;
    uint64_t ivrm_bypass_b : 1;
    uint64_t ivrm_poweron : 1;
    uint64_t ivrm_vreg_slow_dc : 1;
    uint64_t reserved1 : 4;
    uint64_t reserved2 : 56;
#else
    uint64_t reserved2 : 56;
    uint64_t reserved1 : 4;
    uint64_t ivrm_vreg_slow_dc : 1;
    uint64_t ivrm_poweron : 1;
    uint64_t ivrm_bypass_b : 1;
    uint64_t ivrm_vid_valid : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_ivrmcr_or_t;



typedef union ppm_ivrmst {

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
    uint64_t ivrm_vid_done : 1;
    uint64_t reserved1 : 63;
#else
    uint64_t reserved1 : 63;
    uint64_t ivrm_vid_done : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_ivrmst_t;



typedef union ppm_ivrmdvr {

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
    uint64_t ivrm_ivid : 8;
    uint64_t reserved_8_10 : 3;
    uint64_t ivrm_pfet_strength_core : 5;
    uint64_t reserved_16_18 : 3;
    uint64_t ivrm_pfet_strength_cache : 5;
    uint64_t reserved1 : 40;
#else
    uint64_t reserved1 : 40;
    uint64_t ivrm_pfet_strength_cache : 5;
    uint64_t reserved_16_18 : 3;
    uint64_t ivrm_pfet_strength_core : 5;
    uint64_t reserved_8_10 : 3;
    uint64_t ivrm_ivid : 8;
#endif // _BIG_ENDIAN
    } fields;
} ppm_ivrmdvr_t;



typedef union ppm_ivrmavr {

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
    uint64_t ivrm_ivid : 8;
    uint64_t reserved1 : 3;
    uint64_t ivrm_pfet_strength : 5;
    uint64_t reserved2 : 3;
    uint64_t ivrm_enabled_history : 1;
    uint64_t reserved3 : 4;
    uint64_t ivrm_vid_valid : 1;
    uint64_t ivrm_bypass_b : 1;
    uint64_t ivrm_poweron : 1;
    uint64_t ivrm_vreg_slow_dc : 1;
    uint64_t reserved4 : 36;
#else
    uint64_t reserved4 : 36;
    uint64_t ivrm_vreg_slow_dc : 1;
    uint64_t ivrm_poweron : 1;
    uint64_t ivrm_bypass_b : 1;
    uint64_t ivrm_vid_valid : 1;
    uint64_t reserved3 : 4;
    uint64_t ivrm_enabled_history : 1;
    uint64_t reserved2 : 3;
    uint64_t ivrm_pfet_strength : 5;
    uint64_t reserved1 : 3;
    uint64_t ivrm_ivid : 8;
#endif // _BIG_ENDIAN
    } fields;
} ppm_ivrmavr_t;



typedef union ppm_vdmcr {

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
    uint64_t vdm_poweron : 1;
    uint64_t vdm_disable : 1;
    uint64_t reserved1 : 62;
#else
    uint64_t reserved1 : 62;
    uint64_t vdm_disable : 1;
    uint64_t vdm_poweron : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_vdmcr_t;



typedef union ppm_vdmcr_clr {

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
    uint64_t vdm_poweron : 1;
    uint64_t vdm_disable : 1;
    uint64_t reserved1 : 62;
#else
    uint64_t reserved1 : 62;
    uint64_t vdm_disable : 1;
    uint64_t vdm_poweron : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_vdmcr_clr_t;



typedef union ppm_vdmcr_or {

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
    uint64_t vdm_poweron : 1;
    uint64_t vdm_disable : 1;
    uint64_t reserved1 : 62;
#else
    uint64_t reserved1 : 62;
    uint64_t vdm_disable : 1;
    uint64_t vdm_poweron : 1;
#endif // _BIG_ENDIAN
    } fields;
} ppm_vdmcr_or_t;


#endif // __ASSEMBLER__
#endif // __PPM_FIRMWARE_REGISTERS_H__

