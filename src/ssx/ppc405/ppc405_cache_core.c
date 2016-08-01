/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/ppc405/ppc405_cache_core.c $                          */
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

/// \file ppc405_cache_core.c
/// \brief Core cache management routines required of any PPC405 configuration
/// of SSX that interacts with DMA devices using cacheable memory.
///
///  The entry points in this file are considered 'core' routines that will
///  always be present at runtime in any SSX application.
///
///  \todo The compiler generates wierd assembly language for these cache
///  management APIs - probably due to the "volatile" asm - it may be best to
///  recode them directly in assembler.

#include "ssx.h"

/// Invalidate a range of addresses from the D-cache
///
/// \param p A pointer to the memory area to be invalidated.
///
/// \param bytes The size (in bytes) of the area to invalidate.
///
/// The dcache_invalidate() API is used to invalidate an arbitrary range of
/// memory in the cache. Note that invalidation is a destructive operation
/// that may cause the loss of information.  This API will invalidate all
/// cache lines from the line containing the address \a p, to the line
/// containing the address \a p + \a size - 1.  (If \a size == 0 this call is
/// a NOP.) It is the caller's responsibility to insure that no useful data is
/// inadverdently invalidated.  D-cache invalidation is more-or-less a no-op
/// for data either not in the cache or marked as non-cacheable.
///
/// This API always issues a sync() after the invalidation, even in the event
/// of \a size == 0.
///
/// \note For invalidating small blocks of data where some alignmment
/// constraints are known it may be more efficient to use
/// dcache_invalidate_line() rather than this API.

void
dcache_invalidate(void* p, size_t bytes)
{
    size_t lines;

    if (bytes != 0)
    {
        lines = 1;
        bytes -=
            MIN((CACHE_LINE_SIZE - ((unsigned long)p % CACHE_LINE_SIZE)),
                bytes);
        lines += bytes / CACHE_LINE_SIZE;

        if (!cache_aligned(bytes))
        {
            lines++;
        }

        while (lines--)
        {
            dcbi(p);
            p += CACHE_LINE_SIZE;
        }
    }

    sync();
}


/// Flush and invalidate a range of addresses from the D-cache
///
/// \param p A pointer to a memory area to be invalidated.
///
/// \param bytes The size (in bytes) of the area to invalidate.
///
/// The dcache_flush() API is used to flush and invalidate an arbitrary range
/// of memory from the D-cache. Note that flushing is not a destructive
/// operation in the sense that no information is lost.  This API will flush
/// and invalidate all cache lines from the line containing the address \a p,
/// to the line containing the address \a p + \a size - 1.  (If \a size == 0
/// this call is a NOP.)  D-cache flush is more-or-less a no-op for data
/// either not in the cache or marked as non-cacheable.
///
/// This API always issues a sync() after the flush, even in the event of \a
/// size == 0.
///
/// \note For flushing small blocks of data where some alignmment constraints
/// are known it may be more efficient to use dcache_flush_line() rather than
/// this API.


void
dcache_flush(void* p, size_t bytes)
{
    size_t lines;

    if (bytes != 0)
    {
        lines = 1;
        bytes -=
            MIN((CACHE_LINE_SIZE - ((unsigned long)p % CACHE_LINE_SIZE)),
                bytes);
        lines += bytes / CACHE_LINE_SIZE;

        if (!cache_aligned(bytes))
        {
            lines++;
        }

        while (lines--)
        {
            dcbf(p);
            p += CACHE_LINE_SIZE;
        }
    }

    sync();
}






