// $Id: assert.c,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/assert.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file assert.c
/// \brief Implementation of library routines implied by <assert.h>

#include "ssx.h"
#include "ssx_io.h"
#include "libssx.h"

/// The __assert_fail() function is used to implement the assert() interface
/// of ISO POSIX (2003). The __assert_fail() function prints the given \a
/// file filename, \a line line number, \a function function name and a
/// message on the standard error stream then causes a kernel panic.  If there
/// is no standard error stream then the error message is printed on the \a
/// ssxout (printk()) stream.
/// 
/// If function is NULL, __assert_fail() omits information about the
/// function. The aguments \a assertion, \a file, and \a line must be
/// non-NULL.

void
__assert_fail(const char *assertion, 
              const char *file, 
              unsigned line, 
              const char *function)
{
    FILE *stream;

    stream = stderr;
    if (stream == 0) {
        stream = ssxout;
    }

    fprintf(stream, "%s:%u:%s%s Assertion '%s' failed\n",
            file, line, 
            function ? function : "", function ? ":" : "",
            assertion);

    SSX_PANIC(ASSERTION_FAILURE);
}

