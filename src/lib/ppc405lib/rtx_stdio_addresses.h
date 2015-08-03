/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/rtx_stdio_addresses.h $                     */
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
#ifndef __RTX_STDIO_ADDRESSES_H__
#define __RTX_STDIO_ADDRESSES_H__

// $Id$

/// \file rtx_stdio_addresses.h
/// \brief MMIO addresses and offsets of the rtx fake stdio model
///
/// The RTX stdio module appears as OCI device #7?  Reading 1 byte from the
/// stdin offset returns that byte from stdin. Writing 1, 2 or 4 bytes to the
/// stdout or stderr offsets causes output on that stream.  Writing any single
/// byte to the 'flush' offsets flush the stdout or stderr streams.
///
/// -*- This header is maintained as part of the PMX RTX model.    -*-
/// -*- Do not edit in the SSX library as your edits will be lost. -*-

#define RTX_STDIO_BASE          0x40060000

#define RTX_STDIN_OFFSET        0x00
#define RTX_STDOUT_OFFSET       0x04
#define RTX_STDOUT_FLUSH_OFFSET 0x08
#define RTX_STDERR_OFFSET       0x0c
#define RTX_STDERR_FLUSH_OFFSET 0x10

#define RTX_STDIN        (RTX_STDIO_BASE + RTX_STDIN_OFFSET)
#define RTX_STDOUT       (RTX_STDIO_BASE + RTX_STDOUT_OFFSET)
#define RTX_STDOUT_FLUSH (RTX_STDIO_BASE + RTX_STDOUT_FLUSH_OFFSET)
#define RTX_STDERR       (RTX_STDIO_BASE + RTX_STDERR_OFFSET)
#define RTX_STDERR_FLUSH (RTX_STDIO_BASE + RTX_STDERR_FLUSH_OFFSET)

#endif /* __RTX_STDIO_ADDRESSES_H__ */
