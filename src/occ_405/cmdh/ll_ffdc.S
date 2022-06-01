/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cmdh/ll_ffdc.S $                                  */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2022                        */
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
# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occ_405/cmdh/ll_ffdc.S $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2011,2019
# [+] International Business Machines Corp.
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.
#
# IBM_PROLOG_END_TAG

/*****************************************************************************/
// Includes
/*****************************************************************************/
#include "ssx.h"

/*****************************************************************************/
// Literals and Structures
/*****************************************************************************/

#
# Format of FFDC header (8 bytes)
#
# Header Start
# Sequence
# | Command
# | |  Exception code
# | | | Length of data
# | | | |    version
# | | | |    | Panic Checkpoint: Two possible sets, 0xF00 (ABOUT_TO_HALT), a 405
# | | | |    | |                 trap event, and everything else. Other values
# | | | |    | |                 occur at init events, see checkpoints in occ_common.h
# | | | |    | |     Begin data buffer
# | | | |    | |     |
# SSCCRRLL/LLvvKKKK  D...
#                 |
#                 Header End
# Return status is always 'E*'h
#   See cmdh_fsp.h ERRL_RC for command buffer response codes.  All E* RCs are
#   meant to indicate a trap/halt event on the OCC and they should be followed
#   by an FFDC data packet.  If they didn't it means that the occ has either
#   hung or trapped without processing the FFDC save function.
#
# FFDC Data packet:
#
# Offset            Len     Register(s)
# from start of
# rsp buffer
# -----------------------------------------------------------------------------
# 0   (x00)         1       Sequence (0x00 for FFDC)
# 1   (x01)         1       Command (0x00 for FFDC)
# 2   (x02)         1       Exception code (Always 0xE* for OCC FFDC)
# 3   (x03)         1       Length Hi byte
# 4   (x04)         1       Length Lo byte
# 5   (x05)         1       Version (0x00 FFDC, 0x01 FFDC with BCE registers)
# 6   (x06)         2       Checkpoint (Typically 0x0F00)
# 8   (x08)         4       SSX Panic Code
# 12  (x0c)         4       Panic Address (inside function that called panic)
# 16  (x10)         4       Link Register (caller of function that called panic))
# 20  (x14)         4       Machine Status Register (interrupt context?)
# 24  (x18)         4       Condition Register (CR)
# 28  (x1c)         4       Count Register (CTR)
#                   (128)   GPR0 - GPR31 (GPR1 is the stack pointer)
# 32  (x20)         4       GPR0
# 36  (x24)         4       GPR1
# 40  (x28)         4       GPR2
# 44  (x2c)         4       GPR3
# 48  (x30)         4       GPR4
# 52  (x34)         4       GPR5
# 56  (x38)         4       GPR6
# 60  (x3c)         4       GPR7
# 64  (x40)         4       GPR8
# 68  (x44)         4       GPR9
# 72  (x48)         4       GPR10
# 76  (x4c)         4       GPR11
# 80  (x50)         4       GPR12
# 84  (x54)         4       GPR13
# 88  (x58)         4       GPR14
# 92  (x5c)         4       GPR15
# 96  (x60)         4       GPR16
# 100 (x64)         4       GPR17
# 104 (x68)         4       GPR18
# 108 (x6c)         4       GPR19
# 112 (x70)         4       GPR20
# 116 (x74)         4       GPR21
# 120 (x78)         4       GPR22
# 124 (x7c)         4       GPR23
# 128 (x80)         4       GPR24
# 132 (x84)         4       GPR25
# 136 (x88)         4       GPR26
# 140 (x8c)         4       GPR27
# 144 (x90)         4       GPR28
# 148 (x94)         4       GPR29
# 152 (x98)         4       GPR30
# 156 (x9c)         4       GPR31
#
# 160 (xa0)         4       evpr
# 164 (xa4)         4       xer
# 168 (xa8)         4       esr
# 172 (xac)         4       dear (bad address)
# 176 (xb0)         4       srr0 (return address for non critical interrupt)
# 180 (xb4)         4       srr1 (MSR at interrupt)
# 184 (xb8)         4       srr2 (return address for critical interrupt)
# 188 (xbc)         4       srr3 (MSR at interrupt)
# 192 (xc0)         4       mcsr
# 196 (xc4)         4       pid
# 200 (xc8)         4       zpr
# 204 (xcc)         4       usprg0
#                   (32)    sprg0 - sprg7
# 208 (xd0)         4       sprg0
# 212 (xd4)         4       sprg1
# 216 (xd8)         4       sprg2
# 220 (xdc)         4       sprg3
# 224 (xe0)         4       sprg4
# 228 (xe4)         4       sprg5
# 232 (xe8)         4       sprg6
# 236 (xec)         4       sprg7
#
# 240 (xf0)         4       tcr
# 244 (xf4)         4       tsr
# 248 (xf8)         4       dbcr0
# 252 (xfc)         4       dbcr1
# 256 (x100)        4       dbsr
# 260 (x104)        4       OCB_OISR0
# 264 (x108)        4       OCB_OISR1
# 268 (x10c)        4       OCB_OCCMISC
# 272 (x110)        4       OCB_OHTMCR
# 276 (x114)        4       OCB_OIMR0
# 280 (x118)        4       OCB_OIMR1
# 284 (x11c)        4       OCB_OITR0
# 288 (x120)        4       OCB_OITR1
# 292 (x124)        4       OCB_OIEPR0
# 296 (x128)        4       OCB_OIEPR1
# 300 (x12c)        4       OCB_OEHDR
# 304 (x130)        4       OCB_OCICFG
# 308 (x134)        4       OCB_ONISR0
# 312 (x138)        4       OCB_ONISR1
# 316 (x13c)        4       OCB_OCISR0
# 320 (x140)        4       OCB_OCISR1
# 324 (x144)        4       OCB_OCCFLG0
# 328 (x148)        4       OCB_OCCHBR
# 332 (x14c)        4       PBA_BASE
# 336 (x150)        4       PBA_MODE
# 340 (x154)        4       PBA_SLVCTL0
# 344 (x158)        4       PBA_SLVCTL1
# 348 (x15c)        4       PBA_SLVCTL2
# 352 (x160)        4       PBA_SLVCTL3
# 356 (x164)        4       PBA_BCDE_CTL
# 360 (x168)        4       PBA_BCDE_SET
# 364 (x16c)        4       PBA_BCDE_STAT
# 368 (x170)        4       PBA_BCDE_DR
# 372 (x174)        4       PBA_BCDE_OCIBAR
# 376 (x178)        4       PBA_BCUE_CTL
# 380 (x17c)        4       PBA_BCUE_SET
# 384 (x180)        4       PBA_BCUE_STAT
# 388 (x184)        4       PBA_BCUE_DR
# 392 (x188)        4       PBA_BCUE_OCIBAR
# 396 (x18c)        4       PBA_OCCACT
# 400 (x190)        4       PBA_BAR0
# 404 (x194)        4       PBA_BAR1
# 408 (x198)        4       PBA_BAR2
# 412 (x19c)        4       PBA_BAR3
# 416 (x1a0)        4       PBA_BARMSK0
# 420 (x1a4)        4       PBA_BARMSK1
# 424 (x1a8)        4       PBA_BARMSK2
# 428 (x1ac)        4       PBA_BARMSK3
# 432 (x1b0)        4       PBA_ERRRPT0
# 436 (x1b4)        4       PBA_ERRRPT1
# 440 (x1b8)        4       PBA_ERRRPT2
# 444 (x1bc)        4       PBA_RBUFVAL0
# 448 (x1c0)        4       PBA_RBUFVAL1
# 452 (x1c4)        4       PBA_RBUFVAL2
# 456 (x1c8)        4       PBA_RBUFVAL3
# 460 (x1cc)        4       PBA_RBUFVAL4
# 464 (x1d0)        4       PBA_RBUFVAL5
# 468 (x1d4)        4       PBA_WBUFVAL0
# 472 (x1d8)        4       PBA_WBUFVAL1
# 476 (x1dC)        4       PBA_SLVRST
# 480 (x1e0)        4       SSX timebase
# 484 (x1e4)        16      Build name
# 500 (x1f4)        8       OCCLFIR -- not available reserved
# 508 (x1fc)        8       PBAFIR
# 516 (x204)        4       CORES_DECON
#                   (64)    Main thread dump
# 520 (x208)        1        Length of thread dump (64)
# 521 (x209)        1        Priority
# 522 (x20a)        1        State
# 523 (x20b)        1        Flags
# 524 (x20c)        4        Timer
# 528 (x210)        4        Semaphore
# 532 (x214)        4        SRR0
# 536 (x218)        4        SRR1
# 540 (x21c)        4        SRR2
# 544 (x220)        4        SRR3
# 548 (x224)        4        Link register
# 552 (x228)        32       Thread stack trace
#
# 584 (x248)        64      Command thread dump
# 648 (x288)        64      DCOM thread dump
# 712 (x2c8)        32      Stack trace
# 744 (x2e8)        4       xffdcffdc End of buffer marker
# -----------------------------------------------------------------------------
#

# --------------
# FFDC Constants
# --------------

    # The FFDC buffer is the response buffer: verify equality to _LINEAR_RD_WINDOW_SECTION_BASE
    .set        FFDC_BUFFER_ADDR, 0xFFFFE000
    .set        FFDC_DBCR0, (DBCR0_EDM | DBCR0_TDE | DBCR0_FT)
    .set        FFDC_END_MARKER, 0xFFDCFFDC
    .set        FFDC_OCC_CHECKPOINT, 0x0F00
    .set        FFDC_SSX_PANIC_EX_CODE, 0xE0

# ---------------------
# FFDC buffer structure
# ---------------------

    .struct     FFDC_BUFFER_ADDR
SEQUENCE:                   .byte
    .struct SEQUENCE + 1
COMMAND:                    .byte
    .struct COMMAND + 1
EXCEPTION:                  .byte
    .struct EXCEPTION + 1
LENGTH_HI:                  .byte
    .struct LENGTH_HI + 1
LENGTH_LO:                  .byte
    .struct LENGTH_LO + 1
VERSION:                    .byte
    .struct VERSION + 1
CHECKPOINT:                 .short
    .struct CHECKPOINT + 2
PANIC_CODE:                 .int
    .struct PANIC_CODE + 4
PANIC_ADDR:                 .int
    .struct PANIC_ADDR + 4
LINK_REG:                   .int
    .struct LINK_REG + 4
MSR:                        .int
    .struct MSR + 4
CR:                         .int
    .struct CR + 4
CTR:                        .int
    .struct CTR + 4
GPR0:                       .int
    .struct GPR0 + 4
GPR1:                       .int
    .struct GPR1 + 4
GPR2:                       .int
    .struct GPR2 + 4
GPR3:                       .int
    .struct GPR3 + 4
GPR4:                       .int
    .struct GPR4 + 4
GPR5:                       .int
    .struct GPR5 + 4
GPR6:                       .int
    .struct GPR6 + 4
GPR7:                       .int
    .struct GPR7 + 4
GPR8:                       .int
    .struct GPR8 + 4
GPR9:                       .int
    .struct GPR9 + 4
GPR10:                      .int
    .struct GPR10 + 4
GPR11:                      .int
    .struct GPR11 + 4
GPR12:                      .int
    .struct GPR12 + 4
GPR13:                      .int
    .struct GPR13 + 4
GPR14:                      .int
    .struct GPR14 + 4
GPR15:                      .int
    .struct GPR15 + 4
GPR16:                      .int
    .struct GPR16 + 4
GPR17:                      .int
    .struct GPR17 + 4
GPR18:                      .int
    .struct GPR18 + 4
GPR19:                      .int
    .struct GPR19 + 4
GPR20:                      .int
    .struct GPR20 + 4
GPR21:                      .int
    .struct GPR21 + 4
GPR22:                      .int
    .struct GPR22 + 4
GPR23:                      .int
    .struct GPR23 + 4
GPR24:                      .int
    .struct GPR24 + 4
GPR25:                      .int
    .struct GPR25 + 4
GPR26:                      .int
    .struct GPR26 + 4
GPR27:                      .int
    .struct GPR27 + 4
GPR28:                      .int
    .struct GPR28 + 4
GPR29:                      .int
    .struct GPR29 + 4
GPR30:                      .int
    .struct GPR30 + 4
GPR31:                      .int
    .struct GPR31 + 4
EVPR:                       .int
    .struct EVPR + 4
EXR:                        .int
    .struct EXR + 4
ESR:                        .int
    .struct ESR + 4
DEAR:                       .int
    .struct DEAR + 4
SRR0:                       .int
    .struct SRR0 + 4
SRR1:                       .int
    .struct SRR1 + 4
SRR2:                       .int
    .struct SRR2 + 4
SRR3:                       .int
    .struct SRR3 + 4
MCSR:                       .int
    .struct MCSR + 4
PID:                        .int
    .struct PID + 4
ZPR:                        .int
    .struct ZPR + 4
USPRG0:                     .int
    .struct USPRG0 + 4
SPRG0:                      .int
    .struct SPRG0 + 4
SPRG1:                      .int
    .struct SPRG1 + 4
SPRG2:                      .int
    .struct SPRG2 + 4
SPRG3:                      .int
    .struct SPRG3 + 4
SPRG4:                      .int
    .struct SPRG4 + 4
SPRG5:                      .int
    .struct SPRG5 + 4
SPRG6:                      .int
    .struct SPRG6 + 4
SPRG7:                      .int
    .struct SPRG7 + 4
TCR:                        .int
    .struct TCR + 4
TSR:                        .int
    .struct TSR + 4
DBCR0:                      .int
    .struct DBCR0 + 4
DBCR1:                      .int
    .struct DBCR1 + 4
DBSR:                       .int
    .struct DBSR + 4
OISR0:                      .int
    .struct OISR0 + 4
OISR1:                      .int
    .struct OISR1 + 4
OCCMISC:                     .int
    .struct OCCMISC + 4
OHTMCR:                     .int
    .struct OHTMCR + 4
OIMR0:                      .int
    .struct OIMR0 + 4
OIMR1:                      .int
    .struct OIMR1 + 4
OITR0:                      .int
    .struct OITR0 + 4
OITR1:                      .int
    .struct OITR1 + 4
OIEPR0:                     .int
    .struct OIEPR0 + 4
OIEPR1:                     .int
    .struct OIEPR1 + 4
OEHDR:                      .int
    .struct OEHDR + 4
OCICFG:                      .int
    .struct OCICFG + 4
ONISR0:                     .int
    .struct ONISR0 + 4
ONISR1:                     .int
    .struct ONISR1 + 4
OCISR0:                     .int
    .struct OCISR0 + 4
OCISR1:                     .int
    .struct OCISR1 + 4
OCCFLG0:                    .int
    .struct OCCFLG0 + 4
OCCHBR:                     .int
    .struct OCCHBR + 4
PBABASE:                     .int
    .struct PBABASE + 4
PBAMODE:                     .int
    .struct PBAMODE + 4
PBASLVCTL0:                     .int
    .struct PBASLVCTL0 + 4
PBASLVCTL1:                     .int
    .struct PBASLVCTL1 + 4
PBASLVCTL2:                     .int
    .struct PBASLVCTL2 + 4
PBASLVCTL3:                     .int
    .struct PBASLVCTL3 + 4
BCDE_CTL:                     .int
    .struct BCDE_CTL + 4
BCDE_SET:                     .int
    .struct BCDE_SET + 4
BCDE_STAT:                     .int
    .struct BCDE_STAT + 4
BCDE_DR:                     .int
    .struct BCDE_DR + 4
BCDE_OCIBAR:                     .int
    .struct BCDE_OCIBAR + 4
BCUE_CTL:                     .int
    .struct BCUE_CTL + 4
BCUE_SET:                     .int
    .struct BCUE_SET + 4
BCUE_STAT:                     .int
    .struct BCUE_STAT + 4
BCUE_DR:                     .int
    .struct BCUE_DR + 4
BCUE_OCIBAR:                     .int
    .struct BCUE_OCIBAR + 4
OCC_ACTION:                 .int
    .struct OCC_ACTION + 4
PBABAR0:                 .int
    .struct PBABAR0 + 4
PBABAR1:                 .int
    .struct PBABAR1 + 4
PBABAR2:                 .int
    .struct PBABAR2 + 4
PBABAR3:                 .int
    .struct PBABAR3 + 4
PBABARMSK0:                 .int
    .struct PBABARMSK0 + 4
PBABARMSK1:                 .int
    .struct PBABARMSK1 + 4
PBABARMSK2:                 .int
    .struct PBABARMSK2 + 4
PBABARMSK3:                 .int
    .struct PBABARMSK3 + 4
PBAERRRPT0:                 .int
    .struct PBAERRRPT0 + 4
PBAERRRPT1:                 .int
    .struct PBAERRRPT1 + 4
PBAERRRPT2:                 .int
    .struct PBAERRRPT2 + 4
PBARBUFVAL0:                 .int
    .struct PBARBUFVAL0 + 4
PBARBUFVAL1:                 .int
    .struct PBARBUFVAL1 + 4
PBARBUFVAL2:                 .int
    .struct PBARBUFVAL2 + 4
PBARBUFVAL3:                 .int
    .struct PBARBUFVAL3 + 4
PBARBUFVAL4:                 .int
    .struct PBARBUFVAL4 + 4
PBARBUFVAL5:                 .int
    .struct PBARBUFVAL5 + 4
PBAWBUFVAL0:                 .int
    .struct PBAWBUFVAL0 + 4
PBAWBUFVAL1:                 .int
    .struct PBAWBUFVAL1 + 4
PBASLVRST:                   .int
    .struct PBASLVRST + 4

FFDC_TIMEBASE:              .int
    .struct FFDC_TIMEBASE + 4
OCC_BUILD_NAME:             .byte
    .struct OCC_BUILD_NAME + 16
OCCLFIR:                    .long
    .struct OCCLFIR + 8
PBAFIR:                     .long
    .struct PBAFIR + 8
CORES_DECON:                .int
    .struct CORES_DECON + 4
MAIN_THREAD_DUMP:           .byte
    .struct MAIN_THREAD_DUMP + 64
CMD_THREAD_DUMP:            .byte
    .struct CMD_THREAD_DUMP + 64
DCOM_THREAD_DUMP:           .byte
    .struct DCOM_THREAD_DUMP + 64
STACK_TRACE:                .byte
    .struct STACK_TRACE + 32
FFDC_END:                   .int
    .struct FFDC_END + 4
END_OF_BUFFER:  .byte

# The FFDC buffer length is the byte count starting at the version byte after
# the data length and counting to the end of the FFDC buffer.
    .set        FFDC_BUFFER_LENGTH, (END_OF_BUFFER-VERSION)
    .set        FFDC_BUFFER_LENGTH_HI, ((FFDC_BUFFER_LENGTH >> 8) & 0x00FF)
    .set        FFDC_BUFFER_LENGTH_LO, (FFDC_BUFFER_LENGTH & 0x00FF)

/*****************************************************************************/
// Functions
/*****************************************************************************/
# Switch back to text section, .struct switched us to the absolute section
    .section    ".text"

#  __save_ffdc_regs rewritten, __ssx_checkpoint_panic_and_save_ffdc added
#  __panic_checkpoint tweaked, __occ_checkpoint_panic_and_save_ffdc added

# Function Specification ******************************************************
#
#  Name: __occ_checkpoint_panic_and_save_ffdc
#
#  Description: Entry point for OCC_HALT initiated FFDC collection.
#
# Parameters:
#   Inputs:
#   r3 - Contains the LR at the time the function the was called (panic address)
#   r4 - Contains the exception code to be stored with the FFDC
#
# Outputs:
#   None
#
# Notes:
#   GPR1-31 will be restored before the function returns.
#   This function assumes that r3 and r4 have been saved away in special
#   variables called __occ_panic_save_r3 and __occ_panic_save_r4.
#
#   Registers r1, r2 and r13 are dedicated for use by the ABI so they cannot be
#   destroyed.
#       r1  - stack ptr
#       r2  - ro small data anchor
#       r13 - rw small data anchor
#
#   Register USPRG0 is reserved for SSX and cannot be modified
#   Registers SPRG0-7 are reserved for interrupt handlers and cannot be modified
#
#   Clear critical interrupts during this process so it can't be stopped once
#   it has begun.  Be sure to save the MSR first.
#
# End Function Specification **************************************************

.global_function __occ_checkpoint_panic_and_save_ffdc
__occ_checkpoint_panic_and_save_ffdc:

    # Store LR
    stwu       %r1, -8(%r1)
    mflr       %r0
    stw        %r0, 12(%r1)


    # Save r5 for later so we can use the register now
    _stwsd      %r5, __occ_panic_save_r5

    # Save the MSR for later so we can immediately disable all interrupts to
    # prevent critical interrupts from interrupting the FFDC collection.  This
    # will also disable data MMU protection.
    mfmsr       %r5
    _stwsd      %r5, __occ_panic_save_msr
    li          %r5, 0
    mtmsr       %r5
    isync
    sync

    # Some of the FFDC will now be saved to make r3 available for use and
    # to prevent corruption of the link register before proceeding.
    # Save the link register value that was present at the time the panic macro
    # was used, this is passed in r3.  Saving it now will also make r3 available
    # for use.
    _stwi       %r3, %r5, LINK_REG

    # We can use r3 now...
    # Save dbcr0
    mfdbcr0     %r3
    _stwi       %r3, %r5, DBCR0

    # Enable external debug mode and the trap debug event and freeze the timers,
    # this will cause a debug event halting the 405 before execution of a trap
    # instruction in the conditions are such that the trap occurs.
    _liwa       %r5, FFDC_DBCR0
    mtdbcr0     %r5
    isync

    # Save the panic address (the address of the panic macro user), this is just
    # the current link register.  Save this now to the FFDC before continuing so
    # we can call other functions if we need to.
    mflr        %r3
    _stwi       %r3, %r5, PANIC_ADDR

    #
    # r3 and r5 are now available for general use.
    #

    # Unlike the ssx version of this function, a panic code was not passed in.
    # Instead, r4 contains the exception code.  Set the panic code in the FFDC
    # buffer to 0 to indicate this is an OCC HALT.
    li          %r3, 0
    _stwi       %r3, %r5, PANIC_CODE

    # Put the exception code found in r4 in r3 and call __panic_checkpoint, this
    # will also put the panic checkpoint into the FFDC.
    mr          %r3, %r4

    # __panic_checkpoint needs r4 and r5 to be available and expects the
    # exception code in r3, r3 will be destroyed.
    bl          __panic_checkpoint

    # Call __save_ffdc_regs to save the remaining FFDC
    bl          __save_ffdc_regs

# Return to the caller
# Comment out if you don't want to head to the trap handler
#    lwz        %r0, 12(%r1)
#    addi       %r1, %r1, 8
#    mtlr       %r0
#    blr

# If we decide to lock the OCC here, rather than trap handler,
# comment the above 4 LoC, and uncomment the next 2 LoC
# This avoids infinite looping through the
# __ssx_checkpoint_panic_and_save_ffdc code
occ_panic_deathbed:
        b occ_panic_deathbed


# Function Specification ******************************************************
#
#  Name: __ssx_checkpoint_panic_and_save_ffdc
#
#  Description: Entry point for SSX_PANIC initiated FFDC collection.
#
# Parameters:
#   Inputs:
#   r3 - Contains the LR at the time the panic macro was used
#   r4 - Contains the SSX panic code to be stored with the FFDC
#
# Outputs:
#   None
#
# Notes:
#   GPR1-31 will be restored before the function returns.
#   This function assumes that r3 and r4 have been saved away in special
#   variables called __occ_panic_save_r3 and __occ_panic_save_r4.
#
#   Registers r1, r2 and r13 are dedicated for use by the ABI so they cannot be
#   destroyed.
#       r1  - stack ptr
#       r2  - ro small data anchor
#       r13 - rw small data anchor
#
#   Register USPRG0 is reserved for SSX and cannot be modified
#   Registers SPRG0-7 are reserved for interrupt handlers and cannot be modified
#
#   Clear critical interrupts during this process so it can't be stopped once
#   it has begun.  Be sure to save the MSR first.
#
# End Function Specification **************************************************

.global_function __ssx_checkpoint_panic_and_save_ffdc
__ssx_checkpoint_panic_and_save_ffdc:

    # Store LR
    stwu       %r1, -8(%r1)
    mflr       %r0
    stw        %r0, 12(%r1)

    # Save r5 for later so we can use the register now
    _stwsd      %r5, __occ_panic_save_r5

    # Save the MSR for later so we can immediately disable all interrupts to
    # prevent critical interrupts from interrupting the FFDC collection.  This
    # will also disable data MMU protection.
    mfmsr       %r5
    _stwsd      %r5, __occ_panic_save_msr
    li          %r5, 0
    mtmsr       %r5
    isync
    sync

    # Some of the FFDC will now be saved to make r3 and r4 available for use and
    # to prevent corruption of the link register before proceeding.
    # Save the panic code(r4) to make r4 available for use
    _stwi       %r4, %r5, PANIC_CODE

    # Save the link register value that was present at the time the panic macro
    # was used, this is passed in r3.  Saving it now will also make r3 available
    # for use.
    _stwi       %r3, %r5, LINK_REG

    # We can use r3 now...
    # Save dbcr0
    mfdbcr0     %r3
    _stwi       %r3, %r5, DBCR0

    # Enable external debug mode and the trap debug event and freeze the timers,
    # this will cause a debug event halting the 405 before execution of a trap
    # instruction if the conditions are such that the trap occurs.
    _liwa       %r5, FFDC_DBCR0
    mtdbcr0     %r5
    isync

    # Save the panic address (the address of the panic macro user), this is just
    # the current link register.  Save this now to the FFDC before continuing so
    #  we can call other functions if we need to.
    mflr        %r3
    _stwi       %r3, %r5, PANIC_ADDR

    #
    # r3, r4, and r5 are now available for general use.
    #

    # Put the exception code in r3 and call __panic_checkpoint, this
    # will also put the panic checkpoint into the FFDC.  __panic_checkpoint
    # destroys r3.
    _liwa       %r3, FFDC_SSX_PANIC_EX_CODE

    # __panic_checkpoint needs r4 and r5 to be available and expects the
    # exception code in r3, r3 will be destroyed.
    bl          __panic_checkpoint

    # Call __save_ffdc_regs to save the remaining FFDC
    bl          __save_ffdc_regs

# Return to the caller
# Comment out if you don't want to head to the trap handler
#    lwz        %r0, 12(%r1)
#    addi       %r1, %r1, 8
#    mtlr       %r0
#    blr

# If we decide to lock the OCC here, rather than trap handler,
# comment the above 4 LoC, and uncomment the next 2 LoC
# This avoids infinite looping through the
# __ssx_checkpoint_panic_and_save_ffdc code
ssx_panic_deathbed:
        b ssx_panic_deathbed


// New function to save a panic checkpoint and return code to the ffdc
// header before writing the ffdc data packet and going to halt.
# Function Specification ******************************************************
#
#  Name: __panic_checkpoint
#
#  Description: Write the 8 byte FFDC header specific to a panic.
#  __panic_checkpoint functions like __CHECKPOINT(0x00, 0x0F00, 0xE0).
#
#  See ffdc header format in __save_ffdc_regs
#
#  Parameters: r3 contains the exception code.
#
#  Important: This function assumes that r3 contains the exception code and
#  r4 and r5 can be used and that their contents do not need to be restored upon
#  exit.
#
#  Important: This function destroys r3.
#
#  Important: Be sure to save the link register if needed before calling this
#  function!
#
# End Function Specification **************************************************

.global_function __panic_checkpoint
__panic_checkpoint:

    # Header Start
    # Sequence
    # | Command
    # | |  Exception code
    # | | | Length of data
    # | | | |    version
    # | | | |    | Panic Checkpoint
    # | | | |    | |
    # SSCCRRLL/LLvvKKKK
    #                 |
    #                 Header End
    #
    # Write the 8 byte FFDC header to 0000xx00/0000yyyy where xx is the
    # exception passed in r3 and yyyy is the OCC specific panic checkpoint
    # value.
    #

    # Store LR
    stwu       %r1, -8(%r1)
    mflr       %r0
    stw        %r0, 12(%r1)

    # r3 contains the exception code and r4 and r5 are available for use
    li      %r4, 0x00
    # Sequence, command, length are all set to 0
    # Exception was passed in r3
    _stbi   %r4, %r5, SEQUENCE
    _stbi   %r4, %r5, COMMAND
    _stbi   %r3, %r5, EXCEPTION
    _stbi   %r4, %r5, LENGTH_HI
    _stbi   %r4, %r5, LENGTH_LO

    # Set version to 1
    li      %r4, 0x01
    _stbi   %r4, %r5, VERSION

    # Use the predefined OCC panic checkpoint
    li      %r4, FFDC_OCC_CHECKPOINT
    _sthi   %r4, %r5, CHECKPOINT

    # Set r4 to point to beginning of response buffer, sync
    # data access and request a data cache flush
    _liw    %r4, FFDC_BUFFER_ADDR
    sync
    dcbf    0, %r4

    # Return to caller
    lwz        %r0, 12(%r1)
    addi       %r1, %r1, 8
    mtlr       %r0
    blr


# Function Specification ******************************************************
#
#  Name: __save_ffdc_regs
#
#  Description: saves registers to fsp response buffer in sram for debug.  Only
#  the data area and length fields of the FSP response buffer are modified, the
#  FFDC header area (except for the length) is left unmodified.
#
#  GPR1-31 will be restored before the function returns.
#
#  Important: This function assumes that r3, r4, r5 and the MSR have been saved
#  in small data area special variables before being called.  It also assumes
#  that r3, r4 and r5 are available for general use.
#
#  Important: This function assumes that the panic return code, panic address,
#  panic checkpoint, panic code and the link register at the time the panic
#  macro was used have already been saved in the FFDC.
#
#  Important:  This function assumes that the MSR has been set to disable all
#  interrupts and that the DBCR0 register has been saved and then set to enable
#  external debug mode, the trap debug event and timer freeze.
#
# End Function Specification **************************************************

# updated to save new FFDC format
.global_function __save_ffdc_regs
__save_ffdc_regs:

    # Store LR
    stwu       %r1, -8(%r1)
    mflr       %r0
    stw        %r0, 12(%r1)

    # Start by writing out the predetermined length of the FFDC to the buffer.
    _liwa       %r3, FFDC_BUFFER_LENGTH_HI
    _stbi       %r3, %r4, LENGTH_HI
    _liwa       %r3, FFDC_BUFFER_LENGTH_LO
    _stbi       %r3, %r4, LENGTH_LO

    # Start storing the remaining FFDC starting with the MSR, get the MSR value
    # from the special variable in the small data area since we assume the MSR
    # had been altered before this function was called.
    _lwzsd      %r3, __occ_panic_save_msr
    _stwi       %r3, %r4, MSR
    # CR
    mfcr        %r3
    _stwi       %r3, %r4, CR
    # CTR
    mfctr       %r3
    _stwi       %r3, %r4, CTR

    # Save all the gprs
    _stwi       %r0, %r4, GPR0
    _stwi       %r1, %r4, GPR1
    _stwi       %r2, %r4, GPR2
    # Restore r3 from the special variable in the small data area
    _lwzsd      %r3, __occ_panic_save_r3
    _stwi       %r3, %r4, GPR3
    # Restore r4 from the special variable in the small data area
    _lwzsd      %r4, __occ_panic_save_r4
    _stwi       %r4, %r3, GPR4
    # Restore r5 from the special variable in the small data area
    _lwzsd      %r5, __occ_panic_save_r5
    _stwi       %r5, %r3, GPR5
    # Use r3 now as an auto incrementing pointer to multi store the remaining
    # gprs
    # GPR6-GPR31 uses a single instruction (does not update r3!)
    stmw        %r6, 4(%r3)

    # Move SPRs into the FFDC buffer, not going to use a gpr multi store so we
    # avoid altering r13, just brute force it for clarity
    # Change mfivpr to mfevpr
    mfevpr      %r5
    _stwi       %r5, %r3, EVPR
    mfxer       %r5
    _stwi       %r5, %r3, EXR
    mfesr       %r5
    _stwi       %r5, %r3, ESR
    mfdear      %r5
    _stwi       %r5, %r3, DEAR
    mfsrr0      %r5
    _stwi       %r5, %r3, SRR0
    mfsrr1      %r5
    _stwi       %r5, %r3, SRR1
    # Change mfcsrr0 to mfsrr2
    mfsrr2      %r5
    _stwi       %r5, %r3, SRR2
    # Change mfcsrr1 to mfsrr3
    mfsrr3      %r5
    _stwi       %r5, %r3, SRR3
    mfmcsr      %r5
    _stwi       %r5, %r3, MCSR
    # Change mfmcsrr0 to mfpid
    mfpid       %r5
    _stwi       %r5, %r3, PID
    # Change mfmcsrr0 to mfzpr
    mfzpr       %r5
    _stwi       %r5, %r3, ZPR
    mfusprg0    %r5
    _stwi       %r5, %r3, USPRG0
    mfsprg0     %r5
    _stwi       %r5, %r3, SPRG0
    mfsprg1     %r5
    _stwi       %r5, %r3, SPRG1
    mfsprg2     %r5
    _stwi       %r5, %r3, SPRG2
    mfsprg3     %r5
    _stwi       %r5, %r3, SPRG3
    mfsprg4     %r5
    _stwi       %r5, %r3, SPRG4
    mfsprg5     %r5
    _stwi       %r5, %r3, SPRG5
    mfsprg6     %r5
    _stwi       %r5, %r3, SPRG6
    mfsprg7     %r5
    _stwi       %r5, %r3, SPRG7
    mftcr       %r5
    _stwi       %r5, %r3, TCR
    mftsr       %r5
    _stwi       %r5, %r3, TSR

    # DBCR0 is saved by __ssx_checkpoint_panic_and_save_ffdc and
    # __occ_checkpoint_panic_and_save_ffdc since those callers will overwrite
    # DBCR0 before we get here.

    # Save debug control reg 1, debug status reg, interrupt status regs and
    # unconditional debug event registers
    # Save dbcr1
    mfdbcr1     %r5
    _stwi       %r5, %r3, DBCR1
    # Save dbsr
    mfdbsr      %r5
    _stwi       %r5, %r3, DBSR
    # Save OISR0
    # Use _lwzi instead of _liw
    _lwzi       %r5, %r5, OCB_OISR0
    _stwi       %r5, %r3, OISR0
    # Save OISR1
    _lwzi       %r5, %r5, OCB_OISR1
    _stwi       %r5, %r3, OISR1

    # Save OCCMISC
    _lwzi       %r5, %r5, OCB_OCCMISC
    _stwi       %r5, %r3, OCCMISC

    # Save OHTMCR
    _lwzi       %r5, %r5, OCB_OHTMCR
    _stwi       %r5, %r3, OHTMCR

    # Save OIMR0
    _lwzi       %r5, %r5, OCB_OIMR0
    _stwi       %r5, %r3, OIMR0
    # Save OIMR1
    _lwzi       %r5, %r5, OCB_OIMR1
    _stwi       %r5, %r3, OIMR1
    # Save OITR0
    _lwzi       %r5, %r5, OCB_OITR0
    _stwi       %r5, %r3, OITR0
    # Save OITR1
    _lwzi       %r5, %r5, OCB_OITR1
    _stwi       %r5, %r3, OITR1
    # Save OIEPR0
    _lwzi       %r5, %r5, OCB_OIEPR0
    _stwi       %r5, %r3, OIEPR0
    # Save OIEPR1
    _lwzi       %r5, %r5, OCB_OIEPR1
    _stwi       %r5, %r3, OIEPR1
    # Save OEHDR
    _lwzi       %r5, %r5, OCB_OEHDR
    _stwi       %r5, %r3, OEHDR
    # Save OCICFG
    _lwzi       %r5, %r5, OCB_OCICFG
    _stwi       %r5, %r3, OCICFG
    # Save ONISR0
    _lwzi       %r5, %r5, OCB_ONISR0
    _stwi       %r5, %r3, ONISR0
    # Save ONISR1
    _lwzi       %r5, %r5, OCB_ONISR1
    _stwi       %r5, %r3, ONISR1
    # Save OCISR0
    _lwzi       %r5, %r5, OCB_OCISR0
    _stwi       %r5, %r3, OCISR0
    # Save OCISR1
    _lwzi       %r5, %r5, OCB_OCISR1
    _stwi       %r5, %r3, OCISR1
    # Save OCCFLG0
    _lwzi       %r5, %r5, OCB_OCCFLG0
    _stwi       %r5, %r3, OCCFLG0
    # Save OCCHBR
    _lwzi       %r5, %r5, OCB_OCCHBR
    _stwi       %r5, %r3, OCCHBR
    # Save PBABASE
    _lwzi       %r5, %r5, PBA_OCI_BASE
    _stwi       %r5, %r3, PBABASE
    # Save PBAMODE
    _lwzi       %r5, %r5, PBA_MODE
    _stwi       %r5, %r3, PBAMODE
    # Save PBASLVCTL0
    _lwzi       %r5, %r5, PBA_SLVCTL0
    _stwi       %r5, %r3, PBASLVCTL0
    # Save PBASLVCTL1
    _lwzi       %r5, %r5, PBA_SLVCTL1
    _stwi       %r5, %r3, PBASLVCTL1
    # Save PBASLVCTL2
    _lwzi       %r5, %r5, PBA_SLVCTL2
    _stwi       %r5, %r3, PBASLVCTL2
    # Save PBASLVCTL3
    _lwzi       %r5, %r5, PBA_SLVCTL3
    _stwi       %r5, %r3, PBASLVCTL3
    # Save BCDE_CTL
    _lwzi       %r5, %r5, PBA_BCDE_CTL
    _stwi       %r5, %r3, BCDE_CTL
    # Save BCDE_SET
    _lwzi       %r5, %r5, PBA_BCDE_SET
    _stwi       %r5, %r3, BCDE_SET
    # Save BCDE_STAT
    _lwzi       %r5, %r5, PBA_BCDE_STAT
    _stwi       %r5, %r3, BCDE_STAT
    # Save BCDE_DR
    _lwzi       %r5, %r5, PBA_BCDE_DR
    _stwi       %r5, %r3, BCDE_DR
    # Save BCDE_OCIBAR
    _lwzi       %r5, %r5, PBA_BCDE_OCIBAR
    _stwi       %r5, %r3, BCDE_OCIBAR
    # Save BCUE_CTL
    _lwzi       %r5, %r5, PBA_BCUE_CTL
    _stwi       %r5, %r3, BCUE_CTL
    # Save BCUE_SET
    _lwzi       %r5, %r5, PBA_BCUE_SET
    _stwi       %r5, %r3, BCUE_SET
    # Save BCUE_STAT
    _lwzi       %r5, %r5, PBA_BCUE_STAT
    _stwi       %r5, %r3, BCUE_STAT
    # Save BCUE_DR
    _lwzi       %r5, %r5, PBA_BCUE_DR
    _stwi       %r5, %r3, BCUE_DR
    # Save BCUE_OCIBAR
    _lwzi       %r5, %r5, PBA_BCUE_OCIBAR
    _stwi       %r5, %r3, BCUE_OCIBAR
    # Save OCC_ACTION
    _lwzi       %r5, %r5, PBA_OCCACT
    _stwi       %r5, %r3, OCC_ACTION
    # Save PBABAR0
    _lwzi       %r5, %r5, PBA_BAR0
    _stwi       %r5, %r3, PBABAR0
    # Save PBABAR1
    _lwzi       %r5, %r5, PBA_BAR1
    _stwi       %r5, %r3, PBABAR1
    # Save PBABAR2
    _lwzi       %r5, %r5, PBA_BAR2
    _stwi       %r5, %r3, PBABAR2
    # Save PBABAR3
    _lwzi       %r5, %r5, PBA_BAR3
    _stwi       %r5, %r3, PBABAR3
    # Save PBABARMSK0
    _lwzi       %r5, %r5, PBA_BARMSK0
    _stwi       %r5, %r3, PBABARMSK0
    # Save PBABARMSK1
    _lwzi       %r5, %r5, PBA_BARMSK1
    _stwi       %r5, %r3, PBABARMSK1
    # Save PBABARMSK2
    _lwzi       %r5, %r5, PBA_BARMSK2
    _stwi       %r5, %r3, PBABARMSK2
    # Save PBABARMSK3
    _lwzi       %r5, %r5, PBA_BARMSK3
    _stwi       %r5, %r3, PBABARMSK3
    # Save PBAERRRPT0
    _lwzi       %r5, %r5, PBA_ERRRPT0
    _stwi       %r5, %r3, PBAERRRPT0
    # Save PBAERRRPT1
    _lwzi       %r5, %r5, PBA_ERRRPT1
    _stwi       %r5, %r3, PBAERRRPT1
    # Save PBAERRRPT2
    _lwzi       %r5, %r5, PBA_ERRRPT2
    _stwi       %r5, %r3, PBAERRRPT2
    # Save PBARBUFVAL0
    _lwzi       %r5, %r5, PBA_RBUFVAL0
    _stwi       %r5, %r3, PBARBUFVAL0
    # Save PBARBUFVAL1
    _lwzi       %r5, %r5, PBA_RBUFVAL1
    _stwi       %r5, %r3, PBARBUFVAL1
    # Save PBARBUFVAL2
    _lwzi       %r5, %r5, PBA_RBUFVAL2
    _stwi       %r5, %r3, PBARBUFVAL2
    # Save PBARBUFVAL3
    _lwzi       %r5, %r5, PBA_RBUFVAL3
    _stwi       %r5, %r3, PBARBUFVAL3
    # Save PBARBUFVAL4
    _lwzi       %r5, %r5, PBA_RBUFVAL4
    _stwi       %r5, %r3, PBARBUFVAL4
    # Save PBARBUFVAL5
    _lwzi       %r5, %r5, PBA_RBUFVAL5
    _stwi       %r5, %r3, PBARBUFVAL5
    # Save PBAWBUFVAL0
    _lwzi       %r5, %r5, PBA_WBUFVAL0
    _stwi       %r5, %r3, PBAWBUFVAL0
    # Save PBAWBUFVAL1
    _lwzi       %r5, %r5, PBA_WBUFVAL1
    _stwi       %r5, %r3, PBAWBUFVAL1
    # Save PBASLVRST
    _lwzi       %r5, %r5, PBA_SLVRST
    _stwi       %r5, %r3, PBASLVRST

    # Save ssx timebase
    _lwzsd      %r5, __ssx_timebase_frequency_mhz
    _stwi       %r5, %r3, FFDC_TIMEBASE

    # Save the buildname
    # memcpy(OCC_BUILD_NAME, G_occ_buildname, 16)
    _liwa       %r3, OCC_BUILD_NAME
    _liw        %r4, G_occ_buildname
    li          %r5, 16
    bl          memcpy

    # No OCCLFIR.  OCCLFIR is SCOM only, no SCOM available

    # Save PBAFIR
    _lwzi       %r5, %r5, PBA_FIR
    _stwi       %r5, %r3, PBAFIR

    # Save core (de)configuration
    _liw        %r5, OCB_CCSR
    _stwi       %r5, %r3, CORES_DECON

    # Add the Main thread dump
    _liw        %r3, Main_thread
    _liwa       %r4, MAIN_THREAD_DUMP
    bl          ffdc_thread_dumper

    # Add the Command handler thread dump
    _liw        %r3, Cmd_Hndl_thread
    _liwa       %r4, CMD_THREAD_DUMP
    bl          ffdc_thread_dumper

    # Add the DCOM thread dump
    _liw        %r3, Dcom_thread
    _liwa       %r4, DCOM_THREAD_DUMP
    bl          ffdc_thread_dumper

    # Unwind the current stack
    mr          %r3, %r1
    _liwa       %r4, STACK_TRACE
    li          %r5, 8
    bl          ffdc_stack_unwind

    # Mark the end of the buffer
    _liwa       %r5, FFDC_END_MARKER
    _stwi       %r5, %r3, FFDC_END

    # Force a sync and flush the data cache
    sync
    _liwa       %r5, FFDC_BUFFER_ADDR
    li          %r4, 0
1:  dcbf        %r4, %r5
    addi        %r4, %r4, 32
    cmpwi       %r4, FFDC_BUFFER_LENGTH
    blt         1b

    # Restore GPR4-GPR31
    #   - r0 and r3 are volatile
    #   - r1 and r2 are intact
    _liwa       %r3, GPR4
    lmw         %r4, 0(%r3)

    # Ensure preceding memory access synchronization
    sync

    # Return to caller
    lwz        %r0, 12(%r1)
    addi       %r1, %r1, 8
    mtlr       %r0
    blr


# ----------------
# End of ll_ffdc.S
# ----------------
