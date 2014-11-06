// $Id: sprintf.c,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/sprintf.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

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
