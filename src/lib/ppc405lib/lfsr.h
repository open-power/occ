/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/lfsr.h $                                    */
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
#ifndef __LFSR_H__
#define __LFSR_H__

/// \file lfsr.h
/// \brief Linear-Feedback Shift Register Implementations
///
/// The 32- and 64-bit pseudo-random number generators in this library are of
/// the linear-conguential type. These maximal-length LFSR pseudo-random
/// sequence generators are also provided.

/// 64-bit LFSR
///
/// \param[in,out] io_seed The input seed is converted in one step to the
/// output seed.
///
/// This 64-bit LFSR uses taps 64, 63, 61, and 60. In big-endian numbering
/// these are bits 0, 1, 3 and 4.  This LFSR is also implemented for the PORE
/// engines in the file pore_rand.pS.
void
_lfsr64(uint64_t* io_seed);

#endif // __LFSR_H__
