/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/lfsr.c $                                    */
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
// $Id$

/// \file lfsr.c
/// \brief 

#include <stdint.h>
#include "lfsr.h"

// Parity for 4-bit numbers
static uint8_t S_parity4[16] = {
    0, 1, 1, 0,
    1, 0, 0, 1,
    1, 0, 0, 1,
    0, 1, 1, 0
};

// 64, 63, 61, 60 LFSR.  The routine is coded with the uint8_t casting to help
// the compiler generate more efficient code.

void
_lfsr64(uint64_t* io_seed)
{
    *io_seed = (*io_seed << 1) |
        S_parity4[(uint8_t)((*io_seed >> 59) & 0x3) | 
                  (uint8_t)((*io_seed >> 60) & 0xc)];
}
