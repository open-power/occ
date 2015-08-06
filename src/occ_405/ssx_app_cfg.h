/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/ssx_app_cfg.h $                                   */
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
#ifndef __SSX_APP_CFG_H__
#define __SSX_APP_CFG_H__
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ssx_app_cfg.h
/// \brief Application specific overrides go here.
///

#include "global_app_cfg.h"

/// Static configuration data for external interrupts:
///
/// IRQ#, TYPE, POLARITY, ENABLE
///
#define APPCFG_EXT_IRQS_CONFIG \
    OCCHW_IRQ_CHECK_STOP_PPC405         OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_EXTERNAL_TRAP             OCCHW_IRQ_TYPE_LEVEL    OCCHW_IRQ_POLARITY_HI       OCCHW_IRQ_MASKED \
    OCCHW_IRQ_OCC_TIMER0                OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_OCC_TIMER1                OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_IPI4_HI_PRIORITY          OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_PBAX_OCC_SEND             OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_PBAX_OCC_PUSH0            OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_PBAX_OCC_PUSH1            OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_PBA_BCDE_ATTN             OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_PBA_BCUE_ATTN             OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_STRM0_PULL                OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_STRM0_PUSH                OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_STRM1_PULL                OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_STRM1_PUSH                OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_STRM2_PULL                OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_STRM2_PUSH                OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_STRM3_PULL                OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_STRM3_PUSH                OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_IPI4_LO_PRIORITY          OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \

/// The Instance ID of the occ processor that this application is intended to run on
/// 0-3 -> GPE, 4 -> 405
#define APPCFG_OCC_INSTANCE_ID 4


// Common configuration groups for verification. Bypass time-consuming setup
// or setup done by procedures for simulation environments, and set up special
// I/O configurations for simulation environments.

#ifndef VERIFICATION
#define VERIFICATION 0
#endif

#ifndef OCC_UNIT_VERIFICATION
#define OCC_UNIT_VERIFICATION 0
#endif

#ifndef EPM_VERIFICATION
#define EPM_VERIFICATION 0
#endif

#ifndef VBU_VERIFICATION
#define VBU_VERIFICATION 0
#endif

#ifndef LAB_VALIDATION
#define LAB_VALIDATION 0
#endif

#if VERIFICATION || LAB_VALIDATION

#if !LAB_VALIDATION
#define SSX_STACK_CHECK    0
#endif

#define SIMICS_ENVIRONMENT 0

#else

#define INITIALIZE_PBA_SLAVES 1
#define INITIALIZE_PBA_BARS 1

#endif  /* VERIFICATION */


#if OCC_UNIT_VERIFICATION
#define USE_RTX_IO 1
#endif

#if EPM_VERIFICATION
#define USE_EPM_IO 1
#endif

// VBU and the lab use trace buffer I/O.  The DBCR0 is left untouched so VBU
// can use instruction/data breakpoints.

#if VBU_VERIFICATION || LAB_VALIDATION
#define USE_TRACE_IO 1
#define NO_INIT_DBCR0 1
#endif

// Default initializations for validation that affect SSX and library code

#ifndef SIMICS_ENVIRONMENT
#define SIMICS_ENVIRONMENT 1
#endif

#ifndef USE_SIMICS_IO
#define USE_SIMICS_IO 1
#endif

#ifndef USE_RTX_IO
#define USE_RTX_IO 0
#endif

#ifndef USE_TRACE_IO
#define USE_TRACE_IO 0
#endif

#ifndef USE_EPM_IO
#define USE_EPM_IO 0
#endif


/// The buffer used for 'ssxout' in VBU and lab applications
///
/// The buffer is defined to be quite large in order to accomodate full kernel
/// and application dumps in the event of problems.
#ifndef SSXOUT_TRACE_BUFFER_SIZE
#define SSXOUT_TRACE_BUFFER_SIZE (32 * 1024)
#endif

#ifndef APPCFG_USE_EXT_TIMEBASE_FOR_TRACE
#define APPCFG_USE_EXT_TIMEBASE_FOR_TRACE 1
#endif

#ifndef PPC405_TIMEBASE_HZ
#define PPC405_TIMEBASE_HZ  600000000
#endif

//If we are using the external timebase for traces, assume it is 37500000 Hz.
//Otherwise, it will use the PPC405 timebase.
#if APPCFG_USE_EXT_TIMEBASE_FOR_TRACE
#define SSX_TRACE_TIMEBASE_HZ   37500000
#else
#define SSX_TRACE_TIMEBASE_HZ PPC405_TIMEBASE_HZ
#endif /* APPCFG_USE_EXT_TIMEBASE_FOR_TRACE */

#define INIT_SEC_NM_STR     "initSection"
#define INIT_SECTION __attribute__ ((section (INIT_SEC_NM_STR)))

#endif /*__SSX_APP_CFG_H__*/
