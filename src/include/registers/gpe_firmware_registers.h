/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/gpe_firmware_registers.h $              */
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
#ifndef __GPE_FIRMWARE_REGISTERS_H__
#define __GPE_FIRMWARE_REGISTERS_H__

/// \file gpe_firmware_registers.h
/// \brief C register structs for the GPE unit

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




typedef union gpe_ocb_gpetsel
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t watchdog_sel : 4;
        uint32_t fit_sel : 4;
        uint32_t reserved1 : 24;
#else
        uint32_t reserved1 : 24;
        uint32_t fit_sel : 4;
        uint32_t watchdog_sel : 4;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpetsel_t;



typedef union gpe_ocb_gpeivpr
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t ivpr : 23;
        uint32_t reserved1 : 9;
#else
        uint32_t reserved1 : 9;
        uint32_t ivpr : 23;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpeivpr_t;



typedef union gpe_ocb_gpedbg
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t en_dbg : 1;
        uint32_t halt_on_xstop : 1;
        uint32_t halt_on_trig : 1;
        uint32_t en_coverage_mode : 1;
        uint32_t en_intr_addr : 1;
        uint32_t en_trace_extra : 1;
        uint32_t en_trace_stall : 1;
        uint32_t en_wait_cycles : 1;
        uint32_t en_full_speed : 1;
        uint32_t dis_flow_change : 1;
        uint32_t trace_mode_sel : 2;
        uint32_t reserved12_15 : 4;
        uint32_t fir_trigger : 1;
        uint32_t spare : 3;
        uint32_t trace_data_sel : 4;
        uint32_t reserved1 : 8;
#else
        uint32_t reserved1 : 8;
        uint32_t trace_data_sel : 4;
        uint32_t spare : 3;
        uint32_t fir_trigger : 1;
        uint32_t reserved12_15 : 4;
        uint32_t trace_mode_sel : 2;
        uint32_t dis_flow_change : 1;
        uint32_t en_full_speed : 1;
        uint32_t en_wait_cycles : 1;
        uint32_t en_trace_stall : 1;
        uint32_t en_trace_extra : 1;
        uint32_t en_intr_addr : 1;
        uint32_t en_coverage_mode : 1;
        uint32_t halt_on_trig : 1;
        uint32_t halt_on_xstop : 1;
        uint32_t en_dbg : 1;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpedbg_t;



typedef union gpe_ocb_gpe0str
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t reserved1 : 12;
        uint32_t pbase : 10;
        uint32_t reserved2 : 7;
        uint32_t size : 3;
#else
        uint32_t size : 3;
        uint32_t reserved2 : 7;
        uint32_t pbase : 10;
        uint32_t reserved1 : 12;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpe0str_t;



typedef union gpe_ocb_gpemacr
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t mem_low_priority : 2;
        uint32_t mem_high_priority : 2;
        uint32_t local_low_priority : 2;
        uint32_t local_high_priority : 2;
        uint32_t sram_low_priority : 2;
        uint32_t sram_high_priority : 2;
        uint32_t write_protect_enable : 1;
        uint32_t reserved1 : 19;
#else
        uint32_t reserved1 : 19;
        uint32_t write_protect_enable : 1;
        uint32_t sram_high_priority : 2;
        uint32_t sram_low_priority : 2;
        uint32_t local_high_priority : 2;
        uint32_t local_low_priority : 2;
        uint32_t mem_high_priority : 2;
        uint32_t mem_low_priority : 2;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpemacr_t;



typedef union gpe_ocb_gpeswpr0
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t write_protect_bar : 27;
        uint32_t reserved1 : 5;
#else
        uint32_t reserved1 : 5;
        uint32_t write_protect_bar : 27;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpeswpr0_t;



typedef union gpe_ocb_gpeswpr1
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t write_protect_bar : 27;
        uint32_t reserved1 : 5;
#else
        uint32_t reserved1 : 5;
        uint32_t write_protect_bar : 27;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpeswpr1_t;



typedef union gpe_ocb_gpexixcr
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t reserved1 : 1;
        uint32_t xcr : 3;
        uint32_t reserved2 : 28;
#else
        uint32_t reserved2 : 28;
        uint32_t xcr : 3;
        uint32_t reserved1 : 1;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexixcr_t;



typedef union gpe_ocb_gpexiramra
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t reserved1 : 1;
        uint32_t xcr : 3;
        uint32_t reserved2 : 28;
#else
        uint32_t reserved2 : 28;
        uint32_t xcr : 3;
        uint32_t reserved1 : 1;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexiramra_t;



typedef union gpe_ocb_gpexiramga
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t ir : 32;
#else
        uint32_t ir : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexiramga_t;



typedef union gpe_ocb_gpexiramdbg
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t xsr_hs : 1;
        uint32_t xsr_hc : 3;
        uint32_t xsr_hcp : 1;
        uint32_t xsr_rip : 1;
        uint32_t xsr_sip : 1;
        uint32_t xsr_trap : 1;
        uint32_t xsr_iac : 1;
        uint32_t xsr_sib : 3;
        uint32_t xsr_rdac : 1;
        uint32_t xsr_wdac : 1;
        uint32_t xsr_ws : 1;
        uint32_t xsr_trh : 1;
        uint32_t xsr_sms : 4;
        uint32_t xsr_lp : 1;
        uint32_t xsr_ep : 1;
        uint32_t xsr_ee : 1;
        uint32_t reserved1 : 1;
        uint32_t xsr_ptr : 1;
        uint32_t xsr_st : 1;
        uint32_t reserved2 : 2;
        uint32_t xsr_mfe : 1;
        uint32_t xsr_mcs : 3;
#else
        uint32_t xsr_mcs : 3;
        uint32_t xsr_mfe : 1;
        uint32_t reserved2 : 2;
        uint32_t xsr_st : 1;
        uint32_t xsr_ptr : 1;
        uint32_t reserved1 : 1;
        uint32_t xsr_ee : 1;
        uint32_t xsr_ep : 1;
        uint32_t xsr_lp : 1;
        uint32_t xsr_sms : 4;
        uint32_t xsr_trh : 1;
        uint32_t xsr_ws : 1;
        uint32_t xsr_wdac : 1;
        uint32_t xsr_rdac : 1;
        uint32_t xsr_sib : 3;
        uint32_t xsr_iac : 1;
        uint32_t xsr_trap : 1;
        uint32_t xsr_sip : 1;
        uint32_t xsr_rip : 1;
        uint32_t xsr_hcp : 1;
        uint32_t xsr_hc : 3;
        uint32_t xsr_hs : 1;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexiramdbg_t;



typedef union gpe_ocb_gpexiramedr
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t ir : 32;
#else
        uint32_t ir : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexiramedr_t;



typedef union gpe_ocb_gpexidbgpro
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t xsr_hs : 1;
        uint32_t xsr_hc : 3;
        uint32_t xsr_hcp : 1;
        uint32_t xsr_rip : 1;
        uint32_t xsr_sip : 1;
        uint32_t xsr_trap : 1;
        uint32_t xsr_iac : 1;
        uint32_t xsr_sib : 3;
        uint32_t xsr_rdac : 1;
        uint32_t xsr_wdac : 1;
        uint32_t xsr_ws : 1;
        uint32_t xsr_trh : 1;
        uint32_t xsr_sms : 4;
        uint32_t xsr_lp : 1;
        uint32_t xsr_ep : 1;
        uint32_t xsr_ee : 1;
        uint32_t reserved1 : 1;
        uint32_t xsr_ptr : 1;
        uint32_t xsr_st : 1;
        uint32_t reserved2 : 2;
        uint32_t xsr_mfe : 1;
        uint32_t xsr_mcs : 3;
#else
        uint32_t xsr_mcs : 3;
        uint32_t xsr_mfe : 1;
        uint32_t reserved2 : 2;
        uint32_t xsr_st : 1;
        uint32_t xsr_ptr : 1;
        uint32_t reserved1 : 1;
        uint32_t xsr_ee : 1;
        uint32_t xsr_ep : 1;
        uint32_t xsr_lp : 1;
        uint32_t xsr_sms : 4;
        uint32_t xsr_trh : 1;
        uint32_t xsr_ws : 1;
        uint32_t xsr_wdac : 1;
        uint32_t xsr_rdac : 1;
        uint32_t xsr_sib : 3;
        uint32_t xsr_iac : 1;
        uint32_t xsr_trap : 1;
        uint32_t xsr_sip : 1;
        uint32_t xsr_rip : 1;
        uint32_t xsr_hcp : 1;
        uint32_t xsr_hc : 3;
        uint32_t xsr_hs : 1;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexidbgpro_t;



typedef union gpe_ocb_gpexisib
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t pib_addr : 32;
#else
        uint32_t pib_addr : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexisib_t;



typedef union gpe_ocb_gpeximem
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t mem_addr : 32;
#else
        uint32_t mem_addr : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpeximem_t;



typedef union gpe_ocb_gpexisgb
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t store_address : 32;
#else
        uint32_t store_address : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexisgb_t;



typedef union gpe_ocb_gpexiicac
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t icache_tag_addr : 27;
        uint32_t reserved : 5;
#else
        uint32_t reserved : 5;
        uint32_t icache_tag_addr : 27;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexiicac_t;



typedef union gpe_ocb_gpexidcac
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t dcache_tag_addr : 27;
        uint32_t reserved : 5;
#else
        uint32_t reserved : 5;
        uint32_t dcache_tag_addr : 27;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexidcac_t;



typedef union gpe_ocb_gpexidbginf
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t srr0 : 30;
        uint32_t reserved1 : 2;
#else
        uint32_t reserved1 : 2;
        uint32_t srr0 : 30;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexidbginf_t;



typedef union gpe_ocb_gpeoxixcr
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t reserved1 : 1;
        uint32_t xcr : 3;
        uint32_t reserved2 : 28;
#else
        uint32_t reserved2 : 28;
        uint32_t xcr : 3;
        uint32_t reserved1 : 1;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpeoxixcr_t;



typedef union gpe_ocb_gpexixsr
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t xsr_hs : 1;
        uint32_t xsr_hc : 3;
        uint32_t xsr_hcp : 1;
        uint32_t xsr_rip : 1;
        uint32_t xsr_sip : 1;
        uint32_t xsr_trap : 1;
        uint32_t xsr_iac : 1;
        uint32_t xsr_sib : 3;
        uint32_t xsr_rdac : 1;
        uint32_t xsr_wdac : 1;
        uint32_t xsr_ws : 1;
        uint32_t xsr_trh : 1;
        uint32_t xsr_sms : 4;
        uint32_t xsr_lp : 1;
        uint32_t xsr_ep : 1;
        uint32_t xsr_ee : 1;
        uint32_t reserved1 : 1;
        uint32_t xsr_ptr : 1;
        uint32_t xsr_st : 1;
        uint32_t reserved2 : 2;
        uint32_t xsr_mfe : 1;
        uint32_t xsr_mcs : 3;
#else
        uint32_t xsr_mcs : 3;
        uint32_t xsr_mfe : 1;
        uint32_t reserved2 : 2;
        uint32_t xsr_st : 1;
        uint32_t xsr_ptr : 1;
        uint32_t reserved1 : 1;
        uint32_t xsr_ee : 1;
        uint32_t xsr_ep : 1;
        uint32_t xsr_lp : 1;
        uint32_t xsr_sms : 4;
        uint32_t xsr_trh : 1;
        uint32_t xsr_ws : 1;
        uint32_t xsr_wdac : 1;
        uint32_t xsr_rdac : 1;
        uint32_t xsr_sib : 3;
        uint32_t xsr_iac : 1;
        uint32_t xsr_trap : 1;
        uint32_t xsr_sip : 1;
        uint32_t xsr_rip : 1;
        uint32_t xsr_hcp : 1;
        uint32_t xsr_hc : 3;
        uint32_t xsr_hs : 1;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexixsr_t;



typedef union gpe_ocb_gpexisprg0
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t sprg0 : 32;
#else
        uint32_t sprg0 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexisprg0_t;



typedef union gpe_ocb_gpexiedr
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t edr : 32;
#else
        uint32_t edr : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexiedr_t;



typedef union gpe_ocb_gpexiir
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t ir : 32;
#else
        uint32_t ir : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexiir_t;



typedef union gpe_ocb_gpexiiar
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t iar : 30;
        uint32_t reserved1 : 2;
#else
        uint32_t reserved1 : 2;
        uint32_t iar : 30;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexiiar_t;



typedef union gpe_ocb_gpexisibu
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t sib_info_upper : 32;
#else
        uint32_t sib_info_upper : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexisibu_t;



typedef union gpe_ocb_gpexisibl
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t sib_info_lower_part1 : 3;
        uint32_t reserved1 : 14;
        uint32_t sib_info_lower_part2 : 3;
        uint32_t reserved2 : 10;
        uint32_t sib_info_lower_part3 : 2;
#else
        uint32_t sib_info_lower_part3 : 2;
        uint32_t reserved2 : 10;
        uint32_t sib_info_lower_part2 : 3;
        uint32_t reserved1 : 14;
        uint32_t sib_info_lower_part1 : 3;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexisibl_t;



typedef union gpe_ocb_gpeximemu
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t memory_info_upper : 32;
#else
        uint32_t memory_info_upper : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpeximemu_t;



typedef union gpe_ocb_gpeximeml
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t memory_info_lower : 32;
#else
        uint32_t memory_info_lower : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpeximeml_t;



typedef union gpe_ocb_gpexisgbu
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t sgb_info_upper : 32;
#else
        uint32_t sgb_info_upper : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexisgbu_t;



typedef union gpe_ocb_gpexisgbl
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t sgb_info_lower : 32;
#else
        uint32_t sgb_info_lower : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexisgbl_t;



typedef union gpe_ocb_gpexiicacu
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t icache_info_upper : 27;
        uint32_t reserved1 : 5;
#else
        uint32_t reserved1 : 5;
        uint32_t icache_info_upper : 27;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexiicacu_t;



typedef union gpe_ocb_gpexiicacl
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t icache_info_lower_part1 : 12;
        uint32_t reserved1 : 1;
        uint32_t icache_info_lower_part2 : 3;
        uint32_t reserved2 : 16;
#else
        uint32_t reserved2 : 16;
        uint32_t icache_info_lower_part2 : 3;
        uint32_t reserved1 : 1;
        uint32_t icache_info_lower_part1 : 12;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexiicacl_t;



typedef union gpe_ocb_gpexidcacu
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t dcache_info_upper : 27;
        uint32_t reserved1 : 5;
#else
        uint32_t reserved1 : 5;
        uint32_t dcache_info_upper : 27;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexidcacu_t;



typedef union gpe_ocb_gpexidcacl
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t dcache_info_lower_part1 : 1;
        uint32_t reserved1 : 2;
        uint32_t dcache_info_lower_part2 : 5;
        uint32_t reserved2 : 24;
#else
        uint32_t reserved2 : 24;
        uint32_t dcache_info_lower_part2 : 5;
        uint32_t reserved1 : 2;
        uint32_t dcache_info_lower_part1 : 1;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexidcacl_t;



typedef union gpe_ocb_gpexisrr0
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t srr0 : 30;
        uint32_t reserved1 : 2;
#else
        uint32_t reserved1 : 2;
        uint32_t srr0 : 30;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexisrr0_t;



typedef union gpe_ocb_gpexilr
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t lr : 32;
#else
        uint32_t lr : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexilr_t;



typedef union gpe_ocb_gpexictr
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t ctr : 32;
#else
        uint32_t ctr : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexictr_t;



typedef union gpe_ocb_gpexigpr0
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr0 : 32;
#else
        uint32_t gpr0 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexigpr0_t;



typedef union gpe_ocb_gpexigpr1
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr1 : 32;
#else
        uint32_t gpr1 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexigpr1_t;



typedef union gpe_ocb_gpexigpr2
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr2 : 32;
#else
        uint32_t gpr2 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexigpr2_t;



typedef union gpe_ocb_gpexigpr3
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr3 : 32;
#else
        uint32_t gpr3 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexigpr3_t;



typedef union gpe_ocb_gpexigpr4
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr4 : 32;
#else
        uint32_t gpr4 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexigpr4_t;



typedef union gpe_ocb_gpexigpr5
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr5 : 32;
#else
        uint32_t gpr5 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexigpr5_t;



typedef union gpe_ocb_gpexigpr6
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr6 : 32;
#else
        uint32_t gpr6 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexigpr6_t;



typedef union gpe_ocb_gpexigpr7
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr7 : 32;
#else
        uint32_t gpr7 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexigpr7_t;



typedef union gpe_ocb_gpexigpr8
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr8 : 32;
#else
        uint32_t gpr8 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexigpr8_t;



typedef union gpe_ocb_gpexigpr9
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr9 : 32;
#else
        uint32_t gpr9 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexigpr9_t;



typedef union gpe_ocb_gpexigpr10
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr10 : 32;
#else
        uint32_t gpr10 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexigpr10_t;



typedef union gpe_ocb_gpexigpr13
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr13 : 32;
#else
        uint32_t gpr13 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexigpr13_t;



typedef union gpe_ocb_gpexigpr28
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr28 : 32;
#else
        uint32_t gpr28 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexigpr28_t;



typedef union gpe_ocb_gpexigpr29
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr29 : 32;
#else
        uint32_t gpr29 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexigpr29_t;



typedef union gpe_ocb_gpexigpr30
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr30 : 32;
#else
        uint32_t gpr30 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexigpr30_t;



typedef union gpe_ocb_gpexigpr31
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr31 : 32;
#else
        uint32_t gpr31 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexigpr31_t;



typedef union gpe_ocb_gpexivdr0
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr0 : 32;
#else
        uint32_t gpr0 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexivdr0_t;



typedef union gpe_ocb_gpexivdr2
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr2 : 32;
#else
        uint32_t gpr2 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexivdr2_t;



typedef union gpe_ocb_gpexivdr4
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr4 : 32;
#else
        uint32_t gpr4 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexivdr4_t;



typedef union gpe_ocb_gpexivdr6
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr6 : 32;
#else
        uint32_t gpr6 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexivdr6_t;



typedef union gpe_ocb_gpexivdr8
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr8 : 32;
#else
        uint32_t gpr8 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexivdr8_t;



typedef union gpe_ocb_gpexivdrx
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr10 : 32;
#else
        uint32_t gpr10 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexivdrx_t;



typedef union gpe_ocb_gpexivdr28
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr28 : 32;
#else
        uint32_t gpr28 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexivdr28_t;



typedef union gpe_ocb_gpexivdr30
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t gpr30 : 32;
#else
        uint32_t gpr30 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpexivdr30_t;



typedef union gpe_ocb_gpe1str
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t reserved1 : 12;
        uint32_t pbase : 10;
        uint32_t reserved2 : 7;
        uint32_t size : 3;
#else
        uint32_t size : 3;
        uint32_t reserved2 : 7;
        uint32_t pbase : 10;
        uint32_t reserved1 : 12;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpe1str_t;



typedef union gpe_ocb_gpe2str
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t reserved1 : 12;
        uint32_t pbase : 10;
        uint32_t reserved2 : 7;
        uint32_t size : 3;
#else
        uint32_t size : 3;
        uint32_t reserved2 : 7;
        uint32_t pbase : 10;
        uint32_t reserved1 : 12;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpe2str_t;



typedef union gpe_ocb_gpe3str
{

    uint32_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t reserved1 : 12;
        uint32_t pbase : 10;
        uint32_t reserved2 : 7;
        uint32_t size : 3;
#else
        uint32_t size : 3;
        uint32_t reserved2 : 7;
        uint32_t pbase : 10;
        uint32_t reserved1 : 12;
#endif // _BIG_ENDIAN
    } fields;
} gpe_ocb_gpe3str_t;


#endif // __ASSEMBLER__
#endif // __GPE_FIRMWARE_REGISTERS_H__

