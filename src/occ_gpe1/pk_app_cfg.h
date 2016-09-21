/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/pk_app_cfg.h $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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

#include "global_app_cfg.h"

#ifndef SIMICS_ENVIRONMENT
#define SIMICS_ENVIRONMENT 0
#endif

#if SIMICS_ENVIRONMENT
#pragma message "Building for Simics!"
#endif

/// Static configuration data for external interrupts:
///
/// IRQ#, TYPE, POLARITY, ENABLE
///
#define APPCFG_EXT_IRQS_CONFIG \
    OCCHW_IRQ_CHECK_STOP_GPE1           OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_IPI1_HI_PRIORITY          OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \
    OCCHW_IRQ_IPI1_LO_PRIORITY          OCCHW_IRQ_TYPE_EDGE     OCCHW_IRQ_POLARITY_RISING   OCCHW_IRQ_MASKED \

/// The Instance ID of the occ processor that this application is intended to run on
/// 0-3 -> GPE, 4 -> 405
#define APPCFG_OCC_INSTANCE_ID 1

// This application will statically initialize it's external interrupt table
#define STATIC_IRQ_TABLE

#define STATIC_IPC_TABLES

#endif /*__PK_APP_CFG_H__*/
