/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/iota/test/iota_app_cfg.h $                            */
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
#if !defined(__IOTA_APP_CFG__)
#define __IOTA_APP_CFG__

// GPE3 is the irq route ownder
#define OCCHW_IRQ_ROUTE_OWNER  3
// This is GPE2 - see iota.mk

// TODO  get rid of pk_app_cfg.h
//#include "pk_app_cfg.h"
#define PLATFORM_PANIC_CODES_H "iota_test_panic_codes.h"
#include "iota_panic_codes.h"

#include "ocb_register_addresses.h"

#define IOTA_NUM_EXT_IRQ_PRIORITIES 3
#define IOTA_MAX_NESTED_INTERRUPTS  5

//An "idle" task is one that only runs when the ppe42 engine would otherwise
//be idle and thus has the lowest priority and can be interrupted by anything.
//To enable IDLE task support in the kernel set this to 1. (OPT)
#define IOTA_IDLE_TASKS_ENABLE 0

//To automatically disable an "IDLE" task after executing, set this to 1. (OPT)
#define IOTA_AUTO_DISABLE_IDLE_TASKS  0

// Main "execution" stack size in bytes, must be multiple of 8
#define IOTA_EXECUTION_STACK_SIZE  2048

#define LOCAL_TIMEBASE_REGISTER OCB_OTBR

#define APPCFG_PANIC(code) IOTA_PANIC(code)
#define APPCFG_TRACE PK_TRACE

#define PPE_TIMEBASE_HZ 37500000

/// This application will statically initialize it's external interrupt table
/// using the table defined in pk_app_irq_table.c.
#define STATIC_IRQ_TABLE

/// Static configuration data for external interrupts:
///
/// IRQ#, TYPE, POLARITY, ENABLE
///
#define APPCFG_EXT_IRQS_CONFIG \
    OCCHW_IRQ_OCC_ERROR                   OCCHW_IRQ_TYPE_EDGE  OCCHW_IRQ_POLARITY_RISING  OCCHW_IRQ_MASKED \
    OCCHW_IRQ_GPE3_ERROR                  OCCHW_IRQ_TYPE_EDGE  OCCHW_IRQ_POLARITY_RISING  OCCHW_IRQ_MASKED \
    OCCHW_IRQ_CHECK_STOP_GPE2             OCCHW_IRQ_TYPE_EDGE  OCCHW_IRQ_POLARITY_RISING  OCCHW_IRQ_MASKED \
    OCCHW_IRQ_PVREF_ERROR                 OCCHW_IRQ_TYPE_EDGE  OCCHW_IRQ_POLARITY_RISING  OCCHW_IRQ_MASKED \
    OCCHW_IRQ_IPI2_HI_PRIORITY            OCCHW_IRQ_TYPE_EDGE  OCCHW_IRQ_POLARITY_RISING  OCCHW_IRQ_MASKED \
    OCCHW_IRQ_PBAX_PGPE_ATTN              OCCHW_IRQ_TYPE_EDGE  OCCHW_IRQ_POLARITY_RISING  OCCHW_IRQ_MASKED \
    OCCHW_IRQ_PBAX_PGPE_PUSH0             OCCHW_IRQ_TYPE_EDGE  OCCHW_IRQ_POLARITY_RISING  OCCHW_IRQ_MASKED \
    OCCHW_IRQ_PBAX_PGPE_PUSH1             OCCHW_IRQ_TYPE_EDGE  OCCHW_IRQ_POLARITY_RISING  OCCHW_IRQ_MASKED \
    OCCHW_IRQ_PMC_PCB_INTR_TYPE0_PENDING  OCCHW_IRQ_TYPE_LEVEL OCCHW_IRQ_POLARITY_HI      OCCHW_IRQ_MASKED \
    OCCHW_IRQ_PMC_PCB_INTR_TYPE1_PENDING  OCCHW_IRQ_TYPE_LEVEL OCCHW_IRQ_POLARITY_HI      OCCHW_IRQ_MASKED \
    OCCHW_IRQ_PMC_PCB_INTR_TYPE2_PENDING  OCCHW_IRQ_TYPE_LEVEL OCCHW_IRQ_POLARITY_HI      OCCHW_IRQ_MASKED \
    OCCHW_IRQ_PMC_PCB_INTR_TYPEA_PENDING  OCCHW_IRQ_TYPE_LEVEL OCCHW_IRQ_POLARITY_HI      OCCHW_IRQ_MASKED

#endif
