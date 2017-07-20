/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/firdata/fsi.H $                                       */
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

/* Interfaces to read/write FSI registers */

#include <scom_trgt.h>

/**
 * @brief  Read a FSI register.
 * @param  i_trgt Chip/unit to read from.
 * @param  i_addr FSI address to read, relative to slave's base address.
 * @param  o_val  Returned value.
 * @return Non-SUCCESS if an internal function fails. SUCCESS otherwise.
 */
int32_t getfsi( SCOM_Trgt_t* i_trgt, uint32_t i_addr, uint32_t * o_val );

/**
 * @brief  Write a FSI register.
 * @param  i_trgt Chip/unit to write to.
 * @param  i_addr FSI address to write, relative to slave's base address.
 * @param  o_val  Value to write.
 * @return Non-SUCCESS if an internal function fails. SUCCESS otherwise.
 */
int32_t putfsi( SCOM_Trgt_t* i_trgt, uint32_t i_addr, uint32_t i_val );

