/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_wof.c $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <proc_data.h> // for CORE_PRESENT
#include <errl.h>      // Error logging
#include <sensor.h>
#include <amec_sys.h>
#include <cmdh_fsp_cmds_datacnfg.h>
#include <ssx_api.h>  // SsxSemaphore
#include <amec_wof.h> // For wof semaphore in amec_wof_thread.c
#include <common.h>
#include <cmdh_fsp_cmds_datacnfg.h>

//*************************************************************************
// Externs
//*************************************************************************

extern amec_sys_t g_amec_sys;
extern data_cnfg_t * G_data_cnfg; // in cmdh_fsp_cmds_datacnfg
extern SsxSemaphore G_amec_wof_thread_wakeup_sem; // in amec_wof_thread.c

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

// For prototype, one and only one of the following defines must be 1
#define HAB19

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

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

uint8_t G_wof_max_cores_per_chip = 12; //defaulted to 12 for now

#ifdef HAB19

#define AMEC_WOF_LOADLINE_ACTIVE 550  // Active loadline in micro ohms
#define AMEC_WOF_LOADLINE_PASSIVE 50 // Passive loadline in micro ohms

#endif //HAB19


//Approximate y=1.25^((T-85)/10).
//Interpolate T in the table below to find m
// y ~= (T*m) >> 10     (shift out 10 bits)
// Error in estimation is about 0.6% maximum.
uint16_t amec_wof_iddq_mult_table[][2] = {
    //Temperature in C, m
    {20, 240},
    {30, 300},
    {40, 375},
    {50, 469},
    {60, 586},
    {70, 733},
    {80, 916},
    {90, 1145}
};
#define AMEC_WOF_IDDQ_MULT_TABLE_N 8

//Algorithm 3 (aka WOF 2.1)
uint8_t g_amec_wof_pstate_table_ready = 0;

// Useful globals for debug
int32_t  g_amec_eff_vlow = 0;
int32_t  g_amec_eff_vhigh = 0;;
uint32_t g_amec_wof_iout = 0;

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

// Function Specification
//
// Name: amec_wof_vdd_current_out
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
// o_v_sense: Voltage at Vdd remote sense (0.0001 V units)
//
// End Function Specification
void amec_wof_vdd_current_out(const uint16_t i_power_in,
                              const uint16_t i_v_set,
                              uint16_t *o_current_out,
                              uint16_t *o_v_sense)
{
    uint8_t l_iteration;
    uint8_t i;

    // Stuff that can be pre-computed when efficiency table is read
    int32_t v_min = G_amec_wof_vrm_eff_table[1][0] * 100; //0.8500 V = min from efficiency table
    int32_t v_max = G_amec_wof_vrm_eff_table[2][0] * 100; //1.2500 V = max from efficiency table
    int32_t v_diff = v_max - v_min; // max voltage - min voltage from efficiency table

    // helper variables
    int32_t l_x2minusx1, l_xminusx1;
    int32_t l_eff_vlow, l_eff_vhigh, l_v_offset;
    int32_t l_v_sense;

    // Set initial guesses before iteration part of algorithm
    uint32_t l_eff = 8500; //initial guess efficiency=85.00%

    // Compute regulator output power.  out = in * efficiency
    //    power_in: min=0W max=300W = 3000dW
    //    eff: min=0 max=10000=100% (.01% units)
    //    power_in*eff: max=3000dW * 10000 = 30,000,000 (dW*0.0001)
    //                  is under 2^25, fits in 25 bits
    //    *10 = 300M (dW*0.00001) in 29 bits
    uint32_t l_pout = i_power_in * l_eff * 10;

    // Compute current out of regulator.
    // curr_out = power_out (*10 scaling factor) / voltage_out
    //    p_out: max=300M (dW*0.00001) in 29 bits
    //    v_set: min=5000 (0.0001 V)  max=16000(0.0001 V) in 14 bits
    //     iout: max = 300M/5000 = 60000 (dW*0.00001/(0.0001V)= 0.01A), in 16 bits.
    uint32_t l_iout = l_pout/i_v_set; //initial iout
    g_amec_wof_iout = l_iout; //for debugging

    for(l_iteration=0; l_iteration<2; l_iteration++)  // iterate twice
    {
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

        g_amec_eff_vlow = l_eff_vlow;//for debug

        // Reuse same x2minusx1 and xminusx1 because efficiency curves use
        // common current_in values
        l_eff_vhigh = l_xminusx1 * ((int32_t)G_amec_wof_vrm_eff_table[2][i+1] -
                                    (int32_t)G_amec_wof_vrm_eff_table[2][i]) /
            l_x2minusx1 + (int32_t)G_amec_wof_vrm_eff_table[2][i];

        g_amec_eff_vhigh = l_eff_vhigh;// for debug

        l_v_offset = i_v_set - v_min;
        l_eff = ((l_eff_vhigh - l_eff_vlow) * l_v_offset) / v_diff + l_eff_vlow;
        // V_droop = I_chip (0.01 A) * R_loadline (0.000001 ohm) => (in 0.00000001 V)
        // V_droop = V_droop / 10000 => (in 0.0001 V)
        // V_sense = V_reg - V_droop  => (in 0.0001 V)
        l_v_sense = i_v_set - AMEC_WOF_LOADLINE_ACTIVE * l_iout / 10000;
        l_pout = i_power_in * l_eff * 10; // See l_pout above for *10 note
        l_iout = l_pout/l_v_sense;
    }

    l_v_sense = i_v_set - AMEC_WOF_LOADLINE_ACTIVE * l_iout / 10000;

    *o_v_sense = l_v_sense;
    *o_current_out = l_iout;
    g_amec->wof.vdd_eff = l_eff; //for debugging
}

void amec_wof_init(void)
{
    // Initialize DCOM Thread Sem
    ssx_semaphore_create( &G_amecWOFThreadWakeupSem, // Semaphore
                          1,                         // Initial Count
                          0);                        // No Max Count

    // Avoid recomputing total
    g_amec->wof.loadline = AMEC_WOF_LOADLINE_ACTIVE + AMEC_WOF_LOADLINE_PASSIVE;
}


bool amec_wof_validate_input_data(void)
{
    bool                l_valid_data = TRUE;

    // Check operating point at turbo
    TRAC_IMP("WOF Operating Turbo point: vdd_5mv[%d] Iddq_500ma[%d] frequency_mhz[%d]",
             G_sysConfigData.wof_parms.operating_points[TURBO].vdd_5mv,
             G_sysConfigData.wof_parms.operating_points[TURBO].idd_500ma,
             G_sysConfigData.wof_parms.operating_points[TURBO].frequency_mhz);

    // RDP to TDP conversion factor and Max number of cores for this chip
    TRAC_IMP("WOF rdp_tdp_factor[%d] max_cores_per_chip[%d]",
             G_sysConfigData.wof_parms.tdp_rdp_factor,
             G_wof_max_cores_per_chip);

    // IDDQ table
    TRAC_IMP("WOF IDDQ Vdd current: 0.80V[%d] 0.90V[%d] 1.00V[%d] 1.10V[%d] 1.20V[%d] 1.25V[%d]",
             G_sysConfigData.iddq_table.iddq_vdd[0].fields.iddq_corrected_value,
             G_sysConfigData.iddq_table.iddq_vdd[1].fields.iddq_corrected_value,
             G_sysConfigData.iddq_table.iddq_vdd[2].fields.iddq_corrected_value,
             G_sysConfigData.iddq_table.iddq_vdd[3].fields.iddq_corrected_value,
             G_sysConfigData.iddq_table.iddq_vdd[4].fields.iddq_corrected_value,
             G_sysConfigData.iddq_table.iddq_vdd[5].fields.iddq_corrected_value);

    // Uplift table
    TRAC_IMP("WOF Uplift Freqs: Row_1[%d] 1cor[%d] 2cor[%d] 3cor[%d] 12cor[%d]",
             G_amec_wof_uplift_table[1][0],
             G_amec_wof_uplift_table[1][1],
             G_amec_wof_uplift_table[1][2],
             G_amec_wof_uplift_table[1][3],
             G_amec_wof_uplift_table[1][12]);
    TRAC_IMP("WOF Uplift Freqs: Row_2[%d] 1cor[%d] 2cor[%d] 3cor[%d] 12cor[%d]",
             G_amec_wof_uplift_table[2][0],
             G_amec_wof_uplift_table[2][1],
             G_amec_wof_uplift_table[2][2],
             G_amec_wof_uplift_table[2][3],
             G_amec_wof_uplift_table[2][12]);
    TRAC_IMP("WOF Uplift Freqs: Row11[%d] 1cor[%d] 2cor[%d] 3cor[%d] 12cor[%d]",
             G_amec_wof_uplift_table[11][0],
             G_amec_wof_uplift_table[11][1],
             G_amec_wof_uplift_table[11][2],
             G_amec_wof_uplift_table[11][3],
             G_amec_wof_uplift_table[11][12]);
    TRAC_IMP("WOF Uplift Freqs: LastR[%d] 1cor[%d] 2cor[%d] 3cor[%d] 12cor[%d]",
             G_amec_wof_uplift_table[AMEC_WOF_UPLIFT_TBL_ROWS-1][0],
             G_amec_wof_uplift_table[AMEC_WOF_UPLIFT_TBL_ROWS-1][1],
             G_amec_wof_uplift_table[AMEC_WOF_UPLIFT_TBL_ROWS-1][2],
             G_amec_wof_uplift_table[AMEC_WOF_UPLIFT_TBL_ROWS-1][3],
             G_amec_wof_uplift_table[AMEC_WOF_UPLIFT_TBL_ROWS-1][12]);

    // VRM Efficiency Table
    TRAC_IMP("WOF VRM Effic: Current_10mA 1[%d] 2[%d] 3[%d] ... 12[%d] 13[%d]",
             G_amec_wof_vrm_eff_table[0][1],
             G_amec_wof_vrm_eff_table[0][2],
             G_amec_wof_vrm_eff_table[0][3],
             G_amec_wof_vrm_eff_table[0][12],
             G_amec_wof_vrm_eff_table[0][13]);
    TRAC_IMP("WOF VRM Effic: Lo_volt[%d] 20A[%d] 40A[%d] 60A[%d] 260A[%d]",
             G_amec_wof_vrm_eff_table[1][0],
             G_amec_wof_vrm_eff_table[1][1],
             G_amec_wof_vrm_eff_table[1][2],
             G_amec_wof_vrm_eff_table[1][3],
             G_amec_wof_vrm_eff_table[1][13]);
    TRAC_IMP("WOF VRM Effic: Hi_volt[%d] 20A[%d] 40A[%d] 60A[%d] 260A[%d]",
             G_amec_wof_vrm_eff_table[2][0],
             G_amec_wof_vrm_eff_table[2][1],
             G_amec_wof_vrm_eff_table[2][2],
             G_amec_wof_vrm_eff_table[2][3],
             G_amec_wof_vrm_eff_table[2][13]);

    TRAC_IMP("WOF VID Mod Table: Num_pstates[%d] Max_num_cores[%d]",
             G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_pstates,
             G_sysConfigData.wof_parms.ut_vid_mod.ut_max_cores);
    TRAC_IMP("WOF Vdd VID Mod Table 1_core: p0[0x%02X] p-14[0x%02X] p-15[0x%02X] p-16[0x%02X] p-17[0x%02X]",
             G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[0][0],
             G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[14][0],
             G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[15][0],
             G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[16][0],
             G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[17][0]);
    TRAC_IMP("WOF Vdd VID Mod Table 10_cores: p0[0x%02X] p-14[0x%02X] p-15[0x%02X] p-16[0x%02X] p-17[0x%02X]",
             G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[0][9],
             G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[14][9],
             G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[15][9],
             G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[16][9],
             G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[17][9]);
    TRAC_IMP("WOF Vdd VID Mod Table 12_cores: p0[0x%02X] p-14[0x%02X] p-15[0x%02X] p-16[0x%02X] p-17[0x%02X]",
             G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[0][11],
             G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[14][11],
             G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[15][11],
             G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[16][11],
             G_sysConfigData.wof_parms.ut_vid_mod.ut_segment_vdd_vid[17][11]);

    return l_valid_data;
}

uint32_t amec_wof_compute_c_eff(void)
{
    // Estimate IDDQ@RDP(95C)@Vnom P0:
    // IDDQ@85C * 1.25 ^ ([95-85]/10) = 35.84 * 1.25 = 44.8 A (Leakage current)
    //
    // NM_Idd@RDP P0 = 151 A
    //
    // P0: (151 A - 44.8 A) / 1.22 * 1.12 = 97.50 A @TDP
    //
    // C_eff    = I / (V^3 * F)
    //
    //   I is ??? in 0.01 Amps (or 10 mA)
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
    // FIXME: Can we expect m to be negative?
    l_i_leak = (l_v_chip - G_iddq_voltages[i]) *
        ((int32_t)G_sysConfigData.iddq_table.iddq_vdd[i+1].fields.iddq_corrected_value -
         (int32_t)G_sysConfigData.iddq_table.iddq_vdd[i].fields.iddq_corrected_value) /
        ((int32_t)G_iddq_voltages[i+1] - (int32_t)G_iddq_voltages[i]) +
        G_sysConfigData.iddq_table.iddq_vdd[i].fields.iddq_corrected_value;

    // STEP 3: Correct the leakage current computed in the previous step for
    // temperature by multiplying by 1.25
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
uint8_t amec_wof_set_algorithm(const uint8_t i_algorithm)
{
    uint64_t             l_data64 = 0;
    uint32_t             l_rc = 1;

    do
    {
        // Start WOF in safe state.
        if (!(G_data_cnfg->data_mask | DATA_MASK_FREQ_PRESENT))
        {
            // Frequency table is not loaded, don't know safe turbo for WOF.
            // Initialization not yet possible. Try again later.
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
            // Set to an invalid algorithm.
            g_amec->wof.algo_type = 0xFF;

            // Initialization not finished. Try again later.
            break;
        }

        // At this point, all cores are at Turbo or lower.
        // We may turn off the inhibit-wake signal.

        // Validate input data to the WOF algorithm
        amec_wof_validate_input_data();

        // Calculate ceff_tdp from static data in the Pstate SuperStructure
        g_amec->wof.ceff_tdp = amec_wof_compute_c_eff();

        switch(i_algorithm)
        {
            case 3:
                // FALL-THROUGH

            case 2: // WOF algorithm 2
                g_amec->wof.state = AMEC_WOF_NO_CORE_CHANGE; // WOF state
                // Set pstate table to MAX_NUM_CORES;
                // Conservatively assume all cores are on for next interval
                g_amec->wof.pstatetable_cores_current = MAX_NUM_CORES;
                g_amec->wof.pstatetable_cores_next = MAX_NUM_CORES;

                // Inhibit cores from waking up
                l_data64 = 0xffff000000000000ull;
                l_rc = _putscom(PDEMR, l_data64, SCOM_TIMEOUT);
                if (l_rc != 0)
                {
                    g_amec->wof.error = AMEC_WOF_ERROR_SCOM_4;
                }
                break;

            case 0: // No WOF
                // Do not inhibit core wakeup anymore (in case of transitioning
                // out of algorithm 2)
                l_data64 = 0x0000000000000000ull;
                l_rc = _putscom(PDEMR, l_data64, SCOM_TIMEOUT);
                if (l_rc != 0)
                {
                    g_amec->wof.error = AMEC_WOF_ERROR_SCOM_5;
                }
                break;
        }

        // Success, set the new algorithm
        g_amec->wof.algo_type = g_amec->wof.enable_parm;
        l_rc = 0;

        TRAC_INFO("WOF algorithm has been successfully initialized: algo_type[%d] C_eff_tdp[%d]",
                  g_amec->wof.algo_type,
                  g_amec->wof.ceff_tdp);

    } while (0);

    return l_rc;
}


// Function Specification
//
// Name: amec_update_wof_sensors
//
// Description: Common for WOF/non-WOF.
//   Compute CUR250USVDD0 (current out of Vdd regulator)
//   Compute voltage at chip input
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_update_wof_sensors(void)
{
    uint16_t            l_pow_reg_input_dW = AMECSENSOR_PTR(PWR250USVDD0)->sample * 10; // convert to dW by *10.
    uint16_t            l_vdd_reg = AMECSENSOR_PTR(VOLT250USP0V0)->sample;
    uint16_t            l_curr_output;
    uint32_t            l_v_droop;
    uint32_t            l_v_chip;
    uint16_t            l_v_sense;

    // Step 1: Calculate voltage at chip
    amec_wof_vdd_current_out(l_pow_reg_input_dW,
                             l_vdd_reg,
                             &l_curr_output,
                             &l_v_sense);
    sensor_update(AMECSENSOR_PTR(CUR250USVDD0), l_curr_output);

    // Save Vsense estimate for WOF validation
    sensor_update(AMECSENSOR_PTR(WOF250USVDDS), l_v_sense);

    //1c. Compute Vdd load at chip
    //    V_reg = V_chip + I_chip * (R_loadline)
    //    V_droop = I_chip (0.01 A) * R_loadline (0.000001 ohm) => (in 0.00000001 V)
    //    V_droop = V_droop / 10000 => (in 0.0001 V)
    //    V_chip = V_reg - V_droop  => (in 0.0001 V)
    l_v_droop = (uint32_t) l_curr_output * (uint32_t) g_amec->wof.loadline / (uint32_t)10000;
    l_v_chip = l_vdd_reg - l_v_droop;
    g_amec->wof.v_chip = l_v_chip; // expose in parameter
}

void amec_wof_common_steps(void)
{
    uint8_t             i;
    uint32_t            l_result32; //temporary result
    uint32_t            l_result32v;
    uint32_t            l_result32i;
    uint8_t             l_cores_on = 0;
    uint8_t             l_cores_waking = 0;
    uint8_t             l_pstatetable_cores_next=0;
    uint64_t            l_data64 = 0; // For SCOM access
    uint32_t            l_rc;
    Pstate              l_wof_vote_pstate; // pstate that corresponds to wof
                                           // vote. Find associated voltage.
    uint8_t             l_wof_vote_vid; // Vdd regulator VID associated with WOF vote.
    uint16_t            l_temp = 0;   //Processor temperature
    uint32_t            l_accum = 0;  //Vdd current accumulator
    uint32_t            l_v_chip = 0; //Voltage at chip silicon

    // Acquire important sensor data
    l_temp = AMECSENSOR_PTR(TEMP2MSP0)->sample;
    l_accum = AMECSENSOR_PTR(CUR250USVDD0)->accumulator;
    l_v_chip = g_amec->wof.v_chip; //from amec_update_wof_sensors()

    if (g_amec->wof.state != AMEC_WOF_NO_CORE_CHANGE)
    {
        return;
    }
    if (g_amec->wof.pstatetable_cores_next !=
        g_amec->wof.pstatetable_cores_current)
    {
        g_amec->wof.error = AMEC_WOF_ERROR_CORE_COUNT;
        return;
    }

    // Count number of cores that are turned on.
    // They will have non-zero frequency. Cores that are in winkle or sleep
    // have frequency set to zero by OCC code.
    for (i=0; i<MAX_NUM_CORES; i++)
    {
        g_amec->wof.pm_state[i] = g_amec_sys.proc[0].core[i].pm_state_hist;
        if (!CORE_PRESENT(i))
        {
            continue;
        }

        // A core is "on" if it is not in deep sleep or deep winkle
        switch(g_amec_sys.proc[0].core[i].pm_state_hist >> 5)
        {
            case 5: //deep sleep
                // FIXME: P8 only uses deep winkle. Add deep sleep when
                // we can inhibit it.
                //l_cores_on++;
                break;

            case 7: //deep winkle
                break;

            default:
                l_cores_on++;
                break;
        }
    }

    g_amec->wof.cores_on = l_cores_on;

    l_rc = _getscom(PMCWIRVR3, &g_amec->wof.wake_up_mask, SCOM_TIMEOUT);
    if (l_rc != 0)
    {
        g_amec->wof.error = AMEC_WOF_ERROR_SCOM_1;
        return;
    }

    // FIXME: Whenever deep sleep works:
    // Need to do a getscom of register PMCSIRV3 to find the deep sleep cores
    // that want to wake up. Then do an OR with the mask wake_up_mask.

    // Save non-zero wake up mask for debugging
    if (g_amec->wof.wake_up_mask != 0)
    {
        g_amec->wof.wake_up_mask_save = g_amec->wof.wake_up_mask;
    }

    // Count the number of cores that want to wake up
    l_data64 = g_amec->wof.wake_up_mask >> 48;
    for(i=0; i<MAX_NUM_CORES; i++)
    {
        if (l_data64 & 0x1)
        {
            l_cores_waking++;
        }
        l_data64 >>= 1; // shift right
    }
    sensor_update(AMECSENSOR_PTR(WOFCOREWAKE), l_cores_waking);

    // Set the number of cores will be on for the next round.
    l_pstatetable_cores_next = l_cores_on + l_cores_waking;
    // Save number of cores OCC is transitioning toward for other WOF states
    g_amec->wof.pstatetable_cores_next = l_pstatetable_cores_next;

    // Step 2
    // Search table and point i to the lower entry the target value falls
    // between.
    if (l_v_chip < G_iddq_voltages[0])
    {
        i=0; // voltage is lower than table, so use first two entries.
    }
    else
    {
        for (i=0; i<CORE_IDDQ_MEASUREMENTS-1; i++)
        {
            if (G_iddq_voltages[i] <= l_v_chip &&
                G_iddq_voltages[i+1] >= l_v_chip)
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

    g_amec->wof.iddq_i = i;

    // Linear interpolate using the neighboring entries:
    // y = m(x-x1)+y1   m=(y2-y1)/(x2-x1)
    //FIXME: add rounding step after multiplication
    //FIXME: pre-compute m, since table is static
    l_result32 = ((int32_t)l_v_chip - (int32_t)G_iddq_voltages[i])
        * ((int32_t)G_sysConfigData.iddq_table.iddq_vdd[i+1].fields.iddq_corrected_value -
           (int32_t)G_sysConfigData.iddq_table.iddq_vdd[i].fields.iddq_corrected_value)
        / ((int32_t)G_iddq_voltages[i+1] - (int32_t)G_iddq_voltages[i])
        + (int32_t)G_sysConfigData.iddq_table.iddq_vdd[i].fields.iddq_corrected_value;

    l_result32 = l_result32 * l_cores_on / G_wof_max_cores_per_chip;

    g_amec->wof.iddq85c = (uint16_t)l_result32;  // expose to parameter

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
    g_amec->wof.iddq = l_result32;

    // Compute 2ms current average
    // Divide the 250us accumulator by 8 samples to get 2ms average
    g_amec->wof.cur_out = (l_accum - g_amec->wof.cur_out_last) >> 3; // 0.01 A
    g_amec->wof.cur_out_last = l_accum;

    // Step 3: Compute AC portion of chip Vdd current (units of 0.01 A)
    g_amec->wof.ac = g_amec->wof.cur_out - g_amec->wof.iddq;

    // Step 4: Computer ratio of computed workload AC to TDP

    //Victor and Josh request to use WOF freq/volt from last WOF vote
    //(4/14/2015 e-mail exchange)

    // Get voltage associated with prior WOF vote
    l_wof_vote_pstate = proc_freq2pstate(g_amec->wof.f_vote);
    l_wof_vote_vid =
        G_global_pstate_table.pstate[l_wof_vote_pstate].fields.evid_vdd;
    g_amec->wof.vote_vreg = 16125 - ((uint32_t)l_wof_vote_vid * 625)/10;
    //Calculate voltage at chip if pstate changed to wof vote instantly
    g_amec->wof.vote_vchip = g_amec->wof.vote_vreg - (uint32_t) g_amec->wof.cur_out
        * (uint32_t) g_amec->wof.loadline / (uint32_t)10000;

    l_result32i = g_amec->wof.ac << 14; // * 16384
    // estimate g_amec->wof.v_chip^1.3 using equation:
    // = 21374 * (X in 0.1 mV) - 50615296
    l_result32v = (21374 * g_amec->wof.vote_vchip - 50615296) >> 10;
    l_result32 = l_result32i / l_result32v;
    l_result32 = l_result32 << 14; // * 16384
    if (g_amec->wof.f_vote != 0)
    {
        // avoid divide by 0
        l_result32 = l_result32 / (uint32_t) g_amec->wof.f_vote;
    }
    g_amec->wof.ceff = l_result32;

    // Try using the present voltage, since the voltage calculated
    // using the WOF frequency causes Ceff to be too low.
    // estimate g_amec->wof.v_chip^1.3 using equation:
    // = 21374 * (X in 0.1 mV) - 50615296
    l_result32v = (21374 * g_amec->wof.v_chip - 50615296) >> 10;
    l_result32 = l_result32i / l_result32v;
    l_result32 = l_result32 << 14; // * 16384
    if (g_amec->wof.f_vote != 0)
    {
        // avoid divide by 0
        l_result32 = l_result32 / (uint32_t) g_amec->wof.f_vote;
    }
    g_amec->wof.ceff_old = l_result32;

    g_amec->wof.ceff_ratio = g_amec->wof.ceff_old * 10000 / g_amec->wof.ceff_tdp;
    if (g_amec->wof.ceff_ratio > 10000)
    {
        // max freq must be turbo or higher by design
        g_amec->wof.ceff_ratio = 10000;
    }
    sensor_update(AMECSENSOR_PTR(WOFCEFFRATIO),g_amec->wof.ceff_ratio);

    // Step 5: frequency uplift table
    // Search table and point i to the lower entry the target value falls
    // between.
    for (i=1; i<AMEC_WOF_UPLIFT_TBL_ROWS-1; i++)
    {
        if (G_amec_wof_uplift_table[i][0] <= (int16_t) g_amec->wof.ceff_ratio &&
            G_amec_wof_uplift_table[i+1][0] >= (int16_t)g_amec->wof.ceff_ratio)
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
        //FIXME: add rounding step after multiplication
        //FIXME: pre-compute m, since table is static
        l_result32 = ((int32_t)g_amec->wof.ceff_ratio - (int32_t)G_amec_wof_uplift_table[i][0])
            * ((int32_t)G_amec_wof_uplift_table[i+1][l_pstatetable_cores_next] -
               (int32_t)G_amec_wof_uplift_table[i][l_pstatetable_cores_next])
            / ((int32_t)G_amec_wof_uplift_table[i+1][0] - (int32_t)G_amec_wof_uplift_table[i][0])
            + (int32_t)G_amec_wof_uplift_table[i][l_pstatetable_cores_next];
    }

    g_amec->wof.f_uplift = l_result32;
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
    amec_wof_common_steps();

    g_amec->wof.f_vote = g_amec->wof.f_uplift;

    if (g_amec->wof.pstatetable_cores_next >
        g_amec->wof.pstatetable_cores_current)
    {
        // Cores turning on.
        g_amec->wof.state = AMEC_WOF_CORE_REQUEST_TURN_ON;
    }
    else if (g_amec->wof.pstatetable_cores_next <
             g_amec->wof.pstatetable_cores_current)
    {
        // Cores turning off.
        g_amec->wof.state = AMEC_WOF_CORE_REQUEST_TURN_OFF;
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

    TRAC_INFO("Updating Global Pstate table for WOF: cores_current[%d] cores_next[%d]",
              g_amec->wof.pstatetable_cores_current,
              g_amec->wof.pstatetable_cores_next);

    // STEP 1:
    // Prevent any Pstate changes by locking the PMC Rail so that
    // Pmax_rail = Pmin_rail + 1
    l_pmin = gpst_pmin(&G_global_pstate_table) + 1;

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
    amec_wof_common_steps();

    // Determine if we have cores that are turning on or off
    if (g_amec->wof.pstatetable_cores_next >
        g_amec->wof.pstatetable_cores_current)
    {
        // Cores turning on.  Need to increase Pstate voltage.
        // Move frequency to safe region so turbo voltage can be optimized.
        // Go to Turbo - 1 MHz so we get Pstate below Turbo.
        g_amec->wof.f_vote =
            G_sysConfigData.sys_mode_freq.table[OCC_MODE_STURBO] - 1;

        // Update Pstate table in SRAM
        amec_wof_update_pstate_table();

        // Signal that a new Pstate table generation is ready
        g_amec_wof_pstate_table_ready = 1;

        g_amec->wof.state = AMEC_WOF_CORE_REQUEST_TURN_ON;

        //For now, don't use a separate thread
        //ssx_semaphore_post(&G_amecWOFThreadWakeupSem);
    }
    else if (g_amec->wof.pstatetable_cores_next <
             g_amec->wof.pstatetable_cores_current)
    {
        // Cores turning off. Decrease p-state voltage.
        // Move frequency to safe region so turbo voltage can be optimized.
        // Go to Turbo - 1 MHz so we get p-state below Turbo.
        g_amec->wof.f_vote =
            G_sysConfigData.sys_mode_freq.table[OCC_MODE_STURBO] - 1;

        // Update Pstate table in SRAM
        amec_wof_update_pstate_table();

        // Signal that a new Pstate table generation is ready
        g_amec_wof_pstate_table_ready = 1;

        g_amec->wof.state = AMEC_WOF_CORE_REQUEST_TURN_OFF;

        //For now, don't use a separate thread
        //ssx_semaphore_post(&G_amecWOFThreadWakeupSem);
    }
    else
    {
        // No change.  Just adjust WOF frequency.
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
// Thread: RealTime Loop
//
// End Function Specification
void amec_wof_helper_v3(void)
{
    uint64_t            l_data64 = 0;
    uint32_t            l_rc = 0;

    switch (g_amec->wof.state)
    {
        case AMEC_WOF_NO_CORE_CHANGE:
        {
            // Apply the WOF vote in 2ms WOF procedure.  Assume Pstate tables
            // are already set up
            g_amec_wof_pstate_table_ready = 0;
            break;
        }

        case AMEC_WOF_CORE_REQUEST_TURN_ON:
        {
            // We need to be certain that the GPE has applied the last frequency
            // request from the voting box (request != actual)
            if (g_amec->proc[0].core_max_freq_actual != g_amec->proc[0].core_max_freq)
            {
                break;
            }
            // Check if actual clips are above WOF frequency
            if (g_amec->proc[0].core_max_freq_actual > g_amec->wof.f_vote)
            {
                break;
            }
            // Wait until Pstate table is ready
            if (g_amec_wof_pstate_table_ready == 0)
            {
                break;
            }

            // If we reach this point, then we can be assured that
            // 1) GPE has applied the latest frequency selection, including WOF vote
            // 2) The true frequency is not above the WOF frequency
            // 3) The p-state table is ready

            // FIXME: Turn on the p-state table (flip address of table)
            // FIXME: new WOF vote

            // Now that new Pstate table is installed, we can allow WOF 
            // frequency request
            g_amec->wof.f_vote = g_amec->wof.f_uplift;

            g_amec->wof.state = AMEC_WOF_TRANSITION;

            // FALL THROUGH
        }

        case AMEC_WOF_TRANSITION:
        {
            // Signal waking cores to turn on and go back to initial state.

            // Quickly toggle waking core inhibit bits. This will allow cores
            // that want to wake up to actually wake up.
            l_rc = _putscom(PDEMR, ~g_amec->wof.wake_up_mask, SCOM_TIMEOUT);
            if (l_rc != 0)
            {
                g_amec->wof.error = AMEC_WOF_ERROR_SCOM_2;
                break;
            }

            // Now, go back to inhibiting all cores
            l_data64 = 0xffff000000000000ull;
            l_rc = _putscom(PDEMR, l_data64, SCOM_TIMEOUT);
            if (l_rc != 0)
            {
                g_amec->wof.error = AMEC_WOF_ERROR_SCOM_3;
                break;
            }

            g_amec->wof.pstatetable_cores_current =
                g_amec->wof.pstatetable_cores_next;
            g_amec->wof.state = AMEC_WOF_NO_CORE_CHANGE;

            break;
        }

        case AMEC_WOF_CORE_REQUEST_TURN_OFF:
        {
            // Check WOF frequency is applied
            // Check if GPE is applying a new frequency (request != actual)
            if (g_amec->proc[0].core_max_freq_actual != g_amec->proc[0].core_max_freq)
            {
                break;
            }
            // Check if actual clips are above WOF frequency
            if (g_amec->proc[0].core_max_freq_actual > g_amec->wof.f_vote)
            {
                break;
            }
            // Wait until Pstate table is ready
            if (g_amec_wof_pstate_table_ready == 0)
            {
                break;
            }

            // If we reach this point, then we can be assured that
            // 1) GPE has applied the latest frequency selection, including WOF vote
            // 2) The true frequency is not above the WOF frequency
            // 3) The p-state table is ready

            // FIXME: Turn on the p-state table (flip address of table)
            // FIXME: new WOF vote

            // Now that the new Pstate table is installed, we can allow WOF
            // turbo frequency
            g_amec->wof.f_vote = g_amec->wof.f_uplift;

            g_amec->wof.pstatetable_cores_current =
                g_amec->wof.pstatetable_cores_next;
            g_amec->wof.state = AMEC_WOF_NO_CORE_CHANGE;
            break;
        }

        default:
            g_amec->wof.error = AMEC_WOF_ERROR_UNKNOWN_STATE;
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
    uint64_t            l_data64 = 0;
    uint32_t            l_rc = 0;

    switch (g_amec->wof.state)
    {
        case AMEC_WOF_NO_CORE_CHANGE:
        {
            // Apply the WOF vote in 2ms WOF procedure.  Assume p-state tables
            // are already set up
            break;
        }

        case AMEC_WOF_CORE_REQUEST_TURN_ON:
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
            l_rc = _putscom(PDEMR, ~g_amec->wof.wake_up_mask, SCOM_TIMEOUT);
            if (l_rc != 0)
            {
                g_amec->wof.error = AMEC_WOF_ERROR_SCOM_2;
                break;
            }

            // Next, inhibit all cores again
            l_data64 = 0xffff000000000000ull;
            l_rc = _putscom(PDEMR, l_data64, SCOM_TIMEOUT);
            if (l_rc != 0)
            {
                g_amec->wof.error = AMEC_WOF_ERROR_SCOM_3;
                break;
            }

            g_amec->wof.pstatetable_cores_current =
                g_amec->wof.pstatetable_cores_next;
            g_amec->wof.state = AMEC_WOF_NO_CORE_CHANGE;
            break;
        }

        case AMEC_WOF_CORE_REQUEST_TURN_OFF:
        {
            // Go to state 0.
            g_amec->wof.pstatetable_cores_current =
                g_amec->wof.pstatetable_cores_next;
            g_amec->wof.state = AMEC_WOF_NO_CORE_CHANGE;
            break;
        }

        default:
            g_amec->wof.error = AMEC_WOF_ERROR_UNKNOWN_STATE;
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
    uint8_t             l_err = 0;

    // If new algorithm selected, then initialize it. If initialization fails,
    // then do not run algorithm
    if (g_amec->wof.enable_parm != g_amec->wof.algo_type)
    {
        l_err = amec_wof_set_algorithm(g_amec->wof.enable_parm);
        if (l_err)
        {
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
            else if (i_tblType == AMEC_WOF_VRM_EFF_TBL)
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
// Description: Parses and handles all data passed to OCC via data
//              config format 0x30.
//
// Prereq: Data size has been verified.
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
