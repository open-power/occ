/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/string_stream.c $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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

/// \file string_stream.c
/// \brief Implementations of string streams.
///
/// \bug Need to work out overwrite/multiple-write protection.

#include "ssx.h"
#include "string_stream.h"


// 'Flush' a string stream by making it empty

static int
string_stream_fflush(FILE* stream)
{
    StringStream* string = (StringStream*) stream;

    string->next_read = 0;
    string->next_write = 0;
    string->flags &= ~STRING_STREAM_FULL;
    return 0;
}


// Read the remaining tail (if any) of the circular buffer, assuming the
// buffer is not empty.

static size_t
read_tail(StringStream* string, void* buf, size_t size)
{
    size_t read, remainder;

    read = 0;
    if (string->next_read >= string->next_write) {

        remainder = string->size - string->next_read;
        read = MIN(size, remainder);
        memcpy(buf, (void *)(&(string->data[string->next_read])), read);
        if (read != remainder) {
            string->next_read += read;
        } else {
            string->next_read = 0;
        }
    }
    return read;
}


// Read the area (if any) from the read pointer to the write pointer

static size_t
read_head(StringStream* string, void* buf, size_t size)
{
    size_t read, remainder;

    read = 0;
    if (string->next_write > string->next_read) {

        remainder = string->next_write - string->next_read;
        read = MIN(size, remainder);
        memcpy(buf, (void *)(&(string->data[string->next_read])), read);
        string->next_read += read;
    }
    return read;
}


// Read as much data as possible from a circular buffer.  Return -EAGAIN if the
// buffer would underflow.

static int
string_stream_sread(FILE* stream, void* buf, size_t size, size_t* read)
{
    StringStream *string = (StringStream *)stream;
    size_t were_read;
    int rc;

    if ((string->next_read == string->next_write) &&
        !(string->flags & STRING_STREAM_FULL)) {
        were_read = 0;
    } else {
        were_read = read_tail(string, buf, size);
        if (were_read != size) {
            were_read += read_head(string, buf + were_read, size - were_read);
        }
    }
    if (were_read != 0) {
        string->flags &= ~STRING_STREAM_FULL;
    }
    *read = were_read;
    if (were_read < size) {
        rc = -EAGAIN;
    } else {
        rc = 0;
    }
    return rc;
}




// Write the remaining tail (if any) of the circular buffer, assuming the
// buffer is not full.

static size_t
write_tail(StringStream* string, const void* buf, size_t size)
{
    size_t written, remainder;

    written = 0;
    if (string->next_write >= string->next_read) {

        remainder = string->size - string->next_write; // string->size is the size of the stream(our buffer)
        written = MIN(size, remainder);
        // memcpy(void *dest, const void *src, size_t n) from /lib/memcpy.c
        memcpy((void *)(&(string->data[string->next_write])), buf, written);
        if (written != remainder) {
            string->next_write += written;
        } else {
            string->next_write = 0;
        }
    }
    return written;
}

// Write the area (if any) from the write pointer to the read pointer

static size_t
write_head(StringStream* string, const void* buf, size_t size)
{
    size_t written, remainder;

    written = 0;
    if (string->next_read > string->next_write) {

        remainder = string->next_read - string->next_write;
        written = MIN(size, remainder);
        memcpy((void *)(&(string->data[string->next_write])), buf, written);
        string->next_write += written;
    }
    return written;
}


// Write as much data as possible to a circular buffer.  Return -EAGAIN if the
// buffer would overflow.

static int
circular_swrite(FILE* stream, const void* buf, size_t size, size_t* written)
{
    StringStream *string = (StringStream *)stream;
    size_t wrote;
    int rc;

    if (string->flags & STRING_STREAM_FULL) {
        wrote = 0;
    } else {
        wrote = write_tail(string, buf, size);
        if (wrote != size) {
            wrote += write_head(string, buf + wrote, size - wrote);
        }
    }
    if ((wrote != 0) && (string->next_read == string->next_write)) {
        string->flags |= STRING_STREAM_FULL;
    }
    *written = wrote;
    if (wrote < size) {
        rc = -EAGAIN;
    } else {
        rc = 0;
    }
    return rc;
}


// Effectively write all data to a circular buffer with wrapping semantics.

static int
wrapping_swrite(FILE* stream, const void* buf, size_t size, size_t* written)
{
    StringStream *string = (StringStream *)stream;
    size_t wrote;
    int rc;

    if (size >= string->size) { // If size of data >= size of buffer

        // If the amount of data will fill or overflow the entire buffer size
        // then we effectively fill the buffer with the final bytes of data.

        string->next_read = 0;
        string->next_write = 0;
        string->flags |= STRING_STREAM_FULL;
        memcpy((void *)string->data, buf + (size - string->size), size);

    } else {

        // If the string is not full, try to fill it with the
        // circular_swrite().

        if (string->flags & STRING_STREAM_FULL) {
            wrote = 0;
            rc = -EAGAIN;
        } else {
            rc = circular_swrite(stream, buf, size, &wrote);
        }
        if (rc) {

            // The string is full and we need to overflow. We know that size
            // is less than the buffer size, and the next_read == next_write.
            // Mark the stream not full so a new circular write will work, and
            // at the end reset the full condition.

            string->flags &= ~STRING_STREAM_FULL;
            rc = circular_swrite(stream, buf + wrote, size - wrote, &wrote);
            if (rc) {
                SSX_PANIC(STRING_STREAM_BUG);
            }
            string->next_read = string->next_write;
            string->flags |= STRING_STREAM_FULL;
        }
    }
    *written = size;
    return 0;
}


static int
linear_swrite(FILE* stream, const void* buf, size_t size, size_t* written)
{
    // buf and size correspond to the data we are passing to our own buffer
    uint32_t bit_0_mask = 0x80000000;
    StringStream *string = (StringStream *)stream;
    size_t wrote = 0; // right aligned
    // int rc;
    uint32_t num_bytes_written;
    // uint32_t register_contents;


    //if (wrote != size) {
      // register_contents = in32(PMC_PORE_SCRATCH_REG1);
      // register_contents = register_contents & bit_0_mask;

       //if (!register_contents) {
         // Before writing to SRAM, flush everything so it will write to the top
         // of the buffer each time
         string_stream_fflush(stream);

         // Write printk statement to SRAM
         // wrote will contain the number of bytes written

         wrote += write_tail(string, buf + wrote, size - wrote);
         // if wrote != size, tell Tcl to read the whole buffer, When Tcl is done loop back to write_tail
         // buf= buf+wrote
         // size= size - wrote
         // continue loop until size = 0

         // Sync
         eieio();

         // Store "wrote" to register
         // Set bit 0 to 1
         // out32(addr, data)
         num_bytes_written = (uint32_t)wrote | bit_0_mask;
         out32( PMC_PORE_SCRATCH_REG1 , num_bytes_written );

         // Sync
         eieio();
       //}
    //}

    // outside of loop, set *written = wrote (which is equal to size once all data has been copied to our buffer)
    *written = wrote;
    return 0;

}


int
_string_stream_create(StringStream* stream,
                      void* buf, size_t size, int flags,
                      int (*swrite)(FILE* stream,
                                    const void* buf,
                                    size_t size,
                                    size_t* written))
{
    FILE* file = (FILE*)stream;
    int rc;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((stream == 0) ||
                     ((buf == 0) && (size != 0)),
                     STRING_STREAM_INVALID_ARGUMENT);
    }

    rc = FILE_create(file, flags);

    if (!rc) {
        file->swrite = swrite;
        file->sread = string_stream_sread;
        file->fflush = string_stream_fflush;
        stream->data = buf;
        stream->size = size;
        stream->next_read = 0;
        stream->next_write = 0;
        stream->flags = 0;
    }
    return rc;
}


int
circular_stream_create(CircularStream* stream,
                       void* buf, size_t size, int flags)
{
    return _string_stream_create(stream, buf, size, flags, circular_swrite);
}


int
wrapping_stream_create(CircularStream* stream,
                       void* buf, size_t size, int flags)
{
    return _string_stream_create(stream, buf, size, flags, wrapping_swrite);
}


int
linear_stream_create(CircularStream* stream,
                     void* buf, size_t size, int flags)
{
     FILE* file = (FILE*)stream;
     int rc;

     rc = _string_stream_create(stream, buf, size, flags, linear_swrite);
     if (!rc) {
         
         file->sread = NULL;
         // Write to register where location of buffer is
         out32( PMC_PORE_SCRATCH_REG1, (uint32_t)buf);
     }
     return rc;
}


// InputStream uses string_stream_sread(), however returns EOF once all data
// has been read.

static int
input_stream_sread(FILE* stream, void* buf, size_t size, size_t* read)
{
    int rc;

    rc = string_stream_sread(stream, buf, size, read);
    if (rc == -EAGAIN) {
        rc = EOF;
    }

    return rc;
}


// For simplicity (and ease of maintainence) we create a normal string stream
// then overwrite a few key fields.

int
input_stream_create(StringStream* stream, void* buf, size_t size, int flags)
{
    int rc;

    rc = _string_stream_create(stream, buf, size, flags, 0);
    if (!rc) {
        stream->stream.sread = input_stream_sread;
        stream->stream.fflush = 0;
        stream->flags = STRING_STREAM_FULL;
    }
    return rc;
}


