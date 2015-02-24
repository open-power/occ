/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/firdata/firData.H $                                   */
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

#ifndef __firData_h
#define __firData_h

#include "native.h"

/** @brief  Reads the register list from the HOMER data, SCOMs hardware, and
 *          stores the register values in PNOR.
 *  @param  i_hBuf      SRAM pointer to the beginning of the HOMER data buffer.
 *                      This should contain the FIR data information provided by
 *                      PRD that is used to define which registers the OCC will
 *                      need to SCOM.
 *  @param  i_hBufSize  Total size of the HOMER data buffer.
 *  @param  i_pBuf      SRAM pointer to the beginning of the PNOR data buffer.
 *                      This will be used by this function as a temporary area
 *                      of memory to store the PNOR data before writing that
 *                      data to the PNOR.
 *  @param  i_pBufSize  Total size of the PNOR data buffer.
 *  @return Non-SUCCESS if an internal function fails. SUCCESS otherwise.
 */
int32_t FirData_captureCsFirData( uint8_t * i_hBuf, uint32_t i_hBufSize,
                                  uint8_t * i_pBuf, uint32_t i_pBufSize );

#endif /* __firData_h */
