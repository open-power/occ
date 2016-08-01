/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ppc32/ppc32_gcc.c $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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

/// \file ppc32_gcc.h
/// \brief 32-bit PowerPC functions expected by GCC
///
/// GCC expects certain built-in functions to be defined in the environment.
/// Since SSX applications are statically linked, we must define these
/// functions ourselves to avoid a static link with the GCC libraries, which
/// would legaly require us to distribute (at least) the binary forms of SSX
/// applications.
///
/// We obviously had to look at some GCC library code to understand the
/// specifications of these routines.  However, all of the code here is new -
/// no structure definitions or lines of executable code were copied from the
/// GCC sources.

#include "ssx.h"
#include "ppc32_gcc.h"

/// A 64-bit logical right shift.
///
/// Note that shifts with negative shift counts or shifts with shift counts
/// longer than 63 bits are undefined.

uint64_t
__lshrdi3(uint64_t x, int i)
{
    Uint64 input, result;

    if (i == 0)
    {
        return x;
    }

    input.value = x;

    if (i >= 32)
    {
        result.word[0] = 0;
        result.word[1] = input.word[0] >> (i - 32);
    }
    else
    {
        result.word[0] = input.word[0] >> i;
        result.word[1] = (input.word[1] >> i) | (input.word[0] << (32 - i));
    }

    return result.value;
}


/// A 64 bit arithmetic left shift.
///
/// Note that shifts with negative shift counts or shifts with shift counts
/// longer than 63 bits are undefined.

uint64_t
__ashldi3(uint64_t x, int i)
{
    Uint64 input, result;

    if (i == 0)
    {
        return x;
    }

    input.value = x;

    if (i >= 32)
    {
        result.word[1] = 0;
        result.word[0] = input.word[1] << (i - 32);
    }
    else
    {
        result.word[1] = input.word[1] << i;
        result.word[0] = (input.word[0] << i) | (input.word[1] >> (32 - i));
    }

    return result.value ;

}


/// A 64 bit arithmetic right shift.
///
/// Note that shifts with negative shift counts or shifts with shift counts
/// longer than 63 bits are undefined.

uint64_t
__ashrdi3(uint64_t x, int i)
{
    Int64 input, result;

    if (i == 0)
    {
        return x;
    }

    input.value = x;

    if (i >= 32)
    {
        result.word[0] = input.word[0] >> 31;
        result.word[1] = input.word[0] >> (i - 32);
    }
    else
    {
        result.word[0] = input.word[0] >> i;
        result.word[1] =
            (((uint32_t)input.word[1]) >> i) |
            (input.word[0] << (32 - i));
    }

    return result.value ;

}


/// 32-bit Population count

// This is a well-known divide-and-conquer algorithm, e.g. look on Wikipedia
// under "Hamming Weight". The idea is to compute sums of adjacent bit
// segments in parallel, in place. This compiles to 22 PPC405 instructions.

int
__popcountsi2(uint32_t x)
{
    uint32_t m1 = 0x55555555;
    uint32_t m2 = 0x33333333;
    uint32_t m4 = 0x0f0f0f0f;
    x -= (x >> 1) & m1;            /* Sum pairs of bits */
    x = (x & m2) + ((x >> 2) & m2);/* Sum 4-bit segments */
    x = (x + (x >> 4)) & m4;       /* Sum 8-bit segments */
    x += x >>  8;                  /* Sum 16-bit segments */
    return (x + (x >> 16)) & 0x3f; /* Final sum */
}


/// 64-bit Population count

int
__popcountdi2(uint64_t x)
{
    return __popcountsi2(x >> 32) + __popcountsi2(x & 0xffffffff);
}


// 64-bit divides
//
// For the unsigned case, note that divide by 0 returns quotient = remainder =
// 0.
//
// For the signed case, in general we perform the division on the absolute
// values and fix the signs of the quotient and remainder at the end.
//
// For the signed case, the convention in other libraries seems to be to
// ignore the case of the most-negative integer.  Although it seems "wrong" to
// return the wrong answer when the right answer can be easily computed, in
// the interest of code size we follow the convention here and ignore the most
// negative integer.
//
// The assembler routine __ppc32_udiv64() assembles to 304 bytes.  The full C
// routine __ppc_sdiv64 compiles to 416 bytes with the most-negative checks,
// but only 184 bytes as configured here.

#if 0
    // For the signed cases, we need to handle the special case that the dividend
    // or divisor is the most negative integer.
    //
    // If the dividend is the most negative integer, then dividing this integer by
    // -1 would overflow as a positive quotient, so we set quotient and remainder
    // to 0 in this case.  For divide by 1, the quotient is the most negative
    // integer. Otherwise we adjust the dividend by the absolute value of the
    // divisor, then fix up the quotient later by adding or subtracting 1.
    //
    // If the divisor is the most negative integer, then the quotient is always 0
    // unless the dividend is also the most negative integer, in which case the
    // quotient is 1 and the remainder is 0.
    //
#endif

uint64_t
__udivdi3(uint64_t u, uint64_t v)
{
    uint64_t quotient, remainder;

    __ppc32_udiv64(u, v, &quotient, &remainder);
    return quotient;
}


uint64_t
__umoddi3(uint64_t u, uint64_t v)
{
    uint64_t quotient, remainder;

    __ppc32_udiv64(u, v, &quotient, &remainder);
    return remainder;
}


#if 0
    #define INT64_T_MIN ((int64_t)(0x8000000000000000ull))
#endif

void
__ppc32_sdiv64(int64_t u, int64_t v,
               int64_t* quotient, int64_t* remainder)
{
    int q_negate, r_negate;
    uint64_t uu, uv;
#if 0
    int fixup = 0;
#endif

    q_negate = (u < 0) ^ (v < 0);
    r_negate = (u < 0);
    uu = (u < 0 ? -u : u);
    uv = (v < 0 ? -v : v);

#if 0

    if (u == INT64_T_MIN)
    {
        if (v == -1)
        {
            *quotient = 0;
            *remainder = 0;
            return;
        }
        else if (v == 1)
        {
            *quotient = INT64_T_MIN;
            *remainder = 0;
            return;
        }
        else if (v == INT64_T_MIN)
        {
            *quotient = 1;
            *remainder = 0;
            return;
        }
        else
        {
            fixup = 1;
            u += (v < 0 ? -v : v);
        }
    }
    else if (v == INT64_T_MIN)
    {
        *quotient = 0;
        *remainder = u;
        return;
    }

#endif

    __ppc32_udiv64(uu, uv, (uint64_t*)quotient, (uint64_t*)remainder);

#if 0

    if (fixup)
    {
        *quotient += 1;
    }

#endif

    if (q_negate)
    {
        *quotient = -(*quotient);
    }

    if (r_negate)
    {
        *remainder = -(*remainder);
    }
}


int64_t
__divdi3(int64_t u, int64_t v)
{
    int64_t quotient, remainder;

    __ppc32_sdiv64(u, v, &quotient, &remainder);
    return quotient;
}


int64_t
__moddi3(int64_t u, int64_t v)
{
    int64_t quotient, remainder;

    __ppc32_sdiv64(u, v, &quotient, &remainder);
    return remainder;
}


/// 64-bit unsigned compare as a function, returning 0 (<), 1 (==) or 2 (>).

int
__ucmpdi2(uint64_t i_a, uint64_t i_b)
{
    Uint64 a, b;
    int rv;

    a.value = i_a;
    b.value = i_b;

    if (a.word[0] < b.word[0])
    {
        rv = 0;
    }
    else if (a.word[0] > b.word[0])
    {
        rv = 2;
    }
    else if (a.word[1] < b.word[1])
    {
        rv = 0;
    }
    else if (a.word[1] > b.word[1])
    {
        rv = 2;
    }
    else
    {
        rv = 1;
    }

    return rv;
}








