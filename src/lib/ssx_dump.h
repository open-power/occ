#ifndef __SSX_DUMP_H__
#define __SSX_DUMP_H__

// $Id: ssx_dump.h,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/ssx_dump.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

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
