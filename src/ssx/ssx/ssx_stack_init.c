// $Id: ssx_stack_init.c,v 1.1.1.1 2013/12/11 21:03:28 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/ssx/ssx_stack_init.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ssx_stack_init.c
/// \brief SSX stack initialization
///
/// The entry points in this file are initialization routines - they are never
/// needed after SSX initialization and their code space could be reclaimed by
/// the application after initialization if required.
///
/// This code was split out from "ssx_init.c" because it may be needed in a
/// thread configuration if threads are being created dynamically. in an
/// interrupt-only configuration it is not needed after \c ssx_initialize().

#include "ssx.h"

/// Initialize a stack area.  
/// 
/// \param stack A pointer to the smallest legal address of the stack.  The
/// stack address is modified as the stack is aligned and initialized.
///
/// \param size A pointer to the size of the stack (in bytes).  The size is
/// modified as the stack is aligned and initialized.  At exit this is the
/// final usable stack area size aligned to the size of the SSX_STACK_TYPE.
///
/// SSX makes no assumptions about size or alignment of the area provided as a
/// stack, and carefully aligns and initializes the stack.  Regardless of how
/// the stack grows, the \a stack parameter is considered to be the lowest
/// legal address of the stack.  

int
__ssx_stack_init(SsxAddress *stack,
                 size_t     *size)
{
    SsxAddress mask;
    size_t excess, i, count;
    SSX_STACK_TYPE *p;

    if (SSX_STACK_DIRECTION < 0) {

        // Stacks grow down.  The initial stack pointer is set to just above
        // the last allocated stack address.  This is legal for pre-decrement
        // stacks, otherwise the initial address is first brought into range
        // before alignment.  The stack is aligned downward, then the size is
        // adjusted to a multiple of the stack type.  Stacks are optionally
        // prepatterned. Alignment is assumed to be a power of 2.

        *stack += *size;

        if (!SSX_STACK_PRE_DECREMENT) {
            *stack -= sizeof(SSX_STACK_TYPE);
            *size -= sizeof(SSX_STACK_TYPE);
        }

        mask = SSX_STACK_ALIGNMENT - 1;
        excess = *stack & mask;
        *stack -= excess;
        *size -= excess;
        *size = (*size / sizeof(SSX_STACK_TYPE)) * sizeof(SSX_STACK_TYPE);

        if (SSX_STACK_CHECK) {
            p = (SSX_STACK_TYPE *)(*stack);
            count = *size / sizeof(SSX_STACK_TYPE);
            for (i = 0; i < count; i++) {
                if (SSX_STACK_PRE_DECREMENT) {
                    *(--p) = SSX_STACK_PATTERN;
                } else {
                    *(p--) = SSX_STACK_PATTERN;
                }
            }
        }

        __ssx_stack_create_initial_frame(stack, size);

    } else {

        SSX_PANIC(SSX_UNIMPLEMENTED);
    }

    return SSX_OK;
}

