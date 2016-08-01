/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/ssx_dump.c $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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

/// \file ssx_dump.c
/// \brief Routines for dumping SSX kernel data structures
///
/// \note This is a quick hack to help solve a P8 bringup issue.  This code is
/// PPC405 (OCC) specific, i.e., not "portable" SSX. Ideally this type of dump
/// would be implemented in an external debugging tool as well, however it's
/// simplest to implement here in the execution context.

#include "ssx_dump.h"

#define SSX_DUMP_UNIMPLEMENTED 0x00d86701

static const char* _threadState[] = {
    0,
    "Suspended Runnable",
    "Mapped",
    "Suspended Blocked",
    "Completed",
    "Deleted",
};

static const char* _threadFlags[] = {
    0,
    "Semaphore Pend", 
    "Timer Pend",
    "Timer Pend | Semaphore Pend",
    "Timed Out",
    "Timed Out | Semaphore Pend", 
    "Timed Out | Timer Pend",
    "Timed Out | Timer Pend | Semaphore Pend",
};


static void
_dumpTimer(FILE* stream, SsxTimer* timer)
{
    fprintf(stream, 
            "-- Timer @ %p\n"
            "--     Deque.previous = %p\n"
            "--     Deque.next     = %p\n"
            "--     Timeout        = 0x%016llx\n"
            "--     Period         = 0x%016llx\n"
            "--     Callback       = %p\n"
            "--     Arg            = %p\n"
            "--     Options        = 0x%02x\n",
            timer,
            timer->deque.previous,
            timer->deque.next,
            timer->timeout,
            (unsigned long long)(timer->period),
            timer->callback,
            timer->arg,
            timer->options);
}


static void
_dumpThread(FILE* stream, SsxThread* thread)
{
    SsxThreadContext*        threadCtx;
    SsxThreadContextFullIrq* threadCtxIrq;
    uint32_t srr[4], lr, sp;

    fprintf(stream, 
            "-- Thread mapped at priority %d (%p)\n"
            "--     Thread state        = %s (%d)\n"
            "--     Thread flags        = %s (0x%02x)\n"
            "--     Saved Stack Pointer = %p\n",
            thread->priority, thread,
            _threadState[thread->state], thread->state,
            _threadFlags[thread->flags], thread->flags,
            (void*)thread->saved_stack_pointer);

    if (thread->flags & SSX_THREAD_FLAG_SEMAPHORE_PEND) {

        fprintf(stream, 
                "--     Semaphore           = %p\n",
                (void*)thread->semaphore);
    }
    fprintf(stream, 
            "---------------------------------------------\n");
    
    if (thread->flags & SSX_THREAD_FLAG_TIMER_PEND) {

        _dumpTimer(stream, &(thread->timer));
        fprintf(stream, 
                "---------------------------------------------\n");
    }

    if ((thread == ssx_current()) && !__ssx_kernel_context_any_interrupt()) {

        fprintf(stream,
                "--    This thread is executing ssx_dump()\n");

    } else {

        if (thread == ssx_current()) {

            // This is the interrupted thread, and only has its volatile
            // context saved.  The thread stack pointer is stored in a global
            // kernel variable.

            if (__ssx_kernel_context_critical_interrupt()) {

                SSX_PANIC(SSX_DUMP_UNIMPLEMENTED);
                srr[0] = srr[1] = srr[2] = srr[3] = lr = sp = 0; /* For GCC */

            } else {

                threadCtxIrq = 
                    (SsxThreadContextFullIrq*)__ssx_saved_sp_noncritical;
                srr[0] = threadCtxIrq->srr0;
                srr[1] = threadCtxIrq->srr1;
                srr[2] = threadCtxIrq->srr2;
                srr[3] = threadCtxIrq->srr3;
                lr = threadCtxIrq->lr;
                sp = threadCtxIrq->r1;
            }
        } else {

            // This is a fully swapped-out thread.  The context is saved in
            // at the stored stack pointer.

            threadCtx = (SsxThreadContext*)(thread->saved_stack_pointer); 
            srr[0] = threadCtx->srr0;
            srr[1] = threadCtx->srr1;
            srr[2] = threadCtx->srr2;
            srr[3] = threadCtx->srr3;
            lr = threadCtx->lr;
            sp = ((uint32_t*)threadCtx->r1)[0];
        }

        fprintf(stream,
                "--    SRR0: 0x%08x  SRR1: 0x%08x  "
                "SRR2: 0x%08x  SRR3: 0x%08x\n"
                "--      LR: 0x%08x\n",
                srr[0], srr[1], srr[2], srr[3],
                lr);

        fprintf(stream, 
                "---------------------------------------------\n");

        // Unwind the stack

        while (sp != 0) {

            fprintf(stream, 
                    "--      SP: 0x%08x  *LR*:0x%08x\n",
                    sp, ((uint32_t*)sp)[1]);
            sp = ((uint32_t*)sp)[0];
        }
    }
}




void
ssx_dump(FILE* stream, int options)
{
    int i, sep;
    SsxThread* thread;

    fprintf(stream, 
            "------------------------------------------------------------\n");
    fprintf(stream,
            "-- SSX Kernel Dump @ 0x%016llx\n"
            "--          USPRG0 = 0x%08x\n"
            "-- __ssx_run_queue = 0x%08x\n",
            ssx_timebase_get(), 
            mfspr(SPRN_USPRG0),
            __ssx_run_queue);
    fprintf(stream, 
            "------------------------------------------------------------\n");

    sep = 0;

    for (i = 0; i < SSX_THREADS; i++) {

        ssx_thread_at_priority(i, &thread);
        if (thread) {
            if (sep) {
                fprintf(stream, 
                        "*********************************************\n");
            }
            _dumpThread(stream, thread);
            sep = 1;
        }
    }

    fprintf(stream, 
            "------------------------------------------------------------\n");
}

    
        

            

            

