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

#ifndef _DIMM_STRUCTS_H
#define _DIMM_STRUCTS_H

#include "occ_util.h"
#include <gpe_export.h>
#include "gpe_err.h"
//#include "occ_sys_config.h"


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


typedef struct
{
    GpeErrorStruct error;
    uint8_t mca;
} reset_mem_deadman_args_t;

// Base Address of NIMBUS MCA.
#define DIMM_MCA_BASE_ADDRESS     0x07010800

/*
MC/Port Address MCA Port Address    Control Addr        SCOM Address
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
#define N_M_DIMM_TCR(mc,port) (DIMM_MCA_BASE_ADDRESS + 0x116 + \
                               (0x01000000 * (mc)) + ( 0x40 * (port)))

/*
MC/Port Address MCA Port Address   Deadman Offset       SCOM Address
mc01.port0        0x07010800        + 0x0000013C        = 0x0701093C
mc01.port1        0x07010840        + 0x0000013C        = 0x0701097C
mc01.port2        0x07010880        + 0x0000013C        = 0x070109BC
mc01.port3        0x070108C0        + 0x0000013C        = 0x070109FC
mc23.port0        0x08010800        + 0x0000013C        = 0x0801093C
mc23.port1        0x08010840        + 0x0000013C        = 0x0801097C
mc23.port2        0x08010880        + 0x0000013C        = 0x080109BC
mc23.port3        0x080108C0        + 0x0000013C        = 0x080109FC
 */

//  NIMBUS DIMM Deadman SCOM Register Addresses macro
#define DEADMAN_TIMER_PORT(mc,port) (DIMM_MCA_BASE_ADDRESS + 0x13C +         \
                               (0x01000000 * (mc)) + ( 0x40 * (port)))

#define DEADMAN_TIMER_MCA(mca) (DIMM_MCA_BASE_ADDRESS + 0x13C +              \
                               (0x01000000 * (mca>>2)) + ( 0x40 * (mca&3)))

#endif // _DIMM_STRUCTS_H
