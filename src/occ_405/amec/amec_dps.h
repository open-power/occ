/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_dps.h $                                 */
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

#ifndef _AMEC_DPS_H
#define _AMEC_DPS_H

/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Globals                                                                    */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Defines                                                                    */
/*----------------------------------------------------------------------------*/
#define TWO_TO_THE_POWER_OF_FIFTEEN      32768

/*----------------------------------------------------------------------------*/
/* Typedef / Enum                                                             */
/*----------------------------------------------------------------------------*/
///DPS Algorithm Model
typedef struct amec_dps
{
    ///Frequency request for core voting box
    uint16_t                    freq_request;
    ///Utilization speed request
    uint16_t                    util_speed_request;
    ///Utilization threshold for moving down in frequency (low side)
    uint16_t                    tlutil;
    ///Step size for going up in speed
    uint16_t                    step_up;
    ///Step size for going down in speed
    uint16_t                    step_down;
    ///Number of utilization samples in sliding window
    uint16_t                    sample_count_util;
    ///Epsilon used for determining if a core is active (units of 0.01%)
    uint16_t                    epsilon_perc;
    ///Threshold for going up in frequency
    uint16_t                    alpha_up;
    ///Threshold for going down in frequency
    uint16_t                    alpha_down;
    ///8-bit mask for dynamic power save type (=0:none active)
    uint8_t                     type;
}amec_dps_t;

/*----------------------------------------------------------------------------*/
/* Function Prototypes                                                        */
/*----------------------------------------------------------------------------*/

/**
 *  Update per-core utilization variables.
 *
 *  This function updates all the per-core utilization variables. These
 *  variables are used to populate the slack sensors.
 *
 */
void amec_dps_update_core_util(void);

/**
 *  Update utilization sensors for a core group.
 *
 *  This function updates the utilization (slack) sensors for a
 *  given core group.
 *
 */
void amec_dps_partition_update_sensors(const uint16_t i_part_id);

/**
 *  DPS algorithm function.
 *
 *  This function implements the different DPS algorithms for a
 *  given core group.
 *
 */
void amec_dps_partition_alg(const uint16_t i_part_id);

/**
 *  Main DPS function.
 *
 *  This function is the entry-point for running DPS algorithms
 *  and is aware of the different core groups defined.
 *
 */
void amec_dps_main(void);

#endif  /* #ifndef _AMEC_DPS_H */
