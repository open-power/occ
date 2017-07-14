/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/firdata/scom_addr_util.h $                        */
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
#ifndef __scom_addr_util_h
#define __scom_addr_util_h

#include <stdint.h>

//This file is a result of de-classing the p9_scom_addr that currently
//lives in EKB: chips/p9/common/scominfo/p9_scom_addr.H

// @brief Modify SCOM address, update satellite ID field
// @param[in] i_sat_id Satellite ID value to write
// @retval none
//
void set_sat_id(const uint8_t i_sat_id, uint64_t * o_addr);

// @brief Modify SCOM address, update pervasive chiplet ID
// @param[in] i_chiplet_id Chiplet ID value to write
// @retval none
//
void set_chiplet_id(const uint8_t i_chiplet_id, uint64_t * o_addr);

// @brief Modify SCOM address, update ring field value
// @param[in] i_ring Ring field value to write
// @retval none
void set_ring(const uint8_t i_ring, uint64_t * o_addr);

// @brief Modify SCOM address, update satellite offset field
// @param[in] i_sat_offset Satellite offset value to write
// @retval none
void set_sat_offset(const uint8_t i_sat_offset, uint64_t * o_addr);

// @brief Modify SCOM address, update the RX or TX Group ID
// @param[in] i_grp_id Group id to set
// @retval none
void set_rxtx_group_id(const uint8_t i_grp_id, uint64_t * o_addr);

// @brief Extract satellite ID field from SCOM address
// @retval uint8_t Satellite ID field value
uint8_t get_sat_id(const uint64_t i_addr);

// @brief Extract pervasive chiplet ID from SCOM address
// @retval uint8_t Pervasive chiplet ID value
uint8_t get_chiplet_id(const uint64_t i_addr);

// @brief Extract ring field from SCOM address
// @retval uint8_t Ring field value
uint8_t get_ring(const uint64_t i_addr);

// @brief Extract satellite register offset field from SCOM address
// @retval uint8_t Satellite register offset field value
uint8_t get_sat_offset(const uint64_t i_addr);

// @brief Extract the RX or TX Group ID of an indirect scom address
// @retval uint8_t Satellite register offset field value
uint8_t get_rxtx_group_id(const uint64_t i_addr);


#endif
