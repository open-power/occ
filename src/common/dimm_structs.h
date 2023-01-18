/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/common/dimm_structs.h $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2023                        */
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

#ifndef _DIMM_STRUCTS_H
#define _DIMM_STRUCTS_H

#include "occ_util.h"
#include <gpe_export.h>
#include "gpe_err.h"
//#include "occ_sys_config.h"


// DIMM States (GPE1)
typedef enum
{
    DIMM_STATE_INIT                      = 0x01, // Init interrupt registers
    DIMM_STATE_WRITE_MODE                = 0x02, // Set the mode (speed and port)
    DIMM_STATE_WRITE_ADDR                = 0x03, // Write the temp sensor address
    DIMM_STATE_INITIATE_READ             = 0x04, // Start the read
    DIMM_STATE_READ_TEMP                 = 0x05, // Return the sensor reading
    DIMM_STATE_RESET_MASTER              = 0x06, // Reset master
    DIMM_STATE_RESET_SLAVE_PORT          = 0x07, // Start of a slave port reset
    DIMM_STATE_RESET_SLAVE_PORT_WAIT     = 0x08,
    DIMM_STATE_RESET_SLAVE_PORT_COMPLETE = 0x09,
} DIMM_STATE;

// DIMM State Machine arguments (GPE1)
typedef struct
{
  GpeErrorStruct error;
  uint8_t state;
  uint8_t i2cEngine;
  uint8_t i2cAddr;
  uint8_t i2cPort;
  uint8_t ocmb;
  uint8_t dimm;
  uint8_t maxPorts;
  uint8_t temp;
} dimm_sm_args_t;

typedef struct {
    GpeErrorStruct error;
    uint16_t configured_mbas;
} epow_gpio_args_t;

#endif // _DIMM_STRUCTS_H
