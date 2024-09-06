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
// Max number of memory utilization CHOM sensors
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
    // Memory utilization for OCMB
    CHOMMEMUTILP0M0,
    CHOMMEMUTILP0M1,
    CHOMMEMUTILP0M2,
    CHOMMEMUTILP0M3,
    CHOMMEMUTILP0M4,
    CHOMMEMUTILP0M5,
    CHOMMEMUTILP0M6,
    CHOMMEMUTILP0M7,
    CHOMMEMUTILP0M8,
    CHOMMEMUTILP0M9,
    CHOMMEMUTILP0M10,
    CHOMMEMUTILP0M11,
    CHOMMEMUTILP0M12,
    CHOMMEMUTILP0M13,
    CHOMMEMUTILP0M14,
    CHOMMEMUTILP0M15,
    CHOMMEMUTILP1M0,
    CHOMMEMUTILP1M1,
    CHOMMEMUTILP1M2,
    CHOMMEMUTILP1M3,
    CHOMMEMUTILP1M4,
    CHOMMEMUTILP1M5,
    CHOMMEMUTILP1M6,
    CHOMMEMUTILP1M7,
    CHOMMEMUTILP1M8,
    CHOMMEMUTILP1M9,
    CHOMMEMUTILP1M10,
    CHOMMEMUTILP1M11,
    CHOMMEMUTILP1M12,
    CHOMMEMUTILP1M13,
    CHOMMEMUTILP1M14,
    CHOMMEMUTILP1M15,
    CHOMMEMUTILP2M0,
    CHOMMEMUTILP2M1,
    CHOMMEMUTILP2M2,
    CHOMMEMUTILP2M3,
    CHOMMEMUTILP2M4,
    CHOMMEMUTILP2M5,
    CHOMMEMUTILP2M6,
    CHOMMEMUTILP2M7,
    CHOMMEMUTILP2M8,
    CHOMMEMUTILP2M9,
    CHOMMEMUTILP2M10,
    CHOMMEMUTILP2M11,
    CHOMMEMUTILP2M12,
    CHOMMEMUTILP2M13,
    CHOMMEMUTILP2M14,
    CHOMMEMUTILP2M15,
    CHOMMEMUTILP3M0,
    CHOMMEMUTILP3M1,
    CHOMMEMUTILP3M2,
    CHOMMEMUTILP3M3,
    CHOMMEMUTILP3M4,
    CHOMMEMUTILP3M5,
    CHOMMEMUTILP3M6,
    CHOMMEMUTILP3M7,
    CHOMMEMUTILP3M8,
    CHOMMEMUTILP3M9,
    CHOMMEMUTILP3M10,
    CHOMMEMUTILP3M11,
    CHOMMEMUTILP3M12,
    CHOMMEMUTILP3M13,
    CHOMMEMUTILP3M14,
    CHOMMEMUTILP3M15,
    CHOMMEMUTILP4M0,
    CHOMMEMUTILP4M1,
    CHOMMEMUTILP4M2,
    CHOMMEMUTILP4M3,
    CHOMMEMUTILP4M4,
    CHOMMEMUTILP4M5,
    CHOMMEMUTILP4M6,
    CHOMMEMUTILP4M7,
    CHOMMEMUTILP4M8,
    CHOMMEMUTILP4M9,
    CHOMMEMUTILP4M10,
    CHOMMEMUTILP4M11,
    CHOMMEMUTILP4M12,
    CHOMMEMUTILP4M13,
    CHOMMEMUTILP4M14,
    CHOMMEMUTILP4M15,
    CHOMMEMUTILP5M0,
    CHOMMEMUTILP5M1,
    CHOMMEMUTILP5M2,
    CHOMMEMUTILP5M3,
    CHOMMEMUTILP5M4,
    CHOMMEMUTILP5M5,
    CHOMMEMUTILP5M6,
    CHOMMEMUTILP5M7,
    CHOMMEMUTILP5M8,
    CHOMMEMUTILP5M9,
    CHOMMEMUTILP5M10,
    CHOMMEMUTILP5M11,
    CHOMMEMUTILP5M12,
    CHOMMEMUTILP5M13,
    CHOMMEMUTILP5M14,
    CHOMMEMUTILP5M15,
    CHOMMEMUTILP6M0,
    CHOMMEMUTILP6M1,
    CHOMMEMUTILP6M2,
    CHOMMEMUTILP6M3,
    CHOMMEMUTILP6M4,
    CHOMMEMUTILP6M5,
    CHOMMEMUTILP6M6,
    CHOMMEMUTILP6M7,
    CHOMMEMUTILP6M8,
    CHOMMEMUTILP6M9,
    CHOMMEMUTILP6M10,
    CHOMMEMUTILP6M11,
    CHOMMEMUTILP6M12,
    CHOMMEMUTILP6M13,
    CHOMMEMUTILP6M14,
    CHOMMEMUTILP6M15,
    CHOMMEMUTILP7M0,
    CHOMMEMUTILP7M1,
    CHOMMEMUTILP7M2,
    CHOMMEMUTILP7M3,
    CHOMMEMUTILP7M4,
    CHOMMEMUTILP7M5,
    CHOMMEMUTILP7M6,
    CHOMMEMUTILP7M7,
    CHOMMEMUTILP7M8,
    CHOMMEMUTILP7M9,
    CHOMMEMUTILP7M10,
    CHOMMEMUTILP7M11,
    CHOMMEMUTILP7M12,
    CHOMMEMUTILP7M13,
    CHOMMEMUTILP7M14,
    CHOMMEMUTILP7M15,
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
    CHOMMMAONAVGP0,
    CHOMMMAONAVGP1,
    CHOMMMAONAVGP2,
    CHOMMMAONAVGP3,
    CHOMMMAONAVGP4,
    CHOMMMAONAVGP5,
    CHOMMMAONAVGP6,
    CHOMMMAONAVGP7,
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
