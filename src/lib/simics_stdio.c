// $Id: simics_stdio.c,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/simics_stdio.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file simics_stdio.c
/// \brief SSX I/O drivers for Simics stdio streams
///
/// The Simics 'stdio' component is a pseudo serial port for I/O to stdio,
/// stdout and stderr, as well as to other configurable streams.  Each virtual
/// file potentially has a read port, write port and flush port.
///
/// The write ports accept 1, 2 and 4-byte transactions on a 32-bit OCI
/// address and write the data to the associated stream.  Writing any value to
/// the flush port flushes the output stream.  The input ports are not yet
/// implemented.

#include "ssx.h"
#include "simics_stdio.h"


SimicsStdio simics_stdin;
SimicsStdio simics_stdout;
SimicsStdio simics_stderr;

int
simics_stdio_sread(FILE *stream, void *buf, size_t count, size_t *read)
{
    SSX_PANIC(ENXIO);
    return -ENXIO;
}


int
simics_stdio_swrite(FILE *stream, const void *buf, 
                    size_t count, size_t *written)
{
    SimicsStdio *simics = (SimicsStdio *)stream;
    size_t n;
    
    n = count;
    while (n) {
        if (n >= 4) {
            out32(simics->write_address, *((uint32_t *)buf));
            buf += 4;
            n -= 4;
        } else if (n >= 2) {
            out16(simics->write_address, *((uint16_t *)buf));
            buf += 2;
            n -= 2;
        } else {
            out8(simics->write_address, *((uint8_t *)buf));
            buf++;
            n--;
        }
    }

    *written = count;

    return 0;
}


ssize_t
simics_stdio_fflush(FILE *stream)
{
    SimicsStdio *simics = (SimicsStdio *)stream;

    out8(simics->flush_address, 0);
    return 0;
}


/// Create a SimicsStdio stream
///
/// Any of the \a read_address, \a write_address or \a flush_address which are
/// non-0 specify that the stream supports the associated method.  The Simics
/// I/O drivers do not require locking.

int
simics_stdio_create(SimicsStdio* stream, 
                    SsxAddress read_address,
                    SsxAddress write_address,
                    SsxAddress flush_address)
{
    FILE *base = (FILE *)stream;
    int rc;

    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF(stream == 0, EBADF);
    }

    rc = FILE_create((FILE*)stream, 0);

    if (!rc) {

        stream->read_address = read_address;
        if (read_address != 0) {
            base->sread = simics_stdio_sread;
        }
    
        stream->write_address = write_address;
        if (write_address != 0) {
            base->swrite = simics_stdio_swrite;
        }
    
        stream->flush_address = flush_address;
        if (flush_address != 0) {
            base->fflush = simics_stdio_fflush;
        }
    }

    return rc;
}


int
simics_stdin_create(SimicsStdio *stream)
{
    return simics_stdio_create(stream, SIMICS_STDIN, 0, 0);
}


int
simics_stdout_create(SimicsStdio *stream)
{
    return simics_stdio_create(stream, 0, SIMICS_STDOUT, SIMICS_STDOUT_FLUSH);
}
   
int
simics_stderr_create(SimicsStdio *stream)
{
    return simics_stdio_create(stream, 0, SIMICS_STDERR, SIMICS_STDERR_FLUSH);
}

int
simics_stdfile_create(SimicsStdio *stream, int fn)
{
    if (SSX_ERROR_CHECK_API) {
        SSX_ERROR_IF((fn < 0) > (fn >= SIMICS_STDFILE_STREAMS), EBADF);
    }

    return simics_stdio_create(stream, 
                               SIMICS_STDFILE_READ(fn),
                               SIMICS_STDFILE_WRITE(fn),
                               SIMICS_STDFILE_FLUSH(fn));
}
