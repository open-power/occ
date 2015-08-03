/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/ppe42_gcc.h $                                */
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
#ifndef __PPE42_GCC_H__
#define __PPE42_GCC_H__
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppe42_gcc.h
/// \brief 32-bit functions expected by GCC

#ifndef __ASSEMBLER__

#include <stdint.h>

/// A 64-bit unsigned integer type

typedef union {
    uint64_t value;
    uint32_t word[2];
} Uint64;

/// A 64-bit signed integer type

typedef union {
    int64_t value;
    int32_t word[2];
} Int64;

uint64_t
__lshrdi3(uint64_t x, int i);

uint64_t
__ashldi3(uint64_t x, int i);

uint64_t
__ashrdi3(uint64_t x, int i);

int
__popcountsi2(uint32_t x);

int
__popcountdi2(uint64_t x);

/// Unsigned 64/64 bit divide, returning quotient and remainder via pointers.

void
__ppe42_udiv64(uint64_t u, uint64_t v, uint64_t *q, uint64_t *r);

/// Signed 64/64 bit divide, returning quotient and remainder via pointers.

void
__ppe42_sdiv64(int64_t u, int64_t v, int64_t *q, int64_t *r);

uint64_t 
__udivdi3(uint64_t u, uint64_t v);

int64_t
__divdi3(int64_t u, int64_t v);

int64_t
__moddi3(int64_t u, int64_t v);

uint64_t 
__umoddi3(uint64_t u, uint64_t v);

int
__ucmpdi2(uint64_t a, uint64_t b);

#endif /* __ASSEMBLER__ */

#endif /* __PPE42_GCC_H__ */
