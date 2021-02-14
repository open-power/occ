/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor.c $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2021                        */
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



#include <sensor.h>               // sensor structure and other defines
#include <occ_common.h>           // For size_t needed by memset
#include <string.h>               // For memset
#include "ssx_io.h"               // For sprintf
#include <occ_service_codes.h>    // OCC reason codes
#include <sensor_service_codes.h> // sensor module ids
#include <trac.h>                 // Trace macros
#include <get_tod_structs.h>      // For TOD_VALUE_UNKNOWN

uint16_t G_sensor_debug_group[NUM_SENSOR_DEBUG_GROUPS][MAX_NUMBER_SENSORS_PER_DEBUG_GROUP];

#define UINT16_MIN                  0

// Global sensor counter
uint32_t G_amec_sensor_count = 0;

// See header file for description
volatile uint64_t G_tod = TOD_VALUE_UNKNOWN;

void sensor_init(sensor_t * io_sensor_ptr,
                 const uint16_t i_gsid,
                 const uint16_t * i_miniSnsrPtr
                 ) INIT_SECTION;

void sensor_init_all(void) INIT_SECTION;

// Function Specification
//
//  Name: sensor_init
//
//  Description: Initialize global sensor list.
//
// End Function Specification
void sensor_init(sensor_t * io_sensor_ptr,
                 const uint16_t i_gsid,
                 const uint16_t * i_miniSnsrPtr
                 )
{
    // check if input pointers are valid and global sensor count is
    // within range.
    // Note: Don't need to check i_miniSnsrPtr here as it can be NULL
    if(( io_sensor_ptr != NULL) &&
       (G_amec_sensor_count < MAX_AMEC_SENSORS))
    {
        // zero out sensor pointer
        memset(io_sensor_ptr,0x0,sizeof(sensor_t));

        io_sensor_ptr->gsid = i_gsid;

        io_sensor_ptr->status.reset = 1;

        // set mini sensor pointer to point to i_miniSnsrPtr. i_miniSnsrPtr can
        // be NULL
        io_sensor_ptr->mini_sensor = (uint16_t*)i_miniSnsrPtr;

        G_amec_sensor_count++;

    } // end valid input and max sensor count check
    else
    {
        TRAC_ERR("Invalid input parameters OR Number of "
               "sensor is out of range. Current sensor count: 0x%x, "
               "max allowed: 0x%x",G_amec_sensor_count,MAX_AMEC_SENSORS);
    }
}

// Function Specification
//
//  Name: sensor_clear_minmax
//
//  Description: Clears minimum and maximum fields in the sensor structure.
//               i_clear_type contains one or more values OR'd together from the
//               AMEC_SENSOR_CLEAR_TYPE enumeration.
//
// End Function Specification
void sensor_clear_minmax(sensor_t * io_sensor_ptr,
                         AMEC_SENSOR_CLEAR_TYPE i_clear_type)
{
    if (io_sensor_ptr != NULL)
    {
        if (i_clear_type & AMEC_SENSOR_CLEAR_SAMPLE_MINMAX)
        {
            io_sensor_ptr->sample_min = UINT16_MAX;
            io_sensor_ptr->sample_max = UINT16_MIN;

            // If it has vector sensor, clear max and min position
            if (io_sensor_ptr->vector != NULL)
            {
                io_sensor_ptr->vector->max_pos = VECTOR_SENSOR_DEFAULT_VAL;
                io_sensor_ptr->vector->min_pos = VECTOR_SENSOR_DEFAULT_VAL;
            }
        }

        if (i_clear_type & AMEC_SENSOR_CLEAR_CSM_SAMPLE_MINMAX)
        {
            io_sensor_ptr->csm_sample_min = UINT16_MAX;
            io_sensor_ptr->csm_sample_max = UINT16_MIN;
        }

        if (i_clear_type & AMEC_SENSOR_CLEAR_PROFILER_SAMPLE_MINMAX)
        {
            io_sensor_ptr->profiler_sample_min = UINT16_MAX;
            io_sensor_ptr->profiler_sample_max = UINT16_MIN;
        }

        if (i_clear_type & AMEC_SENSOR_CLEAR_JOB_S_SAMPLE_MINMAX)
        {
            io_sensor_ptr->job_s_sample_min = UINT16_MAX;
            io_sensor_ptr->job_s_sample_max = UINT16_MIN;
        }
    }
    else
    {
        TRAC_ERR("Invalid input parameters ");
    }
}


// Function Specification
//
//  Name: sensor_reset
//
//  Description: Reset sensor fields
//
// End Function Specification
void sensor_reset( sensor_t * io_sensor_ptr)
{
    if( io_sensor_ptr != NULL)
    {
        io_sensor_ptr->timestamp = 0x0;
        io_sensor_ptr->accumulator = 0x0;
        io_sensor_ptr->sample = 0x0;

        // clear mini sensor value
        if( io_sensor_ptr->mini_sensor != NULL)
        {
            *(io_sensor_ptr->mini_sensor) = 0x0;
        }

        sensor_clear_minmax(io_sensor_ptr, AMEC_SENSOR_CLEAR_ALL_MINMAX);

        io_sensor_ptr->status.reset = 0;
    }
    else
    {
        TRAC_ERR("Invalid input parameters ");
    }
}


// Function Specification
//
//  Name: sensor_vectorize
//
//  Description: vectorize sensor
//
// End Function Specification
void sensor_vectorize( sensor_t * io_sensor_ptr,
                       vectorSensor_t * io_vec_sensor_ptr,
                       const VECTOR_SENSOR_OP i_op)
{
    if( (io_vec_sensor_ptr != NULL) && (io_sensor_ptr != NULL))
    {
        // assign to sensor vector pointer
        io_sensor_ptr->vector = io_vec_sensor_ptr;
        // zero out vector sensor and set operation
        memset(io_vec_sensor_ptr,0x0,sizeof(vectorSensor_t));
        io_vec_sensor_ptr->operation = i_op;
    }
    else
    {
        TRAC_ERR("Invalid input sensor pointer");
    }
}


// Function Specification
//
//  Name: sensor_update_minmax
//
//  Description: Updates minimum and maximum fields in the sensor structure.
//
//  Implementation Notes:
//    * This is an internal function so we don't validate parameters
//
// End Function Specification
void sensor_update_minmax(sensor_t * io_sensor_ptr, uint16_t i_sensor_value)
{
    // Update sample min/max fields if needed
    if (i_sensor_value < io_sensor_ptr->sample_min)
    {
        io_sensor_ptr->sample_min = i_sensor_value;
    }
    if (i_sensor_value > io_sensor_ptr->sample_max)
    {
        io_sensor_ptr->sample_max = i_sensor_value;
    }

    // Update CSM sample min/max fields if needed
    if (i_sensor_value < io_sensor_ptr->csm_sample_min)
    {
        io_sensor_ptr->csm_sample_min = i_sensor_value;
    }
    if (i_sensor_value > io_sensor_ptr->csm_sample_max)
    {
        io_sensor_ptr->csm_sample_max = i_sensor_value;
    }

    // Update profiler sample min/max fields if needed
    if (i_sensor_value < io_sensor_ptr->profiler_sample_min)
    {
        io_sensor_ptr->profiler_sample_min = i_sensor_value;
    }
    if (i_sensor_value > io_sensor_ptr->profiler_sample_max)
    {
        io_sensor_ptr->profiler_sample_max = i_sensor_value;
    }

    // Update job scheduler sample min/max fields if needed
    if (i_sensor_value < io_sensor_ptr->job_s_sample_min)
    {
        io_sensor_ptr->job_s_sample_min = i_sensor_value;
    }
    if (i_sensor_value > io_sensor_ptr->job_s_sample_max)
    {
        io_sensor_ptr->job_s_sample_max = i_sensor_value;
    }
}


// Function Specification
//
//  Name: sensor_update
//
//  Description: Update sensor
//
// End Function Specification
void sensor_update( sensor_t * io_sensor_ptr, const uint16_t i_sensor_value)
{
    if( io_sensor_ptr != NULL)
    {
        // reset sensors if requested
        if( io_sensor_ptr->status.reset == 1)
        {
            sensor_reset(io_sensor_ptr);
        }

        // set timestamp to current time of day
        io_sensor_ptr->timestamp = G_tod;

        // update sample value
        io_sensor_ptr->sample = i_sensor_value;

        // update min/max values if needed
        sensor_update_minmax(io_sensor_ptr, i_sensor_value);

        // If this sensor has mini sensor, update it's value
        if( io_sensor_ptr->mini_sensor != NULL)
        {
            *(io_sensor_ptr->mini_sensor) = i_sensor_value;
        }

        // add sample value to accumulator
        io_sensor_ptr->accumulator += i_sensor_value;

        // increment update tag
        io_sensor_ptr->update_tag += 1;
    }
    else
    {
        TRAC_ERR("Invalid sensor pointer");
    }
}


// Function Specification
//
//  Name: sensor_op_min
//
//  Description:  Perform min operation on vector sensor
//
// End Function Specification
uint16_t sensor_op_min(const vectorSensor_t * i_vecSensorPtr,
                       uint8_t * o_position)
{
    uint16_t l_value = UINT16_MAX;
    uint16_t i = 0;

    // Internal function so not checking for input NULL pointers

    // traverse through enabled vector sensors to find the minimum value
    for(; i <  i_vecSensorPtr->size; i++)
    {
        // check if sensor is enabled
        if( i_vecSensorPtr->elem_enabled[i] == 1)
        {
            if( i_vecSensorPtr->source_ptr[i]->sample < l_value)
            {
                l_value = i_vecSensorPtr->source_ptr[i]->sample;
                *o_position = i;
            }
        } // end element enabled check

    } // end for loop

    return l_value;

}


// Function Specification
//
//  Name: sensor_op_max
//
//  Description:  Perform max operation for vector sensor
//
// End Function Specification
uint16_t sensor_op_max(const vectorSensor_t * i_vecSensorPtr,
                       uint8_t * o_position)
{
    uint16_t l_value = UINT16_MIN;
    uint16_t i = 0;

    // Internal function so not checking for input NULL pointers

    // traverse through enabled vector sensors to find the maximum value
    for(; i <  i_vecSensorPtr->size; i++)
    {
        // Check if element is enabled
        if( i_vecSensorPtr->elem_enabled[i] == 1)
        {
            if( i_vecSensorPtr->source_ptr[i]->sample > l_value)
            {
                l_value = i_vecSensorPtr->source_ptr[i]->sample;
                *o_position = i;
            }
        }// end element enabled check

    } // end for loop

    return l_value;

}


// Function Specification
//
//  Name: sensor_op_avg
//
//  Description:  Perform average operation for vector sensor
//
// End Function Specification
uint16_t sensor_op_avg(const vectorSensor_t * i_vecSensorPtr,
                       const uint16_t i_threshold)
{
    uint32_t l_sum = 0;
    uint16_t i = 0;
    uint16_t l_number = 0;

    // Internal function so not checking for input NULL pointers

    // traverse through enabled vector sensors to get sum of sensor sample
    for(; i <  i_vecSensorPtr->size; i++)
    {
        // check if element is enabled
        if( i_vecSensorPtr->elem_enabled[i] == 1)
        {
            // Include sample only if it is higher than threshold
            if( i_vecSensorPtr->source_ptr[i]->sample > i_threshold)
            {
                l_number++;
                l_sum += i_vecSensorPtr->source_ptr[i]->sample;
            }
        } // end element enabled check

    } // end for loop

    // Calculate average
    if( l_number != 0)
    {
        l_sum = l_sum / l_number;
    }

    return l_sum;
}


// Function Specification
//
//  Name: sensor_vector_update
//
//  Description: Update Vector Sensor
//
// End Function Specification
void sensor_vector_update( sensor_t * io_sensor_ptr,const uint32_t i_threshold)
{
    if( io_sensor_ptr != NULL)
    {
        // Reset sensor if requested
        if( io_sensor_ptr->status.reset == 1)
        {
            sensor_reset(io_sensor_ptr);
        }

        // Perform vector sensor operation and update sensor
        if( io_sensor_ptr->vector != NULL)
        {
            uint16_t l_value = 0;
            uint8_t l_position = 0;

            // Min operation
            if( VECTOR_OP_MIN ==  io_sensor_ptr->vector->operation)
            {
                // calculate min value and get sensor position holding min value
                l_value = sensor_op_min(io_sensor_ptr->vector, &l_position);
                // set min position in vector sensor
                io_sensor_ptr->vector->min_pos = l_position;
                // Update only if needed
                if( l_value != UINT16_MAX)
                {
                    // update sensor with new min value
                    sensor_update(io_sensor_ptr,l_value);
                }
            }
            // Max operation
            else if( VECTOR_OP_MAX == io_sensor_ptr->vector->operation)
            {
                // calculate max value and get sensor position holding max value
                l_value = sensor_op_max(io_sensor_ptr->vector, &l_position);
                // set max position in vector sensor
                io_sensor_ptr->vector->max_pos = l_position;
                // Update only if needed
                if( l_value != UINT16_MIN)
                {
                    // update sensor with new max value
                    sensor_update(io_sensor_ptr,l_value);
                }
            }
            // Average operation
            else if( VECTOR_OP_AVG == io_sensor_ptr->vector->operation)
            {
                // Calculate average of the sensor sample
                l_value = sensor_op_avg(io_sensor_ptr->vector,i_threshold);
                // Update only if needed
                if( l_value != 0)
                {
                    // update sensor with new average value
                    sensor_update(io_sensor_ptr,l_value);
                }
            }
            // Unsupported operation
            else
            {
                TRAC_ERR("Unsupported vector sensor operation: 0x%x",
                         io_sensor_ptr->vector->operation);
            }

        } // end valid vector sensor check
        else
        {
            TRAC_ERR("This sensor does not have vector sensor");
        }
    }
    else
    {
        TRAC_ERR("Invalid input sensor pointer");
    }
}


// Function Specification
//
//  Name: getSensorByGsid
//
//  Description: Get sensor data using GSID (Global Sensor ID)
//
// End Function Specification
sensor_t * getSensorByGsid( const uint16_t i_gsid)
{
    sensor_t * l_sensorPtr = NULL;

    // check if input gsid is within range. Return sensor pointer if within
    // range. Else return NULL
    if( i_gsid < G_amec_sensor_count)
    {
        l_sensorPtr = G_amec_sensor_list[i_gsid];
    }

    return l_sensorPtr;
}


// Function Specification
//
//  Name: sensor_vector_elem_enable
//
//  Description: Enable/disable vector sensor element. This interface can be
//               used at runtime
//
// End Function Specification
void sensor_vector_elem_enable( vectorSensor_t* io_sensor_vector_ptr,
                                const uint8_t i_loc,
                                const uint8_t i_enable)
{
    if( io_sensor_vector_ptr != NULL)
    {
        // check if location of the vector sensor is within range
        if( i_loc < io_sensor_vector_ptr->size)
        {
            // set element enabled
            io_sensor_vector_ptr->elem_enabled[i_loc] = i_enable;
        }
        else
        {
            TRAC_ERR("Invalid input location: 0x%x, max size: 0x%x",
                     i_loc,io_sensor_vector_ptr->size);
        }
    }
    else
    {
        TRAC_ERR("NULL input vector sensor pointer");
    }
}


// Function Specification
//
//  Name: sensor_vector_elem_add
//
//  Description: Add element to the vector sensor. If element at the given
//  location is already present, it will be overwritten.
//
// End Function Specification
void sensor_vector_elem_add( vectorSensor_t* io_sensor_vector_ptr,
                             const uint8_t i_loc,
                             const sensor_t * i_elemPtr)
{
    if( (io_sensor_vector_ptr == NULL) ||
        (i_elemPtr == NULL ) ||
        (i_loc >= MAX_VECTOR_SENSORS))
    {
        TRAC_ERR("Invalid input parameters. Either pointers are NULL or "
               "location is out of range i_loc: 0x%x,max allowed: 0x%x",
               i_loc,MAX_VECTOR_SENSORS);
    }
    else if( (i_loc > io_sensor_vector_ptr->size))
    {
        TRAC_ERR("Invalid location. Location does not make element contiguous "
               "i_loc: 0x%x,current vector size: 0x%x",i_loc,
               io_sensor_vector_ptr->size);
    }
    else
    {
        // Increase size if spot is empty. Else we are overwriting existing
        // slot so no need to increment vector size
        if(io_sensor_vector_ptr->source_ptr[i_loc] == NULL)
        {
            io_sensor_vector_ptr->size++;
        }
        // set element and enable it.
        io_sensor_vector_ptr->source_ptr[i_loc] = (sensor_t*)i_elemPtr;
        io_sensor_vector_ptr->elem_enabled[i_loc] = 1;
    }
}



// Function Specification
//
//  Name: sensor_init_all
//
//  Description: Initialize all sensors in the global sensor list.
//
// End Function Specification
void sensor_init_all(void)
{
  int i;
  int l_num_entries = NUMBER_OF_SENSORS_IN_LIST;
  const sensor_ptr_t * l_argPtrSensor = &G_amec_sensor_list[0];
  const minisensor_ptr_t * l_argPtrMiniSensor = &G_amec_mini_sensor_list[0];

  for(i=0; i < l_num_entries; i++)
  {
      sensor_init(l_argPtrSensor[i], i, l_argPtrMiniSensor[i]);
  }

  // If G_amec_sensor_count doesn't match the number of sensors, we must have
  // failed to initialize one or more sensors.
  if(NUMBER_OF_SENSORS_IN_LIST != G_amec_sensor_count)
  {
    TRAC_ERR("Sensor Initialization Failed to initialize all sensors");

      /* @
       * @errortype
       * @moduleid    SENSOR_INITIALIZE
       * @reasoncode  INTERNAL_FAILURE
       * @userdata1   G_amec_sensor_count - number of sensors initialized
       * @userdata2   NUMBER_OF_SENSORS_IN_LIST - total number of OCC sensors
       * @userdata4   OCC_NO_EXTENDED_RC
       * @devdesc     Firmware internal failure initializing sensors
       */
      errlHndl_t l_err = createErrl(
          SENSOR_INITIALIZE,        // Module ID
          INTERNAL_FAILURE,         // Reason Code  // @wb003
          OCC_NO_EXTENDED_RC,       // Extended reason code
          ERRL_SEV_PREDICTIVE,      // Severity
          NULL,                     // Trace
          0,                        // Trace Size
          G_amec_sensor_count,      // UserData 1
          NUMBER_OF_SENSORS_IN_LIST // UserData 2
          );

      commitErrl(&l_err);
  }

  // init sensor debug groups to no sensors
  memset(&G_sensor_debug_group[0][0],0x0,sizeof(G_sensor_debug_group));
  G_sensor_debug_group[0][0] = NUMBER_OF_SENSORS_IN_LIST;
  G_sensor_debug_group[1][0] = NUMBER_OF_SENSORS_IN_LIST;
  G_sensor_debug_group[2][0] = NUMBER_OF_SENSORS_IN_LIST;

  TRAC_IMP("Sensor Initialization Complete");
}

