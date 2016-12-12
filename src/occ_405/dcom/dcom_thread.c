/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/dcom/dcom_thread.c $                              */
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

#ifndef _DCOM_THREAD_C
#define _DCOM_THREAD_C

#include "ssx.h"
#include "occhw_pba.h"
#include <rtls.h>
#include <apss.h>
#include <dcom.h>
#include <dcom_service_codes.h>
#include <occ_service_codes.h>
#include <trac.h>
#include <state.h>
#include <proc_pstate.h>
#include <amec_freq.h>

// Debug Counter to make sure dcom thread is running
uint16_t G_dcom_thread_counter = 0;

SsxSemaphore G_dcomThreadWakeupSem;

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
//              FWIW -- It is pointless to set this thread to run any more
//              often than the length of the RTL loop, since it is acting
//              on data passed back and forth via that loop.
//
// End Function Specification
void Dcom_thread_routine(void *arg)
{
    OCC_STATE l_newOccState  = 0;
    OCC_MODE  l_newOccMode   = 0;
    SsxTimer  l_timeout_timer;
    errlHndl_t l_errlHndl = NULL;
    // --------------------------------------------------
    // Create a timer that pops every 10 seconds to wake up
    // this thread, in case a semaphore never gets posted.
    // --------------------------------------------------
    ssx_timer_create(&l_timeout_timer,
                     (SsxTimerCallback) ssx_semaphore_post,
                     (void *) &G_dcomThreadWakeupSem);
    ssx_timer_schedule(&l_timeout_timer,
                       SSX_SECONDS(10),
                       SSX_SECONDS(10));

    DCOM_TRAC_INFO("DCOM Thread Started");
    for(;;)
    {
        // --------------------------------------------------
        // Wait on Semaphore until we get new data over DCOM
        // (signalled by sem_post() or timeout occurs.
        // Sem timeout is designed to be the slowest
        // interval we will attempt to run this thread at.
        // --------------------------------------------------

        // Wait for sem_post before we run through this thread.
        ssx_semaphore_pend(&G_dcomThreadWakeupSem, SSX_WAIT_FOREVER);

        // --------------------------------------------------
        // Counter to ensure thread is running (can wrap)
        // --------------------------------------------------
        G_dcom_thread_counter++;

// NOTE: Temporary system config must say we are FSP system so
// that we don't try to access main memory here.
        // --------------------------------------------------
        // Check if we need to update the opal table
        // --------------------------------------------------
        if(G_sysConfigData.system_type.kvm)
        {
            check_for_opal_updates();
        }

        // --------------------------------------------------
        // Set Mode and State Based on Master
        // --------------------------------------------------
        l_newOccState = (G_occ_master_state == CURRENT_STATE()) ? OCC_STATE_NOCHANGE : G_occ_master_state;

        if(G_sysConfigData.system_type.kvm)
        {
            l_newOccMode  = (G_occ_master_mode  == G_occ_external_req_mode_kvm ) ? OCC_MODE_NOCHANGE : G_occ_master_mode;
        }
        else
        {
            l_newOccMode  = (G_occ_master_mode  == CURRENT_MODE() ) ? OCC_MODE_NOCHANGE : G_occ_master_mode;
        }

        // Override State if SAFE state is requested
        l_newOccState = ( isSafeStateRequested() ) ? OCC_STATE_SAFE : l_newOccState;

        // Override State if we are in SAFE state already
        l_newOccState = ( OCC_STATE_SAFE == CURRENT_STATE() ) ? OCC_STATE_NOCHANGE : l_newOccState;

        if( (OCC_STATE_NOCHANGE != l_newOccState)
            || (OCC_MODE_NOCHANGE != l_newOccMode) )
        {
            // If we're active, then we should always process the mode change first
            // If we're not active, then we should always process the state change first
            if(OCC_STATE_ACTIVE == CURRENT_STATE())
            {
                // Set the new mode
                l_errlHndl = SMGR_set_mode(l_newOccMode);
                if(l_errlHndl)
                {
                    commitErrl(&l_errlHndl);
                }
                // Set the new state
                l_errlHndl = SMGR_set_state(l_newOccState);
                if(l_errlHndl)
                {
                    commitErrl(&l_errlHndl);
                }
            }
            else
            {
                // Set the new state
                l_errlHndl = SMGR_set_state(l_newOccState);
                                if(l_errlHndl)
                {
                    commitErrl(&l_errlHndl);
                }
                // Set the new mode
                l_errlHndl = SMGR_set_mode(l_newOccMode);
                if(l_errlHndl)
                {
                    commitErrl(&l_errlHndl);
                }
            }
        }


        // --------------------------------------------------
        // SSX Sleep
        // --------------------------------------------------
        // Even if semaphores are continually posted, there is no reason
        // for us to run this thread any more often than once every tick
        // so we don't starve any other thread
        ssx_sleep(SSX_MICROSECONDS(MICS_PER_TICK));
    }
}

#endif //_DCOM_THREAD_C

