/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/assert.c $                                            */
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

