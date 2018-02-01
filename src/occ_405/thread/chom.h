/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/thread/chom.h $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2018                        */
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

#ifndef _CHOM_H
#define _CHOM_H

#include <occ_common.h>
#include <trac_interface.h>
#include <apss.h>

#define  CHOM_GEN_LOG_PERIODIC_TIME     86400 // seconds in a day
#define  CHOM_VERSION                   0x02
// Max size of chom data log
#define  CHOM_LOG_DATA_MAX              3072
// Max number of memory bandwidth CHOM sensors
#define  MAX_NUM_MEMORY_SENSORS         32
// Max number of procs Call Home will get data for
#define CHOM_MAX_OCCS                   4
// Max number of error history entries to add to call home log
#define CHOM_MAX_ERRH_ENTRIES            4
// List of call home sensors (Max 126)
enum
{
    // Node total power (DC)
    CHOMPWR                = 0,
    // APSS Channels
    CHOMPWRAPSSCH0,
    CHOMPWRAPSSCH1,
    CHOMPWRAPSSCH2,
    CHOMPWRAPSSCH3,
    CHOMPWRAPSSCH4,
    CHOMPWRAPSSCH5,
    CHOMPWRAPSSCH6,
    CHOMPWRAPSSCH7,
    CHOMPWRAPSSCH8,
    CHOMPWRAPSSCH9,
    CHOMPWRAPSSCH10,
    CHOMPWRAPSSCH11,
    CHOMPWRAPSSCH12,
    CHOMPWRAPSSCH13,
    CHOMPWRAPSSCH14,
    CHOMPWRAPSSCH15,
    // Processor frequency
    CHOMFREQP0,
    CHOMFREQP1,
    CHOMFREQP2,
    CHOMFREQP3,
    // Processor utilization sensor
    CHOMUTILP0,
    CHOMUTILP1,
    CHOMUTILP2,
    CHOMUTILP3,
    // Proc temperatures across all nodes
    CHOMTEMPPROC0,
    CHOMTEMPPROC1,
    CHOMTEMPPROC2,
    CHOMTEMPPROC3,
    // Centaur temperature for all Centaurs in the node
    CHOMTEMPCENTP0,
    CHOMTEMPCENTP1,
    CHOMTEMPCENTP2,
    CHOMTEMPCENTP3,
    // Dimm temperature for all Dimms in the node
    CHOMTEMPDIMMP0,
    CHOMTEMPDIMMP1,
    CHOMTEMPDIMMP2,
    CHOMTEMPDIMMP3,
    // VRM VDD temperature per proc
    CHOMTEMPVDDP0,
    CHOMTEMPVDDP1,
    CHOMTEMPVDDP2,
    CHOMTEMPVDDP3,
    // Instructions per second sensor
    CHOMIPS,
    // Memory bandwidth for process memory controller
    CHOMBWP0M0,
    CHOMBWP0M1,
    CHOMBWP0M2,
    CHOMBWP0M3,
    CHOMBWP0M4,
    CHOMBWP0M5,
    CHOMBWP0M6,
    CHOMBWP0M7,
    CHOMBWP1M0,
    CHOMBWP1M1,
    CHOMBWP1M2,
    CHOMBWP1M3,
    CHOMBWP1M4,
    CHOMBWP1M5,
    CHOMBWP1M6,
    CHOMBWP1M7,
    CHOMBWP2M0,
    CHOMBWP2M1,
    CHOMBWP2M2,
    CHOMBWP2M3,
    CHOMBWP2M4,
    CHOMBWP2M5,
    CHOMBWP2M6,
    CHOMBWP2M7,
    CHOMBWP3M0,
    CHOMBWP3M1,
    CHOMBWP3M2,
    CHOMBWP3M3,
    CHOMBWP3M4,
    CHOMBWP3M5,
    CHOMBWP3M6,
    CHOMBWP3M7,
    // The number of chom sensors reported
    CHOM_NUM_OF_SENSORS
};

enum chom_supported_modes
{
    CHOM_MODE_NOMINAL,
    CHOM_MODE_SPS,
    CHOM_MODE_DPS,
    CHOM_MODE_DPS_MP,
    CHOM_MODE_FFO,
    CHOM_MODE_NOM_PERF,
    CHOM_MODE_MAX_PERF,
    CHOM_MODE_FMF,
    // number of modes required to run Call home
    NUM_CHOM_MODES
};
// Call home sensor Structure
struct ChomSensor
{
    uint16_t    sample;           // last sample value during the polling period
    uint16_t    sampleMin;        // min sample value recorded during polling period
    uint16_t    sampleMax;        // max sample value recorded during polling period
    uint16_t    average;          // average sample value during polling period
    uint32_t    accumulator;      // accumulator register to computer the average
} __attribute__ ((__packed__));

typedef struct ChomSensor ChomSensor_t;


// Power mode structure
struct ChomPwrMode
{
    uint8_t     mode;            // OCC power mode
    uint32_t    numOfSamples;    // Number of times samples were polled while in this mode
}__attribute__ ((__packed__));

typedef struct ChomPwrMode ChomPwrMode_t;

// Call home data structure
struct ChomNodeData
{
    uint32_t       eyecatcher;                    // "CHOM" will mark the beginning of the data
    uint8_t        version;                       // version of call home data being reported
    uint8_t        curPwrMode;                    // the current power mode at the time of the polling event
    uint32_t       totalTime;                     // duration of the polling period
    uint8_t        modeInLog;                     // the number of different power mode in the polling period
    uint8_t        channelFuncIds[MAX_APSS_ADC_CHANNELS];
    uint16_t       numSensors;                    // the number of sensors for which call home data was collected
    // error history counts. Only collect on up to 3 slaves, excluding master
    error_history_count_t errhCounts[CHOM_MAX_OCCS-1][CHOM_MAX_ERRH_ENTRIES];
    uint32_t       fClipHist[CHOM_MAX_OCCS-1];
} __attribute__ ((__packed__));

typedef struct ChomNodeData ChomNodeData_t;

// Call home sensor data
struct ChomSensorData
{
    ChomPwrMode_t   pwrMode;
    ChomSensor_t    sensor[CHOM_NUM_OF_SENSORS];
}__attribute__ ((__packed__));

typedef struct ChomSensorData ChomSensorData_t;

// Call home log data
struct ChomLogData
{
    ChomNodeData_t    nodeData;      // general node data
    ChomSensorData_t  sensorData[2]; // sensors data (current and previous power mode)
}__attribute__ ((__packed__));

typedef struct ChomLogData ChomLogData_t;

extern uint32_t g_chom_gen_periodic_log_timer;
extern ChomLogData_t * g_chom;
extern uint8_t g_chom_force;

void chom_data_init();
void chom_update_sensors();
void chom_gen_periodic_log();
void chom_data_reset();
void chom_force_gen_log();
void chom_main();

#endif //_CHOM_H
