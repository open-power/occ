/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/initcall.c $                                */
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
// $Id$

/// \file initcall.c
/// \brief An anonymous early initialization facility for SSX applications

#include "ssx.h"
#include "initcall.h"

// These linker symbols must be defined if the initcall facility is used.  The
// special ELF section .data.initcall contains an array of Initcall structures
// for all declared initcalls.

extern InitCall        _INITCALL_SECTION_BASE[];
extern SsxLinkerSymbol _INITCALL_SECTION_SIZE;

void
_initcall_run(InitCall* initcall)
{
    void (*f)(void* arg);

    f = initcall->initcall;
    if (f) {
        initcall->initcall = 0;
        f(initcall->arg);
    }
}


void
initcall_run_all()
{
    InitCall* initcall;
    size_t nCalls;

    initcall = _INITCALL_SECTION_BASE;
    nCalls = (size_t)(&_INITCALL_SECTION_SIZE) / sizeof(InitCall);

    for (; nCalls--; initcall++) {
        _initcall_run(initcall);
    }
}



    
    
