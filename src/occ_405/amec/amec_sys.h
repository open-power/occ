/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_sys.h $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2022                        */
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

#ifndef _AMEC_SYS_H
#define _AMEC_SYS_H

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <sensor.h>
#include <occ_sys_config.h>
#include <amec_part.h>
#include <amec_perfcount.h>
#include <mode.h>
#include <amec_controller.h>
#include <amec_oversub.h>
#include <amec_amester.h>
#include <amec_pcap.h>
#include <amec_smh.h>
#include <wof.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
// Number of States in the AMEC State Machine (= AMEC_SMH_STATES_PER_LVL)
#define NUM_AMEC_SMH_STATES AMEC_SMH_STATES_PER_LVL

// Number of PORE-GPE Engines
#define NUM_GPE_ENGINES     2
#define GPE_ENGINE_0        0
#define GPE_ENGINE_1        1

// Number of generic fw timing sensors
#define NUM_TIME_SENSORS    8

// Number of processor io sensors
#define NUM_PROCIO_SENSORS  4

//*************************************************************************
// Structures
//*************************************************************************

//-------------------------------------------------------------
// FW Sub-structure
//-------------------------------------------------------------
typedef struct
{
  // Sensors
  sensor_t ameintdur;
  sensor_t times[NUM_TIME_SENSORS];    // generic timing sensors for various IPC, BCE cmds...
  sensor_t gpetickdur[NUM_GPE_ENGINES];
  sensor_t prcdupdatedur;
  sensor_t voltvddsense;
  sensor_t voltvdnsense;
  sensor_t voltvcssense;
  sensor_t vsensereserve;

  // DPS update flag
  // 8 bit flag: =1, no updating allowed; =0, updating is allowed
  uint8_t  dps_no_update_flag;

} amec_fw_t;

//-------------------------------------------------------------
// Proc Sub-structure
//-------------------------------------------------------------

typedef struct
{
    uint32_t wr_cnt_accum;
    uint32_t rd_cnt_accum;
    uint16_t memwrite2ms;
    uint16_t memread2ms;
} amec_chpair_perf_counter_t;

//convenient format for storing throttle settings
typedef union
{
    uint32_t    word32;
    struct
    {
        uint32_t mba_n: 16;
        uint32_t chip_n: 16;
    };
} amec_mem_speed_t;

typedef struct
{
  amec_chpair_perf_counter_t perf;

  // The most recent throttle value sent to this MBA
  // This is used to only send values to the membuf when it changes.
  amec_mem_speed_t last_mem_speed_sent;
} amec_portpair_t;

// bit masks for fru_temp_t flags
#define FRU_SENSOR_STATUS_STALLED         0x01
#define FRU_SENSOR_STATUS_ERROR           0x02
#define FRU_SENSOR_STATUS_VALID_OLD       0x04
#define FRU_TEMP_OUT_OF_RANGE             0x08
#define FRU_SENSOR_STATUS_INVALID         0x10 //membuf only
#define FRU_TEMP_FAST_CHANGE              0x20
#define FRU_SENSOR_STATUS_REDUNDANCY_LOST 0x40

// OpenCAPI memory only bit masks for fru_temp_t dts_type_mask
#define OCM_DTS_TYPE_DIMM_MASK          0x01
#define OCM_DTS_TYPE_MEMCTRL_DRAM_MASK  0x02
#define OCM_DTS_TYPE_PMIC_MASK          0x04
#define OCM_DTS_TYPE_MEMCTRL_EXT_MASK   0x08

typedef struct
{
    uint8_t  cur_temp;
    uint8_t  sample_age;
    uint8_t  flags;
    // Sensor ID for reporting temperature to BMC and FSP
    uint32_t temp_sid;

    // Type of thermal sensor this represents used by (H)TMGT to send mem config and thermal thresholds
    eConfigDataFruType  temp_fru_type;

    // Indicates what this temperature is for used by amec_health_check_dimm_timeout()
    uint8_t  dts_type_mask;

}fru_temp_t;

typedef struct
{
  // Sub-structures under memory buffer
  union
  {
      amec_portpair_t       portpair[NUM_PORT_PAIRS_PER_MEM_BUF];
      amec_portpair_t       mba[NUM_PORT_PAIRS_PER_MEM_BUF];
  };  // Just a different name to refer to same thing

  // Current temperatures for the on board DTS sensors aka "DIMM" but what the sensor
  // really represents (i.e. DRAM, PMIC...) is defined in dimm_temps[].temp_fru_type
  fru_temp_t dimm_temps[MAX_NUM_DTS_PER_OCMB];

  // Temperature of this membuf
  sensor_t tempmembuf;

  // Hottest membuf temperature for this membuf
  fru_temp_t membuf_hottest;

  // Sensor ID for reporting temperature to BMC and FSP
  uint32_t  temp_sid;
} amec_membuf_t;

typedef struct
{
  // Sub-structures under MemCtl
  amec_membuf_t      membuf;

  // Performance Sensors
  sensor_t mrd;
  sensor_t mwr;
  sensor_t memspstat;
  sensor_t memsp;

} amec_memctl_t;

typedef struct
{
  //-----------------------------------
  // Previous Tick Data
  //-----------------------------------
  uint32_t prev_PC_RUN_Th_CYCLES;

} amec_core_thread_t;

typedef struct
{
  // Sub-structures under Core
  amec_core_perf_counter_t     core_perf;
  amec_core_thread_t thread[MAX_THREADS_PER_CORE];

  //-----------------------------------
  // Sensors
  //-----------------------------------
  sensor_t freqa;
  sensor_t ips4ms;
  sensor_t mcpifd4ms;
  sensor_t mcpifi4ms;
  sensor_t tempprocthermal;
  sensor_t util;
  sensor_t nutil3s;
  sensor_t mstl2ms;
  sensor_t tempc;
  sensor_t stopdeepreqc;
  sensor_t stopdeepactc;
  sensor_t voltdroopcntc;

  //-----------------------------------
  // Previous Tick Data
  //-----------------------------------
  uint32_t prev_PC_RAW_CYCLES;
  uint32_t prev_PC_RUN_CYCLES;
  uint32_t prev_PC_COMPLETED;
  uint32_t prev_tod_2mhz;
  uint32_t prev_FREQ_SENS_BUSY;
  uint32_t prev_FREQ_SENS_FINISH;

  //-----------------------------------
  // Calculations & Interim Data
  //-----------------------------------
  // Hottest DTS sensor per core
  uint16_t dts_hottest;
  // Counter of number of samples for calculating average utilization & frequency
  uint16_t sample_count;

  // Average utilization over a fixed time interval
  uint32_t avg_util;
  // Average frequency over a fixed time interval
  uint32_t avg_freq;

  // ---------------------------------
  // Frequency State Machine variables
  // ---------------------------------
  // Frequency request generated by the voting box
  uint16_t f_request;
  // Reason for the frequency request generated by the voting box
  uint32_t  f_reason;

} amec_core_t;

//-------------------------------------------------------------
// System-wide Sub-structure
//-------------------------------------------------------------
typedef struct
{
  // Sensors
  //sensor_t fake_sensor[NUM_OCCS];
  //
} amec_master_t;


//-------------------------------------------------------------
// System-wide Sub-structure
//-------------------------------------------------------------
typedef struct
{
  // System Sensors
  sensor_t pwrsys;
  sensor_t pwr250usgpu;
  sensor_t pwrapssch[MAX_APSS_ADC_CHANNELS];
  sensor_t cur12Vstby;
  sensor_t vrhot_mem_proc;

  sensor_t tempvdd;

  // Chip Sensors
  sensor_t todclock0;
  sensor_t todclock1;
  sensor_t todclock2;

  // Maximum Frequency that can be set in the current policy
  uint16_t fmax;

  // Maximum speed in current policy
  uint16_t max_speed;

  // Minimum speed in current policy
  uint16_t min_speed;

  // Speed step size
  uint16_t speed_step;

  // Speed step limit
  uint16_t speed_step_limit;

  // Current Memory Power Control values (applied last through GPE1)
  uint8_t current_mem_pwr_ctl;

  // ambient temperature (deg C)
  sensor_t ambient;

  // altitude in meters 0xffff indicates not available
  uint16_t altitude;

  // effective frequency when the system is at max throttled pstate
  uint16_t fmin_max_throttled;

} amec_systemwide_t;


typedef struct
{
  //Maximum frequency allowed on this chip by the power capping and
  //performance preserving boundary algorithm. Set by amec_ppb_fmax_calc
  uint16_t  ppb_fmax;

  //Maximum frequency allowed on this chip by the Pmax_clip register.
  //Set by amec_slv_check_apss_fail
  uint16_t apss_pmax_clip_freq;

  //Maximum frequency allowed on this chip by Vdd socket power capping
  //Set by amec_socket_pcap_calc()
  uint16_t socket_vdd_pcap_clip_freq;

  //Maximum frequency allowed on this chip by total socket power capping
  //Set by amec_socket_pcap_calc()
  uint16_t socket_total_pcap_clip_freq;

} amec_proc_pwr_votes_t;

//-------------------------------------------------------------
// Quad Structure
//-------------------------------------------------------------
typedef struct
{
    sensor_t tempq;
} amec_quad_t;

typedef struct
{
    sensor_t tempprocio;
} amec_procio_t;

//-------------------------------------------------------------
// GPU Structures
//-------------------------------------------------------------

typedef struct {
    bool     disabled;            // GPU has been marked failed and no longer monitored
    bool     readOnce;            // Comm has been established with GPU
    bool     commErrorLogged;     // GPU has been called out due to comm error
    bool     overtempError;       // Core OT error has been logged against GPU
    bool     memOvertempError;    // Memory OT error has been logged against GPU
    bool     checkDriverLoaded;   // Indicates if need to check if driver is loaded
    bool     driverLoaded;        // Indicates if GPU driver is loaded
    bool     checkMemTempSupport; // Indicates if need to check if mem monitoring is supported
    bool     memTempSupported;    // Indicates if memory temperature monitoring is supported
    bool     notReset;            // '1' = GPU NOT in reset.  Read from OCC FLAGS register
    bool     coreTempNotAvailable; // for fan control: '1' = core temp not available. (send 0 for fan control)
    bool     memTempNotAvailable; // for fan control: '1' = Mem temp not available. (send 0 for fan control)
    bool     coreTempFailure;     // for fan control: '1' = timeout failure reading core temp (send 0xFF for fan control)
    bool     memTempFailure;      // for fan control: '1' = timeout failure reading Mem temp (send 0xFF for fan control)
    uint8_t  memErrorCount;       // count of consecutive GPU mem temp read failures when GPU not in reset
    uint8_t  errorCount;          // count of consecutive GPU core temp read failures when GPU not in reset
    uint8_t  retryCount;          // count of consecutive GPU core temp read failures before I2C reset
} gpuStatus_t;

typedef struct {
    bool     check_pwr_limit;        // Indicates if need to read power limits from GPU
    bool     pwr_limits_read;        // Indicates if power limits were read i.e. have min/max
    bool     set_failed;             // Indicates if failed to set power limit
    uint32_t gpu_min_pcap_mw;        // Min GPU power limit in mW read from the GPU
    uint32_t gpu_max_pcap_mw;        // Max GPU power limit in mW read from the GPU
    uint32_t gpu_desired_pcap_mw;    // AMEC determined pcap in mW to set
    uint32_t gpu_requested_pcap_mw;  // Requested power cap in mW sent to GPU
    uint32_t gpu_default_pcap_mw;    // Default power cap in mW read from the GPU
} gpuPcap_t;


typedef struct
{
  //-----------------------------------
  // Sensors
  //-----------------------------------
  sensor_t tempgpu;    // GPU core temperature
  sensor_t tempgpumem; // GPU HBM temperature

  //-----------------------------------
  // Data
  //-----------------------------------
  // General Status of GPU
  gpuStatus_t status;

  // GPU Power Cap Information
  gpuPcap_t pcap;

} amec_gpu_t;

//-------------------------------------------------------------
// Proc Structure
//-------------------------------------------------------------
typedef struct
{
  // Sub-structures under Proc
  amec_core_t    core[MAX_NUM_CORES];
  amec_memctl_t  memctl[MAX_NUM_MEM_CONTROLLERS];
  amec_proc_pwr_votes_t pwr_votes;
  amec_quad_t    quad[MAXIMUM_QUADS];
  amec_procio_t io[NUM_PROCIO_SENSORS];

  // Processor Sensors
  sensor_t freqa;
  sensor_t ips4ms;
  vectorSensor_t ips4ms_vector;
  sensor_t pwrproc;
  sensor_t pwr250usmem;
  sensor_t tempprocavg;
  vectorSensor_t temp4ms_vector;
  sensor_t temprtavg;
  vectorSensor_t temprt_vector;
  sensor_t tempprocthermal;
  vectorSensor_t tempprocthermal_vector;
  sensor_t util;
  vectorSensor_t util_vector;
  sensor_t tempnest0;
  sensor_t tempnest1;
  sensor_t tempprociothermal;
  vectorSensor_t tempprociothermal_vector;
  sensor_t curvdd;
  sensor_t curvdn;
  sensor_t curvcs;
  sensor_t pstate;
  sensor_t voltvdd;
  sensor_t voltvdn;
  sensor_t voltvcs;
  sensor_t voltreserve;
  sensor_t pwrvdd;
  sensor_t pwrvdn;
  sensor_t pwrvcs;
  sensor_t procottime;
  sensor_t procpwrthrot;
  sensor_t procotthrot;
  sensor_t ddsavg;
  sensor_t ddsmin;

  // Memory Summary Sensors
  sensor_t tempmembufthrm;
  vectorSensor_t tempmembufthrm_vector;
  sensor_t tempdimmthrm;
  sensor_t tempmcdimmthrm; // hottest of all DATA_FRU_MEMCTRL_DRAM monitored by this OCC
  sensor_t temppmicthrm;   // hottest of all DATA_FRU_PMIC monitored by this OCC
  sensor_t tempmcextthrm;  // hottest of all DATA_FRU_MEMCTRL_EXT monitored by this OCC
  sensor_t memotthrot;

  // Calculations & Interim Data
  uint16_t core_min_freq;               // for debug.  Minimum requested freq for all cores on chip.
  uint8_t  current_clip_count;          // for debug.  #consecutive ticks core_min_freq is below max possible for same reason
  uint32_t chip_f_reason_history;       // for debug.  bit mask history of all frequency reason(s) for the chip
  uint32_t f_reason;                    // for debug.  current reason across all cores driving the lowest f request

  // Parameters used through Amester interface
  // Note: keep core arrays here, not in per-cores structure so one parameter
  // can be used to pass array.
  uint32_t parm_f_reason[MAX_NUM_CORES];   // per-core frequency reason
  uint16_t parm_f_override[MAX_NUM_CORES]; // per-core frequency override in MHz
  uint8_t  parm_f_override_enable;         // enable using the frequency override

} amec_proc_t;



//-------------------------------------------------------------
// Mode Freq Structure
//-------------------------------------------------------------
typedef struct amec_mode_freq
{
    uint16_t                      fmin;
    uint16_t                      fmax;
    ///Minimum speed allowed based on fmin/fmax ratio
    uint16_t                      min_speed;
} amec_mode_freq_t;


//-------------------------------------------------------------
// Parameters for manufacturing commands
//-------------------------------------------------------------
typedef struct amec_mnfg
{
    ///Auto-slewing flag: enable=1, disable=0
    uint8_t             auto_slew;
    ///Minimum frequency as a pState for auto-slewing
    uint16_t            pmin;
    ///Maximum frequency as a pState for auto-slewing
    uint16_t            pmax;
    ///Step size in Pstate(s) for auto-slewing
    uint16_t            pstep;
    ///Additional delay in ticks for auto-slewing
    uint16_t            delay;
    ///Frequency override as a Pstate to be sent to all slave OCCs
    uint16_t            poverride;
    ///Counter of times we reached fmin or fmax
    uint16_t            slew_counter;
    ///memory auto-slewing flag: enable=1, disable=0
    bool                mem_autoslew;
    ///memory slewing count
    uint32_t            mem_slew_counter;
} amec_mnfg_t;

//-------------------------------------------------------------
// Parameters for Idle Power Save (IPS) mode
//-------------------------------------------------------------
typedef struct amec_ips
{
    ///Enable/Disable IPS (=0:disable; =1:enable)
    uint8_t             enable;
    ///Current 'active' state of IPS (=0:inactive; =1:active)
    uint8_t             active;
    ///IPS frequency request to be sent to all OCC Slaves
    uint16_t            freq_request;
    ///Utilization threshold to enter idle condition (in hundreth of a percent)
    uint16_t            entry_threshold;
    ///Utilization threshold to exit idle condition (in hundreth of a percent)
    uint16_t            exit_threshold;
    ///Delay time to enter idle condition (in number of samples)
    uint32_t            entry_delay;
    ///Delay time to exit idle condition (in number of samples)
    uint32_t            exit_delay;
}amec_ips_t;


//-------------------------------------------------------------
//
// AMEC/OCC Overall System Structure -- g_amec
//
//-------------------------------------------------------------
typedef struct
{
  //---------------------------------------------------------
  //
  // System Management Settings
  //
  //---------------------------------------------------------
  // Global memory throttle reason
  uint8_t               mem_throttle_reason;
  // Global memory speed request
  uint16_t              mem_speed_request;

  // Must use Pstate (not frequency) for mfg auto slew in order to allow auto slew into throttle space
  // Flag to enable Pstate override in the voting box due to Master OCC request (mfg auto slew)
  uint8_t               poverride_enable;
  // Override Pstate to be used by the voting box due to Master OCC request  (mfg auto slew)
  uint16_t              poverride;

  // Idle Power Saver frequency request sent by Master OCC
  uint16_t              slv_ips_freq_request;

  //---------------------------------------------------------
  //
  // Physical Structure
  //
  //---------------------------------------------------------
  // Overall System Data
  amec_systemwide_t    sys;

  // Processor Card Data
  //   - This is an array of 1.  This was initialized this way
  //     in the hopes of perhaps reusing some code from previous projects.
  amec_proc_t   proc[NUM_PROC_CHIPS_PER_OCC];

  // GPU Data
  amec_gpu_t   gpu[MAX_NUM_GPU_PER_DOMAIN];

  // OCC Firmware Data
  amec_fw_t     fw;

  // Sensors on master for calculations across multiple OCCs
  //amec_master_t mstr;

  // Partition Information
  amec_part_config_t    part_config;

  //---------------------------------------------------------
  //
  // Control Systems
  //
  //---------------------------------------------------------
  // Thermal Controller based on processor temperatures
  amec_controller_t     thermalproc;
  // Thermal Controller based on membuf temperatures
  amec_controller_t     thermalmembuf;
  // Thermal Controller based on DIMM temperatures
  amec_controller_t     thermaldimm;
  // Thermal Controller based on temperature sensors covering both Memctrl+DIMM
  amec_controller_t     thermalmcdimm;
  // Thermal Controller based on PMIC temperatures
  amec_controller_t     thermalpmic;
  // Thermal Controller based on external mem controller temperatures
  amec_controller_t     thermalmcext;
  // Thermal Controller based on VRM Vdd temperatures
  amec_controller_t     thermalvdd;
  // Thermal Controller based on Processor IO temperatures
  amec_controller_t     thermalprocio;

  // OC controller when WOF is off based on dirty bits from PGPE
  oc_wof_off_t          oc_wof_off;

  // Oversubscription Status
  oversub_status_t      oversub_status;

  // Parameters for manufacturing commands
  amec_mnfg_t           mnfg_parms;

  // Parameters for Idle Power Save (IPS) mode
  amec_ips_t            mst_ips_parms;

  // PowerCap Data
  amec_pcap_t           pcap;

  // Save off proc and mem sensor data for debug usage
  uint16_t              proc_snr_pwr[MAX_NUM_CHIP_MODULES];
  uint16_t              mem_snr_pwr[MAX_NUM_CHIP_MODULES];

  // save off when pcap is considered valid
  uint8_t               pcap_valid;

  // Static WOF data -- data read in once and doesn't change
  amec_static_wof_t     static_wof_data;

  // WOF structure -- data that changes
  amec_wof_t            wof;

  // WOF sensors
  amec_wof_sensors_t    wof_sensors;

  // 32 bit counter of 250usec ticks
  uint32_t      r_cnt;
  // 32-bit ptr to streaming buffer which contains 16 bit elements
  uint16_t      *ptr_stream_buffer;
  // 32-bit index for next write into streaming buffer
  uint32_t      write_stream_index;
  // 32-bit index for next read from streaming buffer
  uint32_t      read_stream_index;
  // stream buffer for vector recordings
  uint16_t      stream_buffer[STREAM_BUFFER_SIZE];
  // initially 0 until recording is valid
  uint8_t       recordflag;
  // 16-bit delay in msec before stream vector records (set to 0 to avoid delay)
  uint16_t      stream_vector_delay;
  // 8-bit mode control for stream vector mode:
  uint8_t       stream_vector_mode;    //  0=stop recording
                                       //  1=record unconditionally from begin to end of buffer, then stop
                                       //  2=record unconditionally forever
                                       //  3=record until a checkstop event is detected
  // 8-bit mode control for stream vector recording:
  // 0=fastest sampling on platform: (250usec on OCC); 7=32msec
  uint8_t       stream_vector_rate;
  // 8-bit group # that selects which group of sensors to record as a vector
  uint8_t       stream_vector_group;
  // input from TMGT to signal a reset of the OCC is desired (!=0)
  uint8_t       reset_prep;
  // holds the sum of all the memory power sensors (32msec)
  uint16_t      total_memory_power;

} amec_sys_t;

//*************************************************************************
// Globals
//*************************************************************************
extern amec_sys_t * g_amec;

//*************************************************************************
// Function Prototypes
//*************************************************************************
void amec_slave_init(void) INIT_SECTION;

#endif
