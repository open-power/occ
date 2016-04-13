/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ssx_io.c $                                            */
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
/// \file ssx_io.c
/// \brief SSX analog-replacement for C \<stdio.h\> and \<unistd.h\> functions
///
/// The SSX library provides simple analogs for I/O functions found in the
/// typical C libraries and Posix standards. I/O is not considered part of the
/// SSX kernel per-se - I/O is implemented by the application creating I/O
/// abstractions and drivers using the simple SSX kernel services. [However,
/// the ssx_io library does provide a printk() function for use by kernel
/// drivers, so the lines are not completely clear.]
///
/// This library provides a FILE structure or 'base class' to represent a
/// character stream.  Abstractions for different devices will 'derive' a
/// subclass from this base class by including the FILE as the first member of
/// the subclass, and then likely adding more data members.  The FILE
/// structure contains pointers to the routines that implement I/O operations
/// for derived classes. There is no requiremengt that a stream type implement
/// every possible stream operation.  If an unsupported operation is invoked
/// on a stream then the call will simply return -ENXIO, unless otherwise
/// specified.
///
/// The generic I/O calls use the same error-handling protocol as used by the
/// SSX kernel services.  A configuration setting (\c SSX_ERROR_CHECK_API)
/// determines if error checks are made at all, and another setting (\c
/// SSX_ERROR_PANIC) determines whether the presence of a fatal error causes a
/// bad return code or an immediate panic.
///
/// The following errors are never considered fatal errors:
///
/// \b EOF Used to signal end-of-file
///
/// \b EAGAIN Used to signal that an operation was made to a non-blocking
/// stream and the call would block.
///
/// Following are the specifications and requirements for the stream methods
/// that implement a stream type.
///
/// <b> int sread(FILE *stream, void *buf, size_t count, size_t *read) </b>
///
/// The low-level read() function is named sread() here due to differences in
/// the prototype and semantics from the Unix counterpart.  The sread()
/// functions attempts to read \a count bytes from the \a stream into \a buf,
/// sets the number of bytes read, and returns either 0 for success or a
/// negative error code.  The only reason that sread() may terminate without
/// reading \a count bytes is if sread() also returns a negative return
/// code. 
///
/// The device-specific sread() function may assume that the \a stream, \a buf
/// and \a read parameters are non-NULL at entry, and the \a count is greater
/// than 0. These conditions are checked/guaranteed by the generic interface.
///
/// <b> int swrite(FILE *stream, void *buf, size_t count, size_t *written) </b>
///
/// The low-level write() function is named swrite() here due to differences
/// in the prototype and semantics from the Unix counterpart.  The swrite()
/// function attempts to writes \a count bytes from buf to the \a stream, sets
/// the number of bytes written, and returns either 0 for success or a
/// negative error code.  The only reason that swrite() may terminate without
/// writing \a count bytes is if write() also returns a negative return
/// code. 
///
/// The device-specific swrite() function may assume that the \a stream, \a
/// buf and \a written parameters are non-NULL at entry, and the \a count is
/// greater than 0. These conditions are checked/guaranteed by the generic
/// interface.
///
/// <b> int fflush(FILE *stream) </b>
///
/// SSX implements the fflush() call, even though the semantics are slightly
/// different from the Posix standard.  The Posix fflush() returns either 0
/// for success, or EOF to indicate an error.  Here, fflush() returns negative
/// 'errno' codes directly in the event of errors. The Posix standard
/// specifies that calling fflush() with a NULL \a stream causes all open
/// files to be flushed - here it returns the -EBADF error.
///
/// The stream-specific fflush() implementation should return one of the
/// 'errno' codes specified by the Posix standard in the event of
/// problems.

#include "ssx.h"
#include "ssx_io.h"

/// Initialize an SSX I/O FILE structure
///
/// \param file A pointer to an uninitialized FILE
///
/// \param flags Flags for the generic FILE
///
/// This API is designed to be called from "constructors of classes derived
/// from FILE", e.g., the StringStream.  The generic FILE structure is
/// cleared, and the flags are installed and other initialization based on the
/// flags is performed. The subclass constructors then install function
/// pointers for any of the FILE functions that they implement.
///
/// \retval 0 Success
///
/// \retval -FILE_INVALID_OBJECT The \a file pointer is null (0).
///
/// \retval -FILE_INVALID_ARGUMENT The \a flags are not valid.

int
FILE_create(FILE* file, int flags)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(file == 0, FILE_INVALID_OBJECT);
        SSX_ERROR_IF(flags & ~SSX_FILE_VALID_FLAGS, FILE_INVALID_ARGUMENT);
        SSX_ERROR_IF(__builtin_popcount(flags & SSX_FILE_OP_LOCK_OPTIONS) > 1,
                     FILE_INVALID_ARGUMENT);
    }
    memset((void*)file, 0, sizeof(FILE));
    file->flags = flags;
    ssx_semaphore_create(&(file->fop_sem), 1, 1);
    return 0;
}


/// Assign an error code to a stream, returning a non-zero value for errors
/// considered fatal.

int
ssx_io_error_set(FILE *stream, int code)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(stream == 0, EBADF);
    }
    stream->error = code;
    if ((code == EOF) ||
        (code == EAGAIN)) {
        return 0;
    } else {
        return -1;
    }
}


static int
null_stream_sread(FILE *stream, void *buf, size_t count, size_t *read)
{
    *read = 0;
    ssx_io_error_set(stream, EOF);
    return EOF;
}


static int
null_stream_swrite(FILE *stream, const void *buf, size_t count, size_t *written)
{
    *written = count;
    return 0;
}


static int
null_stream_fflush(FILE *stream)
{
    return 0;
}


/// A null stream; equivalent to Unix /dev/null
///
/// This stream is statically initialized as it is required to be assigned to
/// \a ssxout at load time.  The NULL stream does not require any locking.

FILE _null_stream = {
    .sread  = null_stream_sread,
    .swrite = null_stream_swrite,
    .fflush = null_stream_fflush,
    .error = 0,
    .flags = 0
};


// It's up to the application to initialize the standard streams if it wishes
// to use them.  Reading or writing a NULL stream will signal an error.

FILE *stdin = 0;
FILE *stdout = 0;
FILE *stderr = 0;

/// Stream used by printk()
///
/// \a ssxout is the stream used by printk().  \a ssxout \e must be assigned
/// to a non-blocking stream as it may be called from interrupt handlers.  \a
/// ssxout defaults to _null_stream, the equivalent of Unix' /dev/null.

FILE *ssxout = &_null_stream;


/// /dev/null for SSX
FILE* ssxnull = &_null_stream;


/// Lock low-level file operations
///
/// The decision on if and how low-level FILE operations needs to be locked is
/// made by the class derived from FILE, and may also require an argument
/// about its intended use.  The minimum bar is always correctness. The next
/// bar is whether the application allows data from multiple writers to be
/// intermixed, or allows multiple readers to access the same input stream.
///
/// A device like the StringStream used to implement sprintf() and snprintf()
/// does not require locking at all because it is only called from a single
/// context. String streams that implement circular buffers must be locked if
/// there is the possibility of multiple readers or writers, and the wrapping
/// buffer always needs to be locked.
///
/// The lock implemented here is a lock on the low-level stream operations
/// sread(), swrite() and fflush().

#define LOCK_FILE_OPERATION(stream, operation)                          \
    ({                                                                  \
        int __rc;                                                       \
        SsxMachineContext __ctx = SSX_THREAD_MACHINE_CONTEXT_DEFAULT; /* GCC */ \
        if ((stream)->flags & SSX_FILE_OP_LOCK_OPTIONS) {               \
            if ((stream)->flags & SSX_FILE_OP_LOCK_CRITICAL) {          \
                ssx_critical_section_enter(SSX_CRITICAL, &__ctx);       \
            } else if ((stream)->flags & SSX_FILE_OP_LOCK_NONCRITICAL) { \
                ssx_critical_section_enter(SSX_NONCRITICAL, &__ctx);    \
            } else {                                                    \
                ssx_semaphore_pend(&((stream)->fop_sem), SSX_WAIT_FOREVER); \
            }                                                           \
        }                                                               \
        __rc = (operation);                                             \
        if ((stream)->flags & SSX_FILE_OP_LOCK_OPTIONS) {               \
            if ((stream)->flags & (SSX_FILE_OP_LOCK_CRITICAL |          \
                                   SSX_FILE_OP_LOCK_NONCRITICAL)) {     \
                ssx_critical_section_exit(&__ctx);                      \
            } else {                                                    \
                ssx_semaphore_post(&((stream)->fop_sem));               \
            }                                                           \
        }                                                               \
        __rc;                                                           \
    })


/// Call the sread() operation of a stream

int
sread(FILE *stream, void *buf, size_t count, size_t *read)
{
    ssize_t rc;
    size_t _read;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((stream == 0), EBADF);
        SSX_IO_ERROR_IF(stream, (stream->sread == 0), ENXIO);
        SSX_IO_ERROR_IF(stream, (buf == 0), EINVAL);
    }
    if (count == 0) {
        if (read != 0) {
            *read = 0;
        }
        return 0;
    }
    rc = LOCK_FILE_OPERATION(stream, stream->sread(stream, buf, count, &_read));
    if (read != 0) {
        *read = _read;
    }
    return rc;
}


/// Call the swrite() operation of a stream

ssize_t
swrite(FILE *stream, const void *buf, size_t count, size_t *written)
{
    ssize_t rc;
    size_t _written;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((stream == 0), EBADF);
        SSX_IO_ERROR_IF(stream, (stream->swrite == 0), ENXIO);
        SSX_IO_ERROR_IF(stream, (buf == 0), EINVAL);
    }
    if (count == 0) {
        rc = 0;
        _written = 0;
    } else {
        rc = LOCK_FILE_OPERATION(stream,
                                 stream->swrite(stream, buf, count, &_written));
    }
    if (written != 0) {
        *written = _written;
    }
    return rc;
}


/// Call the fflush() operation of the stream

int
fflush(FILE *stream)
{
    int rc;

    if (SSX_ERROR_CHECK_API) {
        SSX_IO_ERROR_IF(stream, (stream == 0), EBADF);
        SSX_IO_ERROR_IF(stream, (stream->fflush == 0), ENXIO);
    }
    rc = LOCK_FILE_OPERATION(stream, stream->fflush(stream));
    return rc;
}
