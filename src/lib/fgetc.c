// $Id: fgetc.c,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/fgetc.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file fgetc.c
/// \brief Implementation of fgetc() and ungetc()
///
/// The implementations of these APIs are split out to save code space for
/// applications that do not require them.

#include "ssx_io.h"


/// Read a character from a stream
///
/// fgetc() reads the next character from \a stream and returns it as an
/// unsigned char cast to an int, or EOF on end of file or error.

int
fgetc(FILE* stream)
{
    unsigned char c;
    size_t read;
    int rc;

    if (stream->flags & SSX_FILE_HAS_CHARACTER) {
        stream->flags &= ~SSX_FILE_HAS_CHARACTER;
        rc = stream->character;
    } else {
        rc = sread(stream, &c, 1, &read);
        if (rc || (read != 1)) {
            rc = EOF;
        } else {
            rc = c;
            if (c == '\n') {
                stream->lines++;
            }
        }
    }
    return rc;
}


/// Push a character back onto a stream
///
/// ungetc() pushes \a c back to \a stream, cast to unsigned char, where it is
/// available for subsequent fgetc() operations.  Only one pushback is
/// implemented. A call of ungetc() on a stream that already has a character
/// pushed back will drop the new push-back and return EOF.  Otherwise
/// ungetc() returns \a c.

int
ungetc(int c, FILE* stream)
{
    int rc;

    if (stream->flags & SSX_FILE_HAS_CHARACTER) {
        rc = EOF;
    } else {
        stream->flags |= SSX_FILE_HAS_CHARACTER;
        stream->character = c;
        rc = c;
    }
    return rc;
}

/// Return the number of newline characters read from a stream
///
/// This API is an SSX entension to the \<stdio\> APIs.  It returns the number
/// of newline characters read from the stream using fgetc(). Newline
/// characters read via direct calls to sread() in the stream are not counted.
///
/// An application that sees an error while reading from a stream can print
/// flines() or flines() + 1 (depending on the application) to help users
/// track down errors in their input.
size_t
flines(FILE* stream)
{
    return stream->lines;
}


    

