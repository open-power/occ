#ifndef __SSX_H__
#define __SSX_H__

// $Id: ssx.h,v 1.1.1.1 2013/12/11 21:03:27 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ssx/ssx.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ssx.h
/// \brief The combined header of the SSX kernel.
///
/// This header will be included in any C or assembler source file that
/// requires any of the SSX API.  All headers defined by SSX and co-compiled
/// code should be protected such that they can be included without error into
/// assembly.

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <stddef.h>
#endif  /* __ASSEMBLER__ */

#define __SSX__

/// The application environment specifies whether or not it will provide an
/// application configuration file, which must be named "ssx_app_cfg.h".

#ifndef USE_SSX_APP_CFG_H
#define USE_SSX_APP_CFG_H 0
#endif

#if USE_SSX_APP_CFG_H
#include "ssx_app_cfg.h"
#endif

#include "ssx_macros.h"
#include "ssx_api.h"
#include "ssx_port.h"
#include "ssx_kernel.h"
#include "ssx_io.h"

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
/// _checkstop(void* arg, SsxIrqId irq, int priority) 
/// {
///     SSX_PANIC(VALIDATION_CHECKSTOP);
/// }
/// 
/// SSX_IRQ_FAST2FULL(_validationCheckstopHandler, _checkstop);
///
/// \endcode
#define USED_ATTRIBUTE __attribute__ ((used))

#endif  /* __ASSEMBLER__ */

#endif /* __SSX_H__ */
