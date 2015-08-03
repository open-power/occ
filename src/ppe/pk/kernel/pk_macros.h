/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/kernel/pk_macros.h $                               */
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
#ifndef __PK_MACROS_H__
#define __PK_MACROS_H__
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pk_macros.h
/// \brief Boilerplate macros for PK

/// This macro encapsulates error handling boilerplate for code that uses the
/// PK API-type error handling, for errors that do not occur in critical
/// sections.

#define PK_ERROR(code) \
    do { \
        if (PK_ERROR_PANIC) {                  \
            PK_PANIC(code);                    \
        } else {                                \
            return -(code);                     \
        }                                       \
    } while (0)


/// This macro encapsulates error handling boilerplate in the PK API
/// functions, for errors that do not occur in critical sections.

#define PK_ERROR_IF(condition, code) \
    do { \
        if (condition) {                        \
            PK_ERROR(code);                    \
        }                                       \
    } while (0)


/// This macro encapsulates error handling boilerplate in the PK API
/// functions, for errors that do not occur in critical sections and always
/// force a kernel panic, indicating a kernel or API bug.

#define PK_PANIC_IF(condition, code)           \
    do {                                        \
        if (condition) {                        \
            PK_PANIC(code);                    \
        }                                       \
    } while (0)


/// This macro encapsulates error handling boilerplate in the PK API
/// functions, for errors that do not occur in critical sections.
/// The error handling will only be enabled when PK_ERROR_CHECK_API 
/// is enabled.

#define PK_ERROR_IF_CHECK_API(condition, code) \
    do { \
        if (PK_ERROR_CHECK_API) {              \
            PK_ERROR_IF(condition, code);       \
        }                                       \
    } while (0)

/// This macro encapsulates error handling boilerplate in the PK API
/// functions, for errors that occur in critical sections.

#define PK_ERROR_IF_CRITICAL(condition, code, context) \
    do { \
        if (condition) {                                \
            if (PK_ERROR_PANIC) {                      \
                PK_PANIC(code);                        \
                pk_critical_section_exit(context);     \
            } else {                                    \
                pk_critical_section_exit(context);     \
                return -(code);                         \
            }                                           \
        }                                               \
    } while (0)


/// This is a general macro for errors that require cleanup before returning
/// the error code.

#define PK_ERROR_IF_CLEANUP(condition, code, cleanup) \
    do { \
        if (condition) {                        \
            if (PK_ERROR_PANIC) {              \
                PK_PANIC(code);                \
                cleanup;                        \
            } else {                            \
                cleanup;                        \
                return -(code);                 \
            }                                   \
        }                                       \
    } while (0)



/// Some PK APIs can only be called from thread contexts - these are APIs
/// that threads call on 'themselves'.

#define PK_ERROR_UNLESS_THREAD_CONTEXT() \
    PK_ERROR_IF(!__pk_kernel_context_thread(), \
                 PK_ILLEGAL_CONTEXT_THREAD_CONTEXT)


/// Some PK APIs must be called from an interrupt context only.

#define PK_ERROR_UNLESS_ANY_INTERRUPT_CONTEXT() \
    PK_ERROR_IF(!__pk_kernel_context_any_interrupt(), \
                 PK_ILLEGAL_CONTEXT_INTERRUPT_CONTEXT)

#endif /* __PK_MACROS_H__ */
