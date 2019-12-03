/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/iota/iota_ppe42_vectors.S $                           */
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
#include "iota_ppe42.h"
#include "iota_panic_codes.h"

.section    .vectors, "ax", @progbits

.global __vectors
__vectors:
        b   __machine_check

/* __set_msr becomes the PPE idle address
   when the corresponding msr bit is set */

.global __set_msr
.org    __vectors + 0x0010
__set_msr:
    mtmsr   %r3
    blr

.global __machine_check
.org    __vectors + 0x0020
__machine_check:
    _pk_panic IOTA_MACHINE_CHECK_PANIC

.global __system_reset
.org    __vectors + 0x0040
__system_reset:
    __m_iota_boot
    b __iota_halt

.global __data_storage
.org    __vectors + 0x0060
__data_storage:
    _pk_panic IOTA_DATA_STORAGE_PANIC

.global __instruction_storage
.org    __vectors + 0x0080
__instruction_storage:
    _pk_panic IOTA_INSTRUCTION_STORAGE_PANIC

.global __external_interrupt
.org    __vectors + 0x00a0
__external_interrupt:
    __m_iota_interrupt_and_exception_handler _IOTA_SCHEDULE_REASON_EXT

.global __alignment_exception
.org    __vectors + 0x00c0
__alignment_exception:
    _pk_panic IOTA_DATA_ALIGNMENT_PANIC

.global __program_exception
.org    __vectors + 0x00e0
__program_exception:
    _pk_panic IOTA_ILLEGAL_INSTRUCTION

.global __dec_interrupt
.org    __vectors + 0x0100
__dec_interrupt:
    _pk_panic IOTA_DEC_NOT_SUPPORTED
/*  Instead of trap above, Enable the following if reenable DEC timer:  */
/*  __m_iota_interrupt_and_exception_handler _IOTA_SCHEDULE_REASON_DEC  */

.global __fit_interrupt
.org    __vectors + 0x0120
__fit_interrupt:
    __m_iota_interrupt_and_exception_handler _IOTA_SCHEDULE_REASON_FIT

.global __watchdog_interrupt
.org    __vectors + 0x0140
__watchdog_interrupt:
/*   b __watchdog_interrupt  */
    _pk_panic IOTA_WATCHDOG_NOT_SUPPORTED

.global __virtual_exception
.org    __vectors + 0x0160
__virtual_exception:
    _pk_panic IOTA_VIRTUAL_EXCEPTION
