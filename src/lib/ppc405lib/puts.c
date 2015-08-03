/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/puts.c $                                    */
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
// $Id: puts.c,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/puts.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file puts.c
/// \brief Implementation of puts(), fputs(), putchar(), fputc()
///
/// The implementations of these APIs are split out to save code space for
/// applications that do not require them.

#include "ssx_io.h"


/// Put a character to a stream
///
/// The fputc() function writes the byte specified by \a c (converted to an
/// unsigned char) to the stream pointed to by \a stream. On success, fputc()
/// returns the value written, i.e., the \c unsigned \c char form of \a c.
///
/// The POSIX standard fputc() returns EOF on error; this version returns the
/// negative error code of the underlying I/O error, which will also be set in
/// the \a error field of the \a stream. Note that SSX may also be configured
/// to panic in the event of an I/O error.

int
fputc(int c, FILE *stream)
{
    unsigned char uc = (unsigned char)c;
    int rc = swrite(stream, (void *)(&uc), 1, 0);
    if (rc < 0) {
        return rc;
    } else {
        return uc;
    }
}

    
/// Put a string to a stream
///
/// The fputs() function writes the null-terminated string \a s to the stream
/// pointed to by \a stream.  The terminating null byte is not written. On
/// success, fputs() returns 0.
///
/// The POSIX standard fputc() returns EOF on error; this version returns the
/// negative error code of the underlying I/O error, which will also be set in
/// the \a error field of the \a stream. Note that SSX may also be configured
/// to panic in the event of an I/O error.

int
fputs(const char *s, FILE *stream)
{
    return swrite(stream, s, strlen(s), 0);
}
    
/// Put a string to \c stdout
///
/// The puts() function writes the null-terminated string \a s followed by a
/// newline to \a stdout.  The terminating null byte is not written. On
/// success, puts() returns 0.
///
/// The POSIX standard fputc() returns EOF on error; this version returns the
/// negative error code of the underlying I/O error, which will also be set in
/// the \a error field of the \a stream. Note that SSX may also be configured
/// to panic in the event of an I/O error.

int
puts(const char *s)
{
    int rc = fputs(s, stdout);
    if (rc < 0) {
        return rc;
    } else {
        rc = fputc('\n', stdout);
        if (rc < 0) {
            return rc;
        } else {
            return 0;
        }
    }
}


/// Put a character to \a stdout.
///
/// \c putchar(c) is equivalent to <c> fputc(c, stdout) </c>.

int
putchar(int c)
{
    return fputc(c, stdout);
}
