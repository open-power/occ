/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/tools/cft/cftocctool.C $                                  */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2020                             */
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
  @file cft_occ.H
  @brief OCC specific structures and function prototypes
*/

#ifndef __CFT_OCC_H
#define __CFT_OCC_H

const char * getStateString(const uint8_t i_state);
const char * getPollStatus(const uint8_t i_flags);
const char * getPollExtStatus(const uint8_t i_flags);
const char * getFruString(const uint8_t i_fru);
const char * getApssFuncName(uint8_t i_funcId);
const char * getHistoryName(uint8_t id);
uint32_t send_occ_command(const uint8_t i_occ,
                          const uint8_t i_cmd,
                          const uint8_t* i_cmd_data,
                          const uint16_t i_len);

#endif
