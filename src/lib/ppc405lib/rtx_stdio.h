/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/rtx_stdio.h $                               */
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
#ifndef __RTX_STDIO_H__
#define __RTX_STDIO_H__

// $Id$

/// \file rtx_stdio.h
/// \brief SSX I/O implementations for RTX stdio streams

#include "ssx_io.h"
#include "rtx_stdio_addresses.h"

/// A FILE structure for a RTX fake stdio stream

typedef struct {

    /// The base class
    FILE stream;

    /// The MMIO address of the RTX device for the stream
    SsxAddress address;

    /// The MMIO address of the RTX device for flushing the stream;
    SsxAddress flush_address;

} RtxStdio;

extern RtxStdio rtx_stdin;
extern RtxStdio rtx_stdout;
extern RtxStdio rtx_stderr;

int
rtx_stdin_create(RtxStdio *stream);

int
rtx_stdout_create(RtxStdio *stream);

int
rtx_stderr_create(RtxStdio *stream);

int
rtx_stdio_sread(FILE *stream, void *buf, size_t count, size_t *read);

int
rtx_stdio_swrite(FILE *stream, const void *buf,
                    size_t count, size_t *written);

int
rtx_stdio_sflush(FILE *stream);

#endif /* __RTX_STDIO_H__ */
