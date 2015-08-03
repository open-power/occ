/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/thread/threadSch.h $                              */
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

#ifndef _THREADSCH_H
#define _THREADSCH_H

#include <occ_common.h>
#include <thread.h>
#include "ssx.h"
#include <errl.h>

// Function to reprioritize the threads in the array
void threadSwapcallback(void * arg);

// Function to initilize the threads and the thread schedule timer
void initThreadScheduler(void) INIT_SECTION;

// Function to create and resume thread. Externalizing as it is used to
// create main thread
int createAndResumeThreadHelper(SsxThread         *io_thread,
                                SsxThreadRoutine  i_thread_routine,
                                void              *io_arg,
                                SsxAddress        i_stack,
                                size_t            i_stack_size,
                                THREAD_PRIORITY   i_priority);

#endif //_THREADSCH_H
