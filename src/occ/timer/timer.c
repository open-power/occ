/******************************************************************************
// @file timer.c
// @brief OCC timer interfaces
*/

/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section timer.c TIMER.C
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @pb007             pbavari   09/27/2011  Created
 *   @pb008             pbavari   10/04/2011  Combined watchdog reset tasks
 *   @pb009             pbavari   10/20/2011  Main thread support
 *   @rc001             rickylie  01/02/2012  Change TRAC_DBG to TMER_DBG
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @nh001             neilhsu   05/23/2012  Add missing error log tags
 *   @sb002  908891     sbroyles  12/09/2013  FFDC updates
 *   @sb022  910404     sbroyles  01/06/2014  Extend watchdog timeout
 *   @sb012  910394     sbroyles  01/10/2014  More FFDC updates
 *   @sb023  913769     sbroyles  02/08/2014  Clear WIS on each interrupt
 *   @wb001  919163     wilbryan  03/06/2014  Updating error call outs, descriptions, and severities
 *  @endverbatim
 *
 *///*************************************************************************/

//*************************************************************************
// Includes
//*************************************************************************
#include <timer.h>                  // timer defines
#include "ssx.h"
#include <trac.h>                   // Trace macros
#include <pgp_common.h>             // PGP common defines
#include <pgp_ocb.h>                // OCB timer interfaces
#include <occ_service_codes.h>      // Reason codes
#include <timer_service_codes.h>    // Module Id
#include <cmdh_fsp.h>               // @sb002 for RCs in the checkpoint macros

//*************************************************************************
// Externs
//*************************************************************************
// @sb022 Variable holding main thread loop count
extern uint32_t G_mainThreadLoopCounter;

//*************************************************************************
// Macros
//*************************************************************************
// PPC405 watchdog timer handler
SSX_IRQ_FAST2FULL(ppc405WDTHndler, ppc405WDTHndlerFull);
// OCB timer handler
SSX_IRQ_FAST2FULL(ocbTHndler, ocbTHndlerFull);

//*************************************************************************
// Defines/Enums
//*************************************************************************
// @sb023 Change watchdog reset control to take no action on state TSR[WIS]=1
// and TSR[ENW]=1
// Watchdog reset control set to "No reset"
#define OCC_TCR_WRC 0
// @sb002 Bump up wdog period to ~1s
#define OCC_TCR_WP 3
// 4ms represented in nanoseconds
#define OCB_TIMER_TIMOUT    4000000
// @sb022 Removed old watchdog code

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
// @sb022
bool G_wdog_enabled = FALSE;

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

// Function Specification
//
// Name: initWatchdogTimers
//
// Description:
//
// Flow:              FN=None
//
// End Function Specification
void initWatchdogTimers()
{
    int l_rc = SSX_OK;
    errlHndl_t l_err = NULL;

    TRAC_IMP("Initializing ppc405 watchdog. period=%d, reset_ctrl=%d",
             OCC_TCR_WP,
             OCC_TCR_WRC);

    // set up PPC405 watchdog timer
    l_rc = ppc405_watchdog_setup(OCC_TCR_WP,          // watchdog period
                                 OCC_TCR_WRC,         // watchdog reset control
                                 ppc405WDTHndler,     // interrupt handler
                                 NULL);               // argument to handler

    if (SSX_OK != l_rc)
    {
        TRAC_ERR("Error setting up ppc405 watchdog timer: l_rc: %d",l_rc);
        /*
         * @errortype
         * @moduleid    INIT_WD_TIMERS
         * @reasoncode  INTERNAL_HW_FAILURE
         * @userdata1   Return code of PPC405 watchdog timer setup
         * @userdata4   ERC_PPC405_WD_SETUP_FAILURE
         * @devdesc     Failure on hardware related function
         */
        l_err = createErrl(INIT_WD_TIMERS,                 // mod id
                           INTERNAL_HW_FAILURE,            // reason code      // @nh001c
                           ERC_PPC405_WD_SETUP_FAILURE,    // Extended reason code
                           ERRL_SEV_UNRECOVERABLE,         // severity
                           NULL,                           // trace buffer
                           0,                              // trace size
                           l_rc,                           // userdata1
                           0);                             // userdata2
                           
        // @wb001 -- Callout firmware
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);
                             
        // Commit error log
        commitErrl(&l_err);
    }

    // @sb002 Defer setting up OCB timer 1, all it does is time the real time
    // loop at this point
    // FIXME #timer_c_001 Review RTL timer requirements
    //// Set up OCB timer
    //TRAC_IMP("Initializing OCB timer. timer=%d, auto_reload=%d, timeout_us=%d",
    //         OCB_TIMER1,
    //         OCB_TIMER_ONE_SHOT,
    //         OCB_TIMER_TIMOUT/1000);
    //l_rc = ocb_timer_setup(OCB_TIMER1,              // ocb timer
    //                       OCB_TIMER_ONE_SHOT,      // no auto reload
    //                       OCB_TIMER_TIMOUT,        // timeout in ns
    //                       ocbTHndler,              // interrupt handler
    //                       NULL,                    // argument to handler
    //                       SSX_CRITICAL);           // interrupt priority

    if( SSX_OK != l_rc)
    {
        TRAC_ERR("Error setting up OCB timer: l_rc: %d",l_rc);
        /*
        * @errortype
        * @moduleid    INIT_WD_TIMERS
        * @reasoncode  INTERNAL_HW_FAILURE
        * @userdata1   Return code of OCB timer setup
        * @userdata4   ERC_OCB_WD_SETUP_FAILURE
        * @devdesc     Failure on hardware related function
        */
        l_err =  createErrl(INIT_WD_TIMERS,                 // mod id
                            INTERNAL_HW_FAILURE,            // reason code      // @nh001c
                            ERC_OCB_WD_SETUP_FAILURE,       // Extended reason code
                            ERRL_SEV_UNRECOVERABLE,         // severity
                            NULL,                           // trace buffer
                            0,                              // trace size
                            l_rc,                           // userdata1
                            0);                             // userdata2
                            
        // @wb001 -- Callout firmware
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);
                            
        // Commit error log
        commitErrl(&l_err);
    }
    // @sb002 Removed UDER set to halt on OCB timer expiration. Let timer
    // interrupt handler take over.
    // @sb002 Simplified error handling
}


// Function Specification
//
// Name: task_poke_watchdogs
//
// Description: Reset PPC405 watchdog timer by clearing TSR[ENW] bit and reset OCB timer
//
// Flow:              FN=None
//
// End Function Specification
void task_poke_watchdogs( struct task * i_self )
{
    // @sb002 Read the PMC status register on every RTL, this is how the OCC
    // generates a PMC hearbeat.
    pmc_status_reg_t psr;
    psr.value = in32(PMC_STATUS_REG);

    // @sb002 FIXME #timer_c_002 Review RTL timer requirements
    // Monitor the real time loop and make sure the poke task runs at
    // least once every 4ms.
    //static bool L_loggedOnce = FALSE;
    //int l_rc = SSX_OK;
    //// Reset OCB timer
    //l_rc = ocb_timer_reset(OCB_TIMER1,           // OCB Timer
    //                       OCB_TIMER_ONE_SHOT,   // no auto reload
    //                       OCB_TIMER_TIMOUT);    // timeout in ns
    // @sb002 Removed all the 405 wdog thread counter code, 405 wdog control
    // now in main thread loop
    //if (SSX_OK != l_rc)
    //{
    //    if (L_loggedOnce == FALSE)
    //    {
    //        TRAC_ERR("Error resetting OCB timer: l_rc: %d",l_rc);
    //        /*
    //        * @errortype
    //        * @moduleid    RESET_OCB_TIMER
    //        * @reasoncode  INTERNAL_HW_FAILURE
    //        * @userdata1   Return code of OCB timer reset
    //        * @userdata4   OCC_NO_EXTENDED_RC
    //        * @devdesc     Faiure on hardware related function
    //        */
    //        errlHndl_t l_err = createErrl(RESET_OCB_TIMER,            // mod id
    //                                      INTERNAL_HW_FAILURE,        // reason code    // @nh001c
    //                                      OCC_NO_EXTENDED_RC,         // Extended reason code
    //                                      ERRL_SEV_UNRECOVERABLE,     // severity
    //                                      NULL,                       // trace buf
    //                                      0,                          // trace size
    //                                      l_rc,                       // userdata1
    //                                      0);                         // userdata2
    //        // Commit error log
    //        commitErrl(&l_err);
    //        // Set flag to indicate already logged once
    //        L_loggedOnce = TRUE;
    //    }
    //}
}


// Function Specification
//
// Name: ppc405WDTHndlerFull
//
// Description: PPC405 watchdog interrupt handler
//
// Flow:              FN=None
//
// End Function Specification
void ppc405WDTHndlerFull(void * i_arg,SsxIrqId i_irq,int i_priority)
{
    // @sb002 Removed trace, causes data storage exception in interrupt context
    // @sb022 Small rewrite, added enable/disable switch, extend total time to
    // around 6 seconds
    static uint8_t l_wdog_intrpt_cntr = 0;

    // @sb023 Always reset the watchdog interrupt status in the TSR.  If we halt
    // and leave TSR[WIS]=1 then the watchdog counter will eventually set
    // TSR[ENW]=1 and upon expiration of the next watchdog period the 405 will
    // take whatever action is in TCR[WRC] potentially resetting the OCC while
    // we have it in a halted state, an undesirable outcome.
    // Always clear TSR[ENW,WIS] to reset the watchdog state machine.
    mtspr(SPRN_TSR, (TSR_ENW | TSR_WIS));

    if (WDOG_ENABLED)
    {
        // When enabled, always increment this local static counter
        l_wdog_intrpt_cntr++;
        // The hardware timer should be set to around a second, on the third
        // interrupt we go to halt if the main thread counter hasn't incremented
        // since the last time it was reset.
        if (l_wdog_intrpt_cntr == 3)
        {
            l_wdog_intrpt_cntr = 0;
            // The watchdog interrupt has fired three times, time to check the
            // state of the main thread by looking at the main thread loop
            // counter, it must be non-zero else we will halt the occ
            if (G_mainThreadLoopCounter > 0)
            {
                // The main thread has run at least once in the last ~6 seconds
                G_mainThreadLoopCounter = 0;
            }
            else
            {
                // @sb002 Added checkpoint and and halt macros
                // @sb012 Removed SPR set and __CHECKPOINT macros, use new
                // OCC_HALT with exception code passed in.
                OCC_HALT(ERRL_RC_WDOG_TIMER);
            }
        }
    }
}


// Function Specification
//
// Name: ocbTHndlerFull
//
// Description: OCB timer interrupt handler
//
// Flow:              FN=None
//
// End Function Specification
void ocbTHndlerFull(void * i_arg,SsxIrqId i_irq,int i_priority)
{
    // @sb002 Removed trace, causes data storage exception in interrupt context
    // @sb002 Added checkpoint and and halt macros
    // Enable external debug mode, checkpoint and halt
    // @sb012 Removed SPR set and __CHECKPOINT macros, use new
    // OCC_HALT with exception code passed in.
    OCC_HALT(ERRL_RC_OCB_TIMER);
}
