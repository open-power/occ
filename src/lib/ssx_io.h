/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ssx_io.h $                                            */
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
#ifndef __SSX_IO_H__
#define __SSX_IO_H__

/// \file ssx_io.h
/// \brief SSX analog-replacement for C <stdio.h> and <unistd.h> functions

#if !defined(_STDIO_H) && !defined(_UNISTD_H)

#include "ssx_macros.h"
#include "ssx_api.h"

#include "ctype.h"
#include "errno.h"
#include "string.h"

#define EOF -1

#define FILE_INVALID_OBJECT   0x00345301
#define FILE_INVALID_ARGUMENT 0x00345302

#ifndef __ASSEMBLER__

#include <stdarg.h>

/// The SSX implementation of the FILE structure

typedef struct FILE {

    /// The sread() method
    int (*sread)(struct FILE *stream, void *buf, 
                 size_t count, size_t *read);

    /// The swrite() method
    int (*swrite)(struct FILE *stream, const void *buf,
                  size_t count, size_t *written);

    /// The fflush() method
    int (*fflush)(struct FILE *stream);

    /// The last error code encountered
    ///
    /// This field is not set in the event of an error panic.
    int error;

    /// SSX file/stream flags; See \ref ssx_file_flags
    int flags;

    /// The semaphore used to lock low-level file operations if required.
    SsxSemaphore fop_sem;

    /// The number of newline characters read by fgetc.
    ///
    /// This variable supports an SSX extension to <stdio.h>, the flines()
    /// API. flines() returns the number of newline characters read by
    /// fgetc(). This counter does not count newline characters read using
    /// sread() on the stream directly.
    size_t lines;

    /// The character pushed back by ungetc()
    ///
    /// If a character is back the flag SSX_FILE_HAS_CHARACTER will be
    /// set. Note that characters pushed back with ungetc() are not returned
    /// by any subsequent sread() call on the stream, so it is best not to mix
    /// sread() and fgetc().
    unsigned char character;

} FILE;


/// \defgroup ssx_file_flags SSX File/Stream Flags
/// @{

/// Low-level access is locked by an SSX_CRITICAL critical section
#define SSX_FILE_OP_LOCK_CRITICAL 0x1

/// Low-level access is locked by an SSX_NONCRITICAL critical section
#define SSX_FILE_OP_LOCK_NONCRITICAL 0x2

/// Low-level access is locked by a semaphore
#define SSX_FILE_OP_LOCK_SEMAPHORE 0x4

/// Mask of all locking options; 
#define SSX_FILE_OP_LOCK_OPTIONS 0x7

/// The FILE has a character pushed back by ungetc()
#define SSX_FILE_HAS_CHARACTER 0xf

/// All valid flags
#define SSX_FILE_VALID_FLAGS 0xf

/// @}


extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

/// SSX kernel output stream
///
/// This stream implements the printk() API.  It must be a non-blocking stream
/// so that it can be used in interrupt contexts.

extern FILE *ssxout;

extern FILE *ssxnull;

int
FILE_create(FILE *stream, int flags);

int
sread(FILE *stream, void *buf, size_t count, size_t *read);

int
swrite(FILE *stream, const void *buf, size_t count, size_t *written);

int
fflush(FILE *stream);

int
vfprintf(FILE *stream, const char *format, va_list argp)
    __attribute__ ((format (printf, 2, 0)));

int
vprintf(const char *format, va_list argp)
    __attribute__ ((format (printf, 1, 0)));

int
fprintf(FILE *stream, const char *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));

int
printf(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

int
printk(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

int
vsprintf(char *str, const char *format, va_list argp)
    __attribute__ ((format (printf, 2, 0)));

int
sprintf(char *str, const char *format, ...)
    __attribute__ ((format (printf, 2, 3)));

int
vsnprintf(char *str, size_t size, const char *format, va_list argp)
    __attribute__ ((format (printf, 3, 0)));

int
snprintf(char *str, size_t size, const char *format, ...)
    __attribute__ ((format (printf, 3, 4)));

int
fputc(int c, FILE *stream);

int
fputs(const char *s, FILE *stream);

int
puts(const char *s);

int
putchar(int c);

int
fgetc(FILE* stream);

int
ungetc(int c, FILE* stream);

size_t
flines(FILE* stream);

int 
ssx_io_error_set(FILE *stream, int code);

/// Handle I/O errors including panic configurations
#define SSX_IO_ERROR_IF(stream, condition, code) \
    do { \
        if (condition) {                                                \
            if (ssx_io_error_set(stream, code) && SSX_ERROR_PANIC) {    \
                SSX_PANIC(code);                                        \
            } else {                                                    \
                return -(code);                                         \
            }                                                           \
        }                                                               \
    } while (0)

#define SSX_IO_ERROR(stream, code) SSX_IO_ERROR_IF((stream), 1, (code))


#endif  /* __ASSEMBLER__ */

#endif /* !defined(_STDIO_H) && !defined(_UNISTD_H) */

#endif  /* __SSX_IO_H__ */
