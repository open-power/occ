/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/include/proc_hcd_common.H $                           */
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
#ifndef __PROC_HCD_COMMON_H
#define __PROC_HCD_COMMON_H

/// \file proc_hcd_common.H
/// \brief Standard bit-manipulation macros (C and Assembler) for PORE code

/// \defgroup be64_bits Bit manipulation for 64-bit Big-Endian values
///
/// \note These macros only work in the assembler context because we build our
/// assemblers to do 64-bit arithmetic, which is required for PORE assembly.
///
/// @{

/// Create a multi-bit mask of \a n bits starting at bit \a b
#define BITS(b, n) ((ULL(0xffffffffffffffff) << (64 - (n))) >> (b))

/// Create a single bit mask at bit \a b
#define BIT(b) BITS((b), 1)

#endif  // __PROC_HCD_COMMON_H
