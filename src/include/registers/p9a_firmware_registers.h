/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/p9a_firmware_registers.h $              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2018                        */
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

/* This header file is used by both occ_405 and occ_gpe1.                 */
/* Contains common structures and globals.                                */
#if !defined(__P9A_FIRMWARE_REGISTERS_H__)
#define __P9A_FIRMWARE_REGISTERS_H__

typedef union mcfgpr
{
    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t config_valid : 1;
        uint64_t mmio_valid : 1;
        uint64_t config_group_addr : 31;
        uint64_t mmio_group_base_addr : 31;
#else
        uint64_t mmio_group_base_addr : 31;
        uint64_t config_group_addr : 31;
        uint64_t mmio_valid : 1;
        uint64_t config_valid : 1;
#endif // _BIG_ENDIAN
    } fields;
} mcfgpr_t;


typedef union
{
    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t dont_care : 56;
        uint64_t addr_extension_group_id : 4;
        uint64_t addr_extension_chip_id : 3;
        uint64_t dont_care1 : 1;
#else
        uint64_t dont_care1 : 1;
        uint64_t addr_extension_chip_id : 3;
        uint64_t addr_extension_group_id : 4;
        uint64_t dont_care : 56;
#endif
    } fields;
} pb_mode_t;
#endif
