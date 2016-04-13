/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/registers/plb_arbiter_firmware_registers.h $      */
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
#ifndef __PLB_ARBITER_FIRMWARE_REGISTERS_H__
#define __PLB_ARBITER_FIRMWARE_REGISTERS_H__

/// \file plb_arbiter_firmware_registers.h
/// \brief C register structs for the PLB_ARBITER unit

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




typedef union plb_prev {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} plb_prev_t;



typedef union plb_pacr {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t ppm : 1;
    uint32_t ppo : 3;
    uint32_t hbu : 1;
    uint32_t rdp : 2;
    uint32_t wrp : 1;
    uint32_t _reserved0 : 24;
#else
    uint32_t _reserved0 : 24;
    uint32_t wrp : 1;
    uint32_t rdp : 2;
    uint32_t hbu : 1;
    uint32_t ppo : 3;
    uint32_t ppm : 1;
#endif // _BIG_ENDIAN
    } fields;
} plb_pacr_t;



typedef union plb_pesr {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t pte0 : 1;
    uint32_t rw0 : 1;
    uint32_t flk0 : 1;
    uint32_t alk0 : 1;
    uint32_t pte1 : 1;
    uint32_t rw1 : 1;
    uint32_t flk1 : 1;
    uint32_t alk1 : 1;
    uint32_t pte2 : 1;
    uint32_t rw2 : 1;
    uint32_t flk2 : 1;
    uint32_t alk2 : 1;
    uint32_t pte3 : 1;
    uint32_t rw3 : 1;
    uint32_t flk3 : 1;
    uint32_t alk3 : 1;
    uint32_t pte4 : 1;
    uint32_t rw4 : 1;
    uint32_t flk4 : 1;
    uint32_t alk4 : 1;
    uint32_t pte5 : 1;
    uint32_t rw5 : 1;
    uint32_t flk5 : 1;
    uint32_t alk5 : 1;
    uint32_t pte6 : 1;
    uint32_t rw6 : 1;
    uint32_t flk6 : 1;
    uint32_t alk6 : 1;
    uint32_t pte7 : 1;
    uint32_t rw7 : 1;
    uint32_t flk7 : 1;
    uint32_t alk7 : 1;
#else
    uint32_t alk7 : 1;
    uint32_t flk7 : 1;
    uint32_t rw7 : 1;
    uint32_t pte7 : 1;
    uint32_t alk6 : 1;
    uint32_t flk6 : 1;
    uint32_t rw6 : 1;
    uint32_t pte6 : 1;
    uint32_t alk5 : 1;
    uint32_t flk5 : 1;
    uint32_t rw5 : 1;
    uint32_t pte5 : 1;
    uint32_t alk4 : 1;
    uint32_t flk4 : 1;
    uint32_t rw4 : 1;
    uint32_t pte4 : 1;
    uint32_t alk3 : 1;
    uint32_t flk3 : 1;
    uint32_t rw3 : 1;
    uint32_t pte3 : 1;
    uint32_t alk2 : 1;
    uint32_t flk2 : 1;
    uint32_t rw2 : 1;
    uint32_t pte2 : 1;
    uint32_t alk1 : 1;
    uint32_t flk1 : 1;
    uint32_t rw1 : 1;
    uint32_t pte1 : 1;
    uint32_t alk0 : 1;
    uint32_t flk0 : 1;
    uint32_t rw0 : 1;
    uint32_t pte0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} plb_pesr_t;



typedef union plb_pearl {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} plb_pearl_t;



typedef union plb_pearh {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} plb_pearh_t;



typedef union plb_sto_pesr {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t icu_te : 1;
    uint32_t icu_rw : 1;
    uint32_t reserved2 : 2;
    uint32_t dcu_te : 1;
    uint32_t dcu_rw : 1;
    uint32_t reserved6 : 2;
    uint32_t _reserved0 : 24;
#else
    uint32_t _reserved0 : 24;
    uint32_t reserved6 : 2;
    uint32_t dcu_rw : 1;
    uint32_t dcu_te : 1;
    uint32_t reserved2 : 2;
    uint32_t icu_rw : 1;
    uint32_t icu_te : 1;
#endif // _BIG_ENDIAN
    } fields;
} plb_sto_pesr_t;



typedef union plb_sto_pear {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} plb_sto_pear_t;


#endif // __ASSEMBLER__
#endif // __PLB_ARBITER_FIRMWARE_REGISTERS_H__

