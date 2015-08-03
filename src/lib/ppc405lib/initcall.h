/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/initcall.h $                                */
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
#ifndef __INITCALL_H__
#define __INITCALL_H__

// $Id$

/// \file initcall.h
/// \brief An early initialization facility for SSX applications
///
/// The C language standard does not define a generic load-time initialization
/// method, unlike C++ which supports load-time initialization of static
/// objects.  The \e initcall facility implements a simple method for SSX
/// applications to declare early initialization functions that are executed
/// prior to or during the invocation of main().
///
/// An \e initcall can be any function with the prototype
///
/// \code
///
/// void (*initcall)(void* arg)
///
/// \endcode
///
/// Initcalls are declared with the INITCALL() macro. An initcall is
/// represented by a named structure, and typically an initcall will be
/// declared static to the compilation unit that implements the initcall:
///
/// \code
///
/// void (*init_fn)(void* arg);
/// void* init_data = ...;
/// static INITCALL(init_var, init_fn, init_data);
///
/// \endcode
///
/// All INITCALLS loaded in the executable image are executed by the
/// initcall_run_all() API.  An SSX application will typically call
/// initcall_run_all() in the function declared as the \a ssx_main_hook, or in
/// the main() routine itself.
///
/// Initcalls are run in an arbitrary order. However if initcall \a b is
/// dependent on initcall \a a, then initcall \a b can execute
/// initcall_run(&a) to guarantee that initcall \a a runs before \a b.
/// Regardless, every initcall is run exectly once by the initcall facility,
/// even if initcall_run() or initcall_run_all() were to be used multiple
/// times.
///
/// Behind the scenes, initcalls are implemented by a special ELF section,
/// .data.initcall, that records all declared initcalls.  The
/// initcall_run_all() API simply runs all initcalls declared in
/// .data.initcall.

/// The structure representing an initcall

typedef struct {

    /// The initialization function
    ///
    /// Prior to running the initcall, this field is zeroed.  This guarantess
    /// that each initcall is run at most 1 time.
    void (*initcall)(void* arg);

    /// The argument to the initialization function
    void* arg;

} InitCall;


/// Declare an initcall
///
/// This macro generates C code and global data so must be placed at file
/// scope in a C file, not in a header file or inside a C function
/// body. Unless the initcall needs to be referenced by another initcall (to
/// guarantee ordering), this declaration will normally be prepended with
/// 'static'.
#define INITCALL(_var, _initcall, _arg)                                 \
    InitCall _var __attribute__ ((used, section (".data.initcall"))) = \
    {.initcall = _initcall, .arg = _arg};


/// Run the initcall represented by an InitCall structure, assuming it has not
/// already run.
///
/// \param[in] i_initcall The address of the initcall structure to run
void
initcall_run(InitCall* i_initcall);


/// Run all initcalls
void
initcall_run_all();

#endif // __INITCALL_H__
