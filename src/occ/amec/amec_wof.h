/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_wof.h $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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

#ifndef _AMEC_WOF_H
#define _AMEC_WOF_H

//*************************************************************************
// Includes
//*************************************************************************
#include <sensor.h>
#include <occ_common.h>
#include <occ_sys_config.h> //MAX_NUM_CORES
#include <ssx_api.h> // SsxSemaphore
#include <pstates.h> // GlobalPstateTable

//*************************************************************************
// Externs
//*************************************************************************

//WOF parameters defined in amec_wof.c
extern sensor_t g_amec_wof_ceff_ratio_sensor;
extern sensor_t g_amec_wof_core_wake_sensor;
extern sensor_t g_amec_wof_vdd_sense_sensor;
extern uint8_t g_amec_wof_pstate_table_ready;
extern uint8_t G_wof_max_cores_per_chip;
extern uint16_t g_amec_wof_leak_overhead;

extern uint16_t G_amec_wof_vrm_eff_table[AMEC_WOF_VRM_EFF_TBL_ROWS][AMEC_WOF_VRM_EFF_TBL_CLMS];
extern uint16_t G_amec_wof_uplift_table[AMEC_WOF_UPLIFT_TBL_ROWS][AMEC_WOF_UPLIFT_TBL_CLMS];

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

#define AMEC_WOF_LOADLINE_ACTIVE 550  // Active loadline in micro ohms
#define AMEC_WOF_LOADLINE_PASSIVE 50  // Passive loadline in micro ohms

//OCI address
//PMC Winkle Interrupt Request Vector Register 3 (PMCWIRVR3)
#define PMCWIRVR3 0x40010458

//PMC Deep Exit Mask Register (PDEMR)
//The PDEMR register masks the ability for a given chiplet to exit from Deep
//Sleep or Winkle state. NOTE that this register is to be used by the OCC for
//implementing power shift algorithms
#define PDEMR     0x40010490

//Default number of iterations to compute Vdd regulator current out.
#define AMEC_WOF_VDD_ITER 4
//Buffer initial estimate and N+2 iterations for
//debugging/experimentation
//index 0: initial estimate     (+1 below)
//index 1-N+2: iteration output (+2 below)
#define AMEC_WOF_VDD_ITER_BUFF AMEC_WOF_VDD_ITER+1+2

#define AMEC_WOF_ERROR_THRESHOLD 20

typedef enum
{
    AMEC_WOF_ERROR_NONE,                     // 0
    AMEC_WOF_ERROR_SCOM_1,                   // 1
    AMEC_WOF_ERROR_SCOM_2,                   // 2
    AMEC_WOF_ERROR_SCOM_3,                   // 3
    AMEC_WOF_ERROR_SCOM_4,                   // 4
    AMEC_WOF_ERROR_SCOM_5,                   // 5
    AMEC_WOF_ERROR_CORE_COUNT,               // 6
    AMEC_WOF_ERROR_UNKNOWN_STATE,            // 7
    AMEC_WOF_ERROR_FREQ_VOTE_NOT_APPLIED,    // 8
    AMEC_WOF_ERROR_WOF_FREQ_NOT_APPLIED,     // 9
    AMEC_WOF_ERROR_PSTATETABLE_NOT_READY,    //10
    AMEC_WOF_ERROR_THRESHOLD_REACHED,        //11
    AMEC_WOF_ERROR_TRANSITION                //12
} AMEC_WOF_ERROR_ENUM;

typedef enum
{
    AMEC_WOF_INFO_NONE              = 0x00, // Not used
    AMEC_WOF_INFO_WAIT_FREQ_DATA    = 0x01,
    AMEC_WOF_INFO_WAIT_PSTATE_DATA  = 0x02,
    AMEC_WOF_INFO_WAIT_VRM_DATA     = 0x03,
    AMEC_WOF_INFO_WAIT_UPLIFT_DATA  = 0x04,
    AMEC_WOF_INFO_WAIT_FREQ_SAFE    = 0x05,
    AMEC_WOF_INFO_SET_ALG           = 0x10, // (last nibble is alg#)
    AMEC_WOF_INFO_PSTATE_CORES      = 0x20  // (last nibble is cores on)
} AMEC_WOF_INFO_ENUM;

typedef enum
{
    AMEC_WOF_NO_CORE_CHANGE,
    AMEC_WOF_CORE_CHANGE,
    AMEC_WOF_TRANSITION,
} AMEC_WOF_STATE_MACHINE;

//Definition of types of wof tables
typedef enum {
    AMEC_WOF_CORE_FREQ_TBL = 0x00,
    AMEC_WOF_VRM_EFF_TBL   = 0x01
} wof_tbl_type_t;

//*************************************************************************
// Structures
//*************************************************************************

//Structure used in g_amec
typedef struct amec_wof
{
    // Total loadline resistance in micro-ohm (R_ll + R_drop)
    uint16_t            loadline;
    // Vdd regulator efficiency in 0.01% units
    uint16_t            vdd_eff;
    // Chip Vdd current in 0.01 A (out of regulator)
    uint16_t            cur_out;
    // Last Vdd current accumulator to compute 2ms average
    uint32_t            cur_out_last;
    // Voltage at chip silicon (Vreg - V_loadline_droop)
    uint16_t            v_chip;
    // First index into iddq table for interpolation
    uint8_t             iddq_i;
    // First index into iddq table for interpolation
    uint8_t             iddq_i_core[MAX_NUM_CORES];
    // Check interpolation of iddq table
    uint16_t            iddq85c;
    // Check interpolation of iddq table
    uint16_t            iddq85c_core[MAX_NUM_CORES];
    // Estimated temperature-corrected leakage current in 0.01 Amps (whole chip)
    uint16_t            iddq;
    // Estimated temperature-corrected leakage current in 0.01 Amps (per core)
    uint16_t            iddq_core[MAX_NUM_CORES];
    // Estimated temperature-corrected leakage current in 0.01 Amps (old way)
    uint16_t            iddq_chip;
    // I_AC extracted in 0.01 Amps
    uint16_t            ac;
    // Effective capacitance for TDP workload @ Turbo in 0.005904 nF
    // Index = number_of_cores_on (e.g. 1-12 cores on is index 1-12)
    uint32_t            ceff_tdp[MAX_NUM_CORES+1];
    // Effective capacitance right now.
    uint32_t            ceff;
    // Effective capacitance old.
    uint32_t            ceff_old;
    // Voltage used in ceff calculation (units in 0.1 mV, like sensors)
    uint16_t            ceff_volt;
    // Effective capacitance ratio
    uint16_t            ceff_ratio;
    // Uplift frequency adjustment
    int16_t             f_uplift;
    // Frequency vote. Lowest vote, until WOF is initialized with safe Turbo freq.
    uint16_t            f_vote;
    // Voltage set at regulator associated with wof vote
    uint16_t            vote_vreg;
    // Voltage at chip associated with wof vote at present current.
    uint16_t            vote_vchip;
    // Non-zero is a WOF error flag
    uint32_t             error;
    // Consecutive errors counting toward threshold
    uint32_t             error_count_consecutive;
    // Maximum number of consecutive errors
    uint32_t            err_cnt_consec_max;
    // All WOF errors ever observed
    uint32_t            error_count_total;
    // Consecutive error count threshold before disabling WOF
    uint32_t             error_threshold;
    // WOF error history. First byte (MSB) is most recent error code.
    // 2nd byte is next most recent code, etc.
    uint64_t            error_history;
    // WOF error history (only non-zero error codes)
    uint64_t            error_history_nz;
    // Snapshot of error history for max consecutive error count.
    uint64_t            error_history_snap;
    // WOF info history
    uint64_t            info_history;
    // The WOF algorithm can be selected (and enabled/disabled) by setting
    // g_amec->wof.enable_parm (either automatically from frequency data
    // packet or manually from the Amester parameter interface).
    // OCC will check this against the current setting (g_amec->wof.algo_type)
    // and within 250us, do initialization for the next setting and start the
    // new WOF algorithm.
    uint8_t             enable_parm;
    // Current algorithm type: 0xFF=invalid (will cause init of enable_parm
    // setting)
    uint8_t             algo_type;
    // Count number of cores on
    uint8_t             cores_on;
    // WOF state
    uint8_t             state;
    // pmstate for debugging
    uint8_t             pm_state[MAX_NUM_CORES];
    // Bit mask of the sleeping cores that want to wake up
    uint64_t            wake_up_mask;
    // Copy of previous bit mask to detect flapping case
    uint64_t            wake_up_mask_prev;
    // Copy of previous non-zero bit mask for debugging
    uint64_t            wake_up_mask_save;
    // The current number of cores Pstate table allows
    uint8_t             pstatetable_cores_current;
    //The next pstate table max number of cores
    uint8_t             pstatetable_cores_next;
    //Estimated leakage current by core
    uint16_t            leakage[MAX_NUM_CORES]; // leakage current in 0.01 A
    //Vdd current out algorithm iterations
    uint8_t             vdd_iter;
    //Start tick for iout debugging info
    uint32_t            vdd_t1;
    //Input to vdd iout algorithm
    uint16_t            vdd_pin;
    //Input to vdd iout algorithm
    uint16_t            vdd_vset;
    //Upper voltage trend line
    int32_t            vdd_vhi;
    //Lower voltage trend line
    int32_t            vdd_vlo;
    //iout values for each iteration
    uint16_t            vdd_iouti[AMEC_WOF_VDD_ITER_BUFF];
    //Voltage sense for each iteration
    uint16_t            vdd_vouti[AMEC_WOF_VDD_ITER_BUFF];
    //overall efficiency for each iteration
    uint16_t            vdd_effi[AMEC_WOF_VDD_ITER_BUFF];
    //efficiency for higher voltage trend
    int32_t             vdd_effhii[AMEC_WOF_VDD_ITER_BUFF];
    //efficiency for lower voltage trend
    int32_t             vdd_effloi[AMEC_WOF_VDD_ITER_BUFF];
    //End tick for vdd iout debugging (t1==t2 means debug data is synchronized)
    uint32_t            vdd_t2;
    //Last scom error reading core voltage
    uint32_t            volt_err;
    //Total error count reading core voltage
    uint32_t            volt_err_cnt;
} amec_wof_t;

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************
void amec_wof_main(void);
void amec_wof_helper(void);
void amec_update_wof_sensors(void);
int  amec_wof_set_algorithm(const uint8_t i_algorithm);
void amec_wof_update_pstate_table(void);
void amec_wof_alg_v2(void);
void amec_wof_alg_v3(void);
int  amec_wof_common_steps(void);
void amec_wof_helper_v2(void);
void amec_wof_helper_v3(void);
uint32_t amec_wof_compute_ceff_tdp(uint8_t i_cores_on);

void amec_wof_writeToTable(wof_tbl_type_t i_tblType,
                          const uint16_t i_size,
                          const uint8_t i_clmnCount,
                          uint8_t *i_data_ptr);

void amec_wof_store_core_freq(const uint8_t i_max_good_cores,
                              const uint16_t i_size,
                              const uint8_t i_clmnCount,
                              uint8_t *i_data_ptr);

void amec_wof_store_vrm_eff( const uint16_t i_size,
                             const uint8_t i_clmnCount,
                             uint8_t *i_data_ptr);

void amec_wof_vdd_current_out(const uint16_t i_power_in,
                              const uint16_t i_v_set,
                              uint16_t *o_current_out,
                              uint16_t *o_v_sense);

void amec_wof_validate_input_data(void);

uint16_t amec_wof_get_max_freq(const uint8_t i_cores);

void amec_wof_calc_core_leakage(uint8_t i_core);

#endif
