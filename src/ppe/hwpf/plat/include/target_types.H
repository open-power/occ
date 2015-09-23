/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwpf/plat/include/target_types.H $                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2012,2015                        */
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
/**
 * @file target_types.H
 * @brief definitions for fapi2 target types
 */

#ifndef __FAPI2_TARGET_TYPES__
#define __FAPI2_TARGET_TYPES__

#ifndef __ASSEMBLER__


/// FAPI namespace
namespace fapi2
{

    ///
    /// @enum fapi::TargetType
    /// @brief Types, kinds, of targets
    /// @note TYPE_NONE is used to represent empty/NULL targets in lists
    /// or tables. TYPE_ALL is used to pass targets to methods which
    /// can act generally on any type of target
    ///
    /// Target Kind
    enum TargetType
    {
        TARGET_TYPE_NONE              = 0x00, ///< No type
        TARGET_TYPE_PROC_CHIP         = 0x01, ///< Processor type
        TARGET_TYPE_EX                = 0x02, ///< Ex type
        TARGET_TYPE_CORE              = 0x04, ///< Core type
        TARGET_TYPE_EQ                = 0x08, ///< EQ type
        TARGET_TYPE_MCS               = 0x10, ///< MCS type
        TARGET_TYPE_PERV              = 0x20, ///< Pervasive type
        TARGET_TYPE_MCAST             = 0x80, ///< Multicast type

        TARGET_TYPE_ALL               = 0xFF, ///< Any/All types

        // The following are actually illegal targets on PPE platforms    
//         TARGET_TYPE_SYSTEM            = 0xFE, ///< System type
//         TARGET_TYPE_DIMM              = 0xFD, ///< DIMM type
//         TARGET_TYPE_MEMBUF_CHIP       = 0xFC, ///< Membuf type
//         TARGET_TYPE_MBA               = 0xFB, ///< MBA type
//         TARGET_TYPE_XBUS              = 0xFA, ///< XBUS type
//         TARGET_TYPE_ABUS              = 0xF9, ///< ABUS type
//         TARGET_TYPE_L4                = 0xF8, ///< L4 type
//         TARGET_TYPE_MCA               = 0xF7, ///< MCA type
//         TARGET_TYPE_MCBIST            = 0xF6, ///< MCBIST type
//         TARGET_TYPE_MIA               = 0xF5, ///< MIA type
//         TARGET_TYPE_MIS               = 0xF4, ///< MIS type
//         TARGET_TYPE_DMI               = 0xF3, ///< DMI type
//         TARGET_TYPE_OBUS              = 0xF2, ///< OBUS type
//         TARGET_TYPE_NV                = 0xF1, ///< NV bus type
//         TARGET_TYPE_SBE               = 0xF0, ///< SBE type
//         TARGET_TYPE_PPE               = 0xEF, ///< PPE type
//         TARGET_TYPE_PEC               = 0xEE, ///< PEC type
//         TARGET_TYPE_PHB               = 0xED, ///< PHB type  
//         TARGET_TYPE_MI                = 0xEC, ///< MI type  

        // Mappings to target types found in the error xml files
        TARGET_TYPE_EX_CHIPLET    = TARGET_TYPE_EX,
//        TARGET_TYPE_MBA_CHIPLET   = TARGET_TYPE_MBA,
        TARGET_TYPE_MCS_CHIPLET   = TARGET_TYPE_MCS,
//        TARGET_TYPE_XBUS_ENDPOINT = TARGET_TYPE_XBUS,
//        TARGET_TYPE_ABUS_ENDPOINT = TARGET_TYPE_ABUS,
    };
    
    ///
    /// @brief Typedef used when passing multiple TargetType values
    ///
    typedef uint8_t TargetTypes_t;           

    /// @cond
    constexpr TargetType operator|(TargetType x, TargetType y)
    {
        return static_cast<TargetType>(static_cast<int>(x) |
                                       static_cast<int>(y));
    }

    template<uint64_t V>
    class bitCount {
    public:
        // Don't use enums, too hard to compare
        static const uint8_t count = bitCount<(V >> 1)>::count + (V & 1);
    };

    template<>
    class bitCount<0> {
    public:
        static const uint8_t count = 0;
    };
    /// @endcond
};

#endif  // __ASSEMBLER__
#endif  // __FAPI2_TARGET_TYPES__
