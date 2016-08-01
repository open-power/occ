/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ppc405/ppc405_spr.h $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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
#ifndef __PPC405_SPR_H__
#define __PPC405_SPR_H__

/// \file ppc405_spr.h
/// \brief Everything related to PPC405-specific SPRs

/// \defgroup ppc405_sprs PowerPC 405 SPRs
///
/// These are the documented SPRs of the PPC405.  Most of these SPRs are
/// available in RISCWatch and eCmd using the defined names (minus SPRN_). In
/// some cases RISCWatch/eCMD use different names, which appear in square
/// brackets in the brief comments for each register. RISCWatch/eCMD also
/// allow CR, MSR and IAR (Instruction Address Register) to be accessed as
/// SPRs.
///
/// @{

#define SPRN_CCR0        0x3b3 /// Core configuration register 0     
#define SPRN_CCR1        0x378 /// Core configuration register 1     
#define SPRN_CTR         0x009 /// Count register                    
#define SPRN_DAC1        0x3f6 /// Data address compare 1            
#define SPRN_DAC2        0x3f7 /// Data address compare 2            
#define SPRN_DBCR0       0x3f2 /// Debug control register 0          
#define SPRN_DBCR1       0x3bd /// Debug control register 1          
#define SPRN_DBSR        0x3f0 /// Debug status register             
#define SPRN_DCCR        0x3fa /// Data cacheability (real mode)     
#define SPRN_DCWR        0x3ba /// Data cache writeback (real mode)  
#define SPRN_DEAR        0x3d5 /// Data exception address register   
#define SPRN_DVC1        0x3b6 /// Data value compare 1              
#define SPRN_DVC2        0x3b7 /// Data value compare 2              
#define SPRN_ESR         0x3d4 /// Exception syndrome register       
#define SPRN_EVPR        0x3d6 /// Exception. vec. prefix reg.       
#define SPRN_IAC1        0x3f4 /// Instruction address compare 1     
#define SPRN_IAC2        0x3f5 /// Instruction address compare 2     
#define SPRN_IAC3        0x3b4 /// Instruction address compare 3     
#define SPRN_IAC4        0x3b5 /// Instruction address compare 4     
#define SPRN_ICCR        0x3fb /// Instruction cache. (real mode)    
#define SPRN_ICDBDR      0x3d3 /// Instruction cache debug data reg. 
#define SPRN_LR          0x008 /// Link register                     
#define SPRN_MCSR        0x23c /// Machine check syndrome register   
#define SPRN_PID         0x3b1 /// Process ID                        
#define SPRN_PIT         0x3db /// Programmable interrupt timer      
#define SPRN_PVR         0x11f /// Processor version register        
#define SPRN_SGR         0x3b9 /// Storage guarded (real mode)       
#define SPRN_SLER        0x3bb /// Storage little-endian (real mode) 
#define SPRN_SPRG0       0x110 /// SPR general register 0            
#define SPRN_SPRG1       0x111 /// SPR general register 1            
#define SPRN_SPRG2       0x112 /// SPR general register 2            
#define SPRN_SPRG3       0x113 /// SPR general register 3            
#define SPRN_SPRG4       0x114 /// SPR general register 4 [SPRG4_W]  
#define SPRN_SPRG5       0x115 /// SPR general register 5 [SPRG5_W]  
#define SPRN_SPRG6       0x116 /// SPR general register 6 [SPRG6_W]  
#define SPRN_SPRG7       0x117 /// SPR general register 7 [SPRG7_W]  
#define SPRN_SRR0        0x01a /// Save/restore register 0           
#define SPRN_SRR1        0x01b /// Save/restore register 1           
#define SPRN_SRR2        0x3de /// Save/restore register 2           
#define SPRN_SRR3        0x3df /// Save/restore register 3           
#define SPRN_SU0R        0x3bc /// Storage user 0 (real mode)        
#define SPRN_TBL         0x11c /// Time base lower [TBL_W]           
#define SPRN_TBU         0x11d /// Time base upper [TBU_W]           
#define SPRN_TCR         0x3da /// Timer control register            
#define SPRN_TSR         0x3d8 /// Timer status register             
#define SPRN_USPRG0      0x100 /// User read/write SPR general 0     
#define SPRN_XER         0x001 /// Fixed-point exception register    
#define SPRN_ZPR         0x3b0 /// Zone protection register          
#define SPRN_UR_SPRG4    0x104 /// User-readable SPRG4 [SPRG4_R]     
#define SPRN_UR_SPRG5    0x105 /// User-readable SPRG5 [SPRG5_R]     
#define SPRN_UR_SPRG6    0x106 /// User-readable SPRG6 [SPRG6_R]     
#define SPRN_UR_SPRG7    0x107 /// User-readable SPRG7 [SPRG7_R]     
#define SPRN_UR_TBL      0x10c /// User-readable TBL   [TBL, TBL_R]  
#define SPRN_UR_TBU      0x10d /// User-readable TBU   [TBU, TBU_R]  

/// @}

/// \defgroup ppc405_undocumented_sprs PowerPC 405 Undocumented SPRs
///
/// These are undocumented SPRs related to RISCWatch and debugging.  These
/// registers are also available in RISCWatch/eCMD.
///
/// - DBDR is a scratch register used by RISCwatch when "RAM-ing" data in/out of
/// the core. This register can be read and written.
///
/// - DBSRS and TSRS are "hidden" registers connected to DBSR and TSR
/// respectively. These are write-only registers. When written, any 1 bits in
/// the write data are OR-ed into the DBSR and TSR respectively, as a way to
/// force status bits and cause interrupts.
///
/// @{

#define SPRN_DBDR        0x3f3 /// Debug data register               0x3f3 */
#define SPRN_DBSRS       0x3f1 /// Debug status register set         0x3f1 */
#define SPRN_TSRS        0x3d9 /// Timer status register set         0x3d9 */

/// @}

/* CCR0 - Cache Control Register 0 */

#define CCR0_LWL   0x02000000   /* Load Word as Line */
#define CCR0_LWOA  0x01000000   /* Load Without Allocate */
#define CCR0_SWOA  0x00800000   /* Store Without Allocate */
#define CCR0_DPP1  0x00400000   /* DCU PLB Priority Bit 1 */
#define CCR0_IPP0  0x00200000   /* ICU PLB Priority Bit 0 */
#define CCR0_IPP1  0x00100000   /* ICU PLB Priority Bit 1 */
#define CCR0_DPE   0x00080000   /* Data Cache Parity Enable */
#define CCR0_DPP   0x00040000   /* DCU Parity is Precise (0/1) */
#define CCR0_U0XE  0x00020000   /* Enable U0 Exception */
#define CCR0_LDBE  0x00010000   /* Load Debug Enable */
#define CCR0_IPE   0x00002000   /* Instruction Cache Parity Enable */
#define CCR0_TPE   0x00001000   /* TLB Parity Enable */
#define CCR0_PFC   0x00000800   /* ICU Prefetching for Cacheable Regions */
#define CCR0_PFNC  0x00000400   /* ICU Prefetching for Non-Cacheable Regions */
#define CCR0_NCRS  0x00000200   /* Non-Cacheable ICU request is 16(0)/32(1)B */
#define CCR0_FWOA  0x00000100   /* Fetch Without Allocate */
#define CCR0_CIS   0x00000010   /* Cache Information Select Data(0)/Tag(1) */
#define CCR0_PRS   0x00000008   /* Parity Read Select */
#define CCR0_CWS   0x00000001   /* Cache Way Select A(0)/B(1) */

/* CCR1 - Cache Control Register 1 */

#define CCR1_ICTE 0x80000000    /* Instruction Cache Tag Parity Insertion */
#define CCR1_ICDE 0x40000000    /* Instruction Cache Data Parity Insertion */
#define CCR1_DCTE 0x20000000    /* Data Cache Tag Parity Insertion */
#define CCR1_DCDE 0x10000000    /* Data Cache Data Parity Insertion */
#define CCR1_TLBE 0x08000000    /* TLB Parity Insertion */

/* DBCR0 - Debug Control Register 0 */

#define DBCR0_EDM         0x80000000 /* External Debug Mode */
#define DBCR0_IDM         0x40000000 /* Internal Debug Mode */
#define DBCR0_RST_MASK    0x30000000 /* ReSeT */
#define DBCR0_RST_NONE    0x00000000 /* No action */
#define DBCR0_RST_CORE    0x10000000 /* Core reset */
#define DBCR0_RST_CHIP    0x20000000 /* Chip reset */
#define DBCR0_RST_SYSTEM  0x30000000 /* System reset */
#define DBCR0_IC          0x08000000 /* Instruction Completion debug event */
#define DBCR0_BT          0x04000000 /* Branch Taken debug event */
#define DBCR0_EDE         0x02000000 /* Exception Debug Event */
#define DBCR0_TDE         0x01000000 /* Trap Debug Event */
#define DBCR0_IA1         0x00800000 /* IAC (Instruction Address Compare) 1 debug event */
#define DBCR0_IA2         0x00400000 /* IAC 2 debug event */
#define DBCR0_IA12        0x00200000 /* Instruction Address Range Compare 1-2 */
#define DBCR0_IA12X       0x00100000 /* IA12 eXclusive */
#define DBCR0_IA3         0x00080000 /* IAC 3 debug event */
#define DBCR0_IA4         0x00040000 /* IAC 4 debug event */
#define DBCR0_IA34        0x00020000 /* Instruction Address Range Compare 3-4 */
#define DBCR0_IA34X       0x00010000 /* IA34 eXclusive */
#define DBCR0_IA12T       0x00008000 /* Instruction Address Range Compare 1-2 range Toggle */
#define DBCR0_IA34T       0x00004000 /* Instruction Address Range Compare 3-4 range Toggle */
#define DBCR0_FT          0x00000001 /* Freeze Timers on debug event */

/* DBSR - Debug Status Register */

#define DBSR_IC   0x80000000 /* Instruction completion debug event */
#define DBSR_BT   0x40000000 /* Branch Taken debug event */
#define DBSR_EDE  0x20000000 /* Exception debug event */
#define DBSR_TIE  0x10000000 /* Trap Instruction debug event */
#define DBSR_UDE  0x08000000 /* Unconditional debug event */
#define DBSR_IA1  0x04000000 /* IAC1 debug event */
#define DBSR_IA2  0x02000000 /* IAC2 debug event */
#define DBSR_DR1  0x01000000 /* DAC1 Read debug event */
#define DBSR_DW1  0x00800000 /* DAC1 Write debug event */
#define DBSR_DR2  0x00400000 /* DAC2 Read debug event */
#define DBSR_DW2  0x00200000 /* DAC2 Write debug event */
#define DBSR_IDE  0x00100000 /* Imprecise debug event */
#define DBSR_IA3  0x00080000 /* IAC3 debug event */
#define DBSR_IA4  0x00040000 /* IAC4 debug event */
#define DBSR_MRR  0x00000300 /* Most recent reset */

/* TCR - Timer Control Register */

#define TCR_WP_MASK    0xc0000000 /* Watchdog Period mask */
#define TCR_WP_2_17    0x00000000 /* 2**17 clocks */
#define TCR_WP_2_21    0x40000000 /* 2**21 clocks */
#define TCR_WP_2_25    0x80000000 /* 2**25 clocks */
#define TCR_WP_2_29    0xc0000000 /* 2**29 clocks */
#define TCR_WRC_MASK   0x30000000 /* Watchdog Reset Control mask */
#define TCR_WRC_NONE   0x00000000 /* No watchdog reset */
#define TCR_WRC_CORE   0x10000000 /* Core reset */
#define TCR_WRC_CHIP   0x20000000 /* Chip reset */
#define TCR_WRC_SYSTEM 0x30000000 /* System reset */
#define TCR_WIE        0x08000000 /* Watchdog Interrupt Enable */
#define TCR_PIE        0x04000000 /* PIT Interrupt Enable */
#define TCR_FP_MASK    0x03000000 /* FIT Period */
#define TCR_FP_2_9     0x00000000 /* 2**9 clocks */
#define TCR_FP_2_13    0x01000000 /* 2**13 clocks */
#define TCR_FP_2_17    0x02000000 /* 2**17 clocks */
#define TCR_FP_2_21    0x03000000 /* 2**21 clocks */
#define TCR_FIE        0x00800000 /* FIT Interrupt Enable */
#define TCR_ARE        0x00400000 /* Auto-reload Enable */

#ifndef __ASSEMBLER__

typedef union
{
    uint32_t value;
    struct
    {
        unsigned int wp       : 2;
        unsigned int wrc      : 2;
        unsigned int wie      : 1;
        unsigned int pie      : 1;
        unsigned int fp       : 2;
        unsigned int fie      : 1;
        unsigned int are      : 1;
        unsigned int reserved : 22;
    } fields;
} Ppc405TCR;

#endif /* __ASSEMBLER__ */

/* TSR - Timer Status Register */

#define TSR_ENW        0x80000000 /* Enable Next Watchdog */
#define TSR_WIS        0x40000000 /* Watchdog Interrupt Status */
#define TSR_WRS_MASK   0x30000000 /* Watchdog Reset Status */
#define TSR_WRS_NONE   0x00000000 /* No watchdog reset has occurred */
#define TSR_WRS_CORE   0x10000000 /* Core reset was forced by the watchdog */
#define TSR_WRS_CHIP   0x20000000 /* Chip reset was forced by the watchdog */
#define TSR_WRS_SYSTEM 0x30000000 /* System reset was forced by the watchdog */
#define TSR_PIS        0x08000000 /* PIT Interrupt Status */
#define TSR_FIS        0x04000000 /* FIT Interrupt Status */

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


/// Move From Time Base (Lower)

#define mftb() mfspr(SPRN_TBL)

/// Move To Time Base (Lower)

#define mttbl(x) mtspr(SPRN_TBL, (x))

/// Move From Time Base (Upper)

#define mftbu() mfspr(SPRN_TBU)

/// Move To Time Base (UPPER)

#define mttbu(x) mtspr(SPRN_TBU, (x))

#endif  /* __ASSEMBLER__ */

#ifdef __ASSEMBLER__
// *INDENT-OFF*

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

        _sprinstrs      ccr0,   SPRN_CCR0
        _sprinstrs      ccr1,   SPRN_CCR1
        _sprinstrs      dbcr0,  SPRN_DBCR0
        _sprinstrs      dbcr1,  SPRN_DBCR1
        _sprinstrs      dcwr,   SPRN_DCWR
        _sprinstrs      mcsr,   SPRN_MCSR
        _sprinstrs      pid,    SPRN_PID
        _sprinstrs      sgr,    SPRN_SGR
        _sprinstrs      sler,   SPRN_SLER
        _sprinstrs      su0r,   SPRN_SU0R
        _sprinstrs      usprg0, SPRN_USPRG0

        /// \endcond

// *INDENT-ON*
#endif  /* __ASSEMBLER__ */

#endif /* __PPC405_SPR_H__ */
