/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/arl_test_data.h $                                     */
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

#ifndef _arl_test_data_h
#define _arl_test_data_h



// Uncomment this to enable the ARL Test Code compile flag.
//#define AMEC_ARL_TEST_COMPILE 1


#ifdef AMEC_ARL_TEST_COMPILE
typedef uint32_t UINT32;
typedef uint16_t UINT16;
typedef uint8_t  UINT8;

#define MAX_THREADS 8

// Definitions of SCOM_ARRAY offsets
// The following is an offset into a 32 bit array that contains the SCOM values read by the OCC
// Later, the order will likely change to reflect the order in which the SCOMs are written into local OCC memory
// by the GUPI program that is reading them from the physical SCOM registers and depositing them into OCC memory.
#define INDEX_PC_RUN_T0 0
#define INDEX_PC_RUN_T1 1
#define INDEX_PC_RUN_T2 2
#define INDEX_PC_RUN_T3 3
#define INDEX_PC_RUN_T4 4
#define INDEX_PC_RUN_T5 5
#define INDEX_PC_RUN_T6 6
#define INDEX_PC_RUN_T7 7
#define INDEX_PC_RAW    8
#define MAX_INDICES     9   // equal to the total number of SCOM_ARRAY entries

#define CYCLES_INC  1000000 // cycles increment: corresponds to the # of core cycles every 250usec (currently set to correspond to 4GHz core)
#define AME_SM_TICKS  8     // Basic state machine modulo 8 (8*250usec = 2msec)



typedef struct
{
    UINT32  r_cnt;                        // 32 bit time stamp counting interrupt ticks
    UINT32  SCOM_ARRAY[MAX_INDICES];      // Array of SCOM values (32 bits each) read by the GUPI program and placed into OCC SRAM
    UINT8   AME_sm_cnt;                   // Underlying AME state machine counter (modulo 8)
} sOCCData_t;

typedef struct
{
    UINT32  cycles250us;                  // base cycle counter counting each tick of the core's clock
    UINT32  cycles250usincrement;         // amount to increment base cycles by every 250usec
    UINT32  cycles2ms;                    // computes cycles every 2msec using PC_RAW SCOM register
    UINT32  scom_prev_PC_RAW_250us;       // Remembers previous SCOM value for PC_RAW SCOM register 250us ago
    UINT32  scom_prev_PC_RAW_2ms;         // Remembers previous SCOM value for PC_RAW SCOM register 2msec ago
    UINT32  scom_prev_thread[MAX_THREADS];// Remembers previous SCOM value for each thread
    UINT32  utilcounter[MAX_THREADS];     // base counter for synthesizing per thread utilization (value written to SCOMs every 250usec)
    UINT32  utilincrement[MAX_THREADS];   // increment for synthesizing per thread utilization (cycles per 250usec with run latch on)
    UINT16  util2ms_thread[MAX_THREADS];  // array of utilization sensors for P0 Core 0  (0.01% increments)
    UINT16  FREQ250USP0C0;                // actual frequency sensor for P0 Core 0 (1MHz increments)

} sRTLData_t;

extern sOCCData_t gOCC;
extern sRTLData_t gRTL;

extern sOCCData_t * gpOCC;
extern sRTLData_t * gpRTL;

#endif

#endif //_arl_test_data_h
