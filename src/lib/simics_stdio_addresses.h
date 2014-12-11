#ifndef __SIMICS_STDIO_ADDRESSES_H__
#define __SIMICS_STDIO_ADDRESSES_H__

// $Id: simics_stdio_addresses.h,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/simics_stdio_addresses.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file simics_stdio_adresses.h
/// \brief MMIO addresses and offsets of the Simics fake stdio model
///
/// The Simics 'stdio' component is a pseudo serial port for I/O to stdio,
/// stdout and stderr, as well as to other configurable streams.  Each virtual
/// file potentially has a read port, write port and flush port.
///
/// The write ports accept 1, 2 and 4-byte transactions on a 32-bit OCI
/// address and write the data to the associated stream.  Writing any value to
/// the flush port flushes the output stream.  The input ports are not yet
/// implemented.

// -*- This header is maintained as part of the PMX Simics model. -*-
// -*- Do not edit in the SSX library as your edits will be lost. -*-

#define SIMICS_STDIO_BASE          0x40060000

#define SIMICS_STDIN_OFFSET        0x00
#define SIMICS_STDOUT_OFFSET       0x04
#define SIMICS_STDOUT_FLUSH_OFFSET 0x08
#define SIMICS_STDERR_OFFSET       0x0c
#define SIMICS_STDERR_FLUSH_OFFSET 0x10

#define SIMICS_STDFILE_0_OFFSET    0x14 
#define SIMICS_STDFILE_1_OFFSET    0x20
#define SIMICS_STDFILE_2_OFFSET    0x2c 
#define SIMICS_STDFILE_3_OFFSET    0x38 

#define SIMICS_STDFILE_STREAMS 4

#define SIMICS_STDIN        (SIMICS_STDIO_BASE + SIMICS_STDIN_OFFSET)
#define SIMICS_STDOUT       (SIMICS_STDIO_BASE + SIMICS_STDOUT_OFFSET)
#define SIMICS_STDOUT_FLUSH (SIMICS_STDIO_BASE + SIMICS_STDOUT_FLUSH_OFFSET)
#define SIMICS_STDERR       (SIMICS_STDIO_BASE + SIMICS_STDERR_OFFSET)
#define SIMICS_STDERR_FLUSH (SIMICS_STDIO_BASE + SIMICS_STDERR_FLUSH_OFFSET)

#define SIMICS_STDFILE_READ_OFFSET(fn) \
    (SIMICS_STDFILE_0_OFFSET + (12 * fn) + 0x00)

#define SIMICS_STDFILE_WRITE_OFFSET(fn) \
    (SIMICS_STDFILE_0_OFFSET + (12 * fn) + 0x04)

#define SIMICS_STDFILE_FLUSH_OFFSET(fn) \
    (SIMICS_STDFILE_0_OFFSET + (12 * fn) + 0x08)

#define SIMICS_STDFILE_READ(fn) \
         (SIMICS_STDIO_BASE + SIMICS_STDFILE_READ_OFFSET(fn))

#define SIMICS_STDFILE_WRITE(fn) \
         (SIMICS_STDIO_BASE + SIMICS_STDFILE_WRITE_OFFSET(fn))

#define SIMICS_STDFILE_FLUSH(fn) \
         (SIMICS_STDIO_BASE + SIMICS_STDFILE_FLUSH_OFFSET(fn))


#endif /* __SIMICS_STDIO_ADDRESSES_H__ */
