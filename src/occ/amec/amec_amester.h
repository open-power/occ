/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_amester.h $                                 */
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

#ifndef _AMEC_AMESTER_H
#define _AMEC_AMESTER_H

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
/* Define Completion Codes (IPMI 1.5 Table 5-2) */
#define COMPCODE_NORMAL                     0x00
#define COMPCODE_NODE_BUSY                  0xC0
#define COMPCODE_CMD_UNKNOWN                0xC1
#define COMPCODE_CMD_INVALID_FOR_LUN        0xC2
#define COMPCODE_TIMEOUT                    0xC3
#define COMPCODE_OUT_OF_SPACE               0xC4
#define COMPCODE_RESERVATION_CANCELLED      0xC5
#define COMPCODE_REQ_DATA_TRUNCATED         0xC6
#define COMPCODE_REQ_DATA_LEN_INVALID       0xC7
#define COMPCODE_REQ_LEN_LIMIT_EXCEEDED     0xC8
#define COMPCODE_PARAM_OUT_OF_RANGE         0xC9
#define COMPCODE_REQ_BYTE_CNT_ERR           0xCA
#define COMPCODE_OBJ_ABSENT                 0xCB
#define COMPCODE_REQ_FIELD_INVALID          0xCC
#define COMPCODE_CMD_ILL_FOR_SEN            0xCD
#define COMPCODE_RESP_UNAVAILABLE           0xCE
#define COMPCODE_REQ_DUPLICATE              0xCF
#define COMPCODE_SDR_REPO_IN_UPD            0xD0
#define COMPCODE_FW_IN_UPD                  0xD1
#define COMPCODE_INIT_IN_PROGRESS           0xD2
#define COMPCODE_DEST_UNAVAILABLE           0xD3
#define COMPCODE_WRONG_PRIV                 0xD4
#define COMPCODE_CUR_NOT_SUPPORT            0xD5
#define COMPCODE_UNSPECIFIED                0xFF

//Habanero uses IPMI, which is constrained to 256 Byte IPMI response.
//This will slow down the Amester connection on FSP-based systems
#define     IPMI_MAX_MSG_SIZE      246     // BMC size
#define     AMEC_AME_CMD_HEADER_SZ 2

// Autonomic Management of Energy (AME) Parameters
#define AME_API_MAJ     2       // API version major
#define AME_API_MIN     27      // API version minor

#define AME_VERSION_MAJ 7       // Major Version (e.g. Ver. 1.4 has MAJ=1)
#define AME_VERSION_MIN 28      // Minor Version (e.g. Ver. 1.4 has MIN=4)

#define AME_YEAR        2015    // Year of Release (e.g. 2006)
#define AME_MONTH       7       // Month of Release (e.g. September=9)
#define AME_DAY         31       // Day of Release

#define AME_SDRS        24      // AME Sensor Data Record Size: 24 bytes

// AME data types for AME_GetInfo_*() functions
#define AME_INFO_NAME   0
#define AME_INFO_FREQ   1
#define AME_INFO_UNITS  2
#define AME_INFO_SCALE  3
#define AME_INFO_KIND   4
#define AME_INFO_ALL    5

#define COMMON_MAX_MTU_SIZE 2056

// The command response length
#define AME_COMPONENT_LEVEL_RSPCMD_LEN 11

// Histogram copy interval in milliseconds (default to 8 seconds)
#define AME_HISTOGRAM_COPY_INTERVAL     8000

// WARNING -> STREAM_BUFFER_SIZE must be a perfect multiple of the vector size.
#define STREAM_VECTOR_SIZE      32    // # of 16 bit elements in a stream vector-> must be a power of 2
#define SHIFT_VECTOR_SIZE        5    // Log base 2 of STREAM_VECTOR_SIZE for shifting
#define INJECTION_BUFFER_SIZE   32    // Size of injection buffer (must be a power of 2)
#define STREAM_VECTOR_SIZE_EX   74    // # of 16 bit elements in a stream vector
#define STREAM_BUFFER_SIZE  (40*1*STREAM_VECTOR_SIZE_EX)  // Stream buffer size in 16 bit samples for recording real time data to stream to Amester
#define MAX_SENSORS_ANALYTICS   134   // Maximum sensors making up the analytics group 45 (includes all Centaur data & L4 data & supports 12 cores)
#define MSA                     MAX_SENSORS_ANALYTICS

#define OCA_MAX_ENTRIES         0 // no POWER7 OCA on POWER8
#define AMEC_TB_SENSORS_MAX     40
#define AMEC_TB_PARM_MAX        40

// Macro for dividing two UINT32
// Returns a 32-bit value with the quotient of the division.
#define UTIL_DIV32(I_NUM, I_DEN) (((UINT32)(I_NUM)) / ((UINT32)(I_DEN)))

//*************************************************************************
// Structures
//*************************************************************************
typedef struct
{
    /** The IPMI command ID */
    uint8_t               u8Cmd;
    /** The IPMI command data length */
    uint16_t               u8CmdDataLen;
    /** the IPMI command data (including completion code) */
    uint8_t               *au8CmdData_ptr;
} IPMIMsg_t;

typedef struct sensorrec
{
    uint32_t  timestamp;
    uint32_t  updates;
    uint64_t  accumulated_value;
    uint16_t  value;
    uint16_t  value_min;
    uint16_t  value_max;
    uint16_t  status;   // bit 0, LSB: calibration complete on this sensor (=1);
                        //             no calibration complete (=0)
                        // bit 1: histogram gets updated (=1);
                        //        histogram is frozen (=0)
                        // bit 2: histogram gets reset (=1);
                        //        histogram not being reset (=0)
                        //        WARNING -> bit 2 only takes effect when bit 1
                        //        is toggled high. As soon as the histogram of
                        //        interest is reset, bit 2 is cleared to a 0
                        //        automatically. It is expected that the
                        //        external entity will carry out a reset, first
                        //        by setting bit 1 to a 0 to freeze updating,
                        //        then set bit 2 to a 1 to enable a reset, and
                        //        then set bit 1 to a 1 to enable a reset and
                        //        immediate updating again from a known state
                        //        of 0 in all counters.
                        // bit 3: select buffering area to be a time series
                        //        trace (=1) or a histogram (=0) (msw324)
                        // bit 4: if set to 1, time series trigger is armed;
                        //        if =0, trigger happened (msw354)
                        // bit 5: if set to 1, then time series buffer trace
                        //        ptr was reset to 0; this bit is set to 0
                        //        whenever the next histogram snapshot interval
                        //        arrives. If the bit stays at 0, and the
                        //        next histogram snapshot interval arrives,
                        //        then the update tag is not incremented. This
                        //        allows for much shorter intervals when using
                        //        exclusively time series modes for all the
                        //        sensors with a buffer because the Amester
                        //        never sees the update tag keep getting
                        //        incremented once the recording of each time
                        //        trace is completed.  msw357
                        // bit 6: if set to 1, this forces all sensor snapshots
                        //        to occur simultaneously in time msw366
                        //        This is important for histograms to be
                        //        compared. if set to 0, snapshots will be
                        //        spreadout with one done every 64msec once
                        //        the snap-shot interval is finished. This
                        //        flattens out the cycles for a large number
                        //        of sensors being monitored for time-tracing
                        //        where the contents of the histogram/time
                        //        trace buffer aren't changing once the trace
                        //        has finished recording, so snapshotting all
                        //        these sensors at the same time instant
                        //        is not essential. Normally, if all histograms
                        //        are in use, set this bit to 1. If all
                        //        time-series are in used, set this to 0. If
                        //        mixed, be very careful about the choice!
                        // bits 11 to 13:
                        //        encodes function to be performed in creating a vector
                        //        sensor's sample_reg output.
                        //        000 -> find max of vector elements
                        //        001 -> find min of vector elements
                        //        010 -> find average of vector elements
                        //        011 to 111 are available for new functions.
                        // bit 14:if set to 1, high resolution mode is used for
                        //        histograms and tracing on all vector
                        //        sensors. All samples of the vector pass
                        //        through add_histogram routine every time
                        //        the vector is updated. if set to 0, low
                        //        resolution mode is used, and only sample_reg
                        //        (latest max or min of the vector) is passed
                        //        through add_histogram every time.
                        // bit 15:if set to 1, this is an AME vector sensor.
                        //        if set to 0, this is a normal AME scalar
                        //        sensor.

} sensorrec_t;

typedef UINT8                   AMEC_TB_GUID;
typedef UINT16                  AMEC_SENSOR_GUID;

typedef enum
{
    AMEC_TB_2MS,        // GUID for 2ms trace
    AMEC_TB_250US,      // GUID for 250us trace
    AMEC_TB_NUMBER_OF_TRACES // Number of trace buffers supported
} AMEC_TB_ENUM;

typedef struct amec_tb
{
    // Constant properties

    ///Trace name
    CHAR*                       name;
    ///Update Frequency
    UINT32                      freq;

    /* Configurable properties */
    ///Pointer to raw bytes for trace buffer
    UINT8*                      bytes;
    ///Number of raw bytes
    UINT32  size;
    ///Number of bytes in 1 trace record. Used to increment write pointer.
    UINT32  entry_size;
    ///Number of records that fit in the trace buffer. (0 to n-1 can be used)
    UINT32  entry_n;
    ///Record number to write next for sensor data
    UINT32  write;
    ///Record number for next read (not used yet)
    UINT32  read;
    ///Number of sensors tracked in tb
    UINT8   sensors_n;
    ///Number of paramters tracked in tb
    UINT8 parm_n;
    ///List of corresponding sensor fields tracked in this trace buffer
    UINT8   sensors_field[AMEC_TB_SENSORS_MAX];
    ///List of sensors tracked in this trace buffer
    AMEC_SENSOR_GUID  sensors_num[AMEC_TB_SENSORS_MAX];
    ///List of parameters tracked
    uint16_t parm_num[AMEC_TB_PARM_MAX];

} amec_tb_t;

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************
//Amester interface entry point
uint8_t amester_entry_point( const IPMIMsg_t * i_msg,
                             uint16_t   * o_resp_length,
                             uint8_t    * o_resp);

// Write sensor data to trace record
// Called periodically to write next trace record with sensor data.
void amec_tb_record(const AMEC_TB_GUID i_guid);

// Get global information on traces (names, frequencies)
// Get a list of all available trace buffers in OCC and their frequencies.
void amec_tb_cmd_info(const IPMIMsg_t * i_psMsg, UINT8 *o_pu8Resp,UINT16 *o_pu16RespLength,UINT8 *o_retval);

// Set the configuration of a trace (which sensors to trace)
// Choose which sensors and SCOMs to trace. Choose size of trace buffer memory.
void amec_tb_cmd_set_config(const IPMIMsg_t *i_psMsg, UINT8 *o_pu8Resp,UINT16 *o_pu16RespLength,UINT8 *o_retval);

// Begin recording all configured traces
void amec_tb_cmd_start_recording(const IPMIMsg_t *i_psMsg,UINT8 *o_pu8Resp,UINT16 *o_pu16RespLength,UINT8 *o_retval);

// Stop recording all traces
void amec_tb_cmd_stop_recording(const IPMIMsg_t *i_psMsg,UINT8 *o_pu8Resp,UINT16 *o_pu16RespLength,UINT8 *o_retval);

// Get bytes from trace buffer memory
// Returns a maximum size packet starting at a given index
void amec_tb_cmd_read(const IPMIMsg_t *i_psMsg,UINT8 *o_pu8Resp,UINT16 *o_pu16RespLength,UINT8 *o_retval);

// Returns configuration of a trace
void amec_tb_cmd_get_config(const IPMIMsg_t *i_psMsg,UINT8 *o_pu8Resp,UINT16 *o_pu16RespLength,UINT8 *o_retval);

#endif
