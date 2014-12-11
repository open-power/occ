#ifndef __PPC32_GCC_H__
#define __PPC32_GCC_H__

// $Id: ppc32_gcc.h,v 1.1.1.1 2013/12/11 21:03:25 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ppc32/ppc32_gcc.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppc32_gcc.h
/// \brief 32-bit PowerPC functions expected by GCC

#ifndef __ASSEMBLER__

#include <stdint.h>

/// A 64-bit unsigned integer type

typedef union {
    uint64_t value;
    uint32_t word[2];
} Uint64;

/// A 64-bit signed integer type

typedef union {
    int64_t value;
    int32_t word[2];
} Int64;

uint64_t
__lshrdi3(uint64_t x, int i);

uint64_t
__ashldi3(uint64_t x, int i);

uint64_t
__ashrdi3(uint64_t x, int i);

int
__popcountsi2(uint32_t x);

int
__popcountdi2(uint64_t x);

/// Unsigned 64/64 bit divide, returning quotient and remainder via pointers.

void
__ppc32_udiv64(uint64_t u, uint64_t v, uint64_t *q, uint64_t *r);

/// Signed 64/64 bit divide, returning quotient and remainder via pointers.

void
__ppc32_sdiv64(int64_t u, int64_t v, int64_t *q, int64_t *r);

uint64_t 
__udivdi3(uint64_t u, uint64_t v);

int64_t
__divdi3(int64_t u, int64_t v);

int64_t
__moddi3(int64_t u, int64_t v);

uint64_t 
__umoddi3(uint64_t u, uint64_t v);

int
__ucmpdi2(uint64_t a, uint64_t b);

#endif /* __ASSEMBLER__ */

#endif /* __PPC32_GCC_H__ */
