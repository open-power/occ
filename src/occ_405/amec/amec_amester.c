/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_amester.c $                             */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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

//*************************************************************************/
// Includes
//*************************************************************************/
#include <occ_common.h>
#include <ssx.h>
#include <errl.h>               // Error logging
#include <rtls.h>
#include <occ_service_codes.h>  // for SSX_GENERIC_FAILURE
#include <sensor.h>
#include <amec_smh.h>
#include <amec_master_smh.h>
#include <amec_amester.h>
#include <amec_sys.h>
#include <trac.h>               // For traces
#include <sensor_query_list.h>
#include <proc_data.h>
#include <amec_parm.h>
#include <string.h>
#include <occ_sys_config.h>
#include <dcom.h>

//*************************************************************************/
// Externs
//*************************************************************************/
extern uint32_t G_present_hw_cores;
//*************************************************************************/
// Macros
//*************************************************************************/

//*************************************************************************/
// Defines/Enums
//*************************************************************************/
// default length to IPMI limit so we don't break AMESTER using IPMI
// Updated AMESTER can send command to increase the length on systems that do not
// use IPMI and support a larger data length to improve AMESTER performance
uint16_t G_amester_max_data_length = IPMI_MAX_MSG_SIZE;

///Maximum size of trace buffer
// NOTE:  Any names in this file using timescale will NOT be kept in sync
//         with RTL tick time changes since this is just for AMESTER, names
//         used outside of the file by the main OCC code will be kept in sync
#define AMEC_TB_2MS_SIZE_BYTES   8192
#define AMEC_TB_250US_SIZE_BYTES 8192
#define AMEC_TB_SIZE_BYTES (AMEC_TB_250US_SIZE_BYTES + AMEC_TB_2MS_SIZE_BYTES)

///Maximum number of trace buffers we will support
#define AMEC_MAX_NUM_TB          2

///Maximum size of config info for 1 trace buffer
#define AMEC_TB_CONFIG_SIZE (MAX_SENSOR_NAME_SZ + 4)
#define MAX_NUM_CHIPS MAX_NUM_OCC

//*************************************************************************/
// Structures
//*************************************************************************/

//*************************************************************************/
// Globals
//*************************************************************************/

// Each trace buffer should be aligned to 128 bytes in main memory because the
// block copy engine only copies multiples of 128 byte units.
// Make this a power of 2 (bytes) in size and aligned to 4 bytes.
DMA_BUFFER(UINT8 g_amec_tb_bytes[AMEC_TB_SIZE_BYTES]);

// Array that maintains a list of all trace buffers built.
// NOTE: Must be in same order as AMEC_TB_ENUM
DMA_BUFFER(amec_tb_t  g_amec_tb_list[AMEC_MAX_NUM_TB]) = {
    //trace every 8th tick
    [AMEC_TB_EVERY_8TH_TICK] = {
        "trace2ms", // name
        AMEFP(500,0), // freq
        (UINT8*)(UINT32)g_amec_tb_bytes, // bytes
        AMEC_TB_2MS_SIZE_BYTES, // size
        0, // entry_size
        0, // entry_n
        0, // write
        0, // read
        0, // sensors_n
        0, // parm_n
        {0}, // sensors_field[]
        {0}, // sensors_num[]
        {0} // parms_num[]
    },
    // trace every tick
    [AMEC_TB_EVERY_TICK] = {
        "trace250us",  // name
        AMEFP(4000,0), // freq
        (UINT8*)(UINT32)g_amec_tb_bytes+AMEC_TB_2MS_SIZE_BYTES, // bytes
        AMEC_TB_250US_SIZE_BYTES, //size
        0, // entry_size
        0, // entry_n
        0, // write
        0, // read
        0, // sensors_n
        0, // parm_n
        {0}, // sensors_field[]
        {0}, // sensors_num[]
        {0} // parms_num[]
    }
};

//Throw a compiler error when the enum and array are not both updated
STATIC_ASSERT((AMEC_TB_NUMBER_OF_TRACES != (sizeof(g_amec_tb_list)/sizeof(amec_tb_t))));

///=1 signals a trace is being taken
UINT8   g_amec_tb_record=0;
///=1 signals continuous tracing
UINT8   g_amec_tb_continuous=0; //CL273

//*************************************************************************/
// Function Prototypes
//*************************************************************************/

//*************************************************************************/
// Functions
//*************************************************************************/
// Function Specification
//
// Name: amester_get_sensor_info
//
// Description: Returns name, units, update frequency, and scalefactor for a sensor
//
// Task Flags:
//
// End Function Specification
static uint8_t amester_get_sensor_info( uint8_t* o_resp, uint16_t* io_resp_length, const uint8_t i_type, const uint16_t i_sensor)
{
    uint8_t l_rc = COMPCODE_NORMAL; // assume no error
    sensor_t * l_sensor_ptr = NULL;
    uint16_t l_numOfSensors = 1;
    sensor_info_t l_sensorInfo;
    errlHndl_t l_err = NULL;
    uint16_t l_resp_length = 0;

    do
    {
        // Check o_resp and io_resp_length pointers
        if( (o_resp == NULL) ||
            (io_resp_length == NULL) )
        {
            l_rc = COMPCODE_UNSPECIFIED;
            break;
        }

        l_resp_length = *io_resp_length;
        *io_resp_length = 0;

        if (i_sensor >= MAX_AMEC_SENSORS )
        {
            l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
            break;
        }

        l_sensor_ptr = getSensorByGsid(i_sensor);
        if(l_sensor_ptr == NULL)
        {
            // Didn't find it
            l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
            break;
        }

        querySensorListArg_t l_qsl_arg = {
                i_sensor,          // i_startGsid
                0,               // i_present
                SENSOR_TYPE_ALL, // i_type
                SENSOR_LOC_ALL,  // i_loc
                &l_numOfSensors, // io_numOfSensors
                NULL,            // o_sensors
                &l_sensorInfo    // o_sensorInfoPtr
        };

        // Get sensor list
        l_err = querySensorList(&l_qsl_arg);

        if( NULL != l_err)
        {
            // Query failure, it should not happens
            TRAC_ERR("amester_get_sensor_info: Failed to get sensor list. Error status is : 0x%x", l_err->iv_reasonCode);

            // commit error log
            commitErrl( &l_err );

            l_rc = COMPCODE_UNSPECIFIED;
            break;
        }

        switch (i_type)
        {
            case AME_INFO_NAME:
            {
                char *src = l_sensorInfo.name;
                char *dest = (char*)o_resp;
                uint16_t l_length = strlen(src)+1; // add string terminator

                // Check length
                if(l_resp_length < l_length)
                {
                    l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
                    break;
                }
                // Copy string
                strcpy( dest, src );
                *io_resp_length = l_length;
                break;
            }

            case AME_INFO_UNITS:
            {
                char *src = l_sensorInfo.sensor.units;
                char *dest = (char*)o_resp;
                uint16_t l_length = strlen(src)+1; // add string terminator

                // Check length
                if(l_resp_length < l_length)
                {
                    l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
                    break;
                }

                // Copy string
                strcpy( dest, src );
                *io_resp_length = l_length;
                break;
            }

            case AME_INFO_FREQ:
            {
                uint16_t l_length = sizeof( uint32_t);

                // Check length
                if(l_resp_length < l_length)
                {
                    l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
                    break;
                }

                *((uint32_t *)o_resp) = l_sensorInfo.sensor.freq;
                *io_resp_length = l_length;
                break;
            }

            case AME_INFO_SCALE:
            {
                uint16_t l_length = sizeof( uint32_t);

                // Check length
                if(l_resp_length < l_length)
                {
                    l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
                    break;
                }

                *((uint32_t *)o_resp) = l_sensorInfo.sensor.scalefactor;
                *io_resp_length = l_length;
                break;
            }

            case AME_INFO_ALL:        //Added for AME API 2.16
            {
                char *src = NULL;
                char *dest = (char*)o_resp;
                uint16_t l_length = strlen(l_sensorInfo.name)         + 1 +\
                                    strlen(l_sensorInfo.sensor.units) + 1 + \
                                    sizeof(uint32_t) + sizeof(uint32_t);
                // Check length
                if(l_resp_length < l_length)
                {
                    l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
                    break;
                }

                src = l_sensorInfo.name;
                // Copy string
                strcpy( dest, src );
                dest += strlen(src)+1; // add string terminator

                src = l_sensorInfo.sensor.units;
                // Copy string
                strcpy( dest, src );
                dest += strlen(src)+1; // add string terminator

                *((uint32_t *)dest) = l_sensorInfo.sensor.freq;
                dest+= 4;

                *((uint32_t *)dest) = l_sensorInfo.sensor.scalefactor;
                dest+= 4;

                *io_resp_length = (uint8_t) ((uint32_t)dest - (uint32_t)o_resp);
                break;
            }

            default:
                l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
        } // End of switch case
    } while (0);
    return l_rc;
}

// Function Specification
//
// Name: amester_api
//
// Description: amester entry point for ipmicommand 0x3C
//
// Task Flags:
//
// End Function Specification
uint8_t amester_api( const IPMIMsg_t * i_msg,
                     uint16_t   * io_resp_length,
                     uint8_t    * o_resp )
{
    uint8_t l_rc = COMPCODE_NORMAL;
    uint8_t l_temp_buffer[ sizeof(sensor_info_t) ];
    sensor_t *l_sensor_ptr = NULL;
    uint16_t l_sensor_id = 0;  // sensor id
    uint16_t l_sensor_count = 0; // sensor count
    uint8_t l_sensor_type = 0; // sensor type
    uint16_t l_maxlen = 0, l_retlen = 0;  // for echo command, 0xfd and 0xff commands
    uint16_t l_resp_length = *io_resp_length;
    sensorrec_t SensorInfo;

    switch( i_msg->au8CmdData_ptr[0] )
    {
        // commands 0x01 ~ 0x1B are DEPRECATED except 0x07, 0x0A

        case 0x07:  // Get Multiple Sensor Data
        {
            uint16_t l_in;              // input pointer
            uint16_t l_out=0;           // output pointer
            char *t;
            int k;
            l_rc = COMPCODE_NORMAL;;    // assume no error

            // Process each sensor in turn
            for (l_in = 1; l_in + 1 < i_msg->u8CmdDataLen; l_in=l_in+2)
            {
                // exit when a return message is filled. -1 is for IPMI return code
                if (l_out + AME_SDRS > (G_amester_max_data_length - 1)) break;

                // Get the next sensor
                l_sensor_id = CONVERT_UINT8_ARRAY_UINT16(i_msg->au8CmdData_ptr[l_in],
                                                         i_msg->au8CmdData_ptr[l_in+1]);
                l_sensor_ptr = getSensorByGsid(l_sensor_id);
                if(l_sensor_ptr == NULL)
                {
                    // Mark which sensor number does not exist
                    o_resp[0] = (uint8_t)(l_in >> 8);
                    o_resp[1] = (uint8_t)(l_in);
                    *io_resp_length = 2;
                    l_rc = COMPCODE_DEST_UNAVAILABLE;
                    break;
                }

                /* Get a snap-shot of this sensors registers */
                /* This copy is required so the bytes in each field are self-consistent
                    since the AME interrupt can modify them at any time. Note that it is
                    possible that the fields are not consistent with each other, but this
                    is how Amester has always been. Use traces to get a consistent view.*/
                SensorInfo.timestamp=G_current_tick;
                SensorInfo.updates=l_sensor_ptr->update_tag;
                SensorInfo.accumulated_value=l_sensor_ptr->accumulator;
                SensorInfo.value=l_sensor_ptr->sample;
                SensorInfo.value_min=l_sensor_ptr->sample_min;
                SensorInfo.value_max=l_sensor_ptr->sample_max;
                memcpy(&SensorInfo.status,&l_sensor_ptr->status,sizeof(uint16_t));

                // Copy to output buffer.
                t=(char *)&SensorInfo;
                for (k=0;k<AME_SDRS;k++) o_resp[l_out++]=*t++;

               *io_resp_length = l_out;

            } // for each sensor

            break;
        };

        case 0x0a:  // Get API version
            o_resp[0] = AME_API_MAJ;
            o_resp[1] = AME_API_MIN;
            *io_resp_length = 2;
            l_rc = COMPCODE_NORMAL;
            break;

        //----------------------------------------------------------------------
        // AME 2.16 commands
        //----------------------------------------------------------------------
        case 0x1c:  // AME component level constants
            // Goal is to allow 1 IPMI to get a bunch of important data at startup.
            // Dramatically speedup Amester initialization

            // Check length
            if(l_resp_length < AME_COMPONENT_LEVEL_RSPCMD_LEN)
            {
                l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
                break;
            }
            o_resp[0] = AME_API_MAJ;
            o_resp[1] = AME_API_MIN;
            o_resp[2] = AME_VERSION_MAJ;
            o_resp[3] = AME_VERSION_MIN;
            o_resp[4] = CONVERT_UINT16_UINT8_HIGH(AME_YEAR);
            o_resp[5] = CONVERT_UINT16_UINT8_LOW(AME_YEAR);
            o_resp[6] = AME_MONTH;
            o_resp[7] = AME_DAY;
            o_resp[8] = CONVERT_UINT16_UINT8_HIGH(G_amec_sensor_count);
            o_resp[9] = CONVERT_UINT16_UINT8_LOW(G_amec_sensor_count);
            o_resp[10] = AMEC_TB_NUMBER_OF_TRACES;
            *io_resp_length = AME_COMPONENT_LEVEL_RSPCMD_LEN;
            l_rc = COMPCODE_NORMAL;
            break;

        case 0x1d:

            // Check length
            if(l_resp_length < 2)
            {
                l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
                break;
            }
            o_resp[0] = 0;
            o_resp[1] = 0;
            *io_resp_length = 2;
            l_rc = COMPCODE_NORMAL;
            break;

        //Clear min/max fields in all sensors.
        case 0x21:
            l_sensor_ptr = NULL;
            l_sensor_count = G_amec_sensor_count;

            // clear min/max
            uint16_t i = 0;
            for( i = 0;i < l_sensor_count; i++)
            {
                l_sensor_ptr = getSensorByGsid(i);
                sensor_clear_minmax(l_sensor_ptr, AMEC_SENSOR_CLEAR_SAMPLE_MINMAX);
            }
            *io_resp_length = 0;
            l_rc = COMPCODE_NORMAL;
            break;

        case 0x25:  // Get sensors info
            l_rc = COMPCODE_NORMAL;
            l_sensor_type = i_msg->au8CmdData_ptr[3];
            l_sensor_count = G_amec_sensor_count;
            l_sensor_id = CONVERT_UINT8_ARRAY_UINT16( i_msg->au8CmdData_ptr[1],
                                                      i_msg->au8CmdData_ptr[2] );
            uint16_t j = 0;
            uint16_t l_final_length = 0;

            for( j = l_sensor_id; j < l_sensor_count; j++)
            {
                *io_resp_length = sizeof(sensor_info_t);
                l_rc = amester_get_sensor_info(l_temp_buffer,io_resp_length,l_sensor_type,j);
                if(l_rc != COMPCODE_NORMAL)
                {
                    l_final_length = 0;
                    break;
                }

                // max response length is G_amester_max_data_length
                if( ((l_final_length+(*io_resp_length)) < G_amester_max_data_length) &&
                    ((l_final_length+(*io_resp_length)) < l_resp_length ) )
                {
                    memcpy( o_resp, l_temp_buffer, *io_resp_length); // Copy to final output buffer
                    o_resp += (*io_resp_length);
                    l_final_length = l_final_length+(*io_resp_length);
                }
                else
                {
                    break;
                }
            }
            *io_resp_length = l_final_length;
            break;

        // Trace buffer commands
        // Get trace buffer configuration
        case 0x30:
            amec_tb_cmd_info(i_msg,o_resp,io_resp_length,&l_rc);
            break;

        // Configure TB
        case 0x31:
            amec_tb_cmd_set_config(i_msg,o_resp,io_resp_length,&l_rc);
            break;

        // Read TB
        // Input is an index into the buffer.
        // Output is a full-sized response, possibly wrapping around at end of buffer.
        case 0x32:
            amec_tb_cmd_read(i_msg,o_resp,io_resp_length,&l_rc);
            break;

        // Start recording TB
        case 0x33:
            amec_tb_cmd_start_recording(i_msg,o_resp,io_resp_length,&l_rc);
            break;

        // Stop recording TB
        case 0x34:
            amec_tb_cmd_stop_recording(i_msg,o_resp,io_resp_length,&l_rc);
            break;

        // Get sensor table, not support
        case 0x35: //No support
            *io_resp_length = 0;
            l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
            break;

        // Get SCOM table, not support
        case 0x36: //No support
            *io_resp_length = 0;
            l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
            break;

        // Return all configurable parameters for a trace
        case 0x3f:
            amec_tb_cmd_get_config(i_msg,o_resp,io_resp_length,&l_rc);
            break;

        // Get number of parameters
        case 0x40:
            amec_parm_get_number(i_msg,o_resp,io_resp_length,&l_rc);
            break;

        // Return configuration of parameters starting with a given guid
        case 0x41:
            amec_parm_get_config(i_msg,o_resp,io_resp_length,&l_rc);
            break;

        // Read parameter
        case 0x42:
            amec_parm_read(i_msg,o_resp,io_resp_length,&l_rc);
            break;

        // Write parameter
        case 0x43:
            amec_parm_write(i_msg,o_resp,io_resp_length,&l_rc);
            break;

        // Partition management
        case 0x50: //No support
            *io_resp_length = 0;
            l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
            break;

        // Configure AMESTER data length
        case 0xfd:
            *io_resp_length = 0;
            // Check command data length
            if (i_msg->u8CmdDataLen == 3)
            {
                l_maxlen = CONVERT_UINT8_ARRAY_UINT16( i_msg->au8CmdData_ptr[1],
                                                       i_msg->au8CmdData_ptr[2]);

                // make sure the OCC command/response buffer supports the size -6 byte header
                // and the length isn't going below the IPMI limit (save performance)
                if( (l_maxlen > (CMDH_FSP_CMD_SIZE - 6)) ||
                    (l_maxlen < IPMI_MAX_MSG_SIZE) )
                {
                    l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
                }
                else
                {
                    G_amester_max_data_length = l_maxlen;
                    l_rc = COMPCODE_NORMAL;
                }

            }
            else
            {
                l_rc = COMPCODE_REQ_DATA_LEN_INVALID;
            }
            break;

        // Note: Amester uses the echo command to figure out how much data it is
        // allowed to send in 1 message to OCC.
        case 0xfe: //echo
            l_maxlen = G_amester_max_data_length - 1;  // -1 for completion code
            l_retlen = l_maxlen;

            // Pick the smaller of the input length and max output length.
            if (i_msg->u8CmdDataLen < l_maxlen)
            {
                l_retlen = i_msg->u8CmdDataLen;
            }

            // Check length
            if(l_resp_length < l_retlen)
            {
                l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
                break;
            }

            l_rc = COMPCODE_NORMAL; /* assume no error */
            // Copy back as much of the input message as possible.
            memcpy( o_resp, i_msg->au8CmdData_ptr, l_retlen);
            *io_resp_length = l_retlen;
            break;

        // Note: Amester uses this command to find out the maximum length output
        // message OCC supports.
        case 0xff:
            l_maxlen = G_amester_max_data_length - 1;  // -1 for completion code

            if (i_msg->u8CmdDataLen == 3)
            {
                l_maxlen = CONVERT_UINT8_ARRAY_UINT16( i_msg->au8CmdData_ptr[1],
                                                       i_msg->au8CmdData_ptr[2]);
            }

            if (l_maxlen > (G_amester_max_data_length -1))
            {
                l_maxlen = G_amester_max_data_length -1;
            }

            // Check length
            if(l_resp_length < l_maxlen)
            {
                l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
                break;
            }

            l_rc = COMPCODE_NORMAL; /* assume no error */
            for (i = 0; i<l_maxlen; i++)
            {
                o_resp[i] = i;
            }
            *io_resp_length = l_maxlen;
            break;

        default:
            *io_resp_length = 0;
            l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
    }

    return l_rc;
}


// Function Specification
//
// Name: amester_manual_throttle
//
// Description: Amester interface entry point for ipmicommand 0x3B
//
// Task Flags:
//
// End Function Specification
uint8_t amester_manual_throttle( const IPMIMsg_t * i_msg,
                                 uint16_t   * io_resp_length,
                                 uint8_t    * o_resp )

{
    /*------------------------------------------------------------------------*/
    /* Local variables                                                        */
    /*------------------------------------------------------------------------*/
    uint8_t  l_rc,temp1,temp2;
    uint16_t l_resp_length = *io_resp_length;
    uint16_t i,j,cc,idx,temp16;
    uint16_t k;
    uint32_t temp32a;
    uint32_t *temp32;
    /*------------------------------------------------------------------------*/
    /* Code                                                                   */
    /*------------------------------------------------------------------------*/

    switch (i_msg->au8CmdData_ptr[0])
    {
        case 0x03:    // CPU(s) Present Bit Mask
            // The CPU Present Bit Mask is now being generated by the
            // PROC component of OCC.

            // Check length
            if(l_resp_length < 2)
            {
                l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
                break;
            }

            o_resp[0] = CONVERT_UINT32_UINT8_UPPER_HIGH( G_present_hw_cores);
            o_resp[1] = CONVERT_UINT32_UINT8_UPPER_LOW( G_present_hw_cores);
            *io_resp_length = 2;
            l_rc = COMPCODE_NORMAL;
            break;

        case 0x04:    // Get last throttle value sent to CPU 0. DEPRECATED.
            *io_resp_length = 0;
            l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
            break;

        case 0x05:    // Get AME enable/disable flag (old style interface...do not use), no support
            *io_resp_length = 0;
            l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
            break;

        case 0x06:    // Get new PTVR (Power Threshold Vector Request), no support
            *io_resp_length = 0;
            l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
            break;

        case 0x07:    // Write individual AME parameters
            switch (i_msg->au8CmdData_ptr[1])
            {

                case 20:    // parameter 20: Set Probe Parameters
                {
                        if (i_msg->au8CmdData_ptr[2]> (NUM_AMEC_FW_PROBES-1))
                        {
                            o_resp[0]=i_msg->au8CmdData_ptr[2];
                            *io_resp_length=1;
                            l_rc=COMPCODE_PARAM_OUT_OF_RANGE;
                            break;
                        }
                        if (i_msg->au8CmdData_ptr[3] < 1)
                        {
                            o_resp[0]=i_msg->au8CmdData_ptr[2];
                            *io_resp_length=1;
                            l_rc=COMPCODE_PARAM_OUT_OF_RANGE;
                            break;
                        }

                        temp32a=((uint32_t)i_msg->au8CmdData_ptr[4]<<24)+((uint32_t)i_msg->au8CmdData_ptr[5]<<16);
                        temp32a=temp32a+((uint32_t)i_msg->au8CmdData_ptr[6]<<8)+((uint32_t)i_msg->au8CmdData_ptr[7]);
                        temp32=(uint32_t*)temp32a;

                        g_amec->ptr_probe250us[i_msg->au8CmdData_ptr[2]]=temp32;
                        g_amec->size_probe250us[i_msg->au8CmdData_ptr[2]]=i_msg->au8CmdData_ptr[3];
                        g_amec->index_probe250us[i_msg->au8CmdData_ptr[2]]=0;   // Reset index

                        o_resp[0]=i_msg->au8CmdData_ptr[2];     // Return probe #
                        *io_resp_length=1;
                        l_rc = COMPCODE_NORMAL;
                        break;
                    };

                case 22:   // parameter 22: Analytics parameters
                {
                    g_amec->analytics_group=i_msg->au8CmdData_ptr[2];         // Set group
                    g_amec->analytics_chip=i_msg->au8CmdData_ptr[3];          // Select which chip to analyze
                    g_amec->analytics_option=i_msg->au8CmdData_ptr[4];        // Select which option
                    g_amec->analytics_total_chips=i_msg->au8CmdData_ptr[5];   // Select total number of chips
                    g_amec->analytics_slot=i_msg->au8CmdData_ptr[6];          // Select time slot to read data
                    o_resp[0]=i_msg->au8CmdData_ptr[2];
                    o_resp[1]=i_msg->au8CmdData_ptr[3];
                    o_resp[2]=i_msg->au8CmdData_ptr[4];
                    o_resp[3]=i_msg->au8CmdData_ptr[5];
                    o_resp[4]=i_msg->au8CmdData_ptr[6];
                    *io_resp_length=5;
                    l_rc = COMPCODE_NORMAL;
                    break;
                  }

                case 23:   // parameter 23: CPM calibration parameters
                {
                    // g_amec->cpms_enabled=i_msg->au8CmdData_ptr[2];           // Enable CPMs
                    o_resp[0]=i_msg->au8CmdData_ptr[2];
                    *io_resp_length=1;
                    l_rc = COMPCODE_NORMAL;
                    break;
                }


                case 29:   // parameter 29: Control vector recording modes and stream rates.
                {
                    g_amec->stream_vector_rate=255; // First step is to set an invalid rate so no recording done at all
                    g_amec->stream_vector_mode=0; // Also is to assure NO recording during parameter changes
                    g_amec->stream_vector_group=i_msg->au8CmdData_ptr[4];  // Choose group #
                    g_amec->write_stream_index=(uint32_t)CONVERT_UINT8_ARRAY_UINT16(i_msg->au8CmdData_ptr[5],i_msg->au8CmdData_ptr[6]);
                    g_amec->stream_vector_delay=(uint32_t)CONVERT_UINT8_ARRAY_UINT16(i_msg->au8CmdData_ptr[7],i_msg->au8CmdData_ptr[8]);
                    g_amec->stream_vector_mode=i_msg->au8CmdData_ptr[2];  // Choose mode

                    switch (g_amec->stream_vector_group)
                    {
                        case 45:    //group 45 decimal (amec_analytics support)
                            g_amec->stream_vector_map[0]=0;                     // Leave space for 250usec time stamp
                            k = 1;
                            for (i=0; i<=(STREAM_VECTOR_SIZE_EX-2); i++)
                            {
                                g_amec->stream_vector_map[k++] =  &g_amec->analytics_array[i];
                            }
                            //gpEMP->stream_vector_map[64]=(void *) 0xffffffff;   // Termination of partial vector
                            g_amec->analytics_group=45;
                            g_amec->analytics_bad_output_count=2;  // drop first 2 frames of output
                            break;

                        default:
                            break;
                    }

                    // Final step is to set a valid rate to begin recording at
                    g_amec->stream_vector_rate=i_msg->au8CmdData_ptr[3];  // Choose stream rate
                    g_amec->recordflag=1; // Recording is now valid
                    *io_resp_length = 1;
                    l_rc = COMPCODE_NORMAL;
                    break;
                }

                case 64:    // support for THREADMODE group 44 recording
                    g_amec->analytics_threadmode=i_msg->au8CmdData_ptr[2];
                    g_amec->analytics_threadcountmax=i_msg->au8CmdData_ptr[3];
                    o_resp[0]=i_msg->au8CmdData_ptr[2];
                    o_resp[1]=i_msg->au8CmdData_ptr[3];
                    *io_resp_length=2;
                    l_rc = COMPCODE_NORMAL;
                    break;

                default:
                    *io_resp_length = 0;
                    l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
                    break;
               }

            break;

        case 0x08:    // Read individual AME parameters
            switch (i_msg->au8CmdData_ptr[1])
            {
                case 0x08:      // parameter 8: Set histogram copy interval in msec (4 bytes)
                    o_resp[0] = (uint8_t)(AME_HISTOGRAM_COPY_INTERVAL>>24);
                    o_resp[1] = (uint8_t)(AME_HISTOGRAM_COPY_INTERVAL>>16);
                    o_resp[2] = (uint8_t)(AME_HISTOGRAM_COPY_INTERVAL>>8);
                    o_resp[3] = (uint8_t)(AME_HISTOGRAM_COPY_INTERVAL);
                    *io_resp_length = 4;
                    l_rc = COMPCODE_NORMAL;
                    break;

               case 20:    // parameter 20: Read Probe Parameters
                    {
                        if (i_msg->au8CmdData_ptr[2]> (NUM_AMEC_FW_PROBES-1))
                        {
                            o_resp[0]=i_msg->au8CmdData_ptr[2];
                            *io_resp_length=1;
                            l_rc=COMPCODE_PARAM_OUT_OF_RANGE;
                            break;
                        }
                        o_resp[1]=g_amec->size_probe250us[i_msg->au8CmdData_ptr[2]];   // Return size of object read by probe in bytes
                        temp32=g_amec->ptr_probe250us[i_msg->au8CmdData_ptr[2]];       // Get copy of 32 bit probe ptr
                        temp32a=(uint32_t)temp32;
                        o_resp[5]=(uint8_t)temp32a;
                        o_resp[4]=(uint8_t)((uint32_t)temp32a>>8);
                        o_resp[3]=(uint8_t)((uint32_t)temp32a>>16);
                        o_resp[2]=(uint8_t)((uint32_t)temp32a>>24);
                        o_resp[0]=i_msg->au8CmdData_ptr[2];     // Return probe #
                        *io_resp_length=6;
                        l_rc=COMPCODE_NORMAL;
                        break;
                    };

                case 22:   // parameter 22: Analytics parameters
                    o_resp[0]=g_amec->analytics_group;
                    o_resp[1]=g_amec->analytics_chip;
                    o_resp[2]=g_amec->analytics_option;
                    o_resp[3]=g_amec->analytics_total_chips;
                    o_resp[4]=g_amec->analytics_slot;
                    *io_resp_length=5;
                    l_rc = COMPCODE_NORMAL;
                    break;

                case 23:   // parameter 23: CPM parameters
                    //      o_resp[0]=g_amec->cpms_enabled;
                    //      o_resp[1]=g_amec->cpm_active_core;
                    //      o_resp[2]=g_amec->cpm_cal_state;
                    //      o_resp[3]=g_amec->cpm_core_state;
                    //      o_resp[4]=g_amec->cpm_measure_state;
                    //      o_resp[5]=g_amec->cpm_cal_count;
                    *io_resp_length=6;
                    l_rc = COMPCODE_NORMAL;
                    break;

                case 29:   // parameter 29: Stream recording control parameters
                    o_resp[0]=(uint8_t)(g_amec->stream_vector_mode);
                    o_resp[1]=(uint8_t)(g_amec->stream_vector_rate);
                    o_resp[2]=(uint8_t)(g_amec->write_stream_index>>8);
                    o_resp[3]=(uint8_t)(g_amec->write_stream_index & 0xff);
                    o_resp[4]=(uint8_t)(g_amec->stream_vector_delay>>8);
                    o_resp[5]=(uint8_t)(g_amec->stream_vector_delay & 0xff);
                    *io_resp_length=6;
                    l_rc=COMPCODE_NORMAL;
                    break;

                case 37:   // parameter 37: Read out (G_amester_max_data_length-2*STREAM_VECTOR_SIZE) byte vector from
                           // streaming buffer
                    g_amec->read_stream_index=(uint32_t)((i_msg->au8CmdData_ptr[2]<<8)+i_msg->au8CmdData_ptr[3]);
                    temp1=i_msg->au8CmdData_ptr[4];
                    temp2=i_msg->au8CmdData_ptr[5];
                    if (g_amec->read_stream_index > (STREAM_BUFFER_SIZE-1*STREAM_VECTOR_SIZE_EX))
                    {
                        o_resp[0]=i_msg->au8CmdData_ptr[2];
                        *io_resp_length=1;
                        l_rc=COMPCODE_PARAM_OUT_OF_RANGE;
                        break;
                    }
                    if (temp1 > 1)   // No averaging is allowed when using large read sizes
                    {
                        o_resp[0]=i_msg->au8CmdData_ptr[4];
                        *io_resp_length=1;
                        l_rc=COMPCODE_PARAM_OUT_OF_RANGE;
                        break;
                    }
                    if (temp2 > 0)
                    {
                        o_resp[0]=i_msg->au8CmdData_ptr[5];
                        *io_resp_length=1;
                        l_rc=COMPCODE_PARAM_OUT_OF_RANGE;
                        break;
                    }
                    if (g_amec->write_stream_index >= g_amec->read_stream_index)
                    {
                        temp32a=g_amec->write_stream_index-g_amec->read_stream_index;
                    }
                    else
                    {
                        temp32a=STREAM_BUFFER_SIZE+g_amec->write_stream_index-g_amec->read_stream_index;
                    }
                    if (temp32a < 1*STREAM_VECTOR_SIZE_EX)
                    {
                        o_resp[0]=1;   // Indicate insufficient data, but return a zero return code
                        *io_resp_length=STREAM_VECTOR_SIZE_EX+3;  // # of bytes (STREAM_VECTOR_SIZE is in 16 bit words)
                        l_rc=COMPCODE_NORMAL;
                        break;
                    }
                    o_resp[0]=0;   // Indicate sufficient data
                    i=0;
                    j=1*STREAM_VECTOR_SIZE_EX;                   //  used to be 10*STREAM_VECTOR_SIZE_EX
                    cc=3;  // Begin just past return code and time stamp

                    for(idx = i; idx < j; idx++) // Skip first 1 entry: either write_index and time stamp
                    {
                        temp16 = (uint16_t)g_amec->ptr_stream_buffer[g_amec->read_stream_index + idx];
                        o_resp[cc] = (temp16 >> 8);
                        o_resp[cc + 1] = (temp16 & 0xff);
                        cc = cc + 2;  // output index
                    }
                    if(i_msg->au8CmdData_ptr[7] == 0)
                    {
                        temp16 = g_amec->ptr_stream_buffer[g_amec->read_stream_index]; // Send back time stamp
                    } else
                    {
                        temp16 = g_amec->write_stream_index;   // Send back write stream index
                    }
                    o_resp[1] = (uint8_t)(temp16 >> 8);
                    o_resp[2] = (uint8_t)(temp16 & 0xff);
                    *io_resp_length = 3 + 2 * (1 * STREAM_VECTOR_SIZE_EX);  // # of bytes (STREAM_VECTOR_SIZE_EX is in 16 bit words)
                    l_rc = COMPCODE_NORMAL;
                    break;

                case 64:    // support for THREADMODE group 45 recording
                    o_resp[0]=(uint8_t)(g_amec->analytics_threadmode);
                    o_resp[1]=(uint8_t)(g_amec->analytics_threadcountmax);
                    *io_resp_length=2;
                    l_rc=COMPCODE_NORMAL;
                    break;

                default:
                    *io_resp_length = 0;
                    l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
                    break;
            }
            break;

        default:
            *io_resp_length = 0;
            l_rc = COMPCODE_PARAM_OUT_OF_RANGE;
            break;
    } // end of switch

    return l_rc;
}

// Function Specification
//
// Name: AMEC_entry_point
//
// Description: Amester interface entry point
//
// Task Flags:
//
// End Function Specification
uint8_t amester_entry_point( const IPMIMsg_t * i_msg,
                             uint16_t   * io_resp_length,
                             uint8_t    * o_resp)

{
    uint8_t l_rc = COMPCODE_NORMAL;

    do
    {
        if( (i_msg == NULL) ||
            (io_resp_length == NULL) ||
            (o_resp == NULL) )
        {
            l_rc = COMPCODE_UNSPECIFIED;
            break;
        }

        switch (i_msg->u8Cmd)
        {
            case 0x3C:
                l_rc = amester_api( i_msg, io_resp_length, o_resp);
                break;

            case 0x3B:
                l_rc = amester_manual_throttle( i_msg, io_resp_length, o_resp);
                break;

            default:
                l_rc = COMPCODE_CMD_UNKNOWN;
                break;
        }
    } while (0);

    return l_rc;
}

amec_tb_t* AMEC_tb_get_by_guid(const AMEC_TB_GUID i_guid)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    amec_tb_t* l_tb_ptr = NULL;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        if(i_guid > AMEC_TB_NUMBER_OF_TRACES)
        {
            //TRACE(g_trac_amec,"E>amec_tb_get_by_guid:Invalid GUID (%d) max GUID supported at the moment is (%d)",i_guid,g_amec_tb_count);
            break;
        }
        //traces are arranged in an array by GUID
        l_tb_ptr = &g_amec_tb_list[i_guid];

    } while( 0 );

    return l_tb_ptr;
}

//
// Commands that run during AME interrupt time
//
void amec_tb_record(const AMEC_TB_GUID i_guid)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    UINT8                       *l_w; // pointer for tracebuffer writing
    UINT16                      l_i;
    sensor_t                    *l_s;
    UINT32                      l_totalbytes;
    amec_parm_t                 *l_parm;
    UINT8                       *l_src_ptr;
    AMEC_PARM_GUID              l_parm_guid;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    if(G_dcom_slv_inbox_rx.tb_record)
    {
        // Check for valid tb write entry index
        if(g_amec_tb_list[i_guid].write < g_amec_tb_list[i_guid].entry_n)
        {
            l_w = g_amec_tb_list[i_guid].bytes + g_amec_tb_list[i_guid].write * g_amec_tb_list[i_guid].entry_size;
            // Write the tracked sensors
            for(l_i = 0; l_i < g_amec_tb_list[i_guid].sensors_n; l_i++)
            {
                l_s = getSensorByGsid(g_amec_tb_list[i_guid].sensors_num[l_i]);
                switch(g_amec_tb_list[i_guid].sensors_field[l_i])
                {
                    case 0:
                        { //value
                            *l_w++ = (UINT8)(l_s->sample >> 8);
                            *l_w++ = (UINT8)(l_s->sample);
                            break;
                        }
                    case 1:
                        { //min
                            *l_w++ = (UINT8)(l_s->sample_min >> 8);
                            *l_w++ = (UINT8)(l_s->sample_min);
                            break;
                        }
                    case 2:
                        { //max
                            *l_w++ = (UINT8)(l_s->sample_max >> 8);
                            *l_w++ = (UINT8)(l_s->sample_max);
                            break;
                        }
                    case 3:
                        { //accumulator
                            *l_w++ = (UINT8)(l_s->accumulator >> 56);
                            *l_w++ = (UINT8)(l_s->accumulator >> 48);
                            *l_w++ = (UINT8)(l_s->accumulator >> 40);
                            *l_w++ = (UINT8)(l_s->accumulator >> 32);
                            *l_w++ = (UINT8)(l_s->accumulator >> 24);
                            *l_w++ = (UINT8)(l_s->accumulator >> 16);
                            *l_w++ = (UINT8)(l_s->accumulator >> 8);
                            *l_w++ = (UINT8)(l_s->accumulator);
                            break;
                        }
                    case 4:
                        { //update tag
                            *l_w++ = (UINT8)(l_s->update_tag >> 24);
                            *l_w++ = (UINT8)(l_s->update_tag >> 16);
                            *l_w++ = (UINT8)(l_s->update_tag >> 8);
                            *l_w++ = (UINT8)(l_s->update_tag);
                            break;
                        }
                    case 5:
                        { //test (not available in POWER8)
                            *l_w++ = (UINT8)0;
                            *l_w++ = (UINT8)0;
                            break;
                        }
                    case 6:
                        { //rcnt
                            *l_w++ = (UINT8)(g_amec->r_cnt >> 24);
                            *l_w++ = (UINT8)(g_amec->r_cnt >> 16);
                            *l_w++ = (UINT8)(g_amec->r_cnt >> 8);
                            *l_w++ = (UINT8)(g_amec->r_cnt);
                            break;
                        }
                    default:
                        break;
                }
            } // write sensors

            // Write the tracked parameters
            for(l_i = 0; l_i < g_amec_tb_list[i_guid].parm_n; l_i++)
            {
                l_parm_guid = g_amec_tb_list[i_guid].parm_num[l_i];
                if(g_amec_parm_list[l_parm_guid].preread) amec_parm_preread(l_parm_guid); // get latest version
                l_parm = &g_amec_parm_list[l_parm_guid];
                l_totalbytes = l_parm->length * l_parm->vector_length;
                l_src_ptr = l_parm->value_ptr;
                while(l_totalbytes--)
                {
                    *l_w++ = *l_src_ptr++;
                }
            }

            // Advance the write pointer and decide if recording should stop
            g_amec_tb_list[i_guid].write++; // Signal this entry is ready for reading
            if(g_amec_tb_list[i_guid].write >= g_amec_tb_list[i_guid].entry_n)
            {
                if(g_amec_tb_continuous)
                {
                    // Start at beginning of trace
                    g_amec_tb_list[i_guid].write = 0;
                } else
                {
                    // Stop recording if this trace is configured
                    if(g_amec_tb_list[i_guid].entry_n) g_amec_tb_record = 0;
                }
            }
        } // valid write index
    } // end if(record)}
}


// Get tb config.  Pack as many traces as possible
void amec_tb_cmd_info(const IPMIMsg_t *i_psMsg,
                      UINT8 *o_pu8Resp,
                      UINT16 *o_pu16RespLength,
                      UINT8 *o_retval)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    AMEC_TB_GUID                l_id;  // trace id
    UINT16                      l_j; // size of return message
    CHAR                        *l_src; //pointer for copying name

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    l_id = (AMEC_TB_GUID) i_psMsg->au8CmdData_ptr[1];
    l_j = 0; // write index byte for response

    for(; l_id < AMEC_TB_NUMBER_OF_TRACES; l_id++)
    {
        if(l_j + AMEC_TB_CONFIG_SIZE >= G_amester_max_data_length) break; // end of response buffer

        l_src = g_amec_tb_list[l_id].name;
        while(*l_src != 0)
        {
            o_pu8Resp[l_j++] = *l_src++;
        } /* copy string up until \0 */
        o_pu8Resp[l_j++] = '\0'; /* add string terminator */

        o_pu8Resp[l_j++] = (UINT8)(g_amec_tb_list[l_id].freq >> 24);
        o_pu8Resp[l_j++] = (UINT8)(g_amec_tb_list[l_id].freq >> 16);
        o_pu8Resp[l_j++] = (UINT8)(g_amec_tb_list[l_id].freq >> 8);
        o_pu8Resp[l_j++] = (UINT8)(g_amec_tb_list[l_id].freq);
        // has_scoms field from POWER7 is always 0 on POWER8
        o_pu8Resp[l_j++] = 0;
    }
    *o_pu16RespLength=l_j;
    *o_retval=COMPCODE_NORMAL;
    return;
}


void amec_tb_cmd_set_config(const IPMIMsg_t *i_psMsg,
                            UINT8 *o_pu8Resp,
                            UINT16 *o_pu16RespLength,
                            UINT8 *o_retval)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    UINT8                       l_i;
    UINT16                      l_sensors_n;
    UINT16                      l_oca_n;
    UINT16                      l_num_index;
    UINT16                      l_field_index;
    UINT8                       l_valid_sockets = 0;
    UINT32                      l_socket_bitmap = 0;
    UINT16                      l_parm_n;
    UINT16                      l_parm_index;
    amec_parm_t                 *l_parm;
    UINT8                       l_count;
    amec_tb_t                   *l_trace;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        *o_retval = COMPCODE_NORMAL;

        //Stop recording while setting up a trace.
        g_amec_tb_record = 0;

        // 0. Parse input
        l_trace = AMEC_tb_get_by_guid(i_psMsg->au8CmdData_ptr[1]);
        l_sensors_n = CONVERT_UINT8_ARRAY_UINT16(
            i_psMsg->au8CmdData_ptr[2],
            i_psMsg->au8CmdData_ptr[3]
            );
        l_parm_n = CONVERT_UINT8_ARRAY_UINT16(
            i_psMsg->au8CmdData_ptr[4],
            i_psMsg->au8CmdData_ptr[5]
            );
        l_oca_n = CONVERT_UINT8_ARRAY_UINT16(
            i_psMsg->au8CmdData_ptr[6],
            i_psMsg->au8CmdData_ptr[7]
            );
        l_socket_bitmap = CONVERT_UINT8_ARRAY_UINT32(
            i_psMsg->au8CmdData_ptr[8],
            i_psMsg->au8CmdData_ptr[9],
            i_psMsg->au8CmdData_ptr[10],
            i_psMsg->au8CmdData_ptr[11]
            );

        if(l_sensors_n > AMEC_TB_SENSORS_MAX)
        {
            *o_retval = COMPCODE_PARAM_OUT_OF_RANGE;
            o_pu8Resp[0] = 0;  // mark second byte is bad.
            o_pu8Resp[1] = 2;  // mark second byte is bad.
            *o_pu16RespLength = 2;
            break;
        }

        if(l_parm_n > AMEC_TB_PARM_MAX)
        {
            *o_retval = COMPCODE_PARAM_OUT_OF_RANGE;
            o_pu8Resp[0] = 0;  // mark fourth byte is bad.
            o_pu8Resp[1] = 4;  //
            *o_pu16RespLength = 2;
        }

        if(l_oca_n > OCA_MAX_ENTRIES)
        {
            *o_retval = COMPCODE_PARAM_OUT_OF_RANGE;
            o_pu8Resp[0] = 0;  // mark sixth byte is bad
            o_pu8Resp[1] = 6;
            *o_pu16RespLength = 2;
            break;
        }

        // Count valid sockets
        l_count = l_socket_bitmap;
        while(l_count)
        {
            if(l_count & 0x01)
            {l_valid_sockets++;}
            l_count >>= 1;
        }
        if(l_valid_sockets > MAX_NUM_CHIPS)
        {
            l_valid_sockets = MAX_NUM_CHIPS;
        }

        l_trace->entry_size = 0;

        // Set pointers to input
        l_num_index = 12; // start of sensor numbers
        l_field_index = 12 + 2 * l_sensors_n;  // start of sensor fields
        l_parm_index = l_field_index + l_sensors_n;  // start of parameters

        // Read sensor configuration
        for(l_i = 0; l_i < l_sensors_n; l_i++)
        {
            l_trace->sensors_num[l_i] = CONVERT_UINT8_ARRAY_UINT16(
                i_psMsg->au8CmdData_ptr[l_num_index],
                i_psMsg->au8CmdData_ptr[l_num_index + 1]
                );
            l_trace->sensors_field[l_i] = i_psMsg->au8CmdData_ptr[l_field_index];

            if(l_trace->sensors_num[l_i] >= G_amec_sensor_count)
            {
                *o_retval = COMPCODE_PARAM_OUT_OF_RANGE;
                o_pu8Resp[0] = (UINT8)(l_num_index >> 8);  // mark which byte input is bad
                o_pu8Resp[1] = (UINT8)(l_num_index);  // mark which byte input is bad
                *o_pu16RespLength = 2;
                break;
            }

            if(l_trace->sensors_field[l_i] > 6)
            {
                *o_retval = COMPCODE_PARAM_OUT_OF_RANGE;
                o_pu8Resp[0] = (UINT8)(l_field_index >> 8);  // mark which byte input is bad
                o_pu8Resp[1] = (UINT8)(l_field_index);  // mark which byte input is bad
                *o_pu16RespLength = 2;
                return;
            }

            switch(l_trace->sensors_field[l_i])
            {
                case 0: // value
                case 1: // min
                case 2: // max
                    l_trace->entry_size += 2;
                    break;
                case 3: //acc
                    l_trace->entry_size += 8;
                    break;
                case 4: //updates
                    l_trace->entry_size += 4;
                    break;
                case 5: //test
                    l_trace->entry_size += 2;
                    break;
                case 6: //rcnt
                    l_trace->entry_size += 4;
                    break;
                default:
                    break;
            }

            l_num_index += 2;
            l_field_index++;
        }

        if(*o_retval) break;

        // Record number of sensors in this trace
        l_trace->sensors_n = l_sensors_n;

        // Read Parameter configuration
        for(l_i = 0; l_i < l_parm_n; l_i++)
        {
            l_trace->parm_num[l_i] = CONVERT_UINT8_ARRAY_UINT16(
                i_psMsg->au8CmdData_ptr[l_parm_index],
                i_psMsg->au8CmdData_ptr[l_parm_index + 1]
                );

            if(l_trace->parm_num[l_i] >= AMEC_PARM_NUMBER_OF_PARAMETERS)
            {
                *o_retval = COMPCODE_PARAM_OUT_OF_RANGE;
                o_pu8Resp[0] = (UINT8)(l_parm_index >> 8);  // mark which byte input is bad
                o_pu8Resp[1] = (UINT8)(l_parm_index);  // mark which byte input is bad
                *o_pu16RespLength = 2;
                break;
            }
            l_parm = &g_amec_parm_list[l_trace->parm_num[l_i]];
            l_trace->entry_size += l_parm->length * l_parm->vector_length;
            l_parm_index += 2;
        }

        if(*o_retval) break;

        // Record this number of parameters in the trace
        l_trace->parm_n = l_parm_n;

        l_trace->entry_n = l_trace->size / l_trace->entry_size;
        l_trace->read = 0;
        l_trace->write = 0;

        *o_pu16RespLength = 0;
    } while(0);

    return;
}

void amec_tb_cmd_start_recording(const IPMIMsg_t *i_psMsg,
                                 UINT8 *o_pu8Resp,
                                 UINT16 *o_pu16RespLength,
                                 UINT8 *o_retval)
{
    g_amec_tb_record = 1;
    *o_pu16RespLength = 0;
    *o_retval=COMPCODE_NORMAL;
}

void amec_tb_cmd_stop_recording(const IPMIMsg_t *i_psMsg,
                                UINT8 *o_pu8Resp,
                                UINT16 *o_pu16RespLength,
                                UINT8 *o_retval)
{
    g_amec_tb_record = 0;
    *o_pu16RespLength = 0;
    *o_retval=COMPCODE_NORMAL;
}

void amec_tb_cmd_read(const IPMIMsg_t *i_psMsg,
                      UINT8 *o_pu8Resp,
                      UINT16 *o_pu16RespLength,
                      UINT8 *o_retval)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    amec_tb_t *l_trace;
    UINT16 l_i=0;  // output index
    UINT32 l_j; // index to copy from

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    do
    {
        *o_retval = COMPCODE_NORMAL; /* assume no error */

        // Parse input command
        l_trace = AMEC_tb_get_by_guid(i_psMsg->au8CmdData_ptr[1]);
        if(l_trace == NULL)
        {
            *o_retval = COMPCODE_PARAM_OUT_OF_RANGE;
            *o_pu16RespLength = 0;
            break;
        }
        l_j = CONVERT_UINT8_ARRAY_UINT32(
            i_psMsg->au8CmdData_ptr[2],
            i_psMsg->au8CmdData_ptr[3],
            i_psMsg->au8CmdData_ptr[4],
            i_psMsg->au8CmdData_ptr[5]
            );

        // Copy bytes to be read into response buffer. -1 since return code is 1B
        for(l_i = 0; l_i < (G_amester_max_data_length - 1); l_i++, l_j++)
        {
            if(l_j >= l_trace->size) // wrap around to beginning of buffer.
            {
                l_j = 0;
            }
            o_pu8Resp[l_i] = l_trace->bytes[l_j];
        }
        *o_pu16RespLength = l_i;
    } while(0);
}


void amec_tb_cmd_get_config(const IPMIMsg_t *i_psMsg,
                            UINT8 *o_pu8Resp,
                            UINT16 *o_pu16RespLength,
                            UINT8 *o_retval)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    amec_tb_t *l_trace;
    UINT8 l_i = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    do
    {
        *o_retval = COMPCODE_NORMAL;

        l_trace = AMEC_tb_get_by_guid(i_psMsg->au8CmdData_ptr[1]);
        if(l_trace == NULL)
        {
            *o_retval = COMPCODE_PARAM_OUT_OF_RANGE;
            *o_pu16RespLength = 0;
            break;
        }

        o_pu8Resp[l_i++] = (UINT8)((UINT32)l_trace->bytes >> 24);
        o_pu8Resp[l_i++] = (UINT8)((UINT32)l_trace->bytes >> 16);
        o_pu8Resp[l_i++] = (UINT8)((UINT32)l_trace->bytes >> 8);
        o_pu8Resp[l_i++] = (UINT8)((UINT32)l_trace->bytes);
        o_pu8Resp[l_i++] = (UINT8)(l_trace->size >> 24);
        o_pu8Resp[l_i++] = (UINT8)(l_trace->size >> 16);
        o_pu8Resp[l_i++] = (UINT8)(l_trace->size >> 8);
        o_pu8Resp[l_i++] = (UINT8)(l_trace->size);
        o_pu8Resp[l_i++] = (UINT8)(l_trace->entry_size >> 24);
        o_pu8Resp[l_i++] = (UINT8)(l_trace->entry_size >> 16);
        o_pu8Resp[l_i++] = (UINT8)(l_trace->entry_size >> 8);
        o_pu8Resp[l_i++] = (UINT8)(l_trace->entry_size);
        o_pu8Resp[l_i++] = (UINT8)(l_trace->entry_n >> 24);
        o_pu8Resp[l_i++] = (UINT8)(l_trace->entry_n >> 16);
        o_pu8Resp[l_i++] = (UINT8)(l_trace->entry_n >> 8);
        o_pu8Resp[l_i++] = (UINT8)(l_trace->entry_n);
        // 32-bit oca_offset field is always 0 on POWER8
        o_pu8Resp[l_i++] = 0;
        o_pu8Resp[l_i++] = 0;
        o_pu8Resp[l_i++] = 0;
        o_pu8Resp[l_i++] = 0;
        o_pu8Resp[l_i++] = (UINT8)((UINT32)l_trace->write >> 24);
        o_pu8Resp[l_i++] = (UINT8)((UINT32)l_trace->write >> 16);
        o_pu8Resp[l_i++] = (UINT8)((UINT32)l_trace->write >> 8);
        o_pu8Resp[l_i++] = (UINT8)((UINT32)l_trace->write);
        // 32-bit write_oca field is always 0 on POWER8
        o_pu8Resp[l_i++] = 0;
        o_pu8Resp[l_i++] = 0;
        o_pu8Resp[l_i++] = 0;
        o_pu8Resp[l_i++] = 0;
        o_pu8Resp[l_i++] = (UINT8)((UINT32)l_trace->read >> 24);
        o_pu8Resp[l_i++] = (UINT8)((UINT32)l_trace->read >> 16);
        o_pu8Resp[l_i++] = (UINT8)((UINT32)l_trace->read >> 8);
        o_pu8Resp[l_i++] = (UINT8)((UINT32)l_trace->read);
        o_pu8Resp[l_i++] = l_trace->sensors_n;
        o_pu8Resp[l_i++] = l_trace->parm_n;
        // oca_n field from POWER7 is always 0 on POWER8
        o_pu8Resp[l_i++] = 0;

        o_pu8Resp[l_i++] = g_amec_tb_record;
        o_pu8Resp[l_i++] = g_amec_tb_continuous;
        o_pu8Resp[l_i++] = (UINT8)((UINT16)AMEC_TB_SENSORS_MAX);
        o_pu8Resp[l_i++] = (UINT8)((UINT16)OCA_MAX_ENTRIES);
        o_pu8Resp[l_i++] = (UINT8)(MAX_NUM_CHIPS);
        o_pu8Resp[l_i++] = (UINT8)(AMEC_TB_PARM_MAX);
        *o_pu16RespLength = l_i;

    } while(0);
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
