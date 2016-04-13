/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/pgp_config.h $                                        */
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
#ifndef __PGP_CONFIG_H__
#define __PGP_CONFIG_H__

/// \file pgp_config.h
/// \brief Chip configuration data structures for PgP OCC procedures

#ifndef __ASSEMBLER__

#include <stdint.h>

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

#endif // __ASSEMBLER__


#define CHIP_CONFIG_CORE_BASE 0
#define CHIP_CONFIG_CORE(n) \
    ((0x8000000000000000ull >> CHIP_CONFIG_CORE_BASE) >> (n))

#define CHIP_CONFIG_MCS_BASE 16
#define CHIP_CONFIG_MCS(n) \
    ((0x8000000000000000ull >> CHIP_CONFIG_MCS_BASE) >> (n))

#define CHIP_CONFIG_CENTAUR_BASE 24
#define CHIP_CONFIG_CENTAUR(n) \
    ((0x8000000000000000ull >> CHIP_CONFIG_CENTAUR_BASE) >> (n))


// PGAS macros to left justify configuration groups, allowing each member to
// be tested in a loop that rotates the data (d) register left on each loop,
// assuming standard big-endian bit assignments. The macros mask off all other
// configuration bits so the destination register can also be tested for
// 0/non-0 to determine if any of a configuration class are selected.

#ifdef __PGAS__

        .macro  left_justify_core_config, d
        extldi  (\d), (\d), PGP_NCORES, CHIP_CONFIG_CORE_BASE
        .endm

        .macro  left_justify_mcs_config, d
        extldi  (\d), (\d), PGP_NMCS, CHIP_CONFIG_MCS_BASE
        .endm

        .macro  left_justify_centaur_config, d
        extldi  (\d), (\d), PGP_NCENTAUR, CHIP_CONFIG_CENTAUR_BASE
        .endm

#endif /* __PGAS__ */

#endif  /* __PGP_CONFIG_H__ */
