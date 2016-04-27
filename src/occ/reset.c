/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/reset.c $                                             */
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

#include <occ_common.h>
#include <common_types.h>
#include "ssx_io.h"
#include "trac.h"
#include "rtls.h"
#include "state.h"
#include "dcom.h"
#include "amec_wof.h"
#include "amec_sys.h"

// Holds the state of the reset state machine
uint8_t G_reset_state = RESET_NOT_REQUESTED;

// Flag indicating if we should halt on a reset request, or if we should
// enter the reset state machine.  Default this to false
bool G_halt_on_reset_request = FALSE;


// Function Specification
//
// Name: reset_disable_halt
//
// Description: Clear Flag that indicates if OCC should call halt
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
        TRAC_IMP("Activating reset required state.");

        G_reset_state = RESET_REQUESTED_DUE_TO_ERROR;

        // Post the semaphore to wakeup the thread that
        // will put us into SAFE state.
        ssx_semaphore_post(&G_dcomThreadWakeupSem);

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
// Name: reset_wof_clear_inhibit
//
// Description: This function clears the inhibit bits that are
// set as part of the WOF function
//
// End Function Specification
void reset_wof_clear_inhibit()
{
    uint32_t l_data32 = 0;
    uint32_t l_rc = 0;

    // Do not inhibit core wakeup anymore
    g_amec->wof.enable_parm = 0;
    l_data32 = 0;
    out32(PDEMR, l_data32);

    TRAC_IMP("reset_wof_clear_inhibit: PDEMR register has been successfully cleared");
}

// Function Specification
//
// Name: task_check_for_checkstop
//
// Description: Check for checkstop
//
// End Function Specification
void task_check_for_checkstop(task_t *i_self)
{
    pore_status_t l_gpe0_status;
    ocb_oisr0_t l_oisr0_status;
    static bool L_checkstop_traced = FALSE;
    uint8_t l_reason_code = 0;

    do
    {
        // This check is disabled once a checkstop or frozen GPE is detected
        if(L_checkstop_traced)
        {
            break;
        }

        // Looked for a frozen GPE, a sign that the chip has stopped working or
        // check-stopped.  This check also looks for an interrupt status flag that
        // indicates if the system has check-stopped.
        l_gpe0_status.value = in64(PORE_GPE0_STATUS);
        l_oisr0_status.value = in32(OCB_OISR0);

        if (l_gpe0_status.fields.freeze_action
            ||
            l_oisr0_status.fields.check_stop)
        {
            errlHndl_t l_err = NULL;

            if (l_gpe0_status.fields.freeze_action)
            {
                TRAC_IMP("Frozen GPE0 detected by RTL");
                l_reason_code = OCC_GPE_HALTED;
            }

            if (l_oisr0_status.fields.check_stop)
            {
                TRAC_IMP("System checkstop detected by RTL");
                l_reason_code = OCC_SYSTEM_HALTED;
            }

            L_checkstop_traced = TRUE;

            /*
             * @errortype
             * @moduleid    MAIN_SYSTEM_HALTED_MID
             * @reasoncode  OCC_GPE_HALTED
             * @userdata1   High order word of PORE_GPE0_STATUS
             * @userdata2   OCB_OISR0
             * @devdesc     OCC detected frozen GPE0
             */
            /*
             * @errortype
             * @moduleid    MAIN_SYSTEM_HALTED_MID
             * @reasoncode  OCC_SYSTEM_HALTED
             * @userdata1   High order word of PORE_GPE0_STATUS
             * @userdata2   OCB_OISR0
             * @devdesc     OCC detected system checkstop
             */
             l_err = createErrl(MAIN_SYSTEM_HALTED_MID,
                                l_reason_code,
                                OCC_NO_EXTENDED_RC,
                                ERRL_SEV_INFORMATIONAL,
                                NULL,
                                DEFAULT_TRACE_SIZE,
                                l_gpe0_status.words.high_order,
                                l_oisr0_status.value);

             // The commit code will check for the frozen GPE0 and system
             // checkstop conditions and take appropriate actions.
             commitErrl(&l_err);
        }
    }
    while(0);
}
