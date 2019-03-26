/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor_enum.h $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2018                        */
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

    AMESSdur0,         // Combined duration of the AMEC Master & Slave 0
    AMESSdur1,         // Combined duration of the AMEC Master & Slave 1
    AMESSdur2,         // Combined duration of the AMEC Master & Slave 2
    AMESSdur3,         // Combined duration of the AMEC Master & Slave 3
    AMESSdur4,         // Combined duration of the AMEC Master & Slave 4
    AMESSdur5,         // Combined duration of the AMEC Master & Slave 5
    AMESSdur6,         // Combined duration of the AMEC Master & Slave 6
    AMESSdur7,         // Combined duration of the AMEC Master & Slave 7

    PROBE250US0,       // Internal Sensor for debug via AMESTER
    PROBE250US1,       // Internal Sensor for debug via AMESTER
    PROBE250US2,       // Internal Sensor for debug via AMESTER
    PROBE250US3,       // Internal Sensor for debug via AMESTER
    PROBE250US4,       // Internal Sensor for debug via AMESTER
    PROBE250US5,       // Internal Sensor for debug via AMESTER
    PROBE250US6,       // Internal Sensor for debug via AMESTER
    PROBE250US7,       // Internal Sensor for debug via AMESTER

    GPEtickdur0,       // Duration on the GPE0 Engine
    GPEtickdur1,       // Duration on the GPE1 Engine
    RTLtickdur,        // Duration on the RTL tick interrupt

    // ------------------------------------------------------
    // System Sensors
    // ------------------------------------------------------
    PWRSYS,            // System DC Power (from APSS)
    PWRFAN,            // Fan Power (from APSS)
    PWRIO,             // IO Subsystem Power (from APSS)
    PWRSTORE,          // Storage Subsys Power (from APSS)
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
    TEMPPROCAVG,
    TEMPPROCTHRM,
    UTIL,
    TEMPNEST,
    VOLTVDDSENSE,   // chip voltage (remote sense adjusted for loadline)
    VOLTVDNSENSE,   // chip voltage (remote sense adjusted for loadline)
    PWRVDD,         // calculated from AVSBUS data
    PWRVDN,         // calculated from AVSBUS data
    PROCPWRTHROT,
    PROCOTTHROT,
    TEMPQ0,
    TEMPQ1,
    TEMPQ2,
    TEMPQ3,
    TEMPQ4,
    TEMPQ5,
    VOLTDROOPCNTQ0,
    VOLTDROOPCNTQ1,
    VOLTDROOPCNTQ2,
    VOLTDROOPCNTQ3,
    VOLTDROOPCNTQ4,
    VOLTDROOPCNTQ5,

    // ------------------------------------------------------
    // Regulator Sensors
    // ------------------------------------------------------
    VOLTVDD,
    VOLTVDN,
    CURVDD,
    CURVDN,
    TEMPVDD,

    // ------------------------------------------------------
    // Core Sensors
    // ------------------------------------------------------

    FREQREQC0,   // requested frequency
    FREQREQC1,
    FREQREQC2,
    FREQREQC3,
    FREQREQC4,
    FREQREQC5,
    FREQREQC6,
    FREQREQC7,
    FREQREQC8,
    FREQREQC9,
    FREQREQC10,
    FREQREQC11,
    FREQREQC12,
    FREQREQC13,
    FREQREQC14,
    FREQREQC15,
    FREQREQC16,
    FREQREQC17,
    FREQREQC18,
    FREQREQC19,
    FREQREQC20,
    FREQREQC21,
    FREQREQC22,
    FREQREQC23,

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

    TEMPPROCTHRMC0,
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

    TEMPC0,
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

    // ------------------------------------------------------
    // Memory Sensors
    // ------------------------------------------------------
    MRDM0,
    MRDM1,
    MRDM2,
    MRDM3,
    MRDM4,
    MRDM5,
    MRDM6,
    MRDM7,

    MWRM0,
    MWRM1,
    MWRM2,
    MWRM3,
    MWRM4,
    MWRM5,
    MWRM6,
    MWRM7,

    MEMSPM0,
    MEMSPM1,
    MEMSPM2,
    MEMSPM3,
    MEMSPM4,
    MEMSPM5,
    MEMSPM6,
    MEMSPM7,

    MEMSPSTATM0,
    MEMSPSTATM1,
    MEMSPSTATM2,
    MEMSPSTATM3,
    MEMSPSTATM4,
    MEMSPSTATM5,
    MEMSPSTATM6,
    MEMSPSTATM7,

    MIRCM0,
    MIRCM1,
    MIRCM2,
    MIRCM3,
    MIRCM4,
    MIRCM5,
    MIRCM6,
    MIRCM7,

    MLP2M0,
    MLP2M1,
    MLP2M2,
    MLP2M3,
    MLP2M4,
    MLP2M5,
    MLP2M6,
    MLP2M7,

    // Individual DIMM temperatures (NIMBUS)
    TEMPDIMM00,
    TEMPDIMM01,
    TEMPDIMM02,
    TEMPDIMM03,
    TEMPDIMM04,
    TEMPDIMM05,
    TEMPDIMM06,
    TEMPDIMM07,
    TEMPDIMM08,
    TEMPDIMM09,
    TEMPDIMM10,
    TEMPDIMM11,
    TEMPDIMM12,
    TEMPDIMM13,
    TEMPDIMM14,
    TEMPDIMM15,

    TEMPDIMMAXM0,
    TEMPDIMMAXM1,
    TEMPDIMMAXM2,
    TEMPDIMMAXM3,
    TEMPDIMMAXM4,
    TEMPDIMMAXM5,
    TEMPDIMMAXM6,
    TEMPDIMMAXM7,

    LOCDIMMAXM0,
    LOCDIMMAXM1,
    LOCDIMMAXM2,
    LOCDIMMAXM3,
    LOCDIMMAXM4,
    LOCDIMMAXM5,
    LOCDIMMAXM6,
    LOCDIMMAXM7,

    MEMPWRTHROT,
    MEMOTTHROT,

    // ------------------------------------------------------
    // Centaur Sensors - 8 MemC/Proc - 1 Cent/MemC - 2 PP/Cent
    // ------------------------------------------------------
    MACM0C0P0,
    MACM0C0P1,
    MACM1C0P0,
    MACM1C0P1,
    MACM2C0P0,
    MACM2C0P1,
    MACM3C0P0,
    MACM3C0P1,
    MACM4C0P0,
    MACM4C0P1,
    MACM5C0P0,
    MACM5C0P1,
    MACM6C0P0,
    MACM6C0P1,
    MACM7C0P0,
    MACM7C0P1,

    MPUM0C0P0,
    MPUM0C0P1,
    MPUM1C0P0,
    MPUM1C0P1,
    MPUM2C0P0,
    MPUM2C0P1,
    MPUM3C0P0,
    MPUM3C0P1,
    MPUM4C0P0,
    MPUM4C0P1,
    MPUM5C0P0,
    MPUM5C0P1,
    MPUM6C0P0,
    MPUM6C0P1,
    MPUM7C0P0,
    MPUM7C0P1,

    MIRBM0C0P0,
    MIRBM0C0P1,
    MIRBM1C0P0,
    MIRBM1C0P1,
    MIRBM2C0P0,
    MIRBM2C0P1,
    MIRBM3C0P0,
    MIRBM3C0P1,
    MIRBM4C0P0,
    MIRBM4C0P1,
    MIRBM5C0P0,
    MIRBM5C0P1,
    MIRBM6C0P0,
    MIRBM6C0P1,
    MIRBM7C0P0,
    MIRBM7C0P1,

    MIRLM0C0P0,
    MIRLM0C0P1,
    MIRLM1C0P0,
    MIRLM1C0P1,
    MIRLM2C0P0,
    MIRLM2C0P1,
    MIRLM3C0P0,
    MIRLM3C0P1,
    MIRLM4C0P0,
    MIRLM4C0P1,
    MIRLM5C0P0,
    MIRLM5C0P1,
    MIRLM6C0P0,
    MIRLM6C0P1,
    MIRLM7C0P0,
    MIRLM7C0P1,

    MIRMM0C0P0,
    MIRMM0C0P1,
    MIRMM1C0P0,
    MIRMM1C0P1,
    MIRMM2C0P0,
    MIRMM2C0P1,
    MIRMM3C0P0,
    MIRMM3C0P1,
    MIRMM4C0P0,
    MIRMM4C0P1,
    MIRMM5C0P0,
    MIRMM5C0P1,
    MIRMM6C0P0,
    MIRMM6C0P1,
    MIRMM7C0P0,
    MIRMM7C0P1,

    MIRHM0C0P0,
    MIRHM0C0P1,
    MIRHM1C0P0,
    MIRHM1C0P1,
    MIRHM2C0P0,
    MIRHM2C0P1,
    MIRHM3C0P0,
    MIRHM3C0P1,
    MIRHM4C0P0,
    MIRHM4C0P1,
    MIRHM5C0P0,
    MIRHM5C0P1,
    MIRHM6C0P0,
    MIRHM6C0P1,
    MIRHM7C0P0,
    MIRHM7C0P1,

    MTSM0C0P0,
    MTSM0C0P1,
    MTSM1C0P0,
    MTSM1C0P1,
    MTSM2C0P0,
    MTSM2C0P1,
    MTSM3C0P0,
    MTSM3C0P1,
    MTSM4C0P0,
    MTSM4C0P1,
    MTSM5C0P0,
    MTSM5C0P1,
    MTSM6C0P0,
    MTSM6C0P1,
    MTSM7C0P0,
    MTSM7C0P1,

    M4RDM0C0P0,
    M4RDM0C0P1,
    M4RDM1C0P0,
    M4RDM1C0P1,
    M4RDM2C0P0,
    M4RDM2C0P1,
    M4RDM3C0P0,
    M4RDM3C0P1,
    M4RDM4C0P0,
    M4RDM4C0P1,
    M4RDM5C0P0,
    M4RDM5C0P1,
    M4RDM6C0P0,
    M4RDM6C0P1,
    M4RDM7C0P0,
    M4RDM7C0P1,

    M4WRM0C0P0,
    M4WRM0C0P1,
    M4WRM1C0P0,
    M4WRM1C0P1,
    M4WRM2C0P0,
    M4WRM2C0P1,
    M4WRM3C0P0,
    M4WRM3C0P1,
    M4WRM4C0P0,
    M4WRM4C0P1,
    M4WRM5C0P0,
    M4WRM5C0P1,
    M4WRM6C0P0,
    M4WRM6C0P1,
    M4WRM7C0P0,
    M4WRM7C0P1,

    TEMPCENT,
    TEMPDIMMTHRM,

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
    CEFFVDNRATIO,
    VRATIO,
    OCS_ADDR,

    // ------------------------------------------------------
    // END of Sensor List (this must be last entry)
    // ------------------------------------------------------
    NUMBER_OF_SENSORS_IN_LIST,
};

#endif

