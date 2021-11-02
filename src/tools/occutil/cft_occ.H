/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/tools/occutil/cft_occ.H $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2020,2021                        */
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
/**
  @file cft_occ.H
  @brief OCC specific structures and function prototypes
*/

#ifndef __CFT_OCC_H
#define __CFT_OCC_H

extern char G_occ_string_file[512];
extern uint16_t G_sensor_type;
extern uint16_t G_sensor_loc;
extern uint16_t G_sensor_guid;

enum occStateId
{
    OCC_STATE_UNKNOWN           = 0x00,
    OCC_STATE_STANDBY           = 0x01,
    OCC_STATE_OBSERVATION       = 0x02,
    OCC_STATE_ACTIVE            = 0x03,
    OCC_STATE_SAFE              = 0x04,
    OCC_STATE_CHARACTERIZATION  = 0x05
};

#define IS_VALID_STATE(state) ((state == OCC_STATE_OBSERVATION) || \
                               (state == OCC_STATE_ACTIVE) || \
                               (state == OCC_STATE_CHARACTERIZATION))

enum powerMode
{
    POWERMODE_DISABLE           = 0x01, // Disabled Mode
    POWERMODE_SFP               = 0x03, // Static Frequency Point
    POWERMODE_SAFE              = 0x04, // -- not settable mode --
    POWERMODE_POWER_SAVE        = 0x05, // Static Power Save
    POWERMODE_FMAX              = 0x09, // Maximum Frequency
    POWERMODE_DYNAMIC_PERF      = 0x0A, // Dynamic Performace
    POWERMODE_FFO               = 0x0B, // Fixed Frequency Override
    POWERMODE_MAX_PERF          = 0x0C, // Maximum Performance
};

#define IS_VALID_MODE(mode) ((mode == POWERMODE_DISABLE) || \
                             (mode == POWERMODE_SFP) || \
                             (mode == POWERMODE_POWER_SAVE) || \
                             (mode == POWERMODE_FMAX) || \
                             (mode == POWERMODE_DYNAMIC_PERF) || \
                             (mode == POWERMODE_FFO) || \
                             (mode == POWERMODE_MAX_PERF))

const char * getCmdString(const uint8_t i_cmd);
const char * getStateString(const uint8_t i_state);
const char * getModeString(const uint8_t i_mode);
const char * getPollStatus(const uint8_t i_flags);
const char * getPollExtStatus(const uint8_t i_flags);
const char * getFruString(const uint8_t i_fru);
const char * getApssFuncName(uint8_t i_funcId);
const char * getHistoryName(uint8_t id);
uint32_t send_occ_command(const uint8_t i_occ,
                          const uint8_t i_cmd,
                          const uint8_t* i_cmd_data,
                          const uint16_t i_len,
                          const uint8_t i_node = 0);

uint32_t get_occ_trace(const uint8_t i_occ = 0,
                       const uint8_t i_node = 0);

int get_occ_sensors(const uint8_t i_occ = 0,
                    const uint8_t i_node = 0);

#endif
