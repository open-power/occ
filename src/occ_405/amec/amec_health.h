/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_health.h $                              */
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

#ifndef AMEC_HEALTH_H
#define AMEC_HEALTH_H

//*************************************************************************
// Includes
//*************************************************************************
#include "cmdh_fsp_cmds_datacnfg.h"

//*************************************************************************
// Defines/Enums
//*************************************************************************
// Error threshold check is done every 16msec. Error timer is 5x16 = 80msec
#define AMEC_HEALTH_ERROR_TIMER 5

/*******************************************************************/
/* Function Definitions                                            */
/*******************************************************************/

void amec_health_check_proc_temp(void);
void amec_health_check_proc_timeout(void);
void amec_health_check_cent_temp(void);
void amec_health_check_cent_timeout(void);
void amec_health_check_dimm_temp(void);
void amec_health_check_dimm_timeout(void);
void amec_mem_mark_logged(uint8_t i_cent,
                          uint8_t i_dimm,
                          uint16_t* i_clog_bitmap,
                          uint8_t* i_dlog_bitmap);
void amec_health_check_vrm_vdd_temp(const sensor_t *i_sensor);
void amec_health_check_vrm_vdd_temp_timeout(void);

#endif
