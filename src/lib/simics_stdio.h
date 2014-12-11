#ifndef __SIMICS_STDIO_H__
#define __SIMICS_STDIO_H__

// $Id: simics_stdio.h,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/simics_stdio.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

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
