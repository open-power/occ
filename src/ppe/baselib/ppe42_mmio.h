/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/baselib/ppe42_mmio.h $                                */
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
#if !defined(__PPE42_MMIO_H__)
#define __PPE42_MMIO_H__

/// 8-bit MMIO Write
#define out8(addr, data) \
    do {*(volatile uint8_t *)(addr) = (data);} while(0)

/// 8-bit MMIO Read
#define in8(addr) \
    ({uint8_t __data = *(volatile uint8_t *)(addr); __data;})

/// 16-bit MMIO Write
#define out16(addr, data) \
    do {*(volatile uint16_t *)(addr) = (data);} while(0)

/// 16-bit MMIO Read
#define in16(addr) \
    ({uint16_t __data = *(volatile uint16_t *)(addr); __data;})

/// 32-bit MMIO Write
#define out32(addr, data) \
    do {*(volatile uint32_t *)(addr) = (data);} while(0)

/// 32-bit MMIO Read
#define in32(addr) \
    ({uint32_t __data = *(volatile uint32_t *)(addr); __data;})

/// 64-bit MMIO Write
#define out64(addr, data) \
    {\
        uint64_t __d = (data); \
        uint32_t* __a = (uint32_t*)(addr); \
        asm volatile \
        (\
         "stvd %1, %0 \n" \
         : "=o"(*__a) \
         : "r"(__d) \
        ); \
    }

/// 64-bit MMIO Read
#define in64(addr) \
    ({\
        uint64_t __d; \
        uint32_t* __a = (uint32_t*)(addr); \
        asm volatile \
        (\
         "lvd %0, %1 \n" \
         :"=r"(__d) \
         :"o"(*__a) \
        ); \
        __d; \
    })


#endif
