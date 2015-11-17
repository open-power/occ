/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/p9_config.h $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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

/// \file p9_config.h
/// \brief Chip configuration data structures for P9 OCC procedures
///
//  *HWP HWP Owner: Doug Gilbert <dgilbert@us.ibm.com>
//  *HWP FW Owner: Martha Broyles <mbroyles@us.ibm.com>
//  *HWP Team: PM
//  *HWP Level: 1
//  *HWP Consumed by: OCC

#ifndef __P9_GPE_CONFIG_H__
#define __P9_GPE_CONFIG_H__


#include <stdint.h>

// TODO is any of the "Config" needed in P9?
//
/// A bitmask defining a chip configuration
///
/// Since we are using the conventional big-endian notation, any use of these
/// bitmasks requires that the data being tested is of this type - otherwise
/// the masks won't work.
///
/// Layout:
///
/// Bits 0:15  - Core chiplet 0..15 is configured
/// Bits 16:23 - MCS 0..7 is configured
/// Bits 24:31 - Centaur 0..7 is configured

typedef uint64_t ChipConfig;
typedef uint16_t ChipConfigCores;
typedef uint8_t  ChipConfigMcs;
typedef uint8_t  ChipConfigCentaur;


/// Convert a ChipConfig into a mask suitable for use as the 32-bit chiplet
/// mask argument of a PORE wakeup program.

static inline uint32_t
pore_exe_mask(ChipConfig config)
{
    return (uint32_t)((config >> 32) & 0xffff0000);
}

/// Left justify and mask core chiplet configuration into a uint32_t

static inline uint32_t
left_justify_core_config(ChipConfig config)
{
    return (uint32_t)((config >> 32) & 0xffff0000);
}

/// Left justify and mask MCS configuration into a uint32_t

static inline uint32_t
left_justify_mcs_config(ChipConfig config)
{
    return (uint32_t)((config >> 16) & 0xff000000);
}

/// Left justify and mask Centaur configuration into a uint32_t

static inline uint32_t
left_justify_centaur_config(ChipConfig config)
{
    return (uint32_t)((config >> 8) & 0xff000000);
}

/// SCOM address Reanges:
// Cores (EX chiplet): 0x20000000 - 0x37000000
// Caches: 0x10000000 - 0x15000000
//
#define CHIPLET_CORE_SCOM_BASE  0x20000000
#define CHIPLET_CACHE_SCOM_BASE 0x10000000

#define CHIPLET_CORE_ID(n) \
    (((n) << 24) + CHIPLET_CORE_SCOM_BASE)

#define CHIPLET_CACHE_ID(n) \
    (((n) << 24) + CHIPLET_CACHE_SCOM_BASE)


#endif  /* __P9_GPE_CONFIG_H__ */
