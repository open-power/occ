/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/wof/wof.h $                                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2019                        */
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
#ifndef _WOF_H
#define _WOF_H

//******************************************************************************
// Define
//******************************************************************************
#define ACTIVE_QUAD_SZ_MIN          1
#define ACTIVE_QUAD_SZ_MAX          6
#define MIN_BCE_REQ_SIZE            256
#define WOF_HEADER_SIZE             128
#define CORE_IDDQ_MEASUREMENTS      6
#define QUAD_POWERED_OFF            0xFF
#define PGPE_WOF_OFF                0
#define PGPE_WOF_ON                 1
#define NUM_CORES_PER_QUAD          4

// default number of chances for WOF running every 8th tick, actual timeout
// used will be adjusted if WOF runs every tick
#define MAX_VRT_CHANCES_EVERY_8TH_TICK  2  // 8ms = 4ms * 2
#define MAX_VRT_CHANCES_EVERY_TICK  16  // 8ms = 500us * 16
#define MAX_VRT_CHANCES G_max_vrt_chances
#define MAX_WOF_CONTROL_CHANCES_EVERY_8TH_TICK  2  // 8ms = 4ms * 2
#define MAX_WOF_CONTROL_CHANCES_EVERY_TICK  16  // 8ms = 500us * 16
#define MAX_WOF_CONTROL_CHANCES G_max_wof_control_chances
extern uint8_t G_max_vrt_chances;
extern uint8_t G_max_wof_control_chances;
extern uint32_t G_max_ceff_ratio;

#define WOF_TABLES_OFFSET           0xC0000 // Relative to PPMR_ADDRESS_HOMER
#define MAX_CEFF_RATIO              10000   // 1.0 ratio = 10000
                                            // (scaled to avoid floating point)
//******************************************************************************
// Bit Vector Masks
//******************************************************************************
#define IVRM_STATE_QUAD_MASK 0x80

#define OCS_PGPE_DIRTY_MASK 0x40
#define OCS_PGPE_DIRTY_TYPE_MASK 0x20


//******************************************************************************
// WOF Reason Code Masks
//******************************************************************************
#define WOF_RC_NO_WOF_HEADER_MASK                  0x00000001
#define WOF_RC_INVALID_ACTIVE_QUADS                0x00000002
#define WOF_RC_INVALID_VDD_VDN                     0x00000004
#define WOF_RC_PGPE_REQ_NOT_IDLE                   0x00000008
#define WOF_RC_PGPE_WOF_DISABLED                   0x00000010
#define WOF_RC_PSTATE_PROTOCOL_OFF                 0x00000020
#define WOF_RC_VRT_REQ_TIMEOUT                    0x00000040
#define WOF_RC_CONTROL_REQ_TIMEOUT                 0x00000080
#define WOF_RC_STATE_CHANGE                        0x00000100
#define WOF_RC_MODE_CHANGE                         0x00000200
#define WOF_RC_MODE_NO_SUPPORT_MASK                0x00000400
#define WOF_RC_DIVIDE_BY_ZERO_VDD                  0x00000800
#define WOF_RC_VRT_REQ_FAILURE                    0x00001000
#define WOF_RC_CONTROL_REQ_FAILURE                 0x00002000
#define WOF_RC_VRT_ALIGNMENT_ERROR                0x00004000
#define WOF_RC_DRIVER_WOF_DISABLED                 0x00008000
#define WOF_RC_UTURBO_IS_ZERO                      0x00010000
#define WOF_RC_OCC_WOF_DISABLED                    0x00020000
#define WOF_RC_OPPB_WOF_DISABLED                   0x00040000
#define WOF_RC_SYSTEM_WOF_DISABLE                  0x00080000
#define WOF_RC_RESET_LIMIT_REACHED                 0x00100000
#define WOF_RC_UNSUPPORTED_FREQUENCIES             0x00200000
#define WOF_RC_NO_CONFIGURED_CORES                 0x00400000
#define WOF_RC_IPC_FAILURE                         0x00800000
#define WOF_RC_USER_DISABLED_WOF                   0x01000000
#define WOF_RC_RESET_DEBUG_CMD                     0x02000000
#define WOF_RC_DIVIDE_BY_ZERO_VDN                  0x04000000

//***************************************************************************
// Temp space used to save hard coded addresses
//***************************************************************************
#define PSTATE_TBL_ADDR 0xFFF2B85C

// Reason codes which should NOT create an error log should be added here
#define ERRL_RETURN_CODES ~(WOF_RC_MODE_CHANGE  | \
                            WOF_RC_STATE_CHANGE | \
                            WOF_RC_MODE_NO_SUPPORT_MASK | \
                            WOF_RC_NO_CONFIGURED_CORES | \
                            WOF_RC_RESET_LIMIT_REACHED | \
                            WOF_RC_UTURBO_IS_ZERO | \
                            WOF_RC_PSTATE_PROTOCOL_OFF | \
                            WOF_RC_USER_DISABLED_WOF )

// Reason codes that should NOT request a reset should be added here
#define IGNORE_WOF_RESET (WOF_RC_SYSTEM_WOF_DISABLE | \
                          WOF_RC_UNSUPPORTED_FREQUENCIES )
// Enumeration to define the WOF initialization steps
enum wof_init_states
{
    WOF_DISABLED,                   //0
    INITIAL_VRT_SENT_WAITING,       //1
    INITIAL_VRT_SUCCESS,            //2
    WOF_CONTROL_ON_SENT_WAITING,    //3
    PGPE_WOF_ENABLED_NO_PREV_DATA,  //4
    WOF_ENABLED,                    //5
};

// Enumeration to define VRT send state
enum vrt_send_states
{
    STANDBY,
    SEND_INIT,
    NEED_TO_SCHEDULE,
    SCHEDULED,
};

// Enumeration to define reasons (H)TMGT is disabling WOF
enum wof_disabled_htmgt_rc
{
    WOF_MISSING_ULTRA_TURBO     = 0x0000,
    WOF_SYSTEM_DISABLED         = 0x0001,
    WOF_RESET_LIMIT_REACHED     = 0x0002,
    WOF_UNSUPPORTED_FREQ        = 0x0003,
};

// Enumeration
enum wof_disabled_actions
{
    CLEAR,
    SET,
};

//#define WOF_PGPE_SUPPORT 1
#define WOF_MAGIC_NUMBER            0x57465448   // "WFTH"

// Structure to hold relevant data from the WOF header in Mainstore
typedef struct __attribute__ ((packed))
{
    uint32_t magic_number;
    uint8_t  reserved_1[3];
    uint8_t  version;
    uint16_t vrt_block_size;
    uint16_t vrt_blck_hdr_sz;
    uint16_t vrt_data_size;
    uint8_t  active_quads_size;
    uint8_t  core_count;
    uint16_t vdn_start;
    uint16_t vdn_step;
    uint16_t vdn_size;
    uint16_t vdd_start;
    uint16_t vdd_step;
    uint16_t vdd_size;
    uint16_t vratio_start;
    uint16_t vratio_step;
    uint16_t vratio_size;
    uint16_t fratio_start;
    uint16_t fratio_step;
    uint16_t fratio_size;
    uint16_t vdn_percent[8];
    uint16_t socket_power_w;
    uint16_t nest_freq_mhz;
    uint16_t nom_freq_mhz;
    uint16_t rdp_capacity;
    uint64_t wof_tbls_src_tag;
    uint64_t package_name_hi;
    uint64_t package_name_lo;
    uint8_t  reserved_2[40];
} wof_header_data_t;

// Structure used in g_amec
typedef struct __attribute__ ((packed))
{
    // Bit vector where each bit signifies a different failure case
    uint32_t wof_disabled;
    // Data from wof header for debug
    uint8_t  version;
    uint16_t vrt_block_size;
    uint16_t vrt_blck_hdr_sz;
    uint16_t vrt_data_size;
    uint8_t  active_quads_size;
    uint8_t  core_count;
    uint16_t vdn_start;
    uint16_t vdn_step;
    uint16_t vdn_size;
    uint16_t vdd_start;
    uint16_t vdd_step;
    uint16_t vdd_size;
    uint16_t vratio_start;
    uint16_t vratio_step;
    uint16_t vratio_size;
    uint16_t fratio_start;
    uint16_t fratio_step;
    uint16_t fratio_size;
    uint16_t vdn_percent[8];
    uint16_t socket_power_w;
    uint16_t nest_freq_mhz;
    uint16_t nom_freq_mhz;
    uint16_t rdp_capacity;
    uint64_t wof_tbls_src_tag;
    uint64_t package_name_hi;
    uint64_t package_name_lo;
    // Calculated step from start for VDD
    uint16_t vdd_step_from_start;
    // Calculated step from start for VDN
    uint16_t vdn_step_from_start;
    // Calculated step from start for quads
    uint8_t quad_step_from_start;
    // Array to hold the core voltages per quad (in 100uV)
    uint32_t v_core_100uV[MAXIMUM_QUADS];
    // Bit vector to hold the power on status of all 24 cores
    uint32_t core_pwr_on;
    // Number of cores on per quad
    uint8_t cores_on_per_quad[MAXIMUM_QUADS];
    // The most recently read value in the sensor VOLTVDDSENSE
    uint32_t voltvddsense_sensor;
    // The most recently read value in the sensor TEMPPROCTHRMCy where y is core num
    uint16_t tempprocthrmc[MAX_NUM_CORES];
    // The most recently read value in the sensor TEMPNEST
    uint16_t tempnest_sensor;
    // The most recently read value in the sensor TEMPQx where x is the quad num
    uint16_t tempq[MAXIMUM_QUADS];
    // The most recently read value in the sensor CURVDD
    uint16_t curvdd_sensor;
    // The most recently read value in the sensor CURVDN
    uint16_t curvdn_sensor;
    // The most recently read value in the sensor VOLTVDN
    uint16_t voltvdn_sensor;
    // Array to hold the current 1-byte pstate values read from SRAM. 0xFF=off
    uint8_t  quad_x_pstates[MAXIMUM_QUADS];
    // Array to hold the voltage index for the quads based on their voltage
    uint8_t quad_v_idx[MAXIMUM_QUADS];
    // Bit vector to hold the ivrm states of the quads. 0=BYPASS, 1=REGULATION
    uint8_t  quad_ivrm_states;
    // Contains the estimated core leakage based on temp, voltage, and vpd-leak
    uint32_t idc_vdd;
    // Contains the estimated nest leakage based on temp, voltage and vpd-leak
    uint32_t idc_vdn;
    // Contains the leakage current for quads
    uint32_t idc_quad;
    // Contains the AC component of the workload for the core
    uint32_t iac_vdd;
    // Contains the AC component of the workload for the nest
    uint32_t iac_vdn;
    // Contains iac_tdp_vdd(@turbo) read from the pstate parameter block
    uint32_t iac_tdp_vdd;
    // Contains Vratio, read from OCC-PGPE shared SRAM
    uint16_t v_ratio;
    // Contains Fratio, read from OCC-PGPE shared SRAM
    uint16_t f_ratio;
    // Contains Vclip, read from OCC-PGPE shared SRAM
    uint16_t v_clip;
    // Contains Fclip_ps pstate, read from OCC-PGPE shared SRAM
    uint8_t f_clip_ps;
    // Contains the actual frequency translated from f_clip_ps;
    uint32_t f_clip_freq;
    // Contains the calculated effective capacitance for tdp_vdd
    uint32_t ceff_tdp_vdd;
    // Contains the calculated effective capacitance for vdd
    uint32_t ceff_vdd;
    // Contains the calculated effective capacitance ratio for vdd
    uint32_t ceff_ratio_vdd;
    // Contains the calculated effective capacitance for tdp_vdn
    uint32_t ceff_tdp_vdn;
    // Contains the calculated effective capacitance for vdn
    uint32_t ceff_vdn;
    // Contains the calculated effective capacitance ratio for vdn
    uint32_t ceff_ratio_vdn;
    // Contains the index used for interpolation in the ALL_CORES_OFF_ISO calc
    uint8_t chip_volt_idx;
    // Contains the final calculated value of ALL_CORES_OFF_ISO
    uint32_t all_cores_off_iso;
    // Contains the final calculated value of ALL_GOOD_CACHES_ON_ISO
    uint32_t all_good_caches_on_iso;
    // Contains the final calculated value of ALL_CACHES_OFF_ISO
    uint32_t all_caches_off_iso;
    // Contains good_cores_only (per_quad)
    uint32_t quad_good_cores_only[MAXIMUM_QUADS];
    // Contains on_cores
    uint16_t quad_on_cores[MAXIMUM_QUADS];
    // Contains BAD_OFF_cores
    uint16_t quad_bad_off_cores[MAXIMUM_QUADS];
    // Contains the most recently read value from SRAM for Requested active quads
    // Value represents a bit vector denoting which quads are active
    uint8_t req_active_quad_update;
    // Contains the previous value read from shared SRAM for requested active quads
    uint8_t prev_req_active_quads;
    // Contains the number of active quads i.e. the number of 1 bits in req_active_quads_update
    uint8_t num_active_quads;
    // The current ping pong buffer SRAM address being used by PGPE
    uint32_t curr_ping_pong_buf;
    // The next ping pong buffer SRAM address to be used by PGPE if IPC request succeeds
    uint32_t next_ping_pong_buf;
    // The current vrt address in Main Memory that WOF pulled to give to PGPE
    uint32_t curr_vrt_main_mem_addr;
    // The most recently calculated vrt Main Memory address based off recently computed
    // data. This is not what the PGPE is using. It is the candidate addr for the next
    // vrt
    uint32_t next_vrt_main_mem_addr;
    // Main Memory address where the WOF VRT tables are located
    uint32_t vrt_tbls_main_mem_addr;
    // The length of the WOF VRT data in main memory
    uint32_t vrt_tbls_len;
    // The state of the wof routine during initialization. states defined above
    uint8_t wof_init_state;
    // The address in shared OCC-PGPE SRAM of Quad State 0
    uint32_t quad_state_0_addr;
    // The address in shared OCC-PGPE SRAM of Quad State 1
    uint32_t quad_state_1_addr;
    // The address in shared OCC-PGPE SRAM of the PGPE WOF state
    uint32_t pgpe_wof_state_addr;
    // The address in shared OCC-PGPE SRAM of the Requested Active quads
    uint32_t req_active_quads_addr;
    // The core leakage percent portion of VDD
    uint16_t core_leakage_percent;
    // The SRAM address of the pstates for the quads.
    uint32_t pstate_tbl_sram_addr;
    // Return code of IPC request called from callback func
    uint32_t gpe_req_rc;
    // Return code of failed control message
    uint32_t control_ipc_rc;
    // Keeps track of whether we got an error in wof_vrt_callback to be
    // logged later
    uint8_t vrt_callback_error;
    // Keeps track of whether the 405 was the one who disabled WOF on the PGPE
    uint8_t pgpe_wof_off;
    // Keeps track of whether or not the PGPE can enable WOF
    uint8_t pgpe_wof_disabled;
    // Offset into main memory with the beginning of the wof vrt data as base
    uint32_t vrt_mm_offset;
    // Return code returned from a bad VRT request
    uint8_t wof_vrt_req_rc;
    // Voltage used in ceff_ratio_vdd calc
    uint32_t c_ratio_vdd_volt;
    // Frequency used in ceff_ratio_vdd calc
    uint32_t c_ratio_vdd_freq;
    // Voltage used in ceff_ratio_vdn calc
    uint32_t c_ratio_vdn_volt;
    // Frequency used in ceff_ratio_vdn calc
    uint32_t c_ratio_vdn_freq;
    // Holds the state of various async operations relating to sending a VRT
    uint8_t vrt_state;
    uint32_t all_cores_off_before;
    //OPPB variables
    uint8_t good_quads_per_sort;
    uint8_t good_normal_cores_per_sort;
    uint8_t good_caches_per_sort;
    uint8_t good_normal_cores[MAXIMUM_QUADS];
    uint8_t good_caches[MAXIMUM_QUADS];
    uint16_t allGoodCoresCachesOn[CORE_IDDQ_MEASUREMENTS];
    uint16_t allCoresCachesOff[CORE_IDDQ_MEASUREMENTS];
    uint16_t coresOffCachesOn[CORE_IDDQ_MEASUREMENTS];
    uint16_t quad1CoresCachesOn[CORE_IDDQ_MEASUREMENTS];
    uint16_t quad2CoresCachesOn[CORE_IDDQ_MEASUREMENTS];
    uint16_t quad3CoresCachesOn[CORE_IDDQ_MEASUREMENTS];
    uint16_t quad4CoresCachesOn[CORE_IDDQ_MEASUREMENTS];
    uint16_t quad5CoresCachesOn[CORE_IDDQ_MEASUREMENTS];
    uint16_t quad6CoresCachesOn[CORE_IDDQ_MEASUREMENTS];
    uint16_t ivdn[CORE_IDDQ_MEASUREMENTS];
    uint8_t allCoresCachesOnT[CORE_IDDQ_MEASUREMENTS];
    uint8_t allCoresCachesOffT[CORE_IDDQ_MEASUREMENTS];
    uint8_t coresOffCachesOnT[CORE_IDDQ_MEASUREMENTS];
    uint8_t quad1CoresCachesOnT[CORE_IDDQ_MEASUREMENTS];
    uint8_t quad2CoresCachesOnT[CORE_IDDQ_MEASUREMENTS];
    uint8_t quad3CoresCachesOnT[CORE_IDDQ_MEASUREMENTS];
    uint8_t quad4CoresCachesOnT[CORE_IDDQ_MEASUREMENTS];
    uint8_t quad5CoresCachesOnT[CORE_IDDQ_MEASUREMENTS];
    uint8_t quad6CoresCachesOnT[CORE_IDDQ_MEASUREMENTS];
    uint8_t avgtemp_vdn[CORE_IDDQ_MEASUREMENTS];
    uint32_t full_leakage_08v_mA;
    uint64_t pgpe_wof_values_dw0;
    uint64_t pgpe_wof_values_dw1;
    uint64_t pgpe_wof_values_dw2;
    uint64_t pgpe_wof_values_dw3;
    uint8_t  ocs_dirty;  // Set by PGPE read from OCC Flag 0 register
    uint16_t ocs_increase_ceff; // Fixed CeffRatio increase addr defined in attribute
    uint16_t ocs_decrease_ceff; // Fixed CeffRatio decrease addr defined in attribute
    uint16_t vdd_oc_ceff_add;  // OCC calculated CeffRatio Addr
    uint16_t vdd_ceff_ratio_adj_prev;  // Final adjusted CeffRatio from previous tick
    uint32_t vdd_avg_tdp_100uv;
    uint32_t ocs_not_dirty_count; // count of number of times not dirty (type 0)
    uint32_t ocs_not_dirty_type1_count;  // count of not dirty (type 1) this counter should be 0
    uint32_t ocs_dirty_type0_count;  // count of number of times dirty with type hold (0)
    uint32_t ocs_dirty_type1_count;  // count of number of times dirty with type act (1)
} amec_wof_t;

// Structure for sensors used in g_amec for AMESTER for additional debug
typedef struct __attribute__ ((packed))
{
    sensor_t ceff_ratio_vdd_sensor;  // raw ceff ratio Vdd
    sensor_t ceff_ratio_vdn_sensor;
    sensor_t v_ratio_sensor;
    sensor_t ocs_addr_sensor;
    sensor_t ceff_ratio_vdd_adj_sensor; // final adjusted ratio should reflect amec_wof_t ceff_ratio_vdd
} amec_wof_sensors_t;

typedef struct __attribute__ ((packed))
{
    uint8_t data[MIN_BCE_REQ_SIZE];
} temp_bce_request_buffer_t __attribute ((aligned(128)));

//******************************************************************************
// Function Prototypes
//******************************************************************************

void call_wof_main( void );

void wof_main( void );

uint16_t calculate_step_from_start( uint16_t i_ceff_vdx,
                                    uint16_t i_step_size,
                                    uint16_t i_min_step,
                                    uint16_t i_max_step );

uint8_t calc_quad_step_from_start( void );



uint32_t calc_vrt_mainstore_addr( void );

void copy_vrt_to_sram_callback( void );

void wof_vrt_callback( void );

void send_vrt_to_pgpe( uint32_t i_vrt_address );

void read_shared_sram( void );

void read_pgpe_produced_wof_values( void );

void calculate_core_voltage( void );

void calculate_core_leakage( void );

void calculate_nest_leakage( void );

void calculate_ceff_ratio_vdn( void );

void calculate_ceff_ratio_vdd( void );

void calculate_AC_currents( void );

uint32_t  core_powered_on( uint8_t i_core_num );

uint8_t num_cores_on_in_quad( uint8_t i_quad_num );

int32_t interpolate_linear( int32_t i_X,
                            int32_t i_x1,
                            int32_t i_x2,
                            int32_t i_y1,
                            int32_t i_y2 );

void calculate_temperature_scaling_08V( void );

uint32_t calculate_multiplier( int32_t i_temp );

uint32_t calculate_effective_capacitance( uint32_t i_iAC,
                                          uint32_t i_voltage,
                                          uint32_t i_frequency );

void read_sensor_data( void );

void set_clear_wof_disabled( uint8_t  i_action,
                             uint32_t i_bit_mask,
                             uint16_t i_ext_rc );

void disable_wof( void );

bool enable_wof( void );

void wof_control_callback( void );

void send_initial_vrt_to_pgpe( void );

void read_req_active_quads( void );

int get_voltage_index( uint32_t i_voltage );

uint32_t scale( uint16_t i_current,
                int16_t i_delta_temp );

uint32_t scale_and_interpolate( uint16_t * i_leak_arr,
                                uint8_t * i_avgtemp_arr,
                                int i_idx,
                                uint16_t i_base_temp,
                                uint16_t i_voltage );

void print_data( void );

void print_oppb( void );

uint32_t prevent_over_current( uint32_t i_ceff_ratio );

void schedule_vrt_request( void );

uint32_t multiply_ratio( uint32_t i_operating_point,
                                uint32_t i_ratio );
#endif
