/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/baselib/ppe42_cache.h $                               */
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
#ifndef __PPE42_CACHE_H__
#define __PPE42_CACHE_H__

/// \file ppe42_cache.h
/// \brief PowerPC-lite (PPE) cache management header for PK
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

void
dcache_invalidate_all(void);

void
dcache_flush_all(void);

void
dcache_invalidate(void* p, size_t bytes);

void
dcache_flush(void* p, size_t bytes);

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
dcache_invalidate_line(void* p)
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
dcache_flush_line(void* p)
{
    dcbf(p);
    sync();
}

#endif  /* __ASSEMBLER__ */

#endif  /* __PPE42_CAHE_H__ */
