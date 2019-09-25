/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/test/pk_app_cfg.h $                                */
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

#define GLOBAL_CFG_USE_IPC 1

// All GPE's will use the external timebase register
#define APPCFG_USE_EXT_TIMEBASE

// If we are using the OCB timebase then assume
// a frequency of 37.5Mhz.  Otherwise, the default is to use
// the decrementer as a timebase and assume a frequency of
// 600MHz
// In product code, this value will be IPL-time configurable.
#ifdef APPCFG_USE_EXT_TIMEBASE
    #define PPE_TIMEBASE_HZ 37500000
#else
    #define PPE_TIMEBASE_HZ 600000000
#endif /* APPCFG_USE_EXT_TIMEBASE */

//#define SIMICS_ENVIRONMENT 1
/*
#define OCCHW_IRQ_ROUTES \
    OCCHW_IRQ_DEBUGGER                      OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_TRACE_TRIGGER                 OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_OCC_ERROR                     OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_PBA_ERROR                     OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_SRT_ERROR                     OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_GPE0_HALT                     OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_GPE1_HALT                     OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_GPE2_HALT                     OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_GPE3_HALT                     OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_PPC405_HALT                   OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_OCB_ERROR                     OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_SPIPSS_ERROR                  OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_CHECK_STOP_PPC405             OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_CHECK_STOP_GPE0               OCCHW_IRQ_TARGET_ID_GPE0 \
    OCCHW_IRQ_CHECK_STOP_GPE1               OCCHW_IRQ_TARGET_ID_GPE1 \
    OCCHW_IRQ_CHECK_STOP_GPE2               OCCHW_IRQ_TARGET_ID_GPE2 \
    OCCHW_IRQ_CHECK_STOP_GPE3               OCCHW_IRQ_TARGET_ID_GPE3 \
    OCCHW_IRQ_OCC_MALF_ALERT                OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_ADU_MALF_ALERT                OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_EXTERNAL_TRAP                 OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_IVRM_PVREF_ERROR              OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_OCC_TIMER0                    OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_OCC_TIMER1                    OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_HALT_PSTATES                  OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_IPI_SCOM                      OCCHW_IRQ_TARGET_ID_GPE0 \
    OCCHW_IRQ_IPI0_HI_PRIORITY              OCCHW_IRQ_TARGET_ID_GPE0 \
    OCCHW_IRQ_IPI1_HI_PRIORITY              OCCHW_IRQ_TARGET_ID_GPE1 \
    OCCHW_IRQ_IPI2_HI_PRIORITY              OCCHW_IRQ_TARGET_ID_GPE2 \
    OCCHW_IRQ_IPI3_HI_PRIORITY              OCCHW_IRQ_TARGET_ID_GPE3 \
    OCCHW_IRQ_IPI4_HI_PRIORITY              OCCHW_IRQ_TARGET_ID_405_CRIT \
    OCCHW_IRQ_ADCFSM_ONGOING                OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_RESERVED_31                   OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_PBAX_OCC_SEND                 OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_PBAX_OCC_PUSH0                OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_PBAX_OCC_PUSH1                OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_PBA_BCDE_ATTN                 OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_PBA_BCUE_ATTN                 OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_STRM0_PULL                    OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_STRM0_PUSH                    OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_STRM1_PULL                    OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_STRM1_PUSH                    OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_STRM2_PULL                    OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_STRM2_PUSH                    OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_STRM3_PULL                    OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_STRM3_PUSH                    OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_PMC_PCB_INTR_TYPE0_PENDING    OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_PMC_PCB_INTR_TYPE1_PENDING    OCCHW_IRQ_TARGET_ID_GPE2 \
    OCCHW_IRQ_PMC_PCB_INTR_TYPE2_PENDING    OCCHW_IRQ_TARGET_ID_GPE3 \
    OCCHW_IRQ_PMC_PCB_INTR_TYPE3_PENDING    OCCHW_IRQ_TARGET_ID_GPE3 \
    OCCHW_IRQ_PMC_PCB_INTR_TYPE4_PENDING    OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_PMC_PCB_INTR_TYPE5_PENDING    OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_PMC_PCB_INTR_TYPE6_PENDING    OCCHW_IRQ_TARGET_ID_GPE3 \
    OCCHW_IRQ_PMC_PCB_INTR_TYPE7_PENDING    OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_PMC_O2S_0A_ONGOING            OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_PMC_O2S_0B_ONGOING            OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_PMC_O2S_1A_ONGOING            OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_PMC_O2S_1B_ONGOING            OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_PSSBRIDGE_ONGOING             OCCHW_IRQ_TARGET_ID_NONE \
    OCCHW_IRQ_IPI0_LO_PRIORITY              OCCHW_IRQ_TARGET_ID_GPE0 \
    OCCHW_IRQ_IPI1_LO_PRIORITY              OCCHW_IRQ_TARGET_ID_GPE1 \
    OCCHW_IRQ_IPI2_LO_PRIORITY              OCCHW_IRQ_TARGET_ID_GPE2 \
    OCCHW_IRQ_IPI3_LO_PRIORITY              OCCHW_IRQ_TARGET_ID_GPE3 \
    OCCHW_IRQ_IPI4_LO_PRIORITY              OCCHW_IRQ_TARGET_ID_405_NONCRIT \
    OCCHW_IRQ_RESERVED_63                   OCCHW_IRQ_TARGET_ID_NONE
*/

/// Static configuration data for external interrupts:
///
/// IRQ#, TYPE, POLARITY, ENABLE
///
#define APPCFG_EXT_IRQS_CONFIG \
    OCCHW_IRQ_IPI_SCOM                  OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_IPI1_HI_PRIORITY          OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_IPI1_LO_PRIORITY          OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \

// This application will statically initialize it's external interrupt table
#define STATIC_IRQ_TABLE

#define STATIC_IPC_TABLES
#endif /*__PK_APP_CFG_H__*/
