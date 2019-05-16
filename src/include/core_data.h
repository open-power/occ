/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/core_data.h $                                     */
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

/// \file core_data.h
/// \brief Data structures for the GPE programs that collect raw data defined
/// in core_data.C.  The data structure layouts are also documented in the
/// spreadsheet \todo (RTC 137031) location.

#ifndef __GPE_CORE_DATA_H__
#define __GPE_CORE_DATA_H__

#include <stdint.h>
#include <eq_config.h>

#define PC_OCC_SPRC             0x00010482
#define PC_OCC_SPRD             0x00010483
#define TOD_VALUE_REG           0x00040020
#define STOP_STATE_HIST_OCC_REG 0x000F0112 // TODO RTC 213673 Exist on P10?

#define CORE_RAW_CYCLES             0x200

#define EMPATH_CORE_THREADS 4

// return codes:
#define SIBRC_RESOURCE_OCCUPIED (1)
#define SIBRC_CORE_FENCED       (2)
#define SIBRC_PARTIAL_GOOD      (3)
#define SIBRC_ADDRESS_ERROR     (4)
#define SIBRC_CLOCK_ERROR       (5)
#define SIBRC_PACKET_ERROR      (6)
#define SIBRC_TIMEOUT           (7)

#define EMPATH_VALID    (1)
#define EMPATH_CORE_REGION(n) \
    (0x8000 >> n)


typedef struct
{
    uint32_t raw_cycles;       // 0x200
    uint32_t run_cycles;       // 0x208
    uint32_t complete;         // 0x210 Core instruction complete Utilization
    uint32_t freq_sens_busy;   // 0x218 Core workrate busy counter
    uint32_t freq_sens_finish; // 0x220 Core workrate finish counter
    uint32_t mem_latency_a;    // 0x228
    uint32_t mem_latency_b;    // 0x230
    uint32_t mem_access_c;     // 0x238

} CoreDataEmpath;

typedef struct
{
    uint32_t ifu_throttle;      // 0x240  IFU Throttle Block Fetch
    uint32_t ifu_active;        // 0x248  IFU Throttle Active
    uint32_t pwr_proxy;         // 0x250  power proxy
    uint32_t fine_throttle;     // 0x258  Fine Throttle blocked
    uint32_t harsh_throttle;    // 0x260  Harsh Trottle blocked
} CoreDataThrottle;

typedef struct
{
    uint32_t v_droop_small;     // 0x268  Small Droop present
    uint32_t v_droop_large;     // 0x270  Larget Droop present
    uint32_t mma_active;        // 0x278  MMA Active
} CoreDataDroop;

typedef struct
{
    sensor_result_t core[2];
    sensor_result_t cache;
    sensor_result_t racetrack;
} CoreDataDts;

//
// The instance of this data object must be 8 byte aligned and
// size must be muliple of 8
//
typedef struct // 136 bytes
{
    CoreDataEmpath             empath;          //32
    CoreDataThrottle           throttle;        //20
    CoreDataDroop              droop;           //12
    CoreDataDts                dts;             // 8
    uint64_t                   stop_state_hist; // 8
    uint32_t                   tod_2mhz;        // 4
    uint32_t                   empathValid;     // 4
} CoreData;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Get core data
 * @param[in] The system core number [0-23]
 * @param[out] Data pointer for the result
 * @return result of scom operation
 */
uint32_t  get_core_data(uint32_t i_core, CoreData* o_data);

#ifdef __cplusplus
};
#endif
#endif  /* __GPE_CORE_DATA_H__ */
