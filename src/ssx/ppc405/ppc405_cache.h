#ifndef __PPC405_CACHE_H__
#define __PPC405_CACHE_H__

// $Id: ppc405_cache.h,v 1.1.1.1 2013/12/11 21:03:27 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ppc405/ppc405_cache.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ppc405_cache.h
/// \brief PowerPC 405 cache management header for SSX
///
/// The data cache flush/invalidate macros defined here create a compiler
/// memory barrier that will cause GCC to flush/invalidate all memory data
/// held in registers before the macro. 

#ifndef __ASSEMBLER__

/// Determine cache-alignment of a pointer or byte-count
#define cache_aligned(x) \
    ((((unsigned long)(x)) & (POW2_32(LOG_CACHE_LINE_SIZE) - 1)) == 0)

/// Cache-align a pointer or byte count. If the 'direction' is <= 0 then we
/// round down, else round up.
#define cache_align(x, direction)                                       \
    ({                                                                  \
        unsigned long __x = (unsigned long)(x);                         \
        unsigned long __r;                                              \
        if ((direction) <= 0) {                                         \
            __r = __x & ~(((unsigned long)CACHE_LINE_SIZE) - 1);        \
        } else {                                                        \
            if (__x % CACHE_LINE_SIZE) {                                \
                __r = __x + (CACHE_LINE_SIZE - (__x % CACHE_LINE_SIZE)); \
            }                                                           \
        }                                                               \
        (void *)__r;                                                    \
    })

/// Data Cache Block Flush
#define dcbf(p) asm volatile ("dcbf 0, %0" : : "r" (p) : "memory")

/// Data Cache Block Touch
#define dcbt(p) asm volatile ("dcbt 0, %0" : : "r" (p) : "memory")

/// Data Cache Block Invalidate (Privileged)
#define dcbi(p) asm volatile ("dcbi 0, %0" : : "r" (p) : "memory")

/// Instruction Cache Block Invalidate
#define icbi(p) asm volatile ("icbi 0, %0" : : "r" (p) : "memory")

/// Instruction Cache Block Touch
#define icbt(p) asm volatile ("icbt 0, %0" : : "r" (p) : "memory")

void 
icache_invalidate_all(void);

void
dcache_invalidate_all(void);

void
dcache_flush_all(void);

void
dcache_invalidate(void *p, size_t bytes);

void
dcache_flush(void *p, size_t bytes);

/// Invalidate a line in the D-cache
///
/// \param p An address withing the cache line to be invalidated.
///
/// The dcache_invalidate_line() API is used to invalidate a single cache line
/// containing the address \a p.  Note that invalidation is a destructive
/// operation that may cause the loss of information.  It is the caller's
/// responsibility to insure that no useful data is inadverdently invalidated.
/// D-cache invalidation is more-or-less a no-op for data either not in the
/// cache or marked as non-cacheable.
///
/// This API always issues a sync() after the invalidation.

static inline void
dcache_invalidate_line(void *p)
{
    dcbi(p);
    sync();
}

/// Flush and invalidate a line from the D-cache
///
/// \param p An address within the cache line to be flushed.
///
/// The dcache_flush_line() API can be used as a shortcut to flush and
/// invalidate a single cache line. Note that flushing is not a destructive
/// operation in the sense that no information is lost, however the caller
/// must make sure that the entirity of the data to be flushed is contained in
/// the line that includes the address \a p. D-cache flush is more-or-less a
/// no-op for data either not in the cache or marked as non-cacheable.
///
/// This API always issues a sync() after the flush.

static inline void
dcache_flush_line(void *p)
{
    dcbf(p);
    sync();
}

#endif  /* __ASSEMBLER__ */

#endif  /* __PPC405_CAHE_H__ */
