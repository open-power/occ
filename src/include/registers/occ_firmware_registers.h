/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/occ_firmware_registers.h $              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2019                             */
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
#ifndef __OCC_FIRMWARE_REGISTERS_H__
#define __OCC_FIRMWARE_REGISTERS_H__

/// \file occ_firmware_registers.h
/// \brief C register structs for the OCC unit

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




typedef union occ_scom_occlfir
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
        uint64_t occ_fw0 : 1;
        uint64_t occ_fw1 : 1;
        uint64_t qme_error_notify : 1;
        uint64_t stop_recovery_notify_prd : 1;
        uint64_t occ_hb_error : 1;
        uint64_t gpe0_watchdog_timeout : 1;
        uint64_t gpe1_watchdog_timeout : 1;
        uint64_t gpe2_watchdog_timeout : 1;
        uint64_t gpe3_watchdog_timeout : 1;
        uint64_t gpe0_error : 1;
        uint64_t gpe1_error : 1;
        uint64_t gpe2_error : 1;
        uint64_t gpe3_error : 1;
        uint64_t ocb_error : 1;
        uint64_t srt_ue : 1;
        uint64_t srt_ce : 1;
        uint64_t gpe0_halted : 1;
        uint64_t gpe1_halted : 1;
        uint64_t gpe2_halted : 1;
        uint64_t gpe3_halted : 1;
        uint64_t gpe0_write_protect_error : 1;
        uint64_t gpe1_write_protect_error : 1;
        uint64_t gpe2_write_protect_error : 1;
        uint64_t gpe3_write_protect_error : 1;
        uint64_t spare_24_25 : 2;
        uint64_t external_trap : 1;
        uint64_t ppc405_core_reset : 1;
        uint64_t ppc405_chip_reset : 1;
        uint64_t ppc405_system_reset : 1;
        uint64_t ppc405_dbgmsrwe : 1;
        uint64_t ppc405_dbgstopack : 1;
        uint64_t ocb_db_error : 1;
        uint64_t ocb_pib_addr_parity_err : 1;
        uint64_t ocb_idc_error : 1;
        uint64_t opit_parity_error : 1;
        uint64_t spare_36_41 : 6;
        uint64_t jtagacc_err : 1;
        uint64_t ocb_oci_ocislv_err : 1;
        uint64_t c405_ecc_ue : 1;
        uint64_t c405_ecc_ce : 1;
        uint64_t c405_oci_machinecheck : 1;
        uint64_t sram_spare_direct_error : 1;
        uint64_t srt_other_error : 1;
        uint64_t spare_49_50 : 2;
        uint64_t gpe0_ocislv_err : 1;
        uint64_t gpe1_ocislv_err : 1;
        uint64_t gpe2_ocislv_err : 1;
        uint64_t gpe3_ocislv_err : 1;
        uint64_t c405icu_m_timeout : 1;
        uint64_t c405dcu_m_timeout : 1;
        uint64_t occ_complex_fault : 1;
        uint64_t occ_complex_notify : 1;
        uint64_t spare_59_61 : 3;
        uint64_t reserved1 : 2;
#else
        uint64_t reserved1 : 2;
        uint64_t spare_59_61 : 3;
        uint64_t occ_complex_notify : 1;
        uint64_t occ_complex_fault : 1;
        uint64_t c405dcu_m_timeout : 1;
        uint64_t c405icu_m_timeout : 1;
        uint64_t gpe3_ocislv_err : 1;
        uint64_t gpe2_ocislv_err : 1;
        uint64_t gpe1_ocislv_err : 1;
        uint64_t gpe0_ocislv_err : 1;
        uint64_t spare_49_50 : 2;
        uint64_t srt_other_error : 1;
        uint64_t sram_spare_direct_error : 1;
        uint64_t c405_oci_machinecheck : 1;
        uint64_t c405_ecc_ce : 1;
        uint64_t c405_ecc_ue : 1;
        uint64_t ocb_oci_ocislv_err : 1;
        uint64_t jtagacc_err : 1;
        uint64_t spare_36_41 : 6;
        uint64_t opit_parity_error : 1;
        uint64_t ocb_idc_error : 1;
        uint64_t ocb_pib_addr_parity_err : 1;
        uint64_t ocb_db_error : 1;
        uint64_t ppc405_dbgstopack : 1;
        uint64_t ppc405_dbgmsrwe : 1;
        uint64_t ppc405_system_reset : 1;
        uint64_t ppc405_chip_reset : 1;
        uint64_t ppc405_core_reset : 1;
        uint64_t external_trap : 1;
        uint64_t spare_24_25 : 2;
        uint64_t gpe3_write_protect_error : 1;
        uint64_t gpe2_write_protect_error : 1;
        uint64_t gpe1_write_protect_error : 1;
        uint64_t gpe0_write_protect_error : 1;
        uint64_t gpe3_halted : 1;
        uint64_t gpe2_halted : 1;
        uint64_t gpe1_halted : 1;
        uint64_t gpe0_halted : 1;
        uint64_t srt_ce : 1;
        uint64_t srt_ue : 1;
        uint64_t ocb_error : 1;
        uint64_t gpe3_error : 1;
        uint64_t gpe2_error : 1;
        uint64_t gpe1_error : 1;
        uint64_t gpe0_error : 1;
        uint64_t gpe3_watchdog_timeout : 1;
        uint64_t gpe2_watchdog_timeout : 1;
        uint64_t gpe1_watchdog_timeout : 1;
        uint64_t gpe0_watchdog_timeout : 1;
        uint64_t occ_hb_error : 1;
        uint64_t stop_recovery_notify_prd : 1;
        uint64_t qme_error_notify : 1;
        uint64_t occ_fw1 : 1;
        uint64_t occ_fw0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} occ_scom_occlfir_t;



typedef union occ_scom_occlfir_and
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
        uint64_t occ_fw0 : 1;
        uint64_t occ_fw1 : 1;
        uint64_t qme_error_notify : 1;
        uint64_t stop_recovery_notify_prd : 1;
        uint64_t occ_hb_error : 1;
        uint64_t gpe0_watchdog_timeout : 1;
        uint64_t gpe1_watchdog_timeout : 1;
        uint64_t gpe2_watchdog_timeout : 1;
        uint64_t gpe3_watchdog_timeout : 1;
        uint64_t gpe0_error : 1;
        uint64_t gpe1_error : 1;
        uint64_t gpe2_error : 1;
        uint64_t gpe3_error : 1;
        uint64_t ocb_error : 1;
        uint64_t srt_ue : 1;
        uint64_t srt_ce : 1;
        uint64_t gpe0_halted : 1;
        uint64_t gpe1_halted : 1;
        uint64_t gpe2_halted : 1;
        uint64_t gpe3_halted : 1;
        uint64_t gpe0_write_protect_error : 1;
        uint64_t gpe1_write_protect_error : 1;
        uint64_t gpe2_write_protect_error : 1;
        uint64_t gpe3_write_protect_error : 1;
        uint64_t spare_24_25 : 2;
        uint64_t external_trap : 1;
        uint64_t ppc405_core_reset : 1;
        uint64_t ppc405_chip_reset : 1;
        uint64_t ppc405_system_reset : 1;
        uint64_t ppc405_dbgmsrwe : 1;
        uint64_t ppc405_dbgstopack : 1;
        uint64_t ocb_db_error : 1;
        uint64_t ocb_pib_addr_parity_err : 1;
        uint64_t ocb_idc_error : 1;
        uint64_t opit_parity_error : 1;
        uint64_t spare_36_41 : 6;
        uint64_t jtagacc_err : 1;
        uint64_t ocb_oci_ocislv_err : 1;
        uint64_t c405_ecc_ue : 1;
        uint64_t c405_ecc_ce : 1;
        uint64_t c405_oci_machinecheck : 1;
        uint64_t sram_spare_direct_error : 1;
        uint64_t srt_other_error : 1;
        uint64_t spare_49_50 : 2;
        uint64_t gpe0_ocislv_err : 1;
        uint64_t gpe1_ocislv_err : 1;
        uint64_t gpe2_ocislv_err : 1;
        uint64_t gpe3_ocislv_err : 1;
        uint64_t c405icu_m_timeout : 1;
        uint64_t c405dcu_m_timeout : 1;
        uint64_t occ_complex_fault : 1;
        uint64_t occ_complex_notify : 1;
        uint64_t spare_59_61 : 3;
        uint64_t reserved1 : 2;
#else
        uint64_t reserved1 : 2;
        uint64_t spare_59_61 : 3;
        uint64_t occ_complex_notify : 1;
        uint64_t occ_complex_fault : 1;
        uint64_t c405dcu_m_timeout : 1;
        uint64_t c405icu_m_timeout : 1;
        uint64_t gpe3_ocislv_err : 1;
        uint64_t gpe2_ocislv_err : 1;
        uint64_t gpe1_ocislv_err : 1;
        uint64_t gpe0_ocislv_err : 1;
        uint64_t spare_49_50 : 2;
        uint64_t srt_other_error : 1;
        uint64_t sram_spare_direct_error : 1;
        uint64_t c405_oci_machinecheck : 1;
        uint64_t c405_ecc_ce : 1;
        uint64_t c405_ecc_ue : 1;
        uint64_t ocb_oci_ocislv_err : 1;
        uint64_t jtagacc_err : 1;
        uint64_t spare_36_41 : 6;
        uint64_t opit_parity_error : 1;
        uint64_t ocb_idc_error : 1;
        uint64_t ocb_pib_addr_parity_err : 1;
        uint64_t ocb_db_error : 1;
        uint64_t ppc405_dbgstopack : 1;
        uint64_t ppc405_dbgmsrwe : 1;
        uint64_t ppc405_system_reset : 1;
        uint64_t ppc405_chip_reset : 1;
        uint64_t ppc405_core_reset : 1;
        uint64_t external_trap : 1;
        uint64_t spare_24_25 : 2;
        uint64_t gpe3_write_protect_error : 1;
        uint64_t gpe2_write_protect_error : 1;
        uint64_t gpe1_write_protect_error : 1;
        uint64_t gpe0_write_protect_error : 1;
        uint64_t gpe3_halted : 1;
        uint64_t gpe2_halted : 1;
        uint64_t gpe1_halted : 1;
        uint64_t gpe0_halted : 1;
        uint64_t srt_ce : 1;
        uint64_t srt_ue : 1;
        uint64_t ocb_error : 1;
        uint64_t gpe3_error : 1;
        uint64_t gpe2_error : 1;
        uint64_t gpe1_error : 1;
        uint64_t gpe0_error : 1;
        uint64_t gpe3_watchdog_timeout : 1;
        uint64_t gpe2_watchdog_timeout : 1;
        uint64_t gpe1_watchdog_timeout : 1;
        uint64_t gpe0_watchdog_timeout : 1;
        uint64_t occ_hb_error : 1;
        uint64_t stop_recovery_notify_prd : 1;
        uint64_t qme_error_notify : 1;
        uint64_t occ_fw1 : 1;
        uint64_t occ_fw0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} occ_scom_occlfir_and_t;



typedef union occ_scom_occlfir_or
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
        uint64_t occ_fw0 : 1;
        uint64_t occ_fw1 : 1;
        uint64_t qme_error_notify : 1;
        uint64_t stop_recovery_notify_prd : 1;
        uint64_t occ_hb_error : 1;
        uint64_t gpe0_watchdog_timeout : 1;
        uint64_t gpe1_watchdog_timeout : 1;
        uint64_t gpe2_watchdog_timeout : 1;
        uint64_t gpe3_watchdog_timeout : 1;
        uint64_t gpe0_error : 1;
        uint64_t gpe1_error : 1;
        uint64_t gpe2_error : 1;
        uint64_t gpe3_error : 1;
        uint64_t ocb_error : 1;
        uint64_t srt_ue : 1;
        uint64_t srt_ce : 1;
        uint64_t gpe0_halted : 1;
        uint64_t gpe1_halted : 1;
        uint64_t gpe2_halted : 1;
        uint64_t gpe3_halted : 1;
        uint64_t gpe0_write_protect_error : 1;
        uint64_t gpe1_write_protect_error : 1;
        uint64_t gpe2_write_protect_error : 1;
        uint64_t gpe3_write_protect_error : 1;
        uint64_t spare_24_25 : 2;
        uint64_t external_trap : 1;
        uint64_t ppc405_core_reset : 1;
        uint64_t ppc405_chip_reset : 1;
        uint64_t ppc405_system_reset : 1;
        uint64_t ppc405_dbgmsrwe : 1;
        uint64_t ppc405_dbgstopack : 1;
        uint64_t ocb_db_error : 1;
        uint64_t ocb_pib_addr_parity_err : 1;
        uint64_t ocb_idc_error : 1;
        uint64_t opit_parity_error : 1;
        uint64_t spare_36_41 : 6;
        uint64_t jtagacc_err : 1;
        uint64_t ocb_oci_ocislv_err : 1;
        uint64_t c405_ecc_ue : 1;
        uint64_t c405_ecc_ce : 1;
        uint64_t c405_oci_machinecheck : 1;
        uint64_t sram_spare_direct_error : 1;
        uint64_t srt_other_error : 1;
        uint64_t spare_49_50 : 2;
        uint64_t gpe0_ocislv_err : 1;
        uint64_t gpe1_ocislv_err : 1;
        uint64_t gpe2_ocislv_err : 1;
        uint64_t gpe3_ocislv_err : 1;
        uint64_t c405icu_m_timeout : 1;
        uint64_t c405dcu_m_timeout : 1;
        uint64_t occ_complex_fault : 1;
        uint64_t occ_complex_notify : 1;
        uint64_t spare_59_61 : 3;
        uint64_t reserved1 : 2;
#else
        uint64_t reserved1 : 2;
        uint64_t spare_59_61 : 3;
        uint64_t occ_complex_notify : 1;
        uint64_t occ_complex_fault : 1;
        uint64_t c405dcu_m_timeout : 1;
        uint64_t c405icu_m_timeout : 1;
        uint64_t gpe3_ocislv_err : 1;
        uint64_t gpe2_ocislv_err : 1;
        uint64_t gpe1_ocislv_err : 1;
        uint64_t gpe0_ocislv_err : 1;
        uint64_t spare_49_50 : 2;
        uint64_t srt_other_error : 1;
        uint64_t sram_spare_direct_error : 1;
        uint64_t c405_oci_machinecheck : 1;
        uint64_t c405_ecc_ce : 1;
        uint64_t c405_ecc_ue : 1;
        uint64_t ocb_oci_ocislv_err : 1;
        uint64_t jtagacc_err : 1;
        uint64_t spare_36_41 : 6;
        uint64_t opit_parity_error : 1;
        uint64_t ocb_idc_error : 1;
        uint64_t ocb_pib_addr_parity_err : 1;
        uint64_t ocb_db_error : 1;
        uint64_t ppc405_dbgstopack : 1;
        uint64_t ppc405_dbgmsrwe : 1;
        uint64_t ppc405_system_reset : 1;
        uint64_t ppc405_chip_reset : 1;
        uint64_t ppc405_core_reset : 1;
        uint64_t external_trap : 1;
        uint64_t spare_24_25 : 2;
        uint64_t gpe3_write_protect_error : 1;
        uint64_t gpe2_write_protect_error : 1;
        uint64_t gpe1_write_protect_error : 1;
        uint64_t gpe0_write_protect_error : 1;
        uint64_t gpe3_halted : 1;
        uint64_t gpe2_halted : 1;
        uint64_t gpe1_halted : 1;
        uint64_t gpe0_halted : 1;
        uint64_t srt_ce : 1;
        uint64_t srt_ue : 1;
        uint64_t ocb_error : 1;
        uint64_t gpe3_error : 1;
        uint64_t gpe2_error : 1;
        uint64_t gpe1_error : 1;
        uint64_t gpe0_error : 1;
        uint64_t gpe3_watchdog_timeout : 1;
        uint64_t gpe2_watchdog_timeout : 1;
        uint64_t gpe1_watchdog_timeout : 1;
        uint64_t gpe0_watchdog_timeout : 1;
        uint64_t occ_hb_error : 1;
        uint64_t stop_recovery_notify_prd : 1;
        uint64_t qme_error_notify : 1;
        uint64_t occ_fw1 : 1;
        uint64_t occ_fw0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} occ_scom_occlfir_or_t;



typedef union occ_scom_occlfirmask
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
        uint64_t occ_fw0_mask : 1;
        uint64_t occ_fw1_mask : 1;
        uint64_t qme_error_mask : 1;
        uint64_t stop_recovery_notify_prd_mask : 1;
        uint64_t occ_hb_error_mask : 1;
        uint64_t gpe0_watchdog_timeout_mask : 1;
        uint64_t gpe1_watchdog_timeout_mask : 1;
        uint64_t gpe2_watchdog_timeout_mask : 1;
        uint64_t gpe3_watchdog_timeout_mask : 1;
        uint64_t gpe0_error_mask : 1;
        uint64_t gpe1_error_mask : 1;
        uint64_t gpe2_error_mask : 1;
        uint64_t gpe3_error_mask : 1;
        uint64_t ocb_error_mask : 1;
        uint64_t srt_ue_mask : 1;
        uint64_t srt_ce_mask : 1;
        uint64_t gpe0_halted_mask : 1;
        uint64_t gpe1_halted_mask : 1;
        uint64_t gpe2_halted_mask : 1;
        uint64_t gpe3_halted_mask : 1;
        uint64_t gpe0_write_protect_error_mask : 1;
        uint64_t gpe1_write_protect_error_mask : 1;
        uint64_t gpe2_write_protect_error_mask : 1;
        uint64_t gpe3_write_protect_error_mask : 1;
        uint64_t spare_24_25_mask : 2;
        uint64_t external_trap_mask : 1;
        uint64_t ppc405_core_reset_mask : 1;
        uint64_t ppc405_chip_reset_mask : 1;
        uint64_t ppc405_system_reset_mask : 1;
        uint64_t ppc405_dbgmsrwe_mask : 1;
        uint64_t ppc405_dbgstopack_mask : 1;
        uint64_t ocb_db_error_mask : 1;
        uint64_t ocb_pib_addr_parity_err_mask : 1;
        uint64_t ocb_idc_error_mask : 1;
        uint64_t spare_35_41_mask : 7;
        uint64_t jtagacc_err_mask : 1;
        uint64_t spare_err_38_mask : 1;
        uint64_t c405_ecc_ue_mask : 1;
        uint64_t c405_ecc_ce_mask : 1;
        uint64_t c405_oci_machinecheck_mask : 1;
        uint64_t sram_spare_direct_error_mask : 1;
        uint64_t srt_other_error_mask : 1;
        uint64_t spare_49_50_mask : 2;
        uint64_t gpe0_ocislv_err_mask : 1;
        uint64_t gpe1_ocislv_err_mask : 1;
        uint64_t gpe2_ocislv_err_mask : 1;
        uint64_t gpe3_ocislv_err_mask : 1;
        uint64_t c405icu_m_timeout_mask : 1;
        uint64_t c405dcu_m_timeout_mask : 1;
        uint64_t occ_complex_fault_mask : 1;
        uint64_t occ_complex_notify_mask : 1;
        uint64_t spare_59_61_mask : 3;
        uint64_t reserved1 : 2;
#else
        uint64_t reserved1 : 2;
        uint64_t spare_59_61_mask : 3;
        uint64_t occ_complex_notify_mask : 1;
        uint64_t occ_complex_fault_mask : 1;
        uint64_t c405dcu_m_timeout_mask : 1;
        uint64_t c405icu_m_timeout_mask : 1;
        uint64_t gpe3_ocislv_err_mask : 1;
        uint64_t gpe2_ocislv_err_mask : 1;
        uint64_t gpe1_ocislv_err_mask : 1;
        uint64_t gpe0_ocislv_err_mask : 1;
        uint64_t spare_49_50_mask : 2;
        uint64_t srt_other_error_mask : 1;
        uint64_t sram_spare_direct_error_mask : 1;
        uint64_t c405_oci_machinecheck_mask : 1;
        uint64_t c405_ecc_ce_mask : 1;
        uint64_t c405_ecc_ue_mask : 1;
        uint64_t spare_err_38_mask : 1;
        uint64_t jtagacc_err_mask : 1;
        uint64_t spare_35_41_mask : 7;
        uint64_t ocb_idc_error_mask : 1;
        uint64_t ocb_pib_addr_parity_err_mask : 1;
        uint64_t ocb_db_error_mask : 1;
        uint64_t ppc405_dbgstopack_mask : 1;
        uint64_t ppc405_dbgmsrwe_mask : 1;
        uint64_t ppc405_system_reset_mask : 1;
        uint64_t ppc405_chip_reset_mask : 1;
        uint64_t ppc405_core_reset_mask : 1;
        uint64_t external_trap_mask : 1;
        uint64_t spare_24_25_mask : 2;
        uint64_t gpe3_write_protect_error_mask : 1;
        uint64_t gpe2_write_protect_error_mask : 1;
        uint64_t gpe1_write_protect_error_mask : 1;
        uint64_t gpe0_write_protect_error_mask : 1;
        uint64_t gpe3_halted_mask : 1;
        uint64_t gpe2_halted_mask : 1;
        uint64_t gpe1_halted_mask : 1;
        uint64_t gpe0_halted_mask : 1;
        uint64_t srt_ce_mask : 1;
        uint64_t srt_ue_mask : 1;
        uint64_t ocb_error_mask : 1;
        uint64_t gpe3_error_mask : 1;
        uint64_t gpe2_error_mask : 1;
        uint64_t gpe1_error_mask : 1;
        uint64_t gpe0_error_mask : 1;
        uint64_t gpe3_watchdog_timeout_mask : 1;
        uint64_t gpe2_watchdog_timeout_mask : 1;
        uint64_t gpe1_watchdog_timeout_mask : 1;
        uint64_t gpe0_watchdog_timeout_mask : 1;
        uint64_t occ_hb_error_mask : 1;
        uint64_t stop_recovery_notify_prd_mask : 1;
        uint64_t qme_error_mask : 1;
        uint64_t occ_fw1_mask : 1;
        uint64_t occ_fw0_mask : 1;
#endif // _BIG_ENDIAN
    } fields;
} occ_scom_occlfirmask_t;



typedef union occ_scom_occlfirmask_and
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
        uint64_t occ_fw0_mask : 1;
        uint64_t occ_fw1_mask : 1;
        uint64_t qme_error_mask : 1;
        uint64_t stop_recovery_notify_prd_mask : 1;
        uint64_t occ_hb_error_mask : 1;
        uint64_t gpe0_watchdog_timeout_mask : 1;
        uint64_t gpe1_watchdog_timeout_mask : 1;
        uint64_t gpe2_watchdog_timeout_mask : 1;
        uint64_t gpe3_watchdog_timeout_mask : 1;
        uint64_t gpe0_error_mask : 1;
        uint64_t gpe1_error_mask : 1;
        uint64_t gpe2_error_mask : 1;
        uint64_t gpe3_error_mask : 1;
        uint64_t ocb_error_mask : 1;
        uint64_t srt_ue_mask : 1;
        uint64_t srt_ce_mask : 1;
        uint64_t gpe0_halted_mask : 1;
        uint64_t gpe1_halted_mask : 1;
        uint64_t gpe2_halted_mask : 1;
        uint64_t gpe3_halted_mask : 1;
        uint64_t gpe0_write_protect_error_mask : 1;
        uint64_t gpe1_write_protect_error_mask : 1;
        uint64_t gpe2_write_protect_error_mask : 1;
        uint64_t gpe3_write_protect_error_mask : 1;
        uint64_t spare_24_25_mask : 2;
        uint64_t external_trap_mask : 1;
        uint64_t ppc405_core_reset_mask : 1;
        uint64_t ppc405_chip_reset_mask : 1;
        uint64_t ppc405_system_reset_mask : 1;
        uint64_t ppc405_dbgmsrwe_mask : 1;
        uint64_t ppc405_dbgstopack_mask : 1;
        uint64_t ocb_db_error_mask : 1;
        uint64_t ocb_pib_addr_parity_err_mask : 1;
        uint64_t ocb_idc_error_mask : 1;
        uint64_t spare_35_41_mask : 7;
        uint64_t jtagacc_err_mask : 1;
        uint64_t spare_err_38_mask : 1;
        uint64_t c405_ecc_ue_mask : 1;
        uint64_t c405_ecc_ce_mask : 1;
        uint64_t c405_oci_machinecheck_mask : 1;
        uint64_t sram_spare_direct_error_mask : 1;
        uint64_t srt_other_error_mask : 1;
        uint64_t spare_49_50_mask : 2;
        uint64_t gpe0_ocislv_err_mask : 1;
        uint64_t gpe1_ocislv_err_mask : 1;
        uint64_t gpe2_ocislv_err_mask : 1;
        uint64_t gpe3_ocislv_err_mask : 1;
        uint64_t c405icu_m_timeout_mask : 1;
        uint64_t c405dcu_m_timeout_mask : 1;
        uint64_t occ_complex_fault_mask : 1;
        uint64_t occ_complex_notify_mask : 1;
        uint64_t spare_59_61_mask : 3;
        uint64_t reserved1 : 2;
#else
        uint64_t reserved1 : 2;
        uint64_t spare_59_61_mask : 3;
        uint64_t occ_complex_notify_mask : 1;
        uint64_t occ_complex_fault_mask : 1;
        uint64_t c405dcu_m_timeout_mask : 1;
        uint64_t c405icu_m_timeout_mask : 1;
        uint64_t gpe3_ocislv_err_mask : 1;
        uint64_t gpe2_ocislv_err_mask : 1;
        uint64_t gpe1_ocislv_err_mask : 1;
        uint64_t gpe0_ocislv_err_mask : 1;
        uint64_t spare_49_50_mask : 2;
        uint64_t srt_other_error_mask : 1;
        uint64_t sram_spare_direct_error_mask : 1;
        uint64_t c405_oci_machinecheck_mask : 1;
        uint64_t c405_ecc_ce_mask : 1;
        uint64_t c405_ecc_ue_mask : 1;
        uint64_t spare_err_38_mask : 1;
        uint64_t jtagacc_err_mask : 1;
        uint64_t spare_35_41_mask : 7;
        uint64_t ocb_idc_error_mask : 1;
        uint64_t ocb_pib_addr_parity_err_mask : 1;
        uint64_t ocb_db_error_mask : 1;
        uint64_t ppc405_dbgstopack_mask : 1;
        uint64_t ppc405_dbgmsrwe_mask : 1;
        uint64_t ppc405_system_reset_mask : 1;
        uint64_t ppc405_chip_reset_mask : 1;
        uint64_t ppc405_core_reset_mask : 1;
        uint64_t external_trap_mask : 1;
        uint64_t spare_24_25_mask : 2;
        uint64_t gpe3_write_protect_error_mask : 1;
        uint64_t gpe2_write_protect_error_mask : 1;
        uint64_t gpe1_write_protect_error_mask : 1;
        uint64_t gpe0_write_protect_error_mask : 1;
        uint64_t gpe3_halted_mask : 1;
        uint64_t gpe2_halted_mask : 1;
        uint64_t gpe1_halted_mask : 1;
        uint64_t gpe0_halted_mask : 1;
        uint64_t srt_ce_mask : 1;
        uint64_t srt_ue_mask : 1;
        uint64_t ocb_error_mask : 1;
        uint64_t gpe3_error_mask : 1;
        uint64_t gpe2_error_mask : 1;
        uint64_t gpe1_error_mask : 1;
        uint64_t gpe0_error_mask : 1;
        uint64_t gpe3_watchdog_timeout_mask : 1;
        uint64_t gpe2_watchdog_timeout_mask : 1;
        uint64_t gpe1_watchdog_timeout_mask : 1;
        uint64_t gpe0_watchdog_timeout_mask : 1;
        uint64_t occ_hb_error_mask : 1;
        uint64_t stop_recovery_notify_prd_mask : 1;
        uint64_t qme_error_mask : 1;
        uint64_t occ_fw1_mask : 1;
        uint64_t occ_fw0_mask : 1;
#endif // _BIG_ENDIAN
    } fields;
} occ_scom_occlfirmask_and_t;



typedef union occ_scom_occlfirmask_or
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
        uint64_t occ_fw0_mask : 1;
        uint64_t occ_fw1_mask : 1;
        uint64_t qme_error_mask : 1;
        uint64_t stop_recovery_notify_prd_mask : 1;
        uint64_t occ_hb_error_mask : 1;
        uint64_t gpe0_watchdog_timeout_mask : 1;
        uint64_t gpe1_watchdog_timeout_mask : 1;
        uint64_t gpe2_watchdog_timeout_mask : 1;
        uint64_t gpe3_watchdog_timeout_mask : 1;
        uint64_t gpe0_error_mask : 1;
        uint64_t gpe1_error_mask : 1;
        uint64_t gpe2_error_mask : 1;
        uint64_t gpe3_error_mask : 1;
        uint64_t ocb_error_mask : 1;
        uint64_t srt_ue_mask : 1;
        uint64_t srt_ce_mask : 1;
        uint64_t gpe0_halted_mask : 1;
        uint64_t gpe1_halted_mask : 1;
        uint64_t gpe2_halted_mask : 1;
        uint64_t gpe3_halted_mask : 1;
        uint64_t gpe0_write_protect_error_mask : 1;
        uint64_t gpe1_write_protect_error_mask : 1;
        uint64_t gpe2_write_protect_error_mask : 1;
        uint64_t gpe3_write_protect_error_mask : 1;
        uint64_t spare_24_25_mask : 2;
        uint64_t external_trap_mask : 1;
        uint64_t ppc405_core_reset_mask : 1;
        uint64_t ppc405_chip_reset_mask : 1;
        uint64_t ppc405_system_reset_mask : 1;
        uint64_t ppc405_dbgmsrwe_mask : 1;
        uint64_t ppc405_dbgstopack_mask : 1;
        uint64_t ocb_db_error_mask : 1;
        uint64_t ocb_pib_addr_parity_err_mask : 1;
        uint64_t ocb_idc_error_mask : 1;
        uint64_t spare_35_41_mask : 7;
        uint64_t jtagacc_err_mask : 1;
        uint64_t spare_err_38_mask : 1;
        uint64_t c405_ecc_ue_mask : 1;
        uint64_t c405_ecc_ce_mask : 1;
        uint64_t c405_oci_machinecheck_mask : 1;
        uint64_t sram_spare_direct_error_mask : 1;
        uint64_t srt_other_error_mask : 1;
        uint64_t spare_49_50_mask : 2;
        uint64_t gpe0_ocislv_err_mask : 1;
        uint64_t gpe1_ocislv_err_mask : 1;
        uint64_t gpe2_ocislv_err_mask : 1;
        uint64_t gpe3_ocislv_err_mask : 1;
        uint64_t c405icu_m_timeout_mask : 1;
        uint64_t c405dcu_m_timeout_mask : 1;
        uint64_t occ_complex_fault_mask : 1;
        uint64_t occ_complex_notify_mask : 1;
        uint64_t spare_59_61_mask : 3;
        uint64_t reserved1 : 2;
#else
        uint64_t reserved1 : 2;
        uint64_t spare_59_61_mask : 3;
        uint64_t occ_complex_notify_mask : 1;
        uint64_t occ_complex_fault_mask : 1;
        uint64_t c405dcu_m_timeout_mask : 1;
        uint64_t c405icu_m_timeout_mask : 1;
        uint64_t gpe3_ocislv_err_mask : 1;
        uint64_t gpe2_ocislv_err_mask : 1;
        uint64_t gpe1_ocislv_err_mask : 1;
        uint64_t gpe0_ocislv_err_mask : 1;
        uint64_t spare_49_50_mask : 2;
        uint64_t srt_other_error_mask : 1;
        uint64_t sram_spare_direct_error_mask : 1;
        uint64_t c405_oci_machinecheck_mask : 1;
        uint64_t c405_ecc_ce_mask : 1;
        uint64_t c405_ecc_ue_mask : 1;
        uint64_t spare_err_38_mask : 1;
        uint64_t jtagacc_err_mask : 1;
        uint64_t spare_35_41_mask : 7;
        uint64_t ocb_idc_error_mask : 1;
        uint64_t ocb_pib_addr_parity_err_mask : 1;
        uint64_t ocb_db_error_mask : 1;
        uint64_t ppc405_dbgstopack_mask : 1;
        uint64_t ppc405_dbgmsrwe_mask : 1;
        uint64_t ppc405_system_reset_mask : 1;
        uint64_t ppc405_chip_reset_mask : 1;
        uint64_t ppc405_core_reset_mask : 1;
        uint64_t external_trap_mask : 1;
        uint64_t spare_24_25_mask : 2;
        uint64_t gpe3_write_protect_error_mask : 1;
        uint64_t gpe2_write_protect_error_mask : 1;
        uint64_t gpe1_write_protect_error_mask : 1;
        uint64_t gpe0_write_protect_error_mask : 1;
        uint64_t gpe3_halted_mask : 1;
        uint64_t gpe2_halted_mask : 1;
        uint64_t gpe1_halted_mask : 1;
        uint64_t gpe0_halted_mask : 1;
        uint64_t srt_ce_mask : 1;
        uint64_t srt_ue_mask : 1;
        uint64_t ocb_error_mask : 1;
        uint64_t gpe3_error_mask : 1;
        uint64_t gpe2_error_mask : 1;
        uint64_t gpe1_error_mask : 1;
        uint64_t gpe0_error_mask : 1;
        uint64_t gpe3_watchdog_timeout_mask : 1;
        uint64_t gpe2_watchdog_timeout_mask : 1;
        uint64_t gpe1_watchdog_timeout_mask : 1;
        uint64_t gpe0_watchdog_timeout_mask : 1;
        uint64_t occ_hb_error_mask : 1;
        uint64_t stop_recovery_notify_prd_mask : 1;
        uint64_t qme_error_mask : 1;
        uint64_t occ_fw1_mask : 1;
        uint64_t occ_fw0_mask : 1;
#endif // _BIG_ENDIAN
    } fields;
} occ_scom_occlfirmask_or_t;



typedef union occ_scom_occlfiract0
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
        uint64_t fir_action0 : 62;
        uint64_t reserved1 : 2;
#else
        uint64_t reserved1 : 2;
        uint64_t fir_action0 : 62;
#endif // _BIG_ENDIAN
    } fields;
} occ_scom_occlfiract0_t;



typedef union occ_scom_occlfiract1
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
        uint64_t fir_action1 : 62;
        uint64_t reserved1 : 2;
#else
        uint64_t reserved1 : 2;
        uint64_t fir_action1 : 62;
#endif // _BIG_ENDIAN
    } fields;
} occ_scom_occlfiract1_t;



typedef union occ_scom_occerrrpt
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
        uint64_t sram_cerrrpt : 10;
        uint64_t jtagacc_cerrpt : 6;
        uint64_t c405_dcu_ecc_ue : 1;
        uint64_t c405_dcu_ecc_ce : 1;
        uint64_t c405_icu_ecc_ue : 1;
        uint64_t c405_icu_ecc_ce : 1;
        uint64_t gpe0_ocislv_err : 7;
        uint64_t reserved1 : 1;
        uint64_t gpe1_ocislv_err : 7;
        uint64_t reserved2 : 1;
        uint64_t gpe2_ocislv_err : 7;
        uint64_t reserved3 : 1;
        uint64_t gpe3_ocislv_err : 7;
        uint64_t reserved4 : 1;
        uint64_t ocb_ocislv_err : 6;
        uint64_t reserved5 : 6;
#else
        uint64_t reserved5 : 6;
        uint64_t ocb_ocislv_err : 6;
        uint64_t reserved4 : 1;
        uint64_t gpe3_ocislv_err : 7;
        uint64_t reserved3 : 1;
        uint64_t gpe2_ocislv_err : 7;
        uint64_t reserved2 : 1;
        uint64_t gpe1_ocislv_err : 7;
        uint64_t reserved1 : 1;
        uint64_t gpe0_ocislv_err : 7;
        uint64_t c405_icu_ecc_ce : 1;
        uint64_t c405_icu_ecc_ue : 1;
        uint64_t c405_dcu_ecc_ce : 1;
        uint64_t c405_dcu_ecc_ue : 1;
        uint64_t jtagacc_cerrpt : 6;
        uint64_t sram_cerrrpt : 10;
#endif // _BIG_ENDIAN
    } fields;
} occ_scom_occerrrpt_t;



typedef union occ_scom_occerrrpt2
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
        uint64_t sram_spare_direct_error0 : 1;
        uint64_t sram_spare_direct_error1 : 1;
        uint64_t sram_spare_direct_error2 : 1;
        uint64_t sram_spare_direct_error3 : 1;
        uint64_t ocb_idc0_error : 1;
        uint64_t ocb_idc1_error : 1;
        uint64_t ocb_idc2_error : 1;
        uint64_t ocb_idc3_error : 1;
        uint64_t ocb_db_oci_timeout : 1;
        uint64_t ocb_db_oci_read_data_parity : 1;
        uint64_t ocb_db_oci_slave_error : 1;
        uint64_t ocb_db_pib_data_parity_err : 1;
        uint64_t srt_read_error : 1;
        uint64_t srt_write_error : 1;
        uint64_t srt_dataout_perr : 1;
        uint64_t srt_oci_write_data_parity : 1;
        uint64_t srt_oci_be_parity_err : 1;
        uint64_t srt_oci_addr_parity_err : 1;
        uint64_t srt_fsm_err : 1;
        uint64_t reserved1 : 45;
#else
        uint64_t reserved1 : 45;
        uint64_t srt_fsm_err : 1;
        uint64_t srt_oci_addr_parity_err : 1;
        uint64_t srt_oci_be_parity_err : 1;
        uint64_t srt_oci_write_data_parity : 1;
        uint64_t srt_dataout_perr : 1;
        uint64_t srt_write_error : 1;
        uint64_t srt_read_error : 1;
        uint64_t ocb_db_pib_data_parity_err : 1;
        uint64_t ocb_db_oci_slave_error : 1;
        uint64_t ocb_db_oci_read_data_parity : 1;
        uint64_t ocb_db_oci_timeout : 1;
        uint64_t ocb_idc3_error : 1;
        uint64_t ocb_idc2_error : 1;
        uint64_t ocb_idc1_error : 1;
        uint64_t ocb_idc0_error : 1;
        uint64_t sram_spare_direct_error3 : 1;
        uint64_t sram_spare_direct_error2 : 1;
        uint64_t sram_spare_direct_error1 : 1;
        uint64_t sram_spare_direct_error0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} occ_scom_occerrrpt2_t;


#endif // __ASSEMBLER__
#endif // __OCC_FIRMWARE_REGISTERS_H__

