# IBM_PROLOG_BEGIN_TAG
# This is an automatically generated prolog.
#
# $Source: src/ppe/pk/ppe42/ppe42_thread_init.S $
#
# OpenPOWER OnChipController Project
#
# Contributors Listed Below - COPYRIGHT 2015
# [+] International Business Machines Corp.
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.
#
# IBM_PROLOG_END_TAG
/// \file ppe42_thread_init.S
/// \brief PPE42-specific thread initialization
///
/// The entry points in this file are routines that are typically used during
/// initialization, and their code space could be deallocated and recovered if
/// no longer needed by the application after initialization.

        .nolist
#include "pk.h"
        .list
        
/// \fn void __pk_thread_context_initialize(PkThread *thread, PkThreadRoutine thread_routine, void *private)
/// \brief Create the initial thread context on the stack
///
/// The non-reserved GPRs are prepatterned with 0x0000\<rn\>\<rn\> where \<rn\> is
/// the register number (as decimal).  The initial context is set up with the
/// thread running in the default machine context, and when the thread is
/// switched in it will begin executing at the entry point of the thread
/// routine with the \c private parameter in R3.  The LR is initialized such
/// that when the thread returns, it will return to the entry point of \c
/// pk_complete().
#ifdef DOXYGEN_ONLY
void 
__pk_thread_context_initialize(PkThread        *thread, 
                                PkThreadRoutine thread_routine, 
                                void             *private);
#endif
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------
        
/// \cond

        .global_function __pk_thread_context_initialize

__pk_thread_context_initialize:

        ## R3 = thread (param)
        ## R4 = thread_routine (param)
        ## R5 = private (param)
        ## R6 = thread stack pointer (computed)
        ## R7 = scratch

        .macro  _gpr_init, prefix, reg, val
        li      %r7, \val
        stw     %r7, \prefix\reg(%r6)
        .endm

        ## Initialize volatile context on the thread stack. The CR is cleared,
        ## the LR = pk_complete(), R3 has the private parameter.

        lwz     %r6, PK_THREAD_OFFSET_SAVED_STACK_POINTER(%r3)
        
        stwu    %r6, -PK_CTX_SIZE(%r6)

        li      %r7, 0
        stw     %r7, PK_CTX_CR(%r6)

        _liw    %r7, pk_complete
        stw     %r7, PK_CTX_LR(%r6)

        stw     %r5, PK_CTX_GPR3(%r6)

        _gpr_init PK_CTX_GPR, 4, 0x0404
        _gpr_init PK_CTX_GPR, 5, 0x0505
        _gpr_init PK_CTX_GPR, 6, 0x0606

        ## XER and CTR are clear, SRR0 = thread_routine, SRR1 = default machine
        ## context.

        li      %r7, 0
        stw     %r7, PK_CTX_XER(%r6)
        stw     %r7, PK_CTX_CTR(%r6)

        stw     %r4, PK_CTX_SRR0(%r6)

        _lwzsd  %r7, __pk_thread_machine_context_default
        stw     %r7, PK_CTX_SRR1(%r6)

        _gpr_init PK_CTX_GPR, 0,  0x0000
        _gpr_init PK_CTX_GPR, 7,  0x0707
        _gpr_init PK_CTX_GPR, 8,  0x0808
        _gpr_init PK_CTX_GPR, 9,  0x0909
        _gpr_init PK_CTX_GPR, 10, 0x1010

        ## Initialize the non-volatile context on the thread stack.

        _gpr_init PK_CTX_GPR, 28, 0x2828       
        _gpr_init PK_CTX_GPR, 29, 0x2929       
        _gpr_init PK_CTX_GPR, 30, 0x3030       
        _gpr_init PK_CTX_GPR, 31, 0x3131

        ## Initialize the kernel context on the thread stack.
        ## Note: Thread priority is set later each time the thread is
        ##       resumed.

        lis     %r7, PPE42_THREAD_MODE
        stw     %r7, PK_CTX_KERNEL_CTX(%r6)

        ## Initialization is done - the stack pointer is stored back in the
        ## thread.

        stw     %r6, PK_THREAD_OFFSET_SAVED_STACK_POINTER(%r3)
        blr

        .epilogue __pk_thread_context_initialize

/// \endcond
