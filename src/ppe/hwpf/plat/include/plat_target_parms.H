/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwpf/plat/include/plat_target_parms.H $               */
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
 * @file plat_ppe_target.H
 * @brief Definitions for fapi2 PPE targets
 */

#ifndef __FAPI2_PPE_TARGET_PARMS__
#define __FAPI2_PPE_TARGET_PARMS__


#ifndef __ASSEMBLER__

const uint64_t CHIPLET_COUNT        = 0x38;

const uint64_t CHIP_TARGET_COUNT    = 1;
const uint64_t CHIP_TARGET_OFFSET   = 0;

const uint64_t PERV_TARGET_OFFSET   = CHIP_TARGET_OFFSET + CHIP_TARGET_COUNT;
const uint64_t PERV_CHIPLET_OFFSET  = 0x1; 
const uint64_t PERV_TARGET_COUNT    = 15;

const uint64_t EQ_TARGET_OFFSET     = PERV_TARGET_OFFSET + PERV_TARGET_COUNT;
const uint64_t EQ_CHIPLET_OFFSET    = 0x10;  
const uint64_t EQ_TARGET_COUNT      = 6; 

const uint64_t CORE_TARGET_OFFSET   = EQ_TARGET_OFFSET + EQ_TARGET_COUNT; 
const uint64_t CORE_CHIPLET_OFFSET  = 0x20;
const uint64_t CORE_TARGET_COUNT    = 24;

const uint64_t EX_TARGET_OFFSET     = CORE_TARGET_OFFSET + CORE_TARGET_COUNT;
const uint64_t EX_CHIPLET_OFFSET    = 0x10;  
const uint64_t EX_TARGET_COUNT      = 12;


const uint64_t TARGET_COUNT         = EX_TARGET_OFFSET + EX_TARGET_COUNT;

#endif

#endif  // __FAPI2_PPE_TARGET_PARMS__
