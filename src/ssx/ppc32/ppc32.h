#ifndef __PPC32_H__
#define __PPC32_H__

// $Id: ppc32.h,v 1.2 2014/03/14 15:11:46 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ppc32/ppc32.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppc32.h
/// \brief Generic 32-bit PowerPC header for SSX
///
/// The synchronization macros defined here all create a compiler
/// memory barrier that will cause GCC to flush/invalidate all memory data
/// held in registers before the macro. This is consistent with other systems,
/// e.g., the PowerPC Linux kernel, and is the safest way to define these
/// macros.


#include "ppc32_asm.h"
#include "ppc32_gcc.h"

// Condition register fields

#define CR_LT(n) (0x80000000u >> (4 * (n)))
#define CR_GT(n) (0x40000000u >> (4 * (n)))
#define CR_EQ(n) (0x20000000u >> (4 * (n)))
#define CR_SO(n) (0x10000000u >> (4 * (n)))


#ifndef __ASSEMBLER__

#include "stdint.h"

/// ssize_t is defined explictly rather than bringing in all of <unistd.h>
#ifndef __ssize_t_defined
#define __ssize_t_defined
typedef int ssize_t;
#endif

/// A memory barrier
#define barrier() asm volatile ("" : : : "memory")

/// Ensure In-order Execution of Input/Output
#define eieio() asm volatile ("eieio" : : : "memory")

/// Memory barrier
#define sync() asm volatile ("sync" : : : "memory")

/// Instruction barrier
#define isync() asm volatile ("isync" : : : "memory")

/// CouNT Leading Zeros Word
#define cntlzw(x) \
({uint32_t __x = (x); \
  uint32_t __lzw; \
  asm volatile ("cntlzw %0, %1" : "=r" (__lzw) : "r" (__x)); \
  __lzw;})

/// CouNT Leading Zeros : uint32_t
static inline int
cntlz32(uint32_t x) {
    return cntlzw(x);
}

/// CouNT Leading Zeros : uint64_t
static inline int
cntlz64(uint64_t x) {
    if (x > 0xffffffff) {
        return cntlz32(x >> 32);
    } else {
        return 32 + cntlz32(x);
    }
}


/// 32-bit population count
static inline int
popcount32(uint32_t x)
{
    return __builtin_popcount(x);
}
    

/// 64-bit population count
static inline int
popcount64(uint64_t x)
{
    return __builtin_popcountll(x);
}


// NB: Normally we wouldn't like to force coercion inside a macro because it
// can mask programming errors, but for the MMIO macros the addresses are
// typically manifest constants or 32-bit unsigned integer expressions so we
// embed the coercion to avoid warnings.

/// 8-bit MMIO Write
#define out8(addr, data) \
do {*(volatile uint8_t *)(addr) = (data); eieio();} while(0)

/// 8-bit MMIO Read
#define in8(addr) \
({uint8_t __data = *(volatile uint8_t *)(addr); eieio(); __data;})

/// 16-bit MMIO Write
#define out16(addr, data) \
do {*(volatile uint16_t *)(addr) = (data); eieio();} while(0)

/// 16-bit MMIO Read
#define in16(addr) \
({uint16_t __data = *(volatile uint16_t *)(addr); eieio(); __data;})

/// 32-bit MMIO Write
#define out32(addr, data) \
do {*(volatile uint32_t *)(addr) = (data); eieio();} while(0)

/// 32-bit MMIO Read
#define in32(addr) \
({uint32_t __data = *(volatile uint32_t *)(addr); eieio(); __data;})

/// 64-bit MMIO Write
#define out64(addr, data) \
    do { \
        uint64_t __data = (data);                               \
        volatile uint32_t *__addr_hi = (uint32_t *)(addr);      \
        volatile uint32_t *__addr_lo = __addr_hi + 1;           \
        *__addr_hi = (__data >> 32);                            \
        eieio();                                                \
        *__addr_lo = (__data & 0xffffffff);                     \
        eieio();                                                \
    } while(0)

/// 64-bit MMIO Read
#define in64(addr) \
    ({                                                     \
        uint64_t __data;                                   \
        volatile uint32_t *__addr_hi = (uint32_t *)(addr); \
        volatile uint32_t *__addr_lo = __addr_hi + 1;      \
        __data = *__addr_hi;                               \
        eieio();                                           \
        __data = (__data << 32) | *__addr_lo;              \
        eieio();                                           \
        __data;})

#endif  /* __ASSEMBLER__ */


#ifndef __ASSEMBLER__

/// Store revision information as a (global) string constant
#define REVISION_STRING(symbol, rev) const char* symbol = rev;

#else // __ASSEMBLER__

/// Store revision information as a global string constant
        .macro  .revision_string, symbol:req, rev:req
        .pushsection .rodata
        .balign 4
        .global \symbol
\symbol\():
        .asciz  "\rev"
        .balign 4
        .popsection
        .endm

#endif  // __ASSEMBLER__

#endif /* __PPC32_H__ */
