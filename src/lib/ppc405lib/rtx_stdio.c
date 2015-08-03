/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/rtx_stdio.c $                               */
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
// $Id$

/// \file rtx_stdio.c
/// \brief SSX I/O drivers for RTX stdio streams
///
/// The RTX \a stdout and \a stderr components accept 1, 2 and 4-byte
/// transactions on a 32-bit OCI address and write the data to the RTX
/// job's \a stdout or \a stderr respectively.  The \a stdin device is not yet
/// implemented.

#include "ssx.h"
#include "rtx_stdio.h"

RtxStdio rtx_stdin;
RtxStdio rtx_stdout;
RtxStdio rtx_stderr;

int
rtx_stdio_sread(FILE *stream, void *buf, size_t count, size_t *read)
{
    SSX_PANIC(ENXIO);
    return -ENXIO;
}


int
rtx_stdio_swrite(FILE *stream, const void *buf,
                    size_t count, size_t *written)
{
    RtxStdio *rtx = (RtxStdio *)stream;
    size_t n;

    n = count;
    while (n) {
        if (n >= 4) {
            out32(rtx->address, *((uint32_t *)buf));
            buf += 4;
            n -= 4;
        } else if (n >= 2) {
            out16(rtx->address, *((uint16_t *)buf));
            buf += 2;
            n -= 2;
        } else {
            out8(rtx->address, *((uint8_t *)buf));
            buf++;
            n--;
        }
    }

    if (written != 0) {
        *written = count;
    }

    return 0;
}


ssize_t
rtx_stdio_fflush(FILE *stream)
{
    RtxStdio *rtx = (RtxStdio *)stream;

    out8(rtx->flush_address, 0);
    return 0;
}


int
rtx_stdin_create(RtxStdio *stream)
{
    FILE *base = (FILE *)stream;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(stream == 0, EBADF);
    }

    memset((void *)stream, 0, sizeof(RtxStdio));
    base->sread = rtx_stdio_sread;

    stream->address = RTX_STDIN;

    return 0;
}


int
rtx_stdout_create(RtxStdio *stream)
{
    FILE *base = (FILE *)stream;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(stream == 0, EBADF);
    }

    memset((void *)stream, 0, sizeof(RtxStdio));
    base->swrite = rtx_stdio_swrite;
    base->fflush = rtx_stdio_fflush;

    stream->address = RTX_STDOUT;
    stream->flush_address = RTX_STDOUT_FLUSH;

    return 0;
}


int
rtx_stderr_create(RtxStdio *stream)
{
    FILE *base = (FILE *)stream;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(stream == 0, ENXIO);
    }

    memset((void *)stream, 0, sizeof(RtxStdio));
    base->swrite = rtx_stdio_swrite;
    base->fflush = rtx_stdio_fflush;

    stream->address = RTX_STDERR;
    stream->flush_address = RTX_STDERR_FLUSH;

    return 0;
}


