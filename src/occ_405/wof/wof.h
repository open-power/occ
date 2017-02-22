/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/wof/wof.h $                                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2017                        */
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
#define WOF_HEADER_SIZE             32
#define CORE_IDDQ_MEASUREMENTS      6
#define QUAD_POWERED_OFF            0xFF
#define PGPE_WOF_OFF                0
#define PGPE_WOF_ON                 1
//******************************************************************************
// Bit Vector Masks
//******************************************************************************
#define IVRM_STATE_QUAD_MASK 0x80

//******************************************************************************
// WOF Reason Code Masks
//******************************************************************************
#define WOF_RC_NO_WOF_HEADER_MASK                  0x0001
#define WOF_RC_INVALID_ACTIVE_QUADS                0x0002
#define WOF_RC_INVALID_VDD_VDN                     0x0004
#define WOF_RC_PGPE_REQ_NOT_IDLE                   0x0008
#define WOF_RC_PGPE_WOF_DISABLED                   0x0010
#define WOF_RC_PSTATE_PROTOCOL_OFF                 0x0020
#define WOF_RC_VFRT_REQ_TIMEOUT                    0x0040
#define WOF_RC_CONTROL_REQ_TIMEOUT                 0x0080
#define WOF_RC_STATE_CHANGE                        0x0100
#define WOF_RC_MODE_CHANGE                         0x0200
#define WOF_RC_MODE_NO_SUPPORT_MASK                0x0400
#define WOF_RC_DIVIDE_BY_ZERO                      0x0800
#define WOF_RC_VFRT_REQ_FAILURE                    0x1000
#define WOF_RC_CONTROL_REQ_FAILURE                 0x2000


// Reason codes which should NOT create an error log should be added here
#define ERRL_RETURN_CODES ~(WOF_RC_MODE_CHANGE  | \
                            WOF_RC_STATE_CHANGE | \
                            WOF_RC_MODE_NO_SUPPORT_MASK)

// Enumeration to define the WOF initialization steps
enum wof_init_states
{
    WOF_DISABLED,
    INITIAL_VFRT_SENT_WAITING,
    INITIAL_VFRT_SUCCESS,
    WOF_CONTROL_ON_SENT_WAITING,
    PGPE_WOF_ENABLED_NO_PREV_DATA,
    WOF_ENABLED,
};



#define WOF_MAGIC_NUMBER            0x57465448   // "WFTH"

// Structure to hold relevant data from the WOF header in Mainstore
typedef struct __attribute__ ((packed))
{
    uint32_t magic_number;
    uint8_t reserved[3];
    uint8_t version;
    uint8_t size_of_vfrt;
    uint8_t vfrt_data_size;
    uint8_t active_quads_start;
    uint8_t active_quads_size;
    uint8_t vdn_start;
    uint8_t vdn_step;
    uint8_t vdn_size;
    uint8_t vdd_start;
    uint8_t vdd_step;
    uint8_t vdd_size;
} wof_header_data_t;



// Structure used in g_amec
typedef struct
{
    // Bit vector where each bit signifies a different failure case
    uint16_t wof_disabled;
    // Data from wof header for debug
    uint8_t size_of_vfrt;
    uint8_t vfrt_data_size;
    uint8_t active_quads_start;
    uint8_t active_quads_size;
    uint8_t vdn_start;
    uint8_t vdn_step;
    uint8_t vdn_size;
    uint8_t vdd_start;
    uint8_t vdd_step;
    uint8_t vdd_size;
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
    // Contains iac_tdp_vdn read from the pstate parameter block
    uint32_t iac_tdp_vdn;
    // Contains Vratio, read from OCC-PGPE shared SRAM
    uint32_t v_ratio;
    // Contains Fratio, read from OCC-PGPE shared SRAM
    uint32_t f_ratio;
    // Contains Vclip, read from OCC-PGPE shared SRAM
    uint32_t v_clip;
    // Contains Fclip, read from OCC-PGPE shared SRAM
    uint32_t f_clip;
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
    uint8_t voltage_idx;
    // Contains the final calculated value of ALL_CORES_OFF_ISO
    uint32_t all_cores_off_iso;
    // Contains the final calculated value of ALL_CACHES_ON_ISO
    uint32_t all_caches_on_iso;
    // Contains good_cores_only (per_quad)
    uint16_t quad_good_cores_only[MAXIMUM_QUADS];
    // Contains on_cores
    uint16_t quad_on_cores[MAXIMUM_QUADS];
    // Contains BAD_OFF_cores
    uint16_t quad_bad_off_cores[MAXIMUM_QUADS];
    // Contains the multiplier(m) used in y ~=(T*m)>>10 for nest leak calc
    uint32_t nest_mult;
    // Contains the multiplier(m) used in y ~=(T*m)>>10 for core leak calc 0-23
    uint32_t core_mult[MAX_NUM_CORES];
    // Contains the multiplier(m) used in y ~=(T*m)>>10 for quad leak calc 0-5
    uint32_t quad_mult[MAXIMUM_QUADS];
    // Contains the delta temp used for nest leakage calc (see G_wof_iddq_mult_table)
    // TEMPNEST - tvpd_leak_off
    int16_t nest_delta_temp;
    // Contains the delta temp used for core leakage calc
    // TEMPPROCTHRMy - tvpd_leak_on (where y is the core number)
    int16_t core_delta_temp[MAX_NUM_CORES];
    // Contains the delta temp used for quad leakage calc
    // TEMPQx - tvpd_leak_cache (where x is the quad number)
    int16_t quad_delta_temp[MAXIMUM_QUADS];
    // tvpd leak to use when either the core is off, or the entire quad is off
    uint32_t tvpd_leak_off;
    // tvpd leak to use when the core is on
    uint32_t tvpd_leak_on;
    // tvpd leak to use when performing cache calculations
    uint32_t tvpd_leak_cache;
    // tvpd leak used for nest leakage calculations
    uint32_t tvpd_leak_nest;
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
    // The current vfrt address in Main Memory that WOF pulled to give to PGPE
    uint32_t curr_vfrt_main_mem_addr;
    // The most recently calculated vfrt Main Memory address based off recently computed
    // data. This is not what the PGPE is using. It is the candidate addr for the next
    // vfrt
    uint32_t next_vfrt_main_mem_addr;
    // Main Memory address where the WOF VFRT tables are located
    uint32_t vfrt_tbls_main_mem_addr;
    // The length of the WOF VFRT data in main memory
    uint32_t vfrt_tbls_len;
    // The state of the wof routine during initialization. states defined above
    uint8_t wof_init_state;
    // The address in shared OCC-PGPE SRAM of Quad State 0
    uint32_t quad_state_0_addr;
    // The address in shared OCC-PGPE SRAM of Quad State 1
    uint32_t quad_state_1_addr;
    // The address in shared OCC-PGPE SRAM of the Requested Active quads
    uint32_t req_active_quads_addr;
} amec_wof_t;

typedef struct
{
    // There is no guarantee that we can fit everything into the min BceRequest
    // size of 128 given that there may be a need to padding in the event the
    // Main Memory address is not 128-byte aligned. The data here is 256 to
    // ensure we have enough room for any and all padding that may be needed.
    uint8_t data[MIN_BCE_REQ_SIZE];
} temp_bce_request_buffer_t __attribute ((aligned(128)));


// Parameter structure used to pass information to the copy_vfrt_to_sram
// call back function.
typedef struct
{
    temp_bce_request_buffer_t * vfrt_table;
    uint8_t pad;
} copy_vfrt_to_sram_parms_t;

//******************************************************************************
// Function Prototypes
//******************************************************************************

void call_wof_main( void );

void wof_main( void );

uint16_t calculate_step_from_start( uint16_t i_ceff_vdx,
                                    uint8_t i_step_size,
                                    uint8_t i_min_step,
                                    uint8_t i_max_step );

uint8_t calc_quad_step_from_start( void );



uint32_t calc_vfrt_mainstore_addr( void );

void copy_vfrt_to_sram( copy_vfrt_to_sram_parms_t * i_parms );

void wof_vfrt_callback( void );

void send_vfrt_to_pgpe( uint32_t i_vfrt_address );

void read_shared_sram( void );

void calculate_core_voltage( void );

void calculate_core_leakage( void );

void calculate_nest_leakage( void );

void calculate_ceff_ratio_vdn( void );

void calculate_ceff_ratio_vdd( void );

inline void calculate_AC_currents( void );

inline bool core_powered_on( uint8_t i_core_num );

uint8_t num_cores_on_in_quad( uint8_t i_quad_num );

inline int32_t interpolate_linear( int32_t i_X,
                                   int32_t i_x1,
                                   int32_t i_x2,
                                   int32_t i_y1,
                                   int32_t i_y2 );

int32_t calculate_multiplier( int32_t i_temp );

uint32_t calculate_effective_capacitance( uint32_t i_iAC,
                                          uint32_t i_voltage,
                                          uint32_t i_frequency );


void read_sensor_data( void );

void disable_wof( void );

bool enable_wof( void );

void wof_control_callback( void );

void send_initial_vfrt_to_pgpe( void );

void read_req_active_quads( void );
#endif
