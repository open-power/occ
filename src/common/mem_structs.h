/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/dimm_structs.h $                                          */
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

/* This header file is used by both occ_405 and occ_gpe1.                 */
/* Contains common structures and globals.                                */

#ifndef _MEM_STRUCTS_H
#define _MEM_STRUCTS_H

#include "occ_util.h"
#include <gpe_export.h>
#include "gpe_err.h"

// this enum defines memory power control
typedef enum
{
    MEM_PWR_CTL_OFF                  = 0x00,
    MEM_PWR_CTL_POWER_DOWN           = 0x01,
    MEM_PWR_CTL_PD_AND_STR           = 0x02,
    MEM_PWR_CTL_PD_AND_STR_CLK_STOP  = 0x03,
    MEM_PWR_CTL_NO_SUPPORT           = 0xFF,
} eMemoryPowerControlSetting;

// memory power control IPC argument
typedef struct
{
    GpeErrorStruct    error;
    uint8_t           mem_pwr_ctl;
    uint8_t           port;
    uint8_t           mc;
} mem_power_control_args_t;

#endif  //_MEM_STRUCTS_H
