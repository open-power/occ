/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/stdlib.c $                                  */
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
// $Id: stdlib.c,v 1.1.1.1 2013/12/11 20:49:20 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/lib/stdlib.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file stdlib.c
/// \brief Functions from <stdlib.h>
///
/// \note The strtoX() APIs are defined in strtox.[ch]

#include "ssx.h"
#include "ctype.h"
#include "libssx.h"
#include <stdlib.h>


/// Convert a string to a long integer - base 10 only
///
/// atol(str) is defined here as strtol(str, 0, 10) in sympathy with the POSIX
/// standard.  Note that by specification "atol() does not detect
/// errors", however here it will behave the same as the strtol() call just
/// mentioned.

long
atol(const char *str)
{
    return strtol(str, 0, 10);
}


/// Convert a string to an integer - base 10 only
///
/// atoi(str) is defined here as strtol(str, 0, 10) in sympathy with the POSIX
/// standard. Note that by specification "atoi() does not detect errors",
/// however in this implementation the long integer returned by the strtol()
/// call just mentioned is simply converted to an int.

int
atoi(const char *str)
{
    return strtol(str, 0, 10);
}


/// 'Exit' an application
///
///  An SSX application can not really 'exit'.  By convention, exit(0) is the
///  same as ssx_halt().  Calling exit() with a non-zero code causes a kernel
///  panic - the exit code will be found in R3 on PowerPC.
///
///  Note that to exit a thread, the thread can either return from the thread
///  entry routine or explicitly call ssx_complete().  exit() was implemented
///  to allow porting of the EEMBC benchmarks.

void
exit(int status)
{
    if (status) {
        SSX_PANIC(ERROR_EXIT);
    } 
    ssx_halt();
}


/// Compute the absolute value of the integer argument
int 
abs(int i)
{
    return ((i < 0) ? -i : i);
}


/// Compute the absolute value of the long integer argument
long int
labs(long int i)
{
    return ((i < 0) ? -i : i);
}


/// Compute the absolute value of the long long integer argument
long long int
llabs(long long int i)
{
    return ((i < 0) ? -i : i);
}


