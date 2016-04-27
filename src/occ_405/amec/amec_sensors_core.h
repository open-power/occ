/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_sensors_core.h $                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
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

#ifndef _AMEC_SENSORS_CORE_H
#define _AMEC_SENSORS_CORE_H

/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/
#include <occ_common.h>
#include <ssx.h>
#include <ssx_app_cfg.h>
#include "amec_external.h"
#include <occ_sys_config.h>

/*----------------------------------------------------------------------------*/
/* Defines/Constants                                                          */
/*----------------------------------------------------------------------------*/
// See bit definition of OCC Stop State History Register
#define OCC_CORE_STOP_GATED      0x80000000  // Set upon entry stop entry transition (or initial power on)
                                             // Not cleared until core is fully accessible
#define OCC_CORE_SWUP_DONE       0x40000000  // Special wake-up done
#define OCC_CORE_ACT_STOP_LVL    0x00F00000  // Actual stop level (probably changes too fast to be useful)
// The following clear upon reading
#define OCC_DEEPEST_REQ_STOP_LVL 0x000F0000  // Deepest requestest stop level
#define OCC_DEEPEST_ACT_STOP_LVL 0x0000F000  // Deepest stop state fully entered
#define OCC_IVRM_ENABLED_HIST    0x00000800  // Bit indicating if IVRM has been enabled

/*----------------------------------------------------------------------------*/
/* Structures                                                                 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Function Declarations                                                      */
/*----------------------------------------------------------------------------*/

void amec_update_proc_core_sensors(uint8_t i_core);

#endif // _AMEC_SENSORS_CORE_H
