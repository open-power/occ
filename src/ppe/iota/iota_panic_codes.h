/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/iota/iota_panic_codes.h $                             */
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
#ifndef __IOTA_PANIC_CODES_H__
#define __IOTA_PANIC_CODES_H__

// On PPE42, PANIC codes are stored as part of the trap word instruction.
// tw 31, RA, RB  Where RA and RB would used to encode the trap code.
// There are 16 valid gprs on PP42, so this gives 256 possible trap codes.
// The trap code is defined as a two byte code defined as 0xYYZZ where YY
// is encoded into the RA field and ZZ is incoded into the RB field
// YY and ZZ are limited to the values:
//   00,01,02,03,04,05,06,07,08,09,0a,0d,1c,1d,1e,1f (valid gpr ids)
//
// To add a new panic code, select an unused values and rename it.
// This enum contains all the valid values that can be used. Using a
// panic code not in this list will result in a compiler/assembler error.
#ifndef __ASSEMBLER__

typedef enum
{

#ifdef PLATFORM_PANIC_CODES_H
#include PLATFORM_PANIC_CODES_H
#endif


    IOTA_UNUSED_0000 = 0x0000,
    IOTA_NULLPTR_TASK = 0x0001,
    IOTA_MACHINE_STATE_STACK_OVERFLOW = 0x0002,
    IOTA_MACHINE_STATE_STACK_UNDERFLOW = 0x0003,
    IOTA_MACHINE_CHECK_PANIC = 0x0004,
    IOTA_DATA_STORAGE_PANIC  = 0x0005,
    IOTA_INSTRUCTION_STORAGE_PANIC = 0x0006,
    IOTA_DATA_ALIGNMENT_PANIC = 0x0007,
    IOTA_ILLEGAL_INSTRUCTION = 0x0008,
    IOTA_SOFTWARE_HALT = 0x0009,
    IOTA_DEC_NOT_SUPPORTED = 0x000a,
    IOTA_WATCHDOG_NOT_SUPPORTED = 0x000d,
    IOTA_EXECUTION_STACK_OVERFLOW = 0x001c,
    IOTA_UNUSED_001d = 0x001d,
    IOTA_UNUSED_001e = 0x001e,
    IOTA_UNUSED_001f = 0x001f,


    IOTA_UIH_DISABLED_FIRED = 0x0100,
    IOTA_UIH_STACK_OVERFLOW = 0x0101,
    IOTA_UIH_STACK_UNDERFLOW = 0x0102,
    IOTA_UIH_PHANTOM_INTERRUPT = 0x0103,
    IOTA_UNUSED_0104 = 0x0104,
    IOTA_UNUSED_0105 = 0x0105,
    IOTA_UNUSED_0106 = 0x0106,
    IOTA_UNUSED_0107 = 0x0107,
    IOTA_UNUSED_0108 = 0x0108,
    IOTA_UNUSED_0109 = 0x0109,
    IOTA_UNUSED_010a = 0x010a,
    IOTA_UNUSED_010d = 0x010d,
    IOTA_UNUSED_011c = 0x011c,
    IOTA_UNUSED_011d = 0x011d,
    IOTA_UNUSED_011e = 0x011e,
    IOTA_UNUSED_011f = 0x011f,


    IOTA_UNUSED_0200 = 0x0200,
    IOTA_UNUSED_0201 = 0x0201,
    IOTA_UNUSED_0202 = 0x0202,
    IOTA_UNUSED_0203 = 0x0203,
    IOTA_UNUSED_0204 = 0x0204,
    IOTA_UNUSED_0205 = 0x0205,
    IOTA_UNUSED_0206 = 0x0206,
    IOTA_UNUSED_0207 = 0x0207,
    IOTA_UNUSED_0208 = 0x0208,
    IOTA_UNUSED_0209 = 0x0209,
    IOTA_UNUSED_020a = 0x020a,
    IOTA_UNUSED_020d = 0x020d,
    IOTA_UNUSED_021c = 0x021c,
    IOTA_UNUSED_021d = 0x021d,
    IOTA_UNUSED_021e = 0x021e,
    IOTA_UNUSED_021f = 0x021f,


    IOTA_UNUSED_0300 = 0x0300,
    IOTA_UNUSED_0301 = 0x0301,
    IOTA_UNUSED_0302 = 0x0302,
    IOTA_UNUSED_0303 = 0x0303,
    IOTA_UNUSED_0304 = 0x0304,
    IOTA_UNUSED_0305 = 0x0305,
    IOTA_UNUSED_0306 = 0x0306,
    IOTA_UNUSED_0307 = 0x0307,
    IOTA_UNUSED_0308 = 0x0308,
    IOTA_UNUSED_0309 = 0x0309,
    IOTA_UNUSED_030a = 0x030a,
    IOTA_UNUSED_030d = 0x030d,
    IOTA_UNUSED_031c = 0x031c,
    IOTA_UNUSED_031d = 0x031d,
    IOTA_UNUSED_031e = 0x031e,
    IOTA_UNUSED_031f = 0x031f,


    IOTA_UNUSED_0400 = 0x0400,
    IOTA_UNUSED_0401 = 0x0401,
    IOTA_UNUSED_0402 = 0x0402,
    IOTA_UNUSED_0403 = 0x0403,
    IOTA_UNUSED_0404 = 0x0404,
    IOTA_UNUSED_0405 = 0x0405,
    IOTA_UNUSED_0406 = 0x0406,
    IOTA_UNUSED_0407 = 0x0407,
    IOTA_UNUSED_0408 = 0x0408,
    IOTA_UNUSED_0409 = 0x0409,
    IOTA_UNUSED_040a = 0x040a,
    IOTA_UNUSED_040d = 0x040d,
    IOTA_UNUSED_041c = 0x041c,
    IOTA_UNUSED_041d = 0x041d,
    IOTA_UNUSED_041e = 0x041e,
    IOTA_UNUSED_041f = 0x041f,


    IOTA_UNUSED_0500 = 0x0500,
    IOTA_UNUSED_0501 = 0x0501,
    IOTA_UNUSED_0502 = 0x0502,
    IOTA_UNUSED_0503 = 0x0503,
    IOTA_UNUSED_0504 = 0x0504,
    IOTA_UNUSED_0505 = 0x0505,
    IOTA_UNUSED_0506 = 0x0506,
    OCCHW_INSTANCE_MISMATCH             = 0x0507,
    OCCHW_IRQ_ROUTING_ERROR             = 0x0508,
    OCCHW_XIR_INVALID_POINTER           = 0x0509,
    OCCHW_XIR_INVALID_GPE               = 0x050a,
    IOTA_UNUSED_050d = 0x050d,
    IOTA_UNUSED_051c = 0x051c,
    IOTA_UNUSED_051d = 0x051d,
    IOTA_UNUSED_051e = 0x051e,
    IOTA_UNUSED_051f = 0x051f,


    IOTA_UNUSED_0600 = 0x0600,
    IOTA_UNUSED_0601 = 0x0601,
    IOTA_UNUSED_0602 = 0x0602,
    IOTA_UNUSED_0603 = 0x0603,
    IOTA_UNUSED_0604 = 0x0604,
    IOTA_UNUSED_0605 = 0x0605,
    IOTA_UNUSED_0606 = 0x0606,
    IOTA_UNUSED_0607 = 0x0607,
    IOTA_UNUSED_0608 = 0x0608,
    IOTA_UNUSED_0609 = 0x0609,
    IOTA_UNUSED_060a = 0x060a,
    IOTA_UNUSED_060d = 0x060d,
    IOTA_UNUSED_061c = 0x061c,
    IOTA_UNUSED_061d = 0x061d,
    IOTA_UNUSED_061e = 0x061e,
    IOTA_UNUSED_061f = 0x061f,


    IOTA_UNUSED_0700 = 0x0700,
    IOTA_UNUSED_0701 = 0x0701,
    IOTA_UNUSED_0702 = 0x0702,
    IOTA_UNUSED_0703 = 0x0703,
    IOTA_UNUSED_0704 = 0x0704,
    IOTA_UNUSED_0705 = 0x0705,
    IOTA_UNUSED_0706 = 0x0706,
    IOTA_UNUSED_0707 = 0x0707,
    IOTA_UNUSED_0708 = 0x0708,
    IOTA_UNUSED_0709 = 0x0709,
    IOTA_UNUSED_070a = 0x070a,
    IOTA_UNUSED_070d = 0x070d,
    IOTA_UNUSED_071c = 0x071c,
    IOTA_UNUSED_071d = 0x071d,
    IOTA_UNUSED_071e = 0x071e,
    IOTA_UNUSED_071f = 0x071f,


    IOTA_UNUSED_0800 = 0x0800,
    IOTA_UNUSED_0801 = 0x0801,
    IOTA_UNUSED_0802 = 0x0802,
    IOTA_UNUSED_0803 = 0x0803,
    IOTA_UNUSED_0804 = 0x0804,
    IOTA_UNUSED_0805 = 0x0805,
    IOTA_UNUSED_0806 = 0x0806,
    IOTA_UNUSED_0807 = 0x0807,
    IOTA_UNUSED_0808 = 0x0808,
    IOTA_UNUSED_0809 = 0x0809,
    IOTA_UNUSED_080a = 0x080a,
    IOTA_UNUSED_080d = 0x080d,
    IOTA_UNUSED_081c = 0x081c,
    IOTA_UNUSED_081d = 0x081d,
    IOTA_UNUSED_081e = 0x081e,
    IOTA_UNUSED_081f = 0x081f,


    IOTA_UNUSED_0900 = 0x0900,
    IOTA_UNUSED_0901 = 0x0901,
    IOTA_UNUSED_0902 = 0x0902,
    IOTA_UNUSED_0903 = 0x0903,
    IOTA_UNUSED_0904 = 0x0904,
    IOTA_UNUSED_0905 = 0x0905,
    IOTA_UNUSED_0906 = 0x0906,
    IOTA_UNUSED_0907 = 0x0907,
    IOTA_UNUSED_0908 = 0x0908,
    IOTA_UNUSED_0909 = 0x0909,
    IOTA_UNUSED_090a = 0x090a,
    IOTA_UNUSED_090d = 0x090d,
    IOTA_UNUSED_091c = 0x091c,
    IOTA_UNUSED_091d = 0x091d,
    IOTA_UNUSED_091e = 0x091e,
    IOTA_UNUSED_091f = 0x091f,


    IOTA_UNUSED_0a00 = 0x0a00,
    IOTA_UNUSED_0a01 = 0x0a01,
    IOTA_UNUSED_0a02 = 0x0a02,
    IOTA_UNUSED_0a03 = 0x0a03,
    IOTA_UNUSED_0a04 = 0x0a04,
    IOTA_UNUSED_0a05 = 0x0a05,
    IOTA_UNUSED_0a06 = 0x0a06,
    IOTA_UNUSED_0a07 = 0x0a07,
    IOTA_UNUSED_0a08 = 0x0a08,
    IOTA_UNUSED_0a09 = 0x0a09,
    IOTA_UNUSED_0a0a = 0x0a0a,
    IOTA_UNUSED_0a0d = 0x0a0d,
    IOTA_UNUSED_0a1c = 0x0a1c,
    IOTA_UNUSED_0a1d = 0x0a1d,
    IOTA_UNUSED_0a1e = 0x0a1e,
    IOTA_UNUSED_0a1f = 0x0a1f,


    CORECACHE_BROADSIDE_SCAN = 0x0d00,
    IOTA_UNUSED_0d01 = 0x0d01,
    IOTA_UNUSED_0d02 = 0x0d02,
    IOTA_UNUSED_0d03 = 0x0d03,
    IOTA_UNUSED_0d04 = 0x0d04,
    IOTA_UNUSED_0d05 = 0x0d05,
    IOTA_UNUSED_0d06 = 0x0d06,
    IOTA_UNUSED_0d07 = 0x0d07,
    IOTA_UNUSED_0d08 = 0x0d08,
    IOTA_UNUSED_0d09 = 0x0d09,
    IOTA_UNUSED_0d0a = 0x0d0a,
    IOTA_UNUSED_0d0d = 0x0d0d,
    IOTA_UNUSED_0d1c = 0x0d1c,
    IOTA_UNUSED_0d1d = 0x0d1d,
    IOTA_UNUSED_0d1e = 0x0d1e,
    IOTA_UNUSED_0d1f = 0x0d1f,

    // The following are reserved for instance specific use.
    // Each engine must define its own XXX_panic_codes.h
    // Where XXX = SBE, QME, GPE0, GPE1, PGPE, XGPE
    // They are listed here to show the valid trap values that
    // can be used.

#ifdef PLATFORM_PANIC_CODES_H
#include PLATFORM_PANIC_CODES_H
#endif
    //_UNUSED_1c00                      = 0x1c00,
    //_UNUSED_1c01                      = 0x1c01,
    //_UNUSED_1c02                      = 0x1c02,
    //_UNUSED_1c03                      = 0x1c03,
    //_UNUSED_1c04                      = 0x1c04,
    //_UNUSED_1c05                      = 0x1c05,
    //_UNUSED_1c06                      = 0x1c06,
    //_UNUSED_1c07                      = 0x1c07,
    //_UNUSED_1c08                      = 0x1c08,
    //_UNUSED_1c09                      = 0x1c09,
    //_UNUSED_1c0a                      = 0x1c0a,
    //_UNUSED_1c0d                      = 0x1c0d,
    //_UNUSED_1c1c                      = 0x1c1c,
    //_UNUSED_1c1d                      = 0x1c1d,
    //_UNUSED_1c1e                      = 0x1c1e,
    //_UNUSED_1c1f                      = 0x1c1f,

    //_UNUSED_1d00                      = 0x1d00,
    //_UNUSED_1d01                      = 0x1d01,
    //_UNUSED_1d02                      = 0x1d02,
    //_UNUSED_1d03                      = 0x1d03,
    //_UNUSED_1d04                      = 0x1d04,
    //_UNUSED_1d05                      = 0x1d05,
    //_UNUSED_1d06                      = 0x1d06,
    //_UNUSED_1d07                      = 0x1d07,
    //_UNUSED_1d08                      = 0x1d08,
    //_UNUSED_1d09                      = 0x1d09,
    //_UNUSED_1d0a                      = 0x1d0a,
    //_UNUSED_1d0d                      = 0x1d0d,
    //_UNUSED_1d1c                      = 0x1d1c,
    //_UNUSED_1d1d                      = 0x1d1d,
    //_UNUSED_1d1e                      = 0x1d1e,
    //_UNUSED_1d1f                      = 0x1d1f,

    //_UNUSED_1e00                      = 0x1e00,
    //_UNUSED_1e01                      = 0x1e01,
    //_UNUSED_1e02                      = 0x1e02,
    //_UNUSED_1e03                      = 0x1e03,
    //_UNUSED_1e04                      = 0x1e04,
    //_UNUSED_1e05                      = 0x1e05,
    //_UNUSED_1e06                      = 0x1e06,
    //_UNUSED_1e07                      = 0x1e07,
    //_UNUSED_1e08                      = 0x1e08,
    //_UNUSED_1e09                      = 0x1e09,
    //_UNUSED_1e0a                      = 0x1e0a,
    //_UNUSED_1e0d                      = 0x1e0d,
    //_UNUSED_1e1c                      = 0x1e1c,
    //_UNUSED_1e1d                      = 0x1e1d,
    //_UNUSED_1e1e                      = 0x1e1e,
    //_UNUSED_1e1f                      = 0x1e1f,

    //_UNUSED_1f00                      = 0x1f00,
    //_UNUSED_1f01                      = 0x1f01,
    //_UNUSED_1f02                      = 0x1f02,
    //_UNUSED_1f03                      = 0x1f03,
    //_UNUSED_1f04                      = 0x1f04,
    //_UNUSED_1f05                      = 0x1f05,
    //_UNUSED_1f06                      = 0x1f06,
    //_UNUSED_1f07                      = 0x1f07,
    //_UNUSED_1f08                      = 0x1f08,
    //_UNUSED_1f09                      = 0x1f09,
    //_UNUSED_1f0a                      = 0x1f0a,
    //_UNUSED_1f0d                      = 0x1f0d,
    //_UNUSED_1f1c                      = 0x1f1c,
    //_UNUSED_1f1d                      = 0x1f1d,
    //_UNUSED_1f1e                      = 0x1f1e,
    //_UNUSED_1f1f                      = 0x1f1f
} iotaPanicCode_t;

#else

/// Assembler specific panic codes
#define IOTA_MACHINE_CHECK_PANIC           0x0004
#define IOTA_DATA_STORAGE_PANIC            0x0005
#define IOTA_INSTRUCTION_STORAGE_PANIC     0x0006
#define IOTA_DATA_ALIGNMENT_PANIC          0x0007
#define IOTA_ILLEGAL_INSTRUCTION           0x0008
#define IOTA_SOFTWARE_HALT                 0x0009
#define IOTA_DEC_NOT_SUPPORTED             0x000a
#define IOTA_WATCHDOG_NOT_SUPPORTED        0x000d

#endif  // __ASSEMBLER__

#if !defined(IOTA_PANIC)
#if SIMICS_ENVIRONMENT
#define IOTA_PANIC(code)                  \
    do {                                \
        asm volatile ("stw     %r3, __pk_panic_save_r3@sda21(0)");      \
        asm volatile ("lwz     %r3, __pk_panic_dbcr@sda21(0)");         \
        asm volatile ("mtdbcr  %r3");                                   \
        asm volatile (".long %0" : : "i" (code));                       \
    } while(0)
#else
#define IOTA_PANIC(code)                                                  \
    do {                                                                \
        asm volatile ("tw 31, %0, %1" : : "i" (code/256) , "i" (code%256)); \
    } while (0)
#endif // SIMICS_ENVIRONMENT
#endif // IOTA_PANIC


#endif
