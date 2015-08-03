/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/common/memcpy.c $                                     */
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
// $Id: memcpy.c,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/memcpy.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file memcpy.c
/// \brief The memcpy() function

#include "kernel.h"

/// The memcpy() function copies \a n bytes from memory area \a src to memory
/// area \a dest.  The memory areas should not overlap.  Use memmove(3) if the
/// memory areas do overlap. The memcpy() function returns a pointer to dest.

// This implementation should work well for both 32-bit and 64-bit machines,
// assuming they can handle unaligned accesses. The implementation assumes that
// it is better to avoid the loop setup overhead by a test and branch for
// cases where loops can be bypassed.

//void *
//memcpy(void *dest, const void *src, size_t n)
//{
//    while(n--) {
//      *dest++ = *src++;
//    }
//
//    return s;
//}

void *
memcpy(void *dest, const void *src, size_t n)
{
    uint8_t *d8, *s8;
    uint64_t *d64, *s64;
    size_t doublewords, octawords;

    // First copy memory 32 bytes at a time.

    d64 = (uint64_t *)dest;
    s64 = (uint64_t *)src;
    octawords = n / 32;
    if (octawords) {
        n -= octawords * 32;
        while(octawords--) {
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
            *d64++ = *s64++;
        }
    }

    // Now set memory 8 bytes at a time. This might actually be better done
    // explicitly rather than as a loop because the maximum loop count is 3
    // here. 

    doublewords = n / 8;
    if (doublewords) {
        n -= doublewords * 8;
        while (doublewords--) {
            *d64++ = *s64++;
        }
    }

    // Finally finish any remaining memory bytewise

    if (n) {
        d8 = (uint8_t *)d64;
        s8 = (uint8_t *)s64;
        while (n--) {
            *d8++ = *s8++;
        }
    }

    return dest;
}
