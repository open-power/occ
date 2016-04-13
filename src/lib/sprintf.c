/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/sprintf.c $                                           */
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
/// \file sprintf.c \brief Implementations of sprintf() and snprintf() using
/// StringStream. 

#include "ssx.h"
#include "ssx_io.h"
#include "string_stream.h"

    
// sprintf() is easy - just blindly copy in the data.

static int
sprintf_swrite(FILE *stream, const void *buf, size_t size, size_t *written)
{
    StringStream *string = (StringStream *)stream;

    memcpy((void *)(&(string->data[string->next_write])), buf, size);
    string->next_write += size;
    *written = size;
    return 0;
}


// snprintf() requires that bytes that won't fit in the array are simply
// discarded, but still accounted for. Note that vfprintf() is doing the
// high-level accounting.

static int 
snprintf_swrite(FILE *stream, const void *buf, size_t size, size_t *written)
{
    StringStream *string = (StringStream *)stream;

    size_t to_write = MIN(size, string->size - string->next_write - 1);

    memcpy((void *)(&(string->data[string->next_write])), buf, to_write);
    string->next_write += to_write;
    *written = size;
    return 0;
}


// We use a StringStream to implement [v]sprintf() and [v]snprintf().  Once
// the formatting is finished the NULL terminator is added. 

int
vsprintf(char *str, const char *format, va_list argp)
{
    ssize_t rc;
    StringStream stream;

    _string_stream_create(&stream, str, 1, 0, sprintf_swrite);
    rc = vfprintf((FILE *)(&stream), format, argp);
    stream.data[stream.next_write] = 0;
    return rc;
}
    


int
sprintf(char *str, const char *format, ...)
{
    va_list argp;
    ssize_t rc;

    va_start(argp, format);
    rc = vsprintf(str, format, argp);
    va_end(argp);

    return rc;
}


int
vsnprintf(char *str, size_t size, const char *format, va_list argp)
{
    ssize_t rc;
    StringStream stream;

    if (size == 0) {
        return 0;
    }

    _string_stream_create(&stream, str, size, 0, snprintf_swrite);
    rc = vfprintf((FILE *)(&stream), format, argp);
    stream.data[stream.next_write] = 0;
    return rc;
}


int
snprintf(char *str, size_t size, const char *format, ...)
{
    va_list argp;
    ssize_t rc;

    if (size == 0) {
        return 0;
    }

    va_start(argp, format);
    rc = vsnprintf(str, size, format, argp);
    va_end(argp);

    return rc;
}
