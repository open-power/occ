/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_wof_thread.c $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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
/******************************************************************************
// @file amec_wof_thread.c
// @brief Create p-state table for WOF
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section amec_wof_thread.c AMEC_WOF_THREAD.C
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @cl020             lefurgy   03/11/2015  Created based on dcom_thread.c
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
#ifndef _AMEC_WOF_THREAD_C
#define _AMEC_WOF_THREAD_C

//*************************************************************************
// Includes
//*************************************************************************
//#include <pgp_pmc.h>
//#include "pgp_pba.h"
//#include <rtls.h>
//#include <apss.h>
//#include <dcom.h>
//#include <dcom_service_codes.h>
//#include <occ_service_codes.h>
//#include <trac.h>
//#include <state.h>
//#include <proc_pstate.h>
#include <amec_wof.h>

//*************************************************************************
// Externs
//*************************************************************************
extern uint8_t g_amec_wof_pstate_table_ready; // amec_wof.c

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

// Debug Counter to make sure dcom thread is running
uint16_t G_amec_wof_thread_counter = 0;

SsxSemaphore G_amecWOFThreadWakeupSem;

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************


// Function Specification
//
// Name:  Dcom_thread_routine
//
// Description: Purpose of this task is to handle messages passed from 
//              Master to Slave and vice versa.
//              
//              Nothing in this thread should be time-critical, but should
//              happen more often than the 1-second that other threads run
//              at.
//
//              This thread currently runs ~1ms, based on the RTL loop of 
//              250us.  
//
//              FWIW -- It is pointless to set this thread to run any more
//              often than the length of the RTL loop, since it is acting
//              on data passed back and forth via that loop.
//
// Flow:  XX-XX-XX    FN=
//
// End Function Specification
void amec_wof_thread_routine(void *arg)
{
    for(;;)
    {
        // --------------------------------------------------
        // Wait on Semaphore until we get new data over DCOM
        // (signalled by sem_post() or timeout occurs.
        // Sem timeout is designed to be the slowest 
        // interval we will attempt to run this thread at.
        // --------------------------------------------------

        // Wait for sem_post from WOF 2ms loop before we run through this thread.
        ssx_semaphore_pend(&G_amecWOFThreadWakeupSem, SSX_WAIT_FOREVER);    // @th035

        // --------------------------------------------------
        // Counter to ensure thread is running (can wrap)
        // --------------------------------------------------
        G_amec_wof_thread_counter++;

	//FIXME: update p-state table here

	//Signal 250us real-time WOF loop that p-state table is updated and ready for use
	g_amec_wof_pstate_table_ready = 1;
    }
}

#endif //_DCOM_THREAD_C

