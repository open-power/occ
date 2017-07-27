/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/math.c $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2017                        */
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

// 64 bit divide.  Note: TBD add when needed
//unsigned long long __udivdi3(unsigned long long a, unsigned long long b)
//{
//    unsigned long long c = 0;
//    return c;
//}


#ifdef PSTATE_GPE
#if (NIMBUS_DD_LEVEL != 10)

#include "ocb_register_addresses.h"
#define out64(addr, data) \
    {\
        unsigned long long __d = (data); \
        unsigned long* __a = (unsigned long*)(addr); \
        asm volatile \
        (\
         "stvd %1, %0 \n" \
         : "=o"(*__a) \
         : "r"(__d) \
        ); \
    }

#define in64(addr) \
    ({\
        unsigned long long __d; \
        unsigned long* __a = (unsigned long*)(addr); \
        asm volatile \
        (\
         "lvd %0, %1 \n" \
         :"=r"(__d) \
         :"o"(*__a) \
        ); \
        __d; \
    })


unsigned long udivmodsi4(unsigned long long _a,
                         unsigned long _mod)
{

    out64(OCB_DERP, _a);

    do
    {
        _a = in64(OCB_DORP);
    }
    while((~_a) == 0);

    if(_mod)
    {
        return (unsigned long)_a;
    }

    return (unsigned long)(_a >> 32);
}

unsigned long __udivsi3(unsigned long _a, unsigned long _b)
{
    unsigned long long v =
        ((unsigned long long)_a) << 32 |
        ((unsigned long long)_b);

    return udivmodsi4(v, 0);
}

unsigned long __umodsi3(unsigned long _a, unsigned long _b)
{
    unsigned long long v =
        ((unsigned long long)_a) << 32 |
        ((unsigned long long)_b);
    return udivmodsi4(v, 1);
}
#endif
#endif

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

    int c = (int)__udivsi3((unsigned long)_a, (unsigned long)_b);

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

