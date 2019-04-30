/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_sensors_power.h $                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2019                        */
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

#ifndef _AMEC_SENSORS_POWER_H
#define _AMEC_SENSORS_POWER_H

/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/
#include <occ_common.h>
#include <ssx.h>
#include <ssx_app_cfg.h>
#include "amec_external.h"

/*----------------------------------------------------------------------------*/
/* Defines/Constants                                                          */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Function Declarations                                                      */
/*----------------------------------------------------------------------------*/

// Function that is called by AMEC State Machine that will update the AMEC
// sensors for data that comes from the APSS (Power Data from APSS ADCs)
// Returns False if GPIO EPOW was asserted (sensors not updated)
bool amec_update_apss_sensors(void);

// Function that is called by AMEC State Machine that will update the AMEC
// sensors for GPIO data collected from the APSS.
void amec_update_apss_gpio(void);

// Function that is called by AMEC State Machine that will update the AMEC
// sensors for data that comes from the AVS Bus (Voltage/Current)
void amec_update_avsbus_sensors(void);

// Function called by the AMEC state machine until GPU configuration is
// successfully determined
void amec_update_gpu_configuration(void);

#endif // _AMEC_SENSORS_POWER_H
