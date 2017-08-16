/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/reset.c $                                         */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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
      if( G_reset_state < RESET_REQUESTED_DUE_TO_ERROR )
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
      if( G_reset_state < NOMINAL_REQUESTED_DUE_TO_ERROR )
      {
        TRAC_ERR("Going to Nominal because of error");

        // May need to add counter if multiple places request nominal
        G_reset_state = NOMINAL_REQUESTED_DUE_TO_ERROR;

      }
      break;

    case RESET_NOT_REQUESTED:
      if( G_reset_state == NOMINAL_REQUESTED_DUE_TO_ERROR )
      {
        TRAC_IMP("Clearing Nominal Reset State because of error");

        // May need to add counter check if multiple places request nominal
        G_reset_state = RESET_NOT_REQUESTED;

      }
      break;

    default:
      break;

  }
}

