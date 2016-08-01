/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/ssx_dump.h $                                */
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
#ifndef __SSX_DUMP_H__
#define __SSX_DUMP_H__

/// \file ssx_dump.h 
/// \brief Routines for dumping SSX kernel data structures

#include "ssx.h"

/// \defgroup ssx_dump_options Options for ssx_dump()
///
/// No options are currently specified.
///
/// @{

/// @}

#ifndef __ASSEMBLER__

/// Dump the kernel state
///
/// \param i_stream The stream to receive the dump.  If the dump is being
/// generated prior to a kernel panic then this would typically be \a ssxout,
/// the stream used by printk.
///
/// \param i_options AN OR-mask of option flags; See \ref ssx_dump_options
///
/// The SSX kernel dump produces a formatted snapshot of the state of the
/// kernel and the mapped threads. This API does not manipulate the machine
/// context; If it is required to produce a precise snapshot then the caller
/// will need to make the call from a critical section.
///
/// The following information is standard in the dump
///
/// - The interrupt and thread state of the kernel
/// - The state of each thread
/// - A stack trace for each thread
///
/// Options : TBD
///
/// \bug There are likely several bugs in the current implementation due to
/// the assumption that the code is being called from a state in which the
/// kernel context is not changing. We don't have time to code and test the
/// most general implementation now.  To guarantee correct operation the API
/// must currently be called from an SSX_CRITICAL critical section.
void
ssx_dump(FILE* stream, int options);

#endif // __ASSEMBLER__

#endif // __SSX_DUMP_H__
