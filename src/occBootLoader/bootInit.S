# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/occBootLoader/bootInit.S $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2011,2017
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

#*****************************************************************************/
# Includes
#*****************************************************************************/
#include "bootMain.h"
#include "ppc32_asm.h"
#include "ppc405_spr.h"

#*****************************************************************************/
# Functions
#*****************************************************************************/

# Function Specification ******************************************************
#
#  Name: boot_low_level_init
#
#  Description: Low level boot initialization
#
# End Function Specification **************************************************
.global_function __boot_low_level_init

__boot_low_level_init:

# enable machine check exception
# initialize exception vector prefix to 0x80000000
# enable machine check (19th bit)
addis   %r3, 0, 0x8000
mtevpr  %r3
_liwa   %r4, MACHINE_CHECK_ENABLE
mtmsr   %r4

# Check if TSR[WSR] indicates that reset was due to watchdog,
# then initialize external debug event and trap(halt). Otherwise
# continue normally. Setting up external debug event will set FIR bits on halt
mftsr   %r3
_liwa   %r4, (TSR_WRS_CHIP | TSR_WRS_CORE | TSR_WRS_SYSTEM)
AND     %r5, %r3, %r4
li      %r4, 0
cmp     0, 0, %r4, %r5
beq     skip_wd_reset_trap

# Initialize external debug event before doing trap
_liwa   %r3, (DBCR0_EDM | DBCR0_TDE | DBCR0_FT)
mtdbcr0 %r3
tw 31,0,0

skip_wd_reset_trap:
# enable write back data cache and instruction cache
li       %r3, 0
mtdcwr   %r3
_liw     %r3, DATA_CACHE_BLOCK_ENABLE
mtdccr   %r3
mticcr   %r3

dcache_invalidate:
# We loop through addresses 0 ... (DCACHE_SIZE / DCACHE_WAYS) - 1
# invalidating the D-Cache.  The dccci instruction on the 405
# invalidates both ways.
li    %r3, 0
_liwa    %r4, (DATA_CACHE_LINES / 2)
mtctr    %r4

1:
dccci    %r0, %r3
addi    %r3, %r3, CACHE_LINE_SIZE
bdnz    1b
sync

# fill in 16K with zero for the data cache
# set counter to loop for number of data cache lines
_liw    %r3, 0
_liwa   %r5, DATA_CACHE_BLOCK_ADDR
_liwa   %r4, DATA_CACHE_LINES
mtctr   %r4

dcbz_loop:
dcbz    %r5, %r3
addi    %r3, %r3, CACHE_LINE_SIZE
bdnz    dcbz_loop

# write in initialized writeable data if length is non-zero
_liw    %r5, (__START_ADDR__ - 4)
_liw    %r4, __READ_ONLY_DATA_LEN__
add     %r3, %r5, %r4
_liw    %r4, __WRITEABLE_DATA_LEN__
_liw    %r5, (__WRITEABLE_DATA_ADDR__ - 4)
_liw    %r6, 2
srw     %r4, %r4, %r6
_liw    %r6, 0
cmp     0, 0, %r4, %r6
beq     skip_loop
mtctr   %r4

copy_loop:
lwzu    %r4, 4(%r3)
stwu    %r4, 4(%r5)
bdnz    copy_loop

skip_loop:
# set up EABI constant registers r2, r13
_liw    %r2, _SDA2_BASE_
_liw    %r13, _SDA_BASE_

# set up stack pointer register r1
_liwa   %r1, STACK_POINTER_ADDR

# jump to main
b main
.epilogue  __boot_low_level_init

# vector section, put traps on sections after 0000
.section .vectors_0000, "a", @progbits
.global __vectors_0000

__vectors_0000:
.section .vectors_0100, "ax", @progbits
.global __vectors_0100

__vectors_0100:
tw 31,0,0
.section .vectors_0200, "ax", @progbits
.global __vectors_0200

__vectors_0200:
tw 31,0,0
.section .vectors_0300, "ax", @progbits
.global __vectors_0300

__vectors_0300:
tw 31,0,0
.section .vectors_0400, "ax", @progbits
.global __vectors_0400

__vectors_0400:
tw 31,0,0
.section .vectors_0500, "ax", @progbits
.global __vectors_0500

__vectors_0500:
tw 31,0,0
.section .vectors_0600, "ax", @progbits
.global __vectors_0600

__vectors_0600:
tw 31,0,0
.section .vectors_0700, "ax", @progbits
.global __vectors_0700

__vectors_0700:
tw 31,0,0
