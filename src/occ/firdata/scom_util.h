/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/firdata/scom_util.H $                                 */
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

/* Interfaces to read SCOM registers */

/*#include <stdint.h> */
#include <native.h>
#include <scom_trgt.h>

#define SCOMFAIL 0xDEADBEEF

/** @brief  Performs a hardware scom on a regular register.
 *  @param  i_trgt The SCOM target.
 *  @param  i_addr 32-bit SCOM address.
 *  @param  o_val  64-bit returned value.
 *  @return Non-SUCCESS if the SCOM fails. SUCCESS otherwise.
 */
int32_t SCOM_getScom( SCOM_Trgt_t i_trgt, uint32_t i_addr, uint64_t * o_val );

/** @brief  Performs a hardware scom on an indirect-SCOM register.
 *  @param  i_trgt The SCOM target.
 *  @param  i_addr 64-bit SCOM address.
 *  @param  o_val  32-bit returned value.
 *  @return Non-SUCCESS if the SCOM fails. SUCCESS otherwise.
 */
int32_t SCOM_getIdScom( SCOM_Trgt_t i_trgt, uint64_t i_addr, uint32_t * o_val );

