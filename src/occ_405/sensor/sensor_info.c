/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor_info.c $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2023                        */
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

#include <occ_common.h>     // STATIC_ASSERT macro
#include <sensor.h>         // For Sensor defines

#define AMEEFP_250US_IN_HZ  AMEFP(4,3)       //  4000 Hz
#define AMEEFP_500US_IN_HZ  AMEFP(2,3)       //  2000 Hz
#define AMEEFP_1MS_IN_HZ    AMEFP(1,3)       //  1000 Hz
#define AMEEFP_1500US_IN_HZ AMEFP(75,1)      //   750 Hz
#define AMEEFP_2MS_IN_HZ    AMEFP(5,2)       //   500 Hz
#define AMEEFP_3MS_IN_HZ    AMEFP(375,0)     //   375 Hz
#define AMEEFP_4MS_IN_HZ    AMEFP(25,1)      //   250 Hz
#define AMEEFP_8MS_IN_HZ    AMEFP(125,0)     //   125 Hz
#define AMEEFP_16MS_IN_HZ   AMEFP(625,-1)    //  62.5 Hz
#define AMEEFP_32MS_IN_HZ   AMEFP(3125,-2)   // 31.25 Hz
#define AMEEFP_64MS_IN_HZ   AMEFP(15625,-3)  // 15.625 Hz
#define AMEEFP_1S_IN_HZ     AMEFP(1,0)       //   1.0 Hz
#define AMEEFP_3S_IN_HZ     AMEFP(333,-3)    // 0.333 Hz
#define AMEFP_SCALE_0_16384 AMEFP(610352,-8) // scalar so that digital 16384=100%

// constants to allow fewer changes if tick time changes
#define AMEEFP_EVERY_TICK_HZ        AMEEFP_500US_IN_HZ  // tick time 500us
#define AMEEFP_EVERY_2ND_TICK_HZ    AMEEFP_1MS_IN_HZ    // 1ms
#define AMEEFP_EVERY_4TH_TICK_HZ    AMEEFP_2MS_IN_HZ    // 2ms
#define AMEEFP_EVERY_8TH_TICK_HZ    AMEEFP_4MS_IN_HZ    // 4ms
#define AMEEFP_EVERY_16TH_TICK_HZ   AMEEFP_8MS_IN_HZ    // 8ms
#define AMEEFP_EVERY_32ND_TICK_HZ   AMEEFP_16MS_IN_HZ   // 16ms
#define AMEEFP_EVERY_64TH_TICK_HZ   AMEEFP_32MS_IN_HZ   // 32ms
#define AMEEFP_EVERY_128TH_TICK_HZ  AMEEFP_64MS_IN_HZ   // 64ms

// This will define the fields of the "sensor" member of the sensor_info_t
#define SENSOR_VALUES(units, type, location, number, frequency, scaleFactor) \
  .sensor = { units, type, location, number, frequency, scaleFactor },}

// This will put a single sensor entry into the sensor list table
#define SENSOR_INFO_T_ENTRY(sensor_name, units, type, location, number, frequency, scaleFactor)  \
  [sensor_name] = {.name = #sensor_name,   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}

// These will paste a number onto a sensor name base to create the full enum
// representation of the sensor name
#define SENSOR_W_NUM(sensor_name, num) sensor_name##num

// These will stringify the enum so to create the sensor name.  This will help
// save keystrokes, as well as reduce typos & copy paste errors.
#define SENSOR_STRING(sensor_name) #sensor_name

// This will create a set of 8 sensor entries into the sensor list table.
// (one for each quad...)  The base name of the sensor enum must be passed
// and this macro will take care of the paste & stringify operations.
#define SENS_QUAD_ENTRY_SET(sensor_name, units, type, location, number, frequency, scaleFactor)  \
  [SENSOR_W_NUM(sensor_name,0)] = {.name = SENSOR_STRING(sensor_name ## 0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,1)] = {.name = SENSOR_STRING(sensor_name ## 1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,2)] = {.name = SENSOR_STRING(sensor_name ## 2),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,3)] = {.name = SENSOR_STRING(sensor_name ## 3),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,4)] = {.name = SENSOR_STRING(sensor_name ## 4),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,5)] = {.name = SENSOR_STRING(sensor_name ## 5),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,6)] = {.name = SENSOR_STRING(sensor_name ## 6),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,7)] = {.name = SENSOR_STRING(sensor_name ## 7),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}


// This will create a set of 32 sensor entries into the sensor list table.
// (one for each core...)  The base name of the sensor enum must be passed
// and this macro will take care of the paste & stringify operations.
#define SENS_CORE_ENTRY_SET(sensor_name, units, type, location, number, frequency, scaleFactor)  \
  [SENSOR_W_NUM(sensor_name,0)] = {.name = SENSOR_STRING(sensor_name ## 00),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,1)] = {.name = SENSOR_STRING(sensor_name ## 01),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,2)] = {.name = SENSOR_STRING(sensor_name ## 02),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,3)] = {.name = SENSOR_STRING(sensor_name ## 03),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,4)] = {.name = SENSOR_STRING(sensor_name ## 04),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,5)] = {.name = SENSOR_STRING(sensor_name ## 05),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,6)] = {.name = SENSOR_STRING(sensor_name ## 06),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,7)] = {.name = SENSOR_STRING(sensor_name ## 07),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,8)] = {.name = SENSOR_STRING(sensor_name ## 08),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,9)] = {.name = SENSOR_STRING(sensor_name ## 09),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,10)] = {.name = SENSOR_STRING(sensor_name ## 10),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,11)] = {.name = SENSOR_STRING(sensor_name ## 11),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,12)] = {.name = SENSOR_STRING(sensor_name ## 12),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,13)] = {.name = SENSOR_STRING(sensor_name ## 13),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,14)] = {.name = SENSOR_STRING(sensor_name ## 14),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,15)] = {.name = SENSOR_STRING(sensor_name ## 15),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,16)] = {.name = SENSOR_STRING(sensor_name ## 16),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,17)] = {.name = SENSOR_STRING(sensor_name ## 17),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,18)] = {.name = SENSOR_STRING(sensor_name ## 18),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,19)] = {.name = SENSOR_STRING(sensor_name ## 19),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,20)] = {.name = SENSOR_STRING(sensor_name ## 20),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,21)] = {.name = SENSOR_STRING(sensor_name ## 21),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,22)] = {.name = SENSOR_STRING(sensor_name ## 22),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,23)] = {.name = SENSOR_STRING(sensor_name ## 23),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,24)] = {.name = SENSOR_STRING(sensor_name ## 24),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,25)] = {.name = SENSOR_STRING(sensor_name ## 25),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,26)] = {.name = SENSOR_STRING(sensor_name ## 26),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,27)] = {.name = SENSOR_STRING(sensor_name ## 27),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,28)] = {.name = SENSOR_STRING(sensor_name ## 28),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,29)] = {.name = SENSOR_STRING(sensor_name ## 29),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,30)] = {.name = SENSOR_STRING(sensor_name ## 30),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,31)] = {.name = SENSOR_STRING(sensor_name ## 31),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}

// This will create a set of 16 sensor entries into the sensor list table.
// (one for each mem controller...)  The base name of the sensor enum must be passed
// and this macro will take care of the paste & stringify operations.
#define SENS_MEMC_ENTRY_SET(sensor_name, units, type, location, number, frequency, scaleFactor)  \
  [SENSOR_W_NUM(sensor_name,0)] = {.name = SENSOR_STRING(sensor_name ## 00),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,1)] = {.name = SENSOR_STRING(sensor_name ## 01),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,2)] = {.name = SENSOR_STRING(sensor_name ## 02),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,3)] = {.name = SENSOR_STRING(sensor_name ## 03),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,4)] = {.name = SENSOR_STRING(sensor_name ## 04),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,5)] = {.name = SENSOR_STRING(sensor_name ## 05),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,6)] = {.name = SENSOR_STRING(sensor_name ## 06),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,7)] = {.name = SENSOR_STRING(sensor_name ## 07),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,8)] = {.name = SENSOR_STRING(sensor_name ## 08),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,9)] = {.name = SENSOR_STRING(sensor_name ## 09),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,10)] = {.name = SENSOR_STRING(sensor_name ## 10),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,11)] = {.name = SENSOR_STRING(sensor_name ## 11),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,12)] = {.name = SENSOR_STRING(sensor_name ## 12),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,13)] = {.name = SENSOR_STRING(sensor_name ## 13),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,14)] = {.name = SENSOR_STRING(sensor_name ## 14),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,15)] = {.name = SENSOR_STRING(sensor_name ## 15),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}


// This table takes care of the ordering of the sensors (by GSID) and all parameters needed for AMEC or AMESTER.  The order
// that is in the table below doesn't matter because we use designated initializers.
// If anything more than the barebones sensor_t is need, an applet will need to be called in order to gather that data.
// For refernce:
//    AMEFP(1, 3);       // 1000:1 scale factor
//    AMEFP(1, 0);       // 1:1 scale factor
//    AMEFP(1,-1);       // 1:0.1 scale factor
//    AMEFP(1,-3);       // 1:0.001 scale factor
//
const sensor_info_t G_sensor_info[]   =
{
  /* ==FirmwareSensors== NameString    Units                Type              Location             Number                Freq               ScaleFactor */
  SENSOR_INFO_T_ENTRY(     AMEintdur,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,     AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   VRT_BCEdur,   "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   VRT_IPCdur,   "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   WOFC_IPCdur,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   Sout_BCEdur,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   CORE_IPCdur,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     timer3dur,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     timer4dur,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     timer5dur,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   GPEtickdur0,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,     AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   GPEtickdur1,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,     AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    RTLtickdur,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,     AMEFP(  1, 0)  ),

  /* ==SystemSensors==     NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SENSOR_INFO_T_ENTRY(         PWRSYS, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(         PWRGPU, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_GPU, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWRAPSSCH00, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWRAPSSCH01, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWRAPSSCH02, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWRAPSSCH03, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWRAPSSCH04, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWRAPSSCH05, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWRAPSSCH06, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWRAPSSCH07, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWRAPSSCH08, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWRAPSSCH09, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWRAPSSCH10, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWRAPSSCH11, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWRAPSSCH12, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWRAPSSCH13, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWRAPSSCH14, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWRAPSSCH15, "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     CUR12VSTBY, "A\0", AMEC_SENSOR_TYPE_CURRENT,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1,-2)  ),
  SENSOR_INFO_T_ENTRY( VRHOTMEMPRCCNT, "#\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    TEMPAMBIENT, "C\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM, AMEEFP_1S_IN_HZ,        AMEFP(  1, 0)  ),

  /* ==ChipSensors==       NameString  Units                     Type              Location             Number                Freq         ScaleFactor   */
  SENSOR_INFO_T_ENTRY(      TODclock0, "us\0",  AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_ALL, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP( 16, 0)  ),
  SENSOR_INFO_T_ENTRY(      TODclock1, "sec\0", AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_ALL, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP( 1048576,-6)  ),
  SENSOR_INFO_T_ENTRY(      TODclock2, "day\0", AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_ALL, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP( 795364,-6)  ),

  /* ==ProcSensors==       NameString  Units                      Type              Location             Number                Freq           ScaleFactor   */
  SENSOR_INFO_T_ENTRY(          FREQA, "MHz\0",    AMEC_SENSOR_TYPE_FREQ, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(            IPS, "MIP\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(        PWRPROC,   "W\0",   AMEC_SENSOR_TYPE_POWER, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,      AMEEFP_EVERY_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(         PWRMEM,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,      AMEEFP_EVERY_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    TEMPPROCAVG,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   TEMPPROCTHRM,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   TEMPPROCIOTHRM,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(           UTIL,   "%\0",    AMEC_SENSOR_TYPE_UTIL, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1,-2)  ),
  SENSOR_INFO_T_ENTRY(    TEMPRTAVG,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       TEMPNEST0,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       TEMPNEST1,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   VOLTVDDSENSE,  "mV\0", AMEC_SENSOR_TYPE_VOLTAGE, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,  AMEEFP_EVERY_4TH_TICK_HZ, AMEFP(  1, -1)  ),
  SENSOR_INFO_T_ENTRY(   VOLTVDNSENSE,  "mV\0", AMEC_SENSOR_TYPE_VOLTAGE, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,  AMEEFP_EVERY_4TH_TICK_HZ, AMEFP(  1, -1)  ),
  SENSOR_INFO_T_ENTRY(   VOLTVCSSENSE,  "mV\0", AMEC_SENSOR_TYPE_VOLTAGE, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,  AMEEFP_EVERY_4TH_TICK_HZ, AMEFP(  1, -1)  ),
  SENSOR_INFO_T_ENTRY(  VSENSERESERVE,  "mV\0", AMEC_SENSOR_TYPE_INVALID, AMEC_SENSOR_LOC_INVALID, AMEC_SENSOR_NONUM,  AMEEFP_EVERY_4TH_TICK_HZ, AMEFP(  1, -1)  ),
  SENSOR_INFO_T_ENTRY(         PWRVDD,   "W\0",   AMEC_SENSOR_TYPE_POWER, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,  AMEEFP_EVERY_2ND_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(         PWRVDN,   "W\0",   AMEC_SENSOR_TYPE_POWER, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,  AMEEFP_EVERY_2ND_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(         PWRVCS,   "W\0",   AMEC_SENSOR_TYPE_POWER, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,  AMEEFP_EVERY_2ND_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROCOTTIME,   "ms\0",   AMEC_SENSOR_TYPE_TIME, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_64TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   PROCPWRTHROT,   "#\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,      AMEEFP_EVERY_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PROCOTTHROT,   "#\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_64TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(         DDSAVG,   " \0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(         DDSMIN,   " \0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       TEMPPROCIO00,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       TEMPPROCIO01,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       TEMPPROCIO10,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       TEMPPROCIO11,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),

  SENS_QUAD_ENTRY_SET(          TEMPQ,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_QUAD, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),

  /* ==ReguSensors==       NameString  Units                      Type              Location             Number                Freq          ScaleFactor   */
  SENSOR_INFO_T_ENTRY(        VOLTVDD, "mV\0", AMEC_SENSOR_TYPE_VOLTAGE,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_EVERY_4TH_TICK_HZ, AMEFP(  1, -1)  ),
  SENSOR_INFO_T_ENTRY(        VOLTVDN, "mV\0", AMEC_SENSOR_TYPE_VOLTAGE,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_EVERY_4TH_TICK_HZ, AMEFP(  1, -1)  ),
  SENSOR_INFO_T_ENTRY(        VOLTVCS, "mV\0", AMEC_SENSOR_TYPE_VOLTAGE,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_EVERY_4TH_TICK_HZ, AMEFP(  1, -1)  ),
  SENSOR_INFO_T_ENTRY(    VOLTRESERVE, "mV\0", AMEC_SENSOR_TYPE_INVALID,  AMEC_SENSOR_LOC_INVALID, AMEC_SENSOR_NONUM, AMEEFP_EVERY_4TH_TICK_HZ, AMEFP(  1, -1)  ),
  SENSOR_INFO_T_ENTRY(         CURVDD,  "A\0", AMEC_SENSOR_TYPE_CURRENT,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_EVERY_4TH_TICK_HZ, AMEFP(  1,-2)  ),
  SENSOR_INFO_T_ENTRY(         CURVDN,  "A\0", AMEC_SENSOR_TYPE_CURRENT,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_EVERY_4TH_TICK_HZ, AMEFP(  1,-2)  ),
  SENSOR_INFO_T_ENTRY(         CURVCS,  "A\0", AMEC_SENSOR_TYPE_CURRENT,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_EVERY_4TH_TICK_HZ, AMEFP(  1,-2)  ),
  SENSOR_INFO_T_ENTRY(         PSTATE,  " \0",    AMEC_SENSOR_TYPE_FREQ, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ,     AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(        TEMPVDD,  "C\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_EVERY_4TH_TICK_HZ, AMEFP(  1, 0)  ),

  /* ==CoreSensors==       NameString  Units                      Type              Location             Number                Freq          ScaleFactor   */
  SENS_CORE_ENTRY_SET(         FREQAC, "MHz\0",   AMEC_SENSOR_TYPE_FREQ, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(           IPSC, "MIP\0",   AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(        NOTBZEC, "cyc\0",   AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(        NOTFINC, "cyc\0",   AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(  TEMPPROCTHRMC,   "C\0",   AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(          UTILC,   "%\0",   AMEC_SENSOR_TYPE_UTIL, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1,-2)  ),
  SENS_CORE_ENTRY_SET(         NUTILC,   "%\0",   AMEC_SENSOR_TYPE_UTIL, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,           AMEEFP_3S_IN_HZ, AMEFP(  1,-2)  ),
  SENS_CORE_ENTRY_SET(          TEMPC,   "C\0",   AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(   STOPDEEPREQC,  "ss\0",   AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(   STOPDEEPACTC,  "ss\0",   AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(  VOLTDROOPCNTC,   "#\0",AMEC_SENSOR_TYPE_VOLTAGE, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM, AMEEFP_EVERY_16TH_TICK_HZ, AMEFP(  1, 0)  ),

  /* ==MemSensors==        NameString  Units                      Type              Location             Number                Freq           ScaleFactor   */
  SENS_MEMC_ENTRY_SET(           MRDM, "GBs\0",   AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_EVERY_8TH_TICK_HZ, AMEFP(64, -5) ),
  SENS_MEMC_ENTRY_SET(           MWRM, "GBs\0",   AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_EVERY_8TH_TICK_HZ, AMEFP(64, -5) ),
  SENS_MEMC_ENTRY_SET(         MEMSPM,   "%\0",   AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_EVERY_8TH_TICK_HZ, AMEFP(  1, -1) ),
  SENS_MEMC_ENTRY_SET(     MEMSPSTATM,   "%\0",   AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_EVERY_8TH_TICK_HZ, AMEFP(  1, -1) ),
  SENS_MEMC_ENTRY_SET(     TEMPMEMBUF,   "C\0",   AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_EVERY_128TH_TICK_HZ, AMEFP(  1, 0) ),
  SENSOR_INFO_T_ENTRY(     MEMOTTHROT,   "#\0",   AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,  AMEEFP_EVERY_64TH_TICK_HZ, AMEFP(  1, 0) ),

  /* ==MemSummarySensors== NameString  Units                     Type              Location             Number                Freq            ScaleFactor   */
  SENSOR_INFO_T_ENTRY( TEMPMEMBUFTHRM,   "C\0",   AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_EVERY_8TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   TEMPDIMMTHRM,   "C\0",   AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_EVERY_128TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY( TEMPMCDIMMTHRM,   "C\0",   AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_EVERY_128TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   TEMPPMICTHRM,   "C\0",   AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_EVERY_128TH_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(  TEMPMCEXTTHRM,   "C\0",   AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_EVERY_128TH_TICK_HZ, AMEFP(  1, 0)  ),

  /* ==GPUSensors==      NameString  Units                      Type              Location             Number        Freq         ScaleFactor   */
  SENSOR_INFO_T_ENTRY(   TEMPGPU0,    "C\0",   AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_GPU, AMEC_SENSOR_NONUM,  AMEEFP_1S_IN_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   TEMPGPU1,    "C\0",   AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_GPU, AMEC_SENSOR_NONUM,  AMEEFP_1S_IN_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   TEMPGPU2,    "C\0",   AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_GPU, AMEC_SENSOR_NONUM,  AMEEFP_1S_IN_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   TEMPGPU0MEM, "C\0",   AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_GPU, AMEC_SENSOR_NONUM,  AMEEFP_1S_IN_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   TEMPGPU1MEM, "C\0",   AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_GPU, AMEC_SENSOR_NONUM,  AMEEFP_1S_IN_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   TEMPGPU2MEM, "C\0",   AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_GPU, AMEC_SENSOR_NONUM,  AMEEFP_1S_IN_HZ, AMEFP(  1, 0)  ),

  /* ==PartSummarySensors== NameString Units                   Type              Location             Number                Freq          ScaleFactor   */
  SENSOR_INFO_T_ENTRY(   UTILSLCG000,  "%\0", AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_EVERY_8TH_TICK_HZ, AMEFP_SCALE_0_16384),

  /* ==WOF Sensors==     NameString        Units            Type              Location             Number                Freq          ScaleFactor   */
  SENSOR_INFO_T_ENTRY(   CEFFVDDRATIO,  "%\0",   AMEC_SENSOR_TYPE_WOF,  AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1,-2)  ),
  SENSOR_INFO_T_ENTRY(   CEFFVCSRATIO,  "%\0",   AMEC_SENSOR_TYPE_WOF,  AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1,-2)  ),
  SENSOR_INFO_T_ENTRY(   VRATIO_VDD,    " \0",   AMEC_SENSOR_TYPE_WOF,  AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   OCS_ADDR,      "%\0",   AMEC_SENSOR_TYPE_WOF,  AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1,-2)  ),
  SENSOR_INFO_T_ENTRY(   CEFFVDDRATIOADJ, "%\0", AMEC_SENSOR_TYPE_WOF,  AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1,-2)  ),
  SENSOR_INFO_T_ENTRY(   IO_PWR_PROXY,  "W\0",   AMEC_SENSOR_TYPE_WOF,  AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1,-2) ),
  SENSOR_INFO_T_ENTRY(   UV_AVG,        "%\0",   AMEC_SENSOR_TYPE_WOF,  AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1,-1)  ),
  SENSOR_INFO_T_ENTRY(   OV_AVG,        "%\0",   AMEC_SENSOR_TYPE_WOF,  AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1,-1)  ),
  SENSOR_INFO_T_ENTRY(   FREQ_PSTATE,   " \0",  AMEC_SENSOR_TYPE_FREQ,  AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   THROTTLE_ADDR, "%\0",   AMEC_SENSOR_TYPE_WOF,  AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_EVERY_TICK_HZ, AMEFP(  1,-2)  ),
};

// Cause a compile error if we don't have all the sensors in the enum in the initialization list.
STATIC_ASSERT(   (NUMBER_OF_SENSORS_IN_LIST != (sizeof(G_sensor_info)/sizeof(sensor_info_t)))  );
STATIC_ASSERT(   (MAX_AMEC_SENSORS < (sizeof(G_sensor_info)/sizeof(sensor_info_t)))   );
