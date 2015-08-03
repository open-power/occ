/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/kernel/pk.h $                                      */
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
#ifndef __PK_H__
#define __PK_H__
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pk.h
/// \brief The combined header of the PK kernel.
///
/// This header will be included in any C or assembler source file that
/// requires any of the PK API.  All headers defined by PK and co-compiled
/// code should be protected such that they can be included without error into
/// assembly.

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <stddef.h>
#endif  /* __ASSEMBLER__ */

#ifndef __PK__
#define __PK__ 1
#endif

/// The application environment specifies whether or not it will provide an
/// application configuration file, which must be named "pk_app_cfg.h".

#ifndef USE_PK_APP_CFG_H
#define USE_PK_APP_CFG_H 0
#endif

#if USE_PK_APP_CFG_H
#include "pk_app_cfg.h"
#endif

#include "pk_macros.h"
#include "pk_api.h"
#include "pk_port.h"
#include "pk_kernel.h"
//#include "pk_io.h"

#ifndef __ASSEMBLER__

#define MIN(X, Y)                               \
    ({                                          \
        typeof (X) __x = (X);                   \
        typeof (Y) __y = (Y);                   \
        (__x < __y) ? __x : __y; })

#define MAX(X, Y)                               \
    ({                                          \
        typeof (X) __x = (X);                   \
        typeof (Y) __y = (Y);                   \
        (__x > __y) ? __x : __y;                \
    })

/// \todo These don't require 32/64 bit versions, can always promote 32->64.

#define FLOOR_LOG2_32(x) (32 - 1 - cntlz32(x))
#define FLOOR_LOG2_64(x) (64 - 1 - cntlz64(x))

#define CEILING_LOG2(x)                         \
    ({                                          \
        uint64_t __x = (uint64_t)(x);           \
        int __y;                                \
        __y = FLOOR_LOG2_64(__x);               \
        if ((__x & (__x - 1)) != 0) {           \
            __y++;                              \
        }                                       \
        __y;})
    

#define POW2_32(x) ((uint32_t)1 << (x))
#define POW2_64(x) ((uint64_t)1 << (x))

/// Cast a pointer to another type
///
/// This macro is necessary when casting a pointer to a longer integer type.
/// The pointer is first cast to the equivalent integer type 'unsigned long',
/// then cast to the final type. You can also use this to cast integers longer
/// than pointers back to pointers.

#define CAST_POINTER(t, p) ((t)((unsigned long)(p)))


/// Create an alignment attribute.
#define ALIGNED_ATTRIBUTE(alignment) __attribute__ ((aligned (alignment)))

/// Create a specific-section attribute
///
/// Note that the section \a s here must be a string. Also note that data
/// specified to reside in specific sections must always be
/// initialized. Otherwise it confuses the linker which wants to put
/// uninitialized data into .bss sections.
///
/// \code 
///
/// int foo     SECTION_ATTRIBUTE(".noncacheable") = 0;
/// int bar[10] SECTION_ATTRIBUTE(".noncacheable") = {0};
///
/// \endcode
#define SECTION_ATTRIBUTE(s) __attribute__ ((section (s)))

/// Create a 'used' attribute
///
/// This is required for example to avoid "function unused" warnings when a
/// function is declared static but only referenced by inline assembler:
///
/// \code 
/// 
/// static USED_ATTRIBUTE void
/// _checkstop(void* arg, PkIrqId irq, int priority) 
/// {
///     PK_PANIC(VALIDATION_CHECKSTOP);
/// }
/// 
/// PK_IRQ_FAST2FULL(_validationCheckstopHandler, _checkstop);
///
/// \endcode
#define USED_ATTRIBUTE __attribute__ ((used))

#endif  /* __ASSEMBLER__ */

#endif /* __PK_H__ */
