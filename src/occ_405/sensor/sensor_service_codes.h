/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor_service_codes.h $                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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

#ifndef _SENSOR_SERVICE_CODES_H_
#define _SENSOR_SERVICE_CODES_H_

#include <comp_ids.h>

enum occSensorModuleId
{
    // Sensors
    SENSOR_QUERY_LIST                = SNSR_COMP_ID | 0x00,
    SENSOR_INITIALIZE                = SNSR_COMP_ID | 0x01,

    // Main memory sensors
    MM_SENSORS_INIT_MOD              = SNSR_COMP_ID | 0x10,
    MM_SENSORS_UPDATE_MOD            = SNSR_COMP_ID | 0x11,
    MM_SENSORS_BCE_COPY_MOD          = SNSR_COMP_ID | 0x12,
    MM_SENSORS_IS_BCE_REQ_IDLE_MOD   = SNSR_COMP_ID | 0x13,
    MM_SENSORS_WRITE_DATA_HDR_MOD    = SNSR_COMP_ID | 0x14,
    MM_SENSORS_VALIDATE_DATA_HDR_MOD = SNSR_COMP_ID | 0x15,
    MM_SENSORS_WRITE_NAMES_MOD       = SNSR_COMP_ID | 0x16,
    MM_SENSORS_WRITE_READINGS_MOD    = SNSR_COMP_ID | 0x17,
    MM_SENSORS_VALIDATE_READINGS_MOD = SNSR_COMP_ID | 0x18,

    // Inband commands
    INBAND_CMD_IS_BCE_REQ_IDLE_MOD   = SNSR_COMP_ID | 0x20,
    INBAND_CMD_BCE_COPY_MOD          = SNSR_COMP_ID | 0x21,
    INBAND_CMD_HANDLER_MOD           = SNSR_COMP_ID | 0x22,
    INBAND_CMD_CHECK_MOD             = SNSR_COMP_ID | 0x23,
};

#endif /* #ifndef _SENSOR_SERVICE_CODES_H_ */
