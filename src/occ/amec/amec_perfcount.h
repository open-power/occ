/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_perfcount.h $                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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
 
#ifndef _AMEC_PERFCOUNT_H
#define _AMEC_PERFCOUNT_H

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
#define MAX_UTIL_SLACK_AVG_LEN    64  // Max # of samples in utilization slack averaging buffer

/*----------------------------------------------------------------------------*/
/* Typedef / Enum                                                             */
/*----------------------------------------------------------------------------*/

/// Core Performance Counter Model
typedef struct amec_core_perf_counter
{
    ///32-bit accumulator of util_slack counter
    uint32_t                    util_slack_accumulator;
    ///32-bit accumulator of util_active counter
    uint32_t                    util_active_accumulator;
    ///Circular buffer pointer to put Utilslack signal
    uint16_t                    ptr_putUtilslack;
    ///Frequency request
    uint16_t                    dps_freq_request;
    ///32-bit pointer to utilization slack averaging buffer
    uint8_t                     ptr_util_slack_avg_buffer[2*MAX_UTIL_SLACK_AVG_LEN];
    ///32-bit pointer to utilization active averaging buffer
    uint8_t                     ptr_util_active_avg_buffer[2*MAX_UTIL_SLACK_AVG_LEN];
    ///8-bit counter of cores that are active (utilization>CPU_utilization_threshold)
    uint8_t                     util_active_core_counter;
    ///8-bit counter of cores with slack (utilization<type 1 alg UTIL tlutil)
    uint8_t                     util_slack_core_counter;
}amec_core_perf_counter_t;

/*----------------------------------------------------------------------------*/
/* Function Prototypes                                                        */
/*----------------------------------------------------------------------------*/

/**
 * Calculate the performance counter for a core 
 *  
 */
void amec_calc_dps_util_counters(const uint8_t i_core_id);

/**
 * Build the performance counter for a core 
 *  
 */
amec_core_perf_counter_t* amec_core_perf_counter_ctor(amec_core_perf_counter_t* i_this_ptr,
                                                      const uint8_t i_proc_id,
                                                      const uint8_t i_core_id);

#endif
