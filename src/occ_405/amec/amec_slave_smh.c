/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_slave_smh.c $                           */
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

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <ssx.h>
#include <errl.h>               // Error logging
#include "sensor.h"
#include "rtls.h"
#include "occ_sys_config.h"
#include "occ_service_codes.h"  // for SSX_GENERIC_FAILURE
#include "dcom.h"
#include "proc_data.h"
#include "proc_data_control.h"
#include "amec_smh.h"
#include "amec_slave_smh.h"
#include <trac.h>
#include "amec_sys.h"
#include "sensor_enum.h"
#include "amec_service_codes.h"
#include <amec_sensors_core.h>
#include <amec_sensors_power.h>
#include <amec_sensors_centaur.h>
#include <amec_sensors_fw.h>
#include <amec_freq.h>
#include <amec_data.h>
#include <centaur_data.h>
#include <amec_amester.h>
#include <amec_oversub.h>
#include <amec_health.h>
#include <amec_analytics.h>
#include <common.h>

//*************************************************************************
// Externs
//*************************************************************************
extern dcom_slv_inbox_t G_dcom_slv_inbox_rx;
extern uint8_t G_vrm_present;

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
smh_state_t G_amec_slv_state = {AMEC_INITIAL_STATE,
                                AMEC_INITIAL_STATE,
                                AMEC_INITIAL_STATE};

// Number of ticks for periodically updating VRM-related data
#define AMEC_UPDATE_VRM_TICKS   4000

// This table presents the core data sensors collection pattern.
// This pattern is not completely rbitrary, and it must always
// follow track the GPE request pattern for the data collection.
// The choice made here is for 3 cores data collection every 2 tick,
// with a maximum latency of 2 ticks (counting the dispatch tick).
#define CORES_PER_STATE 3

const uint8_t G_sensor_update_pattern[AMEC_SMH_STATES_PER_LVL][CORES_PER_STATE] =
{
    { 23,  0, 12 },    // Group 0
    {  1,  2, 13 },    // Group 1
    { 14,  3, 15 },    // Group 2
    {  4,  5, 16 },    // Group 3
    { 17,  6, 18 },    // Group 4
    {  7,  8, 19 },    // Group 5
    { 20,  9, 21 },    // Group 6
    { 10, 11, 22 }     // Group 7
};

// --------------------------------------------------------
// AMEC Slave State 1 Substate Table
// --------------------------------------------------------
// Each function inside this state table runs once every 16ms.
//
// No Substates
//
const smh_tbl_t amec_slv_state_1_substate_table[AMEC_SMH_STATES_PER_LVL] =
{
  {amec_slv_substate_1_0, NULL},
  {amec_slv_substate_1_1, NULL},
  {amec_slv_substate_1_2, NULL},
  {amec_slv_substate_1_3, NULL},
  {amec_slv_substate_1_4, NULL},
  {amec_slv_substate_1_5, NULL},
  {amec_slv_substate_1_6, NULL},
  {amec_slv_substate_1_7, NULL},
};

// --------------------------------------------------------
// AMEC Slave State 3 Substate Table
// --------------------------------------------------------
// Each function inside this state table runs once every 16ms.
//
// No Substates
//
const smh_tbl_t amec_slv_state_3_substate_table[AMEC_SMH_STATES_PER_LVL] =
{
  {amec_slv_substate_3_0, NULL},
  {amec_slv_substate_3_1, NULL},
  {amec_slv_substate_3_2, NULL},
  {amec_slv_substate_3_3, NULL},
  {amec_slv_substate_3_4, NULL},
  {amec_slv_substate_3_5, NULL},
  {amec_slv_substate_3_6, NULL},
  {amec_slv_substate_3_7, NULL},
};

// --------------------------------------------------------
// AMEC Slave State 5 Substate Table
// --------------------------------------------------------
// Each function inside this state table runs once every 16ms.
//
// No Substates
//
const smh_tbl_t amec_slv_state_5_substate_table[AMEC_SMH_STATES_PER_LVL] =
{
  {amec_slv_substate_5_0, NULL},
  {amec_slv_substate_5_1, NULL},
  {amec_slv_substate_5_2, NULL},
  {amec_slv_substate_5_3, NULL},
  {amec_slv_substate_5_4, NULL},
  {amec_slv_substate_5_5, NULL},
  {amec_slv_substate_5_6, NULL},
  {amec_slv_substate_5_7, NULL},
};

// --------------------------------------------------------
// AMEC Slave State 7 Substate Table
// --------------------------------------------------------
// Each function inside this state table runs once every 16ms.
//
// No Substates
//
const smh_tbl_t amec_slv_state_7_substate_table[AMEC_SMH_STATES_PER_LVL] =
{
  {amec_slv_substate_7_0, NULL},
  {amec_slv_substate_7_1, NULL},
  {amec_slv_substate_7_2, NULL},
  {amec_slv_substate_7_3, NULL},
  {amec_slv_substate_7_4, NULL},
  {amec_slv_substate_7_5, NULL},
  {amec_slv_substate_7_6, NULL},
  {amec_slv_substate_7_7, NULL},
};


// --------------------------------------------------------
// Main AMEC Slave State Table
// --------------------------------------------------------
// Each function inside this state table runs once every 2ms.
//
// No Substates
//
const smh_tbl_t amec_slv_state_table[AMEC_SMH_STATES_PER_LVL] =
{
  {amec_slv_state_0, NULL},
  {amec_slv_state_1, amec_slv_state_1_substate_table},
  {amec_slv_state_2, NULL},
  {amec_slv_state_3, amec_slv_state_3_substate_table},
  {amec_slv_state_4, NULL},
  {amec_slv_state_5, amec_slv_state_5_substate_table},
  {amec_slv_state_6, NULL},
  {amec_slv_state_7, amec_slv_state_7_substate_table},
};



// This sets up the function pointer that will be called to update the
// fw timings when the AMEC Slave State Machine finishes.
smh_state_timing_t G_amec_slv_state_timings = {amec_slv_update_smh_sensors};

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************


// Function Specification
//
// Name: amec_slv_check_apss_fail
//
// Description: This function checks if there are APSS failures and takes
// action accordingly. If there are APSS failures, it will lower the Pmax_rail
// to nominal. Else, if will release the Pmax_rail.
//
// End Function Specification
void amec_slv_check_apss_fail(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    uint32_t                    l_pmax_rail_freq = g_amec->proc[0].pwr_votes.apss_pmax_clip_freq;
    Pstate                      l_pstate = 0;
    static bool                 L_lower_pmax_rail = FALSE;
    static bool                 L_raise_pmax_rail = TRUE;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    if (G_apss_lower_pmax_rail == TRUE)
    {
        if (L_lower_pmax_rail == FALSE)
        {
            // Lower the Pmax_rail to nominal
            l_pmax_rail_freq = G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL];
            l_pstate = proc_freq2pstate(l_pmax_rail_freq);

            // Set the Pmax_rail register via OCI write
            amec_oversub_pmax_clip(l_pstate);

            L_lower_pmax_rail = TRUE;
            L_raise_pmax_rail = FALSE;
        }
    }
    else
    {
        if (L_raise_pmax_rail == FALSE)
        {
            // Raise the Pmax rail back
            l_pmax_rail_freq = G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO];
            l_pstate = proc_freq2pstate(l_pmax_rail_freq);

            // Set the Pmax_rail register via OCI write
            amec_oversub_pmax_clip(l_pstate);

            L_lower_pmax_rail = FALSE;
            L_raise_pmax_rail = TRUE;
        }
    }

    // Store the frequency vote for the voting box
    g_amec->proc[0].pwr_votes.apss_pmax_clip_freq = l_pmax_rail_freq;
}


// Function Specification
//
// Name: amec_slv_common_tasks_pre
//
// Description: Runs all the functions that need to run pre-AMEC-State-Machine
//              This function will run every tick.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_slv_common_tasks_pre(void)
{
//  @TODO - TEMP - Not ready yet in Phase 1
//  static uint16_t L_counter = 0;

  AMEC_DBG("\tAMEC Slave Pre-State Common\n");

  // Update the FW Worst Case sensors every tick
  amec_update_fw_sensors();

  // Update the fast core data sensors every tick
//  @TODO - TEMP - Not ready yet in Phase 1
//  amec_update_fast_core_data_sensors();

  // Update the sensors that come from the APSS every tick
  amec_update_apss_sensors();

  // Call the stream buffer recording function
//  @TODO - TEMP - Not ready yet in Phase 1
/*  amec_analytics_sb_recording();

  // Update the sensors that come from the VRM
  L_counter++;
  if (L_counter == AMEC_UPDATE_VRM_TICKS)
  {
      if (G_vrm_present)
      {
          amec_update_vrm_sensors();
      }
      L_counter = 0;
  }
*/

  // Update the external voltage sensors
  amec_update_external_voltage();

  // Update estimate of Vdd regulator output current

  amec_update_current_sensor(); // Compute estimate for Vdd output current

  // Over-subscription check
//  @TODO - TEMP - Not ready yet in Phase 1
//  amec_oversub_check();
}

// Function Specification
//
// Name: amec_slv_cmmon_tasks_post
//
// Description: Runs all the functions that need to run post-AMEC-State-Machine
//              This function will run every tick.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_slv_common_tasks_post(void)
{
  AMEC_DBG("\tAMEC Slave Post-State Common\n");

  // Only execute if OCC is in the active state
  if ( IS_OCC_STATE_ACTIVE() )
  {
      // Check if we need to change Pmax_clip register setting due to not
      // getting any APSS data from Master
      amec_slv_check_apss_fail();

      // Call amec_power_control
      amec_power_control();

      // Call the OCC slave's voting box
      amec_slv_voting_box();

      // Call the frequency state machine
      amec_slv_freq_smh();

      // Call the OCC slave's memory voting box
      amec_slv_mem_voting_box();

      // Call the OCC slave's performance check
      amec_slv_check_perf();

      // Call the 250us trace recording if it has been configured via Amester.
      // If not configured, this call will return immediately.
      amec_tb_record(AMEC_TB_250US);
  }
}

// Function Specification
//
// Name: amec_slv_state_0
//
// Description: AMEC Slave State Machine State
//
// End Function Specification
void amec_slv_state_0(void)
{
  AMEC_DBG("\tAMEC Slave State 0\n");

  //-------------------------------------------------------
  // Update Proc Core sensors (for this tick)
  //-------------------------------------------------------

/* Not yet supported
  //-------------------------------------------------------
  // Update Centaur sensors (for this tick)
  //-------------------------------------------------------
  amec_update_centaur_sensors(CENTAUR_0);
*/
  //-------------------------------------------------------
  // Update Sleep Count & Winkle Count Sensors
  //-------------------------------------------------------
  sensor_update(AMECSENSOR_PTR(SLEEPCNT4MSP0),
                __builtin_popcount( g_amec->proc[0].sleep_cnt));
  g_amec->proc[0].sleep_cnt  = 0;

  sensor_update(AMECSENSOR_PTR(WINKCNT4MSP0),
                __builtin_popcount(g_amec->proc[0].winkle_cnt));
  g_amec->proc[0].winkle_cnt = 0;

  //-------------------------------------------------------
  // Update vector sensors
  //-------------------------------------------------------
  sensor_vector_update(AMECSENSOR_PTR(TEMP4MSP0),    1);
  sensor_vector_update(AMECSENSOR_PTR(TEMP4MSP0PEAK),1);
  sensor_vector_update(AMECSENSOR_PTR(FREQA4MSP0),   1);
  sensor_vector_update(AMECSENSOR_PTR(IPS4MSP0),     1);
  sensor_vector_update(AMECSENSOR_PTR(UTIL4MSP0),    1);

  // Call the trace function for 2ms tracing if it has been configured via
  // Amester. If not configured, this call will return immediately.
  amec_tb_record(AMEC_TB_2MS);
}


// Function Specification
//
// Name: amec_slv_state_1
//
// Description: AMEC Slave State Machine State
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_slv_state_1(void)
{
  AMEC_DBG("\tAMEC Slave State 1\n");

  //-------------------------------------------------------
  // Update Proc Core sensors (for this tick)
  //-------------------------------------------------------


/* Not yet supported
  //-------------------------------------------------------
  // Update Centaur sensors (for this tick)
  //-------------------------------------------------------
  amec_update_centaur_sensors(CENTAUR_1);

  //-------------------------------------------------------
  // Update Proc Level Centaur/DIMM Temperature sensors
  //-------------------------------------------------------
  amec_update_centaur_temp_sensors();
*/
}


// Function Specification
//
// Name: amec_slv_state_2
//
// Description: AMEC Slave State Machine State
//
// End Function Specification
void amec_slv_state_2(void)
{
  AMEC_DBG("\tAMEC Slave State 2\n");

  //-------------------------------------------------------
  // Update Proc Core sensors (for this tick)
  //-------------------------------------------------------

/* Not yet supported
  //-------------------------------------------------------
  // Update Centaur sensors (for this tick)
  //-------------------------------------------------------
  amec_update_centaur_sensors(CENTAUR_2);
*/
}


// Function Specification
//
// Name: amec_slv_state_3
//
// Description: AMEC Slave State Machine State
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_slv_state_3(void)
{
  AMEC_DBG("\tAMEC Slave State 3\n");

  //-------------------------------------------------------
  // Update Proc Core sensors (for this tick)
  //-------------------------------------------------------

  //-------------------------------------------------------
  // Update Centaur sensors (for this tick)
  //-------------------------------------------------------
/* Not yet supported
  amec_update_centaur_sensors(CENTAUR_3);

  //-------------------------------------------------------
  // Perform amec_analytics (set amec_analytics_slot to 3)
  //-------------------------------------------------------
  amec_analytics_main();
*/
}


// Function Specification
//
// Name: amec_slv_state_4
//
// Description: AMEC Slave State Machine State
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_slv_state_4(void)
{
  AMEC_DBG("\tAMEC Slave State 4\n");

  //-------------------------------------------------------
  // Update Proc Core sensors (for this tick)
  //-------------------------------------------------------

/* Not yet supported
  //-------------------------------------------------------
  // Update Centaur sensors (for this tick)
  //-------------------------------------------------------
  amec_update_centaur_sensors(CENTAUR_4);
*/
}


// Function Specification
//
// Name: amec_slv_state_5
//
// Description: AMEC Slave State Machine State
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_slv_state_5(void)
{
  AMEC_DBG("\tAMEC Slave State 5\n");

  //-------------------------------------------------------
  // Update Proc Core sensors (for this tick)
  //-------------------------------------------------------

/* Not yet supported
  //-------------------------------------------------------
  // Update Centaur sensors (for this tick)
  //-------------------------------------------------------
  amec_update_centaur_sensors(CENTAUR_5);

  //-------------------------------------------------------
  // Update partition sensors for DPS algorithms (for this tick)
  //-------------------------------------------------------
  amec_dps_main();
*/
}


// Function Specification
//
// Name: amec_slv_state_6
//
// Description: AMEC Slave State Machine State
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_slv_state_6(void)
{
  AMEC_DBG("\tAMEC Slave State 6\n");

  //-------------------------------------------------------
  // Update Proc Core sensors (for this tick)
  //-------------------------------------------------------

/* Not yet supported
  //-------------------------------------------------------
  // Update Centaur sensors (for this tick)
  //-------------------------------------------------------
  amec_update_centaur_sensors(CENTAUR_6);
*/
}


// Function Specification
//
// Name: amec_slv_state_7
//
// Description: AMEC Slave State Machine State
//
// End Function Specification
void amec_slv_state_7(void)
{
  AMEC_DBG("\tAMEC Slave State 7\n");

  //-------------------------------------------------------
  // Update Proc Core sensors (for this tick)
  //-------------------------------------------------------

/* Not yet supported
  //-------------------------------------------------------
  // Update Centaur sensors (for this tick)
  //-------------------------------------------------------
  amec_update_centaur_sensors(CENTAUR_7);
*/
}

// Function Specification
//
// Name: amec_slv_substate_1_0
//       amec_slv_substate_1_1
//       amec_slv_substate_1_2
//       amec_slv_substate_1_3
//       amec_slv_substate_1_4
//       amec_slv_substate_1_5
//       amec_slv_substate_1_6
//       amec_slv_substate_1_7
//
// Description: slave substate amec_slv_substate_1_0
//              slave substate amec_slv_substate_1_1
//              slave substate amec_slv_substate_1_2
//              slave substate amec_slv_substate_1_3
//              slave substate amec_slv_substate_1_4
//              slave substate amec_slv_substate_1_5
//              slave substate amec_slv_substate_1_6
//              slave substate amec_slv_substate_1_7
//
// End Function Specification
void amec_slv_substate_1_0(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 1.0: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[0][0],
             G_sensor_update_pattern[0][1],
             G_sensor_update_pattern[0][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(0);
}

void amec_slv_substate_1_1(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 1.1: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[4][0],
             G_sensor_update_pattern[4][1],
             G_sensor_update_pattern[4][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(4);
}


void amec_slv_substate_1_2(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 1.2: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[0][0],
             G_sensor_update_pattern[0][1],
             G_sensor_update_pattern[0][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(0);
}

void amec_slv_substate_1_3(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 1.3: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[4][0],
             G_sensor_update_pattern[4][1],
             G_sensor_update_pattern[4][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(4);
}


void amec_slv_substate_1_4(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 1.4: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[0][0],
             G_sensor_update_pattern[0][1],
             G_sensor_update_pattern[0][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(0);
}

void amec_slv_substate_1_5(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 1.5: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[4][0],
             G_sensor_update_pattern[4][1],
             G_sensor_update_pattern[4][2]);


    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(4);
}


void amec_slv_substate_1_6(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 1.6: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[0][0],
             G_sensor_update_pattern[0][1],
             G_sensor_update_pattern[0][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(0);
}

void amec_slv_substate_1_7(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 1.7: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[4][0],
             G_sensor_update_pattern[4][1],
             G_sensor_update_pattern[4][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(4);
}


// Function Specification
//
// Name: amec_slv_substate_3_0
//       amec_slv_substate_3_1
//       amec_slv_substate_3_2
//       amec_slv_substate_3_3
//       amec_slv_substate_3_4
//       amec_slv_substate_3_5
//       amec_slv_substate_3_6
//       amec_slv_substate_3_7
//
// Description: slave substate amec_slv_substate_3_0
//              slave substate amec_slv_substate_3_1
//              slave substate amec_slv_substate_3_2
//              slave substate amec_slv_substate_3_3
//              slave substate amec_slv_substate_3_4
//              slave substate amec_slv_substate_3_5
//              slave substate amec_slv_substate_3_6
//              slave substate amec_slv_substate_3_7
//
// End Function Specification
void amec_slv_substate_3_0(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 3.0: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[1][0],
             G_sensor_update_pattern[1][1],
             G_sensor_update_pattern[1][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(1);
}

void amec_slv_substate_3_1(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 3.1: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[5][0],
             G_sensor_update_pattern[5][1],
             G_sensor_update_pattern[5][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(5);
}

void amec_slv_substate_3_2(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 3.2: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[1][0],
             G_sensor_update_pattern[1][1],
             G_sensor_update_pattern[1][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(1);
}

void amec_slv_substate_3_3(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 3.3: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[5][0],
             G_sensor_update_pattern[5][1],
             G_sensor_update_pattern[5][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(5);
}

void amec_slv_substate_3_4(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 3.4: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[1][0],
             G_sensor_update_pattern[1][1],
             G_sensor_update_pattern[1][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(1);
}

void amec_slv_substate_3_5(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 3.5: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[5][0],
             G_sensor_update_pattern[5][1],
             G_sensor_update_pattern[5][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(5);
}

void amec_slv_substate_3_6(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 3.6: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[1][0],
             G_sensor_update_pattern[1][1],
             G_sensor_update_pattern[1][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(1);
}


void amec_slv_substate_3_7(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 3.7: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[5][0],
             G_sensor_update_pattern[5][1],
             G_sensor_update_pattern[5][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(5);
}


// Function Specification
//
// Name: amec_slv_substate_5_0
//       amec_slv_substate_5_1
//       amec_slv_substate_5_2
//       amec_slv_substate_5_3
//       amec_slv_substate_5_4
//       amec_slv_substate_5_5
//       amec_slv_substate_5_6
//       amec_slv_substate_5_7
//
// Description: slave substate amec_slv_substate_5_0
//              slave substate amec_slv_substate_5_1
//              slave substate amec_slv_substate_5_2
//              slave substate amec_slv_substate_5_3
//              slave substate amec_slv_substate_5_4
//              slave substate amec_slv_substate_5_5
//              slave substate amec_slv_substate_5_6
//              slave substate amec_slv_substate_5_7
//
// End Function Specification
void amec_slv_substate_5_0(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 5.0: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[2][0],
             G_sensor_update_pattern[2][1],
             G_sensor_update_pattern[2][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(2);

    // Call processor-based thermal controller
    amec_controller_proc_thermal();
}

void amec_slv_substate_5_1(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 5.1: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[6][0],
             G_sensor_update_pattern[6][1],
             G_sensor_update_pattern[6][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(6);

    // Call controller on VRHOT signal from processor regulator
// @TODO - Verify VRM monitoring
//    amec_controller_vrhotproc();
}


void amec_slv_substate_5_2(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 5.2: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[2][0],
             G_sensor_update_pattern[2][1],
             G_sensor_update_pattern[2][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(2);
}

void amec_slv_substate_5_3(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 5.3: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[6][0],
             G_sensor_update_pattern[6][1],
             G_sensor_update_pattern[6][2]);


    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(6);
}


void amec_slv_substate_5_4(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 5.4: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[2][0],
             G_sensor_update_pattern[2][1],
             G_sensor_update_pattern[2][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(2);
}

void amec_slv_substate_5_5(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 5.5: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[6][0],
             G_sensor_update_pattern[6][1],
             G_sensor_update_pattern[6][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(6);
}


void amec_slv_substate_5_6(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 5.6: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[2][0],
             G_sensor_update_pattern[2][1],
             G_sensor_update_pattern[2][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(2);
}

void amec_slv_substate_5_7(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 5.7: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[6][0],
             G_sensor_update_pattern[6][1],
             G_sensor_update_pattern[6][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(6);
}


// Function Specification
//
// Name: amec_slv_substate_7_0
//       amec_slv_substate_7_1
//       amec_slv_substate_7_2
//       amec_slv_substate_7_3
//       amec_slv_substate_7_4
//       amec_slv_substate_7_5
//       amec_slv_substate_7_6
//       amec_slv_substate_7_7
//
// Description: slave substate amec_slv_substate_7_0
//              slave substate amec_slv_substate_7_1
//              slave substate amec_slv_substate_7_2
//              slave substate amec_slv_substate_7_3
//              slave substate amec_slv_substate_7_4
//              slave substate amec_slv_substate_7_5
//              slave substate amec_slv_substate_7_6
//              slave substate amec_slv_substate_7_7
//
// End Function Specification
void amec_slv_substate_7_0(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 7.0: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[3][0],
             G_sensor_update_pattern[3][1],
             G_sensor_update_pattern[3][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(3);

// @TODO - TEMP: Not Ready yet in Phase 1.
/*
    // Call memory thermal controller based on DIMM temperature
    amec_controller_dimm_thermal();

    // Call memory thermal controller based on Centaur temperature
    amec_controller_centaur_thermal();
*/

}

void amec_slv_substate_7_1(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 7.1: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[7][0],
             G_sensor_update_pattern[7][1],
             G_sensor_update_pattern[7][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(7);
}

void amec_slv_substate_7_2(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 7.2: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[3][0],
             G_sensor_update_pattern[3][1],
             G_sensor_update_pattern[3][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(3);
}

void amec_slv_substate_7_3(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 7.3: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[7][0],
             G_sensor_update_pattern[7][1],
             G_sensor_update_pattern[7][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(7);
}

void amec_slv_substate_7_4(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 7.4: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[3][0],
             G_sensor_update_pattern[3][1],
             G_sensor_update_pattern[3][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(3);
}

void amec_slv_substate_7_5(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 7.5: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[7][0],
             G_sensor_update_pattern[7][1],
             G_sensor_update_pattern[7][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(7);
}

void amec_slv_substate_7_6(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 7.6: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[3][0],
             G_sensor_update_pattern[3][1],
             G_sensor_update_pattern[3][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(3);
}


void amec_slv_substate_7_7(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 7.7: Core Data[%d,      %d, %d]\n",
             G_sensor_update_pattern[7][0],
             G_sensor_update_pattern[7][1],
             G_sensor_update_pattern[7][2]);

    //-------------------------------------------------------
    // Update Proc Core sensors (for this substate)
    //-------------------------------------------------------
    amec_update_proc_core_group(7);

    // Call health monitor to check for processor error temperature conditions
    amec_health_check_proc_temp();
}

void amec_update_proc_core_group(uint8_t group)
{
    int i;
    for(i =0; i < CORES_PER_STATE; i++)
    {
        amec_update_proc_core_sensors(G_sensor_update_pattern[group][i]);
    }
}


/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
