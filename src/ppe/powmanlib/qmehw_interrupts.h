/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/powmanlib/qmehw_interrupts.h $                        */
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
#ifndef __QMEHW_INTERRUPTS_H__
#define __QMEHW_INTERRUPTS_H__


////////////////////////////////////////////////////////////////////////////
// IRQ Definition
////////////////////////////////////////////////////////////////////////////

// The QME interrupt controller consists of 1 x 64-bit controller.
//
// All 64 interrupts are or'd together and presented to the PPE as a single
// external interrupt exception.

#define QMEHW_IRQ_DEBUGGER                             0
#define QMEHW_IRQ_DEBUG_TRIGGER                        1
#define QMEHW_IRQ_QUAD_CHECKSTOP                       2
#define QMEHW_IRQ_LFIR_INDICATION                      3
#define QMEHW_IRQ_SPARE_4                              4
#define QMEHW_IRQ_SPARE_5                              5
#define QMEHW_IRQ_SPARE_6                              6
#define QMEHW_IRQ_SPARE_7                              7
#define QMEHW_IRQ_BCE_BUSY                             8
#define QMEHW_IRQ_RS4_BUSY                             9
#define QMEHW_IRQ_RESCLK_TARGET_ASSIST                10
#define QMEHW_IRQ_RESCLK_DONE_ASSIST                  11
#define QMEHW_IRQ_SPARE_12                            12
#define QMEHW_IRQ_SPARE_13                            13
#define QMEHW_IRQ_TBR_CYCLES_ROLLED                   14
#define QMEHW_IRQ_TBR_TIMEBASE_ROLLED                 15
#define QMEHW_IRQ_DOORBELL_0                          16
#define QMEHW_IRQ_DOORBELL_1                          17
#define QMEHW_IRQ_DOORBELL_2                          18
#define QMEHW_IRQ_PMCR_UPDATE                         19
#define QMEHW_IRQ_DOORBELL_A0                         20
#define QMEHW_IRQ_DOORBELL_A1                         21
#define QMEHW_IRQ_DOORBELL_A2                         22
#define QMEHW_IRQ_DOORBELL_A3                         23
#define QMEHW_IRQ_DOORBELL_B0                         24
#define QMEHW_IRQ_DOORBELL_B1                         25
#define QMEHW_IRQ_DOORBELL_B2                         26
#define QMEHW_IRQ_DOORBELL_B3                         27
#define QMEHW_IRQ_MMA_ACTIVE_CORE0                    28
#define QMEHW_IRQ_MMA_ACTIVE_CORE1                    29
#define QMEHW_IRQ_MMA_ACTIVE_CORE2                    30
#define QMEHW_IRQ_MMA_ACTIVE_CORE3                    31

#define QMEHW_IRQ_SPC_WKUP_RISE_CORE0                 32
#define QMEHW_IRQ_SPC_WKUP_RISE_CORE1                 33
#define QMEHW_IRQ_SPC_WKUP_RISE_CORE2                 34
#define QMEHW_IRQ_SPC_WKUP_RISE_CORE3                 35
#define QMEHW_IRQ_SPC_WKUP_FALL_CORE0                 36
#define QMEHW_IRQ_SPC_WKUP_FALL_CORE1                 37
#define QMEHW_IRQ_SPC_WKUP_FALL_CORE2                 38
#define QMEHW_IRQ_SPC_WKUP_FALL_CORE3                 39
#define QMEHW_IRQ_REG_WKUP_HIPRI_CORE0                40
#define QMEHW_IRQ_REG_WKUP_HIPRI_CORE1                41
#define QMEHW_IRQ_REG_WKUP_HIPRI_CORE2                42
#define QMEHW_IRQ_REG_WKUP_HIPRI_CORE3                43
#define QMEHW_IRQ_REG_WKUP_LOPRI_CORE0                44
#define QMEHW_IRQ_REG_WKUP_LOPRI_CORE1                45
#define QMEHW_IRQ_REG_WKUP_LOPRI_CORE2                46
#define QMEHW_IRQ_REG_WKUP_LOPRI_CORE3                47
#define QMEHW_IRQ_PM_ACTIVE_HIPRI_CORE0               48
#define QMEHW_IRQ_PM_ACTIVE_HIPRI_CORE1               49
#define QMEHW_IRQ_PM_ACTIVE_HIPRI_CORE2               50
#define QMEHW_IRQ_PM_ACTIVE_HIPRI_CORE3               51
#define QMEHW_IRQ_PM_ACTIVE_LOPRI_CORE0               52
#define QMEHW_IRQ_PM_ACTIVE_LOPRI_CORE1               53
#define QMEHW_IRQ_PM_ACTIVE_LOPRI_CORE2               54
#define QMEHW_IRQ_PM_ACTIVE_LOPRI_CORE3               55
#define QMEHW_IRQ_CPMS_INTERRUPT0_CORE0               56
#define QMEHW_IRQ_CPMS_INTERRUPT0_CORE1               57
#define QMEHW_IRQ_CPMS_INTERRUPT0_CORE2               58
#define QMEHW_IRQ_CPMS_INTERRUPT0_CORE3               59
#define QMEHW_IRQ_CPMS_INTERRUPT1_CORE0               60
#define QMEHW_IRQ_CPMS_INTERRUPT1_CORE1               61
#define QMEHW_IRQ_CPMS_INTERRUPT1_CORE2               62
#define QMEHW_IRQ_CPMS_INTERRUPT1_CORE3               63


////////////////////////////////////////////////////////////////////////////
// IRQ Configuration
////////////////////////////////////////////////////////////////////////////

/// Static configuration data for external interrupts:
///
/// IRQ#, TYPE, POLARITY, ENABLE
///
#define APPCFG_EXT_IRQS_CONFIG \
    QMEHW_IRQ_DEBUGGER              STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_DEBUG_TRIGGER         STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_QUAD_CHECKSTOP        STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_LFIR_INDICATION       STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_SPARE_4               STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_SPARE_5               STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_SPARE_6               STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_SPARE_7               STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_BCE_BUSY              STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_RS4_BUSY              STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_RESCLK_TARGET_ASSIST  STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_RESCLK_DONE_ASSIST    STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_SPARE_12              STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_SPARE_13              STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_TBR_CYCLES_ROLLED     STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_TBR_TIMEBASE_ROLLED   STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_DOORBELL_0            STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_DOORBELL_1            STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_DOORBELL_2            STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_PMCR_UPDATE           STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_DOORBELL_A0           STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_DOORBELL_A1           STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_DOORBELL_A2           STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_DOORBELL_A3           STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_DOORBELL_B0           STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_DOORBELL_B1           STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_DOORBELL_B2           STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_DOORBELL_B3           STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_MMA_ACTIVE_CORE0      STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_MMA_ACTIVE_CORE1      STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_MMA_ACTIVE_CORE2      STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_MMA_ACTIVE_CORE3      STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_SPC_WKUP_RISE_CORE0   STD_IRQ_TYPE_EDGE     STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_SPC_WKUP_RISE_CORE1   STD_IRQ_TYPE_EDGE     STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_SPC_WKUP_RISE_CORE2   STD_IRQ_TYPE_EDGE     STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_SPC_WKUP_RISE_CORE3   STD_IRQ_TYPE_EDGE     STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_SPC_WKUP_FALL_CORE0   STD_IRQ_TYPE_EDGE     STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_SPC_WKUP_FALL_CORE1   STD_IRQ_TYPE_EDGE     STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_SPC_WKUP_FALL_CORE2   STD_IRQ_TYPE_EDGE     STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_SPC_WKUP_FALL_CORE3   STD_IRQ_TYPE_EDGE     STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_REG_WKUP_HIPRI_CORE0  STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_REG_WKUP_HIPRI_CORE1  STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_REG_WKUP_HIPRI_CORE2  STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_REG_WKUP_HIPRI_CORE3  STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_REG_WKUP_LOPRI_CORE0  STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_REG_WKUP_LOPRI_CORE1  STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_REG_WKUP_LOPRI_CORE2  STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_REG_WKUP_LOPRI_CORE3  STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_PM_ACTIVE_HIPRI_CORE0 STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_PM_ACTIVE_HIPRI_CORE1 STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_PM_ACTIVE_HIPRI_CORE2 STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_PM_ACTIVE_HIPRI_CORE3 STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_PM_ACTIVE_LOPRI_CORE0 STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_PM_ACTIVE_LOPRI_CORE1 STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_PM_ACTIVE_LOPRI_CORE2 STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_PM_ACTIVE_LOPRI_CORE3 STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_CPMS_INTERRUPT0_CORE0 STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_CPMS_INTERRUPT0_CORE1 STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_CPMS_INTERRUPT0_CORE2 STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_CPMS_INTERRUPT0_CORE3 STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_CPMS_INTERRUPT1_CORE0 STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_CPMS_INTERRUPT1_CORE1 STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_CPMS_INTERRUPT1_CORE2 STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    QMEHW_IRQ_CPMS_INTERRUPT1_CORE3 STD_IRQ_TYPE_LEVEL    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED


/// This 64 bit mask specifies which of the interrupts are not to be used.
#define APPCFG_IRQ_INVALID_MASK 0
/*
#define APPCFG_IRQ_INVALID_MASK \
(\
   STD_IRQ_MASK64(QMEHW_IRQ_SPARE_4)  |  \
   STD_IRQ_MASK64(QMEHW_IRQ_SPARE_5)  |  \
   STD_IRQ_MASK64(QMEHW_IRQ_SPARE_6)  |  \
   STD_IRQ_MASK64(QMEHW_IRQ_SPARE_7)  |  \
   STD_IRQ_MASK64(QMEHW_IRQ_SPARE_12) |  \
   STD_IRQ_MASK64(QMEHW_IRQ_SPARE_13))
*/

#endif  /* __QMEHW_INTERRUPTS_H__ */
