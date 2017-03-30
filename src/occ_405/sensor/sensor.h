/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor.h $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

#ifndef _sensor_h
#define _sensor_h

#include <common_types.h>        // defines for uint8_t,uint3_t..etc
#include <errl.h>                // For errlHndl_t
#include <occ_common.h>              // Common OCC defines
#include <sensor_enum.h>         // For Sensor Enum

// Macro to get mini sensor value. It gives the mini sensor value for
// the given occId at given the given field. If occId is out of range, it will
// return 0.
#define MINI_SENSOR_VALUE(occId,fieldNm) \
            ( (occId < MAX_OCCS) ? G_dcom_slv_outbox[occId].fieldNm : 0x0)

// Make a AMEC Number out of mantissa & exponent.
#define AMEFP(mantissa,exp) (((UINT32)mantissa << 8) | (UINT32)((UINT8) 256 + (UINT8)exp))

// Get pointer to sensor based on GSID
#define AMECSENSOR_PTR(sensor) G_amec_sensor_list[sensor]
#define AMECSENSOR_ARRAY_PTR(sensor_base,idx) G_amec_sensor_list[sensor_base+idx]
#define AMECSENSOR_2D_ARRAY_PTR(sensor_base,idx,idx2) G_amec_sensor_list[sensor_base+idx+idx2]

#define AMEC_SENSOR_NONUM           0xFF
#define SENSOR_TYPE_ALL             0xFFFF
#define SENSOR_LOC_ALL              0xFFFF
#define MAX_VECTOR_SENSORS          32
#define MAX_SENSOR_NAME_SZ          16 // including NULL
#define MAX_SENSOR_UNIT_SZ          4 // including NULL
#define MAX_AMEC_SENSORS            710
#define VECTOR_SENSOR_DEFAULT_VAL   0xFF

// AMEC_SENSOR_TYPE_INVALID can not be used to identify sensor type.
// A bit vector mask is used to specify sensor types that AMESTER is
// probed for, and hence when a new sensor type is defined, it has to
// have only one bit set in its value (e.g., 0x01<<num). When all types
// are requested, a 0xFFFF mask (SENSOR_TYPE_ALL) is used to specify all
// types, which is the case for initial AMESTER sensor scan routine to
// detect all OCC sensors.
typedef enum
{
    AMEC_SENSOR_TYPE_INVALID    = 0x0000,    // Not a valid sensor type
    AMEC_SENSOR_TYPE_GENERIC    = 0x0001,
    AMEC_SENSOR_TYPE_CURRENT    = 0x0002,
    AMEC_SENSOR_TYPE_VOLTAGE    = 0x0004,
    AMEC_SENSOR_TYPE_TEMP       = 0x0008,
    AMEC_SENSOR_TYPE_UTIL       = 0x0010,
    AMEC_SENSOR_TYPE_TIME       = 0x0020,
    AMEC_SENSOR_TYPE_FREQ       = 0x0040,
    AMEC_SENSOR_TYPE_POWER      = 0x0080,
    AMEC_SENSOR_TYPE_PERF       = 0x0200,
    AMEC_SENSOR_TYPE_ALL        = 0xffff,
}AMEC_SENSOR_TYPE;

// AMEC_SENSOR_LOC_INVALID can not be used to identify sensor location.
// A bit vector mask is used to specify sensor locations that AMESTER is
// probed for, and hence when a new sensor type is defined, it has to
// have exactly one bit set in its value (e.g., 0x01<<num). When sensors
// from all locations are requested, a 0xFFFF mask (SENSOR_LOC_ALL) is
// used to specify all locations, and that's the case in the initial
// AMESTER sensor scan routine.
typedef enum
{
    AMEC_SENSOR_LOC_INVALID     = 0x0000,   // Not a valid sensor location.
    AMEC_SENSOR_LOC_SYS         = 0x0001,
    AMEC_SENSOR_LOC_PROC        = 0x0002,
    AMEC_SENSOR_LOC_LPAR        = 0x0004,
    AMEC_SENSOR_LOC_MEM         = 0x0008,
    AMEC_SENSOR_LOC_VRM         = 0x0010,
    AMEC_SENSOR_LOC_OCC         = 0x0020,
    AMEC_SENSOR_LOC_CORE        = 0x0040,
    AMEC_SENSOR_LOC_ALL         = 0xffff,
}AMEC_SENSOR_LOC;

// Vector Sensor operation enumeration
typedef enum
{
    VECTOR_OP_NONE = 0x00,
    VECTOR_OP_MAX = 0x01,
    VECTOR_OP_MIN = 0x02,
    VECTOR_OP_AVG = 0x03,

} VECTOR_SENSOR_OP;

/*****************************************************************************/
// Forward declaration as used in vectorSensor
struct sensor;
typedef struct sensor sensor_t;

// Vector Sensor structure
struct vectorSensor
{
    VECTOR_SENSOR_OP operation; //Operation that this vector should execute
    uint8_t size; // Number of elements in this vector
    uint8_t  elem_enabled[MAX_VECTOR_SENSORS]; // indicates if a given element
                                            // is enabled or disabled
    sensor_t* source_ptr[MAX_VECTOR_SENSORS]; // Pointer to an array of source
                                //pointers (source elements of this vector)
    uint8_t min_pos; // Minimum val vector position for the latest snapshot
    uint8_t max_pos; // Maximum val vector position for the latest snapshot

} __attribute__ ((__packed__));

typedef struct vectorSensor vectorSensor_t;

typedef struct
{
  char name[MAX_SENSOR_NAME_SZ];
  struct
  {
    char units[MAX_SENSOR_UNIT_SZ];
    uint16_t type;
    uint16_t location;
    uint8_t number;
    uint32_t freq;
    uint32_t scalefactor;
  } __attribute__ ((__packed__)) sensor;
} __attribute__ ((__packed__)) sensor_info_t;

/*****************************************************************************/
//Sensor status structure
struct sensorStatus
{
    uint8_t buffer_area_type:1,
            update_histogram:1,
            reset:1,
            reserved:5;
};
typedef struct sensorStatus sensorStatus_t;

/*****************************************************************************/
// Sensor structure
struct sensor
{
    uint16_t gsid;            // Global Sensor ID
    uint16_t sample;          // Latest sample of this sensor
    uint16_t sample_min;      // Minimum value since last reset
    uint16_t sample_max;      // Maximum Value since last reset
    uint64_t accumulator;     // Accumulator register for this sensor
    uint32_t src_accum_snapshot; // Copy of the source sensor's accumulator
                                 // used for time-derived sensors
    uint32_t update_tag;      // Count of the number of 'ticks' that have passed
                              // between updates to this sensor (used for time-
                              // derived sensor)
    uint32_t ipmi_sid;        // Ipmi sensor id obtained from mrw
    vectorSensor_t * vector;  // Pointer to vector control structure. NULL if
                              // this is not a vector sensor.
    uint16_t * mini_sensor;   // Pointer to entry in mini-sensor table. NULL if
                              // this sensor does not have a mini-sensor
    sensorStatus_t status;    // Status and control register

} __attribute__ ((__packed__));

// Typedef for this structure is part of the forward declaration for the
// vector sensor structure section in this file

/*****************************************************************************/
// Sensor Query list structure used for querying sensor data
struct sensorQueryList
{
    uint16_t gsid; // Global Sensor ID
    uint16_t sample; // Latest sample of the sensor
    char name[MAX_SENSOR_NAME_SZ]; // Sensor Name
};

typedef struct sensorQueryList sensorQueryList_t;

/*****************************************************************************/
// Sensor List Structures used to build tables of sensor pointers.
// Global sensor list type
typedef sensor_t * sensor_ptr_t;
// Global mini-sensor list type
typedef uint16_t * minisensor_ptr_t;

/*****************************************************************************/
// These are used by the sensor init
// Global sensor counter
extern uint32_t G_amec_sensor_count;

// Contains array of pointers to sensors, indexed by GSID
extern const sensor_ptr_t     G_amec_sensor_list[];

extern const sensor_info_t    G_sensor_info[];

// Contains array of pointers to mini-sensors, indexed by GSID
extern const minisensor_ptr_t G_amec_mini_sensor_list[];

// sensor_init
void sensor_init(sensor_t * io_sensor_ptr,
                 const uint16_t i_gsid,
                 const uint16_t * i_miniSnsrPtr
                 );

// Clear minmax value
void sensor_clear_minmax( sensor_t * io_sensor_ptr);

// Sensor reset
void sensor_reset( sensor_t * io_sensor_ptr);

//Vectorize sensor
void sensor_vectorize( sensor_t * io_sensor_ptr,
                       vectorSensor_t * io_vec_sensor_ptr,
                       const VECTOR_SENSOR_OP i_op);

//Update sensor
void sensor_update( sensor_t * io_sensor_ptr, const uint16_t i_sensor_value);

//Perform operation on vector sensor and update sensor
void sensor_vector_update( sensor_t * io_sensor_ptr,const uint32_t i_threshold);

// Get sensor by GSID
sensor_t * getSensorByGsid( const uint16_t i_gsid);

// enable/disable vector sensor element
void sensor_vector_elem_enable( vectorSensor_t* io_sensor_vector_ptr,
                                const uint8_t i_loc,
                                const uint8_t i_enable);

// Add element to the vector sensor
void sensor_vector_elem_add( vectorSensor_t* io_sensor_vector_ptr,
                             const uint8_t i_loc,
                             const sensor_t * i_elemPtr);

// Initialize all sensors
void sensor_init_all(void);

#endif // _sensor_h


