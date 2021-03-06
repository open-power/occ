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
  @file cftocctool.C
  @brief Main Program entry point for cmtDllClient Application
*/

#ifndef __CFT_TMGT_H
#define __CFT_TMGT_H

#include "cft_utils.H"


enum htmgt_command
{
    HTMGT_QUERY_STATE =     0x01,
    HTMGT_INTERNAL_FLAGS =  0x02,
    HTMGT_SEND_OCC_CMD =    0x03
};


uint32_t occ_cmd_via_htmgt(const uint8_t i_occNum,
                           const uint8_t i_cmd,
                           const uint8_t* i_cmd_data,
                           const uint16_t i_len,
                           uint8_t *& o_responseData,
                           uint32_t & o_responseSize);

uint32_t occ_cmd_via_tmgt(const uint8_t i_occNum,
                          const uint8_t i_cmd,
                          const uint8_t* i_cmd_data,
                          const uint16_t i_len,
                          uint8_t *& o_responseData,
                          uint32_t & o_responseSize);


uint32_t send_tmgt_command(const uint8_t i_cmd,
                           const uint8_t* i_cmd_data,
                           const uint16_t i_len);



#endif
