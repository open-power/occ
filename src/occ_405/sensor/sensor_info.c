/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor_info.c $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
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

#include <occ_common.h>         // STATIC_ASSERT macro
#include <sensor.h>         // For Sensor defines

#define AMEEFP_2MS_IN_HZ    AMEFP(5,2)      //  500 Hz
#define AMEEFP_4MS_IN_HZ    AMEFP(25,1)     //  250 Hz
#define AMEEFP_250US_IN_HZ  AMEFP(4,3)      // 4000 Hz
#define AMEEFP_3S_IN_HZ     AMEFP(333,-3)   // 0.333 Hz
#define AMEFP_SCALE_0_16384 AMEFP(610352,-8) // scalar so that digital 16384=100%

// This will get the string when given the GSID
#define SENSOR_GSID_TO_STRING(gsid)  G_sensor_list[gsid].name;

// This will define the fields of the "sensor" member of the sensor_info_t
#define SENSOR_VALUES(units, type, location, number, frequency, scaleFactor) \
  .sensor = { units, type, location, number, frequency, scaleFactor },}

// This will put a single sensor entry into the sensor list table
#define SENSOR_INFO_T_ENTRY(sensor_name, units, type, location, number, frequency, scaleFactor)  \
  [sensor_name] = {.name = #sensor_name,   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}

// This will paste a number onto a sensor name base to create the full enum
// representation of the sensor name
#define SENSOR_W_NUM(sensor_name, num) sensor_name##num

// This will paste a number onto a sensor name base to create the full enum
// representation of the sensor name
#define SENSOR_W_CENTAUR_NUM_HELPER(sensor_name, memc,centL,cent,ppL,pp) sensor_name##memc##centL##cent##ppL##pp
#define SENSOR_W_CENTAUR_NUM(sensor_name, memc,cent,pp) SENSOR_W_CENTAUR_NUM_HELPER(sensor_name,memc,C,cent,P,pp)

// This will stringify the enum so to create the sensor name.  This will help
// save keystrokes, as well as reduce typos & copy paste errors.
#define SENSOR_STRING(sensor_name) #sensor_name

// This will stringify the enum so to create the sensor name.  This will help
// save keystrokes, as well as reduce typos & copy paste errors.
#define CENTAUR_SENSOR_STRING_HELPER(sensor_name, memc,centL,cent,ppL,pp) SENSOR_STRING(sensor_name##memc##centL##cent##ppL##pp)
#define CENTAUR_SENSOR_STRING(sensor_name,memc,cent,pp)  CENTAUR_SENSOR_STRING_HELPER(sensor_name, memc,C,cent,P,pp)

// This will create a set of 6 sensor entries into the sensor list table.
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
                   .sensor = { units, type, location, number, frequency, scaleFactor },}


// This will create a set of 24 sensor entries into the sensor list table.
// (one for each core...)  The base name of the sensor enum must be passed
// and this macro will take care of the paste & stringify operations.
#define SENS_CORE_ENTRY_SET(sensor_name, units, type, location, number, frequency, scaleFactor)  \
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
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,8)] = {.name = SENSOR_STRING(sensor_name ## 8),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_NUM(sensor_name,9)] = {.name = SENSOR_STRING(sensor_name ## 9),   \
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
                   .sensor = { units, type, location, number, frequency, scaleFactor },}

// This will create a set of 8 sensor entries into the sensor list table.
// (one for each memc...)  The base name of the sensor enum must be passed
// and this macro will take care of the paste & stringify operations.
#define SENS_MEMC_ENTRY_SET(sensor_name, units, type, location, number, frequency, scaleFactor)  \
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


// This will create a set of 16 sensor entries into the sensor list table.
// (one for each centaur...)  The base name of the sensor enum must be passed
// and this macro will take care of the paste & stringify operations.
#define SEN_CENTR_ENTRY_SET(sensor_name, units, type, location, number, frequency, scaleFactor)  \
  [SENSOR_W_CENTAUR_NUM(sensor_name,0,0,0)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,0,0,0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,0,0,1)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,0,0,1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,1,0,0)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,1,0,0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,1,0,1)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,1,0,1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,2,0,0)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,2,0,0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,2,0,1)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,2,0,1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,3,0,0)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,3,0,0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,3,0,1)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,3,0,1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,4,0,0)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,4,0,0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,4,0,1)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,4,0,1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,5,0,0)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,5,0,0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,5,0,1)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,5,0,1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,6,0,0)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,6,0,0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,6,0,1)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,6,0,1),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,7,0,0)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,7,0,0),   \
                   .sensor = { units, type, location, number, frequency, scaleFactor },}, \
  [SENSOR_W_CENTAUR_NUM(sensor_name,7,0,1)] = {.name = CENTAUR_SENSOR_STRING(sensor_name,7,0,1),   \
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
  /* ==FirmwareSensors==    NameString             Units                      Type              Location             Number                Freq      ScaleFactor */
  SENSOR_INFO_T_ENTRY(       AMEintdur,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       AMESSdur0,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       AMESSdur1,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       AMESSdur2,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       AMESSdur3,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       AMESSdur4,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       AMESSdur5,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       AMESSdur6,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       AMESSdur7,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROBE250US0, "n/a\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROBE250US1, "n/a\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROBE250US2, "n/a\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROBE250US3, "n/a\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROBE250US4, "n/a\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROBE250US5, "n/a\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROBE250US6, "n/a\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PROBE250US7, "n/a\0", AMEC_SENSOR_TYPE_GENERIC,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     GPEtickdur0,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     GPEtickdur1,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(      RTLtickdur,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_OCC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),

  /* ==SystemSensors==      NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SENSOR_INFO_T_ENTRY(     TEMPAMBIENT,   "C\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(        ALTITUDE,   "m\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(        PWR250US,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PWR250USFAN,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(      PWR250USIO,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   PWR250USSTORE,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PWR250USGPU,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     FANSPEEDAVG, "RPM\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(      PWRAPSSCH0,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(      PWRAPSSCH1,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(      PWRAPSSCH2,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(      PWRAPSSCH3,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(      PWRAPSSCH4,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(      PWRAPSSCH5,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(      PWRAPSSCH6,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(      PWRAPSSCH7,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(      PWRAPSSCH8,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(      PWRAPSSCH9,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PWRAPSSCH10,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PWRAPSSCH11,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PWRAPSSCH12,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PWRAPSSCH13,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PWRAPSSCH14,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     PWRAPSSCH15,   "W\0",   AMEC_SENSOR_TYPE_POWER,  AMEC_SENSOR_LOC_SYS, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),


  /* ==ChipSensors==        NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SENSOR_INFO_T_ENTRY(       TODclock0,  "us\0",    AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_ALL, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP( 16, 0)  ),
  SENSOR_INFO_T_ENTRY(       TODclock1,  "sec\0",   AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_ALL, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP( 1048576,-6)  ),
  SENSOR_INFO_T_ENTRY(       TODclock2,  "day\0",   AMEC_SENSOR_TYPE_TIME,  AMEC_SENSOR_LOC_ALL, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP( 795364,-6)  ),

  /* ==ProcSensors==        NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SENSOR_INFO_T_ENTRY(      FREQA4MSP0, "MHz\0",    AMEC_SENSOR_TYPE_FREQ, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(        IPS4MSP0, "MIP\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(      MEMSP2MSP0,   "%\0",    AMEC_SENSOR_TYPE_TIME, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(      PWR250USP0,   "W\0",   AMEC_SENSOR_TYPE_POWER, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWR250USVDD0,   "W\0",   AMEC_SENSOR_TYPE_POWER, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    CUR250USVDD0,   "A\0", AMEC_SENSOR_TYPE_CURRENT, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1,-2)  ),
  SENSOR_INFO_T_ENTRY(    PWR250USVCS0,   "W\0",   AMEC_SENSOR_TYPE_POWER, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    PWR250USMEM0,   "W\0",   AMEC_SENSOR_TYPE_POWER, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   SLEEPCNT4MSP0,   "#\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    WINKCNT4MSP0,   "#\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       SP250USP0,   "%\0",    AMEC_SENSOR_TYPE_FREQ, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(     TEMPPROCAVG,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    TEMPPROCTHRM,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(       UTIL4MSP0,   "%\0",    AMEC_SENSOR_TYPE_UTIL, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1,-2)  ),
  SENSOR_INFO_T_ENTRY(        TEMPNEST,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(  VRFAN250USPROC, "pin\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(  VRHOT250USPROC, "pin\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_QUAD_ENTRY_SET(           TEMPQ,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_PROC, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1, 0)  ),

  /* ==ReguSensors==        NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SENSOR_INFO_T_ENTRY(  UVOLT250USP0V0,  "mV\0", AMEC_SENSOR_TYPE_VOLTAGE,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, -1)  ),
  SENSOR_INFO_T_ENTRY(  UVOLT250USP0V1,  "mV\0", AMEC_SENSOR_TYPE_VOLTAGE,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, -1)  ),
  SENSOR_INFO_T_ENTRY(   VOLT250USP0V0,  "mV\0", AMEC_SENSOR_TYPE_VOLTAGE,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, -1)  ),
  SENSOR_INFO_T_ENTRY(   VOLT250USP0V1,  "mV\0", AMEC_SENSOR_TYPE_VOLTAGE,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, -1)  ),

  /* ==CoreSensors==        NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SENS_CORE_ENTRY_SET(     FREQ250USP0C, "MHz\0",    AMEC_SENSOR_TYPE_FREQ, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(      FREQA4MSP0C, "MHz\0",    AMEC_SENSOR_TYPE_FREQ, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(        IPS4MSP0C, "MIP\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(     NOTBZE4MSP0C, "cyc\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(     NOTFIN4MSP0C, "cyc\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(    TEMPPROCTHRMC,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(       UTIL4MSP0C,   "%\0",    AMEC_SENSOR_TYPE_UTIL, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1,-2)  ),
  SENS_CORE_ENTRY_SET(       NUTIL3SP0C,   "%\0",    AMEC_SENSOR_TYPE_UTIL, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,    AMEEFP_3S_IN_HZ,   AMEFP(  1,-2)  ),
  SENS_CORE_ENTRY_SET(       MSTL2MSP0C, "cpi\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(        CMT2MSP0C,   "%\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(          PPICP0C,   "%\0",    AMEC_SENSOR_TYPE_PERF, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(    PWRPX250USP0C,   "W\0",   AMEC_SENSOR_TYPE_POWER, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_CORE_ENTRY_SET(            TEMPC,   "C\0",    AMEC_SENSOR_TYPE_TEMP, AMEC_SENSOR_LOC_CORE, AMEC_SENSOR_NONUM,   AMEEFP_4MS_IN_HZ,   AMEFP(  1, 0)  ),

  /* ==MemSensors==         NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SENSOR_INFO_T_ENTRY(   VRFAN250USMEM, "pin\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(   VRHOT250USMEM, "pin\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_VRM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_MEMC_ENTRY_SET(       MRD2MSP0M, "GBs\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  128, -5)  ),
  SENS_MEMC_ENTRY_SET(       MWR2MSP0M, "GBs\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  128, -5)  ),
  SENS_MEMC_ENTRY_SET(      MIRC2MSP0M, "eps\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_MEMC_ENTRY_SET(         MLP2P0M, "eps\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_MEMC_ENTRY_SET(   TEMPDIMMAXP0M,   "C\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),
  SENS_MEMC_ENTRY_SET(    LOCDIMMAXP0M, "loc\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  1, 0)  ),

  /* ==CentaurSensors==     NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SEN_CENTR_ENTRY_SET(       MAC2MSP0M, "rps\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SEN_CENTR_ENTRY_SET(       MPU2MSP0M, "rps\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SEN_CENTR_ENTRY_SET(      MIRB2MSP0M, "eps\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SEN_CENTR_ENTRY_SET(      MIRL2MSP0M, "eps\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SEN_CENTR_ENTRY_SET(      MIRM2MSP0M, "eps\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SEN_CENTR_ENTRY_SET(      MIRH2MSP0M, "eps\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SEN_CENTR_ENTRY_SET(       MTS2MSP0M, "cnt\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SEN_CENTR_ENTRY_SET(      MEMSP2MSPM,   "%\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SEN_CENTR_ENTRY_SET(      M4RD2MSP0M, "GBs\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  128, -5)  ),
  SEN_CENTR_ENTRY_SET(      M4WR2MSP0M, "GBs\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM,   AMEEFP_2MS_IN_HZ,   AMEFP(  128, -5)  ),


  /* ==MemSummarySensors==  NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SENSOR_INFO_T_ENTRY(     TEMP2MSCENT,   "C\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(    TEMP16MSDIMM,   "C\0",    AMEC_SENSOR_TYPE_TEMP,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),
  SENSOR_INFO_T_ENTRY(        MEMSP2MS,   "%\0",    AMEC_SENSOR_TYPE_PERF,  AMEC_SENSOR_LOC_MEM, AMEC_SENSOR_NONUM, AMEEFP_250US_IN_HZ,   AMEFP(  1, 0)  ),

  /* ==PartSummarySensors==  NameString  Units                      Type              Location             Number                Freq      ScaleFactor   */
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG000,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG001,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG002,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG003,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG004,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG005,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG006,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG007,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG008,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG009,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG010,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
  SENSOR_INFO_T_ENTRY(  UTIL2MSSLCG011,   "%\0",    AMEC_SENSOR_TYPE_UTIL,  AMEC_SENSOR_LOC_LPAR, AMEC_SENSOR_NONUM,  AMEEFP_2MS_IN_HZ,  AMEFP_SCALE_0_16384),
};

// Cause a compile error if we don't have all the sensors in the enum in the initialization list.
STATIC_ASSERT(   (NUMBER_OF_SENSORS_IN_LIST != (sizeof(G_sensor_info)/sizeof(sensor_info_t)))  );
STATIC_ASSERT(   (MAX_AMEC_SENSORS < (sizeof(G_sensor_info)/sizeof(sensor_info_t)))   );
