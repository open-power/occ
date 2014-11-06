/******************************************************************************
// @file threadtest.c
// @brief OCC THREAD TEST MAIN FUNCTION
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section threadtest.c THREADTEST.C
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @dw000             dwoodham  12/12/2011  Update call to IMAGE_HEADER; Added change log
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *
 *  @endverbatim
 *
 *///*************************************************************************/

//*************************************************************************
// Includes
//*************************************************************************
#include "ssx.h"
#include "ssx_io.h"
#include "simics_stdio.h"
#include <errl.h>
#include <rand.h>
#include <thread.h>
#include <threadSch.h>
#include <appletId.h>  // @dw000a


/** \mainpage OCC Overview
 *
 * Some general info.
 *
 * This manual is divided in the following sections:
 * - \subpage Timer
 */

//*************************************************************************
// Externs
//*************************************************************************
extern void __ssx_boot;

//*************************************************************************
// Image Header
//*************************************************************************
// @dw000c
IMAGE_HEADER (G_mainAppImageHdr,__ssx_boot,MAIN_APP_ID,OCC_APLT_TEST);

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
/// Period in which to run #timer_routine
#define TIMER_INTERVAL (SsxInterval) SSX_MICROSECONDS(5000) 


/*----------------------------------------------------------------------------*/
/* SsxSemaphore Declarations                                                  */
/*----------------------------------------------------------------------------*/
SsxSemaphore prcd_sem;

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
int g_j = 0;
int g_k = 0;
/*----------------------------------------------------------------------------*/
/* noncrtical/critical stack                                                  */
/*----------------------------------------------------------------------------*/
uint8_t noncritical_stack[NONCRITICAL_STACK_SIZE];
uint8_t critical_stack[CRITICAL_STACK_SIZE];
SimicsStdio simics_stdout;
SimicsStdio simics_stderr;

SsxTimer G_test_timer;

//*************************************************************************
// Function Prototypes
//*************************************************************************
extern void timer_routine(void *private);
extern void rtloop_ocb_init(void);

//*************************************************************************
// Functions
//*************************************************************************

// Function Specification
//
// Name: pgp_validation_ssx_main_hook
//
// Description: 
//
// Flow:              FN=None
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
// Flow:              FN=None
// 
// End Function Specification
//TODO placeholder
void Cmd_Hndl_thread_routine(void *arg)
{
     do
    {
        int x=0;
        for(x=0; x < 1000; x++)
        {

        }
        //printf("Thread A running");
    }while(1);

}

// Function Specification
//
// Name: App_thread_routine
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
void App_thread_routine(void *arg)
{
    int z=0;
     do
    {
        int x=0;
        for(x=0; x < 10000; x++)
        {
            z++;
        }
        //printf("Thread A running");
    }while(1);

}

// Function Specification
//
// Name: Thermal_Monitor_thread_routine
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
void Thermal_Monitor_thread_routine(void *arg)
{
       int z=0;
     do
    {
        int x=0;
        for(x=0; x < 10000; x++)
        {
            z++;
        }
        //printf("Thread A running");
    }while(1);
 
}

// Function Specification
//
// Name: Hlth_Monitor_thread_routine
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
void Hlth_Monitor_thread_routine(void *arg)
{
       int z=0;
     do
    {
        int x=0;
        for(x=0; x < 10000; x++)
        {
            z++;
        }
        //printf("Thread A running");
    }while(1);

}

// Function Specification
//
// Name: FFDC_thread_routine
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
void FFDC_thread_routine(void *arg)
{
       int z=0;
     do
    {
        int x=0;
        for(x=0; x < 10000; x++)
        {
            z++;
        }
        //printf("Thread A running");
    }while(1);

}


/** Main Thread
 *
 * This thread currently just loops as the lowest priority thread, handling
 * the lowest priority tasks.
 * 
 */
// Function Specification
//
// Name: main_thread_routine
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
void main_thread_routine(void *private)
{
  // Start the critical 250uS timer
   ssx_timer_schedule(&timer, 1, TIMER_INTERVAL);

  while(1)
  {
    // Only trace the first XX times that this function loops
    if(g_k < 3)
    {
      g_k++;

      // TRACE: Main Thread
    }

    // Sleep for 1000 before we run the loop again
    ssx_sleep_absolute(1000);
  }
}

// Function Specification
//
// Name: timer_routine
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
void  timer_routine (void *arg)
{

}

// Function Specification
//
// Name: dump_thread_info
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
void dump_thread_info(void *arg)
{
    
    printf("dumping thread info--------------------------\n");
    int l_rc = 0;
    SsxThreadState l_state = 0;
    SsxThreadPriority l_pri = 0;
    int l_runnable=0;
    int x=0;
    for(x=0; x < THREADS_TO_SCHEDULE; x++)
    {
        l_rc = ssx_thread_info_get(G_scheduledThreads[x],
                                    &l_state,
                                    &l_pri,
                                    &l_runnable);

        printf("Thread %p: State %x priority %x runnable %x rc %x Global index %x\n",G_scheduledThreads[x],
                                                           l_state,
                                                            l_pri,
                                                            l_runnable,
                                                            l_rc,
                                                            G_threadSchedulerIndex);        

    }

}

/** Entry point for OCC execution
 *
 * main() currently initalizes our trace buffer along with creating threads
 * and timers for execution.  Note that once main runs ssx_start_threads, we
 * never return as the SSX kernel takes over.
 * 
 */
// Function Specification
//
// Name: main
//
// Description: 
//
// Flow:              FN=None
// 
// End Function Specification
int main(int argc, char **argv)
{
    //locals
    errlHndl_t  l_errl = INVALID_ERR_HNDL;

    // Initialize Trace Buffers immediately, so they can be used 
    // from this point on.

    // Initialize stdout so we can do printf from within simics env
    simics_stdout_create(&simics_stdout);
    simics_stderr_create(&simics_stderr);
    stdout = (FILE *)(&simics_stdout);
    stderr = (FILE *)(&simics_stderr);

    // Initialize SSX Stacks (note that this also reinitializes the time base to 0)
    ssx_initialize((SsxAddress)noncritical_stack, NONCRITICAL_STACK_SIZE,
		   (SsxAddress)critical_stack, CRITICAL_STACK_SIZE,
		   0);

    

    // Create Global Semaphores
    ssx_semaphore_create(&prcd_sem, 0, 13);

    ssx_timer_create(&timer, timer_routine, 0);

//     //create thread with already mapped priority
//         // Create Threads
    ssx_thread_create(&main_thread, 
		      main_thread_routine, 
		      (void *)0,
		      (SsxAddress)main_thread_stack,
		      THREAD_STACK_SIZE,
		      0);


    // Make Threads runnable
    ssx_thread_resume(&main_thread);


    //Initialize the thread scheduler
    l_errl = initThreadScheduler();

    if(l_errl)
    {
        // Trace and commit error

        // TODO add trace

        // commit log 
        // NOTE: log should be deleted by reader mechanism
        commitErrl( &l_errl ); 
    }


    //kick off timer
    ssx_timer_create(&G_test_timer, dump_thread_info, 0);
    ssx_timer_schedule(&G_test_timer, 1, 500000000);


    // Enter SSX Kernel
    ssx_start_threads();

    return 0;
}

