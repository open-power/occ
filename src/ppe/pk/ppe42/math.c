/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/math.c $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016                             */
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
#include "ppe42math.h"

#ifdef __cplusplus
extern "C"
{
#endif

unsigned long
udivmodsi4(unsigned long num, unsigned long den, int modwanted)
{
    unsigned long bit = 1;
    unsigned long res = 0;

    while (den < num && bit && !(den & (1L << 31)))
    {
        den <<= 1;
        bit <<= 1;
    }

    while (bit)
    {
        if (num >= den)
        {
            num -= den;
            res |= bit;
        }

        bit >>= 1;
        den >>= 1;
    }

    if (modwanted)
    {
        return num;
    }

    return res;
}

// 64 bit divide.  Note: TBD add when needed
//unsigned long long __udivdi3(unsigned long long a, unsigned long long b)
//{
//    unsigned long long c = 0;
//    return c;
//}

// 32 bit unsigned integer divide
unsigned long __udivsi3(unsigned long a, unsigned long b)
{
    return udivmodsi4(a, b, 0);
}

// 32 bit modulus
unsigned long __umodsi3(unsigned long a, unsigned long b)
{
    return udivmodsi4(a, b, 1);
}

// 32 bit signed divide
int __divsi3(int _a, int _b)
{
    register unsigned long neg = 0;

    if(_a & 0x80000000)
    {
        neg = !neg;
        _a = (~_a) + 1;
    }

    if(_b & 0x80000000)
    {
        _b = (~_b) + 1;
        neg = !neg;
    }

    int c = __udivsi3((unsigned long)_a, (unsigned long)_b);

    if(neg)
    {
        c = (~c) + 1;
    }

    return c;
}

// 32 bit unsigned mutiply
unsigned long __umulsi3(unsigned long _a, unsigned long _b)
{
    register unsigned long a = _a;
    register unsigned long b = _b;
    register unsigned long c;
    register unsigned long d;
    asm volatile("mullhwu %0, %1, %2" : "=r"(c) : "r"(a), "r"(b));
    d = c;
    c = a >> 16;
    asm volatile("mullhwu %0, %1, %2" : "=r"(c) : "r"(c), "r"(b));
    d += (c << 16);
    c = b >> 16;
    asm volatile("mullhwu %0, %1, %2" : "=r"(c) : "r"(c), "r"(a));
    d += (c << 16);
    return d;
}

// 32 bit signed multiply
unsigned int __mulsi3(unsigned int _a, unsigned int _b)
{
    register unsigned long neg = 0;
    register unsigned long a = _a;
    register unsigned long b = _b;
    register unsigned long c;
    register unsigned long d;

    if(a & 0x80000000)
    {
        a = (~a) + 1;
        neg = !neg;
    }

    if(b & 0x80000000)
    {
        b = (~b) + 1;
        neg = !neg;
    }

    asm volatile("mullhwu %0, %1, %2" : "=r"(c) : "r"(a), "r"(b));
    d = c;
    c = a >> 16;
    asm volatile("mullhwu %0, %1, %2" : "=r"(c) : "r"(c), "r"(b));
    d += (c << 16);
    c = b >> 16;
    asm volatile("mullhwu %0, %1, %2" : "=r"(c) : "r"(c), "r"(a));
    d += (c << 16);

    if(neg)
    {
        d = (~d) + 1;
    }

    return d;
}

// 64 bit signed multiply
unsigned long long __muldi3(unsigned long long _a, unsigned long long _b)
{
    unsigned long long sum = 0;

    while(_a)
    {
        if(_a & 1)
        {
            sum += _b;
        }

        _a >>= 1;
        _b <<= 1;
    }

    return sum;
}

//float __mulsf3(float _a , float _b)
//{
//    // floating point math
//    return 0.0;
//}

//float __subsf3(float _a, float _b)
//{
//    // floating point sub
//    return 0.0;
//}

//unsigned long __fixsfsi (float _a)
//{
//    // float to int
//    return 0;
//}

#ifdef __cplusplus
};
#endif


