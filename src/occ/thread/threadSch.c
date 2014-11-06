/******************************************************************************
// @file threadSch.c
// @brief OCC THREAD COMPONENT Thread Scheduler methods
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section _threadSch_c threadSch.c
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      abagepa   07/15/2011  Created Thread Scheduler methods
 *   @pb004             pbavari   09/02/2011  Test applet thread support
 *   @pb009             pbavari   10/20/2011  Main thread support
 *   @th006             thallet   11/21/2011  RESET_REQUEST substituted for todo's
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @pb00E             pbavari   03/11/2012  Added correct include file
 *   @nh001             neilhsu   05/23/2012  Add missing error log tags
 *   @th022             thallet   10/03/2012  Added DCOM Thread for State/Mode setting
 *   @gm006  SW224414   milesg    09/16/2013  Reset and FFDC improvements 
 *   @fk004   907588    fmkassem  11/25/2013  Snapshot buffer support.
 *
 *  @endverbatim
 *
 *///*************************************************************************/

//*************************************************************************
// Includes
//*************************************************************************
//@pb00Ec - changed from common.h to occ_common.h for ODE support
#include <occ_common.h>
#include <threadSch.h>
#include "ssx.h"
#include "thread_service_codes.h"
#include "occ_service_codes.h"
#include <appletManager.h>
#include <trac.h>
#include <state.h>
#include "cmdh_snapshot.h"  // @fk004a


//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
//Numbers of threads to schedule
//@pb009c - Changed to use dynamic calculation
#define THREADS_TO_SCHEDULE (sizeof(G_scheduledThreads)/sizeof(SsxThread*))

//10 ms
#define THREAD_TIME_SLICE (SsxInterval) SSX_MILLISECONDS(10)

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
/* Thread Timer to reprioritize the threads */
SsxTimer G_threadSchTimer;

//Index of highest priority thread in G_scheduledThreads
uint16_t G_threadSchedulerIndex = 0;

/* Array that holds the threads that need scheduling */
//@pb009c - Removed not needed threads
SsxThread* G_scheduledThreads[] =
{
    &Main_thread,
    &Cmd_Hndl_thread,
    &App_thread,
    &TestAppletThread,
    &Dcom_thread,
};

//Error log counter for the callback
// so that only 1 error log is created
uint8_t G_threadSwapErrlCounter = 0;

// Global for the parameter to the applet thread routine
OCC_APLT_TYPE G_apltPdtType = APLT_TYPE_PRODUCT;
OCC_APLT_TYPE G_apltTestType = APLT_TYPE_TEST;

//Thread Stacks
uint8_t main_thread_stack[THREAD_STACK_SIZE];
uint8_t Cmd_hndl_thread_stack[THREAD_STACK_SIZE];
uint8_t App_thread_stack[THREAD_STACK_SIZE];
uint8_t testAppletThreadStack[THREAD_STACK_SIZE];
uint8_t dcomThreadStack[THREAD_STACK_SIZE];

/// Our idle thread.  See #main_thread_routine  
SsxThread Main_thread;

//Command handler thread
SsxThread Cmd_Hndl_thread;

//Application manager thread  
SsxThread App_thread;  

//@pb004a - Test applet thread
SsxThread TestAppletThread;

//@thXXXa - Dcom thread
SsxThread Dcom_thread;

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

// Function Specification
//
// Name: createAndResumeThreadHelper
//
// Description: create and resume thread helper 
//
// Flow:              FN=None
// 
// End Function Specification
int createAndResumeThreadHelper(SsxThread         *io_thread,
                                SsxThreadRoutine  i_thread_routine,
                                void              *io_arg,
                                SsxAddress        i_stack,
                                size_t            i_stack_size,
                                THREAD_PRIORITY   i_priority)
{

    //locals
    int l_rc = SSX_OK;

    //Thread creation
    l_rc =  ssx_thread_create(io_thread, 
                              i_thread_routine, 
                              io_arg,
                              i_stack,
                              i_stack_size,
                              (SsxThreadPriority)i_priority);

    //check for errors creating a thread 
    if(l_rc != SSX_OK)
    {
        TRAC_ERR("Failure creating thread. rc: 0x%x", -l_rc);
    }
    else
    {
        //resume thread once created
        l_rc = ssx_thread_resume(io_thread);
    }

    
    return l_rc;
}


// Function Specification
//
// Name:  initThreadScheduler
//
// Description: Init the threads in the scheduler and start the
//              timer.
//
// Flow:  10/31/11    FN=initThreadScheduler
//
// End Function Specification
// @pb009c - Removed not needed threads from the initialization
void initThreadScheduler(void)
{
    //locals
    int l_appThreadRc        = SSX_OK;
    int l_cmdThreadRc        = SSX_OK;
    int l_timerRc            = SSX_OK;
    int l_testAppletThreadRc = SSX_OK;
    int l_dcomThreadRc       = SSX_OK;
    int l_snapshotTimerRc    = SSX_OK;

    //Creating threads that need to be scheduled 
    //Thread priority range should match scheduled 
    // threads in G_scheduledThreads ie highest priority thread should be 
    // index 0 of G_scheduledThreads

    l_cmdThreadRc = createAndResumeThreadHelper(&Cmd_Hndl_thread,
              Cmd_Hndl_thread_routine,
              (void *)0,
              (SsxAddress)Cmd_hndl_thread_stack,
              THREAD_STACK_SIZE,
              THREAD_PRIORITY_3);

    l_appThreadRc = createAndResumeThreadHelper(&App_thread,
              App_thread_routine, 
              (void *)&G_apltPdtType,
              (SsxAddress)App_thread_stack,
              THREAD_STACK_SIZE,
              THREAD_PRIORITY_4);

    l_testAppletThreadRc = createAndResumeThreadHelper(&TestAppletThread,
                 App_thread_routine,
                 (void *)&G_apltTestType,
                 (SsxAddress)testAppletThreadStack,
                 THREAD_STACK_SIZE,
                 THREAD_PRIORITY_5);

   l_dcomThreadRc = createAndResumeThreadHelper(&Dcom_thread,
                 Dcom_thread_routine,
                 (void *)0,
                 (SsxAddress)dcomThreadStack,
                 THREAD_STACK_SIZE,
                 THREAD_PRIORITY_6);

    //create the thread scheduler timer
    l_timerRc = ssx_timer_create(&G_threadSchTimer, threadSwapcallback, 0);
    
    //check for errors creating the timer
    if(l_timerRc == SSX_OK)
    {
        TRAC_INFO("timer created and scheduled");
        //schedule the timer so that it runs every THREAD_TIME_SLICE
        l_timerRc = ssx_timer_schedule(&G_threadSchTimer, 1, THREAD_TIME_SLICE);
    }
    else
    {
        TRAC_INFO("Error creating timer: RC: %d", l_timerRc);
    }

    //Create snapshot timer
    l_snapshotTimerRc = ssx_timer_create(&G_snapshotTimer, cmdh_snapshot_callback, 0);
    
    //check for errors creating the timer
    if(l_snapshotTimerRc == SSX_OK)
    {
        //TRAC_INFO("snapshot buffer timer created and scheduled every 30 seconds");
        //schedule the timer so that it runs every 30 seconds.
        l_snapshotTimerRc = ssx_timer_schedule(&G_snapshotTimer, 0, SSX_SECONDS(30)); 
        if (l_snapshotTimerRc != SSX_OK)
        {
            TRAC_ERR("cmdh_snapshot_sync: reseting the snapshot timer failed.");

        } 
    }
    else
    {
        TRAC_INFO("Error creating timer: RC: %d", l_snapshotTimerRc);
    }


    //If there are any errors creating the threads or starting the
    //timer create an error log to pass back.
    if(    l_appThreadRc 
        || l_testAppletThreadRc 
        || l_cmdThreadRc 
        || l_dcomThreadRc
        || l_timerRc
		|| l_snapshotTimerRc )
    {
        TRAC_ERR("Error creating thread: l_appThreadRc: %d, "
                 "l_testAppletThreadRc: %d, l_cmdThreadRc: %d, "
                 "l_dcomThreadRc: %d", l_appThreadRc,l_testAppletThreadRc,
				 l_timerRc,l_cmdThreadRc,l_dcomThreadRc);
		TRAC_ERR("Error starting timers: timerRc: %d, snapshotTimerRc: %d.",
		          l_timerRc, l_snapshotTimerRc);
        // Create error log and log it
        // TODO use correct trace
        tracDesc_t  l_trace = NULL; 

        /* @
         * @errortype
         * @moduleid    THRD_MID_INIT_THREAD_SCHDLR
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   Schedule timer return code
		 * @userdata2   Snapshot timer return code
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     SSX thread related failure
         */
        errlHndl_t l_rc = createErrl(THRD_MID_INIT_THREAD_SCHDLR,   //modId
                                     SSX_GENERIC_FAILURE,           //reasoncode
                                     OCC_NO_EXTENDED_RC,            //Extended reason code 
                                     ERRL_SEV_UNRECOVERABLE,        //Severity
                                     l_trace,                       //Trace Buf
                                     DEFAULT_TRACE_SIZE,            //Trace Size
                                     l_timerRc,                     //userdata1
									 l_snapshotTimerRc);            //userdata2

        REQUEST_RESET(l_rc);     // @gm006
    }
}


// Function Specification
//
// Name:  threadSwapcallback
//
// Description: a periodic timer callback to swap prorities of scheduled threads 
//
// Flow:  07/16/11    FN=threadSwapcallback
//
// End Function Specification
void threadSwapcallback(void * arg)
{
    //locals
    int l_rc = SSX_OK;

    //current location of index in scheduled thread array 
    int l_threadAIndex = G_threadSchedulerIndex;

    //if global index == last item swap priorities with 1st
    int l_threadBIndex = (G_threadSchedulerIndex == (THREADS_TO_SCHEDULE-1)) ? 0 : ++G_threadSchedulerIndex;
    
    //swap priorities with global index +1
    l_rc = ssx_thread_priority_swap(G_scheduledThreads[l_threadAIndex],G_scheduledThreads[l_threadBIndex]);

    if(l_rc != SSX_OK) 
    {
        //TODO trace error

        //create and commit error log
        if(G_threadSwapErrlCounter == 0)
        { 
            // TODO use correct trace
            tracDesc_t l_trace = NULL; 

            /*
             * @errortype
             * @moduleid    THRD_MID_THREAD_SWAP_CALLBACK
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   Return code of thread priority swap
             * @userdata2   Current location of index in scheduled thread array 
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     SSX thread related failure
             */
            errlHndl_t  l_err = createErrl(
                    THRD_MID_THREAD_SWAP_CALLBACK,  //modId
                    SSX_GENERIC_FAILURE,            //reasoncode
                    OCC_NO_EXTENDED_RC,             //Extended reason code 
                    ERRL_SEV_PREDICTIVE,            //Severity
                    l_trace,                        //Trace Buf
                    DEFAULT_TRACE_SIZE,             //Trace Size
                    l_rc,                           //userdata1
                    l_threadAIndex                  //userdata2
                    );
            
            // commit log 
            // NOTE: log should be deleted by reader mechanism
            commitErrl( &l_err ); 
                   
            //Increment errl counter
             G_threadSwapErrlCounter++;

        }//end thread swap counter if
    }
    else
    {
        //reset counter since it started working again
        G_threadSwapErrlCounter = 0;

        //set the global to the new location of the highest priority thread
        G_threadSchedulerIndex = l_threadBIndex;
    }
}

