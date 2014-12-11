/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/ssx_app_cfg.h $                                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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

#ifndef __SSX_APP_CFG_H__
#define __SSX_APP_CFG_H__

// These versions of SSX_PANIC are being changed so that they exactly
// mirror each other and are exactly structured at 8 instructions only and
// make only one branch to outside code.
#ifndef __ASSEMBLER__
#ifndef SSX_PANIC
#define SSX_PANIC(code)                                             \
do {                                                                \
    barrier();                                                      \
    asm volatile ("stw  %r3, __occ_panic_save_r3@sda21(0)");        \
    asm volatile ("mflr %r3");                                      \
    asm volatile ("stw  %r4, __occ_panic_save_r4@sda21(0)");        \
    asm volatile ("lis  %%r4, %0"::"i" (code >> 16));               \
    asm volatile ("ori  %%r4, %%r4, %0"::"i" (code & 0xffff));      \
    asm volatile ("bl   __ssx_checkpoint_panic_and_save_ffdc");     \
    asm volatile ("trap");                                          \
    asm volatile (".long %0" : : "i" (code));                       \
} while (0)
#endif // SSX_PANIC
#else  /* __ASSEMBLER__ */
#ifndef SSX_PANIC
// This macro cannot be more than 8 instructions long, but it can be less than
// 8.
#define SSX_PANIC(code) _ssx_panic code
    .macro  _ssx_panic, code
    stw     %r3, __occ_panic_save_r3@sda21(0)
    mflr    %r3
    stw     %r4, __occ_panic_save_r4@sda21(0)
    lis     %r4, \code@h
    ori     %r4, %r4, \code@l
    bl      __ssx_checkpoint_panic_and_save_ffdc
    trap
    .long   \code
    .endm
#endif // SSX_PANIC
#endif /* __ASSEMBLER__ */

#define INIT_SEC_NM_STR     "initSection"
#define INIT_SECTION __attribute__ ((section (INIT_SEC_NM_STR)))

#define SIMICS_ENVIRONMENT 0

/// Since OCC Firmware desires time intervals longer than 7 seconds, we will
/// change the interval to be a uint64_t instead of a uint32_t so we don't
/// hit the overflow condition
#define SSX_TIME_INTERVAL_TYPE uint64_t

#endif /* __SSX_APP_CFG_H__ */


