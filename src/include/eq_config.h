/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/eq_config.h $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2019,2020                        */
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

/// \file eq_config.h
/// \brief EQ (Quad SuperChiplet) configuration data structures OCC core data collection
#ifndef __EQ_CONFIG__
#define __EQ_CONFIG__
#include <stdint.h>

#define THERM_DTS_RESULT        0x00050000

#define MAX_NUM_CORES   32
#define CORES_PER_QUAD  4
#define MAX_NUM_QUADS   (MAX_NUM_CORES/CORES_PER_QUAD)
#define EQ_DTS_RACETRACK_OFFSET 2

typedef union dts_sensor_result_reg
{
    uint64_t value;
    struct
    {
        uint16_t  reading[3];
        uint16_t  unused_hw3;
    } half_words;
} dts_sensor_result_reg_t;

typedef union sensor_result
{
    uint16_t result;
    struct
    {
        uint16_t reading : 12;
        uint16_t thermal_trip : 2;
        uint16_t spare : 1;
        uint16_t valid : 1;
    } fields;

} sensor_result_t;

typedef union cpms_sdsr
{
    uint64_t value;
    struct
    {
        uint64_t data : 5;
        uint64_t data_min : 5;
        uint64_t data_max : 5;
        uint64_t not_used : 49;
    } fields;
} cpms_sdsr_t;

typedef union dpll_echar_reg
{
    uint64_t value;
    struct
    {
        uint64_t unused1 : 1;
        uint64_t encode : 3;
        uint64_t unused2 : 1;
        uint64_t encode_min : 3;
        uint64_t unused3 : 1;
        uint64_t encode_max : 3;
        uint64_t unused4 : 52;
    } fields;
} dpll_echar_reg_t;

/// SCOM address Ranges:
// Cores (EX chiplet): 0x20000000 - 0x37000000
//
#define CHIPLET_CORE_SCOM_BASE  0x20000000

#define CHIPLET_QUAD_BASE(n) \
    (((n) << 24) + CHIPLET_CORE_SCOM_BASE)

#define DTS_CHIPLET_CORE_OFFSET(n) \
    ((n<2)?n:(n-2)+0x20)

#endif

