/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_tasks.c $                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2018                        */
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

//*************************************************************************/
// Includes
//*************************************************************************/
#include <occ_common.h>
#include <ssx.h>
#include <errl.h>               // Error logging
#include <trac.h>               // Error logging
#include "rtls.h"
#include "occ_service_codes.h"  // for SSX_GENERIC_FAILURE
#include "amec_smh.h"
#include "amec_master_smh.h"
#include "amec_slave_smh.h"
#include "amec_sys.h"

//*************************************************************************/
// Externs
//*************************************************************************/

//*************************************************************************/
// Macros
//*************************************************************************/
/* rotate x to the left by s bits */
uint8_t _rotl8(uint8_t val, uint8_t shift)
{
  return (val << shift) | (val >> (8 - shift));
}

//*************************************************************************/
// Defines/Enums
//*************************************************************************/

//*************************************************************************/
// Structures
//*************************************************************************/

//*************************************************************************/
// Globals
//*************************************************************************/

//*************************************************************************/
// Function Prototypes
//*************************************************************************/

//*************************************************************************/
// Functions
//*************************************************************************/

// Function Specification
//
// Name: amec_generic_smh
//
// Description: Generic State Machine that allows for 8 states per level,
//              with 3 levels.
//              If State Machine gets called/incremented every          500us,
//                - Each function entry in Level 0 table runs every       4ms,
//                - Each function entry in Level 1 table will run every  32ms,
//                - Each function entry in Level 2 table will run every 256ms.
//
//              Anything longer than that will need to be taken care of
//              inside of the states themselves.
//
//              NULL entries are allowed, for both state functions and
//              for pointers to substate tables.
//              -  If a state does not have substates, it should have NULL
//                 in the substate table for the substate table pointer.
//              -  If a state does not wish to run a function, it should have
//                 NULL in the substate table for the function pointer.
//              -  A state can have substates without having a function that
//                 runs (i.e. NULL in function pointer, but valid in substate
//                 table pointer)
//
//              In addition, if i_smh_timing is not NULL and points to a
//              smh_state_timing_t structure, this state machine will grab
//              the SSX timings of each "Level 0" state and store the longest
//              time in each state in that structure.
//
// End Function Specification
void amec_generic_smh(const smh_tbl_t * i_smh_tbl,
                      smh_state_t * i_smh_state,
                      smh_state_timing_t * i_smh_timing)
{
  uint64_t l_start = ssx_timebase_get();
  uint8_t l_state = 0;

  // ----------------------------------------------------
  // AMEC State Machine States
  // ----------------------------------------------------
  if(NULL != i_smh_tbl)
  {
    // Read AMEC State Machine "State"
    l_state = AMEC_STATE(i_smh_state);

    AMEC_DBG("SMH: AMEC State: [%d] Mask: [%02x]\n",l_state,i_smh_state->state);

    // check if state value makes sense
    if ( l_state >= NUM_AMEC_SMH_STATES )
    {
        TRAC_ERR("SMH: Invalid AMEC State: [%d] Mask: [%02x]\n",l_state,i_smh_state->state);
        return;
    }

    // If there is a function to run for this state (!NULL), run it
    if(NULL != i_smh_tbl[(l_state)].state)
    {
      // Call into AMEC State (i.e State 6)
      (*(i_smh_tbl[(l_state)].state))();
    }

    // ----------------------------------------------------
    // AMEC State Machine Substates
    // ----------------------------------------------------
    if(NULL != i_smh_tbl[l_state].substate)
    {
      // Read AMEC State Machine "Substate"
      uint8_t l_substate = AMEC_SUBSTATE(i_smh_state);

      AMEC_DBG("\tSMH: AMEC Sub State: %d Mask: %02x\n",l_substate,i_smh_state->substate);

        // check if sub state value makes sense
        if ( l_substate >= NUM_AMEC_SMH_STATES )
        {
            TRAC_ERR("SMH: Invalid AMEC Sub State: [%d] Mask: [%02x]\n",l_substate,i_smh_state->substate);
            return;
        }

      // Set up pointer to sub-state table
      const smh_tbl_t * l_sub_state_tbl = i_smh_tbl[l_state].substate;

      // If there is a function to run for this substate (!NULL), run it
      if(NULL != (l_sub_state_tbl+l_substate)->state)
      {
        // Call into AMEC Substate (i.e State 6.1)
        (*((l_sub_state_tbl+l_substate)->state))();
      }

      // ----------------------------------------------------
      // AMEC State Machine Sub-substates
      // ----------------------------------------------------
      if(NULL != (l_sub_state_tbl+l_substate)->substate)
      {
        // Read AMEC State Machine "Sub-Substate"
        uint8_t l_sub_substate = AMEC_SUB_SUBSTATE(i_smh_state);

        AMEC_DBG("\t\tSMH: AMEC Sub Sub State: %d Mask: %02x\n",l_sub_substate,i_smh_state->sub_substate);

        // check if sub sub state value makes sense
        if ( l_sub_substate >= NUM_AMEC_SMH_STATES )
        {
            TRAC_ERR("SMH: Invalid AMEC Sub State: [%d] Mask: [%02x]\n",l_sub_substate,i_smh_state->substate);
            return;
        }

        // Set up pointer to sub-state table
        const smh_tbl_t * l_sub_substate_tbl = (l_sub_state_tbl+l_substate)->substate;

        // If there is a function to run for this sub-substate (!NULL), run it
        if(NULL != (l_sub_substate_tbl+l_sub_substate)->state)
        {
          // Call into AMEC Sub-Substate (i.e State 6.1.3)
          (*((l_sub_substate_tbl+l_sub_substate)->state))();
        }
      }
    }
  }

  // ------------------------------------------------------
  // Increment the state(s) of the AMEC State Machine
  // ------------------------------------------------------

  // Bump the state as final undertaking before task ends
  AMEC_STATE_NEXT(i_smh_state);
  if(AMEC_INITIAL_STATE == i_smh_state->state)
  {
    // Only bump the substate when "state" has wrapped
    AMEC_SUBSTATE_NEXT(i_smh_state);
    if(AMEC_INITIAL_STATE == i_smh_state->substate)
    {
      // Only bump the substate when "state" & "substate" have wrapped
      AMEC_SUB_SUBSTATE_NEXT(i_smh_state);
    }
  }

  // ------------------------------------------------------
  // Update the max timing of this state(s) of the AMEC State Machine
  // ------------------------------------------------------
  if((NULL != i_smh_timing) && (NULL != i_smh_timing->update_sensor))
  {
    // Calculate the duration of the state
    uint64_t l_state_duration = DURATION_IN_US_UNTIL_NOW_FROM(l_start);

    // Make a function call to update sensor with duration
    (*((i_smh_timing)->update_sensor))(l_state,(uint32_t)l_state_duration);
  }
}


// Function Specification
//
// Name: task_amec_master
//
// Description: Purpose of this function is to run all the AMEC "master-only" code
//
// Task Flags: RTL_FLAG_MSTR
//             RTL_FLAG_OBS
//             RTL_FLAG_ACTIVE
//             RTL_FLAG_MSTR_READY
//             RTL_FLAG_NO_APSS
//             RTL_FLAG_RUN
//
// End Function Specification
void task_amec_master( task_t *i_self)
{
  uint64_t l_start = ssx_timebase_get();

  amec_mst_common_tasks_pre();

  amec_generic_smh( amec_mst_state_table, &G_amec_mst_state, &G_amec_mst_state_timings );

  amec_mst_common_tasks_post();

  // Add the time that this master task took to the total AMEC int task time,
  // which already contains the slave task duration
  G_fw_timing.ameint_dur += DURATION_IN_US_UNTIL_NOW_FROM(l_start);
}


// Function Specification
//
// Name: task_amec_slave
//
// Description: Purpose of this function is to run all the AMEC "slave-only" code
//
// Task Flags: RTL_FLAG_MSTR
//             RTL_FLAG_NONMSTR
//             RTL_FLAG_OBS
//             RTL_FLAG_ACTIVE
//             RTL_FLAG_MSTR_READY
//             RTL_FLAG_NO_APSS
//             RTL_FLAG_RUN
//
// End Function Specification
void task_amec_slave( task_t *i_self)
{
  uint64_t l_start = ssx_timebase_get();

  amec_slv_common_tasks_pre();

  if (FALSE == isSafeStateRequested())
  {
      amec_generic_smh( amec_slv_state_table, &G_amec_slv_state, &G_amec_slv_state_timings );

      amec_slv_common_tasks_post();

      // Set the total AMEC int task time for this tick, to the duration of the slave tasks.
      G_fw_timing.ameint_dur = DURATION_IN_US_UNTIL_NOW_FROM(l_start);
  }

}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
