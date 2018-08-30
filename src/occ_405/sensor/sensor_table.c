/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor_table.c $                           */
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

// These will paste a number onto a sensor name base to create the full enum
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
// base sensor name and ptr to [0] entry of array of 16 memcontroller sensors
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
  SENSOR_PTR( PWRFAN,               &g_amec_sys.fan.pwr250usfan),
  SENSOR_PTR( PWRIO,                &g_amec_sys.io.pwr250usio),
  SENSOR_PTR( PWRSTORE,             &g_amec_sys.storage.pwr250usstore),
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
  SENSOR_PTR( VRHOTMEMPRCCNT,       &g_amec_sys.sys.vrhot_mem_proc),

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
  SENSOR_PTR( IPS,                  &g_amec_sys.proc[0].ips4ms),
  SENSOR_PTR( PWRPROC,              &g_amec_sys.proc[0].pwrproc),
  SENSOR_PTR( PWRMEM,               &g_amec_sys.proc[0].pwr250usmem),
  SENSOR_PTR( TEMPPROCAVG,          &g_amec_sys.proc[0].tempprocavg),
  SENSOR_PTR( TEMPPROCTHRM,         &g_amec_sys.proc[0].tempprocthermal),
  SENSOR_PTR( UTIL,                 &g_amec_sys.proc[0].util),
  SENSOR_PTR( TEMPNEST,             &g_amec_sys.proc[0].tempnest),
  SENSOR_PTR( VOLTVDDSENSE,         &g_amec_sys.fw.voltvddsense),
  SENSOR_PTR( VOLTVDNSENSE,         &g_amec_sys.fw.voltvdnsense),
  SENSOR_PTR( PWRVDD,               &g_amec_sys.proc[0].pwrvdd),
  SENSOR_PTR( PWRVDN,               &g_amec_sys.proc[0].pwrvdn),
  SENSOR_PTR( PROCPWRTHROT,         &g_amec_sys.proc[0].procpwrthrot),
  SENSOR_PTR( PROCOTTHROT,          &g_amec_sys.proc[0].procotthrot),

  // ------------------------------------------------------
  // Quad Sensors (6 each)
  // ------------------------------------------------------
  QUAD_SENSOR_PTRS( TEMPQ,           &g_amec_sys.proc[0].quad, tempq),
  QUAD_SENSOR_PTRS( VOLTDROOPCNTQ,   &g_amec_sys.proc[0].quad, voltdroopcntq),

  // ------------------------------------------------------
  // Regulator Sensors
  // ------------------------------------------------------
  SENSOR_PTR( VOLTVDD,              &g_amec_sys.proc[0].vrm[0].volt250us),
  SENSOR_PTR( VOLTVDN,              &g_amec_sys.proc[0].vrm[1].volt250us),
  SENSOR_PTR( CURVDD,               &g_amec_sys.proc[0].curvdd),
  SENSOR_PTR( CURVDN,               &g_amec_sys.proc[0].curvdn),
  SENSOR_PTR( TEMPVDD,              &g_amec_sys.sys.tempvdd),


  // ------------------------------------------------------
  // Core Sensors (24 of each)
  // ------------------------------------------------------
  CORE_SENSOR_PTRS( FREQREQC ,      &g_amec_sys.proc[0].core, freq250us),
  CORE_SENSOR_PTRS( FREQAC ,        &g_amec_sys.proc[0].core, freqa),
  CORE_SENSOR_PTRS( IPSC ,          &g_amec_sys.proc[0].core, ips4ms),
  CORE_SENSOR_PTRS( NOTBZEC ,       &g_amec_sys.proc[0].core, mcpifd4ms),
  CORE_SENSOR_PTRS( NOTFINC ,       &g_amec_sys.proc[0].core, mcpifi4ms),
  CORE_SENSOR_PTRS( TEMPPROCTHRMC , &g_amec_sys.proc[0].core, tempprocthermal),
  CORE_SENSOR_PTRS( UTILC ,         &g_amec_sys.proc[0].core, util),
  CORE_SENSOR_PTRS( NUTILC ,        &g_amec_sys.proc[0].core, nutil3s),
  CORE_SENSOR_PTRS( TEMPC,          &g_amec_sys.proc[0].core, tempc),
  CORE_SENSOR_PTRS( STOPDEEPREQC,   &g_amec_sys.proc[0].core, stopdeepreqc),
  CORE_SENSOR_PTRS( STOPDEEPACTC,   &g_amec_sys.proc[0].core, stopdeepactc),
  CORE_SENSOR_PTRS( VOLTDROOPCNTC,  &g_amec_sys.proc[0].core, voltdroopcntc),

  // ------------------------------------------------------
  // Memory Sensors
  // ------------------------------------------------------
  MEMCONTROL_SENSOR_PTRS(MRDM,          &g_amec_sys.proc[0].memctl, mrd),
  MEMCONTROL_SENSOR_PTRS(MWRM,          &g_amec_sys.proc[0].memctl, mwr),
  MEMCONTROL_SENSOR_PTRS(MEMSPM,        &g_amec_sys.proc[0].memctl, memsp),
  MEMCONTROL_SENSOR_PTRS(MEMSPSTATM,    &g_amec_sys.proc[0].memctl, memspstat),
  MEMCONTROL_SENSOR_PTRS(MIRCM,         &g_amec_sys.proc[0].memctl, centaur.mirc2ms),
  MEMCONTROL_SENSOR_PTRS(MLP2M,         &g_amec_sys.proc[0].memctl, centaur.mlp2ms),
  DIMM_SENSOR_PTRS(TEMPDIMM,            &g_amec_sys.proc[0],        tempdimm),
  MEMCONTROL_SENSOR_PTRS(TEMPDIMMAXM,   &g_amec_sys.proc[0].memctl, centaur.tempdimmax),
  MEMCONTROL_SENSOR_PTRS(LOCDIMMAXM,    &g_amec_sys.proc[0].memctl, centaur.locdimmax),
  SENSOR_PTR(MEMPWRTHROT,               &g_amec_sys.proc[0].mempwrthrot),
  SENSOR_PTR(MEMOTTHROT,                &g_amec_sys.proc[0].memotthrot),

  PORTPAIR_SENSOR_PTRS(MACM,        &g_amec_sys.proc[0].memctl, centaur.portpair, mac2ms),
  PORTPAIR_SENSOR_PTRS(MPUM,        &g_amec_sys.proc[0].memctl, centaur.portpair, mpu2ms),
  PORTPAIR_SENSOR_PTRS(MIRBM,       &g_amec_sys.proc[0].memctl, centaur.portpair, mirb2ms),
  PORTPAIR_SENSOR_PTRS(MIRLM,       &g_amec_sys.proc[0].memctl, centaur.portpair, mirl2ms),
  PORTPAIR_SENSOR_PTRS(MIRMM,       &g_amec_sys.proc[0].memctl, centaur.portpair, mirm2ms),
  PORTPAIR_SENSOR_PTRS(MIRHM,       &g_amec_sys.proc[0].memctl, centaur.portpair, mirh2ms),
  PORTPAIR_SENSOR_PTRS(MTSM,        &g_amec_sys.proc[0].memctl, centaur.portpair, mts2ms),
  PORTPAIR_SENSOR_PTRS(M4RDM,       &g_amec_sys.proc[0].memctl, centaur.portpair, m4rd2ms),
  PORTPAIR_SENSOR_PTRS(M4WRM,       &g_amec_sys.proc[0].memctl, centaur.portpair, m4wr2ms),


  SENSOR_PTR(TEMPCENT,              &g_amec_sys.proc[0].temp2mscent),
  SENSOR_PTR(TEMPDIMMTHRM,          &g_amec_sys.proc[0].tempdimmthrm),

  // ------------------------------------------------------
  // GPU Sensors
  // ------------------------------------------------------
  SENSOR_PTR(TEMPGPU0,              &g_amec_sys.gpu[0].tempgpu),
  SENSOR_PTR(TEMPGPU1,              &g_amec_sys.gpu[1].tempgpu),
  SENSOR_PTR(TEMPGPU2,              &g_amec_sys.gpu[2].tempgpu),
  SENSOR_PTR(TEMPGPU0MEM,           &g_amec_sys.gpu[0].tempgpumem),
  SENSOR_PTR(TEMPGPU1MEM,           &g_amec_sys.gpu[1].tempgpumem),
  SENSOR_PTR(TEMPGPU2MEM,           &g_amec_sys.gpu[2].tempgpumem),

  // ------------------------------------------------------
  // Partition Sensors -- no PLPM support only 1 core group sensor
  // additional sensors must be added if AMEC_PART_MAX_PART is ever changed to be more than 1
  // ------------------------------------------------------
  SENSOR_PTR( UTILSLCG000,          &g_amec_sys.part_config.part_list[0].util2msslack),

  // ------------------------------------------------------
  // WOF Sensors
  // ------------------------------------------------------
  SENSOR_PTR( CEFFVDDRATIO,         &g_amec_sys.wof_sensors.ceff_ratio_vdd_sensor),
  SENSOR_PTR( CEFFVDNRATIO,         &g_amec_sys.wof_sensors.ceff_ratio_vdn_sensor),
  SENSOR_PTR( VRATIO,               &g_amec_sys.wof_sensors.v_ratio_sensor),

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
  MINI_SENSOR_PTR(         PWRFAN,  NULL),
  MINI_SENSOR_PTR(         PWRIO,   NULL),
  MINI_SENSOR_PTR(       PWRSTORE,  NULL),
  MINI_SENSOR_PTR(         PWRGPU,  NULL),
  MINI_SENSOR_PTR(     PWRAPSSCH0,  NULL),
  MINI_SENSOR_PTR(     PWRAPSSCH1,  NULL),
  MINI_SENSOR_PTR(     PWRAPSSCH2,  NULL),
  MINI_SENSOR_PTR(     PWRAPSSCH3,  NULL),
  MINI_SENSOR_PTR(     PWRAPSSCH4,  NULL),
  MINI_SENSOR_PTR(     PWRAPSSCH5,  NULL),
  MINI_SENSOR_PTR(     PWRAPSSCH6,  NULL),
  MINI_SENSOR_PTR(     PWRAPSSCH7,  NULL),
  MINI_SENSOR_PTR(     PWRAPSSCH8,  NULL),
  MINI_SENSOR_PTR(     PWRAPSSCH9,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH10,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH11,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH12,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH13,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH14,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH15,  NULL),
  MINI_SENSOR_PTR(     CUR12VSTBY,  NULL),
  MINI_SENSOR_PTR( VRHOTMEMPRCCNT,  NULL),

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
  MINI_SENSOR_PTR(            IPS,  &G_dcom_slv_outbox_tx.ips4msp0),
  MINI_SENSOR_PTR(        PWRPROC,  &G_dcom_slv_outbox_tx.pwrproc),
  MINI_SENSOR_PTR(         PWRMEM,  &G_dcom_slv_outbox_tx.pwr250usmemp0),
  MINI_SENSOR_PTR(    TEMPPROCAVG,  &G_dcom_slv_outbox_tx.tempprocavg),
  MINI_SENSOR_PTR(   TEMPPROCTHRM,  &G_dcom_slv_outbox_tx.tempprocthermal),
  MINI_SENSOR_PTR(           UTIL,  &G_dcom_slv_outbox_tx.util),
  MINI_SENSOR_PTR(       TEMPNEST,  NULL),
  MINI_SENSOR_PTR(   VOLTVDDSENSE,  NULL),
  MINI_SENSOR_PTR(   VOLTVDNSENSE,  NULL),
  MINI_SENSOR_PTR(         PWRVDD,  NULL),
  MINI_SENSOR_PTR(         PWRVDN,  NULL),
  MINI_SENSOR_PTR(   PROCPWRTHROT,  NULL),
  MINI_SENSOR_PTR(    PROCOTTHROT,  NULL),

  // ------------------------------------------------------
  // Quad Sensors (6 each)
  // ------------------------------------------------------
  QUAD_MINI_SENSOR_PTRS_NULL( TEMPQ ),
  QUAD_MINI_SENSOR_PTRS_NULL( VOLTDROOPCNTQ),

  // ------------------------------------------------------
  // Regulator Sensors
  // ------------------------------------------------------
  MINI_SENSOR_PTR( VOLTVDD,         NULL),
  MINI_SENSOR_PTR( VOLTVDN,         NULL),
  MINI_SENSOR_PTR( CURVDD,          NULL),
  MINI_SENSOR_PTR( CURVDN,          NULL),
  MINI_SENSOR_PTR( TEMPVDD,         &G_dcom_slv_outbox_tx.tempvdd),

  // ------------------------------------------------------
  // Core Sensors (24 of each)
  // ------------------------------------------------------
  CORE_MINI_SENSOR_PTRS_NULL(      FREQREQC ),
  CORE_MINI_SENSOR_PTRS_NULL(        FREQAC ),
  CORE_MINI_SENSOR_PTRS(               IPSC, &G_dcom_slv_outbox_tx.ips4msp0cy    ),
  CORE_MINI_SENSOR_PTRS(            NOTBZEC, &G_dcom_slv_outbox_tx.mcpifd4msp0cy ),
  CORE_MINI_SENSOR_PTRS(            NOTFINC, &G_dcom_slv_outbox_tx.mcpifi4msp0cy ),
  CORE_MINI_SENSOR_PTRS_NULL( TEMPPROCTHRMC ),
  CORE_MINI_SENSOR_PTRS(              UTILC, &G_dcom_slv_outbox_tx.utilcy        ),
  CORE_MINI_SENSOR_PTRS(             NUTILC, &G_dcom_slv_outbox_tx.nutil3sp0cy   ),
  CORE_MINI_SENSOR_PTRS_NULL(         TEMPC ),
  CORE_MINI_SENSOR_PTRS_NULL(  STOPDEEPREQC ),
  CORE_MINI_SENSOR_PTRS_NULL(  STOPDEEPACTC ),
  CORE_MINI_SENSOR_PTRS_NULL(  VOLTDROOPCNTC),

  // ------------------------------------------------------
  // Memory Sensors
  // ------------------------------------------------------

  MEMCONTROL_MINI_SENSOR_PTRS(MRDM, &G_dcom_slv_outbox_tx.mrd),
  MEMCONTROL_MINI_SENSOR_PTRS(MWRM, &G_dcom_slv_outbox_tx.mwr),
  MEMCONTROL_MINI_SENSOR_PTRS_NULL(MEMSPM),
  MEMCONTROL_MINI_SENSOR_PTRS_NULL(MEMSPSTATM),
  MEMCONTROL_MINI_SENSOR_PTRS_NULL(MIRCM),
  MEMCONTROL_MINI_SENSOR_PTRS_NULL(MLP2M),

  MINI_SENSOR_PTR( TEMPDIMM00,     NULL),
  MINI_SENSOR_PTR( TEMPDIMM01,     NULL),
  MINI_SENSOR_PTR( TEMPDIMM02,     NULL),
  MINI_SENSOR_PTR( TEMPDIMM03,     NULL),
  MINI_SENSOR_PTR( TEMPDIMM04,     NULL),
  MINI_SENSOR_PTR( TEMPDIMM05,     NULL),
  MINI_SENSOR_PTR( TEMPDIMM06,     NULL),
  MINI_SENSOR_PTR( TEMPDIMM07,     NULL),
  MINI_SENSOR_PTR( TEMPDIMM08,     NULL),
  MINI_SENSOR_PTR( TEMPDIMM09,     NULL),
  MINI_SENSOR_PTR( TEMPDIMM10,     NULL),
  MINI_SENSOR_PTR( TEMPDIMM11,     NULL),
  MINI_SENSOR_PTR( TEMPDIMM12,     NULL),
  MINI_SENSOR_PTR( TEMPDIMM13,     NULL),
  MINI_SENSOR_PTR( TEMPDIMM14,     NULL),
  MINI_SENSOR_PTR( TEMPDIMM15,     NULL),

  MEMCONTROL_MINI_SENSOR_PTRS_NULL(TEMPDIMMAXM),
  MEMCONTROL_MINI_SENSOR_PTRS_NULL(LOCDIMMAXM),

  MINI_SENSOR_PTR( MEMPWRTHROT,  NULL),
  MINI_SENSOR_PTR( MEMOTTHROT,   NULL),

  PORTPAIR_MINI_SENSOR_PTRS_NULL(MACM),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(MPUM),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(MIRBM),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(MIRLM),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(MIRMM),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(MIRHM),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(MTSM),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(M4RDM),
  PORTPAIR_MINI_SENSOR_PTRS_NULL(M4WRM),

  MINI_SENSOR_PTR( TEMPCENT,        &G_dcom_slv_outbox_tx.temp2mscent),
  MINI_SENSOR_PTR( TEMPDIMMTHRM,    &G_dcom_slv_outbox_tx.tempdimmthrm),

  // ------------------------------------------------------
  // GPU Sensors
  // ------------------------------------------------------
  MINI_SENSOR_PTR( TEMPGPU0,     NULL),
  MINI_SENSOR_PTR( TEMPGPU1,     NULL),
  MINI_SENSOR_PTR( TEMPGPU2,     NULL),
  MINI_SENSOR_PTR( TEMPGPU0MEM,  NULL),
  MINI_SENSOR_PTR( TEMPGPU1MEM,  NULL),
  MINI_SENSOR_PTR( TEMPGPU2MEM,  NULL),

  // ------------------------------------------------------
  // Partition Sensors
  // ------------------------------------------------------
  MINI_SENSOR_PTR( UTILSLCG000,  NULL),

  // ------------------------------------------------------
  // WOF Sensors
  // ------------------------------------------------------
  SENSOR_PTR( CEFFVDDRATIO,   NULL),
  SENSOR_PTR( CEFFVDNRATIO,   NULL),
  SENSOR_PTR( VRATIO,         NULL),
};
STATIC_ASSERT(   (NUMBER_OF_SENSORS_IN_LIST != (sizeof(G_amec_mini_sensor_list)/sizeof(uint16_t *)))   );
STATIC_ASSERT(   (MAX_AMEC_SENSORS < (sizeof(G_amec_mini_sensor_list)/sizeof(uint16_t *)))   );
