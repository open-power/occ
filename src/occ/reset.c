/******************************************************************************
// @file reset.c
// @brief OCC Reset States
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section reset.c reset.C
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @th011             thallet   07/16/2012  New file
 *   @th042   892056    thallet   07/19/2013  Send OCC to safe mode if first APSS GPE fails
 *   @th046  894648     thallet   08/08/2013  Piggyback a 1-liner on the coreq fix
 *   @jh00b  910184     joshych   01/10/2014  Add check for checkstop
 *   @gm028  911670     milesg    02/27/2014  Immediate safe mode on checkstop
 *
 *  @endverbatim
 *
 *///*************************************************************************/

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <common_types.h>
#include "ssx_io.h"
#include "trac.h"
#include "rtls.h"
#include "state.h"
#include "dcom.h"

//*************************************************************************
// Externs
//*************************************************************************

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
// Forward Declarations
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
// Holds the state of the reset state machine
uint8_t G_reset_state = RESET_NOT_REQUESTED;

// Flag indicating if we should halt on a reset request, or if we should
// enter the reset state machine.  Default this to false
bool G_halt_on_reset_request = FALSE;   // @th042


//*************************************************************************
// Functions
//*************************************************************************

// Function Specification
//
// Name: reset_disable_halt
//
// Description: Clear Flag that indicates if OCC should call halt
//
// Flow:  10/25/11    FN=task_reset_state_machine
//
// End Function Specification
inline void reset_disable_halt(void)
{
  G_halt_on_reset_request = FALSE;
}


// Function Specification
//
// Name: isSafeStateRequested
//
// Description: Helper function for determining if we should go to safe state
//
// Flow:  --/--/--    FN=
//
// End Function Specification
bool isSafeStateRequested(void)
{
    return ((RESET_REQUESTED_DUE_TO_ERROR == G_reset_state) ? TRUE : FALSE);
}


// Function Specification
//
// Name: reset_state_request
//
// Description: Request Reset States 
//
// Flow:  10/25/11    FN=task_reset_state_machine
//
// End Function Specification
void reset_state_request(uint8_t i_request)
{
  //TODO:  This needs to be changed so that G_reset_state operations are 
  // atomic.

  switch(i_request)
  {
    case RESET_REQUESTED_DUE_TO_ERROR:
      // In case we want to just halt() if fw requests a reset, this is 
      // the place to do it.  It is disabled by default, and there is no
      // code to eanble it.
      if( G_halt_on_reset_request )
      {
          TRAC_ERR("Halt()");
          
          // This isn't modeled very well in simics.  OCC will go into an
          // infinite loop, which eventually would crash Simics.
          HALT_WITH_FIR_SET; 
      }

      // If we have TMGT comm, and we aren't already in reset, set the reset
      // state to reset to enter the reset state machine.
      if(G_reset_state < RESET_REQUESTED_DUE_TO_ERROR)
      {
        TRAC_ERR("Resetting via Reset State Machine");

        G_reset_state = RESET_REQUESTED_DUE_TO_ERROR;
        
        // Post the semaphore to wakeup the thread that
        // will put us into SAFE state.
        ssx_semaphore_post(&G_dcomThreadWakeupSem);  // @th042 

        // Set RTL Flags here too, depending how urgent it is that we stop 
        // running tasks.
        rtl_set_run_mask(RTL_FLAG_RST_REQ);
      }
      break;

    case NOMINAL_REQUESTED_DUE_TO_ERROR:
      if(G_reset_state < NOMINAL_REQUESTED_DUE_TO_ERROR)
      {
        TRAC_ERR("Going to Nominal because of error");

        // May need to add counter if multiple places request nominal
        G_reset_state = NOMINAL_REQUESTED_DUE_TO_ERROR;

        //TODO:  Will need to set some flag or event here
      }
      break;

    case RESET_NOT_REQUESTED:
      if(G_reset_state == NOMINAL_REQUESTED_DUE_TO_ERROR)
      {
        TRAC_IMP("Clearing Nominal Reset State because of error");

        // May need to add counter check if multiple places request nominal
        G_reset_state = RESET_NOT_REQUESTED;

        //TODO:  Will need to clear some flag or event here
      }
      break;

    default:
      break;

  }
}

// Function Specification
//
// Name: task_check_for_checkstop
//
// Description: Check for checkstop
//
// Flow:             FN=task_check_for_checkstop
// 
// End Function Specification
void task_check_for_checkstop(task_t *i_self) // @jh00bc
{
    pore_status_t l_gpe0_status;
    static bool L_gpe_halt_traced = FALSE;

    do
    {
        //only do this once
        if(L_gpe_halt_traced)
        {
            break;
        }

        L_gpe_halt_traced = TRUE;

        l_gpe0_status.value = in64(PORE_GPE0_STATUS);

        if (l_gpe0_status.fields.freeze_action)
        {
            errlHndl_t     l_err = NULL;

            TRAC_ERR("task_check_for_checkstop: OCC GPE halted due to checkstop");

            /*
             * @errortype
             * @moduleid    MAIN_GPE_HALTED_MID
             * @reasoncode  OCC_GPE_HALTED
             * @userdata1   0
             * @userdata2   0
             * @devdesc     OCC GPE halted due to checkstop
             */
             l_err = createErrl(MAIN_GPE_HALTED_MID,
                                OCC_GPE_HALTED,
                                OCC_NO_EXTENDED_RC,
                                ERRL_SEV_INFORMATIONAL,
                                NULL,
                                DEFAULT_TRACE_SIZE,
                                0,
                                0);

             //This will request safe mode under the covers due to the GPE being halted.
             commitErrl(&l_err);
        }

    } while (0);
}




