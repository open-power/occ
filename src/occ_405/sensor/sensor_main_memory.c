/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor_main_memory.c $                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017,2020                        */
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

/**
 * @file sensor_main_memory.c
 *
 * This file defines the functions and global variables for copying a subset of
 * the OCC sensors to main memory.  See the header file for more information.
 */

//******************************************************************************/
// Includes
//******************************************************************************/
#include <sensor_main_memory.h>     // Primary header
#include <stdint.h>                 // For uint*_t
#include <string.h>                 // For memset(), memcpy()
#include <sensor.h>                 // For sensor data structures
#include <common_types.h>           // For bool
#include <dcom.h>                   // For G_occ_role, OCC_SLAVE
#include <occ_common.h>             // For size_t, DMA_BUFFER, STATIC_ASSERT
#include <occ_service_codes.h>      // For OCC reason codes
#include <sensor_service_codes.h>   // For sensor module ids
#include <trac.h>                   // For trace macros
#include <errl.h>                   // For error logging functions and types
#include <occhw_async.h>            // For bce_request_*(), async_request_is_idle()
#include <cmdh_fsp_cmds.h>          // For G_apss_ch_to_function


//******************************************************************************/
// Main Memory Sensors - Private Defines/Structs/Globals
//******************************************************************************/


/**
 * Macro to build one main_mem_sensor_t instance.
 */
#define MAIN_MEM_SENSOR(gsid, smf_mode, master_only) \
    { gsid, smf_mode, master_only, true, true, MM_SENSOR_NAMES_STRUCT_VERSION_FULL }

/**
 * Macro to build main_mem_sensor_t instances for all processor cores.
 */
#define MAIN_MEM_CORE_SENSORS(gsid_prefix, smf_mode, master_only) \
    MAIN_MEM_SENSOR(gsid_prefix##0  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##1  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##2  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##3  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##4  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##5  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##6  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##7  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##8  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##9  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##10 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##11 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##12 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##13 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##14 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##15 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##16 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##17 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##18 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##19 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##20 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##21 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##22 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##23 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##24 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##25 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##26 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##27 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##28 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##29 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##30 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##31 , smf_mode, master_only)

/**
 * Macro to build main_mem_sensor_t instance for all quads
 */
#define MAIN_MEM_QUAD_SENSORS(gsid_prefix, smf_mode, master_only) \
    MAIN_MEM_SENSOR(gsid_prefix##0 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##1 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##2 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##3 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##4 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##5 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##6 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##7 , smf_mode, master_only)

/**
 * Macro to build main_mem_sensor_t instances for all memory controllers.
 */
#define MAIN_MEM_MEMORY_SENSORS(gsid_prefix, smf_mode, master_only) \
    MAIN_MEM_SENSOR(gsid_prefix##0 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##1 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##2 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##3 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##4 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##5 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##6 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##7 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##8 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##9 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##10, smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##11, smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##12, smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##13, smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##14, smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##15, smf_mode, master_only)

/**
 * Macro to build main_mem_sensor_t instances for all APSS channels.
 */
#define MAIN_MEM_APSSCH_SENSORS(gsid_prefix, smf_mode, master_only) \
    MAIN_MEM_SENSOR(gsid_prefix##0  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##1  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##2  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##3  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##4  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##5  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##6  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##7  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##8  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##9  , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##10 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##11 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##12 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##13 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##14 , smf_mode, master_only) , \
    MAIN_MEM_SENSOR(gsid_prefix##15 , smf_mode, master_only)

/**
 * Array of main memory sensors.  This is the set of OCC sensors that will be
 * copied to main memory.
 *
 * Note the array is NOT indexed by global sensor ID.  This is because only a
 * subset of OCC sensors are copied to main memory, and the sensors are ordered
 * differently (by type).
 *
 * The sensors MUST be grouped by sensor type (AMEC_SENSOR_TYPE).  Sensors are
 * enabled/disabled by type, so we want sensors of the same type in contiguous
 * memory in the Sensor Readings Buffer.  This will minimize the number of BCE
 * copy operations needed to update the sensor readings when some sensors are
 * disabled.
 *
 * NOTE: When sensors are added/removed, be sure to also update
 *       MAIN_MEM_NUM_SENSORS in sensor_main_memory.h
 */
main_mem_sensor_t G_main_mem_sensors[] =
{
    // AMEC_SENSOR_TYPE_CURRENT:  gsid            smf_mode  master_only
    MAIN_MEM_SENSOR              (CURVDD,         true,     false),
    MAIN_MEM_SENSOR              (CURVDN,         true,     false),

    // AMEC_SENSOR_TYPE_VOLTAGE:  gsid            smf_mode  master_only
    MAIN_MEM_SENSOR              (VOLTVDD,        true,     false),
    MAIN_MEM_SENSOR              (VOLTVDDSENSE,   true,     false),
    MAIN_MEM_SENSOR              (VOLTVDN,        true,     false),
    MAIN_MEM_SENSOR              (VOLTVCSSENSE,   true,     false),
    MAIN_MEM_CORE_SENSORS        (VOLTDROOPCNTC,  true,     false),

    // AMEC_SENSOR_TYPE_TEMP:     gsid            smf_mode  master_only
    MAIN_MEM_SENSOR              (TEMPNEST0,      false,    false),
    MAIN_MEM_SENSOR              (TEMPNEST1,      false,    false),
    MAIN_MEM_SENSOR              (TEMPPROCIOTHRM, false,    false),
    MAIN_MEM_SENSOR              (TEMPVDD,        false,    false),
    MAIN_MEM_CORE_SENSORS        (TEMPPROCTHRMC,  false,    false),
    MAIN_MEM_MEMORY_SENSORS      (TEMPMEMBUF,     false,    false),
    MAIN_MEM_SENSOR              (TEMPDIMMTHRM,   false,    false),
    MAIN_MEM_SENSOR              (TEMPGPU0,       false,    false),
    MAIN_MEM_SENSOR              (TEMPGPU1,       false,    false),
    MAIN_MEM_SENSOR              (TEMPGPU2,       false,    false),
    MAIN_MEM_SENSOR              (TEMPGPU0MEM,    false,    false),
    MAIN_MEM_SENSOR              (TEMPGPU1MEM,    false,    false),
    MAIN_MEM_SENSOR              (TEMPGPU2MEM,    false,    false),

    // AMEC_SENSOR_TYPE_UTIL:     gsid            smf_mode  master_only
    MAIN_MEM_CORE_SENSORS        (UTILC,          false,    false),
    MAIN_MEM_SENSOR              (UTIL,           false,    false),
    MAIN_MEM_CORE_SENSORS        (NUTILC,         false,    false),

    // AMEC_SENSOR_TYPE_FREQ:     gsid            smf_mode  master_only
    MAIN_MEM_SENSOR              (FREQA,          true,     false),
    MAIN_MEM_CORE_SENSORS        (FREQAC,         true,     false),

    // AMEC_SENSOR_TYPE_POWER:    gsid            smf_mode  master_only
    MAIN_MEM_SENSOR              (PWRSYS,         true,     true ),
    MAIN_MEM_SENSOR              (PWRGPU,         true,     false),
    MAIN_MEM_APSSCH_SENSORS      (PWRAPSSCH,      true,     true ),
    MAIN_MEM_SENSOR              (PWRPROC,        true,     false),
    MAIN_MEM_SENSOR              (PWRVDD,         true,     false),
    MAIN_MEM_SENSOR              (PWRVDN,         true,     false),
    MAIN_MEM_SENSOR              (PWRMEM,         true,     false),

    // AMEC_SENSOR_TYPE_PERF:     gsid            smf_mode  master_only
    MAIN_MEM_SENSOR              (IPS,            false,    false),
    MAIN_MEM_CORE_SENSORS        (STOPDEEPACTC,   true,     false),
    MAIN_MEM_CORE_SENSORS        (STOPDEEPREQC,   true,     false),
    MAIN_MEM_CORE_SENSORS        (IPSC,           false,    false),
    MAIN_MEM_CORE_SENSORS        (NOTBZEC,        false,    false),
    MAIN_MEM_CORE_SENSORS        (NOTFINC,        false,    false),
    MAIN_MEM_SENSOR              (PROCPWRTHROT,   false,    false),
    MAIN_MEM_SENSOR              (PROCOTTHROT,    false,    false),
    MAIN_MEM_SENSOR              (MEMOTTHROT,     false,    false),
    MAIN_MEM_MEMORY_SENSORS      (MRDM,           false,    false),
    MAIN_MEM_MEMORY_SENSORS      (MWRM,           false,    false),
    MAIN_MEM_MEMORY_SENSORS      (MEMSPSTATM,     true,     false),
    MAIN_MEM_MEMORY_SENSORS      (MEMSPM,         false,    false),
    MAIN_MEM_SENSOR              (DDSAVG,         false,    false),
    MAIN_MEM_SENSOR              (DDSMIN,         false,    false),

    // NOTE: update MAIN_MEM_NUM_SENSORS when sensors are added/removed
};

/*
 * Check if Sensor Names Block is too small to hold all of the main memory
 * sensors.  If so generate a compile-time error.
 */
#define MM_SENSOR_NAMES_SIZE_NEEDED \
    (MAIN_MEM_SENSOR_COUNT * sizeof(mm_sensor_names_entry_t))
STATIC_ASSERT( (MM_SENSOR_NAMES_SIZE_NEEDED > MM_SENSOR_NAMES_SIZE) )

/*
 * Check if Sensor Readings Buffer is too small to hold all of the main memory
 * sensors.  If so generate a compile-time error.
 *
 * Note: This assumes the 'worst case' scenario where all main memory sensors
 * use the full sensor readings structure.  In reality a few of the main memory
 * sensors use the smaller counter sensor readings structure.  If full accuracy
 * is required, a new #define could be created that contained the number of
 * sensors using the counter structure.  The equation could then use this new
 * #define to calculate the exact number of bytes needed.
 */
#define MM_SENSOR_READINGS_SIZE_NEEDED \
    (sizeof(mm_sensor_readings_buf_header_t) + \
     (MAIN_MEM_SENSOR_COUNT * sizeof(mm_sensor_readings_full_t)))
STATIC_ASSERT( (MM_SENSOR_READINGS_SIZE_NEEDED > MM_SENSOR_READINGS_SIZE) )


//******************************************************************************
// Block Copy Engine (BCE) - Private Defines/Globals
//******************************************************************************

/**
 * Size of the buffer used with the Block Copy Engine.  4kB is the maximum size
 * supported by the BCE.
 */
#define MM_SENSORS_BCE_BUF_SIZE  4096

/**
 * Buffer that contains sensor data to copy to main memory using the BCE.  Use
 * DMA_BUFFER macro so buffer is 128-byte aligned as required by the BCE.
 */
DMA_BUFFER(uint8_t G_mm_sensors_bce_buffer[MM_SENSORS_BCE_BUF_SIZE]) = {0};

/**
 * BCE request structure.  Used by BCE functions to schedule copy request.
 */
BceRequest G_mm_sensors_bce_req;

/**
 * Specifies whether the BCE request was scheduled.  If false, the request
 * finished or has never been scheduled/initialized.
 */
bool G_mm_sensors_bce_req_scheduled = false;

/**
 * Minimum number of bytes that can be written by the BCE.  Since the BCE write
 * size has to be a multiple of 128, the minimum is 128.
 */
#define MM_SENSORS_MIN_WRITE_SIZE  128

/**
 * Buffer used to save 128 bytes from a BCE write so they can later be modified.
 *
 * The minimum write size of the Block Copy Engine is 128 bytes.  This is
 * problematic when trying to write less than 128 bytes, such as when modifying
 * one field in a previously written data structure in main memory.
 *
 * To work around this limitation, a 128 byte range can be saved in this buffer.
 * The saved bytes can be modified later, copied back into the BCE buffer, and
 * re-written to main memory.
 *
 * Example:
 *   BCE Buffer Contents:  AABBBBCDEE...
 *   BCE Buffer Offset:    0123456789...
 *                               ^
 *                               |
 *                               field to modify later
 *
 *   AA is the two byte value of field A, BBBB is the four byte value of field
 *   B, etc.
 *
 *   We want to later change the value of field C at offset 6.  However, we must
 *   write at least 128 bytes.  To do this, we need to save the values of the
 *   bytes before this field (offset 0-5) and after this field (offset 7-127).
 */
uint8_t G_mm_sensors_save_buffer[MM_SENSORS_MIN_WRITE_SIZE] = {0};

/**
 * Buffer used to save the last 128 bytes written by the BCE.
 *
 * The Block Copy Engine requires the main memory address to be 128-byte
 * aligned.  This is problematic when trying to write to a non-aligned address,
 * such as when writing data structures whose size is not a multiple of 128.
 *
 * To work around this limitation the following technique is used:
 *   - The BCE buffer is always associated with a 128-byte aligned address.
 *   - Data structures are stored in the BCE buffer until it is full.
 *   - No partial data structures are stored in the BCE buffer.  The buffer is
 *     padded with zero bytes at the end if needed.
 *   - The BCE buffer is copied to main memory.
 *   - The last 128 bytes of the BCE buffer are stored in G_mm_sensors_last_write_buf.
 *   - The BCE buffer is cleared.
 *   - The 128 bytes in G_mm_sensors_last_write_buf are copied to the beginning
 *     of the BCE buffer.
 *   - The BCE buffer is now associated with the address of the 128 byte range
 *     stored in G_mm_sensors_last_write_buf.
 *   - More data structures are stored in the BCE buffer.  If the first data
 *     structure stored has a non-aligned address, the data structure will be
 *     stored at a non-zero offset from the beginning of the BCE buffer.  The
 *     preceding bytes in the BCE buffer will have the correct contents due to
 *     G_mm_sensors_last_write_buf.
 *
 * Example:
 *   BCE Buffer Contents:  ...XXXXX...XXXXX000
 *                                         ^
 *                                         |
 *                                         offset of next data structure
 *
 *   Bytes marked with an 'X' represent data structures stored in the last 128
 *   bytes of the BCE buffer.  Bytes marked with a '0' represent the remaining
 *   empty space in the buffer.  Those bytes contain the value 0x00.
 *
 *   Only 3 bytes of empty space remain in the BCE buffer.  The next data
 *   structure to write requires 48 bytes.  We do not write partial data
 *   structures, so we will wait until the next BCE operation to write that data
 *   structure.
 *
 *   The address for the next BCE operation must be 128-byte aligned.  The
 *   address where the next data structure will be stored is NOT 128-byte
 *   aligned.  To work around this, we save the last 128 bytes of the BCE
 *   buffer.  When we start the next BCE operation, we copy those 128 bytes into
 *   the beginning of the BCE buffer.
 *
 *   Next BCE Buffer Contents:  XXXXX...XXXXX000...
 *                              ^            ^
 *                              |            |
 *                              |            offset of next data structure
 *                              128-byte aligned address
 */
uint8_t G_mm_sensors_last_write_buf[MM_SENSORS_MIN_WRITE_SIZE] = {0};

/**
 * Main memory address associated with the bytes in G_mm_sensors_last_write_buf.
 */
uint32_t G_mm_sensors_last_write_buf_addr = 0x00000000;

/**
 * Returns the specified number aligned to a 128-byte boundary.  Rounds the
 * number up to the next multiple of 128 if needed.
 *
 * This macro is used when calculating how many bytes to write using the BCE.
 * The BCE write length must be a multiple of 128.
 */
#define MM_SENSORS_ALIGN_128_UP(number) \
    ((((number) % 128) == 0) ? (number) : ((number) + (128 - ((number) % 128))))

/**
 * Returns the specified number aligned to a 128-byte boundary.  Rounds the
 * number down to the previous multiple of 128 if needed.
 *
 * This macro is used when determining the main memory address for a BCE write.
 * The main memory address must be 128-byte aligned.
 */
#define MM_SENSORS_ALIGN_128_DOWN(number) \
    ((((number) % 128) == 0) ? (number) : ((number) - ((number) % 128)))


//******************************************************************************
// Other Private Defines/Enums/Structs/Globals
//******************************************************************************

/**
 * States of the main memory sensors system.  The state name indicates the
 * action that needs to be taken.
 */
typedef enum
{
    MM_SENSORS_WRITE_DATA_HEADER,         ///< Write Sensor Data Header Block
    MM_SENSORS_WRITE_SENSOR_NAMES,        ///< Write Sensor Names Block
    MM_SENSORS_VALIDATE_DATA_HEADER,      ///< Set valid field in Data Header Block
    MM_SENSORS_WRITE_SENSOR_READINGS,     ///< Write Sensor Readings Buffer
    MM_SENSORS_VALIDATE_SENSOR_READINGS,  ///< Set valid field in Readings Buffer
} MM_SENSORS_STATE;

/**
 * Current state of the main memory sensors system
 */
uint8_t G_mm_sensors_state = MM_SENSORS_WRITE_DATA_HEADER;

/**
 * Address of the current Sensor Readings Buffer.  Value will be either
 * MM_SENSOR_READINGS_PING_ADDRESS or MM_SENSOR_READINGS_PONG_ADDRESS.
 * main_mem_sensors_update() alternates between the ping and pong buffer.
 *
 * Note the value cannot be initialized here (at compile time) because the
 * address is dependent on which OCC we are on.
 */
uint32_t G_mm_sensor_readings_buf_addr;

/**
 * Offset to the first sensor readings in a Sensor Readings Buffer.  They are
 * located after the Sensor Readings Buffer Header.
 */
#define MM_SENSORS_FIRST_READINGS_OFFSET  sizeof(mm_sensor_readings_buf_header_t)


//******************************************************************************
// Public Globals (see description in header file)
//******************************************************************************

bool G_main_mem_sensors_initialized = false;

bool G_smf_mode = false;


//******************************************************************************
// Private Functions
//******************************************************************************

/**
 * Initializes internal data structures.
 */
void mm_sensors_init_internals(void)
{
    // Initialize address of current Sensor Readings Buffer address.  Start with
    // ping buffer.  main_mem_sensors_update() alternates between ping and pong.
    G_mm_sensor_readings_buf_addr = MM_SENSOR_READINGS_PING_ADDRESS;

    // Initialize the array of main memory sensors.  Set sensor fields that
    // cannot be initialized at compile time.
    uint16_t l_index;
    for (l_index = 0; l_index < MAIN_MEM_SENSOR_COUNT; ++l_index)
    {
        main_mem_sensor_t * l_mm_sensor = &G_main_mem_sensors[l_index];

        // Set valid field based on SMF mode and OCC role
        if (G_smf_mode && !l_mm_sensor->smf_mode)
        {
            // SMF mode is enabled and sensor is not copied when in SMF mode
            l_mm_sensor->valid = false;
        }
        else if ((G_occ_role == OCC_SLAVE) && l_mm_sensor->master_only)
        {
            // OCC is a slave and sensor is only available from the OCC master
            l_mm_sensor->valid = false;
        }
        else
        {
            l_mm_sensor->valid = true;
        }

        // Set struct_ver field based on units field in corresponding sensor_info_t.
        // Units value of "#" means use counter structure for sensor readings.
        uint16_t l_gsid = l_mm_sensor->gsid;
        const char * l_units = G_sensor_info[l_gsid].sensor.units;
        if ((l_units[0] == '#') && (l_units[1] == '\0'))
        {
            l_mm_sensor->struct_ver = MM_SENSOR_NAMES_STRUCT_VERSION_COUNTER;
        }
        else
        {
            l_mm_sensor->struct_ver = MM_SENSOR_NAMES_STRUCT_VERSION_FULL;
        }
    }
}


/**
 * Returns the number of valid main memory sensors for this OCC.  Static sensor
 * data is only copied to main memory when the sensor is valid.  Some sensors
 * are invalid if SMF mode is enabled or the OCC is a slave.
 *
 * @return Number of valid main memory sensors
 */
uint16_t mm_sensors_valid_sensor_count(void)
{
    uint16_t l_count = 0;

    // Loop through all main memory sensors
    uint16_t l_index;
    for (l_index = 0; l_index < MAIN_MEM_SENSOR_COUNT; ++l_index)
    {
        if (G_main_mem_sensors[l_index].valid)
        {
            ++l_count;
        }
    }

    return l_count;
}


/**
 * Returns the number of main memory sensors that are both valid and enabled.
 * Sensor readings (dynamic data) are only copied to main memory when the sensor
 * is valid and enabled.
 *
 * Sensors may be invalid due to the SMF Mode or OCC role.  Sensors are enabled
 * or disabled dynamically by external software such as OPAL.
 *
 * @return Number of main memory sensors that are both valid and enabled
 */
uint16_t mm_sensors_valid_enabled_sensor_count(void)
{
    uint16_t l_count = 0;

    // Loop through all main memory sensors
    uint16_t l_index;
    for (l_index = 0; l_index < MAIN_MEM_SENSOR_COUNT; ++l_index)
    {
        if (G_main_mem_sensors[l_index].valid && G_main_mem_sensors[l_index].enabled)
        {
            ++l_count;
        }
    }

    return l_count;
}


/**
 * Logs an error caused by the Block Copy Engine.  Does nothing if a BCE error
 * has already been logged.
 *
 * Note that the required error log comment containing tags like 'userdata4' and
 * 'devdesc' must be located by the call to this function.  It is not located
 * inside this function because the value of those tags varies.
 *
 * @param i_modId         Module ID
 * @param i_extReasonCode Extended reason code
 * @param i_userData1     Userdata1 value
 * @param i_userData2     Userdata2 value
 */
void mm_sensors_log_bce_error(uint16_t i_modId, uint16_t i_extReasonCode,
                              uint32_t i_userData1, uint32_t i_userData2)
{
    static bool L_error_logged = false;
    if (!L_error_logged)
    {
        // Create and commit error
        errlHndl_t l_errl = createErrl(i_modId,                // Module ID
                                       SENSOR_MAIN_MEM_ERROR,  // Reason code
                                       i_extReasonCode,        // Extended reason code
                                       ERRL_SEV_INFORMATIONAL, // Severity
                                       NULL,                   // Trace Buffers
                                       DEFAULT_TRACE_SIZE,     // Trace Size
                                       i_userData1,            // Userdata1
                                       i_userData2);           // Userdata2
        commitErrl(&l_errl);
        L_error_logged = true;
    }
}


/**
 * Returns whether the global BCE request struct is idle and ready for re-use.
 * Returns true immediately if the request was not scheduled.  If the request
 * was scheduled, checks to see if it has finished.
 *
 * @param i_caller_mod_id Module ID of calling function in case an error occurs
 * @return True if BCE request is idle, false otherwise
 */
bool mm_sensors_is_bce_req_idle(uint16_t i_caller_mod_id)
{
    // Number of times we've waited for current request to finish
    static uint8_t L_wait_count = 0;

    // If the request was not previously scheduled, then it is idle.  This also
    // handles the case where the request has not been initialized yet.
    if (!G_mm_sensors_bce_req_scheduled)
    {
        return true;
    }

    // Request was scheduled; check if it finished and is now idle
    if (async_request_is_idle(&G_mm_sensors_bce_req.request))
    {
        // Request is now idle and ready for re-use
        G_mm_sensors_bce_req_scheduled = false;

        // If we were waiting for request to finish, trace and clear wait count
        if (L_wait_count > 0)
        {
            TRAC_INFO("mm_sensors_is_bce_req_idle: "
                      "Request finished after waiting %u times: caller=0x%04X",
                      L_wait_count, i_caller_mod_id);
            L_wait_count = 0;
        }
        return true;
    }

    // Request was scheduled but has not finished.  Increment wait count unless
    // we are already at the max (to avoid overflow).
    if (L_wait_count < UINT8_MAX)
    {
        ++L_wait_count;
    }

    // If this is the first time we've waited for this request, trace it
    if (L_wait_count == 1)
    {
        TRAC_INFO("mm_sensors_is_bce_req_idle: "
                  "Waiting for request to finish: caller=0x%04X",
                  i_caller_mod_id);
    }

    // If this is the second time we've waited for this request, log BCE error
    if (L_wait_count == 2)
    {
        /* @
         * @errortype
         * @moduleid    MM_SENSORS_IS_BCE_REQ_IDLE_MOD
         * @reasoncode  SENSOR_MAIN_MEM_ERROR
         * @userdata1   Caller module ID
         * @userdata2   0
         * @userdata4   ERC_GENERIC_TIMEOUT
         * @devdesc     BCE request not finished after waiting twice
         */
        mm_sensors_log_bce_error(MM_SENSORS_IS_BCE_REQ_IDLE_MOD, ERC_GENERIC_TIMEOUT,
                                 i_caller_mod_id, 0);
    }

    // Return false since request is not idle
    return false;
}


/**
 * Copies the specified number of bytes from G_mm_sensors_bce_buffer to
 * the specified main memory address using the Block Copy Engine (BCE).
 *
 * Schedules the copy request with the BCE but does not block waiting for the
 * request to finish.  Call mm_sensors_is_bce_req_idle() to determine when the
 * request is finished and the copying has occurred.
 *
 * This function must not be called if a previous copy request has not yet
 * finished.
 *
 * @param i_main_mem_addr Main memory address to which bytes will be copied.
 *                        Must be 128-byte aligned.
 * @param i_byte_count    Number of bytes to copy.  Must be multiple of 128.
 *                        Must be <= MM_SENSORS_BCE_BUF_SIZE.  If 0 specified,
 *                        function will return true and will not use the BCE.
 * @param i_caller_mod_id Module ID of the calling function in case an error occurs
 * @return True if BCE request was successfully scheduled, false otherwise
 */
bool mm_sensors_bce_copy(uint32_t i_main_mem_addr, size_t i_byte_count,
                         uint16_t i_caller_mod_id)
{
    // Verify main memory address and byte count are valid
    static bool L_traced_param_error = false;
    if (((i_main_mem_addr % 128) != 0) || ((i_byte_count % 128) != 0) ||
        (i_byte_count > MM_SENSORS_BCE_BUF_SIZE))
    {
        if (!L_traced_param_error)
        {
            TRAC_ERR("mm_sensors_bce_copy: Input parameter error: "
                     "address=0x%08X length=%u caller=0x%04X",
                     i_main_mem_addr, i_byte_count, i_caller_mod_id);
            L_traced_param_error = true;
        }
        return false;
    }

    // Check if a copy request was previously scheduled and is not yet finished
    static bool L_traced_sched_error = false;
    if (G_mm_sensors_bce_req_scheduled)
    {
        if (!L_traced_sched_error)
        {
            TRAC_ERR("mm_sensors_bce_copy: Previous request not finished: caller=0x%04X",
                     i_caller_mod_id);
            L_traced_sched_error = true;
        }
        return false;
    }

    // Check if byte count is 0.  If so return true and don't use BCE.
    if (i_byte_count == 0)
    {
        return true;
    }

    // Create BCE request
    int l_rc = bce_request_create(&G_mm_sensors_bce_req,    // Block copy request
                                  &G_pba_bcue_queue,        // Queue (SRAM up to mainstore)
                                  i_main_mem_addr,          // Mainstore address
                       (uint32_t) &G_mm_sensors_bce_buffer, // SRAM start address
                                  i_byte_count,             // Size of copy
                                  SSX_WAIT_FOREVER,         // No timeout
                                  NULL,                     // No call back
                                  NULL,                     // No call back args
                                  0x00);                    // No options; non-blocking
    if (l_rc != SSX_OK)
    {
        TRAC_ERR("mm_sensors_bce_copy: Request create failure: rc=0x%08X caller=0x%04X",
                 -l_rc, i_caller_mod_id);
        /* @
         * @errortype
         * @moduleid    MM_SENSORS_BCE_COPY_MOD
         * @reasoncode  SENSOR_MAIN_MEM_ERROR
         * @userdata1   Return code from bce_request_create()
         * @userdata2   Caller module ID
         * @userdata4   ERC_BCE_REQUEST_CREATE_FAILURE
         * @devdesc     Failed to create BCE request
         */
        mm_sensors_log_bce_error(MM_SENSORS_BCE_COPY_MOD, ERC_BCE_REQUEST_CREATE_FAILURE,
                                 -l_rc, i_caller_mod_id);
        return false;
    }

    // Schedule BCE request
    l_rc = bce_request_schedule(&G_mm_sensors_bce_req);
    if (l_rc != SSX_OK)
    {
        TRAC_ERR("mm_sensors_bce_copy: Request schedule failure: rc=0x%08X caller=0x%04X",
                 -l_rc, i_caller_mod_id);
        /* @
         * @errortype
         * @moduleid    MM_SENSORS_BCE_COPY_MOD
         * @reasoncode  SENSOR_MAIN_MEM_ERROR
         * @userdata1   Return code from bce_request_schedule()
         * @userdata2   Caller module ID
         * @userdata4   ERC_BCE_REQUEST_SCHEDULE_FAILURE
         * @devdesc     Failed to schedule BCE request
         */
        mm_sensors_log_bce_error(MM_SENSORS_BCE_COPY_MOD, ERC_BCE_REQUEST_SCHEDULE_FAILURE,
                                 -l_rc, i_caller_mod_id);
        return false;
    }

    // Successfully scheduled request.  Copy is not blocking, so need to check
    // whether it finished later.  Set flag indicating request is scheduled.
    G_mm_sensors_bce_req_scheduled = true;
    return true;
}


/**
 * Saves the last 128 bytes that were written using the BCE buffer.  The bytes
 * are stored in G_mm_sensors_last_write_buf.  The main memory address of the
 * last 128 bytes is stored in G_mm_sensors_last_write_buf_addr.
 *
 * See documentation by G_mm_sensors_last_write_buf for more details on how this
 * buffer is used.
 *
 * @param i_main_mem_addr Main memory address where BCE buffer was written.
 *                        Must be 128-byte aligned.
 * @param i_byte_count Number of bytes written.  Must be multiple of 128.
 */
void mm_sensors_save_last_write(uint32_t i_main_mem_addr, size_t i_byte_count)
{
    // Make sure at least 128 bytes were written
    if (i_byte_count >= MM_SENSORS_MIN_WRITE_SIZE)
    {
        // Calculate BCE buffer offset to last 128 bytes
        uint16_t l_offset = i_byte_count - MM_SENSORS_MIN_WRITE_SIZE;

        // Copy last 128 bytes into last write buffer
        memcpy(G_mm_sensors_last_write_buf, &G_mm_sensors_bce_buffer[l_offset],
               MM_SENSORS_MIN_WRITE_SIZE);

        // Save main memory address where last 128 bytes were written
        G_mm_sensors_last_write_buf_addr = i_main_mem_addr + l_offset;
    }
}


/**
 * Initializes the specified Sensor Data Header Block struct.
 *
 * @param o_header Header to initialize
 */
void mm_sensors_init_data_header(mm_sensor_data_header_block_t * o_header)
{
    // Get number of valid sensors that will be copied to main memory.  Some
    // sensors may be invalid due to the SMF mode or OCC role.
    uint16_t l_number_of_valid_sensors = mm_sensors_valid_sensor_count();

    // Zero out entire structure including reserved bytes
    memset(o_header, 0x00, sizeof(mm_sensor_data_header_block_t));

    // Set struct field values.  The valid field is set to false (0x00) because
    // the Sensor Names Block has not yet been written.
    o_header->valid                   = MM_SENSOR_DATA_HEADER_VALID_FALSE;
    o_header->header_version          = MM_SENSOR_DATA_HEADER_VERSION;
    o_header->number_of_sensors       = l_number_of_valid_sensors;
    o_header->sensor_readings_version = MM_SENSOR_READINGS_VERSION;
    o_header->sensor_names_offset     = MM_SENSOR_NAMES_OFFSET;
    o_header->sensor_names_version    = MM_SENSOR_NAMES_VERSION;
    o_header->bytes_per_sensor_name   = sizeof(mm_sensor_names_entry_t);
    o_header->ping_buffer_offset      = MM_SENSOR_READINGS_PING_OFFSET;
    o_header->pong_buffer_offset      = MM_SENSOR_READINGS_PONG_OFFSET;
}


/**
 * Writes the Sensor Data Header Block in main memory.  If successful, changes
 * the current state to MM_SENSORS_WRITE_SENSOR_NAMES.
 */
void mm_sensors_write_data_header(void)
{
    // Clear the BCE buffer
    memset(G_mm_sensors_bce_buffer, 0x00, MM_SENSORS_BCE_BUF_SIZE);

    // Initialize a mm_sensor_data_header_block_t struct
    mm_sensor_data_header_block_t l_header;
    mm_sensors_init_data_header(&l_header);

    // Copy struct bytes to BCE buffer
    memcpy(G_mm_sensors_bce_buffer, &l_header, sizeof(l_header));

    // Save the first 128 bytes of the BCE buffer.  Needed later by
    // mm_sensors_validate_data_header() to modify the valid field of the header.
    memcpy(G_mm_sensors_save_buffer, G_mm_sensors_bce_buffer, MM_SENSORS_MIN_WRITE_SIZE);

    // Calculate number of bytes to copy.  The BCE requires this to be a
    // multiple of 128.  May end up copying extra zeroed out bytes in BCE buffer.
    size_t l_byte_count = sizeof(l_header);
    l_byte_count = MM_SENSORS_ALIGN_128_UP(l_byte_count);

    // Copy Sensor Data Header Block to main memory using BCE
    if (mm_sensors_bce_copy(MM_SENSOR_DATA_HEADER_ADDRESS, l_byte_count,
                            MM_SENSORS_WRITE_DATA_HDR_MOD))
    {
        // Copy succeeded.  Change state to write Sensor Names Block.
        G_mm_sensors_state = MM_SENSORS_WRITE_SENSOR_NAMES;
    }
}


/**
 * Validates the Sensor Data Header Block in main memory.
 *
 * Sets the valid field in the Sensor Data Header Block to true indicating that
 * the Sensor Names Block has been written.
 *
 * If successful:
 *   - Sets G_main_mem_sensors_initialized to true
 *   - Changes current state to MM_SENSORS_WRITE_SENSOR_READINGS.
 */
void mm_sensors_validate_data_header(void)
{
    // Clear the BCE buffer
    memset(G_mm_sensors_bce_buffer, 0x00, MM_SENSORS_BCE_BUF_SIZE);

    // We need to modify the valid field of the Sensor Data Header Block.
    // However, the BCE does not support writing a single byte.  The minimum
    // write size is 128 bytes.  To work around this, we saved the first 128
    // bytes of header data in mm_sensors_write_data_header().  Copy those saved
    // bytes back into the BCE buffer.
    memcpy(G_mm_sensors_bce_buffer, G_mm_sensors_save_buffer, MM_SENSORS_MIN_WRITE_SIZE);

    // Cast BCE buffer to a mm_sensor_data_header_block_t *
    mm_sensor_data_header_block_t * l_header =
        (mm_sensor_data_header_block_t *) (&G_mm_sensors_bce_buffer);

    // Set the valid field to true since Sensor Names Block has been written
    l_header->valid = MM_SENSOR_DATA_HEADER_VALID_TRUE;

    // Copy first 128 bytes of Sensor Data Header Block to main memory using BCE
    if (mm_sensors_bce_copy(MM_SENSOR_DATA_HEADER_ADDRESS, MM_SENSORS_MIN_WRITE_SIZE,
                            MM_SENSORS_VALIDATE_DATA_HDR_MOD))
    {
        // Copy succeeded.  Set global indicating all initialization complete.
        G_main_mem_sensors_initialized = true;

        // Change state to write to Sensor Readings Buffer
        G_mm_sensors_state = MM_SENSORS_WRITE_SENSOR_READINGS;
    }
}


/**
 * Initializes the specified Sensor Names Block entry.  Sets entry fields to
 * the static sensor data for the specified main memory sensor.
 *
 * @param i_mm_sensor Main memory sensor
 * @param i_readings_offset Offset to sensor readings in Sensor Readings Buffer
 * @param o_entry Sensor Names Block entry to initialize
 */
void mm_sensors_init_names_entry(const main_mem_sensor_t * i_mm_sensor,
                                 uint32_t i_readings_offset,
                                 mm_sensor_names_entry_t * o_entry)
{
    // Get GSID of main memory sensor
    uint16_t l_gsid = i_mm_sensor->gsid;

    // Find the sensor specific info, if any
    uint8_t l_sensor_specific_info1 = 0;
    if ((l_gsid >= PWRAPSSCH0) && (l_gsid <= PWRAPSSCH15))
    {
        // For APSS channel sensors, set to ADC func ID
        uint8_t l_channel_num = (l_gsid - PWRAPSSCH0);
        l_sensor_specific_info1 = G_apss_ch_to_function[l_channel_num];
    }

    // Get sensor_info_t struct with static data for this sensor
    const sensor_info_t * l_sensor_info = &G_sensor_info[l_gsid];

    // Zero out entire entry structure including reserved bytes
    memset(o_entry, 0x00, sizeof(mm_sensor_names_entry_t));

    // Set entry struct field values
    memcpy(o_entry->name,  l_sensor_info->name,         MAX_SENSOR_NAME_SZ);
    memcpy(o_entry->units, l_sensor_info->sensor.units, MAX_SENSOR_UNIT_SZ);
    o_entry->scale_factor             = l_sensor_info->sensor.scalefactor;
    o_entry->gsid                     = l_gsid;
    o_entry->freq                     = l_sensor_info->sensor.freq;
    o_entry->type                     = l_sensor_info->sensor.type;
    o_entry->location                 = l_sensor_info->sensor.location;
    o_entry->sensor_structure_version = i_mm_sensor->struct_ver;
    o_entry->reading_offset           = i_readings_offset;
    o_entry->sensor_specific_info1    = l_sensor_specific_info1;
}


/**
 * Writes static sensor data to the Sensor Names Block in main memory.
 *
 * Only valid sensors are written to the Sensor Names Block.  Sensors may be
 * invalid due to the SMF Mode or OCC role.
 *
 * This function must be called repeatedly until the entire Sensor Names Block
 * has been written.  The BCE can copy a maximum of 4kB at a time, and the
 * Sensor Names Block is up to 50kB.  Non-blocking copies are performed, so each
 * time this function is called it will schedule a copy of up to 4kB.  Static
 * variables are used to save state across function calls.
 *
 * When using the BCE, the main memory address and byte count must both be
 * 128-byte aligned.  The Sensor Names Block entries are not a multiple of 128
 * bytes in length.  If the BCE buffer is nearly full and does not have enough
 * room for another complete Sensor Names Block entry, the entry will not be
 * partially written.  The BCE buffer will be padded at the end with zeroes, and
 * the entry will be written next time the function is called.
 *
 * When calling this function more than once, the main memory address of the
 * next Sensor Names Block entry will probably not be 128-byte aligned.  The
 * buffer G_mm_sensors_last_write_buf is used to work around this.  See the
 * doxygen comments by G_mm_sensors_last_write_buf for more information.
 *
 * When the Sensor Names Block has been completely written this function changes
 * the current state to MM_SENSORS_VALIDATE_DATA_HEADER.
 */
void mm_sensors_write_sensor_names(void)
{
    // Saved state from previous function call
    static uint16_t L_saved_sensor_index    = 0;
    static uint32_t L_saved_entry_offset    = 0;
    static uint32_t L_saved_readings_offset = MM_SENSORS_FIRST_READINGS_OFFSET;

    // Load saved state from previous function call into local variables
    uint16_t l_sensor_index    = L_saved_sensor_index;
    uint32_t l_entry_offset    = L_saved_entry_offset;
    uint32_t l_readings_offset = L_saved_readings_offset;

    // Calculate main memory address for writing to the Sensor Names Block.  The
    // address must be 128-byte aligned.
    uint32_t l_entry_addr = MM_SENSOR_NAMES_ADDRESS + l_entry_offset;
    uint32_t l_write_addr = MM_SENSORS_ALIGN_128_DOWN(l_entry_addr);

    // Calculate offset within BCE buffer where current entry should be stored.
    // Since the write address must be 128-byte aligned, the current entry might
    // not start at the beginning of the BCE buffer.
    uint16_t l_bce_buf_offset = l_entry_addr - l_write_addr;

    // Clear the BCE buffer
    memset(G_mm_sensors_bce_buffer, 0x00, MM_SENSORS_BCE_BUF_SIZE);

    // If we previously wrote bytes to the same 128-byte aligned address, copy
    // those bytes from the last write buffer.  This will retain the value of
    // the bytes that precede the current names entry.
    if (G_mm_sensors_last_write_buf_addr == l_write_addr)
    {
        memcpy(G_mm_sensors_bce_buffer, G_mm_sensors_last_write_buf,
               MM_SENSORS_MIN_WRITE_SIZE);
    }

    // Loop through main memory sensors storing sensor names entries in BCE buffer
    mm_sensor_names_entry_t l_entry;
    size_t l_entry_size = sizeof(l_entry);
    for (; l_sensor_index < MAIN_MEM_SENSOR_COUNT; ++l_sensor_index)
    {
        // Exit loop if BCE buffer does not have room for this entry
        if ((l_bce_buf_offset + l_entry_size) > MM_SENSORS_BCE_BUF_SIZE)
        {
            break;
        }

        // Skip sensor if it is not valid
        main_mem_sensor_t * l_mm_sensor = &G_main_mem_sensors[l_sensor_index];
        if (!l_mm_sensor->valid)
        {
            continue;
        }

        // Store sensor names entry in BCE buffer
        mm_sensors_init_names_entry(l_mm_sensor, l_readings_offset, &l_entry);
        memcpy(&G_mm_sensors_bce_buffer[l_bce_buf_offset], &l_entry, l_entry_size);
        l_bce_buf_offset  += l_entry_size;
        l_entry_offset    += l_entry_size;
        l_readings_offset += MM_SENSOR_READINGS_STRUCT_SIZE(l_mm_sensor->struct_ver);
    }

    // Calculate number of bytes to copy.  The BCE requires this to be a
    // multiple of 128.  May end up copying extra zeroed out bytes in buffer.
    size_t l_byte_count = MM_SENSORS_ALIGN_128_UP(l_bce_buf_offset);

    // Copy BCE buffer to Sensor Names Block in main memory
    if (mm_sensors_bce_copy(l_write_addr, l_byte_count, MM_SENSORS_WRITE_NAMES_MOD))
    {
        // Copy succeeded.  Save state for next time function called.
        L_saved_sensor_index    = l_sensor_index;
        L_saved_entry_offset    = l_entry_offset;
        L_saved_readings_offset = l_readings_offset;

        // Save last 128 bytes we wrote in case we need it for alignment
        mm_sensors_save_last_write(l_write_addr, l_byte_count);

        // If all sensors have been written to the Sensor Names Block
        if (l_sensor_index >= MAIN_MEM_SENSOR_COUNT)
        {
            // Change state to validate the Sensor Data Header Block
            G_mm_sensors_state = MM_SENSORS_VALIDATE_DATA_HEADER;
        }
    }
}


/**
 * Initializes the specified Sensor Readings Buffer Header struct.
 *
 * @param o_header Header to initialize
 */
void mm_sensors_init_readings_buf_header(mm_sensor_readings_buf_header_t * o_header)
{
    // Zero out entire header structure including reserved bytes
    memset(o_header, 0x00, sizeof(mm_sensor_readings_buf_header_t));

    // Set field values in Sensor Readings Buffer Header (currently only one field).
    // Set valid field to false.  Buffer is not valid until all readings copied.
    o_header->valid = MM_SENSOR_READINGS_VALID_FALSE;
}


/**
 * Skips the sensor readings for the current sensor.  Does not store them in the
 * BCE buffer.
 *
 * If the BCE buffer is currently empty, io_write_addr may be moved forward to
 * the address for the next sensor.  This avoids devoting BCE buffer space to
 * the skipped sensor.
 *
 * @param i_mm_sensor Main memory sensor to skip
 * @param io_readings_offset Offset to current sensor's readings within Sensor
 *                           Readings Buffer
 * @param io_write_addr Main memory address where the BCE buffer will be written
 * @param i_byte_count Number of bytes in BCE buffer
 */
void mm_sensors_skip_readings(const main_mem_sensor_t * i_mm_sensor,
                              uint32_t * io_readings_offset, uint32_t * io_write_addr,
                              size_t i_byte_count)
{
    // If sensor is not valid, there is nothing to do.  Invalid sensors are not
    // written to the Sensor Names Block or the Sensor Readings Buffer.
    if (!i_mm_sensor->valid)
    {
        return;
    }

    // Sensor is valid but not enabled.  It was written to the Sensor Names
    // Block and has an offset defined within the Sensor Readings Buffer.
    // However since it is not enabled we are not copying the readings.  Do not
    // modify i_byte_count since we are not storing readings in the BCE buffer.

    // Move the sensor readings offset to the next sensor
    *io_readings_offset += MM_SENSOR_READINGS_STRUCT_SIZE(i_mm_sensor->struct_ver);

    // Check if BCE buffer is empty.  If so, we can move the main memory address
    // associated with the BCE buffer forward to the next sensor.
    if (i_byte_count == 0)
    {
        // Calculate main memory address for writing the next sensor's readings.
        // The address must be 128-byte aligned.
        uint32_t l_next_readings_addr = G_mm_sensor_readings_buf_addr + *io_readings_offset;
        uint32_t l_new_write_addr = MM_SENSORS_ALIGN_128_DOWN(l_next_readings_addr);

        // If the new main memory address is different
        if (l_new_write_addr != *io_write_addr)
        {
            // Move forward to new write address
            *io_write_addr = l_new_write_addr;

            // Clear the first 128 bytes of the BCE buffer.  We may have copied in
            // the last 128 bytes of sensor readings that were previously written.
            // Those bytes are no longer correct since our address has changed.
            memset(G_mm_sensors_bce_buffer, 0x00, MM_SENSORS_MIN_WRITE_SIZE);
        }
    }
}


/**
 * Initializes the specified Full Sensor Readings Structure.  Obtains sensor
 * readings for the specified main memory sensor.
 *
 * @param i_mm_sensor Main memory sensor
 * @param o_readings Full sensor readings structure to initialize
 */
void mm_sensors_init_readings_full(const main_mem_sensor_t * i_mm_sensor,
                                   mm_sensor_readings_full_t * o_readings)
{
    // Get GSID of main memory sensor
    uint16_t l_gsid = i_mm_sensor->gsid;

    // Get pointer to corresponding sensor_t that holds the sensor readings
    const sensor_t * l_sensor = G_amec_sensor_list[l_gsid];

    // Zero out entire readings structure including reserved bytes
    memset(o_readings, 0x00, sizeof(mm_sensor_readings_full_t));

    // Set struct field values
    o_readings->gsid                = l_gsid;
    o_readings->timestamp           = l_sensor->timestamp;
    o_readings->sample              = l_sensor->sample;
    o_readings->sample_min          = l_sensor->sample_min;
    o_readings->sample_max          = l_sensor->sample_max;
    o_readings->csm_sample_min      = l_sensor->csm_sample_min;
    o_readings->csm_sample_max      = l_sensor->csm_sample_max;
    o_readings->profiler_sample_min = l_sensor->profiler_sample_min;
    o_readings->profiler_sample_max = l_sensor->profiler_sample_max;
    o_readings->job_s_sample_min    = l_sensor->job_s_sample_min;
    o_readings->job_s_sample_max    = l_sensor->job_s_sample_max;
    o_readings->accumulator         = l_sensor->accumulator;
    o_readings->update_tag          = l_sensor->update_tag;
}


/**
 * Initializes the specified Counter Sensor Readings Structure.  Obtains sensor
 * readings for the specified main memory sensor.
 *
 * @param i_mm_sensor Main memory sensor
 * @param o_readings Counter sensor readings structure to initialize
 */
void mm_sensors_init_readings_counter(const main_mem_sensor_t * i_mm_sensor,
                                      mm_sensor_readings_counter_t * o_readings)
{
    // Get GSID of main memory sensor
    uint16_t l_gsid = i_mm_sensor->gsid;

    // Get pointer to corresponding sensor_t that holds the sensor readings
    const sensor_t * l_sensor = G_amec_sensor_list[l_gsid];

    // Zero out entire readings structure including reserved bytes
    memset(o_readings, 0x00, sizeof(mm_sensor_readings_counter_t));

    // Set struct field values
    o_readings->gsid        = l_gsid;
    o_readings->timestamp   = l_sensor->timestamp;
    o_readings->accumulator = l_sensor->accumulator;
    o_readings->sample      = (l_sensor->sample > 0) ? 1 : 0;
}


/**
 * Stores sensor readings in the BCE buffer at the specified offset for the
 * specified sensor.
 *
 * @param i_mm_sensor Main memory sensor whose readings to store
 * @param i_bce_buf_offset Offset in BCE buffer where readings should be stored
 * @param io_readings_offset Offset to current sensor's readings within Sensor
 *                           Readings Buffer
 * @param o_byte_count Number of bytes in BCE buffer
 */
void mm_sensors_store_readings(const main_mem_sensor_t * i_mm_sensor,
                               uint16_t i_bce_buf_offset, uint32_t * io_readings_offset,
                               size_t * o_byte_count)
{
    // Store the correct sensor readings structure version
    size_t l_readings_size = 0;
    if (i_mm_sensor->struct_ver == MM_SENSOR_NAMES_STRUCT_VERSION_FULL)
    {
        // Store full sensor readings structure in BCE buffer
        mm_sensor_readings_full_t l_readings;
        mm_sensors_init_readings_full(i_mm_sensor, &l_readings);
        l_readings_size = sizeof(l_readings);
        memcpy(&G_mm_sensors_bce_buffer[i_bce_buf_offset], &l_readings, l_readings_size);
    }
    else
    {
        // Store counter sensor readings structure in BCE buffer
        mm_sensor_readings_counter_t l_readings;
        mm_sensors_init_readings_counter(i_mm_sensor, &l_readings);
        l_readings_size = sizeof(l_readings);
        memcpy(&G_mm_sensors_bce_buffer[i_bce_buf_offset], &l_readings, l_readings_size);
    }

    // Move the sensor readings offset to the next sensor
    *io_readings_offset += l_readings_size;

    // Update the number of bytes in the BCE buffer.  Note that we do not simply
    // add l_readings_size to o_byte_count.  That is because we may have skipped
    // one or more previous sensors because they were disabled.  Skipped sensors
    // do not update the byte count.  We now need to write the entire BCE buffer
    // up to this sensor's readings.  Any disabled sensors within the BCE buffer
    // range will have zeroes written to their offset in the Sensor Readings Buffer.
    *o_byte_count = (i_bce_buf_offset + l_readings_size);
}


/**
 * Writes sensor readings (dynamic data) to the current Sensor Readings Buffer
 * (ping/pong) in main memory.
 *
 * Only valid and enabled sensors are written to the Sensor Readings Buffer.
 * Sensors may be invalid due to the SMF Mode or OCC role.  Sensors are enabled
 * or disabled dynamically by external software such as OPAL.
 *
 * This function must be called repeatedly until the entire Sensor Readings
 * Buffer has been written.  The BCE can copy a maximum of 4kB at a time, and a
 * Sensor Readings Buffer is up to 40kB.  Non-blocking copies are performed, so
 * each time this function is called it will schedule a copy of up to 4kB.
 * Static variables are used to save state across function calls.
 *
 * When using the BCE, the main memory address and byte count must both be
 * 128-byte aligned.  The sensor readings structures are not a multiple of 128
 * bytes in length.  If the BCE buffer is nearly full and does not have enough
 * room for another complete sensor readings structure, the structure will not
 * be partially written.  The BCE buffer will be padded at the end with zeroes,
 * and the structure will be written next time the function is called.
 *
 * When calling this function more than once, the main memory address of the
 * next sensor readings structure will probably not be 128-byte aligned.  The
 * buffer G_mm_sensors_last_write_buf is used to work around this.  See the
 * doxygen comments by G_mm_sensors_last_write_buf for more information.
 *
 * When the Sensor Readings Buffer has been completely written this function
 * changes the current state to MM_SENSORS_VALIDATE_SENSOR_READINGS.
 */
void mm_sensors_write_sensor_readings(void)
{
    // Saved state from previous function call
    static uint16_t L_saved_sensor_index    = 0;
    static uint32_t L_saved_readings_offset = MM_SENSORS_FIRST_READINGS_OFFSET;

    // Load saved state from previous function call into local variables
    uint16_t l_sensor_index    = L_saved_sensor_index;
    uint32_t l_readings_offset = L_saved_readings_offset;

    // If this is the first sensor and no sensors are valid and enabled, exit
    // immediately.  There are no sensor readings to write to main memory.
    if ((l_sensor_index == 0) && (mm_sensors_valid_enabled_sensor_count() == 0))
    {
        return;
    }

    // Clear the BCE buffer
    memset(G_mm_sensors_bce_buffer, 0x00, MM_SENSORS_BCE_BUF_SIZE);

    // Number of bytes we want to write in the BCE buffer
    size_t l_byte_count = 0;

    // Main memory address for writing to the Sensor Readings Buffer
    uint32_t l_write_addr;

    // If this is the first sensor
    if (l_sensor_index == 0)
    {
        // Set main memory write address to beginning of Sensor Readings Buffer
        l_write_addr = G_mm_sensor_readings_buf_addr;

        // Store Sensor Readings Buffer Header in BCE buffer
        mm_sensor_readings_buf_header_t l_header;
        mm_sensors_init_readings_buf_header(&l_header);
        memcpy(G_mm_sensors_bce_buffer, &l_header, sizeof(l_header));
        l_byte_count = sizeof(l_header);
    }
    else
    {
        // Not the first sensor.  Calculate main memory address for writing to
        // the Sensor Readings Buffer.  The address must be 128-byte aligned.
        uint32_t l_readings_addr = G_mm_sensor_readings_buf_addr + l_readings_offset;
        l_write_addr = MM_SENSORS_ALIGN_128_DOWN(l_readings_addr);

        // If we previously wrote bytes to the same 128-byte aligned address, copy
        // those bytes from the last write buffer.  This will retain the value of
        // the bytes that precede the current sensor readings.
        if (G_mm_sensors_last_write_buf_addr == l_write_addr)
        {
            memcpy(G_mm_sensors_bce_buffer, G_mm_sensors_last_write_buf,
                   MM_SENSORS_MIN_WRITE_SIZE);
        }
    }

    // Loop through main memory sensors storing readings in BCE buffer
    for (; l_sensor_index < MAIN_MEM_SENSOR_COUNT; ++l_sensor_index)
    {
        // Exit loop if BCE buffer does not have room for this sensor's readings
        main_mem_sensor_t * l_mm_sensor = &G_main_mem_sensors[l_sensor_index];
        uint32_t l_readings_addr = G_mm_sensor_readings_buf_addr + l_readings_offset;
        uint16_t l_bce_buf_offset = l_readings_addr - l_write_addr;
        size_t l_readings_size = MM_SENSOR_READINGS_STRUCT_SIZE(l_mm_sensor->struct_ver);
        if ((l_bce_buf_offset + l_readings_size) > MM_SENSORS_BCE_BUF_SIZE)
        {
            break;
        }

        // Skip sensor readings if sensor is not valid or not enabled
        if (!l_mm_sensor->valid || !l_mm_sensor->enabled)
        {
            mm_sensors_skip_readings(l_mm_sensor, &l_readings_offset, &l_write_addr,
                                     l_byte_count);
            continue;
        }

        // Store sensor readings in BCE buffer
        mm_sensors_store_readings(l_mm_sensor, l_bce_buf_offset, &l_readings_offset,
                                  &l_byte_count);
    }

    // If we started at the first sensor, save first 128 bytes of the BCE
    // buffer.  Needed later by mm_sensors_validate_sensor_readings() to modify
    // the valid field in the Sensor Readings Buffer Header.
    if (L_saved_sensor_index == 0)
    {
        memcpy(G_mm_sensors_save_buffer, G_mm_sensors_bce_buffer, MM_SENSORS_MIN_WRITE_SIZE);
    }

    // Calculate number of bytes to copy.  The BCE requires this to be a
    // multiple of 128.  May end up copying extra zeroed out bytes in buffer.
    l_byte_count = MM_SENSORS_ALIGN_128_UP(l_byte_count);

    // Copy BCE buffer to Sensor Readings Buffer in main memory
    if (mm_sensors_bce_copy(l_write_addr, l_byte_count, MM_SENSORS_WRITE_READINGS_MOD))
    {
        // Copy succeeded.  Save state for next time function called.
        L_saved_sensor_index    = l_sensor_index;
        L_saved_readings_offset = l_readings_offset;

        // Save last 128 bytes we wrote in case we need it for alignment
        mm_sensors_save_last_write(l_write_addr, l_byte_count);

        // If all sensors have been written to the Sensor Readings Buffer
        if (l_sensor_index >= MAIN_MEM_SENSOR_COUNT)
        {
            // Reset state.  Next time this function is called we will be
            // writing to the other Sensor Readings Buffer (ping/pong).
            L_saved_sensor_index    = 0;
            L_saved_readings_offset = MM_SENSORS_FIRST_READINGS_OFFSET;

            // Change state to validate current Sensor Readings Buffer
            G_mm_sensors_state = MM_SENSORS_VALIDATE_SENSOR_READINGS;
        }
    }
}


/**
 * Validates the current Sensor Readings Buffer in main memory.
 *
 * Sets the valid field in the Sensor Readings Buffer Header to true indicating
 * that all sensor readings have been written.
 *
 * If successful:
 *   - Switches the current Sensor Readings Buffer (ping/pong)
 *   - Changes current state to MM_SENSORS_WRITE_SENSOR_READINGS.
 */
void mm_sensors_validate_sensor_readings(void)
{
    // Clear the BCE buffer
    memset(G_mm_sensors_bce_buffer, 0x00, MM_SENSORS_BCE_BUF_SIZE);

    // We need to modify the valid field of the Sensor Readings Buffer Header.
    // However, the BCE does not support writing a single byte.  The minimum
    // write size is 128 bytes.  To work around this, we saved the first 128
    // bytes of the Sensor Readings Buffer (including the header) in
    // mm_sensors_write_sensor_readings().  Copy those saved bytes back into the
    // BCE buffer.
    memcpy(G_mm_sensors_bce_buffer, G_mm_sensors_save_buffer, MM_SENSORS_MIN_WRITE_SIZE);

    // Cast BCE buffer to a mm_sensor_readings_buf_header_t *
    mm_sensor_readings_buf_header_t * l_header =
        (mm_sensor_readings_buf_header_t *) (&G_mm_sensors_bce_buffer);

    // Set the valid field to true since all sensor readings have been written
    l_header->valid = MM_SENSOR_READINGS_VALID_TRUE;

    // Copy first 128 bytes of Sensor Readings Buffer to main memory using BCE
    if (mm_sensors_bce_copy(G_mm_sensor_readings_buf_addr, MM_SENSORS_MIN_WRITE_SIZE,
                            MM_SENSORS_VALIDATE_READINGS_MOD))
    {
        // Copy succeeded.  Switch to other Sensor Readings Buffer (ping/pong).
        G_mm_sensor_readings_buf_addr =
            (G_mm_sensor_readings_buf_addr == MM_SENSOR_READINGS_PING_ADDRESS) ?
            MM_SENSOR_READINGS_PONG_ADDRESS : MM_SENSOR_READINGS_PING_ADDRESS;

        // Change state to write sensor readings to other buffer
        G_mm_sensors_state = MM_SENSORS_WRITE_SENSOR_READINGS;
    }
}


//******************************************************************************
// Public Functions
//******************************************************************************

// See description in header file
void main_mem_sensors_init(void)
{
    // Initialize internal data structures if not done before
    static bool L_initialized_internals = false;
    if (!L_initialized_internals)
    {
        mm_sensors_init_internals();
        L_initialized_internals = true;
    }

    // If initialization is already complete exit immediately
    static bool L_traced_init_complete_err = false;
    if (G_main_mem_sensors_initialized)
    {
        // Trace if this is the first time we encountered the error
        if (!L_traced_init_complete_err)
        {
            TRAC_ERR("main_mem_sensors_init: Initialization is already complete");
            L_traced_init_complete_err = true;
        }
        return;
    }

    // Check if there is a scheduled BCE request that has not finished.  If so
    // we must exit and try again later.
    if (!mm_sensors_is_bce_req_idle(MM_SENSORS_INIT_MOD))
    {
        return;
    }

    // Write main memory data structures that contain static sensor data.  Write
    // data for current state.  Data cannot all be written at once due to BCE
    // size limitations and non-blocking writes.
    switch (G_mm_sensors_state)
    {
        case MM_SENSORS_WRITE_DATA_HEADER:
            mm_sensors_write_data_header();
            break;
        case MM_SENSORS_WRITE_SENSOR_NAMES:
            mm_sensors_write_sensor_names();
            break;
        case MM_SENSORS_VALIDATE_DATA_HEADER:
            mm_sensors_validate_data_header();
            break;
    }

    // If initialization is now complete trace the event
    if (G_main_mem_sensors_initialized)
    {
        TRAC_IMP("main_mem_sensors_init: Initialization complete for all %u valid sensors",
                 mm_sensors_valid_sensor_count());
    }
}


// See description in header file
void main_mem_sensors_update(void)
{
    // Make sure main memory sensor initialization is complete
    static bool L_traced_init_err = false;
    if (!G_main_mem_sensors_initialized)
    {
        // Trace if this is the first time we encountered the error
        if (!L_traced_init_err)
        {
            TRAC_ERR("main_mem_sensors_update: Main memory sensors not initialized");
            L_traced_init_err = true;
        }
        return;
    }

    // Check if there is a scheduled BCE request that has not finished.  If so
    // we must exit and try again later.
    if (!mm_sensors_is_bce_req_idle(MM_SENSORS_UPDATE_MOD))
    {
        return;
    }

    // Write sensor readings to main memory.  Write data for current state.
    // Data cannot all be written at once due to BCE size limitations and
    // non-blocking writes.
    switch (G_mm_sensors_state)
    {
        case MM_SENSORS_WRITE_SENSOR_READINGS:
            mm_sensors_write_sensor_readings();
            break;
        case MM_SENSORS_VALIDATE_SENSOR_READINGS:
            mm_sensors_validate_sensor_readings();
            break;
    }
}


// See description in header file
void main_mem_sensors_set_enabled(AMEC_SENSOR_TYPE i_sensor_type, bool i_enabled)
{
    // Loop through all main memory sensors
    uint16_t l_index;
    for (l_index = 0; l_index < MAIN_MEM_SENSOR_COUNT; ++l_index)
    {
        // Get GSID of main memory sensor
        uint16_t l_gsid = G_main_mem_sensors[l_index].gsid;

        // Get sensor type from associated sensor_info_t
        uint16_t l_sensor_type = G_sensor_info[l_gsid].sensor.type;

        // If i_sensor_type parameter includes current sensor's type
        if (i_sensor_type & l_sensor_type)
        {
            // Set enabled state of current sensor
            G_main_mem_sensors[l_index].enabled = i_enabled;
        }
    }
}
