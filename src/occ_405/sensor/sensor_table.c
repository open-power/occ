/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor_table.c $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2021                        */
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

// Will define a set of "core sensor pointers" by passing in base sensor name
// and ptr to [0] entry of array of core sensors
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
  [SENSOR_W_NUM(sensor,23)] = ptrbase[23].ptrmember, \
  [SENSOR_W_NUM(sensor,24)] = ptrbase[24].ptrmember, \
  [SENSOR_W_NUM(sensor,25)] = ptrbase[25].ptrmember, \
  [SENSOR_W_NUM(sensor,26)] = ptrbase[26].ptrmember, \
  [SENSOR_W_NUM(sensor,27)] = ptrbase[27].ptrmember, \
  [SENSOR_W_NUM(sensor,28)] = ptrbase[28].ptrmember, \
  [SENSOR_W_NUM(sensor,29)] = ptrbase[29].ptrmember, \
  [SENSOR_W_NUM(sensor,30)] = ptrbase[30].ptrmember, \
  [SENSOR_W_NUM(sensor,31)] = ptrbase[31].ptrmember

// Will define a set of "quad sensor pointers" by passing in base sensor name
// and ptr to [0] entry of array of quad sensors
#define QUAD_SENSOR_PTRS(sensor,ptrbase,ptrmember) \
  [SENSOR_W_NUM(sensor, 0)] = ptrbase[ 0].ptrmember, \
  [SENSOR_W_NUM(sensor, 1)] = ptrbase[ 1].ptrmember, \
  [SENSOR_W_NUM(sensor, 2)] = ptrbase[ 2].ptrmember, \
  [SENSOR_W_NUM(sensor, 3)] = ptrbase[ 3].ptrmember, \
  [SENSOR_W_NUM(sensor, 4)] = ptrbase[ 4].ptrmember, \
  [SENSOR_W_NUM(sensor, 5)] = ptrbase[ 5].ptrmember, \
  [SENSOR_W_NUM(sensor, 6)] = ptrbase[ 6].ptrmember, \
  [SENSOR_W_NUM(sensor, 7)] = ptrbase[ 7].ptrmember


// Will define a set of "memory controller sensor pointers" by passing in
// base sensor nameand ptr to [0] entry of array of memcontroller sensors
#define MEMCONTROL_SENSOR_PTRS(sensor,ptrbase,ptrmember) \
  [SENSOR_W_NUM(sensor, 0)] = ptrbase[0].ptrmember, \
  [SENSOR_W_NUM(sensor, 1)] = ptrbase[1].ptrmember, \
  [SENSOR_W_NUM(sensor, 2)] = ptrbase[2].ptrmember, \
  [SENSOR_W_NUM(sensor, 3)] = ptrbase[3].ptrmember, \
  [SENSOR_W_NUM(sensor, 4)] = ptrbase[4].ptrmember, \
  [SENSOR_W_NUM(sensor, 5)] = ptrbase[5].ptrmember, \
  [SENSOR_W_NUM(sensor, 6)] = ptrbase[6].ptrmember, \
  [SENSOR_W_NUM(sensor, 7)] = ptrbase[7].ptrmember, \
  [SENSOR_W_NUM(sensor, 8)] = ptrbase[8].ptrmember, \
  [SENSOR_W_NUM(sensor, 9)] = ptrbase[9].ptrmember, \
  [SENSOR_W_NUM(sensor, 10)] = ptrbase[10].ptrmember, \
  [SENSOR_W_NUM(sensor, 11)] = ptrbase[11].ptrmember, \
  [SENSOR_W_NUM(sensor, 12)] = ptrbase[12].ptrmember, \
  [SENSOR_W_NUM(sensor, 13)] = ptrbase[13].ptrmember, \
  [SENSOR_W_NUM(sensor, 14)] = ptrbase[14].ptrmember, \
  [SENSOR_W_NUM(sensor, 15)] = ptrbase[15].ptrmember

// Will define a set of "processor IO sensor pointer" by passing in
// base sensor namead ptr to [0] entry of array of procio dts
#define PROCIO_SENSOR_PTRS(sensor,ptrbase,ptrmember) \
  [SENSOR_W_NUM(sensor, 00)] = ptrbase[0].ptrmember, \
  [SENSOR_W_NUM(sensor, 01)] = ptrbase[1].ptrmember, \
  [SENSOR_W_NUM(sensor, 10)] = ptrbase[2].ptrmember, \
  [SENSOR_W_NUM(sensor, 11)] = ptrbase[3].ptrmember


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
  [SENSOR_W_NUM(sensor,23)] = ptr[23], \
  [SENSOR_W_NUM(sensor,24)] = ptr[24], \
  [SENSOR_W_NUM(sensor,25)] = ptr[25], \
  [SENSOR_W_NUM(sensor,26)] = ptr[26], \
  [SENSOR_W_NUM(sensor,27)] = ptr[27], \
  [SENSOR_W_NUM(sensor,28)] = ptr[28], \
  [SENSOR_W_NUM(sensor,29)] = ptr[29], \
  [SENSOR_W_NUM(sensor,30)] = ptr[30], \
  [SENSOR_W_NUM(sensor,31)] = ptr[31]

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
  [SENSOR_W_NUM(sensor,23)] = NULL, \
  [SENSOR_W_NUM(sensor,24)] = NULL, \
  [SENSOR_W_NUM(sensor,25)] = NULL, \
  [SENSOR_W_NUM(sensor,26)] = NULL, \
  [SENSOR_W_NUM(sensor,27)] = NULL, \
  [SENSOR_W_NUM(sensor,28)] = NULL, \
  [SENSOR_W_NUM(sensor,29)] = NULL, \
  [SENSOR_W_NUM(sensor,30)] = NULL, \
  [SENSOR_W_NUM(sensor,31)] = NULL

// Will define a set of "quad mini-sensor pointers" by passing in base
// sensor name and ptr to [0] entry of array of quad sensors
#define QUAD_MINI_SENSOR_PTRS(sensor,ptr) \
  [SENSOR_W_NUM(sensor, 0)] = ptr[ 0], \
  [SENSOR_W_NUM(sensor, 1)] = ptr[ 1], \
  [SENSOR_W_NUM(sensor, 2)] = ptr[ 2], \
  [SENSOR_W_NUM(sensor, 3)] = ptr[ 3], \
  [SENSOR_W_NUM(sensor, 4)] = ptr[ 4], \
  [SENSOR_W_NUM(sensor, 5)] = ptr[ 5], \
  [SENSOR_W_NUM(sensor, 6)] = ptr[ 6], \
  [SENSOR_W_NUM(sensor, 7)] = ptr[ 7]

// every sensor must have a mini-sensor.
#define QUAD_MINI_SENSOR_PTRS_NULL(sensor) \
  [SENSOR_W_NUM(sensor, 0)] = NULL, \
  [SENSOR_W_NUM(sensor, 1)] = NULL, \
  [SENSOR_W_NUM(sensor, 2)] = NULL, \
  [SENSOR_W_NUM(sensor, 3)] = NULL, \
  [SENSOR_W_NUM(sensor, 4)] = NULL, \
  [SENSOR_W_NUM(sensor, 5)] = NULL, \
  [SENSOR_W_NUM(sensor, 6)] = NULL, \
  [SENSOR_W_NUM(sensor, 7)] = NULL

// Will define a set of "memory controller mini sensor ptrs" by passing in
// base sensor nameand ptr to [0] entry of array of memcontroller sensors
#define MEMCONTROL_MINI_SENSOR_PTRS(sensor,ptr) \
  [SENSOR_W_NUM(sensor, 0)]  = ptr[ 0], \
  [SENSOR_W_NUM(sensor, 1)]  = ptr[ 1], \
  [SENSOR_W_NUM(sensor, 2)]  = ptr[ 2], \
  [SENSOR_W_NUM(sensor, 3)]  = ptr[ 3], \
  [SENSOR_W_NUM(sensor, 4)]  = ptr[ 4], \
  [SENSOR_W_NUM(sensor, 5)]  = ptr[ 5], \
  [SENSOR_W_NUM(sensor, 6)]  = ptr[ 6], \
  [SENSOR_W_NUM(sensor, 7)]  = ptr[ 7], \
  [SENSOR_W_NUM(sensor, 8)]  = ptr[ 8], \
  [SENSOR_W_NUM(sensor, 9)]  = ptr[ 9], \
  [SENSOR_W_NUM(sensor, 10)] = ptr[10], \
  [SENSOR_W_NUM(sensor, 11)] = ptr[11], \
  [SENSOR_W_NUM(sensor, 12)] = ptr[12], \
  [SENSOR_W_NUM(sensor, 13)] = ptr[13], \
  [SENSOR_W_NUM(sensor, 14)] = ptr[14], \
  [SENSOR_W_NUM(sensor, 15)] = ptr[15]

// Will define a set of "memc mini-sensor pointers" as NULL, since not
// every sensor must have a mini-sensor.
#define MEMCONTROL_MINI_SENSOR_PTRS_NULL(sensor) \
  [SENSOR_W_NUM(sensor, 0)]  = NULL, \
  [SENSOR_W_NUM(sensor, 1)]  = NULL, \
  [SENSOR_W_NUM(sensor, 2)]  = NULL, \
  [SENSOR_W_NUM(sensor, 3)]  = NULL, \
  [SENSOR_W_NUM(sensor, 4)]  = NULL, \
  [SENSOR_W_NUM(sensor, 5)]  = NULL, \
  [SENSOR_W_NUM(sensor, 6)]  = NULL, \
  [SENSOR_W_NUM(sensor, 7)]  = NULL, \
  [SENSOR_W_NUM(sensor, 8)]  = NULL, \
  [SENSOR_W_NUM(sensor, 9)]  = NULL, \
  [SENSOR_W_NUM(sensor, 10)] = NULL, \
  [SENSOR_W_NUM(sensor, 11)] = NULL, \
  [SENSOR_W_NUM(sensor, 12)] = NULL, \
  [SENSOR_W_NUM(sensor, 13)] = NULL, \
  [SENSOR_W_NUM(sensor, 14)] = NULL, \
  [SENSOR_W_NUM(sensor, 15)] = NULL

// Will define a set of "processor IO mini sensor ptrs" by passing in
// base sensor name and ptr to [0] entry of array of procio DTS
#define PROCIO_MINI_SENSOR_PTRS(sensor,ptr) \
  [SENSOR_W_NUM(sensor, 00)]  = ptr[ 0], \
  [SENSOR_W_NUM(sensor, 01)]  = ptr[ 1], \
  [SENSOR_W_NUM(sensor, 10)]  = ptr[ 2], \
  [SENSOR_W_NUM(sensor, 11)]  = ptr[ 3]

// Will define a set of "procio mini-sensor pointers" as NULL, since not
// every sensor must have a mini-sensor.
#define PROCIO_MINI_SENSOR_PTRS_NULL(sensor) \
  [SENSOR_W_NUM(sensor, 00)]  = NULL, \
  [SENSOR_W_NUM(sensor, 01)]  = NULL, \
  [SENSOR_W_NUM(sensor, 10)]  = NULL, \
  [SENSOR_W_NUM(sensor, 11)]  = NULL


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
  SENSOR_PTR(    VRT_BCEdur,        &g_amec_sys.fw.times[0]),
  SENSOR_PTR(    VRT_IPCdur,        &g_amec_sys.fw.times[1]),
  SENSOR_PTR(   WOFC_IPCdur,        &g_amec_sys.fw.times[2]),
  SENSOR_PTR(   Sout_BCEdur,        &g_amec_sys.fw.times[3]),
  SENSOR_PTR(   CORE_IPCdur,        &g_amec_sys.fw.times[4]),
  SENSOR_PTR(     timer3dur,        &g_amec_sys.fw.times[5]),
  SENSOR_PTR(     timer4dur,        &g_amec_sys.fw.times[6]),
  SENSOR_PTR(     timer5dur,        &g_amec_sys.fw.times[7]),
  SENSOR_PTR(   GPEtickdur0,        &g_amec_sys.fw.gpetickdur[0]),
  SENSOR_PTR(   GPEtickdur1,        &g_amec_sys.fw.gpetickdur[1]),
  SENSOR_PTR(    RTLtickdur,        &g_amec_sys.fw.prcdupdatedur),

  // ------------------------------------------------------
  // System Sensors
  // ------------------------------------------------------
  SENSOR_PTR( PWRSYS,               &g_amec_sys.sys.pwrsys),
  SENSOR_PTR( PWRGPU,               &g_amec_sys.sys.pwr250usgpu),
  SENSOR_PTR( PWRAPSSCH00,          &g_amec_sys.sys.pwrapssch[0]),
  SENSOR_PTR( PWRAPSSCH01,          &g_amec_sys.sys.pwrapssch[1]),
  SENSOR_PTR( PWRAPSSCH02,          &g_amec_sys.sys.pwrapssch[2]),
  SENSOR_PTR( PWRAPSSCH03,          &g_amec_sys.sys.pwrapssch[3]),
  SENSOR_PTR( PWRAPSSCH04,          &g_amec_sys.sys.pwrapssch[4]),
  SENSOR_PTR( PWRAPSSCH05,          &g_amec_sys.sys.pwrapssch[5]),
  SENSOR_PTR( PWRAPSSCH06,          &g_amec_sys.sys.pwrapssch[6]),
  SENSOR_PTR( PWRAPSSCH07,          &g_amec_sys.sys.pwrapssch[7]),
  SENSOR_PTR( PWRAPSSCH08,          &g_amec_sys.sys.pwrapssch[8]),
  SENSOR_PTR( PWRAPSSCH09,          &g_amec_sys.sys.pwrapssch[9]),
  SENSOR_PTR( PWRAPSSCH10,          &g_amec_sys.sys.pwrapssch[10]),
  SENSOR_PTR( PWRAPSSCH11,          &g_amec_sys.sys.pwrapssch[11]),
  SENSOR_PTR( PWRAPSSCH12,          &g_amec_sys.sys.pwrapssch[12]),
  SENSOR_PTR( PWRAPSSCH13,          &g_amec_sys.sys.pwrapssch[13]),
  SENSOR_PTR( PWRAPSSCH14,          &g_amec_sys.sys.pwrapssch[14]),
  SENSOR_PTR( PWRAPSSCH15,          &g_amec_sys.sys.pwrapssch[15]),
  SENSOR_PTR( CUR12VSTBY,           &g_amec_sys.sys.cur12Vstby),
  SENSOR_PTR( VRHOTMEMPRCCNT,       &g_amec_sys.sys.vrhot_mem_proc),
  SENSOR_PTR( TEMPAMBIENT,          &g_amec_sys.sys.ambient),

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
  SENSOR_PTR( TEMPPROCIOTHRM,       &g_amec_sys.proc[0].tempprociothermal),
  SENSOR_PTR( UTIL,                 &g_amec_sys.proc[0].util),
  SENSOR_PTR( TEMPRTAVG,            &g_amec_sys.proc[0].temprtavg),
  SENSOR_PTR( TEMPNEST0,            &g_amec_sys.proc[0].tempnest0),
  SENSOR_PTR( TEMPNEST1,            &g_amec_sys.proc[0].tempnest1),
  SENSOR_PTR( VOLTVDDSENSE,         &g_amec_sys.fw.voltvddsense),
  SENSOR_PTR( VOLTVDNSENSE,         &g_amec_sys.fw.voltvdnsense),
  SENSOR_PTR( VOLTVCSSENSE,         &g_amec_sys.fw.voltvcssense),
  SENSOR_PTR( VOLTVIOSENSE,         &g_amec_sys.fw.voltviosense),
  SENSOR_PTR( PWRVDD,               &g_amec_sys.proc[0].pwrvdd),
  SENSOR_PTR( PWRVDN,               &g_amec_sys.proc[0].pwrvdn),
  SENSOR_PTR( PWRVCS,               &g_amec_sys.proc[0].pwrvcs),
  SENSOR_PTR( PWRVIO,               &g_amec_sys.proc[0].pwrvio),
  SENSOR_PTR( PROCPWRTHROT,         &g_amec_sys.proc[0].procpwrthrot),
  SENSOR_PTR( PROCOTTHROT,          &g_amec_sys.proc[0].procotthrot),
  SENSOR_PTR( DDSAVG,               &g_amec_sys.proc[0].ddsavg),
  SENSOR_PTR( DDSMIN,               &g_amec_sys.proc[0].ddsmin),

  //-------------------------------------------------------
  // PROCIO (PAU) sensors
  //-------------------------------------------------------
  PROCIO_SENSOR_PTRS(TEMPPROCIO,    &g_amec_sys.proc[0].io , tempprocio),

  // ------------------------------------------------------
  // Quad Sensors (8 each)
  // ------------------------------------------------------
  QUAD_SENSOR_PTRS( TEMPQ,           &g_amec_sys.proc[0].quad, tempq),

  // ------------------------------------------------------
  // Regulator Sensors
  // ------------------------------------------------------
  SENSOR_PTR( VOLTVDD,              &g_amec_sys.proc[0].voltvdd),
  SENSOR_PTR( VOLTVDN,              &g_amec_sys.proc[0].voltvdn),
  SENSOR_PTR( VOLTVCS,              &g_amec_sys.proc[0].voltvcs),
  SENSOR_PTR( VOLTVIO,              &g_amec_sys.proc[0].voltvio),
  SENSOR_PTR( CURVDD,               &g_amec_sys.proc[0].curvdd),
  SENSOR_PTR( CURVDN,               &g_amec_sys.proc[0].curvdn),
  SENSOR_PTR( CURVCS,               &g_amec_sys.proc[0].curvcs),
  SENSOR_PTR( PSTATE,               &g_amec_sys.proc[0].pstate),
  SENSOR_PTR( TEMPVDD,              &g_amec_sys.sys.tempvdd),


  // ------------------------------------------------------
  // Core Sensors (32 of each)
  // ------------------------------------------------------
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
  MEMCONTROL_SENSOR_PTRS(TEMPMEMBUF,    &g_amec_sys.proc[0].memctl, membuf.tempmembuf),

  SENSOR_PTR(MEMOTTHROT,                &g_amec_sys.proc[0].memotthrot),

  SENSOR_PTR(TEMPMEMBUFTHRM,        &g_amec_sys.proc[0].tempmembufthrm),
  SENSOR_PTR(TEMPDIMMTHRM,          &g_amec_sys.proc[0].tempdimmthrm),
  SENSOR_PTR(TEMPMCDIMMTHRM,        &g_amec_sys.proc[0].tempmcdimmthrm),
  SENSOR_PTR(TEMPPMICTHRM,          &g_amec_sys.proc[0].temppmicthrm),
  SENSOR_PTR(TEMPMCEXTTHRM,         &g_amec_sys.proc[0].tempmcextthrm),

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
  SENSOR_PTR( CEFFVCSRATIO,         &g_amec_sys.wof_sensors.ceff_ratio_vcs_sensor),
  SENSOR_PTR( VRATIO_VDD,               &g_amec_sys.wof_sensors.v_ratio_sensor),
  SENSOR_PTR( OCS_ADDR,             &g_amec_sys.wof_sensors.ocs_addr_sensor),
  SENSOR_PTR( CEFFVDDRATIOADJ,      &g_amec_sys.wof_sensors.ceff_ratio_vdd_adj_sensor),
  SENSOR_PTR( IO_PWR_PROXY,         &g_amec_sys.wof_sensors.io_proxy_sensor),
  SENSOR_PTR( UV_AVG,               &g_amec_sys.wof_sensors.uv_avg_sensor),
  SENSOR_PTR( OV_AVG,               &g_amec_sys.wof_sensors.ov_avg_sensor),
  SENSOR_PTR( FREQ_PSTATE,          &g_amec_sys.wof_sensors.freq_pstate_sensor),
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
  MINI_SENSOR_PTR(     VRT_BCEdur,  NULL),
  MINI_SENSOR_PTR(     VRT_IPCdur,  NULL),
  MINI_SENSOR_PTR(    WOFC_IPCdur,  NULL),
  MINI_SENSOR_PTR(    Sout_BCEdur,  NULL),
  MINI_SENSOR_PTR(    CORE_IPCdur,  NULL),
  MINI_SENSOR_PTR(      timer3dur,  NULL),
  MINI_SENSOR_PTR(      timer4dur,  NULL),
  MINI_SENSOR_PTR(      timer5dur,  NULL),
  MINI_SENSOR_PTR(    GPEtickdur0,  NULL),
  MINI_SENSOR_PTR(    GPEtickdur1,  NULL),
  MINI_SENSOR_PTR(     RTLtickdur,  NULL),

  // ------------------------------------------------------
  // System Sensors
  // ------------------------------------------------------
  MINI_SENSOR_PTR(         PWRSYS,  NULL),
  MINI_SENSOR_PTR(         PWRGPU,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH00,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH01,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH02,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH03,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH04,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH05,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH06,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH07,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH08,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH09,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH10,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH11,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH12,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH13,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH14,  NULL),
  MINI_SENSOR_PTR(    PWRAPSSCH15,  NULL),
  MINI_SENSOR_PTR(     CUR12VSTBY,  NULL),
  MINI_SENSOR_PTR( VRHOTMEMPRCCNT,  NULL),
  MINI_SENSOR_PTR(    TEMPAMBIENT,  NULL),

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
  MINI_SENSOR_PTR(   TEMPPROCIOTHRM,&G_dcom_slv_outbox_tx.tempprociothermal),
  MINI_SENSOR_PTR(           UTIL,  &G_dcom_slv_outbox_tx.util),
  MINI_SENSOR_PTR(       TEMPRTAVG, NULL),
  MINI_SENSOR_PTR(       TEMPNEST0, NULL),
  MINI_SENSOR_PTR(       TEMPNEST1, NULL),
  MINI_SENSOR_PTR(   VOLTVDDSENSE,  NULL),
  MINI_SENSOR_PTR(   VOLTVDNSENSE,  NULL),
  MINI_SENSOR_PTR(   VOLTVCSSENSE,  NULL),
  MINI_SENSOR_PTR(   VOLTVIOSENSE,  NULL),
  MINI_SENSOR_PTR(         PWRVDD,  NULL),
  MINI_SENSOR_PTR(         PWRVDN,  NULL),
  MINI_SENSOR_PTR(         PWRVCS,  NULL),
  MINI_SENSOR_PTR(         PWRVIO,  NULL),
  MINI_SENSOR_PTR(   PROCPWRTHROT,  NULL),
  MINI_SENSOR_PTR(    PROCOTTHROT,  NULL),
  MINI_SENSOR_PTR(         DDSAVG,  &G_dcom_slv_outbox_tx.ddsAvg),
  MINI_SENSOR_PTR(         DDSMIN,  &G_dcom_slv_outbox_tx.ddsMin),

  // ------------------------------------------------------
  // Processor IO sensors
  // ------------------------------------------------------
  PROCIO_MINI_SENSOR_PTRS_NULL( TEMPPROCIO ),

  // ------------------------------------------------------
  // Quad Sensors (8 each)
  // ------------------------------------------------------
  QUAD_MINI_SENSOR_PTRS_NULL( TEMPQ ),

  // ------------------------------------------------------
  // Regulator Sensors
  // ------------------------------------------------------
  MINI_SENSOR_PTR( VOLTVDD,         NULL),
  MINI_SENSOR_PTR( VOLTVDN,         NULL),
  MINI_SENSOR_PTR( VOLTVCS,         NULL),
  MINI_SENSOR_PTR( VOLTVIO,         NULL),
  MINI_SENSOR_PTR( CURVDD,          &G_dcom_slv_outbox_tx.curVdd),
  MINI_SENSOR_PTR( CURVDN,          NULL),
  MINI_SENSOR_PTR( CURVCS,          NULL),
  MINI_SENSOR_PTR( PSTATE,          NULL),
  MINI_SENSOR_PTR( TEMPVDD,         &G_dcom_slv_outbox_tx.tempvdd),

  // ------------------------------------------------------
  // Core Sensors (24 of each)
  // ------------------------------------------------------
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
  MEMCONTROL_MINI_SENSOR_PTRS_NULL(TEMPMEMBUF),

  MINI_SENSOR_PTR( MEMOTTHROT,   NULL),

  MINI_SENSOR_PTR( TEMPMEMBUFTHRM,  &G_dcom_slv_outbox_tx.tempmembufthrm),
  MINI_SENSOR_PTR( TEMPDIMMTHRM,    &G_dcom_slv_outbox_tx.tempdimmthrm),
  MINI_SENSOR_PTR( TEMPMCDIMMTHRM,  &G_dcom_slv_outbox_tx.tempmcdimmthrm),
  MINI_SENSOR_PTR( TEMPPMICTHRM,    &G_dcom_slv_outbox_tx.temppmicthrm),
  MINI_SENSOR_PTR( TEMPMCEXTTHRM,   &G_dcom_slv_outbox_tx.tempmcextthrm),

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
  MINI_SENSOR_PTR( CEFFVDDRATIO,   &G_dcom_slv_outbox_tx.ceffRatioVdd),
  MINI_SENSOR_PTR( CEFFVCSRATIO,   NULL),
  MINI_SENSOR_PTR( VRATIO_VDD,     NULL),
  MINI_SENSOR_PTR( OCS_ADDR,       NULL),
  MINI_SENSOR_PTR( CEFFVDDRATIOADJ, NULL),
  MINI_SENSOR_PTR( IO_PWR_PROXY,   NULL),
  MINI_SENSOR_PTR( UV_AVG,         &G_dcom_slv_outbox_tx.uvAvg),
  MINI_SENSOR_PTR( OV_AVG,         &G_dcom_slv_outbox_tx.ovAvg),
  MINI_SENSOR_PTR( FREQ_PSTATE,   NULL),
};
STATIC_ASSERT(   (NUMBER_OF_SENSORS_IN_LIST != (sizeof(G_amec_mini_sensor_list)/sizeof(uint16_t *)))   );
STATIC_ASSERT(   (MAX_AMEC_SENSORS < (sizeof(G_amec_mini_sensor_list)/sizeof(uint16_t *)))   );
