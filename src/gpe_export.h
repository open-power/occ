/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/gpe_export.h $                                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
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

#ifndef _GPE_EXPORT_H
#define _GPE_EXPORT_H

#include "gpe_err.h"

#define WORD_HIGH(data) ((uint32_t)(((uint64_t)data)>>32))
#define WORD_LOW(data)  ((uint32_t)(((uint64_t)data)&0xFFFFFFFF))


// GPE Error structure (common to both GPEs)
typedef struct {
     union
     {
       struct {
         uint32_t rc;
         uint32_t addr;
       };
       uint64_t error;
     };
     uint64_t ffdc;
} GpeErrorStruct;


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

// Arguments for doing a SCOM from GPE0
typedef struct ipc_scom_op
{
    uint32_t        addr;   // Register address
    uint64_t        data;   // Data for read/write
    uint32_t        size;   // Size of data buffer
    uint8_t         read;   // Read (1) or write (0)
    GpeErrorStruct  error;  // Error of SCOM operation
} ipc_scom_op_t;

#endif //_GPE_EXPORT_H
