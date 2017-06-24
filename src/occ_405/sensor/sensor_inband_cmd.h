/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor_inband_cmd.h $                      */
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

#ifndef _SENSOR_INBAND_CMD_H
#define _SENSOR_INBAND_CMD_H

/**
 * @file sensor_inband_cmd.h
 *
 * This file declares the functions and global variables for supporting the inband
 * command protocol defined in the OCC Firmware Interface spec
 *
 */

//******************************************************************************
// Defines/Structs/Globals
//******************************************************************************

// States of inband OCC command processing
typedef enum
{
    INBAND_OCC_CMD_NONE = 0x00,             // No inband command in process
    INBAND_OCC_CMD_CHECK_FOR_CMD = 0x01,    // Check for cmd when BCE read finishes
    INBAND_OCC_CMD_START = 0x02,            // Start processing the command
    INBAND_OCC_CMD_RSP_READY = 0x03,        // Response ready
    INBAND_OCC_CMD_RSP_INT = 0x04,          // Send Response Interrupt
    INBAND_OCC_INVALID_BCE_CALLBACK = 0x05, // BCE callback called with invalid OCC cmd state
} INBAND_OCC_CMD_STATE;

// Current state of inband OCC command processing
extern volatile uint8_t G_inband_occ_cmd_state;
// Last state saved for error handling
extern volatile uint8_t G_inband_occ_bce_saved_state;

#define MAX_TICS_INBAND_BCE_CALLBACK_WAIT 0x03
#define IN_BAND_CMD_READY_MASK 0x80
#define IN_BAND_RSP_IN_PROGRESS_MASK 0x02
#define IN_BAND_RSP_READY_MASK 0x01

typedef struct __attribute__ ((packed))
{
    uint8_t  flags;
    uint8_t  seq;
    uint8_t  cmd_type;
    uint8_t  reserved_rc;    // as a command reserved, as a response RC
    uint8_t  data_length[2];
} inband_occ_cmd_header_t;

// Data Size is Block Copy Engine dependent, 128 min to 4kB max
#define INBAND_CMD_MIN_BCE_BUF_SIZE  128
#define INBAND_CMD_MAX_BCE_BUF_SIZE  4096
#define INBAND_MAX_DATA_LENGTH INBAND_CMD_MAX_BCE_BUF_SIZE - sizeof(inband_occ_cmd_header_t)
#define INBAND_MIN_DATA_LENGTH INBAND_CMD_MIN_BCE_BUF_SIZE - sizeof(inband_occ_cmd_header_t)

typedef struct __attribute__ ((packed))
{
    inband_occ_cmd_header_t  header;
    // Data bytes
    uint8_t   data[INBAND_MIN_DATA_LENGTH];
}inband_min_cmd_t;

typedef struct __attribute__ ((packed))
{
    inband_occ_cmd_header_t  header;
    // Data bytes
    uint8_t   data[INBAND_MAX_DATA_LENGTH];
}inband_max_cmd_t;

//******************************************************************************
// Function Prototypes
//******************************************************************************

/**
 * Check for a command from the inband command/response interface
 *
 * There are inband commands to select sensors to write to main memory and
 * clear sensors min/max by user
 *
 * This function is called from amec_slv_common_tasks_post()
 */
void inband_command_check(void);

/**
 * Handle a command from the inband command/response interface
 *
 * This function is called from amec_slv_substate_6_x
 */
void inband_command_handler(void);

#endif // _SENSOR_INBAND_CMD_H
