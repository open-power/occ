/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_controller.h $                          */
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

#ifndef _AMEC_CONTROLLER_H
#define _AMEC_CONTROLLER_H
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
// Maximum speed supported for memory controllers (100.0%)
#define AMEC_MEMORY_MAX_STEP                 1000

// Minimum speed supported for memory controllers (1.0%)
#define AMEC_MEMORY_MIN_STEP                 10

// Step size for increasing or decreasing memory speed (1.0%)
#define AMEC_MEMORY_STEP_SIZE                10

// Limit value on speed change to avoid overflow of the controller
#define AMEC_MEMORY_SPEED_CHANGE_LIMIT       (UINT16_MAX/4)/AMEC_MEMORY_STEP_SIZE


/*----------------------------------------------------------------------------*/
/* Typedef / Enum                                                             */
/*----------------------------------------------------------------------------*/
///Controller Model
typedef struct amec_controller
{
    ///Over-temperature error for FRU callout (in degrees C)
    uint8_t                       ot_error;
    ///Setpoint or reference for the controller
    uint16_t                      setpoint;
    ///Proportional gain of the controller
    uint16_t                      Pgain;
    ///Theoretical controller residue summation
    uint16_t                      total_res;
    ///Final speed request of the controller
    uint16_t                      speed_request;
    ///Final frequency request of the controller
    uint16_t                      freq_request;
    ///Expiration time for a temperature reading (in seconds)
    uint8_t                       temp_timeout;
}amec_controller_t;


/*----------------------------------------------------------------------------*/
/* Function Prototypes                                                        */
/*----------------------------------------------------------------------------*/

/**
 *  Thermal Control Loop based on processor temperatures.
 *
 *  This function implements the Proportional Controller for the thermal
 *  control loop. Although it doesn't return any results, it populates the
 *  thermal vote in the field g_amec->thermalproc.speed_request.
 *
 */
void amec_controller_proc_thermal();

/**
 *  Thermal Control Loop based on VRM Vdd temperature
 *
 *  This function implements the Proportional Controller for the VRM Vdd thermal
 *  control loop. Although it doesn't return any results, it populates the
 *  thermal vote in the field g_amec->thermalvdd.speed_request.
 *
 */
void amec_controller_vrm_vdd_thermal();

/**
 *  Thermal Control Loop based on membuf temperatures.
 *
 *  This function implements a Proportional Controller for the
 *  thermal control loop based on membuf temperatures.
 *  Although it doesn't return any results, it populates the
 *  thermal vote in the field g_amec->thermalmembuf.speed_request.
 *
 */
void amec_controller_membuf_thermal();

/**
 *  Thermal Control Loop based on DIMM temperatures.
 *
 *  This function implements a Proportional Controller for the
 *  thermal control loop based on DIMM temperatures. Although it
 *  doesn't return any results, it populates the thermal vote in
 *  the field g_amec->thermaldimm.speed_request.
 *
 */
void amec_controller_dimm_thermal();

/**
 *  Helper function to convert speed to MHz
 *
 *  This function converts a given speed in percentage (%) to a
 *  frequency in MHz.
 *
 */
uint16_t amec_controller_speed2freq (const uint16_t i_speed, const uint16_t i_fmax);


#endif //_AMEC_CONTROLLER_H
