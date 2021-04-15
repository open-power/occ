/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_part.h $                                */
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

#ifndef _AMEC_PART_H
#define _AMEC_PART_H

/*----------------------------------------------------------------------------*/
/* Includes                                                                   */
/*----------------------------------------------------------------------------*/
#include <mode.h>
#include <sensor.h>
#include <occ_sys_config.h>
#include <amec_dps.h>

/*----------------------------------------------------------------------------*/
/* Constants                                                                  */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Globals                                                                    */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Defines                                                                    */
/*----------------------------------------------------------------------------*/

// Number of cores in system
#define AMEC_PART_NUM_CORES     (MAX_NUM_OCC * MAX_NUM_CORES)

// No support for PLPM, all cores will be in the same core group (partition)
// The main PLPM code will stay but the partition sensors were removed for all but the
// one core group in order to save SRAM
// If this ever changes then there must be sensors UTILSLCGx defined for each core group possible
#define AMEC_PART_MAX_PART      1

// Invalid core group ID
#define AMEC_PART_INVALID_ID    0xFF

/*----------------------------------------------------------------------------*/
/* Typedef / Enum                                                             */
/*----------------------------------------------------------------------------*/

typedef struct amec_part
{
    ///Total number of cores in this core group.
    uint8_t                     ncores;
    ///List of cores group. Indices 0 to ncores-1 are valid. Valid values: 0 to
    ///AMEC_PART_NUM_CORES-1.
    uint8_t                     core_list[AMEC_PART_NUM_CORES];
    ///Partition ID
    uint8_t                     id;
    ///Valid bit (=1 in use, =0 not in use)
    uint8_t                     valid;
    ///Power saving state
    amec_dps_t                  dpsalg;
    ///slack utilization sensor
    sensor_t                    util2msslack;
} amec_part_t;

// Main structure that contains the partition configuration for PLPM work.
typedef struct amec_part_config
{
    ///Data structure holding core to partition mapping. The value
    ///AMEC_PART_INVALID_ID means that a core is not mapped to a partition. A
    ///value less than that means core is mapped to that partition index.
    uint16_t                    core2part[MAX_NUM_CORES];

    ///Data structure holding all active partitions (core groups)
    amec_part_t                 part_list[AMEC_PART_MAX_PART];
} amec_part_config_t;

/*----------------------------------------------------------------------------*/
/* Function Prototypes                                                        */
/*----------------------------------------------------------------------------*/

/**
 * Given a core, return a valid partition that owns it, or NULL.
 *  
 */
amec_part_t* amec_part_find_by_core(amec_part_config_t* i_config,
                                    const uint16_t i_core_index);

/**
 * This function adds a new core group and should only be called within the
 * AMEC ISR.
 *
 */
void amec_part_add(uint8_t i_id);

/**
 * Add a core group. 
 *  
 */
void amec_part_init(void);

#endif //_AMEC_PART_H
