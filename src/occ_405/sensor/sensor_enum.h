/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor_enum.h $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2020                        */
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

#ifndef _sensor_enum_h
#define _sensor_enum_h

/*****************************************************************************
 * This enum is the list of all the sensors that are used in the system.  The
 * value of the enum is the GSID of the sensor, and all tables are initialized
 * based on this.
 * If you add a sensor to this enum:
 *  - All GSIDs after it will automatically change
 *  - The number of sensors & size of arrays will automatially change
 *  - The name of the enum will be turned into a string, and that is the
 *    string that will be stored as that sensor's name.
 *    - This means the enum name must match the Sensor Name string
 *      and will need to follow the rules for that string (max 15 chars)
 *  - You will need to modify the following files:
 *     - sensor_table.c  - add sensor pointer to G_amec_sensor_list and
 *                         if needed to G_amec_mini_sensor_list
 *     - sensor_info.c    - add sensor information to G_sensor_info
 *     - amec/amec_sys.h - add sensor_t to system structure so you can
 */

enum e_gsid
{
    // ------------------------------------------------------
    // Code/Firmware Sensors
    // ------------------------------------------------------
    AMEintdur    = 0,  // Combined duration of the AMEC Master & Slave Tasks

    // NOTE:  WOF VRT timeout includes time for VRT BCE + VRT IPC
    VRT_BCEdur,        // Duration of BCE for VRT copy from memory to SRAM
    VRT_IPCdur,        // Duration of IPC VRT command to PGPE
    WOFC_IPCdur,       // Duration of IPC WOF Control command to PGPE
    Sout_BCEdur,       // Duration of BCE for master to copy slave outbox
    timer2dur,         // TBD
    timer3dur,         // TBD
    timer4dur,         // TBD
    timer5dur,         // TBD

    GPEtickdur0,       // Duration on the GPE0 Engine
    GPEtickdur1,       // Duration on the GPE1 Engine
    RTLtickdur,        // Duration on the RTL tick interrupt

    // ------------------------------------------------------
    // System Sensors
    // ------------------------------------------------------
    PWRSYS,            // System DC Power (from APSS)
    PWRGPU,            // GPU Subsystem Power (from APSS) e.g. Nvidia GPU
    PWRAPSSCH0,        // These PWRAPSSCH sensors are used to report the power
    PWRAPSSCH1,        // provided by each of the 16 APSS channels.
    PWRAPSSCH2,
    PWRAPSSCH3,
    PWRAPSSCH4,
    PWRAPSSCH5,
    PWRAPSSCH6,
    PWRAPSSCH7,
    PWRAPSSCH8,
    PWRAPSSCH9,
    PWRAPSSCH10,
    PWRAPSSCH11,
    PWRAPSSCH12,
    PWRAPSSCH13,
    PWRAPSSCH14,
    PWRAPSSCH15,
    CUR12VSTBY,        // 12V Standby Current
    VRHOTMEMPRCCNT,
    TEMPAMBIENT,

    // ------------------------------------------------------
    // Chip Sensors
    // ------------------------------------------------------
    TODclock0,         // \       .
    TODclock1,         //  => 32 MHz TimeOfDay Clock split into 3 sensors
    TODclock2,         // /          (from Chip SCOM)

    // ------------------------------------------------------
    // Processor Sensors
    // ------------------------------------------------------
    FREQA,
    IPS,
    PWRPROC,
    PWRMEM,
    TEMPPROCAVG,    // Average of all TEMPPROCTHRMC core temperatures
    TEMPPROCTHRM,   // Maximum of all TEMPPROCTHRMC core temperatures
    UTIL,
    TEMPRTAVG,      // Average of all TEMPQ race track temperatures
    TEMPNEST,       // average temperature of nest DTS sensors
    VOLTVDDSENSE,   // chip voltage (remote sense adjusted for loadline)
    VOLTVDNSENSE,   // chip voltage (remote sense adjusted for loadline)
    VOLTVCSSENSE,   // chip voltage (remote sense adjusted for loadline)
    VOLTVIOSENSE,   // chip voltage (remote sense adjusted for loadline)
    PWRVDD,         // calculated from AVSBUS data
    PWRVDN,         // calculated from AVSBUS data
    PWRVCS,         // calculated from AVSBUS data
    PWRVIO,         // calculated from AVSBUS data
    PROCPWRTHROT,
    PROCOTTHROT,
    TEMPQ0,         // Temperature of the racetrack DTS for this specific quad
    TEMPQ1,
    TEMPQ2,
    TEMPQ3,
    TEMPQ4,
    TEMPQ5,
    TEMPQ6,
    TEMPQ7,

    // ------------------------------------------------------
    // Regulator Sensors
    // ------------------------------------------------------
    VOLTVDD,
    VOLTVDN,
    VOLTVCS,
    VOLTVIO,
    CURVDD,
    CURVDN,
    CURVCS,
    CURVIO,
    TEMPVDD,        // VRM Vdd temperature

    // ------------------------------------------------------
    // Core Sensors
    // ------------------------------------------------------
    FREQAC0,     // actual frequency
    FREQAC1,
    FREQAC2,
    FREQAC3,
    FREQAC4,
    FREQAC5,
    FREQAC6,
    FREQAC7,
    FREQAC8,
    FREQAC9,
    FREQAC10,
    FREQAC11,
    FREQAC12,
    FREQAC13,
    FREQAC14,
    FREQAC15,
    FREQAC16,
    FREQAC17,
    FREQAC18,
    FREQAC19,
    FREQAC20,
    FREQAC21,
    FREQAC22,
    FREQAC23,
    FREQAC24,
    FREQAC25,
    FREQAC26,
    FREQAC27,
    FREQAC28,
    FREQAC29,
    FREQAC30,
    FREQAC31,

    IPSC0,
    IPSC1,
    IPSC2,
    IPSC3,
    IPSC4,
    IPSC5,
    IPSC6,
    IPSC7,
    IPSC8,
    IPSC9,
    IPSC10,
    IPSC11,
    IPSC12,
    IPSC13,
    IPSC14,
    IPSC15,
    IPSC16,
    IPSC17,
    IPSC18,
    IPSC19,
    IPSC20,
    IPSC21,
    IPSC22,
    IPSC23,
    IPSC24,
    IPSC25,
    IPSC26,
    IPSC27,
    IPSC28,
    IPSC29,
    IPSC30,
    IPSC31,

    NOTBZEC0,
    NOTBZEC1,
    NOTBZEC2,
    NOTBZEC3,
    NOTBZEC4,
    NOTBZEC5,
    NOTBZEC6,
    NOTBZEC7,
    NOTBZEC8,
    NOTBZEC9,
    NOTBZEC10,
    NOTBZEC11,
    NOTBZEC12,
    NOTBZEC13,
    NOTBZEC14,
    NOTBZEC15,
    NOTBZEC16,
    NOTBZEC17,
    NOTBZEC18,
    NOTBZEC19,
    NOTBZEC20,
    NOTBZEC21,
    NOTBZEC22,
    NOTBZEC23,
    NOTBZEC24,
    NOTBZEC25,
    NOTBZEC26,
    NOTBZEC27,
    NOTBZEC28,
    NOTBZEC29,
    NOTBZEC30,
    NOTBZEC31,

    NOTFINC0,
    NOTFINC1,
    NOTFINC2,
    NOTFINC3,
    NOTFINC4,
    NOTFINC5,
    NOTFINC6,
    NOTFINC7,
    NOTFINC8,
    NOTFINC9,
    NOTFINC10,
    NOTFINC11,
    NOTFINC12,
    NOTFINC13,
    NOTFINC14,
    NOTFINC15,
    NOTFINC16,
    NOTFINC17,
    NOTFINC18,
    NOTFINC19,
    NOTFINC20,
    NOTFINC21,
    NOTFINC22,
    NOTFINC23,
    NOTFINC24,
    NOTFINC25,
    NOTFINC26,
    NOTFINC27,
    NOTFINC28,
    NOTFINC29,
    NOTFINC30,
    NOTFINC31,

    TEMPPROCTHRMC0,     // Combination of the relevant weighted core/L3/racetrack DTSs
    TEMPPROCTHRMC1,
    TEMPPROCTHRMC2,
    TEMPPROCTHRMC3,
    TEMPPROCTHRMC4,
    TEMPPROCTHRMC5,
    TEMPPROCTHRMC6,
    TEMPPROCTHRMC7,
    TEMPPROCTHRMC8,
    TEMPPROCTHRMC9,
    TEMPPROCTHRMC10,
    TEMPPROCTHRMC11,
    TEMPPROCTHRMC12,
    TEMPPROCTHRMC13,
    TEMPPROCTHRMC14,
    TEMPPROCTHRMC15,
    TEMPPROCTHRMC16,
    TEMPPROCTHRMC17,
    TEMPPROCTHRMC18,
    TEMPPROCTHRMC19,
    TEMPPROCTHRMC20,
    TEMPPROCTHRMC21,
    TEMPPROCTHRMC22,
    TEMPPROCTHRMC23,
    TEMPPROCTHRMC24,
    TEMPPROCTHRMC25,
    TEMPPROCTHRMC26,
    TEMPPROCTHRMC27,
    TEMPPROCTHRMC28,
    TEMPPROCTHRMC29,
    TEMPPROCTHRMC30,
    TEMPPROCTHRMC31,

    UTILC0,
    UTILC1,
    UTILC2,
    UTILC3,
    UTILC4,
    UTILC5,
    UTILC6,
    UTILC7,
    UTILC8,
    UTILC9,
    UTILC10,
    UTILC11,
    UTILC12,
    UTILC13,
    UTILC14,
    UTILC15,
    UTILC16,
    UTILC17,
    UTILC18,
    UTILC19,
    UTILC20,
    UTILC21,
    UTILC22,
    UTILC23,
    UTILC24,
    UTILC25,
    UTILC26,
    UTILC27,
    UTILC28,
    UTILC29,
    UTILC30,
    UTILC31,

    NUTILC0,
    NUTILC1,
    NUTILC2,
    NUTILC3,
    NUTILC4,
    NUTILC5,
    NUTILC6,
    NUTILC7,
    NUTILC8,
    NUTILC9,
    NUTILC10,
    NUTILC11,
    NUTILC12,
    NUTILC13,
    NUTILC14,
    NUTILC15,
    NUTILC16,
    NUTILC17,
    NUTILC18,
    NUTILC19,
    NUTILC20,
    NUTILC21,
    NUTILC22,
    NUTILC23,
    NUTILC24,
    NUTILC25,
    NUTILC26,
    NUTILC27,
    NUTILC28,
    NUTILC29,
    NUTILC30,
    NUTILC31,

    TEMPC0,         // Average temperature of core DTS sensors for processor core#
    TEMPC1,
    TEMPC2,
    TEMPC3,
    TEMPC4,
    TEMPC5,
    TEMPC6,
    TEMPC7,
    TEMPC8,
    TEMPC9,
    TEMPC10,
    TEMPC11,
    TEMPC12,
    TEMPC13,
    TEMPC14,
    TEMPC15,
    TEMPC16,
    TEMPC17,
    TEMPC18,
    TEMPC19,
    TEMPC20,
    TEMPC21,
    TEMPC22,
    TEMPC23,
    TEMPC24,
    TEMPC25,
    TEMPC26,
    TEMPC27,
    TEMPC28,
    TEMPC29,
    TEMPC30,
    TEMPC31,

    STOPDEEPREQC0,
    STOPDEEPREQC1,
    STOPDEEPREQC2,
    STOPDEEPREQC3,
    STOPDEEPREQC4,
    STOPDEEPREQC5,
    STOPDEEPREQC6,
    STOPDEEPREQC7,
    STOPDEEPREQC8,
    STOPDEEPREQC9,
    STOPDEEPREQC10,
    STOPDEEPREQC11,
    STOPDEEPREQC12,
    STOPDEEPREQC13,
    STOPDEEPREQC14,
    STOPDEEPREQC15,
    STOPDEEPREQC16,
    STOPDEEPREQC17,
    STOPDEEPREQC18,
    STOPDEEPREQC19,
    STOPDEEPREQC20,
    STOPDEEPREQC21,
    STOPDEEPREQC22,
    STOPDEEPREQC23,
    STOPDEEPREQC24,
    STOPDEEPREQC25,
    STOPDEEPREQC26,
    STOPDEEPREQC27,
    STOPDEEPREQC28,
    STOPDEEPREQC29,
    STOPDEEPREQC30,
    STOPDEEPREQC31,

    STOPDEEPACTC0,
    STOPDEEPACTC1,
    STOPDEEPACTC2,
    STOPDEEPACTC3,
    STOPDEEPACTC4,
    STOPDEEPACTC5,
    STOPDEEPACTC6,
    STOPDEEPACTC7,
    STOPDEEPACTC8,
    STOPDEEPACTC9,
    STOPDEEPACTC10,
    STOPDEEPACTC11,
    STOPDEEPACTC12,
    STOPDEEPACTC13,
    STOPDEEPACTC14,
    STOPDEEPACTC15,
    STOPDEEPACTC16,
    STOPDEEPACTC17,
    STOPDEEPACTC18,
    STOPDEEPACTC19,
    STOPDEEPACTC20,
    STOPDEEPACTC21,
    STOPDEEPACTC22,
    STOPDEEPACTC23,
    STOPDEEPACTC24,
    STOPDEEPACTC25,
    STOPDEEPACTC26,
    STOPDEEPACTC27,
    STOPDEEPACTC28,
    STOPDEEPACTC29,
    STOPDEEPACTC30,
    STOPDEEPACTC31,

    VOLTDROOPCNTC0,
    VOLTDROOPCNTC1,
    VOLTDROOPCNTC2,
    VOLTDROOPCNTC3,
    VOLTDROOPCNTC4,
    VOLTDROOPCNTC5,
    VOLTDROOPCNTC6,
    VOLTDROOPCNTC7,
    VOLTDROOPCNTC8,
    VOLTDROOPCNTC9,
    VOLTDROOPCNTC10,
    VOLTDROOPCNTC11,
    VOLTDROOPCNTC12,
    VOLTDROOPCNTC13,
    VOLTDROOPCNTC14,
    VOLTDROOPCNTC15,
    VOLTDROOPCNTC16,
    VOLTDROOPCNTC17,
    VOLTDROOPCNTC18,
    VOLTDROOPCNTC19,
    VOLTDROOPCNTC20,
    VOLTDROOPCNTC21,
    VOLTDROOPCNTC22,
    VOLTDROOPCNTC23,
    VOLTDROOPCNTC24,
    VOLTDROOPCNTC25,
    VOLTDROOPCNTC26,
    VOLTDROOPCNTC27,
    VOLTDROOPCNTC28,
    VOLTDROOPCNTC29,
    VOLTDROOPCNTC30,
    VOLTDROOPCNTC31,

    // ------------------------------------------------------
    // Memory Sensors
    // ------------------------------------------------------
    MRDM0,          // Memory read requests per sec for MC
    MRDM1,
    MRDM2,
    MRDM3,
    MRDM4,
    MRDM5,
    MRDM6,
    MRDM7,
    MRDM8,
    MRDM9,
    MRDM10,
    MRDM11,
    MRDM12,
    MRDM13,
    MRDM14,
    MRDM15,

    MWRM0,          // Memory write requests per sec for MC
    MWRM1,
    MWRM2,
    MWRM3,
    MWRM4,
    MWRM5,
    MWRM6,
    MWRM7,
    MWRM8,
    MWRM9,
    MWRM10,
    MWRM11,
    MWRM12,
    MWRM13,
    MWRM14,
    MWRM15,

    MEMSPM0,        // Current memory throttle level setting for MCA
    MEMSPM1,
    MEMSPM2,
    MEMSPM3,
    MEMSPM4,
    MEMSPM5,
    MEMSPM6,
    MEMSPM7,
    MEMSPM8,
    MEMSPM9,
    MEMSPM10,
    MEMSPM11,
    MEMSPM12,
    MEMSPM13,
    MEMSPM14,
    MEMSPM15,

    MEMSPSTATM0,    // Static memory throttle level setting for MCA
    MEMSPSTATM1,
    MEMSPSTATM2,
    MEMSPSTATM3,
    MEMSPSTATM4,
    MEMSPSTATM5,
    MEMSPSTATM6,
    MEMSPSTATM7,
    MEMSPSTATM8,
    MEMSPSTATM9,
    MEMSPSTATM10,
    MEMSPSTATM11,
    MEMSPSTATM12,
    MEMSPSTATM13,
    MEMSPSTATM14,
    MEMSPSTATM15,

    TEMPMEMBUF0,    // Individual Memory Buffer temperatures
    TEMPMEMBUF1,
    TEMPMEMBUF2,
    TEMPMEMBUF3,
    TEMPMEMBUF4,
    TEMPMEMBUF5,
    TEMPMEMBUF6,
    TEMPMEMBUF7,
    TEMPMEMBUF8,
    TEMPMEMBUF9,
    TEMPMEMBUF10,
    TEMPMEMBUF11,
    TEMPMEMBUF12,
    TEMPMEMBUF13,
    TEMPMEMBUF14,
    TEMPMEMBUF15,

    MEMPWRTHROT,    // Count of memory throttled due to power
    MEMOTTHROT,     // Count of memory throttled due to over temperature

    // ------------------------------------------------------
    // Memory Buffers
    // ------------------------------------------------------
    TEMPMEMBUFTHRM,
    TEMPDIMMTHRM,
    TEMPMCDIMMTHRM,
    TEMPPMICTHRM,
    TEMPMCEXTTHRM,

    // ------------------------------------------------------
    // GPU Sensors
    // ------------------------------------------------------
    TEMPGPU0,
    TEMPGPU1,
    TEMPGPU2,
    TEMPGPU0MEM,
    TEMPGPU1MEM,
    TEMPGPU2MEM,

    // ------------------------------------------------------
    // Partition Sensors
    // ------------------------------------------------------
    UTILSLCG000,

    // ------------------------------------------------------
    // WOF Sensors
    // ------------------------------------------------------
    CEFFVDDRATIO,
    CEFFVCSRATIO,
    VRATIO_VDD,
    OCS_ADDR,
    CEFFVDDRATIOADJ,
    IO_PWR_PROXY,
    UV_AVG,
    OV_AVG,

    // ------------------------------------------------------
    // END of Sensor List (this must be last entry)
    // ------------------------------------------------------
    NUMBER_OF_SENSORS_IN_LIST,
};

#endif

