/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/wof/wof.h $                                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2023                        */
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
//                                                 0x00000002
#define WOF_RC_INVALID_VDD_VDN                     0x00000004
//                                                 0x00000008
#define WOF_RC_PGPE_WOF_DISABLED                   0x00000010
#define WOF_RC_PSTATE_PROTOCOL_OFF                 0x00000020
#define WOF_RC_VRT_REQ_TIMEOUT                     0x00000040
#define WOF_RC_CONTROL_REQ_TIMEOUT                 0x00000080
#define WOF_RC_STATE_CHANGE                        0x00000100
//                                                 0x00000200
#define WOF_RC_MODE_NO_SUPPORT_MASK                0x00000400
#define WOF_RC_DIVIDE_BY_ZERO_VDD                  0x00000800
#define WOF_RC_VRT_REQ_FAILURE                     0x00001000
#define WOF_RC_CONTROL_REQ_FAILURE                 0x00002000
#define WOF_RC_VRT_ALIGNMENT_ERROR                 0x00004000
//                                                 0x00008000
#define WOF_RC_UTURBO_IS_ZERO                      0x00010000
#define WOF_RC_OCC_WOF_DISABLED                    0x00020000
#define WOF_RC_OPPB_WOF_DISABLED                   0x00040000
#define WOF_RC_SYSTEM_WOF_DISABLE                  0x00080000
#define WOF_RC_RESET_LIMIT_REACHED                 0x00100000
//                                                 0x00200000
#define WOF_RC_NO_CONFIGURED_CORES                 0x00400000
#define WOF_RC_IPC_FAILURE                         0x00800000
//                                                 0x01000000
#define WOF_RC_RESET_DEBUG_CMD                     0x02000000
#define WOF_RC_DIVIDE_BY_ZERO_VCS                  0x04000000
#define WOF_RC_INVALID_IDDQ_SAMPLE_DEPTH           0x08000000
#define WOF_RC_NEGATIVE_MMA_LEAKAGE                0x20000000

//***************************************************************************
// Temp space used to save hard coded addresses
//***************************************************************************

// Reason codes which should NOT create an error log should be added here
#define ERRL_RETURN_CODES ~(WOF_RC_STATE_CHANGE | \
                            WOF_RC_MODE_NO_SUPPORT_MASK | \
                            WOF_RC_NO_CONFIGURED_CORES | \
                            WOF_RC_RESET_LIMIT_REACHED | \
                            WOF_RC_UTURBO_IS_ZERO | \
                            WOF_RC_PSTATE_PROTOCOL_OFF )

// Reason codes that should NOT request a reset should be added here
#define IGNORE_WOF_RESET (WOF_RC_SYSTEM_WOF_DISABLE )

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
    STANDBY            = 0,
    SEND_INIT          = 1,
    NEED_TO_SCHEDULE   = 2,
    SCHEDULED          = 3,
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
    uint32_t iddq_100ua;
    // [355] Contains the estimated cache leakage based on temp, voltage and vpd-leak
    uint32_t icsq_100ua;
    // [359] Contains the AC component of the workload for the core
    uint32_t iac_vdd_100ua;
    // [363] Contains the AC component of the workload for the cache
    uint32_t iac_vcs_100ua;
    // [367] Contains iac_tdp_vdd(@avg F) interpolated from OPPB operating points
    uint32_t iac_tdp_vdd_100ua;
    // [371] Contains Vdd Vratio average roundup, read from OCC-PGPE shared SRAM
    uint16_t v_ratio_vdd;
    // [373] Contains clip_state value last read from VRT, read from OCC-PGPE shared SRAM
    uint8_t  f_clip_ps;
    // [374] Contains Vcs Vratio average roundup, read from OCC-PGPE shared SRAM
    uint16_t v_ratio_vcs;
    // [376] Contains the numerator for calculating ceff_ratio_vdd
    uint64_t ceff_ratio_vdd_numerator;
    // [384] Contains the denominator for calculating ceff_ratio_vdd
    uint64_t ceff_ratio_vdd_denominator;
    // [392] Contains the calculated effective capacitance ratio for vdd
    uint32_t ceff_ratio_vdd;
    // [396] Contains the numerator for calculating ceff_ratio_vcs
    uint64_t ceff_ratio_vcs_numerator;
    // [404] Contains the denominator for calculating ceff_ratio_vcs
    uint64_t ceff_ratio_vcs_denominator;
    // [412] Contains the calculated effective capacitance ratio for vcs
    uint32_t ceff_ratio_vcs;
    // [416]
    uint8_t Vdd_chip_index;
    // [417] Contains degrees C ambient is changed by to account for higher/lower altitudes than reference altitude this may be lowering ambient
    int8_t ambient_adj_for_altitude;
    // [418] Altitude in meters 0xffff indicates not available
    uint16_t altitude;
    // [420]
    uint8_t Vcs_chip_index;
    // [421]
    uint32_t scaled_all_off_off_vdd_chip_ua_nc;
    // [425]
    uint32_t scaled_all_off_off_vcs_chip_ua_nc;
    // [429]
    uint32_t racetrack_only_vcs_chip_ua;
    // [433]
    uint32_t scaled_all_off_off_vdd_chip_ua_c;
    // [437]
    uint32_t scaled_all_off_off_vdd_vmin_ua_c;
    // [441]
    uint32_t racetrack_only_vdd_chip_ua;
    // [445]
    uint32_t scaled_all_off_on_vdd_chip_ua_c;
    // [449]
    uint32_t scaled_all_off_on_vdd_vmin_ua_c;
    // [453]
    uint32_t single_core_off_vdd_chip_ua_c;
    // [457]
    uint32_t single_cache_off_vdd_chip_ua_c;
    // [461]
    uint32_t single_core_off_vdd_vmin_ua_c;
    // [465]
    uint32_t single_cache_on_vdd_chip_ua_c;
    // [469]
    uint32_t scaled_all_off_off_vdd_vmin_ua_nc;
    // [473]
    uint32_t scaled_all_off_on_vdd_chip_ua_nc;
    // [477]
    uint32_t scaled_all_off_on_vdd_vmin_ua_nc;
    // [481]
    uint32_t single_core_off_vdd_chip_ua_nc;
    // [485]
    uint32_t single_cache_off_vdd_chip_ua_nc;
    // [489]
    uint32_t single_core_off_vdd_vmin_ua_nc;
    // [493]
    uint32_t single_cache_on_vdd_chip_ua_nc;
    // [497]
    uint32_t scaled_all_off_off_vcs_chip_ua_c;
    // [501]
    uint32_t scaled_all_off_on_vcs_chip_ua_c;
    // [505]
    uint32_t single_core_off_vcs_chip_ua_c;
    // [509]
    uint32_t single_cache_off_vcs_chip_ua_c;
    // [513]
    uint32_t single_cache_on_vcs_chip_ua_c;
    // [517]
    uint32_t scaled_all_off_on_vcs_chip_ua_nc;
    // [521]
    uint32_t single_core_off_vcs_chip_ua_nc;
    // [525]
    uint32_t single_cache_off_vcs_chip_ua_nc;
    // [529]
    uint32_t single_cache_on_vcs_chip_ua_nc;
    // [533]
    uint32_t scaled_good_eqs_on_on_vdd_chip_ua;
    // [537]
    uint32_t single_core_on_vdd_chip_eqs_ua;
    // [541]
    uint32_t scaled_good_eqs_on_on_vdd_vmin_ua;
    // [545]
    uint32_t single_core_on_vdd_vmin_eqs_ua;
    // [549]
    uint32_t scaled_good_eqs_on_on_vcs_chip_ua;
    // [553]
    uint32_t single_core_on_vcs_chip_eqs_ua;
    // [557] The current ping pong buffer SRAM address being used by PGPE
    uint32_t curr_ping_pong_buf;
    // [561] The next ping pong buffer SRAM address to be used by PGPE if IPC request succeeds
    uint32_t next_ping_pong_buf;
    // [565] Most recently calculated vrt Main Memory address
    uint32_t vrt_main_mem_addr;
    // [569] if vrt_main_mem_addr needed to be 128B aligned this is number of tables in the real table is
    // 0 if no adjustement required
    uint32_t vrt_bce_table_offset;
    // [573] The state of the wof routine during initialization. states defined above
    uint8_t wof_init_state;
    // [574] Return code of IPC request called from callback func
    uint32_t gpe_req_rc;
    // [578] Return code of failed control message
    uint32_t control_ipc_rc;
    // [582] Keeps track of whether we got an error in wof_vrt_callback to be
    // logged later
    uint8_t vrt_callback_error;
    // [583] Keeps track of whether the 405 was the one who disabled WOF on the PGPE
    uint8_t pgpe_wof_off;
    // [584] Keeps track of whether or not the PGPE can enable WOF
    uint8_t pgpe_wof_disabled;
    // [585] Offset into main memory with the beginning of the wof vrt data as base
    uint32_t vrt_mm_offset;
    // [589] Return code returned from a bad VRT request
    uint8_t wof_vrt_req_rc;
    // [590] scaled Vdd TDP AC current to the number of active cores represented by v_ratio_vdd
    uint32_t c_ratio_iac_tdp_vddp1ma;
    // [594] Average frequency (read from PGPE) used in ceff ratio vdd and vcs calculations
    uint32_t avg_freq_mhz;
    // [598] scaled Vcs TDP AC current to the number of active cores represented by v_ratio_vcs
    uint32_t c_ratio_iac_tdp_vcsp1ma;
    // [602] Contains iac_tdp_vcs(@avg F) interpolated from OPPB operating points
    uint32_t iac_tdp_vcs_100ua;
    // [606] Holds the state of various async operations relating to sending a VRT
    uint8_t vrt_state;

    // [607] PGPE Produced WOF Values
    uint64_t pgpe_wof_values_dw0;
    // [615] PGPE Produced WOF Values
    uint64_t pgpe_wof_values_dw1;
    // [623] PGPE Produced WOF Values
    uint64_t pgpe_wof_values_dw2;
    // [631] PGPE Produced WOF Values
    uint64_t pgpe_wof_values_dw3;
    // [639] Overcurrent status dirty bits. Set by PGPE read from OCC Flag 0 register
    uint8_t  ocs_dirty;

    // [640] XGPE Produced WOF Values
    uint64_t xgpe_wof_values_dw0;

    // the following two vars can be changed via debug command
    // [648] Fixed CeffRatio increase addr defined in attribute
    uint16_t ocs_increase_ceff;
    // [650] Fixed CeffRatio decrease addr defined in attribute
    uint16_t ocs_decrease_ceff;

    // [652] OCC calculated CeffRatio Addr due to dirty
    uint16_t vdd_oc_ceff_add;
    // [654] Final adjusted CeffRatio due to dirty only from previous tick
    uint16_t vdd_ceff_ratio_adj_prev;
    // [656] count of number of times not dirty (type 0)
    uint32_t ocs_not_dirty_count;
    // [660] count of not dirty (type 1) this counter should be 0
    uint32_t ocs_not_dirty_type1_count;
    // [664] count of number of times dirty with type hold (0)
    uint32_t ocs_dirty_type0_count;
    // [668] count of number of times dirty with type act (1)
    uint32_t ocs_dirty_type1_count;
    // [672] Ambient condition used to determine VRT
    uint32_t ambient_condition;
    // [676] #V index 1 used for ceff ratio frequency interpolation
    uint8_t  vpd_index1;
    // [677] #V index 2 used for ceff ratio frequency interpolation
    uint8_t  vpd_index2;
    // [678] Vdd voltage average TDP by interpolating #V voltage@(avg F)
    uint32_t vdd_avg_tdp_100uv;
    // [682] Racetrack AC current average TDP by interpolating VPD current@(avg F)
    uint32_t tdp_idd_rt_ac_100ua;
    // [686] Vcs voltage average TDP by interpolating #V voltage@(avg F)
    uint32_t vcs_avg_tdp_100uv;
    // [690] value of Vdd_chip_p1mv^1.3
    uint32_t vddp1mv_exp1p3;
    // [694] value of Vcs_chip_p1mv^1.3
    uint32_t vcsp1mv_exp1p3;
    // [698] value of vcs_avg_tdp_100uv^1.3
    uint32_t tdpvcsp1mv_exp1p3;
    // [702] value of vdd_avg_tdp_100uv^1.3
    uint32_t tdpvddp1mv_exp1p3;

    // [706] Stop state activity counters read from XGPE
    iddq_activity_t  xgpe_activity_values;  // 128 bytes = 32 cores * 4 bytes
    // [834] Inidcates if VRT was interpolated based on ambient
    uint8_t interpolate_ambient_vrt;
    // [835] VRT contents last sent to PGPE
    uint8_t VRT[16];
    // [851] OCC calculated CeffRatio Addr due to processor throttling
    uint32_t vdd_throt_ceff_add;
    // [855] Final adjusted CeffRatio due to processor throttling only
    uint32_t vdd_ceff_ratio_throt_adj;
    // [859] throttle index ceff percentage in 0.000001% from OCC scratch reg 3 x100 or 1562500
    uint32_t throt_idx_percent;
    // [863] throttle index offset read from OCC scratch reg 3
    uint32_t throt_idx_offset;
    // [867] signed amount in 0.01% unit to add (or subtract) to ceff.  Set by Eco mode
    int16_t eco_mode_ceff_add;
    // [869] MHz to reduce maximum frequency by.  Set by Eco mode
    uint16_t eco_mode_freq_degrade_mhz;
    // [871] Thermal credit constant for memory
    uint16_t mem_thermal_credit_constant;
    // [873] Worst case dimm power per OCMB in cW
    uint32_t max_dimm_pwr_ocmb_cW;
    // [877] indicates which membufs we have valid memory power interpolation data for
    uint32_t ocmbs_present;
    // [881] Total Worst case dimm power in cW
    uint32_t max_dimm_pwr_total_cW;
    // [885] Total DDIMM preheat power in cW
    uint32_t total_dimm_preheat_pwr_cW;
    // [889] Contains degrees C ambient is changed by to account for DIMM power
    int8_t ambient_adj_for_dimm;
} amec_wof_t;  // 890 bytes total

// Structure used in g_amec to hold static WOF data
typedef struct __attribute__ ((packed, aligned(128)))
{
    // Data from wof table header structure defined in /include/pstates_common.H
    WofTablesHeader_t wof_header;
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
    // Vdd voltage in 0.1mv (100uV) with cores in retention state read from OPPB
    uint32_t Vdd_vret_p1mv;
    // voltage index for Vdd_vret_p1mv
    uint8_t Vdd_vret_index;
    // Number of XGPE sample ticks for IDDQ activity counters
    uint8_t iddq_activity_sample_depth;
    // Number of bits to shift right to divide by IDDQ activity sample depth to calculate percentage
    uint8_t iddq_activity_divide_bit_shift;
    // Altitude temperature adjustment (in (degrees Celsius/km)*1000)
    uint32_t altitude_temp_adj_degCpm;
    // Altitude base in meters
    uint32_t altitude_reference_m;
    // Last ambient condition in WOF tables
    uint32_t last_ambient_condition;
} amec_static_wof_t;

// Structure for sensors used in g_amec for AMESTER for additional debug
typedef struct __attribute__ ((packed))
{
    sensor_t ceff_ratio_vdd_sensor;  // raw ceff ratio Vdd
    sensor_t ceff_ratio_vcs_sensor;
    sensor_t v_ratio_sensor;
    sensor_t ocs_addr_sensor;
    sensor_t ceff_ratio_vdd_adj_sensor; // final adjusted ratio should reflect amec_wof_t ceff_ratio_vdd
    sensor_t io_proxy_sensor;
    sensor_t uv_avg_sensor;
    sensor_t ov_avg_sensor;
    sensor_t freq_pstate_sensor;
    sensor_t throttle_addr_sensor;
} amec_wof_sensors_t;

typedef struct __attribute__ ((packed))
{
    uint8_t data[MIN_BCE_REQ_SIZE];
} temp_bce_request_buffer_t __attribute ((aligned(128)));

// structure for overcurrent protection controller when WOF is off
typedef struct __attribute__ ((packed))
{
    // *STATIC VALUE* number of Pstates to decrease by (increase freq) when no OC
    uint8_t                       decrease_pstate;
    // *STATIC VALUE* number of Pstates to increase by (decrease freq) when OC type act detected
    uint8_t                       increase_pstate;
    // *STATIC VALUE* Maximum Pstate (lowest freq) controller may ask for
    uint8_t                      pstate_max;
    // Pstate request of the controller
    uint8_t                       pstate_request;
    // Pstate request as frequency in MHz (may include throttle space)
    uint16_t                      freq_request;
}oc_wof_off_t;

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

uint32_t multiply_v_ratio( uint32_t i_value, bool i_vcs );

void calculate_AC_currents( void );

int32_t interpolate_linear( int32_t i_X,
                            int32_t i_x1,
                            int32_t i_x2,
                            int32_t i_y1,
                            int32_t i_y2,
                            bool    i_roundup );

void get_poundV_points( uint32_t i_freq_mhz,
                        uint8_t* o_point1_index,
                        uint8_t* o_point2_index);

uint32_t calculate_exp_1p3(uint32_t i_x);

void read_sensor_data( void );

void calc_wof_dimm_adjustment( void );

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

void prevent_oc_wof_off( void );

void schedule_vrt_request( void );

void interpolate_ambient_vrt(uint8_t * i_ping_pong_buffer_address,
                             uint8_t * i_vrt_address);

#endif
