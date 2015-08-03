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
        TARGET_TYPE_PERV              = 0x10, ///< Pervasive type
        TARGET_TYPE_EQ_MC_WRITE       = 0x20, ///< EQ Multicast Type
        TARGET_TYPE_EQ_MC_READAND     = 0x21, ///< EQ Multicast Read AND Type
        TARGET_TYPE_EQ_MC_READOR      = 0x22, ///< EQ Multicast Read OR Type
        TARGET_TYPE_CORE_MC_WRITE     = 0x23, ///< Core Multicast Type
        TARGET_TYPE_CORE_MC_READAND   = 0x24, ///< Core Multicast Read AND Type
        TARGET_TYPE_CORE_MC_READOR    = 0x25, ///< Core Multicast Read OR Type
        TARGET_TYPE_CME_CORE0         = 0x40, ///< CME Core0 (CME only)
        TARGET_TYPE_CME_CORE1         = 0x41, ///< CME Core1 (CME only)
        TARGET_TYPE_ALL               = 0xFF, ///< Any/All types
    
    
//         TARGET_TYPE_NONE              = 0x00000000, ///< No type
//         TARGET_TYPE_SYSTEM            = 0x00000001, ///< System type
//         TARGET_TYPE_DIMM              = 0x00000002, ///< DIMM type
//         TARGET_TYPE_PROC_CHIP         = 0x00000004, ///< Processor type
//         TARGET_TYPE_MEMBUF_CHIP       = 0x00000008, ///< Membuf type
//         TARGET_TYPE_EX                = 0x00000010, ///< Ex type
//         TARGET_TYPE_MBA               = 0x00000020, ///< MBA type
//         TARGET_TYPE_MCS               = 0x00000040, ///< MCS type
//         TARGET_TYPE_XBUS              = 0x00000080, ///< XBUS type
//         TARGET_TYPE_ABUS              = 0x00000100, ///< ABUS type
//         TARGET_TYPE_L4                = 0x00000200, ///< L4 type
//         TARGET_TYPE_CORE              = 0x00000400, ///< Core type
//         TARGET_TYPE_EQ                = 0x00000800, ///< EQ type
//         TARGET_TYPE_MCA               = 0x00001000, ///< MCA type
//         TARGET_TYPE_MCBIST            = 0x00002000, ///< MCBIST type
//         TARGET_TYPE_MIA               = 0x00004000, ///< MIA type
//         TARGET_TYPE_MIS               = 0x00008000, ///< MIS type
//         TARGET_TYPE_DMI               = 0x00010000, ///< DMI type
//         TARGET_TYPE_OBUS              = 0x00020000, ///< OBUS type
//         TARGET_TYPE_NV                = 0x00040000, ///< NV bus type
//         TARGET_TYPE_SBE               = 0x00080000, ///< SBE type
//         TARGET_TYPE_PPE               = 0x00100000, ///< PPE type
//         TARGET_TYPE_PERV              = 0x00200000, ///< Pervasive type
//         TARGET_TYPE_PEC               = 0x00400000, ///< PEC type
//         TARGET_TYPE_PHB               = 0x00800000, ///< PHB type     
//         TARGET_TYPE_EQ_MC_WRITE       = 0x01000000, ///< EQ Multicast Type
//         TARGET_TYPE_EQ_MC_READAND     = 0x02000000, ///< EQ Multicast Read AND Type
//         TARGET_TYPE_EQ_MC_READOR      = 0x04000000, ///< EQ Multicast Read OR Type
//         TARGET_TYPE_CORE_MC_WRITE     = 0x08000000, ///< Core Multicast Type
//         TARGET_TYPE_CORE_MC_READAND   = 0x10000000, ///< Core Multicast Read AND Type
//         TARGET_TYPE_CORE_MC_READOR    = 0x20000000, ///< Core Multicast Read OR Type
//         TARGET_TYPE_CME_CORE0         = 0x40000000, ///< CME Core0 (CME only)
//         TARGET_TYPE_CME_CORE1         = 0x80000000, ///< CME Core1 (CME only)
//         TARGET_TYPE_ADDRESS           = 0xAAAAAAAA, ///< Address Overlay Type
//         TARGET_TYPE_ALL               = 0xFFFFFFFF, ///< Any/All types
    };


     /**
      * @brief Typedef used when passing multiple TargetType values
      */
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
