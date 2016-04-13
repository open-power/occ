/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/fgetc.c $                                             */
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


    

