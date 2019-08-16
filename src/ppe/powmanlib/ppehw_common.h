/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/powmanlib/ppehw_common.h $                            */
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
#ifndef __PPEHW_COMMON_H__
#define __PPEHW_COMMON_H__

#include "ppe42_math.h"

/// 64bits data
typedef union
{
    uint64_t value;
    struct
    {
        uint32_t upper;
        uint32_t lower;
    } words;
} data64_t;

/// 64bit variable breaking into two 32bits macro
#define UPPER32(variable) (uint32_t)((variable >> 32) & 0xFFFFFFFF)
#define LOWER32(variable) (uint32_t)(variable & 0xFFFFFFFF)

/// Mark and Tag

// TAG = [18bits reserved][4bits targets][10bits code]

#define CODE2REGA(code)          ((code & 0x1F00) >> 8)
#define CODE2REGB(code)          ((code & 0xF8) >> 3)
#define CODE2TAG(targets, code)  ((targets << 10) | (code >> 3))
#define TAG_SPRG0(tag)           {ppe42_app_ctx_set(tag);}

#if EPM_TUNING
#define MARK_TRAP(code) \
    {asm volatile ("tw 0, %0, %1" : : \
                   "i" (CODE2REGA(code)), \
                   "i" (CODE2REGB(code)));}
#else
#define MARK_TRAP(code)
#endif

#define MARK_TAG(targets, code) \
    TAG_SPRG0(CODE2TAG(targets, code)) \
    MARK_TRAP(code)


/// Wait Macro

#define PPE_CORE_CYCLE_RATIO       8 // core is 8 times faster than qme
#define PPE_FOR_LOOP_CYCLES        4 // fused compare branch(3), addition(1)
#define PPE_CORE_CYCLE_DIVIDER     (PPE_CORE_CYCLE_RATIO*PPE_FOR_LOOP_CYCLES)
#ifdef USE_PPE_IMPRECISE_MODE
#define PPE_WAIT_CORE_CYCLES(cc) \
    {volatile uint32_t l;asm volatile ("sync");for(l=0;l<cc/PPE_CORE_CYCLE_DIVIDER;l++);}
#else
#define PPE_WAIT_CORE_CYCLES(cc) \
    {volatile uint32_t l;for(l=0;l<cc/PPE_CORE_CYCLE_DIVIDER;l++);}
#endif


#define PPE_WAIT_4NOP_CYCLES       \
    asm volatile ("tw 0, 0, 0");   \
    asm volatile ("tw 0, 0, 0");   \
    asm volatile ("tw 0, 0, 0");   \
    asm volatile ("tw 0, 0, 0");


// PPE MULTICAST

enum PPE_MULTICAST_TYPES
{
    PPE_MC_RD_OR                     = 0,
    PPE_MC_RD_AND                    = 1,
    PPE_MC_RD_EQU                    = 4,
    PPE_MC_WR                        = 5
};

// 0     | 1    | 2-4  | 5-7      | 8 | 9-11  | 12-15  | 16-19  | 20-31
// 0/pcb | MC=0 | baseId          | Q | 0     | EndSel | Region |
// R/nW  | MC=0 | CpltId          | 1111/PcbM | EndSel | Region | Addr
// 0/pcb | MC=1 | MC_T | 0        | Q | 0     | EndSel | Region |
// R/nW  | MC=1 | MC_T | 111/MC_G | 1111/PcbM | EndSel | Region | Addr

// Current MC GROUP for all Quads is 6
#define PPE_MC_ENABLE_GROUP 0x46000000

// base macros are for per-core
#define PPE_SCOM_ADDR_MC(base_addr, mc_type, core_select) \
    (PPE_MC_ENABLE_GROUP | (mc_type << 27) | (core_select << 12) | (base_addr))

#define PPE_SCOM_ADDR_UC(base_addr, quad_select, core_select) \
    ((quad_select << 24) | (core_select << 12) | (base_addr))

// *_Q macros are for per-quad where core_select is 0
// when you mc to quad level register(non-per-core/non-cpms)
// feed core_select = 0
#define PPE_SCOM_ADDR_MC_Q(base_addr, mc_type)        PPE_SCOM_ADDR_MC(base_addr, mc_type, 0)
#define PPE_SCOM_ADDR_UC_Q(base_addr, quad_select)    PPE_SCOM_ADDR_UC(base_addr, quad_select, 0)

// MC macros with MC_TYPEs, UC doesnt have MC_TYPEs

// Per-Core:
#define PPE_SCOM_ADDR_MC_WR(base_addr,  core_select)  PPE_SCOM_ADDR_MC(base_addr, PPE_MC_WR,     core_select)
#define PPE_SCOM_ADDR_MC_OR(base_addr,  core_select)  PPE_SCOM_ADDR_MC(base_addr, PPE_MC_RD_OR,  core_select)
#define PPE_SCOM_ADDR_MC_AND(base_addr, core_select)  PPE_SCOM_ADDR_MC(base_addr, PPE_MC_RD_AND, core_select)
#define PPE_SCOM_ADDR_MC_EQU(base_addr, core_select)  PPE_SCOM_ADDR_MC(base_addr, PPE_MC_RD_EQU, core_select)

// Per-Quad:
#define PPE_SCOM_ADDR_MC_Q_WR(base_addr)              PPE_SCOM_ADDR_MC_Q(base_addr, PPE_MC_WR)
#define PPE_SCOM_ADDR_MC_Q_OR(base_addr)              PPE_SCOM_ADDR_MC_Q(base_addr, PPE_MC_RD_OR)
#define PPE_SCOM_ADDR_MC_Q_AND(base_addr)             PPE_SCOM_ADDR_MC_Q(base_addr, PPE_MC_RD_AND)
#define PPE_SCOM_ADDR_MC_Q_EQU(base_addr)             PPE_SCOM_ADDR_MC_Q(base_addr, PPE_MC_RD_EQU)


// GetScom

#define PPE_GETSCOM(addr, data)                        \
    PPE_LVD(addr, data);

// UC Q
#define PPE_GETSCOM_UC_Q(addr, quad_select, data)      \
    PPE_LVD(PPE_SCOM_ADDR_UC_Q(addr, quad_select), data)

// MC Q
#define PPE_GETSCOM_MC_Q(addr, mc_type, data)          \
    PPE_LVD(PPE_SCOM_ADDR_MC_Q(addr, mc_type), data)

// MC Q OR
#define PPE_GETSCOM_MC_Q_OR(addr, data)                \
    PPE_LVD(PPE_SCOM_ADDR_MC_Q_OR(addr), data)

// MC Q AND
#define PPE_GETSCOM_MC_Q_AND(addr, data)               \
    PPE_LVD(PPE_SCOM_ADDR_MC_Q_AND(addr), data)

// MC Q EQU
#define PPE_GETSCOM_MC_Q_EQU(addr, data)               \
    PPE_LVD(PPE_SCOM_ADDR_MC_Q_EQU(addr), data)

// UC C
#define PPE_GETSCOM_UC(addr, quad_select, core_select, data)      \
    PPE_LVD(PPE_SCOM_ADDR_UC(addr, quad_select, core_select), data)

// MC C
#define PPE_GETSCOM_MC(addr, mc_type, core_select, data)          \
    PPE_LVD(PPE_SCOM_ADDR_MC(addr, mc_type, core_select), data)

// MC C OR
#define PPE_GETSCOM_MC_OR(addr, core_select, data)                \
    PPE_LVD(PPE_SCOM_ADDR_MC_OR(addr, core_select), data)

// MC C AND
#define PPE_GETSCOM_MC_AND(addr, core_select, data)               \
    PPE_LVD(PPE_SCOM_ADDR_MC_AND(addr, core_select), data)

// MC C AND
#define PPE_GETSCOM_MC_EQU(addr, core_select, data)               \
    PPE_LVD(PPE_SCOM_ADDR_MC_EQU(addr, core_select), data)


// PutScom

// queued putscom if enabled; otherwise default with nop
#if defined(USE_QME_QUEUED_SCOM)
#define PPE_QUEUED_SCOM(addr)                                     \
    (addr | 0x00800000)
#else
#define PPE_QUEUED_SCOM(addr)                                     \
    (addr)
#endif

// use this to override undesired queued putscom with nop
#define PPE_PUTSCOM_NOQ(addr, data)                               \
    putscom_norc(addr, data);

#define PPE_PUTSCOM(addr, data)                                   \
    putscom_norc(PPE_QUEUED_SCOM(addr), data);

#define PPE_PUTSCOM_UC(addr, quad_select, core_select, data)      \
    putscom_norc(PPE_SCOM_ADDR_UC(PPE_QUEUED_SCOM(addr), quad_select, core_select), data)

#define PPE_PUTSCOM_MC(addr, core_select, data)                   \
    putscom_norc(PPE_SCOM_ADDR_MC_WR(PPE_QUEUED_SCOM(addr), core_select), data)

#define PPE_PUTSCOM_UC_Q(addr, quad_select, data)                 \
    putscom_norc(PPE_SCOM_ADDR_UC_Q(PPE_QUEUED_SCOM(addr), quad_select), data)

#define PPE_PUTSCOM_MC_Q(addr, data)                              \
    putscom_norc(PPE_SCOM_ADDR_MC_Q_WR(PPE_QUEUED_SCOM(addr)), data)


#endif  /* __PPEHW_COMMON_H__ */
