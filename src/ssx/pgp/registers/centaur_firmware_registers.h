/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/registers/centaur_firmware_registers.h $          */
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
#ifndef __CENTAUR_FIRMWARE_REGISTERS_H__
#define __CENTAUR_FIRMWARE_REGISTERS_H__

/// \file centaur_firmware_registers.h
/// \brief C register structs for the CENTAUR unit

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




typedef union centaur_device_id {

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
    uint64_t cfam_id : 32;
    uint64_t module_id : 2;
    uint64_t _reserved0 : 30;
#else
    uint64_t _reserved0 : 30;
    uint64_t module_id : 2;
    uint64_t cfam_id : 32;
#endif // _BIG_ENDIAN
    } fields;
} centaur_device_id_t;



typedef union centaur_mbs_fir_reg {

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
    uint64_t host_protocol_error : 1;
    uint64_t int_protocol_error : 1;
    uint64_t invalid_address_error : 1;
    uint64_t external_timeout : 1;
    uint64_t internal_timeout : 1;
    uint64_t int_buffer_ce : 1;
    uint64_t int_buffer_ue : 1;
    uint64_t int_buffer_sue : 1;
    uint64_t int_parity_error : 1;
    uint64_t cache_srw_ce : 1;
    uint64_t cache_srw_ue : 1;
    uint64_t cache_srw_sue : 1;
    uint64_t cache_co_ce : 1;
    uint64_t cache_co_ue : 1;
    uint64_t cache_co_sue : 1;
    uint64_t dir_ce : 1;
    uint64_t dir_ue : 1;
    uint64_t dir_member_deleted : 1;
    uint64_t dir_all_members_deleted : 1;
    uint64_t lru_error : 1;
    uint64_t edram_error : 1;
    uint64_t emergency_throttle_set : 1;
    uint64_t host_inband_read_error : 1;
    uint64_t host_inband_write_error : 1;
    uint64_t occ_inband_read_error : 1;
    uint64_t occ_inband_write_error : 1;
    uint64_t srb_buffer_ce : 1;
    uint64_t srb_buffer_ue : 1;
    uint64_t srb_buffer_sue : 1;
    uint64_t dir_purge_ce : 1;
    uint64_t spare_fir30 : 1;
    uint64_t spare_fir31 : 1;
    uint64_t internal_scom_error : 1;
    uint64_t internal_scom_error_copy : 1;
    uint64_t _reserved0 : 30;
#else
    uint64_t _reserved0 : 30;
    uint64_t internal_scom_error_copy : 1;
    uint64_t internal_scom_error : 1;
    uint64_t spare_fir31 : 1;
    uint64_t spare_fir30 : 1;
    uint64_t dir_purge_ce : 1;
    uint64_t srb_buffer_sue : 1;
    uint64_t srb_buffer_ue : 1;
    uint64_t srb_buffer_ce : 1;
    uint64_t occ_inband_write_error : 1;
    uint64_t occ_inband_read_error : 1;
    uint64_t host_inband_write_error : 1;
    uint64_t host_inband_read_error : 1;
    uint64_t emergency_throttle_set : 1;
    uint64_t edram_error : 1;
    uint64_t lru_error : 1;
    uint64_t dir_all_members_deleted : 1;
    uint64_t dir_member_deleted : 1;
    uint64_t dir_ue : 1;
    uint64_t dir_ce : 1;
    uint64_t cache_co_sue : 1;
    uint64_t cache_co_ue : 1;
    uint64_t cache_co_ce : 1;
    uint64_t cache_srw_sue : 1;
    uint64_t cache_srw_ue : 1;
    uint64_t cache_srw_ce : 1;
    uint64_t int_parity_error : 1;
    uint64_t int_buffer_sue : 1;
    uint64_t int_buffer_ue : 1;
    uint64_t int_buffer_ce : 1;
    uint64_t internal_timeout : 1;
    uint64_t external_timeout : 1;
    uint64_t invalid_address_error : 1;
    uint64_t int_protocol_error : 1;
    uint64_t host_protocol_error : 1;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbs_fir_reg_t;



typedef union centaur_mbs_fir_reg_and {

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
    uint64_t host_protocol_error : 1;
    uint64_t int_protocol_error : 1;
    uint64_t invalid_address_error : 1;
    uint64_t external_timeout : 1;
    uint64_t internal_timeout : 1;
    uint64_t int_buffer_ce : 1;
    uint64_t int_buffer_ue : 1;
    uint64_t int_buffer_sue : 1;
    uint64_t int_parity_error : 1;
    uint64_t cache_srw_ce : 1;
    uint64_t cache_srw_ue : 1;
    uint64_t cache_srw_sue : 1;
    uint64_t cache_co_ce : 1;
    uint64_t cache_co_ue : 1;
    uint64_t cache_co_sue : 1;
    uint64_t dir_ce : 1;
    uint64_t dir_ue : 1;
    uint64_t dir_member_deleted : 1;
    uint64_t dir_all_members_deleted : 1;
    uint64_t lru_error : 1;
    uint64_t edram_error : 1;
    uint64_t emergency_throttle_set : 1;
    uint64_t host_inband_read_error : 1;
    uint64_t host_inband_write_error : 1;
    uint64_t occ_inband_read_error : 1;
    uint64_t occ_inband_write_error : 1;
    uint64_t srb_buffer_ce : 1;
    uint64_t srb_buffer_ue : 1;
    uint64_t srb_buffer_sue : 1;
    uint64_t dir_purge_ce : 1;
    uint64_t spare_fir30 : 1;
    uint64_t spare_fir31 : 1;
    uint64_t internal_scom_error : 1;
    uint64_t internal_scom_error_copy : 1;
    uint64_t _reserved0 : 30;
#else
    uint64_t _reserved0 : 30;
    uint64_t internal_scom_error_copy : 1;
    uint64_t internal_scom_error : 1;
    uint64_t spare_fir31 : 1;
    uint64_t spare_fir30 : 1;
    uint64_t dir_purge_ce : 1;
    uint64_t srb_buffer_sue : 1;
    uint64_t srb_buffer_ue : 1;
    uint64_t srb_buffer_ce : 1;
    uint64_t occ_inband_write_error : 1;
    uint64_t occ_inband_read_error : 1;
    uint64_t host_inband_write_error : 1;
    uint64_t host_inband_read_error : 1;
    uint64_t emergency_throttle_set : 1;
    uint64_t edram_error : 1;
    uint64_t lru_error : 1;
    uint64_t dir_all_members_deleted : 1;
    uint64_t dir_member_deleted : 1;
    uint64_t dir_ue : 1;
    uint64_t dir_ce : 1;
    uint64_t cache_co_sue : 1;
    uint64_t cache_co_ue : 1;
    uint64_t cache_co_ce : 1;
    uint64_t cache_srw_sue : 1;
    uint64_t cache_srw_ue : 1;
    uint64_t cache_srw_ce : 1;
    uint64_t int_parity_error : 1;
    uint64_t int_buffer_sue : 1;
    uint64_t int_buffer_ue : 1;
    uint64_t int_buffer_ce : 1;
    uint64_t internal_timeout : 1;
    uint64_t external_timeout : 1;
    uint64_t invalid_address_error : 1;
    uint64_t int_protocol_error : 1;
    uint64_t host_protocol_error : 1;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbs_fir_reg_and_t;



typedef union centaur_mbs_fir_reg_or {

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
    uint64_t host_protocol_error : 1;
    uint64_t int_protocol_error : 1;
    uint64_t invalid_address_error : 1;
    uint64_t external_timeout : 1;
    uint64_t internal_timeout : 1;
    uint64_t int_buffer_ce : 1;
    uint64_t int_buffer_ue : 1;
    uint64_t int_buffer_sue : 1;
    uint64_t int_parity_error : 1;
    uint64_t cache_srw_ce : 1;
    uint64_t cache_srw_ue : 1;
    uint64_t cache_srw_sue : 1;
    uint64_t cache_co_ce : 1;
    uint64_t cache_co_ue : 1;
    uint64_t cache_co_sue : 1;
    uint64_t dir_ce : 1;
    uint64_t dir_ue : 1;
    uint64_t dir_member_deleted : 1;
    uint64_t dir_all_members_deleted : 1;
    uint64_t lru_error : 1;
    uint64_t edram_error : 1;
    uint64_t emergency_throttle_set : 1;
    uint64_t host_inband_read_error : 1;
    uint64_t host_inband_write_error : 1;
    uint64_t occ_inband_read_error : 1;
    uint64_t occ_inband_write_error : 1;
    uint64_t srb_buffer_ce : 1;
    uint64_t srb_buffer_ue : 1;
    uint64_t srb_buffer_sue : 1;
    uint64_t dir_purge_ce : 1;
    uint64_t spare_fir30 : 1;
    uint64_t spare_fir31 : 1;
    uint64_t internal_scom_error : 1;
    uint64_t internal_scom_error_copy : 1;
    uint64_t _reserved0 : 30;
#else
    uint64_t _reserved0 : 30;
    uint64_t internal_scom_error_copy : 1;
    uint64_t internal_scom_error : 1;
    uint64_t spare_fir31 : 1;
    uint64_t spare_fir30 : 1;
    uint64_t dir_purge_ce : 1;
    uint64_t srb_buffer_sue : 1;
    uint64_t srb_buffer_ue : 1;
    uint64_t srb_buffer_ce : 1;
    uint64_t occ_inband_write_error : 1;
    uint64_t occ_inband_read_error : 1;
    uint64_t host_inband_write_error : 1;
    uint64_t host_inband_read_error : 1;
    uint64_t emergency_throttle_set : 1;
    uint64_t edram_error : 1;
    uint64_t lru_error : 1;
    uint64_t dir_all_members_deleted : 1;
    uint64_t dir_member_deleted : 1;
    uint64_t dir_ue : 1;
    uint64_t dir_ce : 1;
    uint64_t cache_co_sue : 1;
    uint64_t cache_co_ue : 1;
    uint64_t cache_co_ce : 1;
    uint64_t cache_srw_sue : 1;
    uint64_t cache_srw_ue : 1;
    uint64_t cache_srw_ce : 1;
    uint64_t int_parity_error : 1;
    uint64_t int_buffer_sue : 1;
    uint64_t int_buffer_ue : 1;
    uint64_t int_buffer_ce : 1;
    uint64_t internal_timeout : 1;
    uint64_t external_timeout : 1;
    uint64_t invalid_address_error : 1;
    uint64_t int_protocol_error : 1;
    uint64_t host_protocol_error : 1;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbs_fir_reg_or_t;



typedef union centaur_mbs_fir_mask_reg {

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
    uint64_t host_protocol_error : 1;
    uint64_t int_protocol_error : 1;
    uint64_t invalid_address_error : 1;
    uint64_t external_timeout : 1;
    uint64_t internal_timeout : 1;
    uint64_t int_buffer_ce : 1;
    uint64_t int_buffer_ue : 1;
    uint64_t int_buffer_sue : 1;
    uint64_t int_parity_error : 1;
    uint64_t cache_srw_ce : 1;
    uint64_t cache_srw_ue : 1;
    uint64_t cache_srw_sue : 1;
    uint64_t cache_co_ce : 1;
    uint64_t cache_co_ue : 1;
    uint64_t cache_co_sue : 1;
    uint64_t dir_ce : 1;
    uint64_t dir_ue : 1;
    uint64_t dir_member_deleted : 1;
    uint64_t dir_all_members_deleted : 1;
    uint64_t lru_error : 1;
    uint64_t edram_error : 1;
    uint64_t emergency_throttle_set : 1;
    uint64_t host_inband_read_error : 1;
    uint64_t host_inband_write_error : 1;
    uint64_t occ_inband_read_error : 1;
    uint64_t occ_inband_write_error : 1;
    uint64_t srb_buffer_ce : 1;
    uint64_t srb_buffer_ue : 1;
    uint64_t srb_buffer_sue : 1;
    uint64_t dir_purge_ce : 1;
    uint64_t spare_fir30 : 1;
    uint64_t spare_fir31 : 1;
    uint64_t internal_scom_error : 1;
    uint64_t internal_scom_error_copy : 1;
    uint64_t _reserved0 : 30;
#else
    uint64_t _reserved0 : 30;
    uint64_t internal_scom_error_copy : 1;
    uint64_t internal_scom_error : 1;
    uint64_t spare_fir31 : 1;
    uint64_t spare_fir30 : 1;
    uint64_t dir_purge_ce : 1;
    uint64_t srb_buffer_sue : 1;
    uint64_t srb_buffer_ue : 1;
    uint64_t srb_buffer_ce : 1;
    uint64_t occ_inband_write_error : 1;
    uint64_t occ_inband_read_error : 1;
    uint64_t host_inband_write_error : 1;
    uint64_t host_inband_read_error : 1;
    uint64_t emergency_throttle_set : 1;
    uint64_t edram_error : 1;
    uint64_t lru_error : 1;
    uint64_t dir_all_members_deleted : 1;
    uint64_t dir_member_deleted : 1;
    uint64_t dir_ue : 1;
    uint64_t dir_ce : 1;
    uint64_t cache_co_sue : 1;
    uint64_t cache_co_ue : 1;
    uint64_t cache_co_ce : 1;
    uint64_t cache_srw_sue : 1;
    uint64_t cache_srw_ue : 1;
    uint64_t cache_srw_ce : 1;
    uint64_t int_parity_error : 1;
    uint64_t int_buffer_sue : 1;
    uint64_t int_buffer_ue : 1;
    uint64_t int_buffer_ce : 1;
    uint64_t internal_timeout : 1;
    uint64_t external_timeout : 1;
    uint64_t invalid_address_error : 1;
    uint64_t int_protocol_error : 1;
    uint64_t host_protocol_error : 1;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbs_fir_mask_reg_t;



typedef union centaur_mbs_fir_mask_reg_and {

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
    uint64_t host_protocol_error : 1;
    uint64_t int_protocol_error : 1;
    uint64_t invalid_address_error : 1;
    uint64_t external_timeout : 1;
    uint64_t internal_timeout : 1;
    uint64_t int_buffer_ce : 1;
    uint64_t int_buffer_ue : 1;
    uint64_t int_buffer_sue : 1;
    uint64_t int_parity_error : 1;
    uint64_t cache_srw_ce : 1;
    uint64_t cache_srw_ue : 1;
    uint64_t cache_srw_sue : 1;
    uint64_t cache_co_ce : 1;
    uint64_t cache_co_ue : 1;
    uint64_t cache_co_sue : 1;
    uint64_t dir_ce : 1;
    uint64_t dir_ue : 1;
    uint64_t dir_member_deleted : 1;
    uint64_t dir_all_members_deleted : 1;
    uint64_t lru_error : 1;
    uint64_t edram_error : 1;
    uint64_t emergency_throttle_set : 1;
    uint64_t host_inband_read_error : 1;
    uint64_t host_inband_write_error : 1;
    uint64_t occ_inband_read_error : 1;
    uint64_t occ_inband_write_error : 1;
    uint64_t srb_buffer_ce : 1;
    uint64_t srb_buffer_ue : 1;
    uint64_t srb_buffer_sue : 1;
    uint64_t dir_purge_ce : 1;
    uint64_t spare_fir30 : 1;
    uint64_t spare_fir31 : 1;
    uint64_t internal_scom_error : 1;
    uint64_t internal_scom_error_copy : 1;
    uint64_t _reserved0 : 30;
#else
    uint64_t _reserved0 : 30;
    uint64_t internal_scom_error_copy : 1;
    uint64_t internal_scom_error : 1;
    uint64_t spare_fir31 : 1;
    uint64_t spare_fir30 : 1;
    uint64_t dir_purge_ce : 1;
    uint64_t srb_buffer_sue : 1;
    uint64_t srb_buffer_ue : 1;
    uint64_t srb_buffer_ce : 1;
    uint64_t occ_inband_write_error : 1;
    uint64_t occ_inband_read_error : 1;
    uint64_t host_inband_write_error : 1;
    uint64_t host_inband_read_error : 1;
    uint64_t emergency_throttle_set : 1;
    uint64_t edram_error : 1;
    uint64_t lru_error : 1;
    uint64_t dir_all_members_deleted : 1;
    uint64_t dir_member_deleted : 1;
    uint64_t dir_ue : 1;
    uint64_t dir_ce : 1;
    uint64_t cache_co_sue : 1;
    uint64_t cache_co_ue : 1;
    uint64_t cache_co_ce : 1;
    uint64_t cache_srw_sue : 1;
    uint64_t cache_srw_ue : 1;
    uint64_t cache_srw_ce : 1;
    uint64_t int_parity_error : 1;
    uint64_t int_buffer_sue : 1;
    uint64_t int_buffer_ue : 1;
    uint64_t int_buffer_ce : 1;
    uint64_t internal_timeout : 1;
    uint64_t external_timeout : 1;
    uint64_t invalid_address_error : 1;
    uint64_t int_protocol_error : 1;
    uint64_t host_protocol_error : 1;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbs_fir_mask_reg_and_t;



typedef union centaur_mbs_fir_mask_reg_or {

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
    uint64_t host_protocol_error : 1;
    uint64_t int_protocol_error : 1;
    uint64_t invalid_address_error : 1;
    uint64_t external_timeout : 1;
    uint64_t internal_timeout : 1;
    uint64_t int_buffer_ce : 1;
    uint64_t int_buffer_ue : 1;
    uint64_t int_buffer_sue : 1;
    uint64_t int_parity_error : 1;
    uint64_t cache_srw_ce : 1;
    uint64_t cache_srw_ue : 1;
    uint64_t cache_srw_sue : 1;
    uint64_t cache_co_ce : 1;
    uint64_t cache_co_ue : 1;
    uint64_t cache_co_sue : 1;
    uint64_t dir_ce : 1;
    uint64_t dir_ue : 1;
    uint64_t dir_member_deleted : 1;
    uint64_t dir_all_members_deleted : 1;
    uint64_t lru_error : 1;
    uint64_t edram_error : 1;
    uint64_t emergency_throttle_set : 1;
    uint64_t host_inband_read_error : 1;
    uint64_t host_inband_write_error : 1;
    uint64_t occ_inband_read_error : 1;
    uint64_t occ_inband_write_error : 1;
    uint64_t srb_buffer_ce : 1;
    uint64_t srb_buffer_ue : 1;
    uint64_t srb_buffer_sue : 1;
    uint64_t dir_purge_ce : 1;
    uint64_t spare_fir30 : 1;
    uint64_t spare_fir31 : 1;
    uint64_t internal_scom_error : 1;
    uint64_t internal_scom_error_copy : 1;
    uint64_t _reserved0 : 30;
#else
    uint64_t _reserved0 : 30;
    uint64_t internal_scom_error_copy : 1;
    uint64_t internal_scom_error : 1;
    uint64_t spare_fir31 : 1;
    uint64_t spare_fir30 : 1;
    uint64_t dir_purge_ce : 1;
    uint64_t srb_buffer_sue : 1;
    uint64_t srb_buffer_ue : 1;
    uint64_t srb_buffer_ce : 1;
    uint64_t occ_inband_write_error : 1;
    uint64_t occ_inband_read_error : 1;
    uint64_t host_inband_write_error : 1;
    uint64_t host_inband_read_error : 1;
    uint64_t emergency_throttle_set : 1;
    uint64_t edram_error : 1;
    uint64_t lru_error : 1;
    uint64_t dir_all_members_deleted : 1;
    uint64_t dir_member_deleted : 1;
    uint64_t dir_ue : 1;
    uint64_t dir_ce : 1;
    uint64_t cache_co_sue : 1;
    uint64_t cache_co_ue : 1;
    uint64_t cache_co_ce : 1;
    uint64_t cache_srw_sue : 1;
    uint64_t cache_srw_ue : 1;
    uint64_t cache_srw_ce : 1;
    uint64_t int_parity_error : 1;
    uint64_t int_buffer_sue : 1;
    uint64_t int_buffer_ue : 1;
    uint64_t int_buffer_ce : 1;
    uint64_t internal_timeout : 1;
    uint64_t external_timeout : 1;
    uint64_t invalid_address_error : 1;
    uint64_t int_protocol_error : 1;
    uint64_t host_protocol_error : 1;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbs_fir_mask_reg_or_t;



typedef union centaur_mbs_fir_action0_reg {

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
    uint64_t host_protocol_error : 1;
    uint64_t int_protocol_error : 1;
    uint64_t invalid_address_error : 1;
    uint64_t external_timeout : 1;
    uint64_t internal_timeout : 1;
    uint64_t int_buffer_ce : 1;
    uint64_t int_buffer_ue : 1;
    uint64_t int_buffer_sue : 1;
    uint64_t int_parity_error : 1;
    uint64_t cache_srw_ce : 1;
    uint64_t cache_srw_ue : 1;
    uint64_t cache_srw_sue : 1;
    uint64_t cache_co_ce : 1;
    uint64_t cache_co_ue : 1;
    uint64_t cache_co_sue : 1;
    uint64_t dir_ce : 1;
    uint64_t dir_ue : 1;
    uint64_t dir_member_deleted : 1;
    uint64_t dir_all_members_deleted : 1;
    uint64_t lru_error : 1;
    uint64_t edram_error : 1;
    uint64_t emergency_throttle_set : 1;
    uint64_t host_inband_read_error : 1;
    uint64_t host_inband_write_error : 1;
    uint64_t occ_inband_read_error : 1;
    uint64_t occ_inband_write_error : 1;
    uint64_t srb_buffer_ce : 1;
    uint64_t srb_buffer_ue : 1;
    uint64_t srb_buffer_sue : 1;
    uint64_t dir_purge_ce : 1;
    uint64_t spare_fir30 : 1;
    uint64_t spare_fir31 : 1;
    uint64_t internal_scom_error : 1;
    uint64_t internal_scom_error_copy : 1;
    uint64_t _reserved0 : 30;
#else
    uint64_t _reserved0 : 30;
    uint64_t internal_scom_error_copy : 1;
    uint64_t internal_scom_error : 1;
    uint64_t spare_fir31 : 1;
    uint64_t spare_fir30 : 1;
    uint64_t dir_purge_ce : 1;
    uint64_t srb_buffer_sue : 1;
    uint64_t srb_buffer_ue : 1;
    uint64_t srb_buffer_ce : 1;
    uint64_t occ_inband_write_error : 1;
    uint64_t occ_inband_read_error : 1;
    uint64_t host_inband_write_error : 1;
    uint64_t host_inband_read_error : 1;
    uint64_t emergency_throttle_set : 1;
    uint64_t edram_error : 1;
    uint64_t lru_error : 1;
    uint64_t dir_all_members_deleted : 1;
    uint64_t dir_member_deleted : 1;
    uint64_t dir_ue : 1;
    uint64_t dir_ce : 1;
    uint64_t cache_co_sue : 1;
    uint64_t cache_co_ue : 1;
    uint64_t cache_co_ce : 1;
    uint64_t cache_srw_sue : 1;
    uint64_t cache_srw_ue : 1;
    uint64_t cache_srw_ce : 1;
    uint64_t int_parity_error : 1;
    uint64_t int_buffer_sue : 1;
    uint64_t int_buffer_ue : 1;
    uint64_t int_buffer_ce : 1;
    uint64_t internal_timeout : 1;
    uint64_t external_timeout : 1;
    uint64_t invalid_address_error : 1;
    uint64_t int_protocol_error : 1;
    uint64_t host_protocol_error : 1;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbs_fir_action0_reg_t;



typedef union centaur_mbs_firact1 {

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
    uint64_t host_protocol_error : 1;
    uint64_t int_protocol_error : 1;
    uint64_t invalid_address_error : 1;
    uint64_t external_timeout : 1;
    uint64_t internal_timeout : 1;
    uint64_t int_buffer_ce : 1;
    uint64_t int_buffer_ue : 1;
    uint64_t int_buffer_sue : 1;
    uint64_t int_parity_error : 1;
    uint64_t cache_srw_ce : 1;
    uint64_t cache_srw_ue : 1;
    uint64_t cache_srw_sue : 1;
    uint64_t cache_co_ce : 1;
    uint64_t cache_co_ue : 1;
    uint64_t cache_co_sue : 1;
    uint64_t dir_ce : 1;
    uint64_t dir_ue : 1;
    uint64_t dir_member_deleted : 1;
    uint64_t dir_all_members_deleted : 1;
    uint64_t lru_error : 1;
    uint64_t edram_error : 1;
    uint64_t emergency_throttle_set : 1;
    uint64_t host_inband_read_error : 1;
    uint64_t host_inband_write_error : 1;
    uint64_t occ_inband_read_error : 1;
    uint64_t occ_inband_write_error : 1;
    uint64_t srb_buffer_ce : 1;
    uint64_t srb_buffer_ue : 1;
    uint64_t srb_buffer_sue : 1;
    uint64_t dir_purge_ce : 1;
    uint64_t spare_fir30 : 1;
    uint64_t spare_fir31 : 1;
    uint64_t internal_scom_error : 1;
    uint64_t internal_scom_error_copy : 1;
    uint64_t _reserved0 : 30;
#else
    uint64_t _reserved0 : 30;
    uint64_t internal_scom_error_copy : 1;
    uint64_t internal_scom_error : 1;
    uint64_t spare_fir31 : 1;
    uint64_t spare_fir30 : 1;
    uint64_t dir_purge_ce : 1;
    uint64_t srb_buffer_sue : 1;
    uint64_t srb_buffer_ue : 1;
    uint64_t srb_buffer_ce : 1;
    uint64_t occ_inband_write_error : 1;
    uint64_t occ_inband_read_error : 1;
    uint64_t host_inband_write_error : 1;
    uint64_t host_inband_read_error : 1;
    uint64_t emergency_throttle_set : 1;
    uint64_t edram_error : 1;
    uint64_t lru_error : 1;
    uint64_t dir_all_members_deleted : 1;
    uint64_t dir_member_deleted : 1;
    uint64_t dir_ue : 1;
    uint64_t dir_ce : 1;
    uint64_t cache_co_sue : 1;
    uint64_t cache_co_ue : 1;
    uint64_t cache_co_ce : 1;
    uint64_t cache_srw_sue : 1;
    uint64_t cache_srw_ue : 1;
    uint64_t cache_srw_ce : 1;
    uint64_t int_parity_error : 1;
    uint64_t int_buffer_sue : 1;
    uint64_t int_buffer_ue : 1;
    uint64_t int_buffer_ce : 1;
    uint64_t internal_timeout : 1;
    uint64_t external_timeout : 1;
    uint64_t invalid_address_error : 1;
    uint64_t int_protocol_error : 1;
    uint64_t host_protocol_error : 1;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbs_firact1_t;



typedef union centaur_mbscfgq {

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
    uint64_t eccbp_exit_sel : 1;
    uint64_t dram_ecc_bypass_dis : 1;
    uint64_t mbs_scom_wat_trigger : 1;
    uint64_t mbs_prq_ref_avoidance_en : 1;
    uint64_t reserved4_6 : 3;
    uint64_t occ_deadman_timer_sel : 4;
    uint64_t sync_fsync_mba_strobe_en : 1;
    uint64_t hca_timebase_op_mode : 1;
    uint64_t hca_local_timer_inc_select : 3;
    uint64_t mbs_01_rdtag_delay : 4;
    uint64_t mbs_01_rdtag_force_dead_cycle : 1;
    uint64_t sync_lat_pol_01 : 1;
    uint64_t sync_lat_adj_01 : 2;
    uint64_t mbs_23_rdtag_delay : 4;
    uint64_t mbs_23_rdtag_force_dead_cycle : 1;
    uint64_t sync_lat_pol_23 : 1;
    uint64_t sync_lat_adj_23 : 2;
    uint64_t _reserved0 : 32;
#else
    uint64_t _reserved0 : 32;
    uint64_t sync_lat_adj_23 : 2;
    uint64_t sync_lat_pol_23 : 1;
    uint64_t mbs_23_rdtag_force_dead_cycle : 1;
    uint64_t mbs_23_rdtag_delay : 4;
    uint64_t sync_lat_adj_01 : 2;
    uint64_t sync_lat_pol_01 : 1;
    uint64_t mbs_01_rdtag_force_dead_cycle : 1;
    uint64_t mbs_01_rdtag_delay : 4;
    uint64_t hca_local_timer_inc_select : 3;
    uint64_t hca_timebase_op_mode : 1;
    uint64_t sync_fsync_mba_strobe_en : 1;
    uint64_t occ_deadman_timer_sel : 4;
    uint64_t reserved4_6 : 3;
    uint64_t mbs_prq_ref_avoidance_en : 1;
    uint64_t mbs_scom_wat_trigger : 1;
    uint64_t dram_ecc_bypass_dis : 1;
    uint64_t eccbp_exit_sel : 1;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbscfgq_t;



typedef union centaur_mbsemerthroq {

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
    uint64_t emergency_throttle_ip : 1;
    uint64_t _reserved0 : 63;
#else
    uint64_t _reserved0 : 63;
    uint64_t emergency_throttle_ip : 1;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbsemerthroq_t;



typedef union centaur_mbsocc01hq {

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
    uint64_t occ_01_rd_hit : 32;
    uint64_t occ_01_wr_hit : 32;
#else
    uint64_t occ_01_wr_hit : 32;
    uint64_t occ_01_rd_hit : 32;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbsocc01hq_t;



typedef union centaur_mbsocc23hq {

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
    uint64_t occ_23_rd_hit : 32;
    uint64_t occ_23_wr_hit : 32;
#else
    uint64_t occ_23_wr_hit : 32;
    uint64_t occ_23_rd_hit : 32;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbsocc23hq_t;



typedef union centaur_mbsoccitcq {

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
    uint64_t occ_cent_idle_th_cnt : 32;
    uint64_t _reserved0 : 32;
#else
    uint64_t _reserved0 : 32;
    uint64_t occ_cent_idle_th_cnt : 32;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbsoccitcq_t;



typedef union centaur_mbsoccscanq {

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
    uint64_t occ_01_spec_can : 32;
    uint64_t occ_23_spec_can : 32;
#else
    uint64_t occ_23_spec_can : 32;
    uint64_t occ_01_spec_can : 32;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbsoccscanq_t;



typedef union centaur_mbarpc0qn {

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
    uint64_t cfg_lp2_entry_req : 1;
    uint64_t cfg_lp2_state : 1;
    uint64_t cfg_min_max_domains_enable : 1;
    uint64_t cfg_min_max_domains : 3;
    uint64_t cfg_pup_avail : 5;
    uint64_t cfg_pdn_pup : 5;
    uint64_t cfg_pup_pdn : 5;
    uint64_t reserved0 : 1;
    uint64_t cfg_min_domain_reduction_enable : 1;
    uint64_t cfg_min_domain_reduction_on_time : 10;
    uint64_t cfg_pup_after_activate_wait_enable : 1;
    uint64_t cfg_pup_after_activate_wait_time : 8;
    uint64_t cfg_force_spare_pup : 1;
    uint64_t _reserved0 : 21;
#else
    uint64_t _reserved0 : 21;
    uint64_t cfg_force_spare_pup : 1;
    uint64_t cfg_pup_after_activate_wait_time : 8;
    uint64_t cfg_pup_after_activate_wait_enable : 1;
    uint64_t cfg_min_domain_reduction_on_time : 10;
    uint64_t cfg_min_domain_reduction_enable : 1;
    uint64_t reserved0 : 1;
    uint64_t cfg_pup_pdn : 5;
    uint64_t cfg_pdn_pup : 5;
    uint64_t cfg_pup_avail : 5;
    uint64_t cfg_min_max_domains : 3;
    uint64_t cfg_min_max_domains_enable : 1;
    uint64_t cfg_lp2_state : 1;
    uint64_t cfg_lp2_entry_req : 1;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbarpc0qn_t;



typedef union centaur_mba_farb3qn {

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
    uint64_t cfg_nm_n_per_mba : 15;
    uint64_t cfg_nm_n_per_chip : 16;
    uint64_t cfg_nm_m : 14;
    uint64_t cfg_nm_ras_weight : 3;
    uint64_t cfg_nm_cas_weight : 3;
    uint64_t cfg_nm_per_slot_enabled : 1;
    uint64_t cfg_nm_count_other_mba_dis : 1;
    uint64_t _reserved0 : 11;
#else
    uint64_t _reserved0 : 11;
    uint64_t cfg_nm_count_other_mba_dis : 1;
    uint64_t cfg_nm_per_slot_enabled : 1;
    uint64_t cfg_nm_cas_weight : 3;
    uint64_t cfg_nm_ras_weight : 3;
    uint64_t cfg_nm_m : 14;
    uint64_t cfg_nm_n_per_chip : 16;
    uint64_t cfg_nm_n_per_mba : 15;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mba_farb3qn_t;



typedef union centaur_mbapcn {

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
    uint64_t mode_hp_sub_cnt : 1;
    uint64_t mode_lp_sub_cnt : 1;
    uint64_t mode_static_idle_dly : 5;
    uint64_t mode_emer_min_max_domain : 3;
    uint64_t mode_pup_all_wr_pending : 2;
    uint64_t mode_lp_ref_sim_enq : 1;
    uint64_t _reserved0 : 51;
#else
    uint64_t _reserved0 : 51;
    uint64_t mode_lp_ref_sim_enq : 1;
    uint64_t mode_pup_all_wr_pending : 2;
    uint64_t mode_emer_min_max_domain : 3;
    uint64_t mode_static_idle_dly : 5;
    uint64_t mode_lp_sub_cnt : 1;
    uint64_t mode_hp_sub_cnt : 1;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbapcn_t;



typedef union centaur_mbasrqn {

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
    uint64_t emergency_m : 14;
    uint64_t emergency_n : 15;
    uint64_t _reserved0 : 35;
#else
    uint64_t _reserved0 : 35;
    uint64_t emergency_n : 15;
    uint64_t emergency_m : 14;
#endif // _BIG_ENDIAN
    } fields;
} centaur_mbasrqn_t;



typedef union centaur_pmu0qn {

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
    uint64_t read_count : 32;
    uint64_t write_count : 32;
#else
    uint64_t write_count : 32;
    uint64_t read_count : 32;
#endif // _BIG_ENDIAN
    } fields;
} centaur_pmu0qn_t;



typedef union centaur_pmu1qn {

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
    uint64_t activate_count : 32;
    uint64_t pu_counts : 32;
#else
    uint64_t pu_counts : 32;
    uint64_t activate_count : 32;
#endif // _BIG_ENDIAN
    } fields;
} centaur_pmu1qn_t;



typedef union centaur_pmu2qn {

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
    uint64_t frame_count : 32;
    uint64_t _reserved0 : 32;
#else
    uint64_t _reserved0 : 32;
    uint64_t frame_count : 32;
#endif // _BIG_ENDIAN
    } fields;
} centaur_pmu2qn_t;



typedef union centaur_pmu3qn {

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
    uint64_t low_idle_threshold : 16;
    uint64_t med_idle_threshold : 16;
    uint64_t high_idle_threshold : 32;
#else
    uint64_t high_idle_threshold : 32;
    uint64_t med_idle_threshold : 16;
    uint64_t low_idle_threshold : 16;
#endif // _BIG_ENDIAN
    } fields;
} centaur_pmu3qn_t;



typedef union centaur_pmu4qn {

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
    uint64_t base_idle_count : 32;
    uint64_t low_idle_count : 32;
#else
    uint64_t low_idle_count : 32;
    uint64_t base_idle_count : 32;
#endif // _BIG_ENDIAN
    } fields;
} centaur_pmu4qn_t;



typedef union centaur_pmu5qn {

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
    uint64_t med_idle_count : 32;
    uint64_t high_idle_count : 32;
#else
    uint64_t high_idle_count : 32;
    uint64_t med_idle_count : 32;
#endif // _BIG_ENDIAN
    } fields;
} centaur_pmu5qn_t;



typedef union centaur_pmu6qn {

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
    uint64_t total_gap_counts : 18;
    uint64_t specific_gap_counts : 18;
    uint64_t gap_length_adder : 3;
    uint64_t specific_gap_condition : 4;
    uint64_t cmd_to_cmd_count : 18;
    uint64_t command_pattern_to_count : 3;
#else
    uint64_t command_pattern_to_count : 3;
    uint64_t cmd_to_cmd_count : 18;
    uint64_t specific_gap_condition : 4;
    uint64_t gap_length_adder : 3;
    uint64_t specific_gap_counts : 18;
    uint64_t total_gap_counts : 18;
#endif // _BIG_ENDIAN
    } fields;
} centaur_pmu6qn_t;



typedef union centaur_sensor_cache_data0_3 {

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
    uint64_t crittrip0 : 1;
    uint64_t abovetrip0 : 1;
    uint64_t belowtrip0 : 1;
    uint64_t signbit0 : 1;
    uint64_t temperature0 : 8;
    uint64_t temp_frac0 : 2;
    uint64_t status0 : 2;
    uint64_t crittrip1 : 1;
    uint64_t abovetrip1 : 1;
    uint64_t belowtrip1 : 1;
    uint64_t signbit1 : 1;
    uint64_t temperature1 : 8;
    uint64_t temp_frac1 : 2;
    uint64_t status1 : 2;
    uint64_t crittrip2 : 1;
    uint64_t abovetrip2 : 1;
    uint64_t belowtrip2 : 1;
    uint64_t signbit2 : 1;
    uint64_t temperature2 : 8;
    uint64_t temp_frac2 : 2;
    uint64_t status2 : 2;
    uint64_t crittrip3 : 1;
    uint64_t abovetrip3 : 1;
    uint64_t belowtrip3 : 1;
    uint64_t signbit3 : 1;
    uint64_t temperature3 : 8;
    uint64_t temp_frac3 : 2;
    uint64_t status3 : 2;
#else
    uint64_t status3 : 2;
    uint64_t temp_frac3 : 2;
    uint64_t temperature3 : 8;
    uint64_t signbit3 : 1;
    uint64_t belowtrip3 : 1;
    uint64_t abovetrip3 : 1;
    uint64_t crittrip3 : 1;
    uint64_t status2 : 2;
    uint64_t temp_frac2 : 2;
    uint64_t temperature2 : 8;
    uint64_t signbit2 : 1;
    uint64_t belowtrip2 : 1;
    uint64_t abovetrip2 : 1;
    uint64_t crittrip2 : 1;
    uint64_t status1 : 2;
    uint64_t temp_frac1 : 2;
    uint64_t temperature1 : 8;
    uint64_t signbit1 : 1;
    uint64_t belowtrip1 : 1;
    uint64_t abovetrip1 : 1;
    uint64_t crittrip1 : 1;
    uint64_t status0 : 2;
    uint64_t temp_frac0 : 2;
    uint64_t temperature0 : 8;
    uint64_t signbit0 : 1;
    uint64_t belowtrip0 : 1;
    uint64_t abovetrip0 : 1;
    uint64_t crittrip0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} centaur_sensor_cache_data0_3_t;



typedef union centaur_sensor_cache_data4_7 {

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
    uint64_t crittrip4 : 1;
    uint64_t abovetrip4 : 1;
    uint64_t belowtrip4 : 1;
    uint64_t signbit4 : 1;
    uint64_t temperature4 : 8;
    uint64_t temp_frac4 : 2;
    uint64_t status4 : 2;
    uint64_t crittrip5 : 1;
    uint64_t abovetrip5 : 1;
    uint64_t belowtrip5 : 1;
    uint64_t signbit5 : 1;
    uint64_t temperature5 : 8;
    uint64_t temp_frac5 : 2;
    uint64_t status5 : 2;
    uint64_t crittrip6 : 1;
    uint64_t abovetrip6 : 1;
    uint64_t belowtrip6 : 1;
    uint64_t signbit6 : 1;
    uint64_t temperature6 : 8;
    uint64_t temp_frac6 : 2;
    uint64_t status6 : 2;
    uint64_t crittrip7 : 1;
    uint64_t abovetrip7 : 1;
    uint64_t belowtrip7 : 1;
    uint64_t signbit7 : 1;
    uint64_t temperature7 : 8;
    uint64_t temp_frac7 : 2;
    uint64_t status7 : 2;
#else
    uint64_t status7 : 2;
    uint64_t temp_frac7 : 2;
    uint64_t temperature7 : 8;
    uint64_t signbit7 : 1;
    uint64_t belowtrip7 : 1;
    uint64_t abovetrip7 : 1;
    uint64_t crittrip7 : 1;
    uint64_t status6 : 2;
    uint64_t temp_frac6 : 2;
    uint64_t temperature6 : 8;
    uint64_t signbit6 : 1;
    uint64_t belowtrip6 : 1;
    uint64_t abovetrip6 : 1;
    uint64_t crittrip6 : 1;
    uint64_t status5 : 2;
    uint64_t temp_frac5 : 2;
    uint64_t temperature5 : 8;
    uint64_t signbit5 : 1;
    uint64_t belowtrip5 : 1;
    uint64_t abovetrip5 : 1;
    uint64_t crittrip5 : 1;
    uint64_t status4 : 2;
    uint64_t temp_frac4 : 2;
    uint64_t temperature4 : 8;
    uint64_t signbit4 : 1;
    uint64_t belowtrip4 : 1;
    uint64_t abovetrip4 : 1;
    uint64_t crittrip4 : 1;
#endif // _BIG_ENDIAN
    } fields;
} centaur_sensor_cache_data4_7_t;



typedef union centaur_dts_thermal_sensor_results {

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
    uint64_t dts0 : 12;
    uint64_t thermal_trip0 : 2;
    uint64_t spare0 : 1;
    uint64_t valid0 : 1;
    uint64_t dts1 : 12;
    uint64_t thermal_trip1 : 2;
    uint64_t spare1 : 1;
    uint64_t valid1 : 1;
    uint64_t _reserved0 : 32;
#else
    uint64_t _reserved0 : 32;
    uint64_t valid1 : 1;
    uint64_t spare1 : 1;
    uint64_t thermal_trip1 : 2;
    uint64_t dts1 : 12;
    uint64_t valid0 : 1;
    uint64_t spare0 : 1;
    uint64_t thermal_trip0 : 2;
    uint64_t dts0 : 12;
#endif // _BIG_ENDIAN
    } fields;
} centaur_dts_thermal_sensor_results_t;


#endif // __ASSEMBLER__
#endif // __CENTAUR_FIRMWARE_REGISTERS_H__

