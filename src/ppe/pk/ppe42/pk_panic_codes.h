/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/pk_panic_codes.h $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017                             */
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
#ifndef __PK_PANIC_CODES_H__
#define __PK_PANIC_CODES_H__

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
    PPE42_MACHINE_CHECK_PANIC           = 0x0001,
    PPE42_DATA_STORAGE_PANIC            = 0x0002,
    PPE42_INSTRUCTION_STORAGE_PANIC     = 0x0003,
    PPE42_DATA_ALIGNMENT_PANIC          = 0x0004,
    PK_BOOT_VECTORS_NOT_ALIGNED         = 0x0005,
    PK_DEFAULT_IRQ_HANDLER              = 0x0006,
    PK_DEFAULT_SPECIAL_HANDLER          = 0x0007,
    PPE42_PHANTOM_INTERRUPT             = 0x0008,
    PPE42_ILLEGAL_INSTRUCTION           = 0x0009,
    PK_UNUSED_000a                      = 0x000a,
    PK_UNUSED_000d                      = 0x000d,
    PK_UNUSED_001c                      = 0x001c,
    PPE42_VIRTUAL_EXCEPTION             = 0x001d,
    PK_UNUSED_001e                      = 0x001e,
    PK_UNUSED_001f                      = 0x001f,

    // API return codes
    PK_ILLEGAL_CONTEXT_THREAD_CONTEXT   = 0x0100,
    PK_ILLEGAL_CONTEXT_INTERRUPT_CONTEXT = 0x0101,
    PK_ILLEGAL_CONTEXT_THREAD           = 0x0102,
    PK_ILLEGAL_CONTEXT_TIMER            = 0x0103,
    PK_INVALID_THREAD_AT_RESUME1        = 0x0104,
    PK_INVALID_THREAD_AT_RESUME2        = 0x0105,
    PK_INVALID_THREAD_AT_SUSPEND1       = 0x0106,
    PK_INVALID_THREAD_AT_SUSPEND2       = 0x0107,
    PK_INVALID_THREAD_AT_DELETE         = 0x0108,
    PK_INVALID_THREAD_AT_INFO           = 0x0109,
    PK_INVALID_THREAD_AT_CHANGE         = 0x010a,
    PK_INVALID_THREAD_AT_SWAP1          = 0x010d,
    PK_INVALID_THREAD_AT_SWAP2          = 0x011c,
    PK_INVALID_THREAD_AT_CREATE         = 0x011d,
    PK_INVALID_SEMAPHORE_AT_POST        = 0x011e,
    PK_INVALID_SEMAPHORE_AT_PEND        = 0x011f,
    PK_INVALID_SEMAPHORE_AT_RELEASE     = 0x0200,
    PK_INVALID_SEMAPHORE_AT_INFO        = 0x0201,
    PK_INVALID_SEMAPHORE_AT_CREATE      = 0x0202,
    PK_INVALID_TIMER_AT_SCHEDULE        = 0x0203,
    PK_INVALID_TIMER_AT_CANCEL          = 0x0204,
    PK_INVALID_TIMER_AT_INFO            = 0x0205,
    PK_INVALID_TIMER_AT_CREATE          = 0x0206,
    PK_INVALID_ARGUMENT_IRQ_SETUP       = 0x0207,
    PK_INVALID_ARGUMENT_IRQ_HANDLER     = 0x0208,
    PK_INVALID_ARGUMENT_INTERRUPT       = 0x0209,
    PK_INVALID_ARGUMENT_CONTEXT_SET     = 0x020a,
    PK_INVALID_ARGUMENT_CONTEXT_GET     = 0x020d,
    PK_INVALID_ARGUMENT_FIT             = 0x021c,
    PK_INVALID_ARGUMENT_WATCHDOG        = 0x021d,
    PK_INVALID_ARGUMENT_INIT            = 0x021e,
    PK_INVALID_ARGUMENT_SEMAPHORE       = 0x021f,
    PK_INVALID_ARGUMENT_THREAD_CHANGE   = 0x0300,
    PK_INVALID_ARGUMENT_THREAD_PRIORITY = 0x0301,
    PK_INVALID_ARGUMENT_THREAD1         = 0x0302,
    PK_INVALID_ARGUMENT_THREAD2         = 0x0303,
    PK_INVALID_ARGUMENT_THREAD3         = 0x0304,
    PK_STACK_OVERFLOW                   = 0x0305,
    PK_TIMER_ACTIVE                     = 0x0306,
    PK_TIMER_NOT_ACTIVE                 = 0x0307,
    PK_PRIORITY_IN_USE_AT_RESUME        = 0x0308,
    PK_PRIORITY_IN_USE_AT_CHANGE        = 0x0309,
    PK_PRIORITY_IN_USE_AT_SWAP          = 0x030a,
    PK_SEMAPHORE_OVERFLOW               = 0x030d,
    PK_SEMAPHORE_PEND_NO_WAIT           = 0x031c,
    PK_SEMAPHORE_PEND_TIMED_OUT         = 0x031d,
    PK_SEMAPHORE_PEND_WOULD_BLOCK       = 0x031e,
    PK_INVALID_DEQUE_SENTINEL           = 0x031f,
    PK_INVALID_DEQUE_ELEMENT            = 0x0400,
    PK_INVALID_OBJECT                   = 0x0401,

    // PK Kernel panics
    PK_NO_TIMER_SUPPORT                 = 0x0402,
    PK_START_THREADS_RETURNED           = 0x0403,
    PK_UNIMPLEMENTED                    = 0x0404,
    PK_SCHEDULING_INVARIANT             = 0x0405,
    PK_TIMER_HANDLER_INVARIANT          = 0x0406,
    PK_THREAD_TIMEOUT_STATE             = 0x0407,

    // PK
    PK_UNUSED_0408                      = 0x0408,
    PK_UNUSED_0409                      = 0x0409,
    PK_UNUSED_040a                      = 0x040a,
    PK_UNUSED_040d                      = 0x040d,
    PK_UNUSED_041c                      = 0x041c,
    PK_UNUSED_041d                      = 0x041d,
    PK_UNUSED_041e                      = 0x041e,
    PK_UNUSED_041f                      = 0x041f,

    // Sync panic codes
    SYNC_INVALID_OBJECT                 = 0x0500,
    SYNC_INVALID_ARGUMENT               = 0x0501,
    SYNC_BARRIER_PEND_TIMED_OUT         = 0x0502,
    SYNC_BARRIER_OVERFLOW               = 0x0503,
    SYNC_BARRIER_UNDERFLOW              = 0x0504,
    SYNC_BARRIER_INVARIANT              = 0x0505,
    SYNC_SHARED_UNDERFLOW               = 0x0506,

    OCCHW_INSTANCE_MISMATCH             = 0x0507,
    OCCHW_IRQ_ROUTING_ERROR             = 0x0508,
    OCCHW_XIR_INVALID_POINTER           = 0x0509,
    OCCHW_XIR_INVALID_GPE               = 0x050a,

    PK_UNUSED_050d                      = 0x050d,
    PK_UNUSED_051c                      = 0x051c,
    PK_UNUSED_051d                      = 0x051d,
    PK_UNUSED_051e                      = 0x051e,
    PK_UNUSED_051f                      = 0x051f,

    PK_UNUSED_0600                      = 0x0600,
    PK_UNUSED_0601                      = 0x0601,
    PK_UNUSED_0602                      = 0x0602,
    PK_UNUSED_0603                      = 0x0603,
    PK_UNUSED_0604                      = 0x0604,
    PK_UNUSED_0605                      = 0x0605,
    PK_UNUSED_0606                      = 0x0606,
    PK_UNUSED_0607                      = 0x0607,
    PK_UNUSED_0608                      = 0x0608,
    PK_UNUSED_0609                      = 0x0609,
    PK_UNUSED_060a                      = 0x060a,
    PK_UNUSED_060d                      = 0x060d,
    PK_UNUSED_061c                      = 0x061c,
    PK_UNUSED_061d                      = 0x061d,
    PK_UNUSED_061e                      = 0x061e,
    PK_UNUSED_061f                      = 0x061f,

    PK_UNUSED_0700                      = 0x0700,
    PK_UNUSED_0701                      = 0x0701,
    PK_UNUSED_0702                      = 0x0702,
    PK_UNUSED_0703                      = 0x0703,
    PK_UNUSED_0704                      = 0x0704,
    PK_UNUSED_0705                      = 0x0705,
    PK_UNUSED_0706                      = 0x0706,
    PK_UNUSED_0707                      = 0x0707,
    PK_UNUSED_0708                      = 0x0708,
    PK_UNUSED_0709                      = 0x0709,
    PK_UNUSED_070a                      = 0x070a,
    PK_UNUSED_070d                      = 0x070d,
    PK_UNUSED_071c                      = 0x071c,
    PK_UNUSED_071d                      = 0x071d,
    PK_UNUSED_071e                      = 0x071e,
    PK_UNUSED_071f                      = 0x071f,

    PK_UNUSED_0800                      = 0x0800,
    PK_UNUSED_0801                      = 0x0801,
    PK_UNUSED_0802                      = 0x0802,
    PK_UNUSED_0803                      = 0x0803,
    PK_UNUSED_0804                      = 0x0804,
    PK_UNUSED_0805                      = 0x0805,
    PK_UNUSED_0806                      = 0x0806,
    PK_UNUSED_0807                      = 0x0807,
    PK_UNUSED_0808                      = 0x0808,
    PK_UNUSED_0809                      = 0x0809,
    PK_UNUSED_080a                      = 0x080a,
    PK_UNUSED_080d                      = 0x080d,
    PK_UNUSED_081c                      = 0x081c,
    PK_UNUSED_081d                      = 0x081d,
    PK_UNUSED_081e                      = 0x081e,
    PK_UNUSED_081f                      = 0x081f,

    PK_UNUSED_0900                      = 0x0900,
    PK_UNUSED_0901                      = 0x0901,
    PK_UNUSED_0902                      = 0x0902,
    PK_UNUSED_0903                      = 0x0903,
    PK_UNUSED_0904                      = 0x0904,
    PK_UNUSED_0905                      = 0x0905,
    PK_UNUSED_0906                      = 0x0906,
    PK_UNUSED_0907                      = 0x0907,
    PK_UNUSED_0908                      = 0x0908,
    PK_UNUSED_0909                      = 0x0909,
    PK_UNUSED_090a                      = 0x090a,
    PK_UNUSED_090d                      = 0x090d,
    PK_UNUSED_091c                      = 0x091c,
    PK_UNUSED_091d                      = 0x091d,
    PK_UNUSED_091e                      = 0x091e,
    PK_UNUSED_091f                      = 0x091f,

    PK_UNUSED_0a00                      = 0x0a00,
    PK_UNUSED_0a01                      = 0x0a01,
    PK_UNUSED_0a02                      = 0x0a02,
    PK_UNUSED_0a03                      = 0x0a03,
    PK_UNUSED_0a04                      = 0x0a04,
    PK_UNUSED_0a05                      = 0x0a05,
    PK_UNUSED_0a06                      = 0x0a06,
    PK_UNUSED_0a07                      = 0x0a07,
    PK_UNUSED_0a08                      = 0x0a08,
    PK_UNUSED_0a09                      = 0x0a09,
    PK_UNUSED_0a0a                      = 0x0a0a,
    PK_UNUSED_0a0d                      = 0x0a0d,
    PK_UNUSED_0a1c                      = 0x0a1c,
    PK_UNUSED_0a1d                      = 0x0a1d,
    PK_UNUSED_0a1e                      = 0x0a1e,
    PK_UNUSED_0a1f                      = 0x0a1f,

    PK_UNUSED_0d00                      = 0x0d00,
    PK_UNUSED_0d01                      = 0x0d01,
    PK_UNUSED_0d02                      = 0x0d02,
    PK_UNUSED_0d03                      = 0x0d03,
    PK_UNUSED_0d04                      = 0x0d04,
    PK_UNUSED_0d05                      = 0x0d05,
    PK_UNUSED_0d06                      = 0x0d06,
    PK_UNUSED_0d07                      = 0x0d07,
    PK_UNUSED_0d08                      = 0x0d08,
    PK_UNUSED_0d09                      = 0x0d09,
    PK_UNUSED_0d0a                      = 0x0d0a,
    PK_UNUSED_0d0d                      = 0x0d0d,
    PK_UNUSED_0d1c                      = 0x0d1c,
    PK_UNUSED_0d1d                      = 0x0d1d,
    PK_UNUSED_0d1e                      = 0x0d1e,
    PK_UNUSED_0d1f                      = 0x0d1f,

    // The following are reserved for instance specific use.
    // Each engine must define its own XXX_panic_codes.h
    // Where XXX = SBE, CME, GPE0, GPE1, PGPE, SGPE
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
} pkPanicCode_t;

#else

/// Assembler specific panic codes
#define PPE42_MACHINE_CHECK_PANIC           0x0001
#define PPE42_DATA_STORAGE_PANIC            0x0002
#define PPE42_INSTRUCTION_STORAGE_PANIC     0x0003
#define PPE42_DATA_ALIGNMENT_PANIC          0x0004
#define PPE42_VIRTUAL_EXCEPTION             0x001d

#define PK_BOOT_VECTORS_NOT_ALIGNED         0x0005
#define PPE42_ILLEGAL_INSTRUCTION           0x001c
#define PK_STACK_OVERFLOW                   0x0305


#endif  // __ASSEMBLER__
#endif
