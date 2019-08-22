/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/common/global_app_cfg.h $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2019                        */
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
#define GLOBAL_CFG_USE_IPC  // have lib/occlib/ipc_structs.h use "ipc_func_ids.h"

// Define IRQ Owner (4=405, 2=PGPE, 1=GPE1, 0=GPE0)
#ifdef P10_HW_BUILD
#define OCCHW_IRQ_ROUTE_OWNER 2 // PGPE
#else
#define OCCHW_IRQ_ROUTE_OWNER 4 // 405 will own until PGPE running in Simics
#endif

#ifdef P10_HW_BUILD
// Physical HW will use the external timebase register
#define APPCFG_USE_EXT_TIMEBASE
#endif
// Simics will use internal timebase (external not implemented)

#define DEFAULT_NEST_FREQ_HZ 600000000
#define DEFAULT_EXT_CLK_FREQ_HZ 37500000

// Turn off periodic GPE traces
#define PK_TRACE_TIMER_OUTPUT 0

// Increase size of GPE trace buffers
#define PK_TRACE_SZ 1024

// Redefine the default MSR to mask off SIB errors and avoid data machine checks
// These SIB errors probably occur due to contention on the PIB
#define PK_THREAD_MACHINE_CONTEXT_DEFAULT (MSR_SEM | MSR_UIE | MSR_EE | MSR_ME)
#define PPE42_MSR_INITIAL (MSR_SEM | MSR_ME | MSR_UIE )

// Enable GPE IPC Timers
#define GPE_IPC_TIMERS

// If we are using the OCB timebase then assume
// a frequency of 37.5Mhz.  Otherwise, the default is to use
// the decrementer as a timebase and assume a frequency of
// 600MHz
// In product code, this value will be IPL-time configurable.
#ifdef APPCFG_USE_EXT_TIMEBASE
#define PPE_TIMEBASE_HZ DEFAULT_EXT_CLK_FREQ_HZ
#else
#define PPE_TIMEBASE_HZ DEFAULT_NEST_FREQ_HZ
#endif /* APPCFG_USE_EXT_TIMEBASE */


