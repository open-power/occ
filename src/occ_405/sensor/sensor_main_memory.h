/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor_main_memory.h $                     */
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

#ifndef _SENSOR_MAIN_MEMORY_H
#define _SENSOR_MAIN_MEMORY_H

/**
 * @file sensor_main_memory.h
 *
 * This file declares the functions and global variables for copying a subset of
 * the OCC sensors to main memory.
 *
 * The sensor data is stored in the "GPU / Sensor Data" section of the OCC
 * Common image in main memory.
 *
 * Within that section, each OCC has a separate Sensor Data Block for its sensor
 * data:
 *   - OCC 0 Sensor Data Block
 *   - OCC 1 Sensor Data Block
 *   - ...
 *   - OCC 7 Sensor Data Block
 *
 * A Sensor Data Block contains the following:
 *   - Sensor Data Header Block
 *   - Sensor Names Block
 *   - Sensor Readings Ping Buffer
 *   - Sensor Readings Pong Buffer
 *
 * The Sensor Data Header Block is written once by the OCC during
 * initialization.  It contains static data that describes the Sensor Names
 * Block and Sensor Readings Buffers.
 *
 * The Sensor Names Block is also written once by the OCC during initialization.
 * It contains static data about the sensors being copied, such as the sensor
 * name, global sensor ID, and sensor type.
 *
 * The Sensor Readings Ping and Pong Buffers are updated continually by the OCC.
 * These buffers contain the dynamic sensor data, such as the current sample,
 * timestamp, and maximum value.  The OCC alternates between the two buffers so
 * that one buffer is always complete and readable by OPAL.
 * 
 * For more information see the OCC Firmware Interface Specification document.
 */

//******************************************************************************
// Includes
//******************************************************************************
#include <common_types.h>       // For bool
#include <stdint.h>             // For uint*_t
#include <sensor.h>             // For sensor defines and enums
#include <dcom.h>               // For G_pbax_id, SENSOR_DATA_COMMON_ADDRESS


//******************************************************************************
// Sensor Data Block
//******************************************************************************

#define MM_SENSOR_DATA_BLOCK_SIZE     0x00025800  /* 150kB */
#define MM_SENSOR_DATA_BLOCK_OFFSET   (G_pbax_id.chip_id * MM_SENSOR_DATA_BLOCK_SIZE)
#define MM_SENSOR_DATA_BLOCK_ADDRESS  (SENSOR_DATA_COMMON_ADDRESS + \
                                       MM_SENSOR_DATA_BLOCK_OFFSET)


//******************************************************************************
// Sensor Data Header Block
//******************************************************************************

#define MM_SENSOR_DATA_HEADER_SIZE     0x00000400  /* 1kB */
#define MM_SENSOR_DATA_HEADER_OFFSET   0x00000000
#define MM_SENSOR_DATA_HEADER_ADDRESS  (MM_SENSOR_DATA_BLOCK_ADDRESS + \
                                        MM_SENSOR_DATA_HEADER_OFFSET)
#define MM_SENSOR_DATA_HEADER_VERSION  0x01

/**
 * Values for the 'valid' field of the Sensor Data Header Block
 */
typedef enum
{
    MM_SENSOR_DATA_HEADER_VALID_FALSE = 0x00,
    MM_SENSOR_DATA_HEADER_VALID_TRUE  = 0x01,
} MM_SENSOR_DATA_HEADER_VALID_VALUES;

/**
 * Sensor Data Header Block
 */
typedef struct __attribute__ ((packed))
{
    uint8_t  valid;
    uint8_t  header_version;
    uint16_t number_of_sensors;
    uint8_t  sensor_readings_version;
    uint8_t  reserved_1[3];
    uint32_t sensor_names_offset;
    uint8_t  sensor_names_version;
    uint8_t  bytes_per_sensor_name;
    uint8_t  reserved_2[2];
    uint32_t ping_buffer_offset;
    uint32_t pong_buffer_offset;
    uint8_t  reserved_3[1000];
} mm_sensor_data_header_block_t;


//******************************************************************************
// Sensor Names Block
//******************************************************************************

#define MM_SENSOR_NAMES_SIZE        0x0000C800  /* 50kB */
#define MM_SENSOR_NAMES_OFFSET      0x00000400
#define MM_SENSOR_NAMES_ADDRESS     (MM_SENSOR_DATA_BLOCK_ADDRESS + \
                                     MM_SENSOR_NAMES_OFFSET)
#define MM_SENSOR_NAMES_VERSION     0x01

/**
 * Values for the 'sensor_structure_version' field of a Sensor Names Block Entry
 */
typedef enum
{
    MM_SENSOR_NAMES_STRUCT_VERSION_FULL    = 0x01,
    MM_SENSOR_NAMES_STRUCT_VERSION_COUNTER = 0x02,
} MM_SENSOR_NAMES_STRUCT_VERSION_VALUES;

/**
 * Sensor Names Block Entry
 */
typedef struct __attribute__ ((packed))
{
    char     name[MAX_SENSOR_NAME_SZ];
    char     units[MAX_SENSOR_UNIT_SZ];
    uint16_t gsid;
    uint32_t freq;
    uint32_t scale_factor;
    uint16_t type;
    uint16_t location;
    uint8_t  sensor_structure_version;
    uint32_t reading_offset;
    uint8_t  sensor_specific_info1;
    uint8_t  reserved[8];
} mm_sensor_names_entry_t;


//******************************************************************************
// Sensor Readings Buffers
//******************************************************************************

#define MM_SENSOR_READINGS_SIZE          0x0000A000  /* 40kB */
#define MM_SENSOR_READINGS_PING_OFFSET   0x0000DC00
#define MM_SENSOR_READINGS_PONG_OFFSET   0x00018C00
#define MM_SENSOR_READINGS_PING_ADDRESS  (MM_SENSOR_DATA_BLOCK_ADDRESS + \
                                          MM_SENSOR_READINGS_PING_OFFSET)
#define MM_SENSOR_READINGS_PONG_ADDRESS  (MM_SENSOR_DATA_BLOCK_ADDRESS + \
                                          MM_SENSOR_READINGS_PONG_OFFSET)
#define MM_SENSOR_READINGS_VERSION       0x01

/**
 * Values for the 'valid' field of a Sensor Readings Buffer Header
 */
typedef enum
{
    MM_SENSOR_READINGS_VALID_FALSE = 0x00,
    MM_SENSOR_READINGS_VALID_TRUE  = 0x01,
} MM_SENSOR_READINGS_VALID_VALUES;

/**
 * Sensor Readings Buffer Header
 */
typedef struct __attribute__ ((packed))
{
    uint8_t valid;
    uint8_t reserved[7];
} mm_sensor_readings_buf_header_t;

/**
 * Full Sensor Readings Structure.  Used when structure version is
 * MM_SENSOR_NAMES_STRUCT_VERSION_FULL.
 */
typedef struct __attribute__ ((packed))
{
    uint16_t gsid;
    uint64_t timestamp;
    uint16_t sample;
    uint16_t sample_min;
    uint16_t sample_max;
    uint16_t csm_sample_min;
    uint16_t csm_sample_max;
    uint16_t profiler_sample_min;
    uint16_t profiler_sample_max;
    uint16_t job_s_sample_min;
    uint16_t job_s_sample_max;
    uint64_t accumulator;
    uint32_t update_tag;
    uint8_t  reserved[8];
} mm_sensor_readings_full_t;

/**
 * Counter Sensor Readings Structure.  Used when structure version is
 * MM_SENSOR_NAMES_STRUCT_VERSION_COUNTER.
 */
typedef struct __attribute__ ((packed))
{
    uint16_t gsid;
    uint64_t timestamp;
    uint64_t accumulator;
    uint8_t  sample;
    uint8_t  reserved[5];
} mm_sensor_readings_counter_t;

/**
 * Returns the size of the specified sensor readings structure version.
 * See MM_SENSOR_NAMES_STRUCT_VERSION_VALUES.
 */
#define MM_SENSOR_READINGS_STRUCT_SIZE(sensor_structure_version) \
    (((sensor_structure_version) == MM_SENSOR_NAMES_STRUCT_VERSION_FULL) ? \
     sizeof(mm_sensor_readings_full_t) : \
     sizeof(mm_sensor_readings_counter_t))


//******************************************************************************
// Globals
//******************************************************************************

/**
 * Indicates whether the main memory sensors have been initialized.  If false,
 * main_mem_sensors_init() needs to be called.
 */
extern bool G_main_mem_sensors_initialized;

/**
 * Indicates whether Secure Memory (SMF) mode is enabled.  When secure mode is
 * enabled only a subset of sensors will be copied to main memory.  Default
 * value is false (not enabled).
 */
extern bool G_main_mem_sensors_smf_mode_enabled;


//******************************************************************************
// Function Prototypes
//******************************************************************************

/**
 * Initializes the main memory sensors.  Writes static sensor data to main
 * memory.
 *
 * Performs the following tasks:
 *   - Initializes internal data structures
 *   - Writes the Sensor Data Header Block in main memory
 *   - Writes the Sensor Names Block in main memory
 *   - Sets G_main_mem_sensors_initialized to true if all initialization is done
 *
 * This function must be called repeatedly until G_main_mem_sensors_initialized
 * is true.  This is due to a limit on how much data can be written to main
 * memory at once.
 */
void main_mem_sensors_init(void);


/**
 * Updates the main memory sensors.  Writes dynamic sensor data to main memory.
 *
 * There are two Sensor Readings Buffers (ping and pong) in main memory used to
 * store dynamic sensor data.  This function will automatically alternate
 * between the two buffers.
 *
 * This function should be called repeatedly from a timer.  It will update a
 * portion of a Sensor Readings Buffer with the most recent data.  There is a
 * limit on how much data can be copied to main memory at once, so multiple
 * calls are required to completely update one Sensor Readings Buffer.
 *
 * Note: This function cannot be called until after main_mem_sensors_init() has
 * completely initialized the main memory sensors.
 */
void main_mem_sensors_update(void);


/**
 * Sets the enabled state of all main memory sensors of the specified type(s).
 *
 * @param i_sensor_type Sensor type(s) to enable or disable.  Contains one or
 *                      more values of the AMEC_SENSOR_TYPE enum OR'd together.
 * @param i_enabled True to enable the sensors, false to disable the sensors
 */
void main_mem_sensors_set_enabled(AMEC_SENSOR_TYPE i_sensor_type, bool i_enabled);

#endif // _SENSOR_MAIN_MEMORY_H
