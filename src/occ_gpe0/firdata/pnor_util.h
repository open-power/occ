/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/firdata/pnor_util.h $                            */
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

#ifndef __pnor_util_h
#define __pnor_util_h

#include <homerData_common.h>

/**
 * @brief  Writes a buffer containing the FIR data to PNOR (adding ECC).
 * @param  i_pnorInfo Information regarding PNOR location, size, etc.
 * @param  i_buf      Data buffer (no ECC included).
 * @param  i_bufSize  Size of the data buffer.
 * @return Non-SUCCESS, if the write fails. SUCCESS, otherwise.
 */
int32_t PNOR_writeFirData( HOMER_PnorInfo_t i_pnorInfo,
                           uint8_t * i_buf, uint32_t i_bufSize );

#endif /* __pnor_util_h */
