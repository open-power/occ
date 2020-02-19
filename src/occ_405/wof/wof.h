/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/wof/wof.h $                                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2020                        */
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
#define MIN_BCE_REQ_SIZE            256
#define WOF_HEADER_SIZE             128
#define CORE_IDDQ_MEASUREMENTS      6
#define PGPE_WOF_OFF                0
#define PGPE_WOF_ON                 1

#define MAX_VRT_CHANCES_EVERY_TICK  16  // 8ms = 500us * 16
#define MAX_VRT_CHANCES G_max_vrt_chances
#define MAX_WOF_CONTROL_CHANCES_EVERY_TICK  16  // 8ms = 500us * 16
#define MAX_WOF_CONTROL_CHANCES G_max_wof_control_chances
extern uint8_t G_max_vrt_chances;
extern uint8_t G_max_wof_control_chances;
extern uint32_t G_max_ceff_ratio;

#define MAX_CEFF_RATIO              10000   // 1.0 ratio = 10000
                                            // (scaled to avoid floating point)
// value to indicate VRT dimension does not have an override set by mfg test cmd
// else the override value is the index to use
#define WOF_VRT_IDX_NO_OVERRIDE     0xFF

//******************************************************************************
// Bit Vector Masks
//******************************************************************************
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
#define WOF_RC_VRT_REQ_TIMEOUT                     0x00000040
#define WOF_RC_CONTROL_REQ_TIMEOUT                 0x00000080
#define WOF_RC_STATE_CHANGE                        0x00000100
#define WOF_RC_MODE_CHANGE                         0x00000200
#define WOF_RC_MODE_NO_SUPPORT_MASK                0x00000400
#define WOF_RC_DIVIDE_BY_ZERO_VDD                  0x00000800
#define WOF_RC_VRT_REQ_FAILURE                     0x00001000
#define WOF_RC_CONTROL_REQ_FAILURE                 0x00002000
#define WOF_RC_VRT_ALIGNMENT_ERROR                 0x00004000
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
#define WOF_RC_DIVIDE_BY_ZERO_VCS                 0x04000000
#define WOF_RC_INVALID_IDDQ_SAMPLE_DEPTH           0x08000000

//***************************************************************************
// Temp space used to save hard coded addresses
//***************************************************************************

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

#define WOF_TABLES_MAGIC_NUMBER            0x57465448   // "WFTH"
#define WOF_TABLES_VERSION 1

// Structure to hold relevant data from the WOF header in Mainstore
typedef struct __attribute__ ((packed))
{
    uint32_t magic_number;
    uint8_t  reserved_1[3];
    uint8_t  version;
    uint16_t vrt_block_size;
    uint16_t vrt_blck_hdr_sz;
    uint16_t vrt_data_size;
    uint8_t  ocs_mode;
    uint8_t  core_count;
    uint16_t vcs_start;
    uint16_t vcs_step;
    uint16_t vcs_size;
    uint16_t vdd_start;
    uint16_t vdd_step;
    uint16_t vdd_size;
    uint16_t vratio_start;
    uint16_t vratio_step;
    uint16_t vratio_size;
    uint16_t io_pwr_start;
    uint16_t io_pwr_step;
    uint16_t io_pwr_size;
    uint16_t ambient_start;
    uint16_t ambient_step;
    uint16_t ambient_size;
    uint16_t reserved_2;
    uint16_t socket_power_w;
    uint16_t sort_pwr_target_freq_mhz;
    uint16_t rdp_current_amps;
    uint16_t boost_current_amps;
    uint32_t wof_tbls_timestamp;
    uint16_t wof_tbls_version;
    uint16_t reserved_3;
    uint8_t  tdp_vcs_ceff_idx;
    uint8_t  tdp_vdd_ceff_idx;
    uint8_t  tdp_io_pwr_idx;
    uint8_t  tdp_ambient_idx;
    uint8_t  io_full_pwr_watts;
    uint8_t  io_disabled_pwr_watts;
    uint16_t reserved_4;
    uint64_t package_name_hi;
    uint64_t package_name_lo;
    uint8_t  reserved_5[40];
} wof_header_data_t;

// Structure used in g_amec to hold WOF data that changes
typedef struct __attribute__ ((packed))
{
    // [0] Bit vector where each bit signifies a different failure case
    uint32_t wof_disabled;
    // [4] Calculated step from start for VDD
    uint16_t vdd_step_from_start;
    // [6] override sent for Vdd index via mfg test cmd 0xff indicates no override
    uint8_t vdd_override_index;
    // [7] Calculated step from start for VCS
    uint16_t vcs_step_from_start;
    // [9] override sent for Vcs index via mfg test cmd 0xff indicates no override
    uint8_t vcs_override_index;
    // [10] step from start for IO Power this is read from XGPE Produced WOF values
    uint16_t io_pwr_step_from_start;
    // [12] override sent for IO power index via mfg test cmd 0xff indicates no override
    uint8_t io_pwr_override_index;
    // [13] Calculated step from start for ambient
    uint16_t ambient_step_from_start;
    // [15] override sent for ambient index via mfg test cmd 0xff indicates no override
    uint8_t ambient_override_index;
    // [16] override sent for Vratio index via mfg test cmd 0xff indicates no override
    uint8_t v_ratio_override_index;
    // [17] The most recently read value in the sensor CURVDD
    uint16_t curvdd_sensor;
    // [19] The most recently read value in the sensor CURVCS
    uint16_t curvcs_sensor;
    // [21] The most recently read value in sensor VOLTVDDSENSE 0.1mv (100uV) unit
    uint32_t Vdd_chip_p1mv;
    // [25] The most recently read value in the sensor VOLTVCSSENSE 0.1mv (100uV) unit
    uint32_t Vcs_chip_p1mv;
    // [29] The most recently read value in the sensor TEMPPROCTHRMCy where y is core num
    uint16_t tempprocthrmc[MAX_NUM_CORES];
    // [93] The most recently read value in the sensor TEMPRTAVG
    uint16_t T_racetrack;
    // [95] calculated from XGPE IDDQ activity values (minus off & vmin) in 0.1% unit
    uint16_t p1pct_on[MAX_NUM_CORES];
    // [159] read from IDDQ activity values converted to 0.1% unit
    uint16_t p1pct_off[MAX_NUM_CORES];
    // [223] read from XGPE IDDQ activity values converted to 0.1% unit
    uint16_t p1pct_vmin[MAX_NUM_CORES];
    // [287] read from XGPE IDDQ activity values converted to 0.1% unit
    uint16_t p1pct_mma_off[MAX_NUM_CORES];
    // [351] Contains the estimated core leakage based on temp, voltage, and vpd-leak
    uint32_t iddq_ua;
    // [355] Contains the estimated cache leakage based on temp, voltage and vpd-leak
    uint32_t icsq_ua;
    // [359] Contains the AC component of the workload for the core
    uint32_t iac_vdd;
    // [363] Contains the AC component of the workload for the cache
    uint32_t iac_vcs;
    // [367] Contains iac_tdp_vdd(@turbo) read from the pstate parameter block
    uint32_t iac_tdp_vdd;
    // [371] Contains Vratio, read from OCC-PGPE shared SRAM
    uint16_t v_ratio;
    // [373] Contains clip_state value last read from VRT, read from OCC-PGPE shared SRAM
    uint8_t f_clip_ps;
    // [374] Contains the frequency for average frequency pState read from PGPE shared SRAM;
    uint32_t f_clip_freq;
    // [378] Contains the calculated effective capacitance for tdp_vdd
    uint32_t ceff_tdp_vdd;
    // [382] Contains the calculated effective capacitance for vdd
    uint32_t ceff_vdd;
    // [386] Contains the calculated effective capacitance ratio for vdd
    uint32_t ceff_ratio_vdd;
    // [390] Contains the calculated effective capacitance for tdp_vcs
    uint32_t ceff_tdp_vcs;
    // [394] Contains the calculated effective capacitance for vcs
    uint32_t ceff_vcs;
    // [398] Contains the calculated effective capacitance ratio for vcs
    uint32_t ceff_ratio_vcs;
    // [402]
    uint8_t Vdd_chip_index;
    // [403]
    uint8_t Vdd_vmin_index;
    // [404]
    uint8_t Vcs_chip_index;
    // [405]
    uint32_t scaled_all_off_off_vdd_chip_ua_nc;
    // [409]
    uint32_t scaled_all_off_off_vcs_chip_ua_nc;
    // [413]
    uint32_t racetrack_only_vcs_chip_ua;
    // [417]
    uint32_t scaled_all_off_off_vdd_chip_ua_c;
    // [421]
    uint32_t scaled_all_off_off_vdd_vmin_ua_c;
    // [425]
    uint32_t racetrack_only_vdd_chip_ua;
    // [429]
    uint32_t scaled_all_off_on_vdd_chip_ua_c;
    // [433]
    uint32_t scaled_all_off_on_vdd_vmin_ua_c;
    // [437]
    uint32_t single_core_off_vdd_chip_ua_c;
    // [441]
    uint32_t single_cache_off_vdd_chip_ua_c;
    // [445]
    uint32_t single_core_off_vdd_vmin_ua_c;
    // [449]
    uint32_t single_cache_on_vdd_chip_ua_c;
    // [453]
    uint32_t scaled_all_off_off_vdd_vmin_ua_nc;
    // [457]
    uint32_t scaled_all_off_on_vdd_chip_ua_nc;
    // [461]
    uint32_t scaled_all_off_on_vdd_vmin_ua_nc;
    // [465]
    uint32_t single_core_off_vdd_chip_ua_nc;
    // [469]
    uint32_t single_cache_off_vdd_chip_ua_nc;
    // [473]
    uint32_t single_core_off_vdd_vmin_ua_nc;
    // [477]
    uint32_t single_cache_on_vdd_chip_ua_nc;
    // [481]
    uint32_t scaled_all_off_off_vcs_chip_ua_c;
    // [485]
    uint32_t scaled_all_off_on_vcs_chip_ua_c;
    // [489]
    uint32_t single_core_off_vcs_chip_ua_c;
    // [493]
    uint32_t single_cache_off_vcs_chip_ua_c;
    // [497]
    uint32_t single_cache_on_vcs_chip_ua_c;
    // [501]
    uint32_t scaled_all_off_on_vcs_chip_ua_nc;
    // [505]
    uint32_t single_core_off_vcs_chip_ua_nc;
    // [509]
    uint32_t single_cache_off_vcs_chip_ua_nc;
    // [513]
    uint32_t single_cache_on_vcs_chip_ua_nc;
    // [517]
    uint32_t scaled_good_eqs_on_on_vdd_chip_ua;
    // [521]
    uint32_t single_core_on_vdd_chip_eqs_ua;
    // [525]
    uint32_t scaled_good_eqs_on_on_vdd_vmin_ua;
    // [529]
    uint32_t single_core_on_vdd_vmin_eqs_ua;
    // [533]
    uint32_t scaled_good_eqs_on_on_vcs_chip_ua;
    // [537]
    uint32_t single_core_on_vcs_chip_eqs_ua;
    // [541] The current ping pong buffer SRAM address being used by PGPE
    uint32_t curr_ping_pong_buf;
    // [545] The next ping pong buffer SRAM address to be used by PGPE if IPC request succeeds
    uint32_t next_ping_pong_buf;
    // [549] Most recently calculated vrt Main Memory address
    uint32_t vrt_main_mem_addr;
    // [553] if vrt_main_mem_addr needed to be 128B aligned this is number of tables in the real table is
    // 0 if no adjustement required
    uint32_t vrt_bce_table_offset;
    // [557] The state of the wof routine during initialization. states defined above
    uint8_t wof_init_state;
    // [558] Return code of IPC request called from callback func
    uint32_t gpe_req_rc;
    // [562] Return code of failed control message
    uint32_t control_ipc_rc;
    // [566] Keeps track of whether we got an error in wof_vrt_callback to be
    // logged later
    uint8_t vrt_callback_error;
    // [567] Keeps track of whether the 405 was the one who disabled WOF on the PGPE
    uint8_t pgpe_wof_off;
    // [568] Keeps track of whether or not the PGPE can enable WOF
    uint8_t pgpe_wof_disabled;
    // [569] Offset into main memory with the beginning of the wof vrt data as base
    uint32_t vrt_mm_offset;
    // [573] Return code returned from a bad VRT request
    uint8_t wof_vrt_req_rc;
    // [574] Voltage used in ceff_ratio_vdd calc
    uint32_t c_ratio_vdd_volt;
    // [578] Frequency used in ceff_ratio_vdd calc
    uint32_t c_ratio_vdd_freq;
    // [582] Voltage used in ceff_ratio_vcs calc
    uint32_t c_ratio_vcs_volt;
    // [586] Frequency used in ceff_ratio_vcs calc
    uint32_t c_ratio_vcs_freq;
    // [590] Holds the state of various async operations relating to sending a VRT
    uint8_t vrt_state;

    // [591] PGPE Produced WOF Values
    uint64_t pgpe_wof_values_dw0;
    // [599] PGPE Produced WOF Values
    uint64_t pgpe_wof_values_dw1;
    // [607] PGPE Produced WOF Values
    uint64_t pgpe_wof_values_dw2;
    // [615] PGPE Produced WOF Values
    uint64_t pgpe_wof_values_dw3;
    // [623] OVercurrent status dirty bits. Set by PGPE read from OCC Flag 0 register
    uint8_t  ocs_dirty;

    // the following two vars can be changed via debug command
    // [624] Fixed CeffRatio increase addr defined in attribute
    uint16_t ocs_increase_ceff;
    // [626] Fixed CeffRatio decrease addr defined in attribute
    uint16_t ocs_decrease_ceff;

    // [628] OCC calculated CeffRatio Addr
    uint16_t vdd_oc_ceff_add;
    // [630] Final adjusted CeffRatio from previous tick
    uint16_t vdd_ceff_ratio_adj_prev;
    // [632]
    uint32_t vdd_avg_tdp_100uv;
    // [636] count of number of times not dirty (type 0)
    uint32_t ocs_not_dirty_count;
    // [640] count of not dirty (type 1) this counter should be 0
    uint32_t ocs_not_dirty_type1_count;
    // [644] count of number of times dirty with type hold (0)
    uint32_t ocs_dirty_type0_count;
    // [648] count of number of times dirty with type act (1)
    uint32_t ocs_dirty_type1_count;
    // [652] Ambient condition used to determine VRT
    uint32_t ambient_condition;
} amec_wof_t;  // 656 bytes total

// Structure used in g_amec to hold static WOF data
typedef struct __attribute__ ((packed))
{
    // Data from wof table header
    wof_header_data_t wof_header;
    // Main Memory address where the WOF VRT tables are located
    uint32_t vrt_tbls_main_mem_addr;
    // The length of the WOF VRT data in main memory
    uint32_t vrt_tbls_len;
    // The address in shared OCC-PGPE SRAM for OCC produced WOF values
    uint32_t occ_values_sram_addr;
    // The address in shared OCC-PGPE SRAM for PGPE produced WOF values
    uint32_t pgpe_values_sram_addr;
    // The address in shared OCC-PGPE SRAM for XGPE produced WOF values
    uint32_t xgpe_values_sram_addr;
    // SRAM address in shared OCC-PGPE SRAM for XGPE IDDQ Acitvity values
    uint32_t xgpe_iddq_activity_sram_addr;
    // The SRAM address of the pstates for the quads.
    uint32_t pstate_tbl_sram_addr;
    // Vmin in 0.1mv (100uV) unit read from OPPB
    uint32_t Vdd_vmin_p1mv;
    // voltage index for Vdd_vmin_p1mv
    uint8_t Vdd_vmin_index;
    // Number of XGPE sample ticks for IDDQ activity counters
    uint8_t iddq_activity_sample_depth;
    // Number of bits to shift right to divide by IDDQ activity sample depth to calculate percentage
    uint8_t iddq_activity_divide_bit_shift;
} amec_static_wof_t;

// Structure for sensors used in g_amec for AMESTER for additional debug
typedef struct __attribute__ ((packed))
{
    sensor_t ceff_ratio_vdd_sensor;  // raw ceff ratio Vdd
    sensor_t ceff_ratio_vcs_sensor;
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

uint16_t calculate_step_from_start( uint16_t i_vrt_parm,
                                    uint16_t i_step_size,
                                    uint16_t i_min_step,
                                    uint16_t i_max_step );

uint32_t calc_vrt_mainstore_addr( void );

void copy_vrt_to_sram_callback( void );

void wof_vrt_callback( void );

void copy_vrt_to_sram( uint32_t i_vrt_address );

void task_send_vrt_to_pgpe( task_t* i_task );

void read_pgpe_produced_wof_values( void );

void calculate_core_leakage( void );

void calculate_ceff_ratio_vcs( void );

void calculate_ceff_ratio_vdd( void );

void calculate_AC_currents( void );

int32_t interpolate_linear( int32_t i_X,
                            int32_t i_x1,
                            int32_t i_x2,
                            int32_t i_y1,
                            int32_t i_y2 );

void get_poundV_points( uint32_t i_freq_mhz,
                        uint8_t* o_point1_index,
                        uint8_t* o_point2_index);

uint32_t calculate_effective_capacitance( uint32_t i_iAC,
                                          uint32_t i_voltage,
                                          uint32_t i_frequency );

void read_sensor_data( void );

void setup_vdd( void );

void setup_vcs( void );

void setup_racetrack( void );

void set_clear_wof_disabled( uint8_t  i_action,
                             uint32_t i_bit_mask,
                             uint16_t i_ext_rc );

void disable_wof( void );

bool enable_wof( void );

void wof_control_callback( void );

void send_initial_vrt_to_pgpe( void );

int get_voltage_index( uint32_t i_voltage );

uint32_t scale( uint16_t i_temp1,
                uint16_t i_temp2,
                bool i_non_core_scaling_line );

uint32_t scale_and_interpolate( uint16_t * i_leak_arr,
                                uint8_t * i_avgtemp_arr,
                                int i_idx,
                                uint16_t i_base_temp,
                                uint16_t i_voltage,
                                bool i_non_core_scaling_line );

void print_data( void );

void print_oppb( void );

uint32_t prevent_over_current( uint32_t i_ceff_ratio );

void schedule_vrt_request( void );

uint32_t multiply_ratio( uint32_t i_operating_point,
                                uint32_t i_ratio );
#endif
