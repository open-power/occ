/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/memset.c $                                            */
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
/// \file memset.c
/// \brief The memset() function

#include "ssx.h"

/// The memset() function fills the first \a n bytes of the memory area
/// pointed to by \a s with the constant byte \a c.  The memset() function
/// returns a pointer to the memory area \a s.
///
/// Note that memset() is optimized for setting large memory areas, and
/// entails quite a bit of overhead to do this efficiently.  If a memory area
/// consists of a small number of basic data types (e.g., integers) it is
/// probably more time-efficient to set the memory directly with a for loop
/// (or unrolled loop).

// This implementation should work well for both 32-bit and 64-bit
// machines. The implementation assumes that it is worthwhile to align memory
// pointers and do as much as possible using aligned addresses. [This doesn't
// seem to matter on an X86 server processor, however]. It also assumes that
// it is better to avoid the loop setup overhead by a test and branch for
// cases where loops can be bypassed.

//void *
//memset(void *s, int c, size_t n)
//{
//    uint8_t byte = (uint8_t)c;
//    uint8_t *p = (uint8_t *)s;
//
//    while(n--) {
//      *p++ = byte;
//    }
//
//    return s;
//}

void *
memset(void *s, int c, size_t n)
{
    uint8_t byte, *p8;
    uint32_t word;
    uint64_t doubleword, *p64;
    size_t bytes, doublewords, octawords;

    // Any initial memory segment not aligned to an 8-byte boundary is set
    // bytewise.

    byte = (uint8_t)c;
    p8 = (uint8_t *)s;

    bytes = MIN(n, (unsigned long)s % 8);
    if (bytes) {
        n -= bytes;
        while (bytes--) {
            *p8++ = byte;
        }
    }

    // Short requests are finshed here as well. 

    if (n < 8) {
        while (n--) {
            *p8++ = byte;
        }
        return s;
    }

    // We have at least 8 bytes of memory aligned on an 8-byte boundary. A
    // doubleword initializer is created.

    word = (byte << 8) | byte;
    word = (word << 16) | word;
    doubleword = ((uint64_t)word << 32) | word;

    // First set memory 32 bytes at a time.

    p64 = (uint64_t *)p8;
    octawords = n / 32;
    if (octawords) {
        n -= octawords * 32;
        while(octawords--) {
            *p64++ = doubleword;
            *p64++ = doubleword;
            *p64++ = doubleword;
            *p64++ = doubleword;
        }
    }

    // Now set memory 8 bytes at a time. This might actually be better done
    // explicitly rather than as a loop because the maximum loop count is 3
    // here. 

    doublewords = n / 8;
    if (doublewords) {
        n -= doublewords * 8;
        while (doublewords--) {
            *p64++ = doubleword;
        }
    }

    // Finally finish any remaining memory bytewise

    p8 = (uint8_t *)p64;
    if (n) {
        while (n--) {
            *p8++ = byte;
        }
    }

    return s;
}
