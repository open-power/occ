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

//
// Data about tul237 prototype
//

// $ svpdMFGtool --cc=pf
// RID: 0x1000,  CCIN: 54BC
// RID: 0x1001,  CCIN: 54BC
// RID: 0x1002,  CCIN: 54BC
// RID: 0x1003,  CCIN: 54BC

// $ findchips_nc
// Starting findchips...
// Probing SCAN Engines
// /dev/scan/L02C0E13:L3C0E12:L3C0E03P00> CENTAUR-DD2.0 (0x260E9049)
// /dev/scan/L02C0E13:L3C0E12:L2C0E03P00> CENTAUR-DD2.0 (0x260E9049)
// /dev/scan/L02C0E13:L3C0E12:L1C0E03P00> CENTAUR-DD2.0 (0x260E9049)
// /dev/scan/L02C0E13:L3C0E12:L0C0E03P00> CENTAUR-DD2.0 (0x260E9049)
// /dev/scan/L02C0E13:L3C0E03P00> MURANO-DD2.0 (0x220EF049)
// /dev/scan/L02C0E13:L2C0E12:L3C0E03P00> CENTAUR-DD2.0 (0x260E9049)
// /dev/scan/L02C0E13:L2C0E12:L2C0E03P00> CENTAUR-DD2.0 (0x260E9049)
// /dev/scan/L02C0E13:L2C0E12:L1C0E03P00> CENTAUR-DD2.0 (0x260E9049)
// /dev/scan/L02C0E13:L2C0E12:L0C0E03P00> CENTAUR-DD2.0 (0x260E9049)
// /dev/scan/L02C0E13:L2C0E03P00> MURANO-DD2.0 (0x220EF049)
// /dev/scan/L02C0E13:L1C0E12:L3C0E03P00> CENTAUR-DD2.0 (0x260E9049)
// /dev/scan/L02C0E13:L1C0E12:L2C0E03P00> CENTAUR-DD2.0 (0x260E9049)
// /dev/scan/L02C0E13:L1C0E12:L1C0E03P00> CENTAUR-DD2.0 (0x260E9049)
// /dev/scan/L02C0E13:L1C0E12:L0C0E03P00> CENTAUR-DD2.0 (0x260E9049)
// /dev/scan/L02C0E13:L1C0E03P00> MURANO-DD2.0 (0x220EF049)
// /dev/scan/L02C0E12:L3C0E03P00> CENTAUR-DD2.0 (0x260E9049)
// /dev/scan/L02C0E12:L2C0E03P00> CENTAUR-DD2.0 (0x260E9049)
// /dev/scan/L02C0E12:L1C0E03P00> CENTAUR-DD2.0 (0x260E9049)
// /dev/scan/L02C0E12:L0C0E03P00> CENTAUR-DD2.0 (0x260E9049)
// /dev/scan/L02C0E03P00> MURANO-DD2.0 (0x220EF049)
// scandir: No such file or directory
// ...done

//tul237 data
/*
$ mnfgGetNominalFreqValues -r=1000 -R=LRP4

RID: 1000, Record Name: LRP4
Nominal Frequency       3524 MHz
Nominal Nest Voltage    1000 mV
Nominal Nest Current    72 A
Nominal CS Voltage      1100 mV
Nominal CS Current      7 A
PowerSave Frequency     2028 MHz
PowerSave Nest Voltage  940 mV
PowerSave Nest Current  60 A
PowerSave CS Voltage    1040 mV
PowerSave CS Current    6 A
Turbo Frequency         3724 MHz
Turbo Nest Voltage      1050 mV
Turbo Nest Current      80 A
Turbo CS Voltage        1150 mV
Turbo CS Current        8 A
CPmin Frequency         2268 MHz
CPmin Nest Voltage      960 mV
CPmin Nest Current      60 A
CPmin CS Voltage        1050 mV
CPmin CS Current        6 A
Test Frequency          4508 MHz
Test IO Voltage         1100 mV
Test IO Current         80 A
Test PCIe Voltage       1190 mV
Test PCIe Current       8 A

*/

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <proc_data.h> // for CORE_PRESENT
#include <errl.h>               // Error logging
#include <sensor.h>
#include <amec_sys.h>
#include <cmdh_fsp_cmds_datacnfg.h>
#include <ssx_api.h> // SsxSemaphore
#include <amec_wof.h> // For wof semaphore in amec_wof_thread.c
#include <common.h>

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

//
//SCOM address
//
//PMC Winkle Interrupt Request Vector Register (PMCWIRVR3)
#define PMCWIRVR3 0x6208b
//The PDEMR register masks the ability for a given chiplet to exit from Deep Sleep or Winkle state. NOTE that this register is to be used by the OCC for implementing power shift algorithms
#define PDEMR 0x62092

// For prototype, one and only one of the following defines must be 1
#define HAB19
//#define TUL183
/* Need to update with CEFF calcs from Tul183 10 core */
//#define TUL183_5_CORE_PER_CHIP
//#define TUL237


//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

sensor_t g_amec_wof_ceff_ratio_sensor;
sensor_t g_amec_wof_core_wake_sensor;
sensor_t g_amec_wof_vdd_sense_sensor;

#ifdef HAB19

#ifdef WOF_SYSTEM
#error "WOF algorithm already defined"
#endif
#define WOF_SYSTEM 1

// From 2015-03-12 WOF meeting
#define AMEC_WOF_LOADLINE_ACTIVE 550  // Active loadline in micro ohms
#define AMEC_WOF_LOADLINE_PASSIVE 0 // Passive loadline in micro ohms

#define WOF_MAX_CORES_PER_CHIP 12

//
// Need to update 1.05 (raw to tdp conversion) for Wyatt to be
//  1.12 for Turismo
//

// NOTE: Prototype uses Nominal operation point for C_EFF_TDP, since P8 prototype does not have ultra-turbo
//
//   Estimate IDDQ@RDP(95C)@Vnom P0 = IDDQ@85C * 1.25 ^ ([95-85]/10) = 35.84 * 1.25 = 44.8 A
//
//   NM_Idd@RDP P0 = 151 A
//
//   P0: (151 A - 44.8 A) / 1.22 * 1.05 = 91.40 A TDP
//
//   C_eff    = I / (V^3 * F)
//   C_eff_tdp_P0 = 91.40 A / (0.9 V ^1.3) / 2561 MHz = 0.0409 microF =  40.9 nF
//
//   C_EFF_TDP_P0 = 9140 (0.01A) * 16384 / 9000(100uV)^1.3 * 16384 / 2561(MHz)
//                = 6932
//
//   Note: C_EFF_TDP_P0 / 100 / 16384 / 16384 * 10000^1.3 * 1000
//                = C_eff_tdp_P0 in nF
//

// 0.01 A units of RDP@Vnom
uint16_t g_amec_wof_rdp_idd_nom[MAX_NUM_CHIP_MODULES] = {15700, 0, 0, 0};

// Effective capacitance for TDP workload @ Turbo. FIXME: Put in p-state table superstructure.
// 4 modules max. Value 1 to avoid divide by 0.
uint32_t g_amec_wof_ceff_tdp_module[MAX_NUM_CHIP_MODULES] = {6932, 1, 1, 1};

//Based on DMIW data for tul237P0 ID=B1935398. I extrapolate beyond 1.0V
uint16_t amec_wof_iddq_table[][5] = {
    //0.0001 V, 0.01 A x4 (for 4 modules).  Data corrected to 85C conditions.
    {9000,   3584, 0, 0, 0},
    {10000,  5194, 0, 0, 0},
    {11000,  7507, 0, 0, 0},
    {12000, 10734, 0, 0, 0},
    {12500, 12696, 0, 0, 0}
};
#define AMEC_WOF_IDDQ_TABLE_N 5

//Table from Victor on 3/26/2015 assuming Nominal-UltraTurbo range and deep-winkle.
// 14 is for 1 column of ratio, 13 columns of uplift by # cores turned on
int16_t amec_wof_uplift_table[][14] = {
    // First column is the Ceff ratio. The other colums are clock speed in MHz.
    // Make sure ratio=0 is here, since search algorithm expects input index to be >= first table element.
    //    Number of active cores
    //    0     1     2     3     4     5     6     7     8     9     10    11    12
    {  0, 4023, 4023, 4023, 4023, 4023, 4023, 4023, 4023, 4023, 4023, 4023, 4023, 4023 },
    { 50, 4023, 4023, 4023, 4023, 4023, 4023, 3993, 3875, 3757, 3629, 3486, 3347, 3224 },
    { 55, 4023, 4023, 4023, 4023, 4023, 4023, 3952, 3829, 3703, 3555, 3404, 3268, 3142 },
    { 60, 4023, 4023, 4023, 4023, 4023, 4023, 3913, 3783, 3642, 3478, 3327, 3191, 3066 },
    { 65, 4023, 4023, 4023, 4023, 4023, 4018, 3875, 3736, 3575, 3404, 3252, 3117, 2994 },
    { 70, 4023, 4023, 4023, 4023, 4023, 3985, 3836, 3685, 3506, 3334, 3183, 3048, 2925 },
    { 75, 4023, 4023, 4023, 4023, 4023, 3949, 3795, 3629, 3439, 3268, 3117, 2981, 2858 },
    { 80, 4023, 4023, 4023, 4023, 4023, 3916, 3754, 3570, 3375, 3204, 3053, 2920, 2797 },
    { 85, 4023, 4023, 4023, 4023, 4023, 3885, 3711, 3509, 3314, 3142, 2994, 2858, 2735 },
    { 90, 4023, 4023, 4023, 4023, 4023, 3852, 3665, 3450, 3255, 3083, 2935, 2799, 2674 },
    { 95, 4023, 4023, 4023, 4023, 4005, 3816, 3616, 3391, 3199, 3027, 2879, 2743, 2617 },
    {100, 4023, 4023, 4023, 4023, 3977, 3783, 3565, 3337, 3142, 2973, 2825, 2689, 2561 },

};

#define AMEC_WOF_UPLIFT_TABLE_N 12

#endif //HAB19

#ifdef TUL183

#ifdef WOF_SYSTEM
#error "WOF algorithm already defined"
#endif
#define WOF_SYSTEM 1

//  Tuleta 2u and 4u use this loadline:
//    Vreg = Vvpd + Ivpd * (R_line + R_drop)
//    vdd r_line = 570uOhm  aka Active Loadline (from V_vrm_out to Vsense in mnfgvapi -r) PROC_R_LOADLINE_VDD
//    vdd r_drop = 150uOhm  aka Passive Loadline  (to in2Hdr: cheesit name and test point) PROC_R_DISTLOSS_VDD
//    vcs r_line = 570uOhm
//    vcs r_drop = 1400uOhm
//
//    r_sum = 720uOhm

// Early Feb 2015 meeting decision was made to use Vdd_rdrop of 120uOhm
// Therefore, r_sum = 690uOhm

#define AMEC_WOF_LOADLINE_ACTIVE 750  // Active loadline in micro ohms
#define AMEC_WOF_LOADLINE_PASSIVE 120 // Passive loadline in micro ohms

#define WOF_MAX_CORES_PER_CHIP 10

// NOTE: Prototype uses Nominal operation point for C_EFF_TDP, since P8 prototype does not have ultra-turbo
// C_EFF_TDP for tul183p0: I=14800 (in 0.01 A), V=9500 (=1V in 100 microV), F=3524 (MHz). V^1.3=
//     C_EFF_TDP = I * 10,000 / V^1.3 * 10,000 / F) = 1289.
// Error in above (didn't subtract out leakage current and then apply victors math to adjust the RDP AC to TDP AC)
//
//   Estimate IDDQ@RDP(95C)@Vnom P0 = 18.87 A for 10 core (From Frank's 2-degree poly trend)
//   Estimate IDDQ@RDP(95C)@Vnom P1 = 21.77 A for 10 core
//
//   NM_Idd@RDP P0 = 148 A
//   NM_Idd@RDP P1 = 147 A
//
//   P0: (148A - 18.87 A) / 1.22 * 1.05 = 111.14 A TDP
//   P1: (147A - 21.77 A) / 1.22 * 1.05 = 107.78 A TDP
//
//   C_eff    = I / (V^3 * F)
//   C_eff_tdp_P0 = 111.14 A / (0.950 V ^1.3) / 3425 MHz =  34.69 nF
//   C_eff_tdp_P1 = 107.78 A / (0.895 V ^1.3) / 3425 MHz =  36.35 nF
//
//   C_EFF_TDP_P0 = 11114(0.01A) * 10000/ 9500(100mV)^1.3 * 10000 / 3425(MHz) = 2189
//   C_EFF_TDP_P1 = 10778(0.01A) * 10000/ 8950(100mV)^1.3 * 10000 / 3425(MHz) = 2293
//
//   Note: C_EFF_TDP_P0 / 10000 / 10000 * 10000^1.3 == C_eff_tdp_P0
//

uint16_t g_amec_wof_rdp_idd_nom[MAX_NUM_CHIP_MODULES] = {14800, 14700, 0, 0}; // 0.01 A units of RDP@Vnom

// Effective capacitance for TDP workload @ Turbo. FIXME: Put in p-state table superstructure.
uint32_t g_amec_wof_ceff_tdp_module[MAX_NUM_CHIP_MODULES] = {2189, 2293, 1, 1}; // 4 modules max. Value 1 to avoid divide by 0.

//Based on DMIW data for tul237P0 ID=B1935398. I extrapolate beyond 1.0V
uint16_t amec_wof_iddq_table[][5] = {
    //0.0001 V, 0.01 A x4 (for 4 modules).  Data corrected to 85C conditions.
    //10 core data (says Frank)
    {9000,  1248, 1793, 0, 0},
    {10000, 1831, 2618, 0, 0},
    {11000, 2753, 3903, 0, 0},
    {12000, 4222, 5861, 0, 0},
    {12500, 5223, 7161, 0, 0}

    //12 core data (says Frank)
//     {9000, 1498, 2151, 0, 0},
//     {10000, 2197, 3142, 0, 0},
//     {11000, 3303, 4683, 0, 0},
//     {12000, 5066, 7033, 0, 0},
//     {12500, 6268, 8593, 0, 0}
};
#define AMEC_WOF_IDDQ_TABLE_N 5

// 13 is for 1 column of ratio, 12 columns of uplift by # cores turned on
int16_t amec_wof_uplift_table[][14] = {
    //ratio*100, frequency offsets
    //{0, -100},
    //{90, -100},
    //{100, 0},
    //{110, 100},
    //{120, 200}

    // From Victor's e-mail of 12/03/2014
    // Make sure 0 is here, since search algorithm expects input index to be >= first table element.
    //    0     1     2     3     4     5     6     7     8     9     10    11    12
    {  0, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, },
    { 75, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, },
    { 80, 3665, 3665, 3665, 3665, 3665, 3665, 3665, 3665, 3665, 3665, 3665, 3665, 3665, },
    { 85, 3596, 3596, 3596, 3596, 3596, 3596, 3596, 3596, 3596, 3596, 3596, 3596, 3596, },
    { 90, 3528, 3528, 3528, 3528, 3528, 3528, 3528, 3528, 3528, 3528, 3528, 3528, 3528, },
    { 95, 3494, 3494, 3494, 3494, 3494, 3494, 3494, 3494, 3494, 3494, 3494, 3494, 3494, },
    {100, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, },
    {110, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, },
    {120, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, }

};
#define AMEC_WOF_UPLIFT_TABLE_N 8

#endif //TUL183

#ifdef TUL183_5_CORE_PER_CHIP

#ifdef WOF_SYSTEM
#error "WOF algorithm already defined"
#endif
#define WOF_SYSTEM 1

#define AMEC_WOF_LOADLINE_ACTIVE 750  // Active loadline in micro ohms
#define AMEC_WOF_LOADLINE_PASSIVE 120 // Passive loadline in micro ohms

#define WOF_MAX_CORES_PER_CHIP 5

uint16_t g_amec_wof_rdp_idd_nom[MAX_NUM_CHIP_MODULES] = {74, 73, 0, 0};

// For 5 cores/chip, use 50% of 10 cores/chip.
uint32_t g_amec_wof_ceff_tdp_module[MAX_NUM_CHIP_MODULES] = {1095, 1146, 1, 1}; // 4 modules max. Value 1 to avoid divide by 0.

//Based on DMIW data for tul237P0 ID=B1935398. I extrapolate beyond 1.0V
uint16_t amec_wof_iddq_table[][5] = {
    //0.0001 V, 0.01 A x4 (for 4 modules).  Data corrected to 85C conditions.
    // 50% of 10 core data
    {9000, 624, 897, 0, 0},
    {10000, 916, 1309, 0, 0},
    {11000, 1377, 1952, 0, 0},
    {12000, 2111, 2931, 0, 0},
    {12500, 2612, 3581, 0, 0}
};
#define AMEC_WOF_IDDQ_TABLE_N 5

// 14 is for 1 column of ratio, 13 columns of uplift by # cores turned on
int16_t amec_wof_uplift_table[][14] = {
    // From Victor's e-mail of 12/03/2014
    // Make sure 0 is here, since search algorithm expects input index to be >= first table element.
    // For 5 core, just increase frequency by 66.5MHz per core in winkle for testing.
    // Make sure 0 is here, since search algorithm expects input index to be >= first table element.
    //    0     1     2     3     4     5     6     7     8     9     10    11    12
    {  0, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, },
    { 75, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, 3699, },
    { 80, 3699, 3699, 3669, 3699, 3699, 3665, 3665, 3665, 3665, 3665, 3665, 3665, 3665, },
    { 85, 3699, 3699, 3699, 3699, 3665, 3596, 3596, 3596, 3596, 3596, 3596, 3596, 3596, },
    { 90, 3699, 3699, 3699, 3665, 3596, 3528, 3528, 3528, 3528, 3528, 3528, 3528, 3528, },
    { 95, 3699, 3699, 3665, 3596, 3528, 3494, 3494, 3494, 3494, 3494, 3494, 3494, 3494, },
    {100, 3665, 3665, 3596, 3528, 3494, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, },
    {110, 3665, 3665, 3596, 3528, 3494, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, },
    {120, 3665, 3665, 3596, 3528, 3494, 3425, 3425, 3425, 3425, 3425, 3425, 3425, 3425, }
};
#define AMEC_WOF_UPLIFT_TABLE_N 8

#endif //TUL183_5_CORE_PER_CHIP


#ifdef TUL237

#ifdef WOF_SYSTEM
#error "WOF algorithm already defined"
#endif
#define WOF_SYSTEM 1

#define AMEC_WOF_LOADLINE_ACTIVE 750  // Active loadline in micro ohms
#define AMEC_WOF_LOADLINE_PASSIVE 120 // Passive loadline in micro ohms

// NOTE: Prototype uses Nominal operation point for C_EFF_TDP, since P8 prototype does not have ultra-turbo
// C_EFF_TDP for tul237p0: I=7200 (in 0.01 A), V=10000 (=1V in 100 microV), F=3425 (MHz). V^1.3=158489
//     C_EFF_TDP = I * 10,000 / V^1.3 * 10,000 / F) = 1289.
// Error in above (didn't subtract out leakage current and then apply victors math to adjust the RDP AC to TDP AC)
//
#define AMEC_WOF_C_EFF_TDP 1289


//Based on DMIW data for tul237P0 ID=B1935398. I extrapolate beyond 1.0V
uint16_t amec_wof_iddq_table[][5] = {
    //0.0001 V, 0.01 A.  Data corrected to 85C conditions.
    {8000, 1914, 0, 0, 0},
    {9000, 2769, 0, 0, 0},
    {10000, 3905, 0, 0, 0},
    {11000, 5195, 0, 0, 0},
    {12000, 6395, 0, 0, 0},
    {12500, 6840, 0, 0, 0}
};
#define AMEC_WOF_IDDQ_TABLE_N 6

int16_t amec_wof_uplift_table[][2] = {
    //ratio*100, frequency offset
    //{0, -100},
    //{90, -100},
    //{100, 0},
    //{110, 100},
    //{120, 200}

    // Make sure 0 is here, since search algorithm expects input index to be >= first table element.
    {  0, 3724},
    { 80, 3724},
    { 90, 3624},
    {100, 3425},
    {110, 3425},
    {120, 3425}
};
#define AMEC_WOF_UPLIFT_TABLE_N 6

#endif //TUL237

/* A conversion table from Vdd regulator input power to conversion efficiency.
   Power is in units of dW (Watts * 10).
   Efficiency is in units of 1/100 of 1%.  (Divide by 10000 to get percent)
*/
uint32_t amec_wof_vdd_eff[][2] = {
    {124 ,  8018}, //0  (12.397 W, 80.18% efficiency)
    {318 ,  8804}, //1
    {417 ,  8889}, //2
    {515 ,  8937}, //3
    {608 ,  9031}, //4
    {702 ,  9065}, //5
    {789 ,  9155}, //6
    {895 ,  9021}, //7
    {989 ,  9022}, //8
    {1075,  9066}, //9
    {1182,  8940}, //10
    {1271,  8937}, //11
    {1369,  8875}, //12
    {1457,  8883}, //13
    {1549,  8851}, //14
    {1640,  8820}, //15
    {1734,  8773}, //16
    {1837,  8677}, //17
    {1914,  8705}, //18
    {2015,  8620}, //19
    {2093,  8640}, //20
};
#define AMEC_WOF_VDD_EFF_N 21   /*Number of amec_wof_add_eff entried*/



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

//Pstate g_amec_wof_vote_pstate; // pstate associate with wof vote
//uint8_t g_amec_wof_vote_vid; // VID associated with wof vote

//For testing updating the p-state table
//If wof_make_check != 0, then compute check byte for GPST[50] and place into g_amec_wof_check
uint8_t g_amec_wof_make_check=0;
uint8_t g_amec_wof_check=0;

//The current number of cores pstate table allows
uint8_t g_amec_wof_pstatetable_cores_current = MAX_NUM_CORES;
//The next pstate table max number of cores
uint8_t g_amec_wof_pstatetable_cores_next = MAX_NUM_CORES;

uint64_t g_amec_wof_wake_mask = 0;
uint64_t g_amec_wof_wake_mask_save = 0;

//Algorithm 3 (aka WOF 2.1)
GLOBAL_PSTATE_TABLE(g_amec_wof_pstate_table_0);
GLOBAL_PSTATE_TABLE(g_amec_wof_pstate_table_1);
uint8_t g_amec_wof_current_pstate_table = 0;
uint8_t g_amec_wof_pstate_table_ready = 0;

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

/*

        //Do across each operating point:
    //Calculate max load-line current to set voltage
    g_amec_wof_current_reduction = g_amec_wof_rdp_idd_nom[G_pob_id.module_id]
        * g_amec_wof_pstatetable_cores_next / WOF_MAX_CORES_PER_CHIP;


*/

void amec_wof_init(void)
{
    // Initialize DCOM Thread Sem
    ssx_semaphore_create( &G_amecWOFThreadWakeupSem, // Semaphore
                          1,                         // Initial Count
                          0);                        // No Max Count

    // Avoid recomputing total
    g_amec->wof.loadline = AMEC_WOF_LOADLINE_ACTIVE + AMEC_WOF_LOADLINE_PASSIVE;
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
    pmc_parameter_reg1_t l_ppr1;

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
        g_amec->wof.f_vote =
            G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO];

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

        // Set common things
        g_amec->wof.ceff_tdp = g_amec_wof_ceff_tdp_module[G_pob_id.module_id];

        switch(i_algorithm)
        {
            case 3:
                // Copy pstate-table to WOF
                // Move these to where G_global_pstate_table is initialized, so
                // it is not in real-time loop
                memcpy(&g_amec_wof_pstate_table_0, &G_global_pstate_table,
                       sizeof(GlobalPstateTable));
                memcpy(&g_amec_wof_pstate_table_1, &G_global_pstate_table,
                       sizeof(GlobalPstateTable));

                l_ppr1.value = in32(PMC_PARAMETER_REG1);
                l_ppr1.fields.ba_sram_pstate_table =
                        (unsigned long)&g_amec_wof_pstate_table_0 >> GLOBAL_PSTATE_TABLE_ALIGNMENT;
                l_ppr1.fields.pvsafe = g_amec_wof_pstate_table_0.pvsafe;
                out32(PMC_PARAMETER_REG1, l_ppr1.value);

                // FALL-THROUGH

            case 2: // WOF algorithm 2
                g_amec->wof.state = AMEC_WOF_NO_CORE_CHANGE; // WOF state
                // Set pstate table to MAX_NUM_CORES;
                // Conservatively assume all cores are on for next interval
                g_amec_wof_pstatetable_cores_current = MAX_NUM_CORES;
                g_amec_wof_pstatetable_cores_next = MAX_NUM_CORES;

                // FALL-THROUGH

            case 0: // No WOF
                // Do not inhibit core wakeup (in case of transition out of alg. 2)
                l_data64 = 0xffff000000000000ull;
                l_rc = _putscom(PDEMR, l_data64, SCOM_TIMEOUT);
                if (l_rc != 0)
                {
                    g_amec->wof.error = AMEC_WOF_ERROR_SCOM_4;
                }
                break;
        }

        // Success, set the new algorithm
        g_amec->wof.algo_type = g_amec->wof.enable_parm;
        l_rc = 0;

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
    uint8_t             i;
    uint32_t            l_result32; //temporary result
    uint16_t            l_pow_reg_input_dW = AMECSENSOR_PTR(PWR250USVDD0)->sample * 10; // convert to dW by *10.
    uint16_t            l_vdd_reg = AMECSENSOR_PTR(VOLT250USP0V0)->sample;
    uint32_t            l_pow_reg_output_mW;
    uint32_t            l_curr_output;
    uint32_t            l_v_droop;
    uint32_t            l_v_chip;

    /* Step 1 */

    // 1. Get PWR250USVDD0  (the input power to regulator)
    // 2. Look up efficiency using PWR250USVDD0 as index (and interpolate)
    // 3. Calculate output power = PWR250USVDD0 * efficiency
    // 4. Calculate output current = output power / Vdd set point
    /* Determine regulator efficiency */

    //Search table and point i to the lower entry the target value falls between.
    if (l_pow_reg_input_dW < amec_wof_vdd_eff[0][0])
    {
        i = 0; // value is before table starts. Use first two entries.
    }
    else
    {
        for (i=0; i<AMEC_WOF_VDD_EFF_N-1; i++)
        {
            if (amec_wof_vdd_eff[i][0] <= l_pow_reg_input_dW &&
                amec_wof_vdd_eff[i+1][0] >= l_pow_reg_input_dW)
            {
                break;
            }
        }
    }
    if (i >= AMEC_WOF_VDD_EFF_N - 1)
    {
        i = AMEC_WOF_VDD_EFF_N - 2;
    }

    //Linear interpolate using the neighboring entries.  y = m(x-x1)+y1   m=(y2-y1)/(x2-x1)
    //FIXME: add rounding step after multiplication
    //FIXME: pre-compute m, since table is static
    l_result32 = ((int32_t)l_pow_reg_input_dW - (int32_t)amec_wof_vdd_eff[i][0])
        * ((int32_t)amec_wof_vdd_eff[i+1][1] - (int32_t)amec_wof_vdd_eff[i][1])
        / ((int32_t)amec_wof_vdd_eff[i+1][0] - (int32_t)amec_wof_vdd_eff[i][0])
        + (int32_t)amec_wof_vdd_eff[i][1];

    // For Frank, just use 95% efficiency all the time
    // FIXME: Need to determine efficiency from tables sent by TMGT
    l_result32 = 9500;

    g_amec->wof.vdd_eff = l_result32;

    // Compute regulator output power.  out = in * efficiency
    //    in: min=0W max=300W = 3000dW
    //    eff: min=0 max=10000=100% (.01% units)
    //    p_out: max=3000dW * 10000 = 30,000,000 (dW*0.0001) < 2^25, fits in 25 bits
    l_pow_reg_output_mW = (uint32_t)l_pow_reg_input_dW * (uint32_t)l_result32;

    // Scale up p_out by 10x to give better resolution for the following division step
    //    p_out: max=30M (dW*0.0001) in 25 bits
    //    * 10    = 300M (dW*0.00001) in 29 bits
    l_pow_reg_output_mW *= 10;

    // Compute current out of regulator.  curr_out = power_out (*10 scaling factor) / voltage_out
    //    p_out: max=300M (dW*0.00001) in 29 bits
    //    v_out: min=5000 (0.0001 V)  max=16000(0.0001 V) in 14 bits
    //    i_out: max = 300M/5000 = 60000 (dW*0.00001/(0.0001V)= 0.01A), in 16 bits.
    // VOLT250USP0V0 in units of 0.0001 V = 0.1 mV. (multiply by 0.1 to get mV)
    l_curr_output = l_pow_reg_output_mW / l_vdd_reg;
    sensor_update(AMECSENSOR_PTR(CUR250USVDD0), l_curr_output);

    //1c. Compute Vdd load at chip
    //    V_reg = V_chip + I_chip * (R_loadline)
    //    V_droop = I_chip (0.01 A) * R_loadline (0.000001 ohm) => (in 0.00000001 V)
    //    V_droop = V_droop / 10000 => (in 0.0001 V)
    //    V_chip = V_reg - V_droop  => (in 0.0001 V)
    l_v_droop = (uint32_t) l_curr_output * (uint32_t) g_amec->wof.loadline / (uint32_t)10000;
    l_v_chip = l_vdd_reg - l_v_droop;
    g_amec->wof.v_chip = l_v_chip; // expose in parameter

    // Compute Vsense estimate for WOF validation
    l_v_droop = (uint32_t) l_curr_output * (uint32_t) AMEC_WOF_LOADLINE_ACTIVE / (uint32_t) 10000;
    l_v_chip = l_vdd_reg - l_v_droop;
    sensor_update(AMECSENSOR_PTR(WOF250USVDDS), l_v_chip);
}

void amec_wof_common_steps(void)
{
    uint8_t             i;
    uint32_t            l_result32; //temporary result
    uint32_t            l_result32v;
    uint8_t             l_cores_on = 0;
    uint8_t             l_cores_waking = 0;
    uint8_t             l_pstatetable_cores_next=0;
    uint64_t            l_data64; // For SCOM access
    uint32_t            l_rc;
    Pstate              l_wof_vote_pstate; // pstate that corresponds to wof
                                           // vote. Find associated voltage.
    uint8_t             l_wof_vote_vid; // Vdd regulator VID associated with WOF vote.

    //Step 1c
    uint32_t l_v_chip = g_amec->wof.v_chip; //from common

    //Step 2
    uint16_t l_temp = AMECSENSOR_PTR(TEMP2MSP0)->sample;

    //Step 3
    uint32_t l_accum = AMECSENSOR_PTR(CUR250USVDD0)->accumulator;

    //Step 4
    uint16_t l_freq = AMECSENSOR_PTR(FREQA2MSP0)->sample;

    if (g_amec->wof.state != AMEC_WOF_NO_CORE_CHANGE)
    {
        return;
    }
    if (g_amec_wof_pstatetable_cores_next != g_amec_wof_pstatetable_cores_current)
    {
        g_amec->wof.error = AMEC_WOF_ERROR_CORE_COUNT;
        return;
    }

    if (g_amec_wof_make_check)
    {
        g_amec_wof_check = gpstCheckByte(g_amec_wof_pstate_table_0.pstate[50].value);
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
                // FIXME: Prototype only uses deep winkle. Add deep sleep when
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

    l_rc = _getscom(PMCWIRVR3, &g_amec_wof_wake_mask, SCOM_TIMEOUT);
    if (l_rc != 0)
    {
        g_amec->wof.error = AMEC_WOF_ERROR_SCOM_1;
        return;
    }

    // Save non-zero wake mask for debugging
    if (g_amec_wof_wake_mask != 0)
    {
        g_amec_wof_wake_mask_save = g_amec_wof_wake_mask;
    }
    l_data64 = g_amec_wof_wake_mask >> 48;
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
    g_amec_wof_pstatetable_cores_next = l_pstatetable_cores_next;

    // Step 2

    // Search table and point i to the lower entry the target value falls
    // between.
    if (l_v_chip < amec_wof_iddq_table[0][0])
    {
        i=0; // voltage is lower than table, so use first two entries.
    }
    else
    {
        for (i=0; i<AMEC_WOF_IDDQ_TABLE_N-1; i++)
        {
            if (amec_wof_iddq_table[i][0] <= l_v_chip &&
                amec_wof_iddq_table[i+1][0] >= l_v_chip)
            {
                break;
            }
        }
    }
    if (i >= AMEC_WOF_IDDQ_TABLE_N - 1)
    {
        // Voltage is higher than table, so use last two entries.
        i = AMEC_WOF_IDDQ_TABLE_N - 2;
    }

    g_amec->wof.iddq_i = i;

    //Linear interpolate using the neighboring entries.  y = m(x-x1)+y1   m=(y2-y1)/(x2-x1)
    //FIXME: add rounding step after multiplication
    //FIXME: pre-compute m, since table is static
    l_result32 = ((int32_t)l_v_chip - (int32_t)amec_wof_iddq_table[i][0])
    * ((int32_t)amec_wof_iddq_table[i+1][G_pob_id.module_id+1] - (int32_t)amec_wof_iddq_table[i][G_pob_id.module_id+1])
    / ((int32_t)amec_wof_iddq_table[i+1][0] - (int32_t)amec_wof_iddq_table[i][0])
    + (int32_t)amec_wof_iddq_table[i][G_pob_id.module_id+1];

    l_result32 = l_result32 * l_cores_on / WOF_MAX_CORES_PER_CHIP;

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

    // Step 3: Compute AC portion of chip Vdd current
    g_amec->wof.ac = g_amec->wof.cur_out - l_result32;  // Units of 0.01 A.

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

    l_result32 = g_amec->wof.ac << 14; // * 16384
    // estimate g_amec->wof.v_chip^1.3 using equation:
    // = 21374 * (X in 0.1 mV) - 50615296
    l_result32v = (21374 * g_amec->wof.vote_vchip - 50615296) >> 10;
    l_result32 = l_result32 / l_result32v;
    l_result32 = l_result32 << 14; // * 16384
    if (g_amec->wof.f_vote != 0)
    {
        // avoid divide by 0
        l_result32 = l_result32 / (uint32_t) g_amec->wof.f_vote;
    }
    g_amec->wof.ceff = l_result32;

    // Previous c_eff calcualtion (before Josh/Victor correction)
    l_result32 = g_amec->wof.ac << 14; // * 16384
    // estimate g_amec->wof.v_chip^1.3 using equation:
    // = 21374 * (X in 0.1 mV) - 50615296
    l_result32v = (21374 * g_amec->wof.v_chip - 50615296) >> 10;
    l_result32 = l_result32 / l_result32v;
    l_result32 = l_result32 << 14; // * 16384
    if (l_freq != 0)
    {
        // avoid divide by 0
        l_result32 = l_result32 / (uint32_t) l_freq;
    }
    g_amec->wof.ceff_old = l_result32;

    g_amec->wof.ceff_ratio = l_result32 * 100 / g_amec->wof.ceff_tdp;
    if (g_amec->wof.ceff_ratio > 100)
    {
        // max freq must be turbo or higher by design
        g_amec->wof.ceff_ratio = 100;
    }
    sensor_update(AMECSENSOR_PTR(WOFCEFFRATIO),g_amec->wof.ceff_ratio);

    // Step 5: frequency uplift table
    // Search table and point i to the lower entry the target value falls
    // between.
    for (i=0; i<AMEC_WOF_UPLIFT_TABLE_N-1; i++)
    {
        if (amec_wof_uplift_table[i][0] <= (int16_t) g_amec->wof.ceff_ratio &&
            amec_wof_uplift_table[i+1][0] >= (int16_t)g_amec->wof.ceff_ratio)
        {
            break;
        }
    }

    if (i >= AMEC_WOF_UPLIFT_TABLE_N - 1)
    {
        // Out of table, so clip to 100%
        l_result32 = amec_wof_uplift_table[AMEC_WOF_UPLIFT_TABLE_N - 1][l_pstatetable_cores_next+1];
    }
    else
    {
        // Ratio is within uplift table

        //Linear interpolate using the neighboring entries.  y = m(x-x1)+y1   m=(y2-y1)/(x2-x1)
        //FIXME: add rounding step after multiplication
        //FIXME: pre-compute m, since table is static
        l_result32 = ((int32_t)g_amec->wof.ceff_ratio - (int32_t)amec_wof_uplift_table[i][0])
            * ((int32_t)amec_wof_uplift_table[i+1][l_pstatetable_cores_next+1] - (int32_t)amec_wof_uplift_table[i][l_pstatetable_cores_next+1])
            / ((int32_t)amec_wof_uplift_table[i+1][0] - (int32_t)amec_wof_uplift_table[i][0])
            + (int32_t)amec_wof_uplift_table[i][l_pstatetable_cores_next+1];
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

    if (g_amec_wof_pstatetable_cores_next > g_amec_wof_pstatetable_cores_current)
    {
        // Cores turning on.
        g_amec->wof.state = AMEC_WOF_CORE_REQUEST_TURN_ON;
    }
    else if (g_amec_wof_pstatetable_cores_next < g_amec_wof_pstatetable_cores_current)
    {
        // Cores turning off.
        g_amec->wof.state = AMEC_WOF_CORE_REQUEST_TURN_OFF;
    }
}


// Function Specification
//
// Name: amec_wof_v3
//
// Description: Run WOF version 2. Core-count aware. Re-calculate load-line.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_wof_alg_v3(void)
{
    amec_wof_common_steps();

    // calculate current reduction from all cores on

    if (g_amec_wof_pstatetable_cores_next > g_amec_wof_pstatetable_cores_current)
    {
        // Cores turning on.  Increase p-state voltage.
        // Move frequency to safe region so turbo voltage can be optimized.
        // Go to Turbo - 1 MHz so we get p-state below Turbo.
        g_amec->wof.f_vote = G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL] - 1; // FIXME: turbo

        g_amec_wof_pstate_table_ready = 0; // signal new pstate table generation required
        ssx_semaphore_post(&G_amecWOFThreadWakeupSem);
        g_amec->wof.state = AMEC_WOF_CORE_REQUEST_TURN_ON;
    }
    else if (g_amec_wof_pstatetable_cores_next < g_amec_wof_pstatetable_cores_current)
    {
        // Cores turning off. Decrease p-state voltage.
        // Move frequency to safe region so turbo voltage can be optimized.
        // Go to Turbo - 1 MHz so we get p-state below Turbo.
        g_amec->wof.f_vote = G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL] - 1; // FIXME: turbo
        g_amec_wof_pstate_table_ready = 0; // signal new pstate table generation required
        ssx_semaphore_post(&G_amecWOFThreadWakeupSem);
        g_amec->wof.state = AMEC_WOF_CORE_REQUEST_TURN_OFF;
    }
    else
    {
        // No change.  Just adjust WOF frequency.
        g_amec->wof.f_vote = g_amec->wof.f_uplift;
    }
}


// Run this in every 250us state. Allows cores to wake-up as soon as WOF outcome is ready.
void amec_wof_helper_v3(void)
{
    uint64_t            l_data64;
    uint32_t            l_rc;

    switch (g_amec->wof.state)
    {
        case 0:
        {
            // Apply the WOF vote in 2ms WOF procedure.  Assume p-state tables are already set up
            break;
        }

        case 1:
        {
            // Check WOF frequency is applied
            // Check if GPE is applying a new frequency (request != actual)
            if (g_amec->proc[0].core_max_freq_actual != g_amec->proc[0].core_max_freq) break;
            // Check if actual clips are above WOF frequency
            if (g_amec->proc[0].core_max_freq_actual > g_amec->wof.f_vote) break;
            // Wait until p-state table is ready
            if (g_amec_wof_pstate_table_ready == 0) break;

            // If we reach this point, then we can be assured that
            // 1) GPE has applied the latest frequency selection, including WOF vote
            // 2) The true frequency is not above the WOF frequency
            // 3) The p-state table is ready

            // FIXME: Turn on the p-state table (flip address of table)
            // FIXME: new WOF vote

            // Now new table is installed, so allow WOF turbo frequency
            g_amec->wof.f_vote = g_amec->wof.f_uplift;

            g_amec->wof.state = AMEC_WOF_TRANSITION;

            // FALL THROUGH
        }

        case 2:
        {
            // Signal waking cores to turn on. Go to state 0.

            //Quickly toggle waking core inhibit bits
            //Uninhibit cores waking up
            l_rc = _putscom(PDEMR, ~g_amec_wof_wake_mask, SCOM_TIMEOUT);
            if (l_rc != 0) {
                g_amec->wof.error = AMEC_WOF_ERROR_SCOM_2;
                break;
            }
            //Inhibit all cores
            //putscom pu 62092 ffff000000000000
            l_data64 = 0xffff000000000000ull;
            l_rc = _putscom(PDEMR, l_data64, SCOM_TIMEOUT);
            if (l_rc != 0) {
                g_amec->wof.error = AMEC_WOF_ERROR_SCOM_3;
                break;
            }

            g_amec_wof_pstatetable_cores_current = g_amec_wof_pstatetable_cores_next;
            g_amec->wof.state = AMEC_WOF_NO_CORE_CHANGE;

            break;
        }

        case 3:
        {
            // Check WOF frequency is applied
            // Check if GPE is applying a new frequency (request != actual)
            if (g_amec->proc[0].core_max_freq_actual != g_amec->proc[0].core_max_freq) break;
            // Check if actual clips are above WOF frequency
            if (g_amec->proc[0].core_max_freq_actual > g_amec->wof.f_vote) break;
            // Wait until p-state table is ready
            if (g_amec_wof_pstate_table_ready == 0) break;

            // If we reach this point, then we can be assured that
            // 1) GPE has applied the latest frequency selection, including WOF vote
            // 2) The true frequency is not above the WOF frequency
            // 3) The p-state table is ready

            // FIXME: Turn on the p-state table (flip address of table)
            // FIXME: new WOF vote

            // Now new table is installed, so allow WOF turbo frequency
            g_amec->wof.f_vote = g_amec->wof.f_uplift;

            g_amec_wof_pstatetable_cores_current = g_amec_wof_pstatetable_cores_next;
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
    uint64_t            l_data64;
    uint32_t            l_rc;

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
            // Check WOF frequency is applied
            //If  (current clip > WOF clip), stay at this state.
            //FIXME: How are we certain that the clip was applied to the SCOM at this point?
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

            // If we reach this point, then we can be assured that
            // 1) GPE has applied the latest frequency selection, including WOF vote
            // 2) The true frequency is not above the WOF frequency
            // Therefore, proceed to next state (wake cores up)
            g_amec->wof.state = AMEC_WOF_TRANSITION;

            // FALL THROUGH
        }

        case AMEC_WOF_TRANSITION:
        {
            // Signal waking cores to turn on. Go to state 0.

            // Quickly toggle waking core inhibit bits
            // Uninhibit cores waking up
            l_rc = _putscom(PDEMR, ~g_amec_wof_wake_mask, SCOM_TIMEOUT);
            if (l_rc != 0)
            {
                g_amec->wof.error = AMEC_WOF_ERROR_SCOM_2;
                break;
            }
            // Inhibit all cores
            // putscom pu 62092 ffff000000000000
            l_data64 = 0xffff000000000000ull;
            l_rc = _putscom(PDEMR, l_data64, SCOM_TIMEOUT);
            if (l_rc != 0)
            {
                g_amec->wof.error = AMEC_WOF_ERROR_SCOM_3;
                break;
            }

            g_amec_wof_pstatetable_cores_current = g_amec_wof_pstatetable_cores_next;
            g_amec->wof.state = AMEC_WOF_NO_CORE_CHANGE;
            break;
        }

        case AMEC_WOF_CORE_REQUEST_TURN_OFF:
        {
            // Go to state 0.
            g_amec_wof_pstatetable_cores_current = g_amec_wof_pstatetable_cores_next;
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
            // Experimental algorithm
            amec_wof_alg_v3();
            break;

        default: /*Do nothing. WOF Disabled*/
            break;
    }
}
