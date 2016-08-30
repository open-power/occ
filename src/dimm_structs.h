/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/dimm_control_structs.h $                                  */
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

#ifndef _DIMM_STRUCTS_H
#define _DIMM_STRUCTS_H

#include <gpe_export.h>
#include "gpe_err.h"
//#include "occ_sys_config.h"

#define WORD_HIGH(data) ((uint32_t)(((uint64_t)data)>>32))
#define WORD_LOW(data)  ((uint32_t)(((uint64_t)data)&0xFFFFFFFF))

// DIMM States (GPE1)
typedef enum
{
    DIMM_STATE_INIT                     = 0x01, // Init interrupt registers
    DIMM_STATE_WRITE_MODE               = 0x02, // Set the mode (speed and port)
    DIMM_STATE_WRITE_ADDR               = 0x03, // Write the temp sensor address
    DIMM_STATE_INITIATE_READ            = 0x04, // Start the read
    DIMM_STATE_READ_TEMP                = 0x05, // Return the sensor reading
    DIMM_STATE_RESET_MASTER             = 0x06, // Reset master
    DIMM_STATE_RESET_SLAVE_P0           = 0x07, // Start of slave port 0 reset
    DIMM_STATE_RESET_SLAVE_P0_WAIT      = 0x08,
    DIMM_STATE_RESET_SLAVE_P0_COMPLETE  = 0x09,
    DIMM_STATE_RESET_SLAVE_P1           = 0x0A, // Start of slave port 1 reset
    DIMM_STATE_RESET_SLAVE_P1_WAIT      = 0x0B,
    DIMM_STATE_RESET_SLAVE_P1_COMPLETE  = 0x0C,
} DIMM_STATE;

// DIMM State Machine arguments (GPE1)
typedef struct
{
  GpeErrorStruct error;
  uint8_t state;
  uint8_t i2cEngine;
  uint8_t i2cAddr;
  uint8_t i2cPort;
  uint8_t dimm;
  uint8_t maxPorts;
  uint8_t temp;
} dimm_sm_args_t;

//convenient format for storing throttle settings
typedef union
{
    uint32_t    word32;
    struct
    {
        uint32_t slot_n:      15;      // DIMM Slot N Value
        uint32_t port_n:      16;      // DIMM port N Value
        uint32_t new_n:        1;      // New N Value?
    };
} dimm_n_value_t;


typedef struct
{
    GpeErrorStruct error;
    uint8_t mc;
    uint8_t port;
    dimm_n_value_t dimmNumeratorValues;
} dimm_control_args_t;



// Base Address of N/M DIMM Throttling Control Register
#define N_M_DIMM_TCR_BASE     0x07010800

/*
MC/Port Address  N/M Throttling     Control Addr        SCOM Address
mc01.port0        0x07010800        + 0x00000116        = 0x07010916
mc01.port1        0x07010840        + 0x00000116        = 0x07010956
mc01.port2        0x07010880        + 0x00000116        = 0x07010996
mc01.port3        0x070108C0        + 0x00000116        = 0x070109D6
mc23.port0        0x08010800        + 0x00000116        = 0x08010916
mc23.port1        0x08010840        + 0x00000116        = 0x08010956
mc23.port2        0x08010880        + 0x00000116        = 0x08010996
mc23.port3        0x080108C0        + 0x00000116        = 0x080109D6
 */

//  N/M DIMM Throttling Control SCOM Register Addresses macro
#define N_M_DIMM_TCR(mc,port) (N_M_DIMM_TCR_BASE + 0x116 + \
                               (0x01000000 * (mc)) + ( 0x40 * (port)))

#endif // _DIMM_STRUCTS_H
