/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor_get_tod_task.h $                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017,2017                        */
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

#ifndef _SENSOR_GET_TOD_TASK_H
#define _SENSOR_GET_TOD_TASK_H

/**
 * @file sensor_get_tod_task.h
 *
 * This file declares the functions for the task that gets the current Time Of
 * Day (TOD).
 */

//******************************************************************************
// Includes
//******************************************************************************
#include <rtls.h>                   // For task_t


//******************************************************************************
// Function Prototypes
//******************************************************************************

/**
 * Initial function called by the TASK_ID_GET_TOD task.  Gets the current Time
 * Of Day (TOD) value and stores it in the global variable G_tod.
 *
 * @param i_self This task
 */
void task_get_tod(task_t * i_self);

#endif // _SENSOR_GET_TOD_TASK_H
