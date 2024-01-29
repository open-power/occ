/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/thread/chom.h $                                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2024                        */
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
#define  CHOM_VERSION                   0x10
// Max size of chom data this should be less than MAX_ERRL_CALL_HOME_SZ
// to account for error log header/possible other data in the call home log
#define  CHOM_LOG_DATA_MAX              (MAX_ERRL_CALL_HOME_SZ - 256)
// Max number of procs Call Home will get data for
#define CHOM_MAX_OCCS                   8
// Max number of memory bandwidth CHOM sensors
#define  MAX_NUM_MEMORY_SENSORS         (CHOM_MAX_OCCS * MAX_NUM_MEM_CONTROLLERS)
// Max number of error history entries to add to call home log
#define CHOM_MAX_ERRH_ENTRIES           4
// List of call home sensors
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
    CHOMFREQP4,
    CHOMFREQP5,
    CHOMFREQP6,
    CHOMFREQP7,
    // Processor utilization sensor
    CHOMUTILP0,
    CHOMUTILP1,
    CHOMUTILP2,
    CHOMUTILP3,
    CHOMUTILP4,
    CHOMUTILP5,
    CHOMUTILP6,
    CHOMUTILP7,
    // Proc temperatures across all nodes
    CHOMTEMPPROC0,
    CHOMTEMPPROC1,
    CHOMTEMPPROC2,
    CHOMTEMPPROC3,
    CHOMTEMPPROC4,
    CHOMTEMPPROC5,
    CHOMTEMPPROC6,
    CHOMTEMPPROC7,
    // Proc io temperature accross all nodes
    CHOMTEMPIOP0,
    CHOMTEMPIOP1,
    CHOMTEMPIOP2,
    CHOMTEMPIOP3,
    CHOMTEMPIOP4,
    CHOMTEMPIOP5,
    CHOMTEMPIOP6,
    CHOMTEMPIOP7,
    // Membuf temperature for all membufs in the node
    CHOMTEMPMEMBUFP0,
    CHOMTEMPMEMBUFP1,
    CHOMTEMPMEMBUFP2,
    CHOMTEMPMEMBUFP3,
    CHOMTEMPMEMBUFP4,
    CHOMTEMPMEMBUFP5,
    CHOMTEMPMEMBUFP6,
    CHOMTEMPMEMBUFP7,
    // Dimm temperature for all Dimms in the node
    CHOMTEMPDIMMP0,
    CHOMTEMPDIMMP1,
    CHOMTEMPDIMMP2,
    CHOMTEMPDIMMP3,
    CHOMTEMPDIMMP4,
    CHOMTEMPDIMMP5,
    CHOMTEMPDIMMP6,
    CHOMTEMPDIMMP7,
    // temperature covering mem controller and DRAM for all Dimms in the node
    CHOMTEMPMCDIMMP0,
    CHOMTEMPMCDIMMP1,
    CHOMTEMPMCDIMMP2,
    CHOMTEMPMCDIMMP3,
    CHOMTEMPMCDIMMP4,
    CHOMTEMPMCDIMMP5,
    CHOMTEMPMCDIMMP6,
    CHOMTEMPMCDIMMP7,
    // PMIC temperature read from OCMB cache line for all Dimms in the node
    CHOMTEMPPMICP0,
    CHOMTEMPPMICP1,
    CHOMTEMPPMICP2,
    CHOMTEMPPMICP3,
    CHOMTEMPPMICP4,
    CHOMTEMPPMICP5,
    CHOMTEMPPMICP6,
    CHOMTEMPPMICP7,
    // External memory buffer temperature for all memory controllers in the node
    CHOMTEMPMCEXTP0,
    CHOMTEMPMCEXTP1,
    CHOMTEMPMCEXTP2,
    CHOMTEMPMCEXTP3,
    CHOMTEMPMCEXTP4,
    CHOMTEMPMCEXTP5,
    CHOMTEMPMCEXTP6,
    CHOMTEMPMCEXTP7,
    // VRM VDD temperature per proc
    CHOMTEMPVDDP0,
    CHOMTEMPVDDP1,
    CHOMTEMPVDDP2,
    CHOMTEMPVDDP3,
    CHOMTEMPVDDP4,
    CHOMTEMPVDDP5,
    CHOMTEMPVDDP6,
    CHOMTEMPVDDP7,
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
    CHOMBWP0M8,
    CHOMBWP0M9,
    CHOMBWP0M10,
    CHOMBWP0M11,
    CHOMBWP0M12,
    CHOMBWP0M13,
    CHOMBWP0M14,
    CHOMBWP0M15,
    CHOMBWP1M0,
    CHOMBWP1M1,
    CHOMBWP1M2,
    CHOMBWP1M3,
    CHOMBWP1M4,
    CHOMBWP1M5,
    CHOMBWP1M6,
    CHOMBWP1M7,
    CHOMBWP1M8,
    CHOMBWP1M9,
    CHOMBWP1M10,
    CHOMBWP1M11,
    CHOMBWP1M12,
    CHOMBWP1M13,
    CHOMBWP1M14,
    CHOMBWP1M15,
    CHOMBWP2M0,
    CHOMBWP2M1,
    CHOMBWP2M2,
    CHOMBWP2M3,
    CHOMBWP2M4,
    CHOMBWP2M5,
    CHOMBWP2M6,
    CHOMBWP2M7,
    CHOMBWP2M8,
    CHOMBWP2M9,
    CHOMBWP2M10,
    CHOMBWP2M11,
    CHOMBWP2M12,
    CHOMBWP2M13,
    CHOMBWP2M14,
    CHOMBWP2M15,
    CHOMBWP3M0,
    CHOMBWP3M1,
    CHOMBWP3M2,
    CHOMBWP3M3,
    CHOMBWP3M4,
    CHOMBWP3M5,
    CHOMBWP3M6,
    CHOMBWP3M7,
    CHOMBWP3M8,
    CHOMBWP3M9,
    CHOMBWP3M10,
    CHOMBWP3M11,
    CHOMBWP3M12,
    CHOMBWP3M13,
    CHOMBWP3M14,
    CHOMBWP3M15,
    CHOMBWP4M0,
    CHOMBWP4M1,
    CHOMBWP4M2,
    CHOMBWP4M3,
    CHOMBWP4M4,
    CHOMBWP4M5,
    CHOMBWP4M6,
    CHOMBWP4M7,
    CHOMBWP4M8,
    CHOMBWP4M9,
    CHOMBWP4M10,
    CHOMBWP4M11,
    CHOMBWP4M12,
    CHOMBWP4M13,
    CHOMBWP4M14,
    CHOMBWP4M15,
    CHOMBWP5M0,
    CHOMBWP5M1,
    CHOMBWP5M2,
    CHOMBWP5M3,
    CHOMBWP5M4,
    CHOMBWP5M5,
    CHOMBWP5M6,
    CHOMBWP5M7,
    CHOMBWP5M8,
    CHOMBWP5M9,
    CHOMBWP5M10,
    CHOMBWP5M11,
    CHOMBWP5M12,
    CHOMBWP5M13,
    CHOMBWP5M14,
    CHOMBWP5M15,
    CHOMBWP6M0,
    CHOMBWP6M1,
    CHOMBWP6M2,
    CHOMBWP6M3,
    CHOMBWP6M4,
    CHOMBWP6M5,
    CHOMBWP6M6,
    CHOMBWP6M7,
    CHOMBWP6M8,
    CHOMBWP6M9,
    CHOMBWP6M10,
    CHOMBWP6M11,
    CHOMBWP6M12,
    CHOMBWP6M13,
    CHOMBWP6M14,
    CHOMBWP6M15,
    CHOMBWP7M0,
    CHOMBWP7M1,
    CHOMBWP7M2,
    CHOMBWP7M3,
    CHOMBWP7M4,
    CHOMBWP7M5,
    CHOMBWP7M6,
    CHOMBWP7M7,
    CHOMBWP7M8,
    CHOMBWP7M9,
    CHOMBWP7M10,
    CHOMBWP7M11,
    CHOMBWP7M12,
    CHOMBWP7M13,
    CHOMBWP7M14,
    CHOMBWP7M15,
    CHOMDDSAVGP0,
    CHOMDDSAVGP1,
    CHOMDDSAVGP2,
    CHOMDDSAVGP3,
    CHOMDDSAVGP4,
    CHOMDDSAVGP5,
    CHOMDDSAVGP6,
    CHOMDDSAVGP7,
    CHOMDDSMINP0,
    CHOMDDSMINP1,
    CHOMDDSMINP2,
    CHOMDDSMINP3,
    CHOMDDSMINP4,
    CHOMDDSMINP5,
    CHOMDDSMINP6,
    CHOMDDSMINP7,
    CHOMCURVDDP0,
    CHOMCURVDDP1,
    CHOMCURVDDP2,
    CHOMCURVDDP3,
    CHOMCURVDDP4,
    CHOMCURVDDP5,
    CHOMCURVDDP6,
    CHOMCURVDDP7,
    CHOMCEFFRATIOVDDP0,
    CHOMCEFFRATIOVDDP1,
    CHOMCEFFRATIOVDDP2,
    CHOMCEFFRATIOVDDP3,
    CHOMCEFFRATIOVDDP4,
    CHOMCEFFRATIOVDDP5,
    CHOMCEFFRATIOVDDP6,
    CHOMCEFFRATIOVDDP7,
    CHOMUVAVGP0,
    CHOMUVAVGP1,
    CHOMUVAVGP2,
    CHOMUVAVGP3,
    CHOMUVAVGP4,
    CHOMUVAVGP5,
    CHOMUVAVGP6,
    CHOMUVAVGP7,
    CHOMOVAVGP0,
    CHOMOVAVGP1,
    CHOMOVAVGP2,
    CHOMOVAVGP3,
    CHOMOVAVGP4,
    CHOMOVAVGP5,
    CHOMOVAVGP6,
    CHOMOVAVGP7,
    CHOMPWRVDDP0,
    CHOMPWRVDDP1,
    CHOMPWRVDDP2,
    CHOMPWRVDDP3,
    CHOMPWRVDDP4,
    CHOMPWRVDDP5,
    CHOMPWRVDDP6,
    CHOMPWRVDDP7,
    CHOMPWRVCSP0,
    CHOMPWRVCSP1,
    CHOMPWRVCSP2,
    CHOMPWRVCSP3,
    CHOMPWRVCSP4,
    CHOMPWRVCSP5,
    CHOMPWRVCSP6,
    CHOMPWRVCSP7,
    // The number of chom sensors reported
    CHOM_NUM_OF_SENSORS
};

enum chom_supported_modes
{
    CHOM_MODE_DISABLED,
    CHOM_MODE_SPS,   // Static Power Save
    CHOM_MODE_SFP,   // Static Frequency Point
    CHOM_MODE_FFO,
    CHOM_MODE_BALANCED,
    CHOM_MODE_MAX_PERF,
    CHOM_MODE_FMAX,
    CHOM_MODE_EFF_PWR,
    CHOM_MODE_EFF_PERF,
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
    uint32_t    accumulator;      // accumulator register to compute the average
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
    // error history counts and freq clip history for all slaves, skip master already in error log
    error_history_count_t errhCounts[CHOM_MAX_OCCS-1][CHOM_MAX_ERRH_ENTRIES];
    uint32_t       fClipHist[CHOM_MAX_OCCS-1];
    uint8_t        ddsMinCore[CHOM_MAX_OCCS];       // core number that had the minimum for DDS min sensor
    uint32_t       ocsDirtyTypeAct[CHOM_MAX_OCCS];  // count of OCS dirty with type 1 (act) was set
    uint32_t       ocsDirtyTypeHold[CHOM_MAX_OCCS]; // count of OCS dirty with type 0 (hold) was set
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
