/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/dimm/dimm_control.h $                             */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016                             */
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

#include <occ_common.h>
#include "dimm_structs.h"
#include "rtls.h"

#ifndef _DIMM_CONTROL_H
#define _DIMM_CONTROL_H

bool dimm_control(uint8_t mc, uint8_t port);
void dimm_update_nlimits(uint8_t mc, uint8_t port);
void populate_dimm_control_args(uint16_t i_throttle, uint8_t mc, uint8_t port,
                                dimm_control_args_t * dimm_control_args);
uint16_t convert_speed2numerator(uint16_t i_throttle, uint16_t min_n_value, uint16_t max_n_value);

#endif //_DIMM_CONTROL_H
