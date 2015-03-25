/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/thread/thread.h $                                     */
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

#ifndef _THREAD_H
#define _THREAD_H

#include <occ_common.h>
#include "ssx.h"

// Thread priorities for Thread creation.
typedef enum
{
    THREAD_PRIORITY_0, // Reserved for high priority
    THREAD_PRIORITY_1, // Reserved for high priority
    THREAD_PRIORITY_2,
    THREAD_PRIORITY_3,
    THREAD_PRIORITY_4,
    THREAD_PRIORITY_5,
    THREAD_PRIORITY_6,
    THREAD_PRIORITY_7,
    THREAD_PRIORITY_8,
}THREAD_PRIORITY;

// NOTE: Stack sizes are defined by entity
// - Non-Critical Stack used by non-critical interrupt handlers, including timer callbacks
// - Critical Stack used for critical interrupts
// - Stacks for each thread
#define NONCRITICAL_STACK_SIZE 8192    // 8kB
#define CRITICAL_STACK_SIZE    4096    // 4kB
#define THREAD_STACK_SIZE      4096    // 4kB

extern uint8_t main_thread_stack[THREAD_STACK_SIZE];
extern uint8_t Cmd_hndl_thread_stack[THREAD_STACK_SIZE];
extern uint8_t App_thread_stack[THREAD_STACK_SIZE];
extern uint8_t testAppletThreadStack[THREAD_STACK_SIZE];

/*----------------------------------------------------------*/
/* SsxThread Declaration                                    */
/*----------------------------------------------------------*/

// Our idle thread. See main_thread_routine
extern SsxThread Main_thread;

// Command handler thread
extern SsxThread Cmd_Hndl_thread;

// Application manager thread
extern SsxThread App_thread;

// Test applet thread
extern SsxThread TestAppletThread;

// Application manager thread
extern SsxThread Dcom_thread;

//WOF thread  @cl020
extern SsxThread amec_wof_thread;  

void Main_thread_routine(void *private);

void Cmd_Hndl_thread_routine(void *arg);

void App_thread_routine(void *arg);

void Dcom_thread_routine(void *arg);

void testAppletThreadRoutine(void *arg);

void amec_wof_thread_routine(void *arg);

#endif //_THREAD_H
