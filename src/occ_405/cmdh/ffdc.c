/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cmdh/ffdc.c $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

#include "ffdc.h"

// Function Specification //////////////////////////////////////////////////////
//
// Name: ffdc_thread_dumper
//
// Description: Dump the state of the thread provided
//
// End Function Specification //////////////////////////////////////////////////
void ffdc_thread_dumper(SsxThread *i_thread, void *o_ffdc_buffer)
{
    // Format of data dumped into FFDC buffer.  The buffer space allocated must
    // match the size of the dump.  The dumped structure must agree with the
    // structure of the FFDC defined in ll_ffdc.S.
    //
    // Offset   Length  Contents
    // 0x00     1       Length of thread dump
    // 0x01     1       Priority
    // 0x02     1       State
    // 0x03     1       Flags
    // 0x04     4       Thread timer
    // 0x08     4       Semaphore
    // 0x0c     4       SRR0
    // 0x10     4       SRR1
    // 0x14     4       SRR2
    // 0x18     4       SRR3
    // 0x1c     4       LR
    // 0x20     32      Thread stack unwind
    //
    // Total length = 64 bytes

    uint8_t *l_byte_ptr = (uint8_t *)o_ffdc_buffer;
    // Store length, priority, state and flags to buffer
    l_byte_ptr[0] = 64;
    l_byte_ptr[1] = (uint8_t)(i_thread->priority);
    l_byte_ptr[2] = (uint8_t)(i_thread->state);
    l_byte_ptr[3] = (uint8_t)(i_thread->flags);

    // Store Timer, Semaphore
    uint32_t *l_word_ptr = (uint32_t *)(l_byte_ptr + 4);
    l_word_ptr[0] = (uint32_t)&(i_thread->timer);
    l_word_ptr[1] = (uint32_t)(i_thread->semaphore);

    // Store SRR0-3 and LR from saved context
    SsxThreadContext *l_threadCtx = (SsxThreadContext *)(i_thread->saved_stack_pointer);
    l_word_ptr[2] = (uint32_t)(l_threadCtx->srr0);
    l_word_ptr[3] = (uint32_t)(l_threadCtx->srr1);
    l_word_ptr[4] = (uint32_t)(l_threadCtx->srr2);
    l_word_ptr[5] = (uint32_t)(l_threadCtx->srr3);
    l_word_ptr[6] = (uint32_t)(l_threadCtx->lr);

    // Store up to 8 LRs from stack chain, set stack frame pointer to caller's
    // frame.
    uint32_t *l_sptr = (uint32_t *)(((uint32_t*)l_threadCtx->r1)[0]);
    // Reset the ffdc pointer
    l_word_ptr = &l_word_ptr[7];
    ffdc_stack_unwind(l_sptr, l_word_ptr, 8);
}
// End of ffdc_thread_dumper


// Function Specification //////////////////////////////////////////////////////
//
// Name: ffdc_stack_unwind
//
// Description: Unwind the link registers from the provided stack pointer.
//
// End Function Specification //////////////////////////////////////////////////
void
ffdc_stack_unwind(uint32_t *i_sptr, uint32_t *o_buffer, uint32_t i_frameCount)
{
    // Format of the data dumped to the output buffer.
    // The caller must provide at least one 32 bit word per frame requested by
    // frame count parameter.
    //
    // Offset   Length  Contents
    // 0x00     4       Link register for frame n
    // 0x04     4       Link register for frame n+1
    // ... do as many as frames as requested

    uint32_t *l_sptr = i_sptr;
    int i = 0;
    // Loop through the frames storing the LR words
    for (i = 0; i < i_frameCount; i++)
    {
        // Stop storing if we get to the last frame
        if (l_sptr == NULL)
        {
            // Zero out the remaining words in the buffer and break
            for (; i < i_frameCount; i++)
            {
                o_buffer[i] = 0x00000000;
            }
            break;
        }
        else
        {
            o_buffer[i] = (uint32_t)(l_sptr[1]);
            l_sptr = (uint32_t *)l_sptr[0];
        }
    }
}
// End of ffdc_stack_unwind
