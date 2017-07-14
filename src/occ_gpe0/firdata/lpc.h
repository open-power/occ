/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/firdata/lpc.h $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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

#ifndef _LPC_H
#define _LPC_H

#include <native.h>
#include <errl.h>

/**
 * @enum LPC::TransType
 * @brief LPC Transaction Types
 */
typedef enum  {
    LPC_TRANS_IO = 0,  /* LPC IO Space */
    LPC_TRANS_FW = 1,  /* LPC Firmware Space */
} LpcTransType;

errorHndl_t lpc_read( LpcTransType i_type,
                      uint32_t i_addr,
                      uint8_t* o_data,
                      size_t i_size );

errorHndl_t lpc_write( LpcTransType i_type,
                       uint32_t i_addr,
                       uint8_t* i_data,
                       size_t i_size );

uint32_t checkAddr(LpcTransType i_type,
                   uint32_t i_addr);


#endif
