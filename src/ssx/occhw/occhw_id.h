/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/occhw/occhw_id.h $                                    */
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
#ifndef __OCCHW_ID_H__
#define __OCCHW_ID_H__

//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file occhw_id.h
/// \brief chip and EC-level identification + chip configuration
///
/// During initialization the device identification SCOM registers are read
/// and cached.
///
/// The node and chip ids are read from TPC_GP0 and stored as global
/// variables, accessible through the node_id() and chip_id() APIs.  The
/// global variables are available to PORE programs.
///
/// The TPC_DEVICE_ID register is also read, deconstructed and stored in
/// global variables.  APIs defined here provide access to the fields of the
/// device identification to support run-time behavior based on the chip type
/// and EC level of the POWER8 chip containing the OCC.  The global variables
/// are available to PORE programs.
///
/// - cfam_id() : Obtain the full chip identification as a 32-bit CFAM id
/// - cfam_chip_type() : Obtain the 8-bit CFAM chip type
/// - cfam_ec_level() : Obtain an 8-bit CFAM EC level
///
/// For example, to identify a chip as Murano DD1.0, one could use either
/// method shown below:
///
/// \code
///
/// if (cfam_id() == CFAM_CHIP_ID_MURANO_10) { ... }
///
/// if ((cfam_chip_type() == CFAM_CHIP_TYPE_MURANO) &&
///     (cfam_ec_level() == 0x10)) { ... }
///
/// \encode

// Error/Panic Codes

#define OCCHW_ID_SCOM_ERROR_SELECT   0x00747401
#define OCCHW_ID_SCOM_ERROR_CONFIG   0x00747402
#define OCCHW_ID_SELECT_ERROR        0x00747403


#ifndef __ASSEMBLER__

    #include <stdint.h>
    #include "tpc_firmware_registers.h"


    /// Get TPC device identification (internal API, called once from __occhw_setup().
    void
    _occhw_get_ids(void);

    /// Get the TPC Node Id
    uint8_t node_id(void);

    /// Get the TPC Chip Id
    uint8_t chip_id(void);

    /// Get the CFAM Chip Id
    ///
    /// \returns A 32-bit value to be compared against the enumeration of known
    /// CFAM ids.  See \ref cfam_chip_ids.
    uint32_t cfam_id(void);

    /// Get the CFAM Chip Type
    ///
    /// \returns An 8-bit value to be compared against the enumeration of known
    /// CFAM chip types.  See \ref cfam_chip_types.
    uint8_t cfam_chip_type(void);

    /// Get the CFAM Chip EC Level
    ///
    /// \returns An 8-bit value; The high-order nibble is the major EC level and
    /// the low-order nibble is the minor EC level. Fore example a value of 0x21
    /// indicates DD 2.1.
    uint8_t cfam_ec_level(void);


    /// Compute the chip configuration (internal API, called once from __occhw_setup().
    void
    _occhw_get_chip_configuration(void);

    /// Get the core configuration
    ///
    /// The return value is a 32 bit integer with big-endian bits set to indicate
    /// valid cores.
    uint32_t
    core_configuration(void);


#endif // __ASSEMBLER__


/// \defgroup cfam_chip_types CFAM Chip Types (Including Centaur)
///
/// The CFAM Chip Type is an 8-bit value that uniquely identfies a chip
/// architecture.
///
/// @{

#define CFAM_CHIP_TYPE_CENTAUR 0xe9
#define CFAM_CHIP_TYPE_VENICE  0xea
#define CFAM_CHIP_TYPE_MURANO  0xef

/// @}


/// \defgroup cfam_chip_ids CFAM Chip Ids (Including Centaur)
///
/// The CFAM Chip ID is a 32-bit value that uniquely identfies a chip and its
/// EC level.
///
/// The reference:
///
/// - https://eclipz.pok.ibm.com/sys/ras/docs/cfam_ids.txt
///
/// The interpretation:
///
/// MlmCC049
///
/// M   - Major EC (RIT-A) level
/// l   - 2:Austin, 6:Poughkeepsie
/// m   - Minor EC (RIT-B) level
/// CC  - 0xE9:Centaur, 0xEA:Venice, 0xEF:Murano
/// 049 - IBM (Except for buggy 0x001 in Murano DD1.X)
///
/// @{

#define CFAM_CHIP_ID_VENICE_10 0x120ea049
#define CFAM_CHIP_ID_VENICE_20 0x220ea049
#define CFAM_CHIP_ID_VENICE_21 0x221ea049

#define CFAM_CHIP_ID_MURANO_10 0x120ef001
#define CFAM_CHIP_ID_MURANO_11 0x121ef001
#define CFAM_CHIP_ID_MURANO_12 0x122ef001
#define CFAM_CHIP_ID_MURANO_13 0x123ef001
#define CFAM_CHIP_ID_MURANO_20 0x220ef049
#define CFAM_CHIP_ID_MURANO_21 0x221ef049

#define CFAM_CHIP_ID_CENTAUR_10 0x160e9049
#define CFAM_CHIP_ID_CENTAUR_20 0x260e9049


#ifndef __ASSEMBLER__

/// The CFAM ID as a set of fields

typedef union
{
#ifdef _BIG_ENDIAN
    struct
    {
        uint32_t majorEc  : 4;
        uint32_t location : 4;
        uint32_t minorEc  : 4;
        uint32_t chipType : 8;
        uint32_t vendor   : 12;
    };
#else
    struct
    {
        uint32_t vendor   : 12;
        uint32_t chipType : 8;
        uint32_t minorEc  : 4;
        uint32_t location : 4;
        uint32_t majorEc  : 4;
    };
#endif // _BIG_ENDIAN
    uint32_t value;

} cfam_id_t;

#endif // __ASSEMBLER__

/// @}

#endif // __OCCHW_ID_H__
