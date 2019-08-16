/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/baselib/ppe42_math.h $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2019                             */
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
#ifndef _PPE42_MATH_H
#define _PPE42_MATH_H

#define MIN(X, Y)                               \
    ({                                          \
        typeof (X) __x = (X);                   \
        typeof (Y) __y = (Y);                   \
        (__x < __y) ? __x : __y; })

#define MAX(X, Y)                               \
    ({                                          \
        typeof (X) __x = (X);                   \
        typeof (Y) __y = (Y);                   \
        (__x > __y) ? __x : __y;                \
    })


// Provide a way to use the native 16-bit multiply instruction
// Unfortunately the compiler does not know to use it
/// Signed 16 bit multiply, 32 bit product
#define muls16(x,y) \
    ({\
        int32_t __x = (x); \
        int32_t __y = (y); \
        int32_t __z; \
        asm volatile ("mullhw %0,%1,%2" : "=r" (__z) : "r" (__x), "r" (__y) : "cc"); \
        __z;})

/// Unsigned 16 bit multiply, 32 bit product
#define mulu16(x,y) \
    ({\
        uint32_t __x = (x); \
        uint32_t __y = (y); \
        uint32_t __z; \
        asm volatile("mullhwu %0,%1,%2" : "=r" (__z) : "r" (__x), "r" (__y) : "cc"); \
        __z;})

#ifdef __cplusplus
extern "C"
{
#endif
// These names are emitted by the ppe42 compiler.
// Implement the ones that will be used.

// 64 bit unsigned divide. Implement if needed
// unsigned long long __udivdi3(unsigned long long a, unsigned long long b);

/** 32 bit unsigned divide
 *  @param[in] Dividend
 *  @param[in] Divisor
 *  @return quotient
 */
unsigned long __udivsi3(unsigned long a, unsigned long b);

/** 32 bit signed divide
 * @param[in] Dividend
 * @param[in] Divisor
 * @return quotient
 */
int __divsi3(int _a, int _b);

/** 32 bit unsigned modulus
 * @param[in] Dividend
 * @param[in] Divisor
 * @return modulus
 */
unsigned long __umodsi3(unsigned long a, unsigned long b);


// Only PPE42A instuction set does not have native 32 bit mult.
#if defined(__PPE42A__)

/** 32 bit unsigned multiply
 * @param[in] multiplier
 * @param[in] multiplier
 * @return product
 */
unsigned long __umulsi3(unsigned long _a, unsigned long _b);

/** 32 bit signed multiply
 * @param[in] multiplier
 * @param[in] multiplier
 * @return product
 */
unsigned int __mulsi3(unsigned int _a, unsigned int _b);

#endif // __PPE42A__

/** 64 bit signed multiply
 * @param[in] multiplier
 * @param[in] multiplier
 * @return product
 */
unsigned long long __muldi3(unsigned long long _a, unsigned long long _b);

#ifdef __cplusplus
};
#endif

#endif
