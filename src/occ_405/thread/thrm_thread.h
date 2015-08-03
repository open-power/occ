/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/thread/thrm_thread.h $                            */
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

#ifndef THRM_THREAD_H
#define THRM_THREAD_H

#define THRM_MAX_NUM_ZONES  8

typedef struct thrm_fru_control
{
    uint8_t                     t_control;
    uint8_t                     error;
    uint8_t                     warning;
    uint8_t                     warning_reset;
    uint16_t                    t_inc_zone[THRM_MAX_NUM_ZONES];
}thrm_fru_control_t;

typedef struct thrm_fru_data
{
    // Flag to indicate that we have timed out trying to read this thermal sensor
    uint8_t                     read_failure;
    // Current temperature of this FRU
    uint16_t                    Tcurrent;
    // Previous temperature of this FRU
    uint16_t                    Tprevious;
    // Coooling requests to be sent back to TMGT
    uint16_t                    FanIncZone[THRM_MAX_NUM_ZONES];
    // Flag to indicate if the temperature for this FRU is increasing or not
    BOOLEAN                     temp_increasing;
}thrm_fru_data_t;

errlHndl_t thrm_thread_load_thresholds();
void THRM_thread_update_thresholds();
uint8_t THRM_thread_get_cooling_request();
BOOLEAN thrm_thread_fan_control(const uint8_t i_frutype,
                                const uint16_t i_fru_temperature);
BOOLEAN thrm_thread_vrm_fan_control(const uint16_t i_vrfan);
void thrm_thread_main();

#endif
