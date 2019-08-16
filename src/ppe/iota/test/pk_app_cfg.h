/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/iota/test/pk_app_cfg.h $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2019                             */
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
#ifndef __PK_APP_CFG_H__
#define __PK_APP_CFG_H__

/// \file pk_app_cfg.h
/// \brief Application specific overrides go here.
///

// Debug Switches

//NOTE this is currently broken:
//need iota/fit resolution to restore function
//#define TEST_ONLY_BCE_IRR                0

// --------------------

// Hyp debug support,
#define DISABLE_CORE_XSTOP_INJECTION     0

// --------------------

//override switch to disable stop8 with backup resonant clock support
#define DISABLE_STOP8                    1

// --------------------

// override switch to disable dual cast,
// otherwise automatically be disabled if NDD == 2
// being enabled on all other dd levels unless this override
#define DISABLE_CME_DUAL_CAST            0

// NDD23 Secure Memory Support: RAM URMOR
#if NIMBUS_DD_LEVEL >= 23 || CUMULUS_DD_LEVEL >= 13

    #define SMF_SUPPORT_ENABLE 1

#else

    #define SMF_SUPPORT_ENABLE 0

#endif

#if NIMBUS_DD_LEVEL == 20 || DISABLE_CME_DUAL_CAST == 1
    // NDD2: no catchup due to dual cast bug
    #undef  SKIP_ENTRY_CATCHUP
    #undef  SKIP_EXIT_CATCHUP
    #define SKIP_ENTRY_CATCHUP 1
    #define SKIP_EXIT_CATCHUP  1
#endif

// --------------------

// override swtich for NDD20/21/CDD10 workaround
#define DISABLE_PERIODIC_CORE_QUIESCE    1
#define DISABLE_CME_FIT_TIMER            0

#if !DISABLE_PERIODIC_CORE_QUIESCE
    #define DISABLE_CME_FIT_TIMER        0
#endif

#define ENABLE_CME_WATCHDOG_TIMER        0
// --------------------
#define ENABLE_CME_DEC_TIMER             0
// --------------------

// the following functions are always enabled once working
// (unless earlier DD level condition undef them below)

#define USE_PPE_IMPRECISE_MODE
#define USE_CME_QUEUED_SCOM
#define USE_CME_QUEUED_SCAN
#define USE_CME_VDM_FEATURE
#define USE_CME_RESCLK_FEATURE

#if !defined(USE_PPE_IMPRECISE_MODE) && (defined(USE_CME_QUEUED_SCOM) || defined(USE_CME_QUEUED_SCAN))
    #error "USE_PPE_IMPRECISE_MODE must be defined in order to enable USE_CME_QUEUED_SCOM or USE_CME_QUEUED_SCAN"
#endif

// --------------------

#if NIMBUS_DD_LEVEL == 10
    #define HW386841_NDD1_DSL_STOP1_FIX         1
    #define HW402407_NDD1_TLBIE_STOP_WORKAROUND 1
    #define HW405292_NDD1_PCBMUX_SAVIOR         1
    #define RUN_NDD1_ABIST_IN_PARALLEL_MODE     1
    #define MASK_MSR_SEM6
    #undef  USE_CME_VDM_FEATURE
    #undef  USE_CME_RESCLK_FEATURE
#endif

// --------------------

#if EPM_P9_TUNING
    // EPM use broadside RTX instead of BCE
    #undef  SKIP_BCE_SCAN_RING
    #define SKIP_BCE_SCAN_RING 1

    #undef  SKIP_BCE_SCOM_RESTORE
    #define SKIP_BCE_SCOM_RESTORE 1

    #undef  LAB_P9_TUNING
    #define LAB_P9_TUNING 0

    #define PK_TRACE_BUFFER_WRAP_MARKER 1
    #define __FAPI_DELAY_SIM__
#endif

// --------------------

#if PK_TRACE_LEVEL == 0   /*No TRACEs*/
    #define PK_TRACE_ENABLE        0
    #define PK_KERNEL_TRACE_ENABLE 0
#elif PK_TRACE_LEVEL == 1 /*only PK_TRACE_ERR+INF*/
    #define PK_TRACE_ENABLE        1
    #define PK_TRACE_DBG_SUPPRESS  1
    #define PK_TRACE_CTRL_ENABLE   1
    #define PK_TRACE_CRIT_ENABLE   1
    #define PK_TRACE_CKPT_ENABLE   0
    #define PK_KERNEL_TRACE_ENABLE 0
#elif PK_TRACE_LEVEL == 2 /*only PK_TRACE_ERR+INF+DBG+KERNEL*/
    #define PK_TRACE_ENABLE        1
    #define PK_TRACE_DBG_SUPPRESS  1
    #define PK_TRACE_CTRL_ENABLE   1
    #define PK_TRACE_CRIT_ENABLE   1
    #define PK_TRACE_CKPT_ENABLE   1
    #define PK_KERNEL_TRACE_ENABLE 1
#else                    /*All TRACEs*/
    #define PK_TRACE_ENABLE        1
    #define PK_TRACE_DBG_SUPPRESS  0
    #define PK_TRACE_CTRL_ENABLE   1
    #define PK_TRACE_CRIT_ENABLE   1
    #define PK_TRACE_CKPT_ENABLE   1
    #define PK_KERNEL_TRACE_ENABLE 1
#endif

// --------------------

// The actual timebase is provided in an ipl time attribute.
// If the attribute is not set up then PPE_TIMEBASE_HZ is used as a default.
// If we are using the external timebase register, then assume a nominal
// frequency of nest_freq/64  (2GHz/64).
// If APPCFG_USE_EXT_TIMEBASE is not defined then use the internal
// decrementer as a timebase.

#ifdef APPCFG_USE_EXT_TIMEBASE
    // Nest freq / 64 - use a nominal 2GH as the default
    #define PPE_TIMEBASE_HZ 31250000
#else
    #define PPE_TIMEBASE_HZ 600000000
#endif

// --------------------

/// This file provides architecture-specific symbol names for each interrupt
//#include "cmehw_interrupts.h"


#endif /*__PK_APP_CFG_H__*/
