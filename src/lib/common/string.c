/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/common/string.c $                                     */
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
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file string.c
/// \brief strlen(), strcmp() etc. functions

#include "string.h"

/// Compute the length of a string
///
/// The strlen() function calculates the length of the string \a s, not
/// including the terminating \b '\0' character.  The strlen() function
/// returns the number of characters in \a s.

size_t
strlen(const char *s)
{
    const char *p = s;

    while (*p) {
        p++;
    }

    return p - s;
}


/// Compare two strings
///
/// The strcmp() function compares the two strings \a s1 and \a s2.  It
/// returns an integer less than, equal to, or greater than zero if \a s1 is
/// found, respectively, to be less than, to match, or be greater than \a s2.

int 
strcmp(const char* s1, const char* s2)
{
    int rc;

    if (s1 == s2) {
        rc = 0;
    } else {
        while(*s1 && (*s1 == *s2)) {
            s1++; 
            s2++;
        }
        rc = *((unsigned char *)s1) - *((unsigned char *)s2);
    }
    return rc;
}


/// Compare a portion of two strings
///
/// The strncmp() function compares at most the first \n characters of the two
/// strings \a s1 and \a s2.  It returns an integer less than, equal to, or
/// greater than zero if (the prefix of) \a s1 is found, respectively, to be
/// less than, to match, or be greater than (the prefix of) \a s2.

int 
strncmp(const char* s1, const char* s2, size_t n)
{
    int rc;

    if ((s1 == s2) || (n == 0)) {
        rc = 0;
    } else {
        while(*s1 && (*s1 == *s2) && n--) {
            s1++; 
            s2++;
        }
        rc = *((unsigned char *)s1) - *((unsigned char *)s2);
    }
    return rc;
}


/// Copy a string
///
/// The strcpy() function copies the string pointed to by \a src (including
/// the terminating null character) to the array pointed to by \a dest.  The
/// strings may not overlap, and the destination string \a dest must be large
/// enough to receive the copy.
///
/// The strcpy() function return a pointer to the destination string \a dest.

char *
strcpy(char *dest, const char *src)
{
    char *rv = dest;

    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';

    return rv;
}


/// Safely copy all or part of a string
///
/// The strncpy() function copies the string pointed to by \a src (including
/// the terminating null character) to the array pointed to by \a dest, except
/// that no more than \a n bytes of \a src are copied. This, if there is no
/// null byte among the first \a n bytes of \a src, the result will not be
/// null-terminated. In the case where the length of \a src is less than \a n,
/// the remainder of \a dest will be padded with null bytes.  The strings may
/// not overlap.
///
/// The strncpy() function return a pointer to the destination string \a dest.

char *
strncpy(char *dest, const char *src, size_t n)
{
    char *rv = dest;

    while (*src && n--) {
        *dest++ = *src++;
    }
    memset(dest, 0, n);

    return rv;
}


/// Compare two memory areas
///
/// The memcmp() function compares the first \a n bytes of the memory areas \a
/// s1 and \a s2.  It returns an integer less than, equal to, or greater than
/// zero if \a s1 is found, respectively, to be less than, to match, or be
/// greater than \a s2.
int
memcmp(const void* s1, const void* s2, size_t n) 
{
    unsigned char *p1, *p2;
    int rc;

    p1 = (unsigned char*) s1;
    p2 = (unsigned char*) s2;

    if (s1 == s2) {
        
        rc = 0;

    } else {

        while (n && (*p1 == *p2)) {
            n--;
            p1++; 
            p2++;
        }

        if (n == 0) {
            rc = 0;
        } else {
            rc = (*p1 - *p2);
        }
    }

    return rc;
}

