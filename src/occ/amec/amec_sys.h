/******************************************************************************
// @file amec_sys.h
// @brief OCC AMEC System Structure
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section amec_sys.h AMEC_SYS.H
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th005             thallet   11/23/2011  New File  
 *   @th00a             thallet   02/03/2012  Worst case FW timings in AMEC Sensors
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @th00b             thallet   02/28/2012  Added Vector Sensors
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @gs001             gsilva    08/03/2012  Added frequency points & freq SMH 
 *   @th018  852950     thallet   09/12/2012  Added Centaur thermal readings       
 *   @ly001  853751     lychen    09/17/2012  Support DPS algorithm
 *   @ry002  862116     ronda     11/26/2012  Support thermal controller for processor
 *   @th026  865074     thallet   12/21/2012  Updated Centaur sensors
 *   @at010  859992     alvinwan  11/07/2012  Added oversubscription feature
 *   @ry003  870734     ronda     02/20/2013  Thermal controller for memory
 *   @ry004  872358     ronda     02/27/2013  Added global memory speed request and throttle reason
 *   @th031  878471     thallet   04/15/2013  Centaur Throttles
 *   @gs006  884384     gjsilva   05/30/2013  Support for mnfg auto-slewing function
 *   @fk001  879727     fmkassem  04/16/2013  PCAP support.
 *   @gm004  892961     milesg    07/25/2013  Support memory auto slewing
 *   @gs008  894661     gjsilva   08/08/2013  Initial support for DPS-FP mode
 *   @rt001  897459     tapiar    08/09/2013  Update reason codes to be uint32_t
 *   @gm008  SW226989   milesg    09/30/2013  Sapphire initial support
 *   @gs014  903552     gjsilva   10/22/2013  Support for Amester parameter interface
 *   @gs015  905166     gjsilva   11/04/2013  Full support for IPS function
 *   @rt003  905677     tapiar    11/07/2013  save of proc/mem sensor data into g_amec
 *   @gs017  905990     gjsilva   11/13/2013  Full support for tunable parameters
 *   @fk002  905632     fmkassem  11/05/2013  Remove CriticalPathMonitor code
 *   @cl007  907196     lefurgy   11/15/2013  Fix freqa and ips sensors
 *   @gm013  907548     milesg    11/22/2013  Memory therm monitoring support
 *   @rt004  908817     tapiar    12/11/2013  Add valid pcap field to g_amec to be used
 *                                            by slaves when their pcap is valid
 *   @gs020  909320     gjsilva   12/12/2013  Support for VR_FAN thermal control
 *   @gm016  909061     milesg    12/10/2013  Support memory throttling due to temperature
 *   @gs023  912003     gjsilva   01/16/2014  Generate VRHOT signal and control loop
 *   @gs026  915840     gjsilva   02/13/2014  Support for Nvidia GPU power measurement
 *   @mw644  918066     mware     03/07/2014  Added data to enable freq_sens_busy and freq_sens_finish.
 *   @gs029  919478     gjsilva   03/26/2014  Fixed wrong values for sleep/winkle counters
 *   @gm039  922963     milesg    05/28/2014  Handle centaur nest LFIR 6
 *   @gs043  943177     gjsilva   10/30/2014  Support for mem data packets in BMC-based systems
 *
 *  @endverbatim
 *
 *///*************************************************************************/

#ifndef _AMEC_SYS_H
#define _AMEC_SYS_H

//*************************************************************************
// Includes
//*************************************************************************
//@pb00Ec - changed from common.h to occ_common.h for ODE support
#include <occ_common.h>
#include <sensor.h>
#include <occ_sys_config.h>
#include <amec_part.h> // @ly001a
#include <amec_perfcount.h> // @ly001a
#include <mode.h> // @ly001a
#include <amec_controller.h> 
#include <amec_oversub.h> // @at010a
#include <amec_amester.h> // @mw626
#include <amec_pcap.h> // @fk001

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
// This is an arbitrary number of FW probes for use internally.
#define NUM_AMEC_FW_PROBES  8

// Number of States in the AMEC State Machine (should always be 8)
#define NUM_AMEC_SMH_STATES 8

// Number of PORE-GPE Engines
#define NUM_GPE_ENGINES     2
#define GPE_ENGINE_0        0
#define GPE_ENGINE_1        1

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
  sensor_t amessdur[NUM_AMEC_SMH_STATES];
  sensor_t gpetickdur[NUM_GPE_ENGINES];     // @th00a
  sensor_t prcdupdatedur;
  sensor_t probe250us[NUM_AMEC_FW_PROBES];

  // DPS update flag
  uint8_t  dps_no_update_flag;    // 8 bit flag: =1, no updating allowed; =0, updating is allowed // @ly001a
  
} amec_fw_t;


//-------------------------------------------------------------
// Fan Sub-structure
//-------------------------------------------------------------
typedef struct
{
  // Sensors
  sensor_t fanspeedavg;
  sensor_t pwr250usfan;

} amec_fans_t;

//-------------------------------------------------------------
// IO Sub-structure
//-------------------------------------------------------------
typedef struct
{
  // Sensors
  sensor_t pwr250usio;

} amec_io_t;

//-------------------------------------------------------------
// Storage Sub-structure
//-------------------------------------------------------------
typedef struct
{
  // Sensors
  sensor_t pwr250usstore;

} amec_store_t;

//-------------------------------------------------------------
// Proc Sub-structure
//-------------------------------------------------------------

typedef struct
{
  // Sensors
  sensor_t uvolt250us;
  sensor_t volt250us;

} amec_vrm_t;


typedef struct 
{
    uint32_t wr_cnt_accum;
    uint32_t rd_cnt_accum;
    uint32_t pwrup_cnt_accum;
    uint32_t act_cnt_accum;
    uint32_t fr2_cnt_accum;
    uint32_t l4_rd_cnt_accum;
    uint32_t l4_wr_cnt_accum;
    uint32_t intreq_base_accum;
    uint32_t intreq_low_accum;
    uint32_t intreq_med_accum;
    uint32_t intreq_high_accum;

    uint16_t fr2_cnt;
    uint16_t act_cnt;
    uint16_t pwrup_cnt;
    uint16_t memwrite2ms;
    uint16_t memread2ms;
    uint16_t l4wr2ms;
    uint16_t l4rd2ms;
    uint16_t mirb2ms;
    uint16_t mirl2ms;
    uint16_t mirm2ms;
    uint16_t mirh2ms;
    
} amec_chpair_perf_counter_t;

//convenient format for storing throttle settings - gm016
typedef union
{
    uint32_t    word32;
    struct
    {
        uint32_t mba_n: 16;
        uint32_t chip_n: 16;
    };
} amec_cent_mem_speed_t;

typedef struct
{ 
  // Sensors
  sensor_t mac2ms;
  sensor_t mpu2ms;
  sensor_t mirb2ms;
  sensor_t mirl2ms;
  sensor_t mirm2ms;
  sensor_t mirh2ms;
  sensor_t mts2ms;
  sensor_t memsp2ms;
  sensor_t m4rd2ms;
  sensor_t m4wr2ms;

  amec_chpair_perf_counter_t perf;

  // The most recent throttle value sent to this MBA
  // This is used to only send values to the centaur when it changes.
  amec_cent_mem_speed_t last_mem_speed_sent;
} amec_portpair_t;

typedef struct
{
  uint32_t intreq_highlatency_accum;
  uint32_t lp2exit_accum;

  uint16_t mirc2ms;
  uint16_t mlp2_2ms;
} amec_centaur_perf_counter_t;

#define FRU_SENSOR_STATUS_STALLED       0x01
#define FRU_SENSOR_STATUS_ERROR         0x02
#define FRU_SENSOR_STATUS_VALID_OLD     0x04
#define FRU_TEMP_OUT_OF_RANGE           0x08
#define FRU_SENSOR_STATUS_INVALID       0x10 //centaur only
#define FRU_TEMP_FAST_CHANGE            0x20
#define FRU_SENSOR_CENT_NEST_FIR6       0x40 //centaur only -- gm039

typedef struct
{
    uint8_t  cur_temp;
    uint8_t  sample_age;
    uint8_t  flags;
    // Sensor ID for reporting temperature to BMC
    uint16_t temp_sid;
}fru_temp_t;

typedef struct
{ 
  // Sub-structures under Centaur
  union
  {
      amec_portpair_t       portpair[NUM_PORT_PAIRS_PER_CENTAUR];
      amec_portpair_t       mba[NUM_PORT_PAIRS_PER_CENTAUR];      
  };  // @th031 - Just a different name to refer to same thing

  // Sensors
  sensor_t mlp2ms;
  sensor_t mirc2ms;

  //hottest dimm temperature behind this centaur
  sensor_t tempdimmax;

  //which of the 8 dimm temperatures was the hottest temperature
  //(only changes when the max of tempdimmax changes)
  sensor_t locdimmax;

  // Current dimm tempuratures
  fru_temp_t dimm_temps[NUM_DIMMS_PER_CENTAUR];

  // Hottest centaur temperature for this centaur
  fru_temp_t centaur_hottest;

  // Sensor ID for reporting temperature to BMC
  uint16_t   temp_sid;

  amec_centaur_perf_counter_t perf;

} amec_centaur_t;

typedef struct
{
  // Sub-structures under MemCtl
  amec_centaur_t      centaur;

  // Sensors
  sensor_t mrd2ms;
  sensor_t mwr2ms;

} amec_memctl_t;

typedef struct
{
  //-----------------------------------
  // Previous Tick Data
  //-----------------------------------
  uint32_t prev_PC_RUN_Th_CYCLES;

  //-----------------------------------
  // Calculations & Interim Data
  //-----------------------------------
  uint16_t util2ms_thread;

} amec_core_thread_t;

typedef struct
{
  // Sub-structures under Core  
  amec_core_perf_counter_t     core_perf; // @ly001c
  amec_core_thread_t thread[MAX_THREADS_PER_CORE];

  //-----------------------------------
  // Sensors
  //-----------------------------------
//  sensor_t cpm2ms; //CPM - Commented out as requested by Malcolm
  sensor_t freq250us;
  sensor_t freqa2ms;       // @mw626
  sensor_t ips2ms;
  sensor_t mcpifd2ms;
  sensor_t mcpifi2ms;
  sensor_t spurr2ms;
  sensor_t temp2ms;
  sensor_t util2ms;
  sensor_t nutil3s;
  sensor_t mstl2ms;
  sensor_t cmt2ms;
  sensor_t cmbw2ms;
  sensor_t ppic;
  sensor_t pwrpx250us;

  //-----------------------------------
  // Previous Tick Data
  //-----------------------------------
  uint32_t prev_PC_RAW_CYCLES;
  uint32_t prev_PC_RUN_CYCLES;
  uint32_t prev_PC_DISPATCH;
  uint32_t prev_PC_COMPLETED;
  uint32_t prev_PC_RAW_Th_CYCLES;
  uint32_t prev_tod_2mhz;
  uint32_t prev_lpar_mem_cnt[4];
  uint32_t prev_FREQ_SENS_BUSY;
  uint32_t prev_FREQ_SENS_FINISH;

  //-----------------------------------
  // Calculations & Interim Data
  //-----------------------------------
  // Dispatched Instructions per Second
  uint16_t dps;
  // Dispatched Instruction per Cycle
  uint16_t dpc;
  // Instructions per Cycle
  uint16_t ipc;  
  // Hottest DTS sensor per core
  uint16_t dts_hottest;
  // Counter of number of samples for calculating average utilization & frequency
  uint16_t sample_count;
  // Array of memory bandwidth for each LPAR
  uint16_t membw[4];

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
  uint32_t  f_reason;   // @rt001c
  // Current state of this core frequency state machine
  uint8_t  f_sms;

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
  sensor_t tempambient;
  sensor_t altitude;
  sensor_t pwr250us;
  sensor_t pwr250usgpu;

  sensor_t vrfan250usmem;
  sensor_t vrhot250usmem;
  sensor_t vrfan250usproc;
  sensor_t vrhot250usproc;

  // Chip Sensors
  sensor_t todclock0; 
  sensor_t todclock1;
  sensor_t todclock2;

  // Minimum Frequency that can be set in the current policy
  uint16_t fmin;

  // Maximum Frequency that can be set in the current policy
  uint16_t fmax;

  // Maximum speed in current policy
  uint16_t max_speed; // @ly001a
  
  // Minimum speed in current policy
  uint16_t min_speed;
  
  // Speed step size
  uint16_t speed_step;
  
  // Speed step limit
  uint16_t speed_step_limit;
} amec_systemwide_t;

//@fk001a

//Structure Defined in amec_slv_voting_box.odg
typedef struct
{
  //Maximum frequency allowed on this chip by the 
  //performance preserving boundary algorithm. Set by amec_ppb_fmax_calc
  uint16_t  ppb_fmax;

  //Maximum frequency allowed on this chip by the Pmax_clip register.
  //Set by amec_pmax_clip_controller.
  uint16_t  pmax_clip_freq;

  //Maximum frequency allowed on this chip by the power capping algorithm for 
  //non-nominal cores. Set by amec_pcap_controller.
  uint16_t  proc_pcap_vote;

  //Minimum frequency that power capping is allowed to lower a nominal
  //core to.  Set by amec_pcap_calc.
  uint16_t  nom_pcap_fmin;

  //Maximum frequency allowed on this chip by the power capping algorithm for 
  //nominal cores.  Set by amec_pcpa_controller.
  uint16_t  proc_pcap_nom_vote;

  //Maximum frequency allowed on this chip by the connector overcurrent algorithm.
  //Set by amec_conn_oc_controller.
  uint16_t conn_oc_vote;
} amec_proc_pwr_votes_t;

//-------------------------------------------------------------
// Proc Structure
//-------------------------------------------------------------
typedef struct
{
  // Sub-structures under Proc
  amec_core_t    core[MAX_NUM_CORES];
  amec_memctl_t  memctl[MAX_NUM_MEM_CONTROLLERS];
  amec_vrm_t     vrm[NUM_PROC_VRMS];
  amec_proc_pwr_votes_t pwr_votes;     // @fk001a
  //amec_vrm_t          memvrm[];
  
  // Processor Sensors
  sensor_t freqa2ms;                   // @mw626
  vectorSensor_t freqa2ms_vector;      // @th00b  @mw626
  sensor_t ips2ms;
  vectorSensor_t ips2ms_vector;        // @th00b
  sensor_t memsp2ms;
  vectorSensor_t memsp2ms_vector;      // @th00b
  sensor_t pwr250us;
  sensor_t pwr250usvdd;
  sensor_t cur250usvdd;
  sensor_t pwr250usvcs;
  sensor_t pwr250usmem;   
  sensor_t sleepcnt2ms;
  sensor_t winkcnt2ms;
  sensor_t sp250us;
  sensor_t temp2ms;
  vectorSensor_t temp2ms_vector;       // @th00b
  sensor_t temp2mspeak;
  vectorSensor_t temp2mspeak_vector;    // @th00b
  sensor_t util2ms;
  vectorSensor_t util2ms_vector;        // @th00b
  
  // Memory Summary Sensors
  sensor_t temp2mscent;
  sensor_t temp2msdimm;
  sensor_t memsp2ms_tls;

  // Error count for failing to read VR_FAN signal
  uint8_t  vrfan_error_count;

  // Calculations & Interim Data
  uint16_t sleep_cnt;
  uint16_t winkle_cnt;

  uint16_t core_max_freq;               // Maximum requested freq for all cores on chip. @fk001a  

  // Parameters used through Amester interface
  // Note: keep core arrays here, not in per-cores structure so one parameter
  // can be used to pass array.
  uint32_t parm_f_reason[MAX_NUM_CORES];   // per-core frequency reason
  uint16_t parm_f_override[MAX_NUM_CORES]; // per-core frequency override in MHz
  uint8_t  parm_f_override_enable;         // enable using the frequency override

} amec_proc_t;



//-------------------------------------------------------------
// Mode Freq Structure // @ly001a
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
    ///Minimum frequency in MHz for auto-slewing
    uint16_t            fmin;
    ///Maximum frequency in MHz for auto-slewing
    uint16_t            fmax;
    ///Step size in MHz for auto-slewing
    uint16_t            fstep;
    ///Additional delay in ticks for auto-slewing
    uint16_t            delay;
    ///Frequency override to be sent to all slave OCCs
    uint16_t            foverride;
    ///Counter of times we reached fmin or fmax
    uint16_t            slew_counter;
    ///memory auto-slewing flag: enable=1, disable=0
    bool                mem_autoslew; //gm004
    ///memory slewing count
    uint32_t            mem_slew_counter; //gm004
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
//  Description, etc...
//
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

  // Flag to enable frequency override in the voting box
  uint8_t               foverride_enable;
  // Override frequency to be used by the voting box
  uint16_t              foverride;
  // Idle Power Saver frequency request sent by Master OCC
  uint16_t              slv_ips_freq_request;
  // Flag to indicate that the DPS parameters were overwritten by user
  BOOLEAN               slv_dps_param_overwrite;

  //---------------------------------------------------------
  //
  // Physical Structure
  //
  //---------------------------------------------------------
  // Bulk Power Data
  //amec_bulk_t   bulk;

  // IO Data
  amec_io_t     io;

  // Storage Data
  amec_store_t  storage;

  // Fan Data
  amec_fans_t   fan;

  // Perf Data
  //amec_perf_t   perf;

  // Overall System Data
  amec_systemwide_t    sys;

  // Processor Card Data
  //   - This is an array of 1.  This was initialized this way
  //     in the hopes of perhaps reusing some code from TPMD
  //     where there were multiple proc chips per TPMD.
  amec_proc_t   proc[NUM_PROC_CHIPS_PER_OCC];

  // OCC Firmware Data
  amec_fw_t     fw;

  // AMEC Misc (Non-System-Heirachy) Data
  //amec_amecfw_t fw_amec;

  // Sensors on master for calculations across multiple OCCs
  //amec_master_t mstr;

  // Partition Information
  amec_part_config_t    part_config; // @ly001a
  // Mode frequency table indexed by mode
  amec_mode_freq_t      part_mode_freq[OCC_INTERNAL_MODE_MAX_NUM]; // @ly001a
  
  //---------------------------------------------------------
  //
  // Control Systems  
  //
  //---------------------------------------------------------  
  // Thermal Controller based on processor temperatures
  amec_controller_t     thermalproc;  
  // Thermal Controller based on Centaur temperatures
  amec_controller_t     thermalcent;
  // Thermal Controller based on DIMM temperatures
  amec_controller_t     thermaldimm;
  // Thermal Controller based on VRHOT signal from processor VRM
  amec_controller_t     vrhotproc;

  // Oversubscription Status
  oversub_status_t      oversub_status;  // @at010a

  // Parameters for manufacturing commands
  amec_mnfg_t           mnfg_parms;

  // Parameters for Idle Power Save (IPS) mode
  amec_ips_t            mst_ips_parms;
   
  // PowerCap Data
  amec_pcap_t           pcap;   //fk001a

  // Save off proc and mem sensor data for debug usage @rt003a 
  uint16_t              proc_snr_pwr[MAX_NUM_CHIP_MODULES];
  uint16_t              mem_snr_pwr[MAX_NUM_CHIP_MODULES];

  // save off when pcap is considered valid @rt004a 
  uint8_t               pcap_valid;

  //---------------------------------------------------------
  //
  // Parameters for analytics function
  //
  //---------------------------------------------------------
  uint32_t      r_cnt;                    // 32 bit counter of 250usec ticks  @mw622
  void          *stream_vector_map[STREAM_VECTOR_SIZE_EX];  // array holding sensor ptrs for writing to stream vector
  uint16_t      *ptr_stream_buffer;                         // 32 bit ptr to streaming buffer which contains 16 bit elements
  uint32_t      write_stream_index;                         // 32 bit index for next write into streaming buffer
  uint32_t      read_stream_index;                          // 32 bit index for next read from streaming buffer
  uint16_t      stream_buffer[STREAM_BUFFER_SIZE];          // stream buffer for vector recordings
  uint8_t       recordflag;                                 // initially 0 until recording is valid
  uint16_t      stream_vector_delay;   // 16 bit delay in msec before stream vector records (set to 0 to avoid delay)
  uint8_t       stream_vector_mode;    // 8 bit mode control for stream vector mode:
                                       //  0=stop recording
                                       //  1=record unconditionally from begin to end of buffer, then stop
                                       //  2=record unconditionally forever
                                       //  3=record until a checkstop event is detected
  uint8_t       stream_vector_rate;     // 8 bit mode control for stream vector recording: 0=fastest sampling on platform: (250usec on OCC), (250us*2^7=32msec on TPMD); 7=32msec
  uint8_t       stream_vector_group;    // 8 bit group # that selects which group of sensors to record as a vector
  uint8_t       reset_prep;                         // input from TMGT to signal a reset of the OCC is desired (!=0)
  uint16_t      cent_l4_state[MAX_NUM_CENTAURS];    // holds current state of L4 state machine for Centaur k
  uint16_t      cent_l4_ipl_state[MAX_NUM_CENTAURS];// holds current state of L4 IPL state machine for Centaur k
  uint8_t       l4_powerdown_requestm;              // input from OCC master to signal a desire to power down the L4s (!=0)
  uint16_t      probe_l4_centaur;                   // indicates which of the L4 Centaurs is being monitored by probe.

  uint32_t      g44_avg[MAX_NUM_CHIP_MODULES*MAX_SENSORS_ANALYTICS];   // $mw417  @mw641
  uint16_t      analytics_group;    // parameter driven selection of analytics group  $mw412
  uint8_t       analytics_chip;     // parameter to select which chip to monitor analytics on  $mw417
  uint8_t       analytics_option;   // parameter to select which analytics options (=0 just selected chip) $mw417
  uint8_t       analytics_bad_output_count;   // 8 bit value used to throw away frames until good output has been averaged in amec_analytics buffer outputs @mw587
  uint8_t       analytics_total_chips;    // Total number of chips used in analytics sensor capture
  uint8_t       analytics_thermal_offset; // Current offset in cyclic thermal group output (8 in cycle)
  uint8_t       analytics_threadmode;     // Selects which type of Group 44 averaging is done on per thread data: default=0 (average of non-zero thread utilizations), =1 (average of N), =2 (max of N)  $mw457
  uint8_t       analytics_threadcountmax; // Has the maximum number of threads per core for this processor architecture or for SMT modes. Default=4 on P7+.
  uint8_t       analytics_slot;           // Which of 8 time slots that amec_analytics is called in
  uint16_t      analytics_array[48];      // Used to hold selected analytics group         $mw412
  uint16_t      packednapsleep[MAX_NUM_CHIP_MODULES];  // for group 44 support core bit maps of their napping cores (upper byte) and sleeping cores (lower byte) $mw374
  uint16_t      total_memory_power; // holds the sum of all the memory power sensors (32msec) $mw371

  //---------------------------------------------------------
  //
  // Global Sensors
  //
  //---------------------------------------------------------

} amec_sys_t;

//*************************************************************************
// Globals
//*************************************************************************
extern amec_sys_t * g_amec;

//*************************************************************************
// Function Prototypes
//*************************************************************************
void amec_slave_init(void) INIT_SECTION;     // @th00a

//*************************************************************************
// Functions
//*************************************************************************

#endif
