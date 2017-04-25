/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor_table.c $                           */
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


#include <sensor.h>           // sensor structure and other defines
#include <occ_common.h>           // For size_t needed by memset
#include <amec_sys.h>         // For pointers to amec_sys_t structures
#include <dcom.h>             // For mini-sensor pointers

extern amec_sys_t g_amec_sys;

// Will create an entry in the G_amec_sensor_list with a pointer at the sensor
// index (gsid) passed in by "sensor"
#define SENSOR_PTR(sensor,ptr) [sensor] = ptr

// Will paste number onto enum 'sensor base name'
#define SENSOR_W_NUM(sensor,num) sensor##num

// This will paste a number onto a sensor name base to create the full enum
// representation of the sensor name
#define SENSOR_W_CENTAUR_NUM_HELPER(sensor_name, memc,centL,cent,ppL,pp) sensor_name##memc##centL##cent##ppL##pp
#define SENSOR_W_CENTAUR_NUM(sensor_name, memc,cent,pp) SENSOR_W_CENTAUR_NUM_HELPER(sensor_name,memc,C,cent,P,pp)

// Will define a set of "core sensor pointers" by passing in base sensor name
// and ptr to [0] entry of array of 24 core sensors
#define CORE_SENSOR_PTRS(sensor,ptrbase,ptrmember) \
  [SENSOR_W_NUM(sensor, 0)] = ptrbase[ 0].ptrmember, \
  [SENSOR_W_NUM(sensor, 1)] = ptrbase[ 1].ptrmember, \
  [SENSOR_W_NUM(sensor, 2)] = ptrbase[ 2].ptrmember, \
  [SENSOR_W_NUM(sensor, 3)] = ptrbase[ 3].ptrmember, \
  [SENSOR_W_NUM(sensor, 4)] = ptrbase[ 4].ptrmember, \
  [SENSOR_W_NUM(sensor, 5)] = ptrbase[ 5].ptrmember, \
  [SENSOR_W_NUM(sensor, 6)] = ptrbase[ 6].ptrmember, \
  [SENSOR_W_NUM(sensor, 7)] = ptrbase[ 7].ptrmember, \
  [SENSOR_W_NUM(sensor, 8)] = ptrbase[ 8].ptrmember, \
  [SENSOR_W_NUM(sensor, 9)] = ptrbase[ 9].ptrmember, \
  [SENSOR_W_NUM(sensor,10)] = ptrbase[10].ptrmember, \
  [SENSOR_W_NUM(sensor,11)] = ptrbase[11].ptrmember, \
  [SENSOR_W_NUM(sensor,12)] = ptrbase[12].ptrmember, \
  [SENSOR_W_NUM(sensor,13)] = ptrbase[13].ptrmember, \
  [SENSOR_W_NUM(sensor,14)] = ptrbase[14].ptrmember, \
  [SENSOR_W_NUM(sensor,15)] = ptrbase[15].ptrmember, \
  [SENSOR_W_NUM(sensor,16)] = ptrbase[16].ptrmember, \
  [SENSOR_W_NUM(sensor,17)] = ptrbase[17].ptrmember, \
  [SENSOR_W_NUM(sensor,18)] = ptrbase[18].ptrmember, \
  [SENSOR_W_NUM(sensor,19)] = ptrbase[19].ptrmember, \
  [SENSOR_W_NUM(sensor,20)] = ptrbase[20].ptrmember, \
  [SENSOR_W_NUM(sensor,21)] = ptrbase[21].ptrmember, \
  [SENSOR_W_NUM(sensor,22)] = ptrbase[22].ptrmember, \
  [SENSOR_W_NUM(sensor,23)] = ptrbase[23].ptrmember

// Will define a set of "quad sensor pointers" by passing in base sensor name
// and ptr to [0] entry of array of 6 quad sensors
#define QUAD_SENSOR_PTRS(sensor,ptrbase,ptrmember) \
  [SENSOR_W_NUM(sensor, 0)] = ptrbase[ 0].ptrmember, \
  [SENSOR_W_NUM(sensor, 1)] = ptrbase[ 1].ptrmember, \
  [SENSOR_W_NUM(sensor, 2)] = ptrbase[ 2].ptrmember, \
  [SENSOR_W_NUM(sensor, 3)] = ptrbase[ 3].ptrmember, \
  [SENSOR_W_NUM(sensor, 4)] = ptrbase[ 4].ptrmember, \
  [SENSOR_W_NUM(sensor, 5)] = ptrbase[ 5].ptrmember


// Will define a set of "memory controller sensor pointers" by passing in
// base sensor nameand ptr to [0] entry of array of 8 memcontroller sensors
#define MEMCONTROL_SENSOR_PTRS(sensor,ptrbase,ptrmember) \
  [SENSOR_W_NUM(sensor, 0)] = ptrbase[0].ptrmember, \
  [SENSOR_W_NUM(sensor, 1)] = ptrbase[1].ptrmember, \
  [SENSOR_W_NUM(sensor, 2)] = ptrbase[2].ptrmember, \
  [SENSOR_W_NUM(sensor, 3)] = ptrbase[3].ptrmember, \
  [SENSOR_W_NUM(sensor, 4)] = ptrbase[4].ptrmember, \
  [SENSOR_W_NUM(sensor, 5)] = ptrbase[5].ptrmember, \
  [SENSOR_W_NUM(sensor, 6)] = ptrbase[6].ptrmember, \
  [SENSOR_W_NUM(sensor, 7)] = ptrbase[7].ptrmember

// Will define a set of "memory controller sensor pointers" by passing in
// base sensor nameand ptr to [0] entry of array of 8 memcontroller sensors
#define DIMM_SENSOR_PTRS(sensor,ptrbase,ptrmember) \
  [SENSOR_W_NUM(sensor, 00)] = ptrbase.ptrmember[ 0], \
  [SENSOR_W_NUM(sensor, 01)] = ptrbase.ptrmember[ 1], \
  [SENSOR_W_NUM(sensor, 02)] = ptrbase.ptrmember[ 2], \
  [SENSOR_W_NUM(sensor, 03)] = ptrbase.ptrmember[ 3], \
  [SENSOR_W_NUM(sensor, 04)] = ptrbase.ptrmember[ 4], \
  [SENSOR_W_NUM(sensor, 05)] = ptrbase.ptrmember[ 5], \
  [SENSOR_W_NUM(sensor, 06)] = ptrbase.ptrmember[ 6], \
  [SENSOR_W_NUM(sensor, 07)] = ptrbase.ptrmember[ 7], \
  [SENSOR_W_NUM(sensor, 08)] = ptrbase.ptrmember[ 8], \
  [SENSOR_W_NUM(sensor, 09)] = ptrbase.ptrmember[ 9], \
  [SENSOR_W_NUM(sensor, 10)] = ptrbase.ptrmember[10], \
  [SENSOR_W_NUM(sensor, 11)] = ptrbase.ptrmember[11], \
  [SENSOR_W_NUM(sensor, 12)] = ptrbase.ptrmember[12], \
  [SENSOR_W_NUM(sensor, 13)] = ptrbase.ptrmember[13], \
  [SENSOR_W_NUM(sensor, 14)] = ptrbase.ptrmember[14], \
  [SENSOR_W_NUM(sensor, 15)] = ptrbase.ptrmember[15]

// Will define a set of "centaur_port_pair sensor pointers" by passing in
// base sensor nameand ptr to [0] entry of array of 16 memcontroller sensors
#define PORTPAIR_SENSOR_PTRS(sensor,ptrbase,ptrmember,ptrsnsr) \
  [SENSOR_W_CENTAUR_NUM(sensor, 0, 0, 0)] = ptrbase[0].ptrmember[0].ptrsnsr, \
  [SENSOR_W_CENTAUR_NUM(sensor, 0, 0, 1)] = ptrbase[0].ptrmember[1].ptrsnsr, \
  [SENSOR_W_CENTAUR_NUM(sensor, 1, 0, 0)] = ptrbase[1].ptrmember[0].ptrsnsr, \
  [SENSOR_W_CENTAUR_NUM(sensor, 1, 0, 1)] = ptrbase[1].ptrmember[1].ptrsnsr, \
  [SENSOR_W_CENTAUR_NUM(sensor, 2, 0, 0)] = ptrbase[2].ptrmember[0].ptrsnsr, \
  [SENSOR_W_CENTAUR_NUM(sensor, 2, 0, 1)] = ptrbase[2].ptrmember[1].ptrsnsr, \
  [SENSOR_W_CENTAUR_NUM(sensor, 3, 0, 0)] = ptrbase[3].ptrmember[0].ptrsnsr, \
  [SENSOR_W_CENTAUR_NUM(sensor, 3, 0, 1)] = ptrbase[3].ptrmember[1].ptrsnsr, \
  [SENSOR_W_CENTAUR_NUM(sensor, 4, 0, 0)] = ptrbase[4].ptrmember[0].ptrsnsr, \
  [SENSOR_W_CENTAUR_NUM(sensor, 4, 0, 1)] = ptrbase[4].ptrmember[1].ptrsnsr, \
  [SENSOR_W_CENTAUR_NUM(sensor, 5, 0, 0)] = ptrbase[5].ptrmember[0].ptrsnsr, \
  [SENSOR_W_CENTAUR_NUM(sensor, 5, 0, 1)] = ptrbase[5].ptrmember[1].ptrsnsr, \
  [SENSOR_W_CENTAUR_NUM(sensor, 6, 0, 0)] = ptrbase[6].ptrmember[0].ptrsnsr, \
  [SENSOR_W_CENTAUR_NUM(sensor, 6, 0, 1)] = ptrbase[6].ptrmember[1].ptrsnsr, \
  [SENSOR_W_CENTAUR_NUM(sensor, 7, 0, 0)] = ptrbase[7].ptrmember[0].ptrsnsr, \
  [SENSOR_W_CENTAUR_NUM(sensor, 7, 0, 1)] = ptrbase[7].ptrmember[1].ptrsnsr


// Will create an entry in the G_amec_mini_sensor_list with a pointer at
// the sensor index (gsid) passed in by "sensor"
#define MINI_SENSOR_PTR(sensor,ptr) [sensor] = ptr

// Will define a set of "core mini-sensor pointers" by passing in base
// sensor name and ptr to [0] entry of array of core sensors
#define CORE_MINI_SENSOR_PTRS(sensor,ptr) \
  [SENSOR_W_NUM(sensor, 0)] = ptr[ 0], \
  [SENSOR_W_NUM(sensor, 1)] = ptr[ 1], \
  [SENSOR_W_NUM(sensor, 2)] = ptr[ 2], \
  [SENSOR_W_NUM(sensor, 3)] = ptr[ 3], \
  [SENSOR_W_NUM(sensor, 4)] = ptr[ 4], \
  [SENSOR_W_NUM(sensor, 5)] = ptr[ 5], \
  [SENSOR_W_NUM(sensor, 6)] = ptr[ 6], \
  [SENSOR_W_NUM(sensor, 7)] = ptr[ 7], \
  [SENSOR_W_NUM(sensor, 8)] = ptr[ 8], \
  [SENSOR_W_NUM(sensor, 9)] = ptr[ 9], \
  [SENSOR_W_NUM(sensor,10)] = ptr[10], \
  [SENSOR_W_NUM(sensor,11)] = ptr[11], \
  [SENSOR_W_NUM(sensor,12)] = ptr[12], \
  [SENSOR_W_NUM(sensor,13)] = ptr[13], \
  [SENSOR_W_NUM(sensor,14)] = ptr[14], \
  [SENSOR_W_NUM(sensor,15)] = ptr[15], \
  [SENSOR_W_NUM(sensor,16)] = ptr[16], \
  [SENSOR_W_NUM(sensor,17)] = ptr[17], \
  [SENSOR_W_NUM(sensor,18)] = ptr[18], \
  [SENSOR_W_NUM(sensor,19)] = ptr[19], \
  [SENSOR_W_NUM(sensor,20)] = ptr[20], \
  [SENSOR_W_NUM(sensor,21)] = ptr[21], \
  [SENSOR_W_NUM(sensor,22)] = ptr[22], \
  [SENSOR_W_NUM(sensor,23)] = ptr[23]

// every sensor must have a mini-sensor.
#define CORE_MINI_SENSOR_PTRS_NULL(sensor) \
  [SENSOR_W_NUM(sensor, 0)] = NULL, \
  [SENSOR_W_NUM(sensor, 1)] = NULL, \
  [SENSOR_W_NUM(sensor, 2)] = NULL, \
  [SENSOR_W_NUM(sensor, 3)] = NULL, \
  [SENSOR_W_NUM(sensor, 4)] = NULL, \
  [SENSOR_W_NUM(sensor, 5)] = NULL, \
  [SENSOR_W_NUM(sensor, 6)] = NULL, \
  [SENSOR_W_NUM(sensor, 7)] = NULL, \
  [SENSOR_W_NUM(sensor, 8)] = NULL, \
  [SENSOR_W_NUM(sensor, 9)] = NULL, \
  [SENSOR_W_NUM(sensor,10)] = NULL, \
  [SENSOR_W_NUM(sensor,11)] = NULL, \
  [SENSOR_W_NUM(sensor,12)] = NULL, \
  [SENSOR_W_NUM(sensor,13)] = NULL, \
  [SENSOR_W_NUM(sensor,14)] = NULL, \
  [SENSOR_W_NUM(sensor,15)] = NULL, \
  [SENSOR_W_NUM(sensor,16)] = NULL, \
  [SENSOR_W_NUM(sensor,17)] = NULL, \
  [SENSOR_W_NUM(sensor,18)] = NULL, \
  [SENSOR_W_NUM(sensor,19)] = NULL, \
  [SENSOR_W_NUM(sensor,20)] = NULL, \
  [SENSOR_W_NUM(sensor,21)] = NULL, \
  [SENSOR_W_NUM(sensor,22)] = NULL, \
  [SENSOR_W_NUM(sensor,23)] = NULL

// Will define a set of "quad mini-sensor pointers" by passing in base
// sensor name and ptr to [0] entry of array of quad sensors
#define QUAD_MINI_SENSOR_PTRS(sensor,ptr) \
  [SENSOR_W_NUM(sensor, 0)] = ptr[ 0], \
  [SENSOR_W_NUM(sensor, 1)] = ptr[ 1], \
  [SENSOR_W_NUM(sensor, 2)] = ptr[ 2], \
  [SENSOR_W_NUM(sensor, 3)] = ptr[ 3], \
  [SENSOR_W_NUM(sensor, 4)] = ptr[ 4], \
  [SENSOR_W_NUM(sensor, 5)] = ptr[ 5]

// every sensor must have a mini-sensor.
#define QUAD_MINI_SENSOR_PTRS_NULL(sensor) \
  [SENSOR_W_NUM(sensor, 0)] = NULL, \
  [SENSOR_W_NUM(sensor, 1)] = NULL, \
  [SENSOR_W_NUM(sensor, 2)] = NULL, \
  [SENSOR_W_NUM(sensor, 3)] = NULL, \
  [SENSOR_W_NUM(sensor, 4)] = NULL, \
  [SENSOR_W_NUM(sensor, 5)] = NULL

// Will define a set of "memory controller mini sensor ptrs" by passing in
// base sensor nameand ptr to [0] entry of array of 8 memcontroller sensors
#define MEMCONTROL_MINI_SENSOR_PTRS(sensor,ptr) \
  [SENSOR_W_NUM(sensor, 0)] = ptr[ 0], \
  [SENSOR_W_NUM(sensor, 1)] = ptr[ 1], \
  [SENSOR_W_NUM(sensor, 2)] = ptr[ 2], \
  [SENSOR_W_NUM(sensor, 3)] = ptr[ 3], \
  [SENSOR_W_NUM(sensor, 4)] = ptr[ 4], \
  [SENSOR_W_NUM(sensor, 5)] = ptr[ 5], \
  [SENSOR_W_NUM(sensor, 6)] = ptr[ 6], \
  [SENSOR_W_NUM(sensor, 7)] = ptr[ 7]

// Will define a set of "memc mini-sensor pointers" as NULL, since not
// every sensor must have a mini-sensor.
#define MEMCONTROL_MINI_SENSOR_PTRS_NULL(sensor) \
  [SENSOR_W_NUM(sensor, 0)] = NULL, \
  [SENSOR_W_NUM(sensor, 1)] = NULL, \
  [SENSOR_W_NUM(sensor, 2)] = NULL, \
  [SENSOR_W_NUM(sensor, 3)] = NULL, \
  [SENSOR_W_NUM(sensor, 4)] = NULL, \
  [SENSOR_W_NUM(sensor, 5)] = NULL, \
  [SENSOR_W_NUM(sensor, 6)] = NULL, \
  [SENSOR_W_NUM(sensor, 7)] = NULL

// Will define a set of "memory controller mini sensor ptrs" by passing in
// base sensor nameand ptr to [0] entry of array of 8 memcontroller sensors
#define PORTPAIR_MINI_SENSOR_PTRS(sensor,ptr) \
  [SENSOR_W_CENTAUR_NUM(sensor, 0, 0, 0)] = ptr[ 0], \
  [SENSOR_W_CENTAUR_NUM(sensor, 0, 0, 1)] = ptr[ 1], \
  [SENSOR_W_CENTAUR_NUM(sensor, 1, 0, 0)] = ptr[ 2], \
  [SENSOR_W_CENTAUR_NUM(sensor, 1, 0, 1)] = ptr[ 3], \
  [SENSOR_W_CENTAUR_NUM(sensor, 2, 0, 0)] = ptr[ 4], \
  [SENSOR_W_CENTAUR_NUM(sensor, 2, 0, 1)] = ptr[ 5], \
  [SENSOR_W_CENTAUR_NUM(sensor, 3, 0, 0)] = ptr[ 6], \
  [SENSOR_W_CENTAUR_NUM(sensor, 3, 0, 1)] = ptr[ 7], \
  [SENSOR_W_CENTAUR_NUM(sensor, 4, 0, 0)] = ptr[ 8], \
  [SENSOR_W_CENTAUR_NUM(sensor, 4, 0, 1)] = ptr[ 9], \
  [SENSOR_W_CENTAUR_NUM(sensor, 5, 0, 0)] = ptr[10], \
  [SENSOR_W_CENTAUR_NUM(sensor, 5, 0, 1)] = ptr[11], \
  [SENSOR_W_CENTAUR_NUM(sensor, 6, 0, 0)] = ptr[12], \
  [SENSOR_W_CENTAUR_NUM(sensor, 6, 0, 1)] = ptr[13], \
  [SENSOR_W_CENTAUR_NUM(sensor, 7, 0, 0)] = ptr[14], \
  [SENSOR_W_CENTAUR_NUM(sensor, 7, 0, 1)] = ptr[15]

// Will define a set of "memc mini-sensor pointers" as NULL, since not
// every sensor must have a mini-sensor.
#define PORTPAIR_MINI_SENSOR_PTRS_NULL(sensor) \
  [SENSOR_W_CENTAUR_NUM(sensor, 0, 0, 0)] = NULL, \
  [SENSOR_W_CENTAUR_NUM(sensor, 0, 0, 1)] = NULL, \
  [SENSOR_W_CENTAUR_NUM(sensor, 1, 0, 0)] = NULL, \
  [SENSOR_W_CENTAUR_NUM(sensor, 1, 0, 1)] = NULL, \
  [SENSOR_W_CENTAUR_NUM(sensor, 2, 0, 0)] = NULL, \
  [SENSOR_W_CENTAUR_NUM(sensor, 2, 0, 1)] = NULL, \
  [SENSOR_W_CENTAUR_NUM(sensor, 3, 0, 0)] = NULL, \
  [SENSOR_W_CENTAUR_NUM(sensor, 3, 0, 1)] = NULL, \
  [SENSOR_W_CENTAUR_NUM(sensor, 4, 0, 0)] = NULL, \
  [SENSOR_W_CENTAUR_NUM(sensor, 4, 0, 1)] = NULL, \
  [SENSOR_W_CENTAUR_NUM(sensor, 5, 0, 0)] = NULL, \
  [SENSOR_W_CENTAUR_NUM(sensor, 5, 0, 1)] = NULL, \
  [SENSOR_W_CENTAUR_NUM(sensor, 6, 0, 0)] = NULL, \
  [SENSOR_W_CENTAUR_NUM(sensor, 6, 0, 1)] = NULL, \
  [SENSOR_W_CENTAUR_NUM(sensor, 7, 0, 0)] = NULL, \
  [SENSOR_W_CENTAUR_NUM(sensor, 7, 0, 1)] = NULL

//****************************************************************************
// Sensor Pointer Table
// ----------------------
//   - Indexed by GSID
//   - Resident in SRAM
//   - Must contain every sensor in enum, or STATIC_ASSERT will give compile
//     failure.
//****************************************************************************
const sensor_ptr_t G_amec_sensor_list[] =
{
  // ------------------------------------------------------
  // Code/Firmware Sensors
  // ------------------------------------------------------
  SENSOR_PTR(     AMEintdur,        &g_amec_sys.fw.ameintdur),
  SENSOR_PTR(     AMESSdur0,        &g_amec_sys.fw.amessdur[0]),
  SENSOR_PTR(     AMESSdur1,        &g_amec_sys.fw.amessdur[1]),
  SENSOR_PTR(     AMESSdur2,        &g_amec_sys.fw.amessdur[2]),
  SENSOR_PTR(     AMESSdur3,        &g_amec_sys.fw.amessdur[3]),
  SENSOR_PTR(     AMESSdur4,        &g_amec_sys.fw.amessdur[4]),
  SENSOR_PTR(     AMESSdur5,        &g_amec_sys.fw.amessdur[5]),
  SENSOR_PTR(     AMESSdur6,        &g_amec_sys.fw.amessdur[6]),
  SENSOR_PTR(     AMESSdur7,        &g_amec_sys.fw.amessdur[7]),
  SENSOR_PTR(   PROBE250US0,        &g_amec_sys.fw.probe250us[0]),
  SENSOR_PTR(   PROBE250US1,        &g_amec_sys.fw.probe250us[1]),
  SENSOR_PTR(   PROBE250US2,        &g_amec_sys.fw.probe250us[2]),
  SENSOR_PTR(   PROBE250US3,        &g_amec_sys.fw.probe250us[3]),
  SENSOR_PTR(   PROBE250US4,        &g_amec_sys.fw.probe250us[4]),
  SENSOR_PTR(   PROBE250US5,        &g_amec_sys.fw.probe250us[5]),
  SENSOR_PTR(   PROBE250US6,        &g_amec_sys.fw.probe250us[6]),
  SENSOR_PTR(   PROBE250US7,        &g_amec_sys.fw.probe250us[7]),
  SENSOR_PTR(   GPEtickdur0,        &g_amec_sys.fw.gpetickdur[0]),
  SENSOR_PTR(   GPEtickdur1,        &g_amec_sys.fw.gpetickdur[1]),
  SENSOR_PTR(    RTLtickdur,        &g_amec_sys.fw.prcdupdatedur),

  // ------------------------------------------------------
  // System Sensors
  // ------------------------------------------------------
  SENSOR_PTR( PWRSYS,               &g_amec_sys.sys.pwrsys),
  SENSOR_PTR( PWR250USFAN,          &g_amec_sys.fan.pwr250usfan),
  SENSOR_PTR( PWR250USIO,           &g_amec_sys.io.pwr250usio),
  SENSOR_PTR( PWR250USSTORE,        &g_amec_sys.storage.pwr250usstore),
  SENSOR_PTR( PWRGPU,               &g_amec_sys.sys.pwr250usgpu),
  SENSOR_PTR( PWRAPSSCH0,           &g_amec_sys.sys.pwrapssch[0]),
  SENSOR_PTR( PWRAPSSCH1,           &g_amec_sys.sys.pwrapssch[1]),
  SENSOR_PTR( PWRAPSSCH2,           &g_amec_sys.sys.pwrapssch[2]),
  SENSOR_PTR( PWRAPSSCH3,           &g_amec_sys.sys.pwrapssch[3]),
  SENSOR_PTR( PWRAPSSCH4,           &g_amec_sys.sys.pwrapssch[4]),
  SENSOR_PTR( PWRAPSSCH5,           &g_amec_sys.sys.pwrapssch[5]),
  SENSOR_PTR( PWRAPSSCH6,           &g_amec_sys.sys.pwrapssch[6]),
  SENSOR_PTR( PWRAPSSCH7,           &g_amec_sys.sys.pwrapssch[7]),
  SENSOR_PTR( PWRAPSSCH8,           &g_amec_sys.sys.pwrapssch[8]),
  SENSOR_PTR( PWRAPSSCH9,           &g_amec_sys.sys.pwrapssch[9]),
  SENSOR_PTR( PWRAPSSCH10,          &g_amec_sys.sys.pwrapssch[10]),
  SENSOR_PTR( PWRAPSSCH11,          &g_amec_sys.sys.pwrapssch[11]),
  SENSOR_PTR( PWRAPSSCH12,          &g_amec_sys.sys.pwrapssch[12]),
  SENSOR_PTR( PWRAPSSCH13,          &g_amec_sys.sys.pwrapssch[13]),
  SENSOR_PTR( PWRAPSSCH14,          &g_amec_sys.sys.pwrapssch[14]),
  SENSOR_PTR( PWRAPSSCH15,          &g_amec_sys.sys.pwrapssch[15]),
  SENSOR_PTR( CUR12VSTBY,           &g_amec_sys.sys.cur12Vstby),

  // ------------------------------------------------------
  // Chip Sensors
  // ------------------------------------------------------
  SENSOR_PTR( TODclock0,            &g_amec_sys.sys.todclock0 ),
  SENSOR_PTR( TODclock1,            &g_amec_sys.sys.todclock1 ),
  SENSOR_PTR( TODclock2,            &g_amec_sys.sys.todclock2 ),

  // ------------------------------------------------------
  // Processor Sensors
  // ------------------------------------------------------
  SENSOR_PTR( FREQA,                &g_amec_sys.proc[0].freqa),
  SENSOR_PTR( IPS4MSP0,             &g_amec_sys.proc[0].ips4ms),
  SENSOR_PTR( MEMSP2MSP0,           &g_amec_sys.proc[0].memsp2ms),
  SENSOR_PTR( PWRPROC,              &g_amec_sys.proc[0].pwrproc),
  SENSOR_PTR( PWR250USVDD0,         &g_amec_sys.proc[0].pwr250usvdd),
  SENSOR_PTR( PWRVCSVIOVDN,         &g_amec_sys.proc[0].pwrvcsviovdn),
  SENSOR_PTR( PWR250USMEM0,         &g_amec_sys.proc[0].pwr250usmem),
  SENSOR_PTR( SLEEPCNT4MSP0,        &g_amec_sys.proc[0].sleepcnt4ms),
  SENSOR_PTR( WINKCNT4MSP0,         &g_amec_sys.proc[0].winkcnt4ms),
  SENSOR_PTR( SP250USP0,            &g_amec_sys.proc[0].sp250us),
  SENSOR_PTR( TEMPPROCAVG,          &g_amec_sys.proc[0].tempprocavg),
  SENSOR_PTR( TEMPPROCTHRM,         &g_amec_sys.proc[0].tempprocthermal),
  SENSOR_PTR( UTIL,                 &g_amec_sys.proc[0].util),
  SENSOR_PTR( TEMPNEST,             &g_amec_sys.proc[0].tempnest),
  SENSOR_PTR( VOLTVDDSENSE,         &g_amec_sys.fw.voltvddsense),
  SENSOR_PTR( VOLTVDNSENSE,         &g_amec_sys.fw.voltvdnsense),
  SENSOR_PTR( PWRVDD,               &g_amec_sys.proc[0].pwrvdd),
  SENSOR_PTR( PWRVDN,               &g_amec_sys.proc[0].pwrvdn),

  // ------------------------------------------------------
  // Quad Sensors (6 each)
  // ------------------------------------------------------
  QUAD_SENSOR_PTRS( TEMPQ,          &g_amec_sys.proc[0].quad, tempq),

  // ------------------------------------------------------
  // Core Sensors (24 of each)
  // ------------------------------------------------------
  CORE_SENSOR_PTRS( FREQ250USP0C ,  &g_amec_sys.proc[0].core, freq250us),
  CORE_SENSOR_PTRS( FREQAC ,        &g_amec_sys.proc[0].core, freqa),
  CORE_SENSOR_PTRS( IPS4MSP0C ,     &g_amec_sys.proc[0].core, ips4ms),
  CORE_SENSOR_PTRS( NOTBZE4MSP0C ,  &g_amec_sys.proc[0].core, mcpifd4ms),
  CORE_SENSOR_PTRS( NOTFIN4MSP0C ,  &g_amec_sys.proc[0].core, mcpifi4ms),
  CORE_SENSOR_PTRS( TEMPPROCTHRMC , &g_amec_sys.proc[0].core, tempprocthermal),
  CORE_SENSOR_PTRS( UTILC ,         &g_amec_sys.proc[0].core, util),
  CORE_SENSOR_PTRS( NUTIL3SP0C ,    &g_amec_sys.proc[0].core, nutil3s),
  CORE_SENSOR_PTRS( MSTL2MSP0C ,    &g_amec_sys.proc[0].core, mstl2ms),
  CORE_SENSOR_PTRS( CMT2MSP0C ,     &g_amec_sys.proc[0].core, cmt2ms),
  CORE_SENSOR_PTRS( PPICP0C ,       &g_amec_sys.proc[0].core, ppic),
  CORE_SENSOR_PTRS( PWRPX250USP0C , &g_amec_sys.proc[0].core, pwrpx250us),
  CORE_SENSOR_PTRS( TEMPC,          &g_amec_sys.proc[0].core, tempc),

  // ------------------------------------------------------
  // Memory Sensors
  // ------------------------------------------------------
  MEMCONTROL_SENSOR_PTRS(MRD2MSP0M,     &g_amec_sys.proc[0].memctl, mrd2ms),
  MEMCONTROL_SENSOR_PTRS(MWR2MSP0M,     &g_amec_sys.proc[0].memctl, mwr2ms),
  MEMCONTROL_SENSOR_PTRS(MIRC2MSP0M,    &g_amec_sys.proc[0].memctl, centaur.mirc2ms),
  MEMCONTROL_SENSOR_PTRS(MLP2P0M,       &g_amec_sys.proc[0].memctl, centaur.mlp2ms),
  DIMM_SENSOR_PTRS(TEMPDIMM,            &g_amec_sys.proc[0],        tempdimm),
  MEMCONTROL_SENSOR_PTRS(TEMPDIMMAXP0M, &g_amec_sys.proc[0].memctl, centaur.tempdimmax),
  MEMCONTROL_SENSOR_PTRS(LOCDIMMAXP0M,  &g_amec_sys.proc[0].memctl, centaur.locdimmax),

  PORTPAIR_SENSOR_PTRS(MAC2MSP0M,   &g_amec_sys.proc[0].memctl, centaur.portpair, mac2ms),
  PORTPAIR_SENSOR_PTRS(MPU2MSP0M,   &g_amec_sys.proc[0].memctl, centaur.portpair, mpu2ms),
  PORTPAIR_SENSOR_PTRS(MIRB2MSP0M,  &g_amec_sys.proc[0].memctl, centaur.portpair, mirb2ms),
  PORTPAIR_SENSOR_PTRS(MIRL2MSP0M,  &g_amec_sys.proc[0].memctl, centaur.portpair, mirl2ms),
  PORTPAIR_SENSOR_PTRS(MIRM2MSP0M,  &g_amec_sys.proc[0].memctl, centaur.portpair, mirm2ms),
  PORTPAIR_SENSOR_PTRS(MIRH2MSP0M,  &g_amec_sys.proc[0].memctl, centaur.portpair, mirh2ms),
  PORTPAIR_SENSOR_PTRS(MTS2MSP0M,   &g_amec_sys.proc[0].memctl, centaur.portpair, mts2ms),
  PORTPAIR_SENSOR_PTRS(MEMSP2MSPM,  &g_amec_sys.proc[0].memctl, centaur.portpair, memsp2ms),
  PORTPAIR_SENSOR_PTRS(M4RD2MSP0M,  &g_amec_sys.proc[0].memctl, centaur.portpair, m4rd2ms),
  PORTPAIR_SENSOR_PTRS(M4WR2MSP0M,  &g_amec_sys.proc[0].memctl, centaur.portpair, m4wr2ms),


  SENSOR_PTR(TEMP2MSCENT,           &g_amec_sys.proc[0].temp2mscent),
  SENSOR_PTR(TEMPDIMMTHRM,          &g_amec_sys.proc[0].tempdimmthrm),
  SENSOR_PTR(MEMSP2MS,              &g_amec_sys.proc[0].memsp2ms_tls),

  // ------------------------------------------------------
  // Regulator Sensors
  // ------------------------------------------------------
  SENSOR_PTR( UVOLT250USP0V0,       &g_amec_sys.proc[0].vrm[0].uvolt250us),
  SENSOR_PTR( UVOLT250USP0V1,       &g_amec_sys.proc[0].vrm[1].uvolt250us),
  SENSOR_PTR( VOLTVDD,              &g_amec_sys.proc[0].vrm[0].volt250us),
  SENSOR_PTR( VOLTVDN,              &g_amec_sys.proc[0].vrm[1].volt250us),
  SENSOR_PTR( CURVDD,               &g_amec_sys.proc[0].curvdd),
  SENSOR_PTR( CURVDN,               &g_amec_sys.proc[0].curvdn),
  SENSOR_PTR( VRFAN,                &g_amec_sys.sys.vrfan),

  // ------------------------------------------------------
  // Partition Sensors
  // ------------------------------------------------------
  SENSOR_PTR( UTIL2MSSLCG000,       &g_amec_sys.part_config.part_list[0].util2msslack),
  SENSOR_PTR( UTIL2MSSLCG001,       &g_amec_sys.part_config.part_list[1].util2msslack),
  SENSOR_PTR( UTIL2MSSLCG002,       &g_amec_sys.part_config.part_list[2].util2msslack),
  SENSOR_PTR( UTIL2MSSLCG003,       &g_amec_sys.part_config.part_list[3].util2msslack),
  SENSOR_PTR( UTIL2MSSLCG004,       &g_amec_sys.part_config.part_list[4].util2msslack),
  SENSOR_PTR( UTIL2MSSLCG005,       &g_amec_sys.part_config.part_list[5].util2msslack),
  SENSOR_PTR( UTIL2MSSLCG006,       &g_amec_sys.part_config.part_list[6].util2msslack),
  SENSOR_PTR( UTIL2MSSLCG007,       &g_amec_sys.part_config.part_list[7].util2msslack),
  SENSOR_PTR( UTIL2MSSLCG008,       &g_amec_sys.part_config.part_list[8].util2msslack),
  SENSOR_PTR( UTIL2MSSLCG009,       &g_amec_sys.part_config.part_list[9].util2msslack),
  SENSOR_PTR( UTIL2MSSLCG010,       &g_amec_sys.part_config.part_list[10].util2msslack),
  SENSOR_PTR( UTIL2MSSLCG011,       &g_amec_sys.part_config.part_list[11].util2msslack),

};
STATIC_ASSERT(   (NUMBER_OF_SENSORS_IN_LIST != (sizeof(G_amec_sensor_list)/sizeof(sensor_ptr_t)))   );
STATIC_ASSERT(   (MAX_AMEC_SENSORS < (sizeof(G_amec_sensor_list)/sizeof(sensor_ptr_t)))   );


//****************************************************************************
// Mini-Sensor Pointer Table
// ----------------------
//   - Indexed by GSID
//   - If an initSection is utilized, this will reside there and will be
//     deleted after sensor init
//   - Must contain every sensor in enum, or STATIC_ASSERT will give compile
//     failure.
//   - Pointers to mini-sensors can be NULL, but they must still be in array.
//   - The reason why this is a separate table is so that we can put this
//     in the init section and reclaim ~1.5kB of space (# sensors * 4bytes)
//****************************************************************************
const minisensor_ptr_t G_amec_mini_sensor_list[] INIT_SECTION =
{
  // ------------------------------------------------------
  // Code/Firmware Sensors
  // ------------------------------------------------------
  MINI_SENSOR_PTR(      AMEintdur,  NULL),
  MINI_SENSOR_PTR(      AMESSdur0,  NULL),
  MINI_SENSOR_PTR(      AMESSdur1,  NULL),
  MINI_SENSOR_PTR(      AMESSdur2,  NULL),
  MINI_SENSOR_PTR(      AMESSdur3,  NULL),
  MINI_SENSOR_PTR(      AMESSdur4,  NULL),
  MINI_SENSOR_PTR(      AMESSdur5,  NULL),
  MINI_SENSOR_PTR(      AMESSdur6,  NULL),
  MINI_SENSOR_PTR(      AMESSdur7,  NULL),
  MINI_SENSOR_PTR(    PROBE250US0,  NULL),
  MINI_SENSOR_PTR(    PROBE250US1,  NULL),
  MINI_SENSOR_PTR(    PROBE250US2,  NULL),
  MINI_SENSOR_PTR(    PROBE250US3,  NULL),
  MINI_SENSOR_PTR(    PROBE250US4,  NULL),
  MINI_SENSOR_PTR(    PROBE250US5,  NULL),
  MINI_SENSOR_PTR(    PROBE250US6,  NULL),
  MINI_SENSOR_PTR(    PROBE250US7,  NULL),
  MINI_SENSOR_PTR(    GPEtickdur0,  NULL),
  MINI_SENSOR_PTR(    GPEtickdur1,  NULL),
  MINI_SENSOR_PTR(     RTLtickdur,  NULL),

  // ------------------------------------------------------
  // System Sensors
  // ------------------------------------------------------
  MINI_SENSOR_PTR(         PWRSYS,  NULL),
  MINI_SENSOR_PTR(    PWR250USFAN,  NULL),
  MINI_SENSOR_PTR(     PWR250USIO,  NULL),
  MINI_SENSOR_PTR(  PWR250USSTORE,  NULL),
  MINI_SENSOR_PTR(         PWRGPU,  NULL),

  // ------------------------------------------------------
  // Chip Sensors
  // ------------------------------------------------------
  MINI_SENSOR_PTR(      TODclock0,  &G_dcom_slv_outbox_tx.todclock[0]),
  MINI_SENSOR_PTR(      TODclock1,  &G_dcom_slv_outbox_tx.todclock[1]),
  MINI_SENSOR_PTR(      TODclock2,  &G_dcom_slv_outbox_tx.todclock[2]),

  // ------------------------------------------------------
  // Processor Sensors
  // ------------------------------------------------------
  MINI_SENSOR_PTR(          FREQA,  &G_dcom_slv_outbox_tx.freqa),
  MINI_SENSOR_PTR(       IPS4MSP0,  &G_dcom_slv_outbox_tx.ips4msp0),
  MINI_SENSOR_PTR(     MEMSP2MSP0,  NULL),
  MINI_SENSOR_PTR(        PWRPROC,  &G_dcom_slv_outbox_tx.pwrproc),
  MINI_SENSOR_PTR(   PWR250USVDD0,  NULL),
  MINI_SENSOR_PTR(   PWRVCSVIOVDN,  NULL),
  MINI_SENSOR_PTR(   PWR250USMEM0,  &G_dcom_slv_outbox_tx.pwr250usmemp0),
  MINI_SENSOR_PTR(  SLEEPCNT4MSP0,  &G_dcom_slv_outbox_tx.sleepcnt4msp0),
  MINI_SENSOR_PTR(   WINKCNT4MSP0,  &G_dcom_slv_outbox_tx.winkcnt4msp0),
  MINI_SENSOR_PTR(      SP250USP0,  NULL),
  MINI_SENSOR_PTR(    TEMPPROCAVG,  &G_dcom_slv_outbox_tx.tempprocavg),
  MINI_SENSOR_PTR(   TEMPPROCTHRM,  &G_dcom_slv_outbox_tx.tempprocthermal),
  MINI_SENSOR_PTR(           UTIL,  &G_dcom_slv_outbox_tx.util),
  MINI_SENSOR_PTR(       TEMPNEST,  NULL),
  MINI_SENSOR_PTR(   VOLTVDDSENSE,  NULL),
  MINI_SENSOR_PTR(   VOLTVDNSENSE,  NULL),
  MINI_SENSOR_PTR(         PWRVDD,  NULL),
  MINI_SENSOR_PTR(         PWRVDN,  NULL),

  // ------------------------------------------------------
  // Quad Sensors (6 each)
  // ------------------------------------------------------
  QUAD_MINI_SENSOR_PTRS_NULL( TEMPQ ),

  // ------------------------------------------------------
  // Core Sensors (24 of each)
  // ------------------------------------------------------
  CORE_MINI_SENSOR_PTRS_NULL(  FREQ250USP0C ),
  CORE_MINI_SENSOR_PTRS_NULL(        FREQAC ),
  CORE_MINI_SENSOR_PTRS(          IPS4MSP0C, &G_dcom_slv_outbox_tx.ips4msp0cy    ),
  CORE_MINI_SENSOR_PTRS(       NOTBZE4MSP0C, &G_dcom_slv_outbox_tx.mcpifd4msp0cy ),
  CORE_MINI_SENSOR_PTRS(       NOTFIN4MSP0C, &G_dcom_slv_outbox_tx.mcpifi4msp0cy ),
  CORE_MINI_SENSOR_PTRS_NULL( TEMPPROCTHRMC ),
  CORE_MINI_SENSOR_PTRS(              UTILC, &G_dcom_slv_outbox_tx.utilcy        ),
  CORE_MINI_SENSOR_PTRS(         NUTIL3SP0C, &G_dcom_slv_outbox_tx.nutil3sp0cy   ),
  CORE_MINI_SENSOR_PTRS_NULL(    MSTL2MSP0C ),
  CORE_MINI_SENSOR_PTRS_NULL(     CMT2MSP0C ),
  CORE_MINI_SENSOR_PTRS_NULL(       PPICP0C ),
  CORE_MINI_SENSOR_PTRS(      PWRPX250USP0C, &G_dcom_slv_outbox_tx.pwrpx250usp0cy),
  CORE_MINI_SENSOR_PTRS_NULL(         TEMPC ),

  // ------------------------------------------------------
  // Memory Sensors
  // ------------------------------------------------------

  MEMCONTROL_MINI_SENSOR_PTRS(MRD2MSP0M, &G_dcom_slv_outbox_tx.mrd2msp0mx), //
  MEMCONTROL_MINI_SENSOR_PTRS(MWR2MSP0M, &G_dcom_slv_outbox_tx.mwr2msp0mx), //
  MEMCONTROL_MINI_SENSOR_PTRS_NULL(MIRC2MSP0M),
  MEMCONTROL_MINI_SENSOR_PTRS_NULL(MLP2P0M),

  PORTPAIR_MINI_SENSOR_PTRS_NULL(MAC2MSP0M),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(MPU2MSP0M),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(MIRB2MSP0M),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(MIRL2MSP0M),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(MIRM2MSP0M),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(MIRH2MSP0M),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(MTS2MSP0M),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(MEMSP2MSPM),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(M4RD2MSP0M),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(M4WR2MSP0M),

  MINI_SENSOR_PTR( TEMP2MSCENT,     &G_dcom_slv_outbox_tx.temp2mscent),
  MINI_SENSOR_PTR( TEMPDIMMTHRM,    &G_dcom_slv_outbox_tx.tempdimmthrm),
  MINI_SENSOR_PTR( MEMSP2MS,        NULL),

  // ------------------------------------------------------
  // Regulator Sensors
  // ------------------------------------------------------
  MINI_SENSOR_PTR( UVOLT250USP0V0,  NULL),
  MINI_SENSOR_PTR( UVOLT250USP0V1,  NULL),
  MINI_SENSOR_PTR( VOLTVDD,         NULL),
  MINI_SENSOR_PTR( VOLTVDN,         NULL),
  MINI_SENSOR_PTR( CURVDD,          NULL),
  MINI_SENSOR_PTR( CURVDN,          NULL),
  MINI_SENSOR_PTR( VRFAN,           NULL),

  // ------------------------------------------------------
  // Partition Sensors
  // ------------------------------------------------------
  MINI_SENSOR_PTR( UTIL2MSSLCG000,  NULL),
  MINI_SENSOR_PTR( UTIL2MSSLCG001,  NULL),
  MINI_SENSOR_PTR( UTIL2MSSLCG002,  NULL),
  MINI_SENSOR_PTR( UTIL2MSSLCG003,  NULL),
  MINI_SENSOR_PTR( UTIL2MSSLCG004,  NULL),
  MINI_SENSOR_PTR( UTIL2MSSLCG005,  NULL),
  MINI_SENSOR_PTR( UTIL2MSSLCG006,  NULL),
  MINI_SENSOR_PTR( UTIL2MSSLCG007,  NULL),
  MINI_SENSOR_PTR( UTIL2MSSLCG008,  NULL),
  MINI_SENSOR_PTR( UTIL2MSSLCG009,  NULL),
  MINI_SENSOR_PTR( UTIL2MSSLCG010,  NULL),
  MINI_SENSOR_PTR( UTIL2MSSLCG011,  NULL),
};
STATIC_ASSERT(   (NUMBER_OF_SENSORS_IN_LIST != (sizeof(G_amec_mini_sensor_list)/sizeof(uint16_t *)))   );
STATIC_ASSERT(   (MAX_AMEC_SENSORS < (sizeof(G_amec_mini_sensor_list)/sizeof(uint16_t *)))   );
