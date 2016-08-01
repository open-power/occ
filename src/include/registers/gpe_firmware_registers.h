/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/gpe_firmware_registers.h $              */
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




typedef union gpe_gpentsel {

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
    uint64_t fit_sel : 4;
    uint64_t watchdog_sel : 4;
    uint64_t reserved1 : 56;
#else
    uint64_t reserved1 : 56;
    uint64_t watchdog_sel : 4;
    uint64_t fit_sel : 4;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpentsel_t;



typedef union gpe_gpenivpr {

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
    uint64_t ivpr : 23;
    uint64_t reserved1 : 41;
#else
    uint64_t reserved1 : 41;
    uint64_t ivpr : 23;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenivpr_t;



typedef union gpe_gpendbg {

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
    uint64_t en_dbg : 1;
    uint64_t halt_on_xstop : 1;
    uint64_t halt_on_trig : 1;
    uint64_t en_risctrace : 1;
    uint64_t en_trace_full_iva : 1;
    uint64_t dis_trace_extra : 1;
    uint64_t dis_trace_stall : 1;
    uint64_t en_wide_trace : 1;
    uint64_t sync_timer_sel : 4;
    uint64_t fir_trigger : 1;
    uint64_t spare : 3;
    uint64_t halt_input : 1;
    uint64_t reserved1 : 47;
#else
    uint64_t reserved1 : 47;
    uint64_t halt_input : 1;
    uint64_t spare : 3;
    uint64_t fir_trigger : 1;
    uint64_t sync_timer_sel : 4;
    uint64_t en_wide_trace : 1;
    uint64_t dis_trace_stall : 1;
    uint64_t dis_trace_extra : 1;
    uint64_t en_trace_full_iva : 1;
    uint64_t en_risctrace : 1;
    uint64_t halt_on_trig : 1;
    uint64_t halt_on_xstop : 1;
    uint64_t en_dbg : 1;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpendbg_t;



typedef union gpe_gpenstr {

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
    uint64_t pbase : 10;
    uint64_t reserved2 : 7;
    uint64_t size : 3;
    uint64_t reserved3 : 32;
#else
    uint64_t reserved3 : 32;
    uint64_t size : 3;
    uint64_t reserved2 : 7;
    uint64_t pbase : 10;
    uint64_t reserved1 : 12;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenstr_t;



typedef union gpe_gpenmacr {

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
    uint64_t mem_low_priority : 2;
    uint64_t mem_high_priority : 2;
    uint64_t local_low_priority : 2;
    uint64_t local_high_priority : 2;
    uint64_t sram_low_priority : 2;
    uint64_t sram_high_priority : 2;
    uint64_t reserved1 : 52;
#else
    uint64_t reserved1 : 52;
    uint64_t sram_high_priority : 2;
    uint64_t sram_low_priority : 2;
    uint64_t local_high_priority : 2;
    uint64_t local_low_priority : 2;
    uint64_t mem_high_priority : 2;
    uint64_t mem_low_priority : 2;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenmacr_t;



typedef union gpe_gpenxixcr {

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
    uint64_t xcr : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t xcr : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxixcr_t;



typedef union gpe_gpenxiramra {

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
    uint64_t xcr : 32;
    uint64_t sprg0 : 32;
#else
    uint64_t sprg0 : 32;
    uint64_t xcr : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxiramra_t;



typedef union gpe_gpenxiramga {

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
    uint64_t ir : 32;
    uint64_t sprg0 : 32;
#else
    uint64_t sprg0 : 32;
    uint64_t ir : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxiramga_t;



typedef union gpe_gpenxiramdbg {

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
    uint64_t xsr : 32;
    uint64_t sprg0 : 32;
#else
    uint64_t sprg0 : 32;
    uint64_t xsr : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxiramdbg_t;



typedef union gpe_gpenxiramedr {

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
    uint64_t ir : 32;
    uint64_t edr : 32;
#else
    uint64_t edr : 32;
    uint64_t ir : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxiramedr_t;



typedef union gpe_gpenxidbgpro {

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
    uint64_t xsr : 32;
    uint64_t iar : 32;
#else
    uint64_t iar : 32;
    uint64_t xsr : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxidbgpro_t;



typedef union gpe_gpenxisib {

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
    uint64_t sib_info : 64;
#else
    uint64_t sib_info : 64;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxisib_t;



typedef union gpe_gpenximem {

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
    uint64_t memory_info : 64;
#else
    uint64_t memory_info : 64;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenximem_t;



typedef union gpe_gpenxisgb {

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
    uint64_t storegatherbuffer_info : 64;
#else
    uint64_t storegatherbuffer_info : 64;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxisgb_t;



typedef union gpe_gpenxiicac {

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
    uint64_t icache_info : 64;
#else
    uint64_t icache_info : 64;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxiicac_t;



typedef union gpe_gpenxidcac {

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
    uint64_t dcache_info : 64;
#else
    uint64_t dcache_info : 64;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxidcac_t;



typedef union gpe_gpenoxixcr {

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
    uint64_t xcr : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t xcr : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenoxixcr_t;



typedef union gpe_gpenxixsr {

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
    uint64_t xsr : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t xsr : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxixsr_t;



typedef union gpe_gpenxisprg0 {

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
    uint64_t sprg0 : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t sprg0 : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxisprg0_t;



typedef union gpe_gpenxiedr {

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
    uint64_t edr : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t edr : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxiedr_t;



typedef union gpe_gpenxiir {

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
    uint64_t ir : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t ir : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxiir_t;



typedef union gpe_gpenxiiar {

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
    uint64_t iar : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t iar : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxiiar_t;



typedef union gpe_gpenxisibu {

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
    uint64_t sib_info_upper : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t sib_info_upper : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxisibu_t;



typedef union gpe_gpenxisibl {

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
    uint64_t sib_info_lower : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t sib_info_lower : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxisibl_t;



typedef union gpe_gpenximemu {

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
    uint64_t memory_info_upper : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t memory_info_upper : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenximemu_t;



typedef union gpe_gpenximeml {

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
    uint64_t memory_info_lower : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t memory_info_lower : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenximeml_t;



typedef union gpe_gpenxisgbu {

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
    uint64_t sgb_info_upper : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t sgb_info_upper : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxisgbu_t;



typedef union gpe_gpenxisgbl {

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
    uint64_t sgb_info_lower : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t sgb_info_lower : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxisgbl_t;



typedef union gpe_gpenxiicacu {

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
    uint64_t icache_info_upper : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t icache_info_upper : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxiicacu_t;



typedef union gpe_gpenxiicacl {

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
    uint64_t icache_info_lower : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t icache_info_lower : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxiicacl_t;



typedef union gpe_gpenxidcacu {

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
    uint64_t dcache_info_upper : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t dcache_info_upper : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxidcacu_t;



typedef union gpe_gpenxidcacl {

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
    uint64_t dcache_info_lower : 32;
    uint64_t reserved1 : 32;
#else
    uint64_t reserved1 : 32;
    uint64_t dcache_info_lower : 32;
#endif // _BIG_ENDIAN
    } fields;
} gpe_gpenxidcacl_t;


#endif // __ASSEMBLER__
#endif // __GPE_FIRMWARE_REGISTERS_H__

