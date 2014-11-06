#ifndef __SSX_MACROS_H__
#define __SSX_MACROS_H__

// $Id: ssx_macros.h,v 1.2 2014/02/03 01:30:44 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ssx/ssx_macros.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ssx_macros.h
/// \brief Boilerplate macros for SSX

/// This macro encapsulates error handling boilerplate for code that uses the
/// SSX API-type error handling, for errors that do not occur in critical
/// sections.

#define SSX_ERROR(code) \
    do { \
        if (SSX_ERROR_PANIC) {                  \
            SSX_PANIC(code);                    \
        } else {                                \
            return -(code);                     \
        }                                       \
    } while (0)


/// This macro encapsulates error handling boilerplate in the SSX API
/// functions, for errors that do not occur in critical sections.

#define SSX_ERROR_IF(condition, code) \
    do { \
        if (condition) {                        \
            SSX_ERROR(code);                    \
        }                                       \
    } while (0)


/// This macro encapsulates error handling boilerplate in the SSX API
/// functions, for errors that do not occur in critical sections and always
/// force a kernel panic, indicating a kernel or API bug.

#define SSX_PANIC_IF(condition, code)           \
    do {                                        \
        if (condition) {                        \
            SSX_PANIC(code);                    \
        }                                       \
    } while (0)


/// This macro encapsulates error handling boilerplate in the SSX API
/// functions, for errors that do not occur in critical sections.
/// The error handling will only be enabled when SSX_ERROR_CHECK_API 
/// is enabled.

#define SSX_ERROR_IF_CHECK_API(condition, code) \
    do { \
        if (SSX_ERROR_CHECK_API) {              \
            SSX_ERROR_IF(condition, code);       \
        }                                       \
    } while (0)

/// This macro encapsulates error handling boilerplate in the SSX API
/// functions, for errors that occur in critical sections.

#define SSX_ERROR_IF_CRITICAL(condition, code, context) \
    do { \
        if (condition) {                                \
            if (SSX_ERROR_PANIC) {                      \
                SSX_PANIC(code);                        \
                ssx_critical_section_exit(context);     \
            } else {                                    \
                ssx_critical_section_exit(context);     \
                return -(code);                         \
            }                                           \
        }                                               \
    } while (0)


/// This is a general macro for errors that require cleanup before returning
/// the error code.

#define SSX_ERROR_IF_CLEANUP(condition, code, cleanup) \
    do { \
        if (condition) {                        \
            if (SSX_ERROR_PANIC) {              \
                SSX_PANIC(code);                \
                cleanup;                        \
            } else {                            \
                cleanup;                        \
                return -(code);                 \
            }                                   \
        }                                       \
    } while (0)


/// Most SSX APIs can not be called from critical interrupt contexts.

#define SSX_ERROR_IF_CRITICAL_INTERRUPT_CONTEXT() \
    SSX_ERROR_IF(__ssx_kernel_context_critical_interrupt(), \
                 SSX_ILLEGAL_CONTEXT_CRITICAL_INTERRUPT)


/// Some SSX APIs can only be called from thread contexts - these are APIs
/// that threads call on 'themselves'.

#define SSX_ERROR_UNLESS_THREAD_CONTEXT() \
    SSX_ERROR_IF(!__ssx_kernel_context_thread(), \
                 SSX_ILLEGAL_CONTEXT_THREAD_CONTEXT)


/// Some SSX APIs must be called from an interrupt context only.

#define SSX_ERROR_UNLESS_ANY_INTERRUPT_CONTEXT() \
    SSX_ERROR_IF(!__ssx_kernel_context_any_interrupt(), \
                 SSX_ILLEGAL_CONTEXT_INTERRUPT_CONTEXT)

#endif /* __SSX_MACROS_H__ */
