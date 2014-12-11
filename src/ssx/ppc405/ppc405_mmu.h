#ifndef __PPC405_MMU_H__
#define __PPC405_MMU_H__

// $Id: ppc405_mmu.h,v 1.1.1.1 2013/12/11 21:03:26 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ppc405/ppc405_mmu.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppc405_mmu.h
/// \brief Definitions related to the PPC405 MMU and its use in SSX.

#ifndef __ASSEMBLER__

#include "ssx_io.h"
#include <stdint.h>

/// The PPC405 TLBHI (tag) structure
///
/// Note that in hardware this is a 36-bit register, as it includes the TID
/// field. When writing, TID is set from the current PID, and when reading PID
/// is set from the TID entry of the register.

typedef union {
    uint32_t value;
    struct {
        uint32_t epn  : 22;
        uint32_t size : 3;
        uint32_t v    : 1;
        uint32_t e    : 1;
        uint32_t u0   : 1;
    } fields;
} Ppc405Tlbhi;


/// The PPC405 TLBLO (Data) structure

typedef union {
    uint32_t value;
    struct {
        uint32_t rpn  : 22;
        uint32_t ex   : 1;
        uint32_t wr   : 1;
        uint32_t zsel : 4;
        uint32_t w    : 1;
        uint32_t i    : 1;
        uint32_t m    : 1;
        uint32_t g    : 1;
    } fields;
} Ppc405Tlblo;

#endif /* __ASSEMBLER__ */

// TLBHI contains little-endian and U0 flags (probably never used)

#define TLBHI_E  0x00000020
#define TLBHI_U0 0x00000010

#define TLBHI_LEGAL_FLAGS (TLBHI_E | TLBHI_U0)

// TLBLO contains WIMG + EX/WR bits

#define TLBLO_EX 0x00000200
#define TLBLO_WR 0x00000100
#define TLBLO_W  0x00000008
#define TLBLO_I  0x00000004
#define TLBLO_M  0x00000002
#define TLBLO_G  0x00000001

#define TLBLO_LEGAL_FLAGS \
    (TLBLO_EX | TLBLO_WR | TLBLO_W | TLBLO_I | TLBLO_M | TLBLO_G)


#define PPC405_TLB_ENTRIES 64

#define PPC405_PAGE_SIZE_MIN 1024
#define PPC405_PAGE_SIZE_MAX (16 * 1024 * 1024)

#define PPC405_LOG_PAGE_SIZE_MIN 10
#define PPC405_LOG_PAGE_SIZE_MAX 24

#define PPC405_PAGE_SIZE_1K   0
#define PPC405_PAGE_SIZE_4K   1
#define PPC405_PAGE_SIZE_16K  2
#define PPC405_PAGE_SIZE_64K  3
#define PPC405_PAGE_SIZE_256K 4
#define PPC405_PAGE_SIZE_1M   5
#define PPC405_PAGE_SIZE_4M   6
#define PPC405_PAGE_SIZE_16M  7

// PPC405 MMU error and panic codes

#define PPC405_MMU_ILLEGAL_CONTEXT   0x00668001
#define PPC405_MMU_INVALID_ARGUMENT  0x00668002
#define PPC405_TOO_MANY_TLB_ENTRIES  0x00668003
#define PPC405_DUPLICATE_TLB_ENTRY   0x00668004


#ifndef __ASSEMBLER__

/// A descriptor of a memory region statically defined in the TLB
///
/// These maps are returned by ppc405_mmu_map(), and can be used later
/// to unmap the region with ppc405_mmu_unmap().  They can also be used to
/// control what gets printed by ppc405_mmu_report().
typedef uint64_t Ppc405MmuMap;

/// TLBIA
#define tlbia() asm volatile ("tlbia" : : : "memory")

/// TLBWEHI
#define tlbwehi(entry, tlbhi) \
asm volatile ("tlbwehi %0, %1" : : "r" (tlbhi), "r" (entry) : "memory")

/// TLBWELO
#define tlbwelo(entry, tlblo) \
asm volatile ("tlbwelo %0, %1" : : "r" (tlblo), "r" (entry) : "memory")

/// TLBREHI
#define tlbrehi(entry) \
    ({                 \
        uint32_t __tlbhi;                                               \
        asm volatile ("tlbrehi %0, %1" : "=r" (__tlbhi) : "r" (entry)); \
        __tlbhi;})

/// TLBRELO
#define tlbrelo(entry) \
    ({                 \
        uint32_t __tlblo;                                               \
        asm volatile ("tlbrelo %0, %1" : "=r" (__tlblo) : "r" (entry)); \
        __tlblo;})

/// TLBSX
///
/// Returns 1 if the address is mapped, else 0. If positive the integer
/// pointed to by \a entry is updated with the TLB index of the matching
/// entry, otherwise the return value is undefined.
#define tlbsx(address, entry)                                           \
    ({                                                                  \
        uint32_t __cr, __entry;                                         \
        asm volatile ("tlbsx. %0, 0, %2; mfcr %1" :                     \
                      "=r" (__entry), "=r" (__cr) : "r" (address));     \
        *(entry) = __entry;                                             \
        ((__cr & CR_EQ(0)) != 0);})


int
ppc405_mmu_reset(void);

int
ppc405_mmu_map(SsxAddress effective_address, 
               SsxAddress real_address,
               size_t size, int tlbhi_flags, int tlblo_flags,
               Ppc405MmuMap *map);

int
ppc405_mmu_unmap(Ppc405MmuMap *map);

void
ppc405_mmu_start(void);
                         
void
ppc405_mmu_report(FILE* stream, Ppc405MmuMap* map);

#endif /* __ASSEMBLER__ */


#endif /* __PPC405_MMU_H__ */
