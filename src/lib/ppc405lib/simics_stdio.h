/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/simics_stdio.h $                            */
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
#ifndef __SIMICS_STDIO_H__
#define __SIMICS_STDIO_H__

/// \file simics_stdio.h
/// \brief SSX I/O implementations for Simics stdio streams

#include "ssx_io.h"
#include "simics_stdio_addresses.h"
#include "string.h"

/// A FILE structure for a Simics fake stdio stream

typedef struct {

    /// The base class
    FILE stream;

    /// The MMIO address of the Simics read port for the stream
    SsxAddress read_address;

    /// The MMIO address of the Simics write port for the stream
    SsxAddress write_address;

    /// The MMIO address of the Simics device for flushing the stream;
    SsxAddress flush_address;

} SimicsStdio;

extern SimicsStdio simics_stdin;
extern SimicsStdio simics_stdout;
extern SimicsStdio simics_stderr;

int
simics_stdio_create(SimicsStdio* stream, 
                    SsxAddress read_address,
                    SsxAddress write_address,
                    SsxAddress flush_address);

int
simics_stdin_create(SimicsStdio *stream);

int
simics_stdout_create(SimicsStdio *stream);

int
simics_stderr_create(SimicsStdio *stream);

int
simics_stdfile_create(SimicsStdio *stream, int fn);

int
simics_stdio_sread(FILE *stream, void *buf, size_t count, size_t *read);

int
simics_stdio_swrite(FILE *stream, const void *buf,
                    size_t count, size_t *written);

int 
simics_stdio_sflush(FILE *stream);

#endif /* __SIMICS_STDIO_H__ */
