/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwpf/plat/include/utils.H $                           */
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
 *  @file utils.H
 *
 *  @brief Defines common utility elements for FAPI2 use.
 */

#ifndef FAPI2_UTILS_H_
#define FAPI2_UTILS_H_

#ifdef __ASSEMBLER__

#ifndef ULL
#define ULL(x) x
#endif

#else

#ifndef ULL
#define ULL(x) x##ull

#endif

#endif // __ASSEMBLER

/// Create a multi-bit mask of \a n bits starting at bit \a b
#define BITS(b, n) ((ULL(0xffffffffffffffff) << (64 - (n))) >> (b))

/// Create a single bit mask at bit \a b
#define BIT(b) BITS((b), 1)


#endif // FAPI2_UTILS_H_
