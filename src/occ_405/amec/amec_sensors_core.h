/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_sensors_core.h $                        */
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
// See bit definition of PM State History Register for OCC
#define OCC_PM_STATE_MASK      0x1F000000  //Mask bits 0:2 of the register
#define OCC_PAST_FAST_SLEEP    0x18000000  //Core has been in fast sleep
#define OCC_PAST_DEEP_SLEEP    0x1C000000  //Core has been in deep sleep
#define OCC_PAST_FAST_WINKLE   0x1E000000  //Core has been in fast winkle
#define OCC_PAST_DEEP_WINKLE   0x1F000000  //Core has been in deep winkle
#define OCC_PAST_CORE_CLK_STOP 0x08000000  //Core has been in an idle state with core clocks stopped

/*----------------------------------------------------------------------------*/
/* Structures                                                                 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Function Declarations                                                      */
/*----------------------------------------------------------------------------*/
void amec_update_fast_core_data_sensors(void);

void amec_update_proc_core_sensors(uint8_t i_core);

#endif // _AMEC_SENSORS_CORE_H
