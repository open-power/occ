/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/sbefw/pk_app_cfg.h $                              */
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
/*
 * @file: ppe/sbe/sbefw/pk_app_cfg.h
 *
 * @brief Application specific overrides go here.
 *
 */

#ifndef __PK_APP_CFG_H__
#define __PK_APP_CFG_H__

#include "sbeirq.H"

/*
 * @brief  Static configuration data for external interrupts:
 *     IRQ#, TYPE, POLARITY, ENABLE
 *
 */
#define APPCFG_EXT_IRQS_CONFIG \
    SBE_IRQ_START0              STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    SBE_IRQ_START1              STD_IRQ_TYPE_EDGE    STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    SBE_IRQ_INTR0               STD_IRQ_TYPE_LEVEL   STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    SBE_IRQ_INTR1               STD_IRQ_TYPE_LEVEL   STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    SBE_IRQ_DRTM_REQ            STD_IRQ_TYPE_LEVEL   STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    SBE_IRQ_SBEFIFO_RESET       STD_IRQ_TYPE_LEVEL   STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \
    SBE_IRQ_SBEFIFO_DATA        STD_IRQ_TYPE_LEVEL   STD_IRQ_POLARITY_RISING    STD_IRQ_MASKED \


/*
 * @brief This 64 bit mask specifies which of the interrupts are not to be used.
 *
 */
#define APPCFG_IRQ_INVALID_MASK \
(\
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_7) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_8) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_9) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_10) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_11) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_12) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_13) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_14) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_15) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_16) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_17) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_18) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_19) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_20) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_21) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_22) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_23) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_24) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_25) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_26) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_27) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_28) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_29) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_30) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_31) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_32) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_33) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_34) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_35) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_36) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_37) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_38) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_39) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_40) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_41) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_42) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_43) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_44) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_45) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_46) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_47) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_48) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_49) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_50) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_51) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_52) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_53) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_54) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_55) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_56) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_57) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_58) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_59) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_60) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_61) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_62) |  \
   STD_IRQ_MASK64(SBE_IRQ_RESERVED_63))


/*
 * @brief  Override the default behavior of the PK API error handling.
 *          Force PK to send the return code back to the application,
 *          instead of a kernel panic.
 *
 */
#ifndef PK_ERROR_PANIC
#define PK_ERROR_PANIC 0
#endif


#endif /*__PK_APP_CFG_H__*/
