/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/simics_stdio_addresses.h $                            */
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
#ifndef __SIMICS_STDIO_ADDRESSES_H__
#define __SIMICS_STDIO_ADDRESSES_H__

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
