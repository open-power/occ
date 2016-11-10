/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_wof.c $                                     */
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

//*************************************************************************/
// Includes
//*************************************************************************/
#include <occ_common.h>
#include <proc_data.h> // for CORE_PRESENT
#include <errl.h>      // Error logging
#include <sensor.h>
#include <amec_sys.h>
#include <cmdh_fsp_cmds_datacnfg.h>
#include <ssx_api.h>  // SsxSemaphore
#include <amec_wof.h>
#include <common.h>
#include <cmdh_fsp_cmds_datacnfg.h>
#include <amec_service_codes.h>
#include <rtls.h>     // for G_current_tick debugging

//*************************************************************************/
// Externs
//*************************************************************************/

extern amec_sys_t g_amec_sys;
extern data_cnfg_t * G_data_cnfg; // in cmdh_fsp_cmds_datacnfg

//*************************************************************************/
// Macros
//*************************************************************************/

//*************************************************************************/
// Defines/Enums
//*************************************************************************/

//*************************************************************************/
// Structures
//*************************************************************************/

//*************************************************************************/
// Globals
//*************************************************************************/

sensor_t g_amec_wof_ceff_ratio_sensor;
sensor_t g_amec_wof_core_wake_sensor;
sensor_t g_amec_wof_vdd_sense_sensor;

//Rows    - Output voltage in hundredths of a volt (i.e. 100 = 1V)
//Columns - Output current in AMPs.
uint16_t G_amec_wof_vrm_eff_table[AMEC_WOF_VRM_EFF_TBL_ROWS][AMEC_WOF_VRM_EFF_TBL_CLMS] = {{0},{0}};

//Rows    - is the AC reduction (current efficiency) in hundredths of a percent. (i.e 100 = 1%)
//Columns - is the number of active cores.
uint16_t G_amec_wof_uplift_table[AMEC_WOF_UPLIFT_TBL_ROWS][AMEC_WOF_UPLIFT_TBL_CLMS] = {{0},{0}};

// Core IDDQ voltages array from pstates.h (voltages in 100uV)
uint16_t G_iddq_voltages[CORE_IDDQ_MEASUREMENTS] = {8000, 9000, 10000, 11000, 12000, 12500};

uint8_t  G_wof_max_cores_per_chip = 0; // WOF does not run until this is set.

//Approximate y=1.3^((T-85)/10).
//Interpolate T in the table below to find m
// y ~= (T*m) >> 10     (shift out 10 bits)
// Error in estimation is no more than 0.9%.
uint16_t amec_wof_iddq_mult_table[][2] = {
    //Temperature in C, m
    {20, 186},
    {30, 242},
    {40, 314},
    {50, 409},
    {60, 531},
    {70, 691},
    {80, 898},
    {90, 1168}
};
#define AMEC_WOF_IDDQ_MULT_TABLE_N 8

//Algorithm 3 (aka WOF 2.1)
uint8_t g_amec_wof_pstate_table_ready = 0;

// Useful globals for debug
int32_t  g_amec_eff_vlow = 0;
int32_t  g_amec_eff_vhigh = 0;
uint32_t g_amec_wof_iout = 0;

// Chip Vdd leakage current as a percentage of total Vdd leakage current
// due to iVRM headers that cannot be turned off. Units are 0.1%
uint16_t g_amec_wof_leak_overhead = 50; // default is 5%

//*************************************************************************/
// Function Prototypes
//*************************************************************************/

//*************************************************************************/
// Functions
//*************************************************************************/

// Name: amec_wof_error_update
//
// Description:
// Track WOF error history
//
// Input: Next error code
// Output: update exported error history for debugging
void amec_wof_error_update(uint8_t i_errorcode)
{
    uint64_t history;
    static uint8_t last_err = AMEC_WOF_ERROR_NONE;

    // Update error code history, including success
    history = (g_amec->wof.error_history >> 8)
        | ((uint64_t)i_errorcode << 56);
    g_amec->wof.error_history = history;
    // Update non-success error history
    if (i_errorcode != AMEC_WOF_ERROR_NONE)
    {
        history = (g_amec->wof.error_history_nz >> 8)
            | ((uint64_t)i_errorcode << 56);
        g_amec->wof.error_history_nz = history;
        g_amec->wof.error_count_total++;

        // Update consecutive error count
        if (last_err == AMEC_WOF_ERROR_NONE)
            g_amec->wof.error_count_consecutive=1;
        else
            g_amec->wof.error_count_consecutive++;
        // Set maximum consecutive error count if greater than last
        if ( g_amec->wof.error_count_consecutive > g_amec->wof.err_cnt_consec_max )
        {
            // Snapshot the maximum consecutive error count and its history
            g_amec->wof.err_cnt_consec_max = g_amec->wof.error_count_consecutive;
            g_amec->wof.error_history_snap = g_amec->wof.error_history;
        }
    }
    else
    {
        // Reset error count
        g_amec->wof.error_count_consecutive = 0;
    }

    // Save the prior error
    last_err = g_amec->wof.error;
    // Save the current error code
    g_amec->wof.error = i_errorcode;

}

// Name: amec_wof_info_update
//
// Description:
// Track WOF info history
//
// Input: Next info code
// Output: update exported info history for debugging
void amec_wof_info_update(uint8_t i_code)
{
    uint64_t history;

    // Update error code history, including success
    history = (g_amec->wof.info_history >> 8)
        | ((uint64_t)i_code << 56);
    g_amec->wof.info_history = history;
}

// Function Specification
//
// Name: amec_wof_vdd_iout_vout
//
// Description:
// Calculate Vdd output current and voltage at Vdd remote sense.
//
// Inputs:
// i_power_in: Vdd input power 0.1 W units (deci-watts)
// i_v_set: Vdd setpoint voltage in 0.0001 V units
//
// Outputs:
// o_current_out: Vdd current out (0.01 A units)
// o_v_out: Voltage at Vdd remote sense (0.0001 V units)
//
// End Function Specification
void amec_wof_vdd_iout_vout(const uint16_t i_power_in,
                            const uint16_t i_v_set,
                            uint16_t *o_current_out,
                            uint16_t *o_v_out)
{
    uint8_t l_iteration;
    uint8_t i;

    // Convert table voltages from 0.01 V to 0.00001 V (*100)
    int32_t v_lo = G_amec_wof_vrm_eff_table[1][0] * 100; //low voltage in table
    int32_t v_hi = G_amec_wof_vrm_eff_table[2][0] * 100; //high voltage in table
    // v_diff = max voltage - min voltage from efficiency table
    int32_t v_diff = v_hi - v_lo;

    // helper variables
    int32_t l_x2minusx1, l_xminusx1;
    int32_t l_eff_vlow, l_eff_vhigh, l_v_offset;
    int32_t l_v_out;

    // Set initial guesses before iteration part of algorithm
    uint16_t l_eff = 8500; //initial guess efficiency=85.00%

    // Compute regulator output power in micro-W units.  out = in * efficiency
    //    power_in: min=0W max=300W = 3000 (0.1 W units)
    //    eff: min=0 max=10000=100% (.01% units)
    //    power_in*eff: max=3000(0.1W) * 10000(0.01%) = 30M (0.00001 W)
    //                  is under 2^25, fits in 25 bits
    //    *10 = 300M (0.000001W) fits in 29 bits of 32 bit reg.
    uint32_t l_pout = i_power_in * (uint32_t)l_eff * 10;

    // Compute current out of regulator in 0.01 A.
    // curr_out = power_out (*10 scaling factor) / voltage_out
    //    p_out: max=300M (0.000001 W) in 29 bits
    //    v_set: min=5000 (0.0001 V)  max=16000(0.0001 V) in 14 bits
    //     iout: max = 300M/5000 = 60000 (0.01 A), fits in 16 bits
    uint32_t l_iout = l_pout/i_v_set; //initial iout
    g_amec_wof_iout = l_iout; //for debugging

    // Initial guess for voltage sense. 0.0001 V units.
    //   V_chip = V_setpoint - I_chip * (R_loadline)
    //     V_delta = I_chip(0.01 A) * R_loadline(0.000001 ohm) => 0.00000001 V
    //     V_delta / 10000 => (in 0.0001 V)
    //   V_chip = V_reg - V_delta  => (in 0.0001 V)
    l_v_out = i_v_set - l_iout * AMEC_WOF_LOADLINE_ACTIVE / 10000;

    g_amec->wof.vdd_t1 = G_current_tick;
    // Save inputs for debugging
    g_amec->wof.vdd_pin = i_power_in;
    g_amec->wof.vdd_vset = i_v_set;
    g_amec->wof.vdd_vhi = v_hi;
    g_amec->wof.vdd_vlo = v_lo;
    // Save initial estimate
    g_amec->wof.vdd_iouti[0] = l_iout;
    g_amec->wof.vdd_vouti[0] = l_v_out;
    g_amec->wof.vdd_effi[0]  = l_eff;

    // Iterate to converge
    for(l_iteration=1; l_iteration<=g_amec->wof.vdd_iter; l_iteration++)
    {
        if (l_iout > G_amec_wof_vrm_eff_table[0][AMEC_WOF_VRM_EFF_TBL_CLMS-1])
        {
            // Clip to efficiency table maximum
            l_eff_vlow = G_amec_wof_vrm_eff_table[1][AMEC_WOF_VRM_EFF_TBL_CLMS-1];
            l_eff_vhigh = G_amec_wof_vrm_eff_table[2][AMEC_WOF_VRM_EFF_TBL_CLMS-1];
        }
        else if (l_iout < G_amec_wof_vrm_eff_table[0][1])
        {
            // Clip to efficiency table minimum
            l_eff_vlow = G_amec_wof_vrm_eff_table[1][1];
            l_eff_vhigh = G_amec_wof_vrm_eff_table[2][1];
        }
        else
        {
            // Interpolate in the table to find efficiency for min/max voltage
            for (i=1; i<AMEC_WOF_VRM_EFF_TBL_CLMS; i++)
            {
                if (l_iout >= G_amec_wof_vrm_eff_table[0][i] &&
                    l_iout <= G_amec_wof_vrm_eff_table[0][i+1])
                {
                    break;
                }
            }
            // if beyond table, use last 2 entries
            if (i >= AMEC_WOF_VRM_EFF_TBL_CLMS-1)
            {
                i = AMEC_WOF_VRM_EFF_TBL_CLMS - 2;
            }
            // i points to the first index
            // Compute efficiency at lower voltage (0.85 V)
            // Linear interpolate using the neighboring entries.
            // y = (x-x1)m+y1   m=(y2-y1)/(x2-x1)
            // x2minusx1 = difference in current between entries

            // x2minusx1 in units of 0.01 A
            l_x2minusx1 = ((int32_t)G_amec_wof_vrm_eff_table[0][i+1] -
                           (int32_t)G_amec_wof_vrm_eff_table[0][i]);
            // xminusx1 in units of 0.01 A
            l_xminusx1 = (int32_t)l_iout - (int32_t)G_amec_wof_vrm_eff_table[0][i];
            l_eff_vlow = l_xminusx1 * ((int32_t)G_amec_wof_vrm_eff_table[1][i+1] -
                                       (int32_t)G_amec_wof_vrm_eff_table[1][i]) /
                l_x2minusx1 + (int32_t)G_amec_wof_vrm_eff_table[1][i];

            // Reuse same x2minusx1 and xminusx1 because efficiency curves use
            // common current_in values
            l_eff_vhigh = l_xminusx1 * ((int32_t)G_amec_wof_vrm_eff_table[2][i+1] -
                                        (int32_t)G_amec_wof_vrm_eff_table[2][i]) /
                l_x2minusx1 + (int32_t)G_amec_wof_vrm_eff_table[2][i];
        }

        g_amec_eff_vlow = l_eff_vlow;//for debug
        g_amec_eff_vhigh = l_eff_vhigh;// for debug
        l_v_offset = l_v_out - v_lo;
        l_eff = (uint16_t) ((((int32_t)l_eff_vhigh - (int32_t)l_eff_vlow)
                             * (int32_t)l_v_offset)
                            / (int32_t)v_diff
                            + (int32_t)l_eff_vlow);

        // Estimate new iout,vout.
        // See comments above for unit conversion on pout,iout,vout
        l_pout = i_power_in * l_eff * 10;
        l_iout = l_pout/l_v_out;
        l_v_out = i_v_set - AMEC_WOF_LOADLINE_ACTIVE * l_iout / 10000;

        if (l_iteration < AMEC_WOF_VDD_ITER_BUFF)
        {
            g_amec->wof.vdd_iouti[l_iteration] = l_iout;
            g_amec->wof.vdd_vouti[l_iteration] = l_v_out;
            g_amec->wof.vdd_effi[l_iteration]  = l_eff;
            g_amec->wof.vdd_effhii[l_iteration] = l_eff_vhigh;
            g_amec->wof.vdd_effloi[l_iteration] = l_eff_vlow;
        }
    }
    // zero rest of debugging array
    for(;l_iteration < AMEC_WOF_VDD_ITER_BUFF; l_iteration++)
    {
        g_amec->wof.vdd_iouti[l_iteration] = 0;
        g_amec->wof.vdd_vouti[l_iteration] = 0;
        g_amec->wof.vdd_effi[l_iteration]  = 0;
        g_amec->wof.vdd_effhii[l_iteration] = 0;
        g_amec->wof.vdd_effloi[l_iteration] = 0;
    }

    g_amec->wof.vdd_eff = l_eff; //for debugging
    // If t1 == t2, then debug data between is synchronized
    g_amec->wof.vdd_t2 = G_current_tick;

    *o_v_out = l_v_out;
    *o_current_out = l_iout;
}

void amec_wof_validate_input_data(void)
{
    uint8_t             l_pstate = 0;
    uint8_t             i=0;

    // TODO: Need to add some sort of validation on the input data that is
    // consumed by WOF. For now, we are just tracing all this input data.

    // Operating frequencies
    TRAC_INFO("WOF: Frequencies @  NOMINAL[%d]  TURBO[%d]  UltraTURBO[%d]",
              G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL],
              G_sysConfigData.sys_mode_freq.table[OCC_MODE_STURBO],
              G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO]);

    // Check operating point at turbo
    TRAC_INFO("WOF Operating TURBO point: vdd_5mv[%d] Iddq_500ma[%d] frequency_mhz[%d]",
              G_sysConfigData.wof_parms.operating_points[TURBO].vdd_5mv,
              G_sysConfigData.wof_parms.operating_points[TURBO].idd_500ma,
              G_sysConfigData.wof_parms.operating_points[TURBO].frequency_mhz);

    // RDP to TDP conversion factor and Max number of cores for this chip
    TRAC_INFO("WOF tdp_rdp_factor[%d]     max_cores_per_chip[%d]",
              G_sysConfigData.wof_parms.tdp_rdp_factor,
              G_wof_max_cores_per_chip);

    // IDDQ table
    TRAC_INFO("WOF IDDQ Vdd current:  0.80V[%6d]  0.90V[%6d]  1.00V[%6d]",
              G_sysConfigData.iddq_table.iddq_vdd[0].fields.iddq_corrected_value,
              G_sysConfigData.iddq_table.iddq_vdd[1].fields.iddq_corrected_value,
              G_sysConfigData.iddq_table.iddq_vdd[2].fields.iddq_corrected_value);
    TRAC_INFO("WOF IDDQ Vdd current:  1.10V[%6d]  1.20V[%6d]  1.25V[%6d]",
              G_sysConfigData.iddq_table.iddq_vdd[3].fields.iddq_corrected_value,
              G_sysConfigData.iddq_table.iddq_vdd[4].fields.iddq_corrected_value,
              G_sysConfigData.iddq_table.iddq_vdd[5].fields.iddq_corrected_value);

    // Uplift table
    for (i=1; i < AMEC_WOF_UPLIFT_TBL_ROWS; i++)
    {
        // Print first 2 rows, a middle row, and last row.
        if( (i == 1) ||             //1st row
            (i == 2) ||             //2nd row
            (i == (AMEC_WOF_UPLIFT_TBL_ROWS/2)) ||     //Middle row
            (i ==  AMEC_WOF_UPLIFT_TBL_ROWS - 1))      //last row
        {
            TRAC_INFO("WOF Uplift Freqs: Row_%02i [%6d] 1core[%6d] 2core[%6d] 3core[%6d]",
                      i,
                      G_amec_wof_uplift_table[i][0],
                      G_amec_wof_uplift_table[i][1],
                      G_amec_wof_uplift_table[i][2],
                      G_amec_wof_uplift_table[i][3]);
            TRAC_INFO("WOF Uplift Freqs Row_%02i 12core[%6d]", i, G_amec_wof_uplift_table[i][12]);
        }
    }

    // VRM Efficiency Table
    TRAC_INFO("WOF VRM Effic: Current_10mA 1[%6d] 2[%6d] 3[%6d] ... 12[%6d] 13[%6d]",
              G_amec_wof_vrm_eff_table[0][1],
              G_amec_wof_vrm_eff_table[0][2],
              G_amec_wof_vrm_eff_table[0][3],
              G_amec_wof_vrm_eff_table[0][12],
              G_amec_wof_vrm_eff_table[0][13]);
    TRAC_INFO("WOF VRM Effic: Lo_volt[%6d] 20A[%6d] 40A[%6d] 60A[%6d] 260A[%6d]",
              G_amec_wof_vrm_eff_table[1][0],
              G_amec_wof_vrm_eff_table[1][1],
              G_amec_wof_vrm_eff_table[1][2],
              G_amec_wof_vrm_eff_table[1][3],
              G_amec_wof_vrm_eff_table[1][13]);
    TRAC_INFO("WOF VRM Effic: Hi_volt[%6d] 20A[%6d] 40A[%6d] 60A[%6d] 260A[%6d]",
              G_amec_wof_vrm_eff_table[2][0],
              G_amec_wof_vrm_eff_table[2][1],
              G_amec_wof_vrm_eff_table[2][2],
              G_amec_wof_vrm_eff_table[2][3],
              G_amec_wof_vrm_eff_table[2][13]);

    // VID Modification Table for 1 core, 10 cores and 12 cores
    l_pstate = G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_pstates;
    if (l_pstate > 3)
    {
        TRAC_INFO("WOF VID Mod Table: Num_pstates[%d] Max_num_cores[%d]",
                  G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_pstates,
                  G_sysConfigData.wof_parms.ut_vid_mod.ut_max_cores);
        for (i=0; i < NUM_ACTIVE_CORES; i++)
        {
            //Print data for 1, 10, and 12 cores)
            if( (i == 0) || (i == NUM_ACTIVE_CORES - 3) || (i == NUM_ACTIVE_CORES - 1))
            {
                TRAC_INFO("WOF Vdd VID Mod Table %2d_core: p0[0x%02X] p-1[0x%02X] p-2[0x%02X] p-3[0x%02X]",
                          i + 1,
                          G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[l_pstate][i],
                          G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[l_pstate-1][i],
                          G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[l_pstate-2][i],
                          G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[l_pstate-3][i]);
                TRAC_INFO("WOF Vdd VID Mod Table %2d_core: p-4[0x%02X]",
                          i+1,
                          G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[l_pstate-4][i]);
            }
        }
    }
}

uint32_t amec_wof_compute_ceff_tdp(uint8_t i_cores_on)
{
    // Example of calculation for Nominal operating point at 85C:
    // Estimate IDDQ@RDP(95C)@Vnom P0:
    // IDDQ@85C * 1.25 ^ ([95-85]/10) = 35.84 * 1.25 = 44.8 A (Leakage current)
    //
    // NM_Idd@RDP P0 = 151 A
    //
    // P0: (151 A - 44.8 A) / 1.22 * 1.12 = 97.50 A @TDP
    //
    // C_eff    = I / (V^1.3 * F)
    //
    //   I is AC component of Idd in 0.01 Amps (or 10 mA)
    //   V is the silicon voltage from the operating point data in 100 uV
    //   F is Turbo frequency in MHz
    //   C_eff is in nF
    //
    // C_eff_tdp_P0 = 97.50 A / (0.9 V ^1.3) / 3226 MHz = 0.0347 microF = 34.7 nF
    //
    // C_EFF_TDP_P0 = 9750 (0.01A) * 16384 / 9000(100uV)^1.3 * 16384 / 3226(MHz)
    //                = 5870 (0.005904 nF)
    //
    // Note: C_EFF_TDP_P0 / 100 / 16384 / 16384 * 10000^1.3 * 1000
    //       = C_eff_tdp_P0 in nF
    //

    iddq_entry_t        l_i_leak = 0;
    uint8_t             i = 0;
    uint32_t            l_curr_diff = 0;
    uint32_t            l_v_chip = 0;
    uint32_t            l_temp = 0;
    uint32_t            l_temp_curr = 0;
    uint32_t            l_c_eff_tdp = 0;

    // STEP 1: Acquire the silicon voltage from the operating point data and
    // convert it to 100uV units
    l_v_chip = G_sysConfigData.wof_parms.operating_points[TURBO].vdd_5mv * 5 * 10;

    // STEP 2: Search through the iddq_vdd array for the closest value to our
    // silicon voltage and interpolate to compute the leakage current
    if (l_v_chip < G_iddq_voltages[0])
    {
        // Voltage is lower than any element in the array, so use first two
        // entries
        i = 0;
    }
    else
    {
        for (i=0; i<CORE_IDDQ_MEASUREMENTS-1; i++)
        {
            if (l_v_chip >= G_iddq_voltages[i] &&
                l_v_chip <= G_iddq_voltages[i+1])
            {
                break;
            }
        }
    }
    if (i >= CORE_IDDQ_MEASUREMENTS-1)
    {
        // Voltage is higher than the table, so use last two entries
        i = CORE_IDDQ_MEASUREMENTS - 2;
    }

    // Do linear interpolation using the neighboring entries:
    // Y = m*(X - x1) + y1, where m = (y2-y1) / (x2-x1)
    l_i_leak = (l_v_chip - G_iddq_voltages[i]) *
        ((int32_t)G_sysConfigData.iddq_table.iddq_vdd[i+1].fields.iddq_corrected_value -
         (int32_t)G_sysConfigData.iddq_table.iddq_vdd[i].fields.iddq_corrected_value) /
        ((int32_t)G_iddq_voltages[i+1] - (int32_t)G_iddq_voltages[i]) +
        G_sysConfigData.iddq_table.iddq_vdd[i].fields.iddq_corrected_value;

    // STEP 3: Correct the leakage current computed in the previous step for
    // temperature by multiplying by 1.25 (due to 10 C difference between IDDQ
    // VPD operationg point data).
    l_i_leak = l_i_leak * 125 / 100;

    // STEP 4: Compute current differential in 10mA units at RDP
    l_curr_diff = (G_sysConfigData.wof_parms.operating_points[TURBO].idd_500ma *
                   500 / 10) - l_i_leak;

    // STEP 5: Translate to TDP using factor sent in Pstate superstructure
    l_curr_diff = l_curr_diff * G_sysConfigData.wof_parms.tdp_rdp_factor / 10000;

    // STEP 6: Compute V^1.3 using a best-fit equation:
    // Y = 21374 * (X in 100 uV) - 50615296
    l_temp = (21374 * l_v_chip - 50615296) >> 10;

    // STEP 7: Compute I / (V^1.3)
    l_temp_curr = l_curr_diff << 14; // *16384
    l_temp = l_temp_curr / l_temp;
    l_temp = l_temp << 14; // *16384

    // STEP 8: Divide by turbo frequency I / (V^1.3) / F
    l_c_eff_tdp = l_temp /
        G_sysConfigData.wof_parms.operating_points[TURBO].frequency_mhz;

    // STEP 9: Scale for number of cores on
    l_c_eff_tdp = l_c_eff_tdp * i_cores_on / G_wof_max_cores_per_chip;

    return l_c_eff_tdp;
}

// Function Specification
//
// Name: amec_wof_set_algorithm
//
// Description:
// WOF starts with frequency vote of zero (static initialization).
// This ensures that very first vote into voting box is safe (minimum
// speed) until WOF is initialized with this function.
//
// amec_wof_set_algorithm() is called inside the periodic WOF algorithm.
// This is done so that WOF can be turned on/off dynamically in a safe manner.
//
// Return 0 = no error.
//        1 = not initialized.
//
// Thread: RealTime Loop
//
// End Function Specification
int amec_wof_set_algorithm(const uint8_t i_algorithm)
{
    uint32_t            l_data32 = 0;
    int                 l_rc = 1;
    int                 l_scom_rc = 0;
    errlHndl_t          l_err = NULL;
    uint8_t             i;

    do
    {
        // Start WOF in safe state.

        // Check required configuration data is loaded
        if (!(G_data_cnfg->data_mask | DATA_MASK_FREQ_PRESENT))
        {
            // Frequency table is not loaded, don't know safe turbo for WOF.
            // Initialization not yet possible. Try again later.
            amec_wof_info_update(AMEC_WOF_INFO_WAIT_FREQ_DATA);
            break;
        }
        if (!(G_data_cnfg->data_mask | DATA_MASK_PSTATE_SUPERSTRUCTURE))
        {
            // G_sysConfigData.iddq_table not loaded
            // G_sysConfigData.wof_parms not loaded
            amec_wof_info_update(AMEC_WOF_INFO_WAIT_PSTATE_DATA);
            break;
        }
        if (!(G_data_cnfg->data_mask | DATA_MASK_WOF_VRM_EFF))
        {
            // VRM Efficiency table not loaded.
            // Cannot compute Vdd current out.
            amec_wof_info_update(AMEC_WOF_INFO_WAIT_VRM_DATA);
            break;
        }
        if (!(G_data_cnfg->data_mask | DATA_MASK_WOF_CORE_FREQ))
        {
            // WOF uplift table not loaded
            // WOF algorithm cannot run
            amec_wof_info_update(AMEC_WOF_INFO_WAIT_UPLIFT_DATA);
            break;
        }

        // Vote for a safe frequency (turbo frequency)
        // Check the UltraTurbo frequency to see if WOF function is supported
        if(G_sysConfigData.sys_mode_freq.table[OCC_MODE_STURBO] == 0)
        {
            // UltraTurbo frequency is zero, WOF is not supported
            // Set WOF to safe turbo vote, since WOF vote is always active, even when WOF off.
            g_amec->wof.f_vote = G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO];
        }
        else
        {
            // Initialize WOF frequency request to be turbo.
            // Note: Ultraturbo frequency is stored in OCC_MODE_TURBO
            g_amec->wof.f_vote = G_sysConfigData.sys_mode_freq.table[OCC_MODE_STURBO];
        }

        // Make sure the vote is enforced before continuing.
        // If vote is not enforced, then exit and return again in next 250us
        // interval.
        if (g_amec->proc[0].core_max_freq > g_amec->wof.f_vote)
        {
            amec_wof_info_update(AMEC_WOF_INFO_WAIT_FREQ_SAFE);

            // Set to an invalid algorithm so that no other algorithm
            // is run and signal WOF should try again to transition
            // to the new algorithm.
            g_amec->wof.algo_type = 0xFF;

            // Initialization not finished. Try again later.
            break;
        }

        // At this point, all cores are at Turbo or lower.
        // We may turn off the inhibit-wake signal.

        // Validate input data to the WOF algorithm
        amec_wof_validate_input_data();

        // Calculate ceff_tdp from static data in the Pstate SuperStructure
        for (i=1; i<=G_wof_max_cores_per_chip; i++)
            g_amec->wof.ceff_tdp[i] = amec_wof_compute_ceff_tdp(i);

        switch(i_algorithm)
        {
            case 3:
                // FALL-THROUGH

            case 2: // WOF algorithm 2
                g_amec->wof.state = AMEC_WOF_NO_CORE_CHANGE; // WOF state
                // Set pstate table to number of good cores (all on);
                // Conservatively assume all cores are on for next interval
                g_amec->wof.pstatetable_cores_current = G_wof_max_cores_per_chip;
                g_amec->wof.pstatetable_cores_next = G_wof_max_cores_per_chip;

                // Inhibit cores from waking up
                l_data32 = 0xffff0000ull;
                out32(PDEMR, l_data32);
                break;
            case 0: // No WOF
                // Do not inhibit core wakeup anymore (in case of transitioning
                // out of algorithm 2)

                l_data32 = 0x00000000ull;
                out32(PDEMR, l_data32);

                // Restore original VID codes for turbo to ultraturbo Pstates
                // (in case of transitiong out of algorithm 3)
                g_amec->wof.pstatetable_cores_next = G_wof_max_cores_per_chip;

                amec_wof_update_pstate_table();
                g_amec->wof.pstatetable_cores_current = G_wof_max_cores_per_chip;
                TRAC_INFO("Turn WOF off");
                break;
        }

        // Success, set the new algorithm
        g_amec->wof.algo_type = g_amec->wof.enable_parm;
        amec_wof_error_update(AMEC_WOF_ERROR_NONE);
        l_rc = 0;
        TRAC_INFO("WOF algorithm has been successfully initialized: algo_type[%d] C_eff_tdp[%d]",
                  g_amec->wof.algo_type,
                  g_amec->wof.ceff_tdp[G_wof_max_cores_per_chip]);
        amec_wof_info_update(AMEC_WOF_INFO_SET_ALG | g_amec->wof.algo_type);

    } while (0);

    return l_rc;
}


// Function Specification
//
// Name: amec_update_wof_sensors
//
// Description: Runs when WOF is enabled and disabled.
//   Compute CUR250USVDD0 (current out of Vdd regulator)
//   Compute voltage at chip input
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_wof_sensors(void)
{
    uint16_t            l_pow_reg_input_dW;
    uint16_t            l_vdd_reg;
    uint16_t            l_curr_output;
    uint32_t            l_v_delta;
    uint32_t            l_v_chip;
    uint16_t            l_v_sense;

    if (!(G_data_cnfg->data_mask | DATA_MASK_WOF_VRM_EFF))
    {
        // VRM Efficiency table not loaded yet.
        // Cannot compute Vdd current out.
        return;
    }

    // Vdd regulator input power in 0.1 W units (deci-Watts)
    l_pow_reg_input_dW = AMECSENSOR_PTR(PWR250USVDD0)->sample * 10;
    // Vdd regulator VID setting in 0.0001 V units
    l_vdd_reg = AMECSENSOR_PTR(VOLT250USP0V0)->sample;

    amec_wof_vdd_iout_vout(l_pow_reg_input_dW,
                           l_vdd_reg,
                           &l_curr_output,
                           &l_v_sense);
    // Save Vdd current output
    sensor_update(AMECSENSOR_PTR(CUR250USVDD0), l_curr_output);
    // Save Vsense estimate for WOF validation
    sensor_update(AMECSENSOR_PTR(WOF250USVDDS), l_v_sense);

    // Compute Vdd voltage at chip input
    //   V_chip = V_reg - I_chip * (R_loadline)
    //     V_delta = I_chip(0.01 A) * R_loadline(0.000001 ohm) => 0.00000001 V
    //     V_delta / 10000 => (in 0.0001 V)
    //   V_chip = V_reg - V_delta  => (in 0.0001 V)
    l_v_delta = (uint32_t) l_curr_output * (uint32_t) g_amec->wof.loadline
        / (uint32_t)10000;
    l_v_chip = l_vdd_reg - l_v_delta;
    g_amec->wof.v_chip = l_v_chip; // expose in parameter
}

// Compute Ceff Ratio
// Return non-zero on error
int amec_wof_common_steps(void)
{
    uint8_t             i;
    uint32_t            l_result32; //temporary result
    uint32_t            l_result32v;
    uint32_t            l_result32i;
    uint8_t             l_cores_on = 0;
    uint8_t             l_cores_waking = 0;
    uint8_t             l_pstatetable_cores_next=0;
    uint32_t            l_data32 = 0;
    uint32_t            l_accum = 0;  //Vdd current accumulator
    uint16_t            l_ceff_ratio; //Effective switching capacitance ratio
    uint32_t            l_leakage = 0; // Total chip leakage current 0.01 A
    uint32_t            l_volt_sum = 0; // Total active core voltage
    uint8_t             l_volt_count = 0; // Num cores for Ceff voltage calc.
    uint16_t            l_volt_avg = 0; // Avg active core voltage
    uint8_t             l_hwid = 0; // HW core ID
    uint64_t            l_core_mask = 0; // For accessing Winkle interrupt mask

    // Compute sensors that should be averaged over the WOF time period
    // Avg voltage per core every 2ms

    //If no change, then nothing to do here.
    if (g_amec->wof.state != AMEC_WOF_NO_CORE_CHANGE)
    {
        // Not an error. WOF is transitioning cores. Try later.
        return 0;
    }
    if (g_amec->wof.pstatetable_cores_next !=
        g_amec->wof.pstatetable_cores_current)
    {
        amec_wof_error_update(AMEC_WOF_ERROR_CORE_COUNT);
        TRAC_ERR("amec_wof_common_steps: Invalid core count. "
                 "CoresNext:%i, CoresCurrent:%i.  wof.error:0x%X",
                 g_amec->wof.pstatetable_cores_next,
                 g_amec->wof.pstatetable_cores_current, g_amec->wof.error);
        return 1; //error
    }

    // Acquire important sensor data
    l_accum = (uint32_t)AMECSENSOR_PTR(CUR250USVDD0)->accumulator;

    g_amec->wof.wake_up_mask_prev = g_amec->wof.wake_up_mask;
    l_data32 = in32(PMCWIRVR3);
    g_amec->wof.wake_up_mask = (((uint64_t)l_data32) << 32);

    // In future processor generation when interlock on deep-sleep is added:
    // Need to do a getscom of register PMCSIRV3 to find the deep sleep cores
    // that want to wake up. Then OR into wake_up_mask.

    // Save non-zero wake up mask for debugging
    if (g_amec->wof.wake_up_mask != 0)
    {
        g_amec->wof.wake_up_mask_save = g_amec->wof.wake_up_mask;
    }

    // Count number of cores that are turned on for
    // 1) Determining pstate voltage selection
    // 2) Determining Ceff voltage
    //
    // IMPORTANT:
    //
    // The algorithm uses PM state and the winkle exit vector to
    // determine the sum of cores turned on for pstate voltage
    // selection and also for Ceff leakage power calculation.  The
    // counts can be different, since deep-sleep is counted for pstate
    // voltage selection, since exit is not gated by WOF.
    //
    // Furthermore, the PM state may be stale compared to the winkle
    // exit vector, since it was previously collected by a GPE task.
    // This means that when a core exits winkle, it will take an
    // additional WOF cycle for the PM state to show the core is
    // running. We save the prior winkle exit vector to catch this
    // case and mark the core as counted for purposes of setting the
    // pstate voltage.
    //
    // The core voltage and PM state may be up to 2 ms stale, which
    // implies momentary inaccuracy in the Ceff calculation as the
    // core changes pstate or PM state. This is safe for the WOF
    // frequency selection, since a safe frequency and voltage range
    // is set by the number of active cores in conjunction with the
    // winkle exit vector. Cores used in Ceff calc are marked l_volt=1.

    for (i=0; i<MAX_NUM_CORES; i++)
    {
        // Categorize core state
        uint8_t l_on = 0;     // use to determine pstate voltage
        uint8_t l_waking = 0; // in winkle exit vector
        uint8_t l_volt = 0;   // use for ceff voltage calculation

        if (!CORE_PRESENT(i))
        {
            continue;
        }

        l_hwid = CORE_OCC2HW(i);
        l_core_mask = 0x8000000000000000 >> l_hwid;

        if ((g_amec->wof.wake_up_mask & l_core_mask) != 0)
        {
            // Core exiting winkle
            l_waking = 1;
        }

        switch(g_amec_sys.proc[0].core[i].pm_state_hist >> 5)
        {
            case 5: //deep sleep
                // Deep sleep does not have OCC interlock on POWER8/8+.
                // Therefore, core is considered on.
                l_on = 1;
                l_volt = 1;
                break;

            case 7: //deep winkle
                if ((g_amec->wof.wake_up_mask_prev & l_core_mask) != 0)
                {
                    // Core exited winkle last time.
                    // The PM state is stale and should be ignored.
                    // Conservatively assume core is on to
                    // avoid miscount and "flapping" to pstate voltage.
                    l_on = 1;
                }
                break;

            default:
                l_on = 1;
                l_volt = 1;
                break;
        }

        if (l_volt)
        {
            l_volt_sum += AMECSENSOR_ARRAY_PTR(VOLT2MSP0C0,i)->sample;
            l_volt_count++;
        }

        // Count cores as "on" or "waking", but not both.
        if (l_waking)
        {
            // don't care about PM state, since it may be stale.
            l_cores_waking++;
        }
        else if (l_on)
        {
            l_cores_on++;
        }

    }

    if (l_volt_count) l_volt_avg = l_volt_sum / l_volt_count;
    g_amec->wof.cores_on = l_cores_on; // externalize

    sensor_update(AMECSENSOR_PTR(WOFCOREWAKE), l_cores_waking);

    // Set the number of cores will be on for the next round.
    l_pstatetable_cores_next = l_cores_on + l_cores_waking;
    // Save number of cores OCC is transitioning toward for other WOF states
    g_amec->wof.pstatetable_cores_next = l_pstatetable_cores_next;

    // Step 2: Determine leakage current

    // Sum per-core leakage to get total chip leakage
    for (i=0; i<MAX_NUM_CORES; i++)
    {
        if (CORE_PRESENT(i)) {
            l_leakage += g_amec->wof.iddq_core[i];
        }
    }
    // Note: Divide by good cores on chip, since iddq_core represents full
    // chip current.  Divide here, not in per-core calc, for accuracy.
    g_amec->wof.iddq = l_leakage / G_wof_max_cores_per_chip;


    // Step 3: Compute AC portion of chip Vdd current (units of 0.01 A)

    // Compute 2ms current average
    // Divide the 250us accumulator by 8 samples to get 2ms average
    g_amec->wof.cur_out = (l_accum - g_amec->wof.cur_out_last) >> 3; // 0.01 A
    g_amec->wof.cur_out_last = l_accum;

    g_amec->wof.ac = g_amec->wof.cur_out - g_amec->wof.iddq;

    // Step 4: Compute ratio of workload C_eff to TDP C_eff.
    l_result32i = g_amec->wof.ac << 14; // * 16384
    g_amec->wof.ceff_volt = l_volt_avg; // externalize
    // Estimate voltage^1.3 using equation:
    // = 21374 * (X in 0.1 mV) - 50615296
    l_result32v = (21374 * l_volt_avg - 50615296) >> 10;
    l_result32 = l_result32i / l_result32v;
    l_result32 = l_result32 << 14; // * 16384
    if (g_amec->wof.f_vote != 0)
    {
        // avoid divide by 0
        l_result32 = l_result32 / (uint32_t) g_amec->wof.f_vote;
    }

    g_amec->wof.ceff_old = l_result32;

    l_ceff_ratio = g_amec->wof.ceff_old * 10000
        / g_amec->wof.ceff_tdp[l_cores_on];
    g_amec->wof.ceff_ratio = l_ceff_ratio; // externalize
    sensor_update(AMECSENSOR_PTR(WOFCEFFRATIO),l_ceff_ratio);

    // Step 5: frequency uplift table
    // Clip ratio to 100% to fit in table bounds.
    if (l_ceff_ratio > 10000) l_ceff_ratio = 10000;
    // Search table and point i to the lower entry the target value falls
    // between.
    for (i=1; i<AMEC_WOF_UPLIFT_TBL_ROWS-1; i++)
    {
        if (G_amec_wof_uplift_table[i][0] <= (int16_t) l_ceff_ratio &&
            G_amec_wof_uplift_table[i+1][0] >= (int16_t) l_ceff_ratio)
        {
            break;
        }
    }

    if (i >= AMEC_WOF_UPLIFT_TBL_ROWS - 1)
    {
        // Out of table, so clip to 100%
        l_result32 = G_amec_wof_uplift_table[AMEC_WOF_UPLIFT_TBL_ROWS-1][l_pstatetable_cores_next+1];
    }
    else
    {
        // Ratio is within uplift table
        // Linear interpolate using the neighboring entries:
        // y = m(x-x1)+y1   m=(y2-y1)/(x2-x1)
        l_result32 =
            ((int32_t)l_ceff_ratio - (int32_t)G_amec_wof_uplift_table[i][0])
            * ((int32_t)G_amec_wof_uplift_table[i+1][l_pstatetable_cores_next] -
               (int32_t)G_amec_wof_uplift_table[i][l_pstatetable_cores_next])
            / ((int32_t)G_amec_wof_uplift_table[i+1][0] - (int32_t)G_amec_wof_uplift_table[i][0])
            + (int32_t)G_amec_wof_uplift_table[i][l_pstatetable_cores_next];
    }

    g_amec->wof.f_uplift = l_result32;
    amec_wof_error_update(AMEC_WOF_ERROR_NONE); // clear previous errors
    return 0; // no error
}

// Function Specification
//
// Name: amec_wof_v2
//
// Description: Run WOF version 2. Core-count aware.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_wof_alg_v2(void)
{
    int err;

    err = amec_wof_common_steps();
    if (err) return;

    g_amec->wof.f_vote = g_amec->wof.f_uplift;

    if (g_amec->wof.pstatetable_cores_next !=
        g_amec->wof.pstatetable_cores_current)
    {
        g_amec->wof.state = AMEC_WOF_CORE_CHANGE;
    }
}

// Function Specification
//
// Name: amec_wof_update_pstate_table
//
// Description: This function updates the Pstate table with VID values stored
// in VIDModification Table and based on the number of cores that are awake.
//
// End Function Specification
void amec_wof_update_pstate_table(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    Pstate                      l_pmin = 0;
    Pstate                      l_pmax = 0;
    uint16_t                    i = 0;
    uint8_t                     k = 0;
    uint8_t                     l_index = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // Extract the number of active cores, this will be the second index into
    // the VIDModificationTable array
    k = g_amec->wof.pstatetable_cores_next - 1;

    // STEP 1:
    // Prevent any Pstate changes by locking the PMC Rail to the min
    // Determine minimum pstate (either min+1 from pstate table, or configured min from TMGT)
    l_pmin = gpst_pmin(&G_global_pstate_table) + 1;
    const int8_t l_config_min_pstate = (int8_t)proc_freq2pstate(G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY]);
    if (l_config_min_pstate > l_pmin)
    {
        // use larger min pstate (from TMGT config)
        l_pmin = l_config_min_pstate;
    }

    // Set the Pmax_rail register via OCI write
    amec_oversub_pmax_clip(l_pmin);

    // STEP 2:
    // Update required entries of the Global Pstate table using Vid codes
    // provided in the VIDModification Table.
    // Remark: Note that the parameter ut_segment_pstates represents the
    // number of Pstate segments that need to be modified. Need to add +1
    // to reflect the actual number of Pstates that need to be modified.
    for (i=0; i<G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_pstates+1; i++)
    {
        // Determine the index into the Pstate table entry that needs to be
        // updated
        l_index = (G_global_pstate_table.entries-1) +
            G_global_pstate_table.turbo_ps + i;

        // Modify the field associated with Vdd.
        G_global_pstate_table.pstate[l_index].fields.evid_vdd =
            G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[i][k];

        // Modify the field associated with Vcs.
        G_global_pstate_table.pstate[l_index].fields.evid_vcs =
            G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vcs_vid[i][k];

        // Compute the ECC for this entry
        G_global_pstate_table.pstate[l_index].fields.ecc =
            gpstCheckByte(G_global_pstate_table.pstate[l_index].value);
    }

    // STEP 3:
    // Release the lock on the PMC Rail from Step 1
    l_pmax = gpst_pmax(&G_global_pstate_table);

    // Set the Pmax_rail register via OCI write
    amec_oversub_pmax_clip(l_pmax);

    // Signal that a new Pstate table is ready
    g_amec_wof_pstate_table_ready = 1;
}


// Function Specification
//
// Name: amec_wof_alg_v3
//
// Description: Run WOF algorithm version 3. Core-count aware. Re-calculate
// load-line. Update Pstate table at runtime.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_wof_alg_v3(void)
{
    int err;

    err = amec_wof_common_steps();
    if (err) return;

    // Determine if we have cores that are turning on or off
    if (g_amec->wof.pstatetable_cores_next !=
        g_amec->wof.pstatetable_cores_current)
    {
        amec_wof_info_update(AMEC_WOF_INFO_PSTATE_CORES | g_amec->wof.pstatetable_cores_next);

        // Cores are turning on or off.  Optimize Pstate voltage for new core count.
        // Move frequency to safe region so turbo range voltage can be optimized.
        // Go to Turbo - 1 MHz so all cores go to Pstate below Turbo range.
        // (Cores will be in pstate range that is not being modified.)
        g_amec->wof.f_vote =
            G_sysConfigData.sys_mode_freq.table[OCC_MODE_STURBO] - 1;

        g_amec->wof.state = AMEC_WOF_CORE_CHANGE;
    }
    else if (g_amec->wof.wake_up_mask != 0)
    {
        //Even though total number of cores active, if a core is waking
        //the wake inhibit mask must be toggled
        g_amec->wof.state = AMEC_WOF_TRANSITION;
        g_amec->wof.f_vote = g_amec->wof.f_uplift;
    }
    else
    {
        // No change in total number of cores AND no cores waking.
        // Just adjust WOF frequency
        g_amec->wof.f_vote = g_amec->wof.f_uplift;
    }
}

// Function Specification
//
// Name: amec_wof_helper_v3
//
// Description: Run this in every 250us state. Allows cores to
// wake-up as soon as WOF outcome is ready.
//
// Error handling: On any error that delays WOF state transition,
//   wof_error must be set non-zero so that failure to progress
//   back to AMEC_WOF_NO_CORE_CHANGE will be noticed after a
//   few internvals and WOF can be disabled.
//
// Thread: RealTime Loop
//
// Assumptions:
//   Frequency voting box is run immediately before this routine
//   to set the new frequency request, so WOF has up-to-date
//   information.
//
// End Function Specification
void amec_wof_helper_v3(void)
{
    uint32_t            l_data32 = 0;
    static uint8_t      l_consecutive_freq_error = 0;

    switch (g_amec->wof.state)
    {
        case AMEC_WOF_NO_CORE_CHANGE:
        {
            // Apply the WOF vote in 2ms WOF procedure.  Assume Pstate tables
            // are already set up
            g_amec_wof_pstate_table_ready = 0;
            break;
        }

        case AMEC_WOF_CORE_CHANGE:
        {
            // Check if current frequency clips are above WOF frequency
            if (g_amec->proc[0].core_max_freq_actual > g_amec->wof.f_vote)
            {
                // Expect to hit this once every time we update pstate table,
                // because it takes 1 tick to move frequency out of turbo range.
                // Do not log first violation to converse log space.
                if(l_consecutive_freq_error != 0)
                {
                    amec_wof_error_update(AMEC_WOF_ERROR_WOF_FREQ_NOT_APPLIED);
                    TRAC_INFO("amec_wof_helper_v3: actual frequency clip beyond wof vote."
                                  "core_max_freq_actual=%i, wof_f_vote=%i, wof.error:0x%X",
                                  g_amec->proc[0].core_max_freq_actual,
                                  g_amec->wof.f_vote, g_amec->wof.error);
                }
                l_consecutive_freq_error++;
                break;
            }
            // Mark that we got past the frequency check
            l_consecutive_freq_error = 0;

            // Check if new requested frequency clips are above WOF frequency
            if (g_amec->proc[0].core_max_freq > g_amec->wof.f_vote)
            {
                amec_wof_error_update(AMEC_WOF_ERROR_FREQ_VOTE_NOT_APPLIED);
                TRAC_INFO("amec_wof_helper_v3: wof vote not applied."
                         "core_max_freq=%i, wof_f_vote=%i, wof.error:0x%X",
                         g_amec->proc[0].core_max_freq,
                         g_amec->wof.f_vote, g_amec->wof.error);
                break;
            }
            // At this point, all core frequency must be at or below
            // WOF frequency which has been set below the pstate table
            // changes.

            amec_wof_update_pstate_table();

            // Wait until Pstate table is ready
            if (g_amec_wof_pstate_table_ready == 0)
            {
                amec_wof_error_update(AMEC_WOF_ERROR_PSTATETABLE_NOT_READY);
                TRAC_ERR("amec_wof_helper_v3: pstate table not ready"
                         "pstate_table_ready=%i, wof.error:0x%X",
                         g_amec_wof_pstate_table_ready, g_amec->wof.error);
                break;
            }

            // If we reach this point, then we can be assured that
            // 1) GPE has applied the latest frequency selection, including WOF vote
            // 2) The true frequency is not above the WOF frequency
            // 3) The p-state table is ready

            // Now that new Pstate table is installed, we can allow WOF
            // frequency request
            g_amec->wof.f_vote = g_amec->wof.f_uplift;

            g_amec->wof.state = AMEC_WOF_TRANSITION;

            // FALL THROUGH
        }

        case AMEC_WOF_TRANSITION:
        {
            // NOTE: This case stands alone because it is sometimes called
            // without going through the AMEC_WOF_UPDATE_PSTATES case.

            // Signal any waking cores to turn on and go back to initial state.

            // Quickly toggle waking core inhibit bits. This will allow cores
            // that want to wake up to actually wake up.
            l_data32 = (g_amec->wof.wake_up_mask >> 32);
            out32(PDEMR, ~l_data32);

            // Now, go back to inhibiting all cores
            l_data32 = 0xffff0000ull;
            out32(PDEMR, l_data32);

            g_amec->wof.pstatetable_cores_current =
                g_amec->wof.pstatetable_cores_next;
            g_amec->wof.state = AMEC_WOF_NO_CORE_CHANGE;

            break;
        }

        default:
            amec_wof_error_update(AMEC_WOF_ERROR_UNKNOWN_STATE);
            TRAC_ERR("amec_wof_helper_v3: WOF is in an unknown state: 0x%X. "
                     " wof.error:0x%X", g_amec->wof.state, g_amec->wof.error);
            break;
    }
}

// Function Specification
//
// Name: amec_wof_helper_v2
//
// Description: Run this in every 250us state. Allows cores to
// wake-up as soon as WOF outcome is ready.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_wof_helper_v2(void)
{
    uint32_t            l_data32 = 0;

    switch (g_amec->wof.state)
    {
        case AMEC_WOF_NO_CORE_CHANGE:
        {
            // Apply the WOF vote in 2ms WOF procedure.  Assume p-state tables
            // are already set up
            break;
        }

        case AMEC_WOF_CORE_CHANGE:
        {
            // We need to be certain that the GPE has applied the last frequency
            // request from the voting box: (actual != request)
            if (g_amec->proc[0].core_max_freq_actual != g_amec->proc[0].core_max_freq)
            {
                break;
            }
            // Check if actual clips are above WOF frequency
            if (g_amec->proc[0].core_max_freq_actual > g_amec->wof.f_vote)
            {
                break;
            }

            // If we reach this point, then we can be assured that
            // 1) GPE has applied the latest frequency selection, including WOF vote
            // 2) The true frequency is not above the WOF frequency
            // Therefore, proceed to next state (wake cores up)
            g_amec->wof.state = AMEC_WOF_TRANSITION;

            // FALL THROUGH
        }

        case AMEC_WOF_TRANSITION:
        {
            // Signal waking cores to turn on and go back to initial state.

            // Quickly toggle waking core inhibit bits to let those cores
            // wake up (uninhibit cores waking up)
            l_data32 = (g_amec->wof.wake_up_mask >> 32);
            out32(PDEMR, ~l_data32);

            // Next, inhibit all cores again
            l_data32 = 0xffff0000ull;
            out32(PDEMR, l_data32);

            g_amec->wof.pstatetable_cores_current =
                g_amec->wof.pstatetable_cores_next;
            g_amec->wof.state = AMEC_WOF_NO_CORE_CHANGE;
            break;
        }

        default:
            amec_wof_error_update(AMEC_WOF_ERROR_UNKNOWN_STATE);
            TRAC_ERR("amec_wof_helper_v2: WOF is in an unknown state: 0x%X. "
                     " wof.error:0x%X", g_amec->wof.state, g_amec->wof.error);
            break;
    }
}


// Function Specification
//
// Name: amec_wof_helper
//
// Description: Do WOF tasks required on 250us intervals
//              (e.g. States in algorithm #2)
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_wof_helper(void)
{
    switch (g_amec->wof.algo_type)
    {
        case 1:
            // Not supported
            break;

        case 2:
            amec_wof_helper_v2();
            break;

        case 3:
            amec_wof_helper_v3();
            break;

        default: /*Do nothing. WOF Disabled*/
            break;
    }
}

// Function Specification
//
// Name: amec_wof_main
//
// Description: Run the main WOF algorithm every 2msec.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_wof_main(void)
{
    int                 l_rc  = 0;
    errlHndl_t          l_err = NULL;
    // Count number of consecutive intervals WOF is not making progress
    static uint32_t wof_main_error_count = 0;

    if (g_amec->wof.error)
    {
        wof_main_error_count++;
    }
    else
    {
        wof_main_error_count=0;
    }
    if ((wof_main_error_count >= g_amec->wof.error_threshold)
        && (g_amec->wof.enable_parm != 0))
    {
        //Trigger error if WOF is running and not operating for error_threshold iterations
        // which are each 2ms for POWER8.
        amec_wof_error_update(AMEC_WOF_ERROR_THRESHOLD_REACHED);
        TRAC_ERR("amec_wof_main: >=%i consecutive errors, disabling wof. error_hist = 0x%016llx, error_hist_nz=0x%016llx, error_count_ttl=%lu",
                 g_amec->wof.error_threshold,  g_amec->wof.error_history,
                 g_amec->wof.error_history_nz, g_amec->wof.error_count_total);

        /* @
         * @errortype
         * @moduleid    AMEC_WOF_MAIN
         * @reasoncode  AMEC_WOF_ERROR
         * @userdata1   4 most recent WOF error codes
         * @userdata2   next 4 WOF error codes
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     WOF Consecutive error threshold reached
         *
         */
        l_err = createErrl(AMEC_WOF_MAIN,               //modId
                           AMEC_WOF_ERROR,              //reasoncode
                           ERC_GENERIC_TIMEOUT,         //Extended reason code
                           ERRL_SEV_PREDICTIVE,         //Severity
                           NULL,                        //Trace Buf
                           DEFAULT_TRACE_SIZE,          //Trace Size
                           (uint32_t)(g_amec->wof.error_history>>32), //userdata1
                           (uint32_t)(g_amec->wof.error_history)); //userdata2

        // Callout to firmware
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_MED);

        // Commit the error
        commitErrl(&l_err);

        g_amec->wof.enable_parm = 0; // disable WOF
    }

    // If new algorithm selected, then initialize it. If initialization fails,
    // then do not run algorithm
    if (g_amec->wof.enable_parm != g_amec->wof.algo_type)
    {
        l_rc = amec_wof_set_algorithm(g_amec->wof.enable_parm);
        if (l_rc)
        {
            TRAC_ERR("amec_wof_main: Failed to set algorithm. WOF algorithm "
                     "will not be run. l_rc=0x%X", l_rc);
            return;
        }
    }

    // Run the current WOF algorithm
    switch (g_amec->wof.algo_type)
    {
        case 1:
            // Not supported
            break;

        case 2:
            // Production-level algorithm
            amec_wof_alg_v2();
            break;

        case 3:
            // Production-level algorithm with dynamic Pstate table
            amec_wof_alg_v3();
            break;

        default: /*Do nothing. WOF Disabled*/
            break;
    }
}

// Function Specification
//
// Name:  amec_wof_writeToTable
//
// Description: Writes wof data given in a one dimensional
//              array to a two dimensional predefined table.
//
// PREREQ: This function expects that all data validation has been done.
//
// End Function Specification
void amec_wof_writeToTable(wof_tbl_type_t i_tblType ,
                              const uint16_t i_size,
                              const uint8_t i_clmnCount,
                              uint8_t *i_data_ptr)
{

    uint16_t l_tblIndex = 0;
    uint32_t l_temp = 0;

    if (i_clmnCount > 0)
    {
        //walk the array of data given
        for (l_tblIndex = 0; l_tblIndex < i_size / 2; l_tblIndex++)
        {
            //Get table row and column to write the entry into.
            uint8_t l_tblRow = l_tblIndex / i_clmnCount;
            uint8_t l_tblClmn = l_tblIndex % i_clmnCount;

            //Write CORE Freq Table
            if (i_tblType == AMEC_WOF_CORE_FREQ_TBL)
            {
                //Reset global table
                if (0 == l_tblIndex)
                {
                    memset(&G_amec_wof_uplift_table, 0, sizeof(G_amec_wof_uplift_table));
                }
                //Even though this check should have been made by the
                //calling function, make sure we don't attempt to write
                //beyond table limits.
                if ((l_tblClmn < AMEC_WOF_UPLIFT_TBL_CLMS) &&
                    (l_tblRow  < AMEC_WOF_UPLIFT_TBL_ROWS) )
                {
                    //Write each two bytes of data into each cell.
                    l_temp = (i_data_ptr[l_tblIndex * 2] << 8) | (i_data_ptr[(l_tblIndex * 2) + 1]);

                    if ((l_tblClmn != 0) &&
                        (l_tblRow != 0))
                    {
                        // Translate each entry from 1/100% to frequency in MHz
                        // Table will reflect the frequency uplift based on TURBO.
                        l_temp = G_sysConfigData.sys_mode_freq.table[OCC_MODE_STURBO] * (10000 + l_temp) / 10000;
                    }
                    G_amec_wof_uplift_table[l_tblRow][l_tblClmn] = (uint16_t)l_temp;

                }
                else
                {
                    TRAC_ERR("amec_wof_writeToTable: WOF Core Freq Data given is larger than we can fit in table. "
                             "Attempting to write cell at [%i,%i] location.", l_tblRow, l_tblClmn);
                }
            }
            else if (i_tblType == AMEC_WOF_VRM_EFF_TBL) //Write VRM Efficiency table.
            {
                //Reset data buffer
                if (0 == l_tblIndex)
                {
                    memset(&G_amec_wof_vrm_eff_table, 0, sizeof(G_amec_wof_vrm_eff_table));
                }
                //Even though this check should have been made by the
                //calling function, make sure we don't attempt to write
                //beyond table limits.
                if ((l_tblClmn < AMEC_WOF_VRM_EFF_TBL_CLMS) &&
                    (l_tblRow  < AMEC_WOF_VRM_EFF_TBL_ROWS) )
                {
                    l_temp = (i_data_ptr[l_tblIndex * 2] << 8) | (i_data_ptr[(l_tblIndex * 2) + 1]);

                    if (l_tblRow == 0)
                    {
                        // Translate entries from first row Output Current in
                        // Amps to 10mA resolution
                        l_temp = l_temp * 100;
                    }

                    //Write each two bytes of data into each cell.
                    G_amec_wof_vrm_eff_table[l_tblRow][l_tblClmn] = (uint16_t)l_temp;
                }
                else
                {
                    TRAC_ERR("amec_wof_writeToTable: WOF VRM Eff Data given is larger than we can fit in table. "
                             "Attempting to write cell at [%i,%i] location.", l_tblRow, l_tblClmn);
                }
            }
            else
            {
                TRAC_ERR("amec_wof_writeToTable: Invalid table type.");
            }
        }
    }
    else
    {
        TRAC_ERR("amec_wof_writeToTable: column size given is 0.");
    }

}

// Function Specification
//
// Name:  amec_wof_store_vrm_eff
//
// Description: Parses and handles all data passed to OCC via data
//              config format 0x31.
//
// Prereq: Data size has been verified.
//
// End Function Specification
void amec_wof_store_vrm_eff( const uint16_t i_size,
                             const uint8_t i_clmnCount,
                             uint8_t *i_data_ptr)
{
    //Store given data into global table.
    amec_wof_writeToTable(AMEC_WOF_VRM_EFF_TBL, i_size, i_clmnCount, i_data_ptr);
}

// Function Specification
//
// Name:  amec_wof_store_core_freq
//
// Description: Parses and handles all data passed to OCC via data
//              config format 0x30.
//
// Prereq: Data size has been verified.
//
// End Function Specification
void amec_wof_store_core_freq(const uint8_t i_max_good_cores,
                              const uint16_t i_size,
                              const uint8_t i_clmnCount,
                              uint8_t *i_data_ptr)
{

    //Store max good cores
    G_wof_max_cores_per_chip = i_max_good_cores;

    //FIXME: Need to know the number of rows with valid data

    //Store given data into global table.
    amec_wof_writeToTable(AMEC_WOF_CORE_FREQ_TBL, i_size, i_clmnCount, i_data_ptr);

}

// Function Specification
//
// Name:  amec_wof_get_max_freq
//
// Description: Returns the max frequency calculated
// for the given number of cores.
//
// End Function Specification
uint16_t amec_wof_get_max_freq(const uint8_t i_cores)
{

    uint16_t        l_maxFreq = 0;
    uint8_t         l_clmn = 0;
    uint8_t         l_row = 0;

    //find the correct column for the given number of cores.
    for (l_clmn = 1; l_clmn < AMEC_WOF_UPLIFT_TBL_CLMS; l_clmn++)
    {
        if (i_cores == G_amec_wof_uplift_table[0][l_clmn])
        {
            //Find the largest frequency in this column
            for (l_row = 1; l_row < AMEC_WOF_UPLIFT_TBL_ROWS; l_row++)
            {
                if (G_amec_wof_uplift_table[l_row][l_clmn] > l_maxFreq)
                {
                    l_maxFreq = G_amec_wof_uplift_table[l_row][l_clmn];
                }
            }

            break; //out of for loop.
        }
    }

    return l_maxFreq;
}

// Function Specification
//
// Name: amec_wof_calc_core_leakage
//
// Description: compute core-level leakage current sensor
// Used by each interval of WOF algorithm.
//
//
// Flow:              FN=
//   Run in each AMEC real-time state (as per-core sensors are processed)
//
// Thread:
//
// Task Flags:
//
// Future improvement:
//
// End Function Specification
void amec_wof_calc_core_leakage(uint8_t i_core)
{
    uint8_t             i;
    uint32_t            l_result32; //temporary result
    uint32_t l_temp; // per-core temperature
    uint16_t l_voltage = AMECSENSOR_ARRAY_PTR(VOLT2MSP0C0,i_core)->sample;
    uint16_t l_core_v; // voltage index for IDDQ table

    if (l_voltage == 0)
    {
        // Core is off, estimate header leakage only
        //Use avg. chip temp, since DTS cannot be read for off core
        //Use chip voltage for header voltage
        l_temp = AMECSENSOR_PTR(TEMP2MSP0)->sample;
        l_core_v = g_amec->wof.v_chip;
    }
    else
    {
        // Core is on, use avg. DTS readings for core temperature
        l_temp = AMECSENSOR_ARRAY_PTR(TEMP2MSP0C0,i_core)->sample;
        l_core_v = l_voltage;
    }

    // Search table and point i to the lower entry the target value falls
    // between.
    if (l_core_v < G_iddq_voltages[0])
    {
        i=0; // voltage is lower than table, so use first two entries.
    }
    else
    {
        for (i=0; i<CORE_IDDQ_MEASUREMENTS-1; i++)
        {
            if (G_iddq_voltages[i] <= l_core_v &&
                G_iddq_voltages[i+1] >= l_core_v)
            {
                break;
            }
        }
    }
    if (i >= CORE_IDDQ_MEASUREMENTS - 1)
    {
        // Voltage is higher than table, so use last two entries.
        i = CORE_IDDQ_MEASUREMENTS - 2;
    }

    g_amec->wof.iddq_i_core[i_core] = i;

    // Linear interpolate using the neighboring entries:
    // y = m(x-x1)+y1   m=(y2-y1)/(x2-x1)
    l_result32 = ((int32_t)l_core_v - (int32_t)G_iddq_voltages[i])
        * ((int32_t)G_sysConfigData.iddq_table.iddq_vdd[i+1].fields.iddq_corrected_value -
           (int32_t)G_sysConfigData.iddq_table.iddq_vdd[i].fields.iddq_corrected_value)
        / ((int32_t)G_iddq_voltages[i+1] - (int32_t)G_iddq_voltages[i])
        + (int32_t)G_sysConfigData.iddq_table.iddq_vdd[i].fields.iddq_corrected_value;

    // Note: IDDQ value from table above is in 0.01 A units. The maximum
    // value possible is 655.35 A. This means l_result <= 65535.

    g_amec->wof.iddq85c_core[i_core] = (uint16_t)l_result32;

    // Temperature correction
    if (l_temp < amec_wof_iddq_mult_table[0][0])
    {
        i=0; // index is lower than table, so use first two entries.
    }
    else
    {
        for (i=0; i<AMEC_WOF_IDDQ_MULT_TABLE_N-1; i++)
        {
            if (amec_wof_iddq_mult_table[i][0] <= l_temp &&
                amec_wof_iddq_mult_table[i+1][0] >= l_temp)
            {
                break;
            }
        }
    }
    if (i >= AMEC_WOF_IDDQ_MULT_TABLE_N - 1)
    {
        i = AMEC_WOF_IDDQ_MULT_TABLE_N - 2;
    }

    uint32_t l_mult = ((int32_t)l_temp - (int32_t)amec_wof_iddq_mult_table[i][0])
    * ((int32_t)amec_wof_iddq_mult_table[i+1][1] - (int32_t)amec_wof_iddq_mult_table[i][1])
    / ((int32_t)amec_wof_iddq_mult_table[i+1][0] - (int32_t)amec_wof_iddq_mult_table[i][0])
    + (int32_t)amec_wof_iddq_mult_table[i][1];

    l_result32 = (l_result32*l_mult) >> 10;

    // Modify leakage value for header-only. A percentage of chip
    // leakage comes from iVRM headers which cannot be turned off completely.
    if (l_voltage == 0)
    {
        l_result32 = l_result32
            * g_amec_wof_leak_overhead / 1000;
    }

    g_amec->wof.iddq_core[i_core] = l_result32;
    //Note:
    //iddq_core[] represents the leakage current as if all cores
    //in chip are on at the voltage and temperature of i_core.
    //To calculate true per-core leakage current,
    //divide by number of good cores on chip
    //(G_wof_max_cores_per_chip).
    //This is because IDDQ table represents full chip current.

}
