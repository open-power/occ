/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/strcasecmp.c $                                        */
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
/// \file strcasecmp.c
/// \brief Implementation of strcasecmp() and strncasecmp()
///
/// These routines are rarely used, hence broken out into a separate file to
/// save code space for most applications.

#include "ssx.h"
#include "string.h"

/// Compare two strings ignoring case
///
/// The strcasecmp() function compares the two strings \a s1 and \a s2,
/// ignoring the case of the characters.  It returns an integer less than,
/// equal to, or greater than zero if \a s1 is found, respectively, to be less
/// than, to match, or be greater than \a s2.

int 
strcasecmp(const char* s1, const char* s2)
{
    int rc;

    if (s1 == s2) {
        rc = 0;
    } else {
        while(*s1 && (tolower(*s1) == tolower(*s2))) {
            s1++; 
            s2++;
        }
        rc = *((unsigned char *)s1) - *((unsigned char *)s2);
    }
    return rc;
}


/// Compare a portion of two strings ignoring case
///
/// The strncmp() function compares at most the first \n characters of the two
/// strings \a s1 and \a s2, ignoring the case of the characters.  It returns
/// an integer less than, equal to, or greater than zero if (the prefix of) \a
/// s1 is found, respectively, to be less than, to match, or be greater than
/// (the prefix of) \a s2.

int 
strncasecmp(const char* s1, const char* s2, size_t n)
{
    int rc;

    if ((s1 == s2) || (n == 0)) {
        rc = 0;
    } else {
        while(*s1 && (tolower(*s1) == tolower(*s2)) && n--) {
            s1++; 
            s2++;
        }
        rc = *((unsigned char *)s1) - *((unsigned char *)s2);
    }
    return rc;
}
