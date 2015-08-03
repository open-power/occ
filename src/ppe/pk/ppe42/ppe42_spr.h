/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/ppe42/ppe42_spr.h $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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
#ifndef __PPE42_SPR_H__
#define __PPE42_SPR_H__
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppe42_spr.h
/// \brief Everything related to PPE42-specific SPRs

/// \defgroup ppe42_sprs PPE42 SPRs
///
/// These are the documented SPRs of the PPE42.  Most of these SPRs are
/// available in RISCWatch and eCmd using the defined names (minus SPRN_). In
/// some cases RISCWatch/eCMD use different names, which appear in square
/// brackets in the brief comments for each register. RISCWatch/eCMD also
/// allow CR, MSR and IAR (Instruction Address Register) to be accessed as
/// SPRs.
///
/// @{

 #define SPRN_XER         0x001 /// Fixed-point exception register    
 #define SPRN_LR          0x008 /// Link register                     
 #define SPRN_CTR         0x009 /// Count register
 #define SPRN_DEC         0x016 /// Decrementer
 #define SPRN_SRR0        0x01a /// Save/restore register 0           
 #define SPRN_SRR1        0x01b /// Save/restore register 1           
 #define SPRN_EDR         0x03d /// Error Data Register
 #define SPRN_ISR         0x03e /// Interrupt Status Register
 #define SPRN_IVPR        0x03f /// Interrupt Vector Prefix Register
 #define SPRN_SPRG0       0x110 /// SPR general register 0            
 #define SPRN_PIR         0x11e /// Processor Identification Register
 #define SPRN_PVR         0x11f /// Processor version register        
 #define SPRN_DBCR        0x134 /// Debug Control Register
 #define SPRN_DACR        0x13c /// Debug Address Compare Register
 #define SPRN_TSR         0x150 /// Timer Status Register
 #define SPRN_TCR         0x154 /// Timer Control Register

/* DBCR - Debug Control Register */

#define DBCR_RST         0x30000000 /* Reset: 01=Soft Reset, 10=Hard Reset, 11=Halt */
#define DBCR_TRAP        0x01000000 /* Trap Instruction Enable */
#define DBCR_IACE        0x00800000 /* Instruction Address Compare Enable */
#define DBCR_DACE        0x000c0000 /* Data Address Compare Enable: 01=store, 10=load, 11=both */

/* TCR - Timer Control Register */

#define TCR_WP_MASK    0xc0000000 /* Watchdog timer select bits */
#define TCR_WP_0       0x00000000 /* WDT uses timer 0 */
#define TCR_WP_1       0x40000000 /* WDT uses timer 1 */
#define TCR_WP_2       0x80000000 /* WDT uses timer 2 */
#define TCR_WP_3       0xc0000000 /* WDT uses timer 3 */
#define TCR_WRC_MASK   0x30000000 /* Watchdog Reset Control mask */
#define TCR_WRC_NONE   0x00000000 /* WDT results in no action */
#define TCR_WRC_SOFT   0x10000000 /* WDT results in Soft reset */
#define TCR_WRC_HARD   0x20000000 /* WDT results in Hard reset */
#define TCR_WRC_HALT   0x30000000 /* WDT results in Halt */
#define TCR_WIE        0x08000000 /* Watchdog Interrupt Enable */
#define TCR_DIE        0x04000000 /* Decrementer Interrupt Enable */
#define TCR_FP_MASK    0x03000000 /* FIT Timer Select bits*/
#define TCR_FP_0       0x00000000 /* FIT uses timer 0 */
#define TCR_FP_1       0x01000000 /* FIT uses timer 1 */
#define TCR_FP_2       0x02000000 /* FIT uses timer 2 */
#define TCR_FP_3       0x03000000 /* FIT uses timer 3 */
#define TCR_FIE        0x00800000 /* FIT Interrupt Enable */
#define TCR_DS         0x00400000 /* Decrementer timer select: 0=every cycle, 1=use dec_timer input signal */

#ifndef __ASSEMBLER__

typedef union {
    uint32_t value;
    struct {
        unsigned int wp       : 2;
        unsigned int wrc      : 2;
        unsigned int wie      : 1;
        unsigned int die      : 1;
        unsigned int fp       : 2;
        unsigned int fie      : 1;
        unsigned int ds       : 1;
        unsigned int reserved : 22;
    } fields;
} Ppe42TCR;

#endif /* __ASSEMBLER__ */

/* TSR - Timer Status Register */

#define TSR_ENW        0x80000000 /* Enable Next Watchdog */
#define TSR_WIS        0x40000000 /* Watchdog Interrupt Status */
#define TSR_WRS_MASK   0x30000000 /* Watchdog Reset Status */
#define TSR_WRS_NONE   0x00000000 /* No watchdog reset has occurred */
#define TSR_WRS_SOFT   0x10000000 /* Soft reset was forced by the watchdog */
#define TSR_WRS_HARD   0x20000000 /* Hard reset was forced by the watchdog */
#define TSR_WRS_HALT   0x30000000 /* Halt was forced by the watchdog */
#define TSR_DIS        0x08000000 /* Decrementer Interrupt Status */
#define TSR_FIS        0x04000000 /* FIT Interrupt Status */

/* PIR - Processor Identification Register */
#define PIR_PPE_TYPE_MASK       0x000000E0
#define PIR_PPE_TYPE_GPE        0x00000020
#define PIR_PPE_TYPE_CME        0x00000040
#define PIR_PPE_INSTANCE_MASK   0x0000001F

#ifndef __ASSEMBLER__

/// Move From SPR
///
///  Note that \a sprn must be a compile-time constant.

#define mfspr(sprn)                                             \
   ({uint32_t __value;                                          \
   asm volatile ("mfspr %0, %1" : "=r" (__value) : "i" (sprn)); \
   __value;})    


/// Move to SPR
///
///  Note that \a sprn must be a compile-time constant.

#define mtspr(sprn, value)                                        \
   ({uint32_t __value = (value);                                  \
     asm volatile ("mtspr %0, %1" : : "i" (sprn), "r" (__value)); \
   })
    

/// Read-Modify-Write an SPR with OR (Set SPR bits)
///
///  Note that \a sprn must be a compile-time constant. This operation is only
///  guaranteed atomic in a critical section.

#define or_spr(sprn, x) \
    mtspr(sprn, mfspr(sprn) | (x))


/// Read-Modify-Write an SPR with AND complement (Clear SPR bits)
///
///  Note that \a sprn must be a compile-time constant.  This operation is only
///  guaranteed atomic in a critical section.

#define andc_spr(sprn, x) \
    mtspr(sprn, mfspr(sprn) & ~(x))

#endif  /* __ASSEMBLER__ */

#ifdef __ASSEMBLER__

        /// \cond

        // Use this macro to define new mt<spr> and mf<spr> instructions that
        // may not exist in the assembler.

        .macro _sprinstrs, name, num 
        .macro mt\name, reg
        mtspr   \num, \reg
        .endm
        .macro mf\name, reg
        mfspr   \reg, \num
        .endm
        .endm

        _sprinstrs      dbcr,   SPRN_DBCR
        _sprinstrs      tcr,    SPRN_TCR
        _sprinstrs      tsr,    SPRN_TSR
        _sprinstrs      sprg0,  SPRN_SPRG0
        _sprinstrs      ivpr,   SPRN_IVPR
        _sprinstrs      dec,    SPRN_DEC

        /// \endcond

#endif  /* __ASSEMBLER__ */

#endif /* __PPE42_SPR_H__ */
