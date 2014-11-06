// $Id: strdup.c,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/strdup.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file strdup.c
/// \brief Functions from <string.h> that require malloc()
///
/// These APIs are split from string.c for the benefit of applications like
/// OCC FW that don't use malloc().

#include <stdlib.h>
#include <string.h>

/// Duplicate a string
///
/// \param s The string to duplicate
///
/// The strdup() function returns a pointer to a new string which is a
/// duplicate of the input string \a s.  Memory for the new string is obtained
/// with malloc(), and can be freed with free().
///
/// \returns The strdup() function returns a pointer to the duplicated string,
/// or NULL (0) if insufficient memory was available.

char *
strdup(const char* s)
{
    char* dup;

    dup = (char*)malloc(strlen(s) + 1);
    if (dup != 0) {
        strcpy(dup, s);
    }
    return dup;
}
