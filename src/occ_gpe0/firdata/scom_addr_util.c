/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/firdata/scom_addr_util.c $                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017                             */
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
#include "scom_addr_util.h"


void set_sat_id(const uint8_t i_sat_id, uint64_t * o_addr)
{
    *o_addr &= 0xFFFFFFFFFFFFFC3FULL;
    *o_addr |= ((i_sat_id & 0xF) << 6);
}

void set_chiplet_id(const uint8_t i_chiplet_id, uint64_t * o_addr)
{
    *o_addr &= 0xFFFFFFFFC0FFFFFFULL;
    *o_addr |= ((i_chiplet_id & 0x3F) << 24);
}

void set_ring(const uint8_t i_ring, uint64_t * o_addr)
{
    *o_addr &= 0xFFFFFFFFFFFF03FFULL;
    *o_addr |= ((i_ring & 0x3F) << 10);
}

void set_sat_offset(const uint8_t i_sat_offset, uint64_t * o_addr)
{
    *o_addr &= 0xFFFFFFFFFFFFFFC0ULL;
    *o_addr |= (i_sat_offset & 0x3F);
}

void set_rxtx_group_id(const uint8_t i_grp_id, uint64_t * o_addr)
{
    *o_addr &= 0xFFFFF81FFFFFFFFFULL;
    *o_addr |= (i_grp_id & 0x3FULL) << 37;
}

uint8_t get_sat_id(const uint64_t i_addr)
{
    return ((i_addr >> 6) & 0xF);
}

uint8_t get_chiplet_id(const uint64_t i_addr)
{
    return ((i_addr >> 24) & 0x3F);
}

uint8_t get_ring(const uint64_t i_addr)
{
    return ((i_addr >> 10) & 0x3F);
}

uint8_t get_sat_offset(const uint64_t i_addr)
{
    return (i_addr & 0x3F);
}

uint8_t get_rxtx_group_id(const uint64_t i_addr)
{
    return (i_addr >> 37) & 0x3F;
}

