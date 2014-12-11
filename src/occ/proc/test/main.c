/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/proc/test/main.c $                                    */
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


#include "ssx.h"
#include "ssx_io.h"
#include "simics_stdio.h"
#include <thread.h>
#include <threadSch.h>
#include <errl.h>
#include <apss.h>
#include <appletManager.h>
#include <trac.h>
#include <occ_service_codes.h>
#include <occ_sys_config.h>
#include <proc_data.h>
#include <timer.h>
#include <dcom.h>
#include <rtls.h>

extern void __ssx_boot;

IMAGE_HEADER (G_mainAppImageHdr,__ssx_boot,MAIN_APP_ID,OCC_APLT_TEST);

// Period in which to run #timer_routine
#define TIMER_INTERVAL (SsxInterval) SSX_MICROSECONDS(5000)

int g_j = 0;
int g_k = 0;

SimicsStdio simics_stdout;
SimicsStdio simics_stderr;

uint8_t noncritical_stack[NONCRITICAL_STACK_SIZE];
uint8_t critical_stack[CRITICAL_STACK_SIZE];

// Function Specification
//
// Name: pgp_validation_ssx_main_hook
//
// Description:
//
// End Function Specification
void pgp_validation_ssx_main_hook(void)
{

}

// Function Specification
//
// Name: Cmd_Hndl_thread_routine
//
// Description:
//
// End Function Specification
void Cmd_Hndl_thread_routine(void *arg)
{
    TRAC_INFO("Command Handler Thread Started ... " );
}

// Function Specification
//
// Name: Thermal_Monitor_thread_routine
//
// Description:
//
// End Function Specification
void Thermal_Monitor_thread_routine(void *arg)
{
    TRAC_INFO("Thermal Monitor Thread Started ... " );
}

// Function Specification
//
// Name: Hlth_Monitor_thread_routine
//
// Description:
//
// End Function Specification
void Hlth_Monitor_thread_routine(void *arg)
{
    TRAC_INFO("Health Monitor Thread Started ... " );
}

// Function Specification
//
// Name: FFDC_thread_routine
//
// Description:
//
// End Function Specification
void FFDC_thread_routine(void *arg)
{
    TRAC_INFO("FFDC Thread Started ... " );
}

// Function Specification
//
// Name: get_core_info
//
// Description:
//
// End Function Specification
void get_core_info()
{
    //uintt64_t l_deconfigured_cores;
    uint32_t l_deconfigured_cores;
    uint32_t l_configured_cores;

   //rc = _getscom( PMC_CORE_DECONFIGURATION_REG, &l_deconfigured_cores, SCOM_TIMEOUT );

   l_deconfigured_cores = in32(PMC_CORE_DECONFIGURATION_REG);
   PROC_DEBUG( "Deconfigured cores in the chip [0x%x]\n", l_deconfigured_cores);


   l_configured_cores = ~l_deconfigured_cores & ALL_CORES_MASK;
   PROC_DEBUG( "Configured cores in the chip [0x%x]\n", l_configured_cores);

   return;
}

// Function Specification
//
// Name: main_thread_routine
//
// Description: This thread currently just loops as the lowest priority thread, handling
//              the lowest priority tasks.
//
// End Function Specification
void main_thread_routine(void *private)
{

    TRAC_INFO("Main Thread Started ... " );
    errlHndl_t l_errl = NULL;
    SsxSemaphore    l_appletComplete;
    uint8_t         l_status = 0;
    uint32_t        l_rc = 0 ;
    int             l_ssxrc = 0;

   // Start the critical 250uS timer
   //ssx_timer_schedule(&timer, 1, TIMER_INTERVAL);

    // Initialize applet semaphore
    l_ssxrc = ssx_semaphore_create(&l_appletComplete, 0, 1);

    // Initialize APSS
#ifdef OCC_ALONE_SIMICS
    l_rc = runApplet(OCC_APLT_APSS_INIT,   // Applet enum Name
                     NULL,                 // Applet arguments
                     TRUE,                 // Blocking call?
                     &l_appletComplete,    // Applet finished semaphore
                     &l_errl,              // error log handle
                     &l_status);           // status from applet manager
#endif
    if( NULL != l_errl )
    {
        TRAC_ERR("APSS Init failed! (retrying) ErrLog[%p]", l_errl );
        setErrlSevToInfo(l_errl);
        // commit & delete
        commitErrl( &l_errl );

        l_rc = runApplet(OCC_APLT_APSS_INIT, // Applet enum Name
                         NULL,               // Applet arguments
                         TRUE,               // Blocking call?
                         &l_appletComplete,  // Applet finished semaphore
                         &l_errl,            // error log handle
                         &l_status);         // status from applet manager
        if( NULL != l_errl )
        {
            TRAC_ERR("APSS Init failed again! (OCC will be reset)ErrLog[%p]",
                     l_errl );

            // commit & delete
            commitErrl( &l_errl );

            //$TODO: Request OCC Reset
        }
    }

    while(1)
    {
        // Only trace the first XX times that this function loops
        if(g_k < 3)
        {
            g_k++;

        }

        // Sleep for 1000 before we run the loop again
        ssx_sleep(SSX_SECONDS(5));
    }
}


// Function Specification
//
// Name: main
//
// Description: Entry point for OCC execution
//              Currently initalizes our trace buffer along with creating threads
//              and timers for execution.  Note that once main runs ssx_start_threads,
//              we never return as the SSX kernel takes over.
//
// End Function Specification
int main(int argc, char **argv)
{
    // Initialize stdout so we can do printf from within simics env
    simics_stdout_create(&simics_stdout);
    simics_stderr_create(&simics_stderr);
    stdout = (FILE *)(&simics_stdout);
    stderr = (FILE *)(&simics_stderr);
    ssxout = (FILE *)(&simics_stdout);

    TRAC_INFO("Inside OCC Main");

    // Initialize SSX Stacks (note that this also reinitializes the time base to 0)
    ssx_initialize((SsxAddress)noncritical_stack, NONCRITICAL_STACK_SIZE,
           (SsxAddress)critical_stack, CRITICAL_STACK_SIZE,
           0);

    // Create Threads
    ssx_thread_create(&main_thread,
              main_thread_routine,
              (void *)0,
              (SsxAddress)main_thread_stack,
              THREAD_STACK_SIZE,
              1);


    // Make Threads runnable
    ssx_thread_resume(&main_thread);

    errlHndl_t  l_errl = NULL;
    //Initialize the thread scheduler
    l_errl = initThreadScheduler();

    if( l_errl )
    {
        // Trace and commit error
        TRAC_ERR("init thread Scheduler failure");

        // commit log
        // NOTE: log should be deleted by reader mechanism
        commitErrl( &l_errl );
    }

    // lets map mainstore to oci space only once
    // NOTE:  This will stay mapped for remainder of occ life
    // TODO:  This sounds like a temporary solution and may
    //        end up moving to simics environment setup
    // TODO: This map needs to be unmapped after done accessing
    //       main memory for the OCC initialization.
    Ppc405MmuMap pba_mmu_map;
    int l_ssxrc1 = ppc405_mmu_map(
                0,              //Mainstore address 0x0
                0,              //OCI address 0x0
                1048576,        //Max size = 1 Mb
                0,              //
                0,              //no TLBIE permissions only need to read
                &pba_mmu_map
                );

    if ( l_ssxrc1 != SSX_OK )
    {
        tracDesc_t  l_trace = NULL;

        TRAC_ERR("mmu map failure SsxRc[0x%08X]", -l_ssxrc1 );

        /*
         * @errortype
         * @moduleid    MAIN_MID
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   pore_flex_schedule return code
         * @userdata4   ERC_MMU_MAP_FAILURE
         * @devdesc     Failure mapping OCI space
         */
        l_errl = createErrl(
                    MAIN_MID,               //modId
                    SSX_GENERIC_FAILURE,    //reasoncode
                    ERC_MMU_MAP_FAILURE,    //Extended reason code
                    ERRL_SEV_UNRECOVERABLE, //Severity
                    l_trace,                //Trace Buf
                    DEFAULT_TRACE_SIZE,     //Trace Size
                    l_ssxrc1,               //userdata1
                    0                       //userdata2
                    );

        // Callout firmware
        addCalloutToErrl(l_errl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        // commit log
        commitErrl( &l_errl );

        // TODO request a reset of OCC since applet manager will
        // be toast without this working correctly
    }

    //Initialize the Applet Manager
    l_errl = initAppletManager();

    if( l_errl )
    {
        // Trace and commit error
        TRAC_ERR("init Applet Manager failure");

        // commit log
        commitErrl( &l_errl );

        // TODO: request a reset of OCC since applet manager will
        //       be toast without this working correctly
    }

    //Initialize structures for collecting core data.
    //It needs to run before RTLoop start.
    proc_core_init();

    get_core_info();

    // Initialize Realtime Loop Timer Interrupt
    rtl_ocb_init();

    // Enter SSX Kernel
    ssx_start_threads();

    return 0;
}

