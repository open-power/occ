
/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/main.c $                                              */
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
#include "ipc_api.h"                //ipc base interfaces
#include "occhw_async.h"            //async (GPE, OCB, etc) interfaces
#include "simics_stdio.h"
//#include "heartbeat.h"
#include <thread.h>
#include <sensor.h>
#include <threadSch.h>
#include <errl.h>
#include <apss.h>
#include <trac.h>
#include <occ_service_codes.h>
#include <occ_sys_config.h>
#include <timer.h>
#include <dcom.h>
#include <rtls.h>
#include <proc_data.h>
#include <centaur_data.h>
#include <dpss.h>
#include <state.h>
#include <amec_sys.h>
#include <cmdh_fsp.h>
#include <proc_pstate.h>
//#include <vrm.h>
#include <chom.h>
#include <homer.h>
//#include <amec_health.h>
//#include <amec_freq.h>
#include <thrm_thread.h>
#include "scom.h"
//#include <fir_data_collect.h>
#include <pss_service_codes.h>

extern uint32_t __ssx_boot; // Function address is 32 bits
extern uint32_t G_occ_phantom_critical_count;
extern uint32_t G_occ_phantom_noncritical_count;
extern uint8_t G_occ_interrupt_type;
extern uint8_t G_occ_role;

extern GpeRequest G_meas_start_request;
extern GpeRequest G_meas_cont_request;
extern GpeRequest G_meas_complete_request;
extern apss_start_args_t    G_gpe_start_pwr_meas_read_args;
extern apss_continue_args_t G_gpe_continue_pwr_meas_read_args;
extern apss_complete_args_t G_gpe_complete_pwr_meas_read_args;



IMAGE_HEADER (G_mainAppImageHdr,__ssx_boot,MAIN_APP_ID,ID_NUM_INVALID);

//Set main thread timer for one second
#define MAIN_THRD_TIMER_SLICE ((SsxInterval) SSX_SECONDS(1))

// SIMICS printf/printk
SimicsStdio G_simics_stdout;
SimicsStdio G_simics_stderr;

//  Critical /non Critical Stacks
uint8_t G_noncritical_stack[NONCRITICAL_STACK_SIZE];
uint8_t G_critical_stack[CRITICAL_STACK_SIZE];

//NOTE: Three semaphores are used so that if in future it is decided
// to move thermal, health monitor and FFDC into it's own threads, then
// it can be done easily without more changes.
// Semaphores for the thermal functions
SsxSemaphore G_thrmSem;
// Semaphores for the health monitor functions
SsxSemaphore G_hmonSem;
// Semaphores for the FFDC functions
SsxSemaphore G_ffdcSem;
// Timer for posting thermal, health monitor and FFDC semaphore
SsxTimer G_mainThrdTimer;

// Variable holding main thread loop count
uint32_t G_mainThreadLoopCounter = 0x0;
// Global flag indicating FIR collection is required
bool G_fir_collection_required = FALSE;

extern uint8_t g_trac_inf_buffer[];
extern uint8_t g_trac_imp_buffer[];
extern uint8_t g_trac_err_buffer[];

void pmc_hw_error_isr(void *private, SsxIrqId irq, int priority);

//Macro creates a 'bridge' handler that converts the initial fast-mode to full
//mode interrupt handler
SSX_IRQ_FAST2FULL(pmc_hw_error_fast, pmc_hw_error_isr);


/*
 * Function Specification
 *
 * Name: pmc_hw_error_isr
 *
 * Description: Handles IRQ for PMCLFIR bit being set.  Only bits that are
 *              unmasked (0x1010843) and have action0 (0x1010846) set to 1
 *              and action1 (0x1010847) set to 0 will cause this interupt
 *              (OISR0[9]) to fire. This runs in a non critical context
 *              (tracing allowed).
 *
 *
 * End Function Specification
 */
void pmc_hw_error_isr(void *private, SsxIrqId irq, int priority)
{
    // TEMP / TODO -- Unused var
    //errlHndl_t  l_err;
    //pmc_ffdc_data_t l_pmc_ffdc;
    SsxMachineContext ctx;

    // Mask this interrupt
    ssx_irq_disable(irq);

    // disable critical interrupts
    ssx_critical_section_enter( SSX_NONCRITICAL, &ctx );

    // clear this irq status in OISR0
    ssx_irq_status_clear(irq);

    // dump a bunch of FFDC registers
    //fill_pmc_ffdc_buffer(&l_pmc_ffdc);

    MAIN_TRAC_ERR("PMC Failure detected through OISR0[9]!!!");
    /* @
     * @moduleid   PMC_HW_ERROR_ISR
     * @reasonCode PMC_FAILURE
     * @severity   ERRL_SEV_PREDICTIVE
     * @userdata1  0
     * @userdata2  0
     * @userdata4  OCC_NO_EXTENDED_RC
     * @devdesc    Failure detected in processor
     *             power management controller (PMC)
     */
/* TEMP NO MORE PMC
    l_err = createErrl( PMC_HW_ERROR_ISR,          // i_modId,
                        PMC_FAILURE,               // i_reasonCode,
                        OCC_NO_EXTENDED_RC,
                        ERRL_SEV_PREDICTIVE,
                        NULL,                      // tracDesc_t i_trace,
                        DEFAULT_TRACE_SIZE,        // i_traceSz,
                        0,                         // i_userData1,
                        0);                        // i_userData2
    //Add our register dump to the error log
    addUsrDtlsToErrl(l_err,
            (uint8_t*) &l_pmc_ffdc,
            sizeof(l_pmc_ffdc),
            ERRL_USR_DTL_STRUCT_VERSION_1,
            ERRL_USR_DTL_BINARY_DATA);

    //Add firmware callout
    addCalloutToErrl(l_err,
            ERRL_CALLOUT_TYPE_COMPONENT_ID,
            ERRL_COMPONENT_ID_FIRMWARE,
            ERRL_CALLOUT_PRIORITY_HIGH);

    //Add processor callout
    addCalloutToErrl(l_err,
            ERRL_CALLOUT_TYPE_HUID,
            G_sysConfigData.proc_huid,
            ERRL_CALLOUT_PRIORITY_MED);

    //Add planar callout
    addCalloutToErrl(l_err,
            ERRL_CALLOUT_TYPE_HUID,
            G_sysConfigData.backplane_huid,
            ERRL_CALLOUT_PRIORITY_LOW);

    REQUEST_RESET(l_err);
*/
    // Unmask this interrupt
    ssx_irq_enable(irq);

    // re-enable non-critical interrupts
    ssx_critical_section_exit( &ctx );
}

/*
 * Function Specification
 *
 * Name: occ_hw_error_isr
 *
 * Description: Handles IRQ for OCCLFIR bit being set.  Only bits
 *              that are unmasked (0x1010803) and have action0 (0x1010806)
 *              set to 1 and action1 (0x1010807) set to 0 will cause this
 *              interupt (OISR0[2]) to fire.  This runs in a critical
 *              context (no tracing!).
 *
 * End Function Specification
 */
//NOTE: use "putscom pu 6B111 0 3 101 -ib -p1" to inject the error.
#define OCC_LFIR_SPARE_BIT50 0x0000000000002000ull //gm031
void occ_hw_error_isr(void *private, SsxIrqId irq, int priority)
{
    //set bit 50 of the OCC LFIR so that the PRDF component will log an error and callout the processor
    //TMGT will also see a problem and log an error but it will be informational.
    _putscom(OCB_OCCLFIR_OR, OCC_LFIR_SPARE_BIT50, SCOM_TIMEOUT);

    //Halt occ so that hardware will enter safe mode
    OCC_HALT(ERRL_RC_OCC_HW_ERROR);
}

// Enable and register any ISR's that need to be set up as early as possible.
void occ_irq_setup()
{
    int         l_rc;
    errlHndl_t  l_err;

    do
    {

        // ------------- OCC Error IRQ Setup ------------------

        // Disable the IRQ while we work on it
        ssx_irq_disable(OCCHW_IRQ_OCC_ERROR);

        // Set up the IRQ
        l_rc = ssx_irq_setup(OCCHW_IRQ_OCC_ERROR,
                             SSX_IRQ_POLARITY_ACTIVE_HIGH,
                             SSX_IRQ_TRIGGER_EDGE_SENSITIVE);
        if(l_rc)
        {
            MAIN_TRAC_ERR("occ_irq_setup: ssx_irq_setup(OCCHW_IRQ_OCC_ERROR) failed with rc=0x%08x", l_rc);
            break;
        }

        // Register the IRQ handler with SSX
        l_rc = ssx_irq_handler_set(OCCHW_IRQ_OCC_ERROR,
                                   occ_hw_error_isr,
                                   NULL,
                                   SSX_CRITICAL);
        if(l_rc)
        {
            MAIN_TRAC_ERR("occ_irq_setup: ssx_irq_handler_set(OCCHW_IRQ_OCC_ERROR) failed with rc=0x%08x", l_rc);
            break;
        }

        //enable the IRQ
        ssx_irq_status_clear(OCCHW_IRQ_OCC_ERROR);
        ssx_irq_enable(OCCHW_IRQ_OCC_ERROR);


        // ------------- PMC Error IRQ Setup ------------------
/* TEMP -- IS THIS NO LONGER A THING IN P9??

        // Disable the IRQ while we work on it
        ssx_irq_disable(OCCHW_IRQ_PMC_ERROR);

        // Set up the IRQ
        l_rc = ssx_irq_setup(OCCHW_IRQ_PMC_ERROR,
                             SSX_IRQ_POLARITY_ACTIVE_HIGH,
                             SSX_IRQ_TRIGGER_EDGE_SENSITIVE);
        if(l_rc)
        {
            MAIN_TRAC_ERR("occ_irq_setup: ssx_irq_setup(OCCHW_IRQ_PMC_ERROR) failed with rc=0x%08x", l_rc);
            break;
        }

        // Register the IRQ handler with SSX
        l_rc = ssx_irq_handler_set(OCCHW_IRQ_PMC_ERROR,
                                   pmc_hw_error_fast,
                                   NULL,
                                   SSX_NONCRITICAL);
        if(l_rc)
        {
            MAIN_TRAC_ERR("occ_irq_setup: ssx_irq_handler_set(OCCHW_IRQ_PMC_ERROR) failed with rc=0x%08x", l_rc);
            break;
        }

        //enable the IRQ
        ssx_irq_status_clear(OCCHW_IRQ_PMC_ERROR);
        ssx_irq_enable(OCCHW_IRQ_PMC_ERROR);
END TEMP */
    }while(0);

    if(l_rc)
    {
        //single error for all error cases, just look at trace to see where it failed.
        /* @
         * @moduleid   OCC_IRQ_SETUP
         * @reasonCode SSX_GENERIC_FAILURE
         * @severity   ERRL_SEV_UNRECOVERABLE
         * @userdata1  SSX return code
         * @userdata4  OCC_NO_EXTENDED_RC
         * @devdesc    Firmware failure initializing IRQ
         */
        l_err = createErrl( OCC_IRQ_SETUP,             // i_modId,
                            SSX_GENERIC_FAILURE,       // i_reasonCode,
                            OCC_NO_EXTENDED_RC,
                            ERRL_SEV_UNRECOVERABLE,
                            NULL,                      // tracDesc_t i_trace,
                            0,                         // i_traceSz,
                            l_rc,                      // i_userData1,
                            0);                        // i_userData2

        //Callout firmware
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        commitErrl(&l_err);
    }
}

/*
 * Function Specification
 *
 * Name: gpe_reset
 *
 * Description: Force a GPE to start executing instructions at the reset vector
 *
 * End Function Specification
 */
void gpe_reset(uint32_t instance_id)
{
#define GPE0_XCR            0xC0000080
#define XCR_CMD_HRESET      0x60000000
#define XCR_CMD_TOGGLE_XSR  0x40000000
#define XCR_CMD_RESUME      0x20000000
#define GPE_XCR_ADDR(inst) (GPE0_XCR + (0x10000 * instance_id))


    out32(GPE_XCR_ADDR(instance_id), XCR_CMD_HRESET);
    out32(GPE_XCR_ADDR(instance_id), XCR_CMD_TOGGLE_XSR);
    out32(GPE_XCR_ADDR(instance_id), XCR_CMD_TOGGLE_XSR);
    out32(GPE_XCR_ADDR(instance_id), XCR_CMD_RESUME);
}

/*
 * Function Specification
 *
 * Name: occ_ipc_setup
 *
 * Description: Initialzes IPC (Inter Process Communication) that is used
 *              to communicate with GPEs.
 *              This will also start GPE0 and GPE1.
 * NOTE:  SGPE and PGPE are started prior to the OCC 405 during the IPL.
 *
 * End Function Specification
 */
void occ_ipc_setup()
{
    int         l_rc;
    errlHndl_t  l_err;

    do
    {
        // install our IPC interrupt handler (this disables our cbufs)
        l_rc = ipc_init();
        if(l_rc)
        {
            MAIN_TRAC_ERR("ipc_init failed with rc=0x%08x", l_rc);
            break;
        }

        // enable IPC's
        l_rc = ipc_enable();
        if(l_rc)
        {
            MAIN_TRAC_ERR("ipc_enable failed with rc = 0x%08x", l_rc);
            break;
        }

        MAIN_TRAC_INFO("Calling IPC disable on all GPE's");
        // disable all of the GPE cbufs.  They will enable them once
        // they are ready to communicate.
        ipc_disable(OCCHW_INST_ID_GPE0);
        ipc_disable(OCCHW_INST_ID_GPE1);

        MAIN_TRAC_INFO("IPC initialization completed");

        // start GPE's 0 and 1
        MAIN_TRAC_INFO("Starting GPE0");
        gpe_reset(OCCHW_INST_ID_GPE0);

        MAIN_TRAC_INFO("Starting GPE1");
        gpe_reset(OCCHW_INST_ID_GPE1);

        MAIN_TRAC_INFO("GPE's taken out of reset");

    }while(0);

    if(l_rc)
    {
        // Log single error for all error cases, just look at trace to see where it failed.
        /* @
         * @moduleid   OCC_IPC_SETUP
         * @reasonCode IPC_GENERIC_FAILURE
         * @severity   ERRL_SEV_UNRECOVERABLE
         * @userdata1  IPC return code
         * @userdata4  OCC_NO_EXTENDED_RC
         * @devdesc    Firmware failure initializing IPC
         */
        l_err = createErrl( OCC_IRQ_SETUP,             // i_modId,
                            SSX_GENERIC_FAILURE,       // i_reasonCode,
                            OCC_NO_EXTENDED_RC,
                            ERRL_SEV_UNRECOVERABLE,
                            NULL,                      // tracDesc_t i_trace,
                            0,                         // i_traceSz,
                            l_rc,                      // i_userData1,
                            0);                        // i_userData2

        //Callout firmware
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        commitErrl(&l_err);
    }
}



/*
 * Function Specification
 *
 * Name: hmon_routine
 *
 * Description: Runs various routines that check the health of the OCC
 *
 * End Function Specification
 */
void hmon_routine()
{
/* TEMP -- NOT SUPPORTED IN PHASE1 */
#if 0
    static uint32_t L_critical_phantom_count = 0;
    static uint32_t L_noncritical_phantom_count = 0;
    static bool L_c_phantom_logged = FALSE;
    static bool L_nc_phantom_logged = FALSE;
    bool l_log_phantom_error = FALSE;

    //use MAIN debug traces
    MAIN_DBG("HMON routine processing...");

    //Check if we've had any phantom interrupts
    if(L_critical_phantom_count != G_occ_phantom_critical_count)
    {
        L_critical_phantom_count = G_occ_phantom_critical_count;
        MAIN_TRAC_INFO("hmon_routine: critical phantom irq occurred! count[%d]", L_critical_phantom_count);

        //log a critical phantom error once
        if(!L_c_phantom_logged)
        {
            L_c_phantom_logged = TRUE;
            l_log_phantom_error = TRUE;
        }
    }
    if(L_noncritical_phantom_count != G_occ_phantom_noncritical_count)
    {
        L_noncritical_phantom_count = G_occ_phantom_noncritical_count;
        MAIN_TRAC_INFO("hmon_routine: non-critical phantom irq occurred! count[%d]", L_noncritical_phantom_count);

        //log a non-critical phantom error once
        if(!L_nc_phantom_logged)
        {
            L_nc_phantom_logged = TRUE;
            l_log_phantom_error = TRUE;
        }
    }

    if(l_log_phantom_error)
    {
        /* @
         * @errortype
         * @moduleid    HMON_ROUTINE_MID
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   critical count
         * @userdata2   non-critical count
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     interrupt with unknown source was detected
         */
        errlHndl_t l_err = createErrl(HMON_ROUTINE_MID,             //modId
                                      INTERNAL_FAILURE,             //reasoncode
                                      OCC_NO_EXTENDED_RC,           //Extended reason code
                                      ERRL_SEV_INFORMATIONAL,       //Severity
                                      NULL,                         //Trace Buf
                                      DEFAULT_TRACE_SIZE,           //Trace Size
                                      L_critical_phantom_count,     //userdata1
                                      L_noncritical_phantom_count); //userdata2
        // Commit Error
        commitErrl(&l_err);
    }

    //if we are in activate state, then monitor the processor temperature for
    //timeout conditions and the processor VRHOT signal.
    if (IS_OCC_STATE_ACTIVE())
    {
        amec_health_check_proc_timeout();
        amec_health_check_proc_vrhot();
    }

    //if we are in active state with memory temperature data being collected
    //then monitor the temperature collections for overtemp and timeout conditions
    if(IS_OCC_STATE_ACTIVE() &&
       rtl_task_is_runnable(TASK_ID_CENTAUR_DATA))
    {
        amec_health_check_cent_timeout();
        amec_health_check_cent_temp();
        amec_health_check_dimm_timeout();
        amec_health_check_dimm_temp();
    }
#endif
}



/*
 * Function Specification
 *
 * Name: master_occ_init
 *
 * Description: Master OCC specific initialization.
 *
 * End Function Specification
 */
void master_occ_init()
{

    errlHndl_t l_err = NULL;

    l_err = initialize_apss();

    if( (NULL != l_err))
    {
        MAIN_TRAC_ERR("master_occ_init: Error initializing APSS");
        // commit & delete. CommitErrl handles NULL error log handle
        REQUEST_RESET(l_err);
    }

    // Reinitialize the PBAX Queues
    // TEMP -- NO DCOM YET
    //dcom_initialize_pbax_queues();
}

/*
 * Function Specification
 *
 * Name: slave_occ_init
 *
 * Description: Slave OCC specific initialization.
 *
 * End Function Specification
 */
void slave_occ_init()
{
    // Init the DPSS oversubscription IRQ handler
    MAIN_DBG("Initializing Oversubscription IRQ...");
    // TEMP -- NO DPSS/ERRL YET
/*
    errlHndl_t l_errl = dpss_oversubscription_irq_initialize();

    if( l_errl )
    {
        // Trace and commit error
        MAIN_TRAC_ERR("Initialization of Oversubscription IRQ handler failed");

        // commit log ... log should be deleted by reader mechanism
        // TEMP -- NO ERRL YET
        //commitErrl( &l_errl );
    }
    else
    {
        MAIN_TRAC_INFO("Oversubscription IRQ initialized");
    }
*/
    //Set up doorbell queues
    // TEMP -- NO DCOM YET
    //dcom_initialize_pbax_queues();

    // Run AMEC Slave Init Code
    amec_slave_init();

// @TODO - TEMP: SMGR not ready yet
/*
    // Initialize SMGR State Semaphores
    extern SsxSemaphore G_smgrModeChangeSem;
    ssx_semaphore_create(&G_smgrModeChangeSem, 1, 1);

    // Initialize SMGR Mode Semaphores
    extern SsxSemaphore G_smgrStateChangeSem;
    ssx_semaphore_create(&G_smgrStateChangeSem, 1, 1);
*/
}

/*
 * Function Specification
 *
 * Name: mainThrdTimerCallback
 *
 * Description: Main thread timer to post semaphores handled by main thread
 *
 * End Function Specification
 */
void mainThrdTimerCallback(void * i_argPtr)
{
    int l_rc = SSX_OK;
    do
    {
        // Post Thermal semaphore
        l_rc = ssx_semaphore_post( &G_thrmSem );

        if ( l_rc != SSX_OK )
        {
            MAIN_TRAC_ERR("Failure posting thermal semaphore: rc: 0x%x", l_rc);
            break;
        }

        MAIN_DBG("posted thrmSem");

        // Post health monitor semaphore
        l_rc = ssx_semaphore_post( &G_hmonSem );

        if ( l_rc != SSX_OK )
        {
            MAIN_TRAC_ERR("Failure posting HlTH monitor semaphore: rc: 0x%x", l_rc);
            break;
        }

        MAIN_DBG("posted hmonSem");

        // Post FFDC semaphore
        l_rc = ssx_semaphore_post( &G_ffdcSem );

        if ( l_rc != SSX_OK )
        {
            MAIN_TRAC_ERR("Failure posting FFDC semaphore: rc: 0x%x", l_rc);
            break;
        }

        MAIN_DBG("posted ffdcSem");

    }while(FALSE);

    // create error on failure posting semaphore
    if( l_rc != SSX_OK)
    {
        /* @
         * @errortype
         * @moduleid    MAIN_THRD_TIMER_MID
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   Create thermal semaphore rc
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     SSX semaphore related failure
         */

        errlHndl_t l_err = createErrl(MAIN_THRD_TIMER_MID,          //modId
                                      SSX_GENERIC_FAILURE,          //reasoncode
                                      OCC_NO_EXTENDED_RC,           //Extended reason code
                                      ERRL_SEV_UNRECOVERABLE,       //Severity
                                      NULL,                         //Trace Buf
                                      DEFAULT_TRACE_SIZE,           //Trace Size
                                      l_rc,                         //userdata1
                                      0);                           //userdata2

        // Commit Error
        // TEMP - NO RESET YET
        REQUEST_RESET(l_err);
    }
}

/*
 * Function Specification
 *
 * Name: initMainThrdSemAndTimer
 *
 * Description: Helper function to create semaphores handled by main thread. It also
 *              creates and schedules timer used for posting main thread semaphores
 *
 *
 * End Function Specification
 */
void initMainThrdSemAndTimer()
{
    // create the thermal Semaphore, starting at 0 with a max count of 0
    // NOTE: Max count of 0 is used becuase there is possibility that
    // semaphore can be posted more than once without any semaphore activity
    int l_thrmSemRc = ssx_semaphore_create(&G_thrmSem, 0, 0);
    // create the health monitor Semaphore, starting at 0 with a max count of 0
    // NOTE: Max count of 0 is used becuase there is possibility that
    // semaphore can be posted more than once without any semaphore activity
    int l_hmonSemRc = ssx_semaphore_create(&G_hmonSem, 0, 0);
    // create FFDC Semaphore, starting at 0 with a max count of 0
    // NOTE: Max count of 0 is used becuase there is possibility that
    // semaphore can be posted more than once without any semaphore activity
    int l_ffdcSemRc = ssx_semaphore_create(&G_ffdcSem, 0, 0);
    //create main thread timer
    int l_timerRc = ssx_timer_create(&G_mainThrdTimer,mainThrdTimerCallback,0);

    //check for errors creating the timer
    if(l_timerRc == SSX_OK)
    {
        //schedule the timer so that it runs every MAIN_THRD_TIMER_SLICE
        l_timerRc = ssx_timer_schedule(&G_mainThrdTimer,      // Timer
                                       1,                     // time base
                                       MAIN_THRD_TIMER_SLICE);// Timer period
    }
    else
    {
        MAIN_TRAC_ERR("Error creating main thread timer: RC: %d", l_timerRc);
    }

    // Failure creating semaphore or creating/scheduling timer, create
    // and log error.
    if (( l_thrmSemRc != SSX_OK ) ||
        ( l_hmonSemRc != SSX_OK ) ||
        ( l_ffdcSemRc != SSX_OK ) ||
        ( l_timerRc != SSX_OK))
    {
        MAIN_TRAC_ERR("Semaphore/timer create failure: thrmSemRc: 0x%08x, "
                 "hmonSemRc: 0x08%x, ffdcSemRc: 0x%08x, l_timerRc: 0x%08x",
                 -l_thrmSemRc,-l_hmonSemRc,-l_ffdcSemRc, l_timerRc );

        /* @
         * @errortype
         * @moduleid    MAIN_THRD_SEM_INIT_MID
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   Create thermal semaphore rc
         * @userdata2   Timer create/schedule rc
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     SSX semaphore related failure
         */

        errlHndl_t l_err = createErrl(MAIN_THRD_SEM_INIT_MID,       //modId
                                      SSX_GENERIC_FAILURE,          //reasoncode
                                      OCC_NO_EXTENDED_RC,           //Extended reason code
                                      ERRL_SEV_UNRECOVERABLE,       //Severity
                                      NULL,                         //Trace Buf
                                      DEFAULT_TRACE_SIZE,           //Trace Size
                                      l_thrmSemRc,                  //userdata1
                                      l_timerRc);                   //userdata2

        REQUEST_RESET(l_err);
    }
}

/*
 * Function Specification
 *
 * Name: Main_thread_routine
 *
 * Description: Main thread handling OCC initialization and thernal, health
 *              monitor and FFDC function semaphores
 *
 * End Function Specification
 */
void Main_thread_routine(void *private)
{
    CHECKPOINT(MAIN_THREAD_STARTED);

    MAIN_TRAC_INFO("Main Thread Started ... " );


    // NOTE: At present, we are not planning to use any config data from
    // mainstore. OCC Role will be provided by FSP after FSP communication
    // So instead of doing config_data_init, we will directly use
    // dcom_initialize_roles. If in future design changes, we will make
    // change to use config_data_init at that time.
    // Default role initialization and determine OCC/Chip Id

    // TEMP -- NO DCOM YET, init as OCC Master
    //dcom_initialize_roles();

    // Remove the next LOC when dcom_initialize_roles() is un-commented
    G_occ_role = OCC_MASTER;  // TEMP - @TODO

        // Remove the next 2 LOC when dcom_initialize_roles is un-commented
        // AND cmdh is running to call master_occ_init
        rtl_set_run_mask(RTL_FLAG_MSTR);
      master_occ_init();

    CHECKPOINT(ROLES_INITIALIZED);

    // Sensor Initialization
    // All Master & Slave Sensor are initialized here, it is up to the
    // rest of the firmware if it uses them or not.
    sensor_init_all();
    CHECKPOINT(SENSORS_INITIALIZED);

    // SPIVID Initialization must be done before Pstates
    // All SPIVID inits are done by Hostboot, remove this section.

    //Initialize structures for collecting core data.
    //It needs to run before RTLoop start as pore initialization needs to be
    // done before task to collect core data starts.
    proc_core_init();
    CHECKPOINT(PROC_CORE_INITIALIZED);

    // Run slave OCC init on all OCCs. Master-only initialization will be
    // done after determining actual role. By default all OCCs are slave.
    slave_occ_init();
    CHECKPOINT(SLAVE_OCC_INITIALIZED);

    // Initialize watchdog timers. This needs to be right before
    // start rtl to make sure timer doesn't timeout. This timer is being
    // reset from the rtl task.
// TEMP -- watchdog timers not enabled yet
//    MAIN_TRAC_INFO("Initializing watchdog timers.");
//    initWatchdogTimers();
    CHECKPOINT(WATCHDOG_INITIALIZED);

    // Initialize Real time Loop Timer Interrupt
    rtl_ocb_init();
    CHECKPOINT(RTL_TIMER_INITIALIZED);

    // Initialize semaphores and timer for handling thermal, health monitor and
    // FFDC functions.
    initMainThrdSemAndTimer();
    CHECKPOINT(SEMS_AND_TIMERS_INITIALIZED);

    //Initialize the thread scheduler.
    //Other thread initialization is done here so that don't have to handle
    // blocking commnad handler thread as FSP might start communicating
    // through cmd handler thread in middle of the initialization.
    initThreadScheduler();

    int l_ssxrc = SSX_OK;
    // initWatchdogTimers called before will start running the timer but
    // the interrupt handler will just restart the timer until we use this
    // enable switch to actually start the watchdog function.
//    ENABLE_WDOG;

// TEMP: Normally these flags are set elsewhere, after the BMC/FSP
//       send us configuration data. This is a temporary hack until
//       that communication is enabled. Required for APSS tasks.
    rtl_set_run_mask(RTL_FLAG_OBS);
    rtl_clr_run_mask(RTL_FLAG_STANDBY);
    rtl_clr_run_mask(RTL_FLAG_APSS_NOT_INITD);
    rtl_clr_run_mask(RTL_FLAG_RST_REQ);
// END TEMP
    while (TRUE)
    {
        // Count each loop so the watchdog can tell the main thread is
        // running.
        G_mainThreadLoopCounter++;

        // Flush the loop counter and trace buffers on each loop, this makes
        // debug easier if the cmd interface doesn't respond
        dcache_flush_line(&G_mainThreadLoopCounter);
        dcache_flush(g_trac_inf_buffer, TRACE_BUFFER_SIZE);
        dcache_flush(g_trac_imp_buffer, TRACE_BUFFER_SIZE);
        dcache_flush(g_trac_err_buffer, TRACE_BUFFER_SIZE);

        // Wait for thermal semaphore
        l_ssxrc = ssx_semaphore_pend(&G_thrmSem,SSX_WAIT_FOREVER);
/* TEMP -- FIR DATA IS NOT SUPPORTED IN PHASE1
        static bool L_fir_collection_completed = FALSE;
        // Look for FIR collection flag and status
        if (G_fir_collection_required && !L_fir_collection_completed)
        {
            // If this OCC is the FIR master and PNOR access is allowed perform
            // FIR collection
            if (OCC_IS_FIR_MASTER() && pnor_access_allowed())
            {
                fir_data_collect();
                L_fir_collection_completed = TRUE;
            }

            G_fir_collection_required = FALSE;
            // Error reporting is skipped while FIR collection is required so we
            // don't get reset in flight.  If anyone is listening send the
            // error alert now.
            // If this system is using PSIHB complex, send an interrupt to Host so that
            // Host can inform HTMGT to collect the error log
            if (G_occ_interrupt_type == PSIHB_INTERRUPT)
            {
                // From OCC OpenPower Interface v1.1, OCC needs to set bits 0 and 1 of
                // the OCB_OCCMISC register
                ocb_occmisc_t l_occmiscreg;
                l_occmiscreg.fields.core_ext_intr = 1;
                l_occmiscreg.fields.reason_intr = 1;
                out32(OCB_OCCMISC_OR, l_occmiscreg.value);
            }
        }
*/
        if ( l_ssxrc != SSX_OK )
        {
            MAIN_TRAC_ERR("thermal Semaphore pending failure RC[0x%08X]", -l_ssxrc );
        }
        else
        {
            // For Simics phase 1, we don't want to call the thermal thread
            // Call thermal routine that executes fan control
            //thrm_thread_main();
        }

        if( l_ssxrc == SSX_OK)
        {
            // Wait for health monitor semaphore
            l_ssxrc = ssx_semaphore_pend(&G_hmonSem,SSX_WAIT_FOREVER);

            if( l_ssxrc != SSX_OK)
            {
                MAIN_TRAC_ERR("health monitor Semaphore pending failure RC[0x%08X]",
                         -l_ssxrc );
            }
            else
            {
                // For Simics phase 1, we don't want to call the health monitor thread
                // call health monitor routine
                //hmon_routine();
            }
        }

        if( l_ssxrc == SSX_OK)
        {
            // Wait for FFDC semaphore
            l_ssxrc = ssx_semaphore_pend(&G_ffdcSem,SSX_WAIT_FOREVER);

            if( l_ssxrc != SSX_OK)
            {
                MAIN_TRAC_ERR("FFDC Semaphore pending failure RC[0x%08X]",-l_ssxrc );
            }
            else
            {
                // For Simics phase 1, we don't want to call the call-home thread
                // Only Master OCC will log call home data
                if (OCC_MASTER == G_occ_role)
                {
                    //chom_main();
                }
            }
        }

        if( l_ssxrc != SSX_OK)
        {
            /* @
             * @errortype
             * @moduleid    MAIN_THRD_ROUTINE_MID
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   semaphore pending return code
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     SSX semaphore related failure
             */

            errlHndl_t l_err = createErrl(MAIN_THRD_ROUTINE_MID,        //modId
                                          SSX_GENERIC_FAILURE,          //reasoncode
                                          OCC_NO_EXTENDED_RC,           //Extended reason code
                                          ERRL_SEV_UNRECOVERABLE,       //Severity
                                          NULL,                         //Trace Buf
                                          DEFAULT_TRACE_SIZE,           //Trace Size
                                          -l_ssxrc,                     //userdata1
                                          0);                           //userdata2

            REQUEST_RESET(l_err);
        }
    } // while loop
}

/*
 * Function Specification
 *
 * Name: main
 *
 * Description: Entry point of the OCC application
 *
 * End Function Specification
 */
int main(int argc, char **argv)
{
    int l_ssxrc = 0;
    int l_ssxrc2 = 0;

    // ----------------------------------------------------
    // Initialize TLB for Linear Window access here so we
    // can write checkpoints into the fsp response buffer.
    // ----------------------------------------------------

#if PPC405_MMU_SUPPORT
    l_ssxrc = ppc405_mmu_map(
            CMDH_OCC_RESPONSE_BASE_ADDRESS,
            CMDH_OCC_RESPONSE_BASE_ADDRESS,
            CMDH_FSP_RSP_SIZE,
            0,
            TLBLO_WR | TLBLO_I,
            NULL
            );

    if(l_ssxrc != SSX_OK)
    {
        //failure means we can't talk to FSP.
        SSX_PANIC(0x01000001);
    }

    l_ssxrc = ppc405_mmu_map(
            CMDH_LINEAR_WINDOW_BASE_ADDRESS,
            CMDH_LINEAR_WINDOW_BASE_ADDRESS,
            CMDH_FSP_CMD_SIZE,
            0,
            TLBLO_I,
            NULL
            );

    if(l_ssxrc != SSX_OK)
    {
        //failure means we can't talk to FSP.
        SSX_PANIC(0x01000002);
    }
#endif /* PPC405_MMU_SUPPORT */

/* TEMP -- NO FIR SUPPORT IN PHASE1
    // Setup the TLB for writing to the FIR parms section
    l_ssxrc = ppc405_mmu_map(FIR_PARMS_SECTION_BASE_ADDRESS,
                             FIR_PARMS_SECTION_BASE_ADDRESS,
                             FIR_PARMS_SECTION_SIZE,
                             0,
                             TLBLO_WR | TLBLO_I,
                             NULL);

    if (l_ssxrc != SSX_OK)
    {
        // Panic, this section is required for FIR collection on checkstops
        SSX_PANIC(0x01000003);
    }

    // Setup the TLB for writing to the FIR heap section
    l_ssxrc = ppc405_mmu_map(FIR_HEAP_SECTION_BASE_ADDRESS,
                             FIR_HEAP_SECTION_BASE_ADDRESS,
                             FIR_HEAP_SECTION_SIZE,
                             0,
                             TLBLO_WR | TLBLO_I,
                             NULL);

    if (l_ssxrc != SSX_OK)
    {
        // Panic, this section is required for FIR collection on checkstops
        SSX_PANIC(0x01000004);
    }
*/
    CHECKPOINT_INIT();
    CHECKPOINT(MAIN_STARTED);

    homer_rc_t l_homerrc = HOMER_SUCCESS;
    homer_rc_t l_homerrc2 = HOMER_SUCCESS;

    // Get the homer version
    uint32_t l_homer_version = 0;
    //l_homerrc = homer_hd_map_read_unmap(HOMER_VERSION,
    //                                    &l_homer_version,
    //                                    &l_ssxrc);

    //if ((HOMER_SUCCESS != l_homerrc) && (HOMER_SSX_UNMAP_ERR != l_homerrc))
    {
        // Attempt to use max version if we can't read the homer.
        //l_homer_version = HOMER_VERSION_MAX;

        // For Simics, assume Homer version 2 (no FIR support)
        l_homer_version = HOMER_VERSION_2;
    }

    // Get proc_pb_frequency from HOMER host data and calculate the timebase
    // frequency for the OCC. Pass the timebase frequency to ssx_initialize.
    // The passed value must be in Hz. The occ 405 runs at 1/4 the proc
    // frequency so the passed value is 1/4 of the proc_pb_frequency from the
    // HOMER, ie. if the MRW says that proc_pb_frequency is 2400 MHz, then
    // pass 600000000 (600MHz)

    // The offset from the start of the HOMER is 0x00100000, we will need to
    // create a temporary mapping to this section of the HOMER with ppc405_mmu_map
    // (at address 0x00000000) read the value, convert it, and then unmap.

    // Don't do a version check before reading the nest freq, it's present in
    // all HOMER versions.
    uint32_t l_tb_freq_hz = 0;
    //l_homerrc2 = homer_hd_map_read_unmap(HOMER_NEST_FREQ,
    //                                     &l_tb_freq_hz,
    //                                     &l_ssxrc2);

    //if ((HOMER_SUCCESS == l_homerrc2) || (HOMER_SSX_UNMAP_ERR == l_homerrc2))
    if(0)
    {
        // Data is in Mhz upon return and needs to be converted to Hz and then
        // quartered.
        l_tb_freq_hz = (l_tb_freq_hz * 1000000)/4;

        // @TODO: this parameter should be passsed to all the GPEs/CMEs/etc
        // Can be stored in GPE accessible SRAM areas.
        // The whole OCC complex should be running at the same nest frequency (proc_freq/4).
    }
    else
    {
        l_tb_freq_hz = PPC405_TIMEBASE_HZ;
    }

    CHECKPOINT(SSX_STARTING);

    // Initialize SSX Stacks.  This also reinitializes the time base to 0
    ssx_initialize((SsxAddress)G_noncritical_stack,
                   NONCRITICAL_STACK_SIZE,
                   (SsxAddress)G_critical_stack,
                   CRITICAL_STACK_SIZE,
                   0,
                   l_tb_freq_hz);

    CHECKPOINT(SSX_INITIALIZED);

    // TRAC_XXX needs ssx services, traces can only be done after ssx_initialize
    TRAC_init_buffers();

    CHECKPOINT(TRACE_INITIALIZED);

    MAIN_TRAC_INFO("Inside OCC Main");
    // Trace what happened before ssx initialization
    MAIN_TRAC_INFO("HOMER accessed, rc=%d, version=%d, ssx_rc=%d",
              l_homerrc, l_homer_version, l_ssxrc);

    MAIN_TRAC_INFO("HOMER accessed, rc=%d, nest_freq=%d, ssx_rc=%d",
              l_homerrc2, l_tb_freq_hz, l_ssxrc2);

    // Handle any errors from the version access
    homer_log_access_error(l_homerrc,
                           l_ssxrc,
                           l_homer_version);

    // Handle any errors from the nest freq access
    homer_log_access_error(l_homerrc2,
                           l_ssxrc2,
                           l_tb_freq_hz);

    // Time to access any initialization data needed from the HOMER (besides the
    // nest frequency which was required above to enable SSX and tracing).
    CHECKPOINT(HOMER_ACCESS_INITS);

    if (l_homer_version >= HOMER_VERSION_2)
    {
        // Get OCC interrupt type from HOMER host data area. This will tell OCC
        // which interrupt to Host it should be using.
        uint32_t l_occ_int_type = 0;
        //l_homerrc = homer_hd_map_read_unmap(HOMER_INT_TYPE,
        //                                    &l_occ_int_type,
        //                                    &l_ssxrc);

        //if ((HOMER_SUCCESS == l_homerrc) || (HOMER_SSX_UNMAP_ERR == l_homerrc))
        if(0)
        {
            G_occ_interrupt_type = (uint8_t) l_occ_int_type;
        }
        else
        {
            // For Simics we are assuming the BMC communication path
            //G_occ_interrupt_type = FSP_SUPPORTED_OCC;
            G_occ_interrupt_type = PSIHB_INTERRUPT;
        }

        MAIN_TRAC_INFO("HOMER accessed, rc=%d, host interrupt type=%d, ssx_rc=%d",
                  l_homerrc, l_occ_int_type, l_ssxrc);

        // Handle any errors from the interrupt type access
        homer_log_access_error(l_homerrc,
                               l_ssxrc,
                               l_occ_int_type);
    }
/*
    //TEMP -- NO FIR SUPPORT
    if (l_homer_version >= HOMER_VERSION_3)
    {
        // Get the FIR Master indicator
        uint32_t l_fir_master = FIR_OCC_NOT_FIR_MASTER;
        l_homerrc = homer_hd_map_read_unmap(HOMER_FIR_MASTER,
                                            &l_fir_master,
                                            &l_ssxrc);

        if (((HOMER_SUCCESS == l_homerrc) || (HOMER_SSX_UNMAP_ERR == l_homerrc))
            &&
            (FIR_OCC_IS_FIR_MASTER == l_fir_master))
        {
            OCC_SET_FIR_MASTER(FIR_OCC_IS_FIR_MASTER);
        }
        else
        {
            OCC_SET_FIR_MASTER(FIR_OCC_NOT_FIR_MASTER);
        }

        MAIN_TRAC_INFO("HOMER accessed, rc=%d, FIR master=%d, ssx_rc=%d",
                  l_homerrc, l_fir_master, l_ssxrc);

        // Handle any errors from the FIR master access
        homer_log_access_error(l_homerrc,
                               l_ssxrc,
                               l_fir_master);

        // If this OCC is the FIR master read in the FIR collection parms
        if (OCC_IS_FIR_MASTER())
        {
            MAIN_TRAC_IMP("I am the FIR master");

            // Read the FIR parms buffer
            l_homerrc = homer_hd_map_read_unmap(HOMER_FIR_PARMS,
                                                &G_fir_data_parms[0],
                                                &l_ssxrc);

            MAIN_TRAC_INFO("HOMER accessed, rc=%d, FIR parms buffer 0x%x, ssx_rc=%d",
                      l_homerrc, &G_fir_data_parms[0], l_ssxrc);

            // Handle any errors from the FIR master access
            homer_log_access_error(l_homerrc,
                                   l_ssxrc,
                                   (uint32_t)&G_fir_data_parms[0]);
        }
    }
*/
    // enable and register additional interrupt handlers
    CHECKPOINT(INITIALIZING_IRQS);

//  TODO: Uncomment when this is resolved in Simics. Causes SSX panic
//        currently. Not needed until we want to be able to catch
//        hardware OCC or PMC (or equivalent) errors.
#if !SIMICS_ENVIRONMENT
    occ_irq_setup();
#endif

    CHECKPOINT(IRQS_INITIALIZED);

    // enable IPC and start GPEs
    CHECKPOINT(INITIALIZING_IPC);

    occ_ipc_setup();

    CHECKPOINT(IPC_INITIALIZED);



    // Create and resume main thread
    int l_rc = createAndResumeThreadHelper(&Main_thread,
                                           Main_thread_routine,
                                           (void *)0,
                                           (SsxAddress)main_thread_stack,
                                           THREAD_STACK_SIZE,
                                           THREAD_PRIORITY_2);


    if( SSX_OK != l_rc)
    {
        MAIN_TRAC_ERR("Failure creating/resuming main thread: rc: 0x%x", -l_rc);
        /* @
         * @errortype
         * @moduleid    MAIN_MID
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   return code
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Firmware internal error creating thread
         */
        errlHndl_t l_err = createErrl(MAIN_MID,                 //modId
                                      SSX_GENERIC_FAILURE,      //reasoncode
                                      OCC_NO_EXTENDED_RC,       //Extended reason code
                                      ERRL_SEV_UNRECOVERABLE,   //Severity
                                      NULL,                     //Trace Buf
                                      DEFAULT_TRACE_SIZE,       //Trace Size
                                      -l_rc,                    //userdata1
                                      0);                       //userdata2
        // Commit Error log
        REQUEST_RESET(l_err);
    }

    // Enter SSX Kernel
    ssx_start_threads();

    return 0;
}

