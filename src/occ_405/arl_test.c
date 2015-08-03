/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/arl_test.c $                                      */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

#include "ssx.h"
#include "ssx_io.h"
#include "simics_stdio.h"
#include "arl_test_data.h"

#ifdef AMEC_ARL_TEST_COMPILE
sOCCData_t gOCC = {0};
sRTLData_t gRTL = {0};

sOCCData_t * gpOCC = &gOCC;
sRTLData_t * gpRTL = &gRTL;
#endif

///////////////////////////////////////////////////////////////////////////////
// Function Specification
//
// Name: arl_test
//
// Description:  Perform any needed mode changes
void arl_test(void)
{
#ifdef AMEC_ARL_TEST_COMPILE
    UINT8    i = 0;
    UINT32   temp32  = 0;
    UINT32   temp32a = 0;
    UINT16   temp16 = 0;
    UINT16   temp16a = 0;


    //gpRTL->cycles250usincrement = CYCLES_INC;      // hard coded to correspond to 4GHz and the # of cycles every 250usec
    gpRTL->cycles250us = gpRTL->cycles250us + gpRTL->cycles250usincrement;

    // Inject raw cycle counter (currently based on 4GHz core frequency)
    gpOCC->SCOM_ARRAY[INDEX_PC_RAW]=gpRTL->cycles250us;

    // Compute differential in raw PC cycles
    temp32 = gpOCC->SCOM_ARRAY[INDEX_PC_RAW];
    temp32a = gpRTL->scom_prev_PC_RAW_250us;
    temp32 = temp32 - temp32a;

    // Convert to frequency for this core
    temp32 = temp32 << 8;                         // Shift left 8 bits (raw value is < 2^24)
    temp32a = 64000;                              // Divide by 64000 to convert to frequency sensor in 1MHz increments
    temp32 = temp32 / temp32a;
    gpRTL->FREQ250USP0C0=(UINT16)temp32;          // Write to frequency sensor


    // Debug code to inject data into SCOMs for per thread information
    for (i=0; i<MAX_THREADS; i++)
    {
        temp32 = gpRTL->utilcounter[i]+gpRTL->utilincrement[i];
        gpRTL->utilcounter[i]=temp32;
        gpOCC->SCOM_ARRAY[INDEX_PC_RUN_T0+i]=temp32;
    }


    //////////////////////
    // 2ms state machine

    switch (gpOCC->AME_sm_cnt)
    {

        /////////////////////////////////////////////////////////////
        // STATE 0 of EMPATH 2msec State Machine
        // This state processes core 0's per thread utilization SCOMs
        /////////////////////////////////////////////////////////////
        case 0x00:

            // Code to read SCOM_ARRAY and convert into actual per thread utilization sensors

            // First step is to calculate how many raw cycles went by in the core during the last 2msec
            temp32 = gpOCC->SCOM_ARRAY[INDEX_PC_RAW];
            temp32a = gpRTL->scom_prev_PC_RAW_2ms;
            // Compute 32 bit differential between SCOM reads now and previous 2msec
            gpRTL->cycles2ms = temp32 - temp32a;


            for (i=0; i<MAX_THREADS; i++)
            {
                temp32 = gpOCC->SCOM_ARRAY[INDEX_PC_RUN_T0+i];    // Read 32 bit SCOM value for this thread (free running)
                temp32a = gpRTL->scom_prev_thread[i];             // Read previous 32 bit SCOM value from 32msec ago
                temp32 = temp32 - temp32a;                        // Compute 32 bit differential between SCOM reads now and previous 2msec
                temp32 = temp32 >> 8;                             // Limit to 16 bits (highest count is < 2^24)
                temp16 = (UINT16)temp32;
                temp16a = 10000;                                  // 10000 = 100.00%
                temp32 = ((UINT32)temp16a)*((UINT32)temp16);      // scale by 10000
                temp32a = gpRTL->cycles2ms;
                temp32a = temp32a >> 8;                           // Limit to 16 bits
                temp32 = temp32 / temp32a;
                gpRTL->util2ms_thread[i]=(UINT16)temp32;
            }


            // Last step is to remember current SCOMs in scom_prev_thread array for next time
            for (i=0; i<MAX_THREADS; i++)
            {
                gpRTL->scom_prev_thread[i] = gpOCC->SCOM_ARRAY[INDEX_PC_RUN_T0+i];
            }

            gpRTL->scom_prev_PC_RAW_2ms = gpOCC->SCOM_ARRAY[INDEX_PC_RAW];   // Capture raw cycle count for usage in the next 2msec

            break;

        /////////////////////////////////////////////////////////////
        // STATE 1 of EMPATH 2msec State Machine
        /////////////////////////////////////////////////////////////
        case 0x01:

            break;

        /////////////////////////////////////////////////////////////
        // STATE 2 of EMPATH 2msec State Machine
        /////////////////////////////////////////////////////////////
        case 0x02:

            break;

        /////////////////////////////////////////////////////////////
        // STATE 3 of EMPATH 2msec State Machine
        /////////////////////////////////////////////////////////////
        case 0x03:

        break;

        /////////////////////////////////////////////////////////////
        // STATE 4 of EMPATH 2msec State Machine
        /////////////////////////////////////////////////////////////
        case 0x04:

            break;

        /////////////////////////////////////////////////////////////
        // STATE 5 of EMPATH 2msec State Machine
        /////////////////////////////////////////////////////////////
        case 0x05:

            break;

        /////////////////////////////////////////////////////////////
        // STATE 6 of EMPATH 2msec State Machine
        /////////////////////////////////////////////////////////////
        case 0x06:

            break;

        /////////////////////////////////////////////////////////////
        // STATE 7 of EMPATH 2msec State Machine
        /////////////////////////////////////////////////////////////
        case 0x07:

            break;

    }


    // Capture raw cycle count to use in next 250us
    gpRTL->scom_prev_PC_RAW_250us = gpOCC->SCOM_ARRAY[INDEX_PC_RAW];

    // Increment AME state machine counter using modulo AME_SM_TICK
    gpOCC->AME_sm_cnt = (UINT8) ((gpOCC->AME_sm_cnt + 1) & (AME_SM_TICKS - 1));

    // Always increment AME time on every call to ISR
    gpOCC->r_cnt++;

    // Insert ARL Test Code Here
    //printf("ARL Test Code\n");

#endif
}


