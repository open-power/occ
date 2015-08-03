/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/strdup.c $                                  */
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
