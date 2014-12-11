/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/sensor/sensor_table.c $                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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
// and ptr to [0] entry of array of 12 core sensors
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
  [SENSOR_W_NUM(sensor,11)] = ptrbase[11].ptrmember

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
  [SENSOR_W_NUM(sensor,11)] = ptr[11]

// Will define a set of "core mini-sensor pointers" as NULL, since not
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
  [SENSOR_W_NUM(sensor,11)] = NULL

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
  SENSOR_PTR( TEMPAMBIENT,          &g_amec_sys.sys.tempambient),
  SENSOR_PTR( ALTITUDE,             &g_amec_sys.sys.altitude),
  SENSOR_PTR( PWR250US,             &g_amec_sys.sys.pwr250us),
  SENSOR_PTR( PWR250USFAN,          &g_amec_sys.fan.pwr250usfan),
  SENSOR_PTR( PWR250USIO,           &g_amec_sys.io.pwr250usio),
  SENSOR_PTR( PWR250USSTORE,        &g_amec_sys.storage.pwr250usstore),
  SENSOR_PTR( PWR250USGPU,          &g_amec_sys.sys.pwr250usgpu),
  SENSOR_PTR( FANSPEEDAVG,          &g_amec_sys.fan.fanspeedavg),  

  // ------------------------------------------------------
  // Chip Sensors
  // ------------------------------------------------------
  SENSOR_PTR( TODclock0,            &g_amec_sys.sys.todclock0 ),
  SENSOR_PTR( TODclock1,            &g_amec_sys.sys.todclock1 ),
  SENSOR_PTR( TODclock2,            &g_amec_sys.sys.todclock2 ),

  // ------------------------------------------------------
  // Processor Sensors
  // ------------------------------------------------------
  SENSOR_PTR( FREQA2MSP0,           &g_amec_sys.proc[0].freqa2ms),      
  SENSOR_PTR( IPS2MSP0,             &g_amec_sys.proc[0].ips2ms),
  SENSOR_PTR( MEMSP2MSP0,           &g_amec_sys.proc[0].memsp2ms),
  SENSOR_PTR( PWR250USP0,           &g_amec_sys.proc[0].pwr250us),
  SENSOR_PTR( PWR250USVDD0,         &g_amec_sys.proc[0].pwr250usvdd),
  SENSOR_PTR( CUR250USVDD0,         &g_amec_sys.proc[0].cur250usvdd),
  SENSOR_PTR( PWR250USVCS0,         &g_amec_sys.proc[0].pwr250usvcs),
  SENSOR_PTR( PWR250USMEM0,         &g_amec_sys.proc[0].pwr250usmem),   
  SENSOR_PTR( SLEEPCNT2MSP0,        &g_amec_sys.proc[0].sleepcnt2ms),
  SENSOR_PTR( WINKCNT2MSP0,         &g_amec_sys.proc[0].winkcnt2ms),
  SENSOR_PTR( SP250USP0,            &g_amec_sys.proc[0].sp250us),
  SENSOR_PTR( TEMP2MSP0,            &g_amec_sys.proc[0].temp2ms),
  SENSOR_PTR( TEMP2MSP0PEAK,        &g_amec_sys.proc[0].temp2mspeak),
  SENSOR_PTR( UTIL2MSP0,            &g_amec_sys.proc[0].util2ms),
  SENSOR_PTR( VRFAN250USPROC,       &g_amec_sys.sys.vrfan250usproc),
  SENSOR_PTR( VRHOT250USPROC,       &g_amec_sys.sys.vrhot250usproc),

  // ------------------------------------------------------
  // Core Sensors (12 of each)
  // ------------------------------------------------------
  CORE_SENSOR_PTRS( FREQ250USP0C ,  &g_amec_sys.proc[0].core, freq250us),
  CORE_SENSOR_PTRS( FREQA2MSP0C ,   &g_amec_sys.proc[0].core, freqa2ms), 
  CORE_SENSOR_PTRS( IPS2MSP0C ,     &g_amec_sys.proc[0].core, ips2ms),
  CORE_SENSOR_PTRS( NOTBZE2MSP0C ,  &g_amec_sys.proc[0].core, mcpifd2ms),
  CORE_SENSOR_PTRS( NOTFIN2MSP0C ,  &g_amec_sys.proc[0].core, mcpifi2ms),
  CORE_SENSOR_PTRS( SPURR2MSP0C ,   &g_amec_sys.proc[0].core, spurr2ms),
  CORE_SENSOR_PTRS( TEMP2MSP0C ,    &g_amec_sys.proc[0].core, temp2ms),
  CORE_SENSOR_PTRS( UTIL2MSP0C ,    &g_amec_sys.proc[0].core, util2ms),
  CORE_SENSOR_PTRS( NUTIL3SP0C ,    &g_amec_sys.proc[0].core, nutil3s),
  CORE_SENSOR_PTRS( MSTL2MSP0C ,    &g_amec_sys.proc[0].core, mstl2ms),
  CORE_SENSOR_PTRS( CMT2MSP0C ,     &g_amec_sys.proc[0].core, cmt2ms),
  CORE_SENSOR_PTRS( CMBW2MSP0C ,    &g_amec_sys.proc[0].core, cmbw2ms),
  CORE_SENSOR_PTRS( PPICP0C ,       &g_amec_sys.proc[0].core, ppic),
  CORE_SENSOR_PTRS( PWRPX250USP0C , &g_amec_sys.proc[0].core, pwrpx250us),

  // ------------------------------------------------------
  // Memory Sensors
  // ------------------------------------------------------
  SENSOR_PTR( VRFAN250USMEM,        &g_amec_sys.sys.vrfan250usmem),
  SENSOR_PTR( VRHOT250USMEM,        &g_amec_sys.sys.vrhot250usmem),

  MEMCONTROL_SENSOR_PTRS(MRD2MSP0M,     &g_amec_sys.proc[0].memctl, mrd2ms),
  MEMCONTROL_SENSOR_PTRS(MWR2MSP0M,     &g_amec_sys.proc[0].memctl, mwr2ms),
  MEMCONTROL_SENSOR_PTRS(MIRC2MSP0M,    &g_amec_sys.proc[0].memctl, centaur.mirc2ms),
  MEMCONTROL_SENSOR_PTRS(MLP2P0M,       &g_amec_sys.proc[0].memctl, centaur.mlp2ms),
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


  SENSOR_PTR( TEMP2MSCENT,          &g_amec_sys.proc[0].temp2mscent),
  SENSOR_PTR( TEMP2MSDIMM,          &g_amec_sys.proc[0].temp2msdimm),
  SENSOR_PTR( MEMSP2MS,             &g_amec_sys.proc[0].memsp2ms_tls),

  // ------------------------------------------------------
  // Regulator Sensors
  // ------------------------------------------------------
  SENSOR_PTR( UVOLT250USP0V0,       &g_amec_sys.proc[0].vrm[0].uvolt250us),
  SENSOR_PTR( UVOLT250USP0V1,       &g_amec_sys.proc[0].vrm[1].uvolt250us),
  SENSOR_PTR( VOLT250USP0V0,        &g_amec_sys.proc[0].vrm[0].volt250us),
  SENSOR_PTR( VOLT250USP0V1,        &g_amec_sys.proc[0].vrm[1].volt250us),

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
//   - Resident in initSection of SRAM, and will be deleted after sensor init
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
  MINI_SENSOR_PTR(    TEMPAMBIENT,  NULL),
  MINI_SENSOR_PTR(       ALTITUDE,  NULL),
  MINI_SENSOR_PTR(       PWR250US,  NULL),
  MINI_SENSOR_PTR(    PWR250USFAN,  NULL),
  MINI_SENSOR_PTR(     PWR250USIO,  NULL),
  MINI_SENSOR_PTR(  PWR250USSTORE,  NULL),
  MINI_SENSOR_PTR(    PWR250USGPU,  NULL),
  MINI_SENSOR_PTR(    FANSPEEDAVG,  NULL),  

  // ------------------------------------------------------
  // Chip Sensors
  // ------------------------------------------------------
  MINI_SENSOR_PTR(      TODclock0,  &G_dcom_slv_outbox_tx.todclock[0]),
  MINI_SENSOR_PTR(      TODclock1,  &G_dcom_slv_outbox_tx.todclock[1]),
  MINI_SENSOR_PTR(      TODclock2,  &G_dcom_slv_outbox_tx.todclock[2]),

  // ------------------------------------------------------
  // Processor Sensors
  // ------------------------------------------------------
  MINI_SENSOR_PTR(     FREQA2MSP0,  &G_dcom_slv_outbox_tx.freqa2msp0),  
  MINI_SENSOR_PTR(       IPS2MSP0,  &G_dcom_slv_outbox_tx.ips2msp0),  
  MINI_SENSOR_PTR(     MEMSP2MSP0,  NULL),
  MINI_SENSOR_PTR(     PWR250USP0,  &G_dcom_slv_outbox_tx.pwr250usp0),
  MINI_SENSOR_PTR(   PWR250USVDD0,  NULL),
  MINI_SENSOR_PTR(   CUR250USVDD0,  NULL),
  MINI_SENSOR_PTR(   PWR250USVCS0,  NULL),
  MINI_SENSOR_PTR(   PWR250USMEM0,  &G_dcom_slv_outbox_tx.pwr250usmemp0), 
  MINI_SENSOR_PTR(  SLEEPCNT2MSP0,  &G_dcom_slv_outbox_tx.sleepcnt2msp0),
  MINI_SENSOR_PTR(   WINKCNT2MSP0,  &G_dcom_slv_outbox_tx.winkcnt2msp0),
  MINI_SENSOR_PTR(      SP250USP0,  NULL),
  MINI_SENSOR_PTR(      TEMP2MSP0,  &G_dcom_slv_outbox_tx.temp2msp0),
  MINI_SENSOR_PTR(  TEMP2MSP0PEAK,  &G_dcom_slv_outbox_tx.temp2msp0peak),
  MINI_SENSOR_PTR(      UTIL2MSP0,  &G_dcom_slv_outbox_tx.util2msp0), 
  MINI_SENSOR_PTR( VRFAN250USPROC,  &G_dcom_slv_outbox_tx.vrfan250usproc),
  MINI_SENSOR_PTR( VRHOT250USPROC,  NULL),

  // ------------------------------------------------------
  // Core Sensors (12 of each)
  // ------------------------------------------------------
  CORE_MINI_SENSOR_PTRS_NULL(  FREQ250USP0C ),
  CORE_MINI_SENSOR_PTRS_NULL(   FREQA2MSP0C ),
  CORE_MINI_SENSOR_PTRS(          IPS2MSP0C, &G_dcom_slv_outbox_tx.ips2msp0cy    ),
  CORE_MINI_SENSOR_PTRS(       NOTBZE2MSP0C, &G_dcom_slv_outbox_tx.mcpifd2msp0cy ),
  CORE_MINI_SENSOR_PTRS(       NOTFIN2MSP0C, &G_dcom_slv_outbox_tx.mcpifi2msp0cy ),
  CORE_MINI_SENSOR_PTRS_NULL(   SPURR2MSP0C ),
  CORE_MINI_SENSOR_PTRS_NULL(    TEMP2MSP0C ),
  CORE_MINI_SENSOR_PTRS(         UTIL2MSP0C, &G_dcom_slv_outbox_tx.util2msp0cy   ),
  CORE_MINI_SENSOR_PTRS(         NUTIL3SP0C, &G_dcom_slv_outbox_tx.nutil3sp0cy   ),
  CORE_MINI_SENSOR_PTRS_NULL(    MSTL2MSP0C ),
  CORE_MINI_SENSOR_PTRS_NULL(     CMT2MSP0C ),
  CORE_MINI_SENSOR_PTRS_NULL(    CMBW2MSP0C ),
  CORE_MINI_SENSOR_PTRS_NULL(       PPICP0C ),
  CORE_MINI_SENSOR_PTRS(      PWRPX250USP0C, &G_dcom_slv_outbox_tx.pwrpx250usp0cy), //

  // ------------------------------------------------------
  // Memory Sensors
  // ------------------------------------------------------
  MINI_SENSOR_PTR( VRFAN250USMEM,  &G_dcom_slv_outbox_tx.vrfan250usmem),
  MINI_SENSOR_PTR( VRHOT250USMEM,  NULL),

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
  MINI_SENSOR_PTR( TEMP2MSDIMM,     &G_dcom_slv_outbox_tx.temp2msdimm), 
  MINI_SENSOR_PTR( MEMSP2MS,        NULL),      

  // ------------------------------------------------------
  // Regulator Sensors
  // ------------------------------------------------------
  MINI_SENSOR_PTR( UVOLT250USP0V0,  NULL),
  MINI_SENSOR_PTR( UVOLT250USP0V1,  NULL),
  MINI_SENSOR_PTR(  VOLT250USP0V0,  NULL),
  MINI_SENSOR_PTR(  VOLT250USP0V1,  NULL),

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


