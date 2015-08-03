/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/string_stream.h $                           */
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
#ifndef __STRING_STREAM_H__
#define __STRING_STREAM_H__

// $Id: string_stream.h,v 1.2 2014/02/03 01:30:25 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/string_stream.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file string_stream.h
/// \brief Implementations of string streams.

#include "ssx_io.h"

/// A string stream
///
/// This structure is used for the public CircularStream and WrappingStream
/// types, as well as for the library-internal types used to implement
/// sprintf() and snprintf().

typedef struct {

    /// The base class
    FILE stream;

    /// Data storage - provided by the creator
    uint8_t *data;

    /// The size of the data storage
    size_t size;

    /// The index of the next byte to write
    size_t next_write;

    /// The index of the next byte to read
    size_t next_read;

    /// Stream flags, see \ref string_stream_flags
    int flags;

} StringStream;


/// A StringStream with circular buffer semantics
///
/// The swrite() method copies the input data to the stream buffer, which is
/// treated as a circular buffer.  If the swrite() would overflow the buffer,
/// then as much data as possible is written and swrite returns -EAGAIN.  In
/// all cases swrite() returns the number of bytes actually written to the
/// buffer.
///
/// The sread() method copies data from the StringStream circular buffer to
/// the caller's buffer until either the caller's request is satisfied or all
/// of the immediately available data has been read from the buffer.  If the
/// caller's request can not be immediately granted then as much data as
/// possible is copied and sread() returns -EAGAIN. In all cases sread()
/// returns the number of bytes actually read from the buffer.
///
/// The fflush() method marks the buffer as empty, effectively losing any data
/// currently stored in the buffer.

typedef StringStream CircularStream;


/// A StringStream with wrapping circular buffer semantics
///
/// The swrite() method copies the input data to the stream buffer, which is
/// treated as a circular buffer.  If the swrite() would overflow the buffer,
/// then unread data is overwritten with new data.  If the size of the
/// swrite() exceeds the buffer length then the effect is simply to fill the
/// buffer with the final bytes of the caller's data.  swrite() always returns
/// the number of bytes requested to be written.
///
/// The sread() method copies data from the StringStream circular buffer to
/// the caller's buffer until either the caller's request is satisfied or all
/// of the immediately available data has been read from the buffer.  If the
/// caller's request can not be immediately granted then as much data as
/// possible is copied and sread() returns -EAGAIN. In all cases sread()
/// returns the number of bytes actually read from the buffer.
///
/// The fflush() method marks the buffer as empty, effectively losing any data
/// currently stored in the buffer.

typedef StringStream WrappingStream;


/// Create a StringStream
///
/// \param stream The StringStream to initialize
///
/// \param buf The stream data buffer
///
/// \param size The size of the data buffer in bytes
///
/// \param flags Flags for FILE_create()
///
/// \param swrite  The function to be installed as the swrite() function for
/// the underlying FILE object.  The sread() and fflush() functions are fixed.
///
/// This API initializes a StringStream structure for use in I/O operations.
/// This API will typically only be used by the creation functions of specific
/// StringStream types.
///
/// \retval 0 Success
///
/// \retval -STRING_STREAM_INVALID_ARGUMENT Either \a stream is NULL (0) 
/// or \a buf is NULL(0) and \a size is non-0.

int
_string_stream_create(StringStream* stream,
                      void* buf, size_t size, int flags,
                      int (*swrite)(FILE* stream,
                                    const void* buf,
                                    size_t size,
                                    size_t* written));

/// Create a CircularStream
///
/// \param stream The CircularStream to initialize
///
/// \param buf The stream data buffer
///
/// \param size The size of the data buffer in bytes
///
/// \param flags Flags for FILE_create()
///
/// This API initializes a CircularStream structure for use in I/O
/// operations.  Once created, the pointer to the CircularStream stream can be
/// cast to a FILE* and used for sread(), swrite(), fflush() and fprintf()
/// operations.
///
/// \retval 0 Success
///
/// \retval -STRING_STREAM_INVALID_ARGUMENT Either \a stream is NULL (0) 
/// or \a buf is NULL(0) and \a size is non-0.

int
circular_stream_create(CircularStream* stream,
                       void* buf, size_t size, int flags);


/// Create a WrappingStream
///
/// \param stream The WrappingStream to initialize
///
/// \param buf The stream data buffer
///
/// \param size The size of the data buffer in bytes
///
/// \param flags Flags for FILE_create()
///
/// This API initializes a WrappingStream structure for use in I/O
/// operations.  Once created, the pointer to the WrappingStream stream can be
/// cast to a FILE* and used for sread(), swrite(), fflush() and fprintf()
/// operations.
///
/// \retval 0 Success
///
/// \retval -STRING_STREAM_INVALID_ARGUMENT Either \a stream is NULL (0) 
/// or \a buf is NULL(0) and \a size is non-0.

int
wrapping_stream_create(CircularStream* stream,
                       void* buf, size_t size, int flags);


/// Create a LinearStream
///
/// \param stream The LinearStream to initialize
///
/// \param buf The stream data buffer
///
/// \param size The size of the data buffer in bytes
///
/// \param flags Flags for FILE_create()
///
/// This API initializes a LinearStream structure for use in I/O
/// operations.  Once created, the pointer to the WrappingStream stream can be
/// cast to a FILE* and used for sread(), swrite(), fflush() and fprintf()
/// operations.
///
/// \retval 0 Success
///
/// \retval -STRING_STREAM_INVALID_ARGUMENT Either \a stream is NULL (0) 
/// or \a buf is NULL(0) and \a size is non-0.


int
linear_stream_create(CircularStream* stream,
                       void* buf, size_t size, int flags);


/// A StringStream used as a read-only input stream
///
/// This is a CircualarStream created with a full buffer and without a write
/// method. It uses a special read method that returns EOF once the buffer is
/// empty rather than -EAGAIN.

typedef StringStream InputStream;


/// Create an InputStream
///
/// \param stream The InputStream to initialize
///
/// \param buf The stream data buffer.
///
/// \param size The size of the readable portion of the data buffer in bytes
///
/// \param flags Flags for FILE_create()
///
/// This API initializes an InputStream structure for use in input operations.
/// The stream data buffer contains \a size bytes of data which can be read.
/// Once created, the pointer to the InputStream stream can be cast to a FILE*
/// and used for sread(), fgetc() and ungetc() operations.  Neither fflush()
/// nor swrite() are supported on this class of stream. The read operations
/// will succeed until \a size bytes have been returned from the stream, at
/// which point the stream will return EOF on any subsequent reads.
///
/// \retval 0 Success
///
/// \retval -STRING_STREAM_INVALID_ARGUMENT Either \a stream is NULL (0) 
/// or \a buf is NULL(0) and \a size is non-0.

int
input_stream_create(InputStream* stream,
                    void* buf, size_t size, int flags);


/// \defgroup string_stream_flags StringStream Flags
///
/// @{

/// The StringStream is full
#define STRING_STREAM_FULL 0x1

/// @}


/// \defgroup string_stream_codes StringStream Error/Panic Codes
///
/// @{

/// A bug was detected in a StringStream function
#define STRING_STREAM_BUG              0x00787701
#define STRING_STREAM_INVALID_ARGUMENT 0x00787702

/// @}


#endif // __STRING_STREAM_H__
