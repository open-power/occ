// $Id: strcasecmp.c,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/strcasecmp.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

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
