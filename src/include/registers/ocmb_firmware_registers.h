/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/ocmb_firmware_registers.h $             */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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
#ifndef __OCMB_FIRMWARE_REGISTERS_H__
#define __OCMB_FIRMWARE_REGISTERS_H__

#include <stdint.h>

typedef union ocmb_mmio_merrctl
{
    uint64_t value;
    struct
    {
        uint32_t high_order;
        uint32_t low_order;
    } words;

    struct
    {
        uint64_t dont_care0: 52;
        uint64_t snsc_master_enable : 1;
        uint64_t dont_care1: 11;
    } fields;

} mmio_merrctl_t;

#define SNSC_MASTER_ENABLED 1
#define SNSC_MASTER_DISABLED 0

typedef union ocmb_mmio_mfir
{
    uint64_t value;
    struct
    {
        uint32_t high_order;
        uint32_t low_order;
    } words;

    struct
    {
        uint64_t dont_care0: 7;
        uint64_t snsc_both_starts_err: 1;
        uint64_t snsc_mult_seq_perr:   1;
        uint64_t snsc_fsm_perr:        1;
        uint64_t snsc_reg_perr:        1;
        uint64_t dont_care1: 53;
    } fields;
} mmio_mfir_t;

typedef union ocmb_mba_farb3q
{
    uint64_t value;
    struct
    {
        uint32_t high_order;
        uint32_t low_order;
    } words;

    struct
    {
        uint64_t cfg_nm_n_per_slot:     15; //mba
        uint64_t cfg_nm_n_per_port:     16; //chip
        uint64_t cfg_nm_m:              14;
        uint64_t cfg_nm_ras_weight:      3;
        uint64_t cfg_nm_cas_weight:      3;
        uint64_t reserved0:              2;
        uint64_t cfg_nm_change_after_sync: 1;
        uint64_t reserved1:             10;
    } fields;
} ocmb_mba_farb3q_t;

typedef union ocmb_therm
{
    uint64_t value;
    struct
    {
        uint32_t high_order;
        uint32_t low_order;
    } words;

    struct
    {
        uint64_t reserved0:         45;
        uint64_t present:            1;
        uint64_t valid:              1;
        uint64_t error:              1;
        uint64_t data:              16;
    } fields;
} ocmb_therm_t;

typedef union ocmb_mbarpc0q
{
    uint64_t value;
    struct ocmb_firmware_registers
    {
        uint32_t high_order;
        uint32_t low_order;
    } words;

    struct
    {
        uint64_t dont_care0:                23;
        uint64_t min_domain_reduction_time: 10;
        uint64_t dont_care1:                31;
    } fields;
} ocmb_mbarpc0q_t;

typedef union ocmb_mbastr0q
{
    uint64_t value;
    struct
    {
        uint32_t high_order;
        uint32_t low_order;
    } words;

    struct
    {
        uint64_t str_enable:         1;
        uint64_t dont_care2:         1;
        uint64_t enter_str_time:    10;
        uint64_t dont_care0:        45;
        uint64_t deadman_timer_sel:  4;
        uint64_t deadman_tb_sel:     1;
        uint64_t dont_care1:         2;
    } fields;
}ocmb_mbastr0q_t;

#define MBASTR0Q_STR_OFF (0)
#define MBASTR0Q_STR_ON  (1)

typedef union ocmb_wa0
{
    uint64_t value;
    struct
    {
        uint64_t dont_care0:    1;
        uint64_t octs_present:  1;
        uint64_t octs_valid:    1;
        uint64_t octs_error:    1;
        uint64_t octs_reading: 16;
        uint64_t dont_care1:    1;
        uint64_t dts0_present:  1;
        uint64_t dts0_valid:    1;
        uint64_t dts0_error:    1;
        uint64_t dts0_reading: 16;
        uint64_t dont_care2:    1;
        uint64_t dts1_present:  1;
        uint64_t dts1_valid:    1;
        uint64_t dts1_error:    1;
        uint64_t dts1_reading: 16;
        uint64_t dont_care3:    3;
        uint64_t event:         1;
    } fields;
} ocmb_wa0_t;

typedef union ocmb_wa1
{
    uint64_t value;
    struct
    {
        uint64_t dont_care0:    1;
        uint64_t dts2_present:  1;
        uint64_t dts2_valid:    1;
        uint64_t dts2_error:    1;
        uint64_t dts2_reading: 16;
        uint64_t dont_care1:    1;
        uint64_t dts3_present:  1;
        uint64_t dts3_valid:    1;
        uint64_t dts3_error:    1;
        uint64_t dts3_reading: 16;
        uint64_t sr_cnt_side0:  8;
        uint64_t sr_cnt_side1:  8;
        uint64_t dont_care2:    8;
    } fields;
} ocmb_wa1_t;

typedef union ocmb_wa2
{
    uint64_t value;
    struct
    {
        uint32_t reads_side0_1;
        uint32_t writes_side0_1;
    };
} ocmb_wa2_t;





#endif
