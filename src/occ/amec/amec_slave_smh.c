/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_slave_smh.c $                               */
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
#include <amec_wof.h>

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

// --------------------------------------------------------
// AMEC Slave State 6 Substate Table
// --------------------------------------------------------
// Each function inside this state table runs once every 16ms.
//
// No Substates
//
const smh_tbl_t amec_slv_state_6_substate_table[AMEC_SMH_STATES_PER_LVL] =
{
  {amec_slv_substate_6_0, NULL},
  {amec_slv_substate_6_1, NULL},
  {amec_slv_substate_6_2, NULL},
  {amec_slv_substate_6_3, NULL},
  {amec_slv_substate_6_4, NULL},
  {amec_slv_substate_6_5, NULL},
  {amec_slv_substate_6_6, NULL},
  {amec_slv_substate_6_7, NULL},
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
  {amec_slv_state_1, NULL},
  {amec_slv_state_2, NULL},
  {amec_slv_state_3, NULL},
  {amec_slv_state_4, NULL},
  {amec_slv_state_5, NULL},
  {amec_slv_state_6, amec_slv_state_6_substate_table},
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
// Name: amec_slv_pstate_uplift_check
//
// Description: This function checks if the Global Pstate table needs to be
// modified with a voltage uplift. If an uplift has been requested, it will
// proceed to update every entry of the table.
//
// End Function Specification
void amec_slv_pstate_uplift_check(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/
    Pstate                      l_pmin = 0;
    Pstate                      l_pmax = 0;
    uint16_t                    i = 0;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    // Check if a new uplift request for Vdd has been made
    if ((G_sysConfigData.vdd_vid_delta != 0) ||
        (G_sysConfigData.vcs_vid_delta != 0))
    {
        TRAC_INFO("Updating Global Pstate table with requested uplift values!");

        // STEP 1:
        // Prevent any Pstate changes by locking the PMC Rail so that
        // Pmax_rail = Pmin_rail + 1
        l_pmin = gpst_pmin(&G_global_pstate_table) + 1;

        // Set the Pmax_rail register via OCI write
        amec_oversub_pmax_clip(l_pmin);

        // STEP 2:
        // Update all entries of the Global Pstate table using uplift provided
        for (i=0; i<G_global_pstate_table.entries; i++)
        {
            // Modify the fields associated with Vdd. Per HW procedure team,
            // the evid_vdd_eff and max_reg_vdd should get decremented
            G_global_pstate_table.pstate[i].fields.evid_vdd += G_sysConfigData.vdd_vid_delta;
            G_global_pstate_table.pstate[i].fields.evid_vdd_eff -= G_sysConfigData.vdd_vid_delta;
            G_global_pstate_table.pstate[i].fields.maxreg_vdd -= G_sysConfigData.vdd_vid_delta;

            // Modify the fields associated with Vcs. Per HW procedure team,
            // the evid_vcs_eff and max_reg_vcs should get decremented
            G_global_pstate_table.pstate[i].fields.evid_vcs += G_sysConfigData.vcs_vid_delta;
            G_global_pstate_table.pstate[i].fields.evid_vcs_eff -= G_sysConfigData.vcs_vid_delta;
            G_global_pstate_table.pstate[i].fields.maxreg_vcs -= G_sysConfigData.vcs_vid_delta;

            // Compute the ECC for this entry
            G_global_pstate_table.pstate[i].fields.ecc =
                gpstCheckByte(G_global_pstate_table.pstate[i].value);
        }

        // STEP 3:
        // Release the lock on the PMC Rail from Step 1
        l_pmax = gpst_pmax(&G_global_pstate_table);

        // Set the Pmax_rail register via OCI write
        amec_oversub_pmax_clip(l_pmax);

        // STEP 4:
        // In order to inform the HW about the new Global Pstate table, perform
        // a single +1 Pstate jump
        g_amec->pstate_foverride_enable = 1;
        if (g_amec->proc[0].core_max_freq ==
            G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO])
        {
            // If we are at turbo frequency, then perform a single -1 Pstate
            // jump instead
            g_amec->pstate_foverride = g_amec->proc[0].core_max_freq -
                (uint16_t)(G_global_pstate_table.frequency_step_khz / 1000);
        }
        else
        {
            g_amec->pstate_foverride = g_amec->proc[0].core_max_freq +
            (uint16_t)(G_global_pstate_table.frequency_step_khz / 1000);
        }

        // After updating Global Pstate table, reset the delta to 0
        G_sysConfigData.vdd_vid_delta = 0;
        G_sysConfigData.vcs_vid_delta = 0;
    }
    else
    {
        // Check if we updated the Global Pstate table
        if (g_amec->pstate_foverride_enable)
        {
            // STEP 5:
            // Go back to the initial Pstate by disabling the override enable
            // and maxing out the frequency request so it doesn't influence
            // the final vote in the voting box
            g_amec->pstate_foverride_enable = 0;
            g_amec->pstate_foverride = 0xFFFF;
        }
    }
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
  static uint16_t L_counter = 0;

  AMEC_DBG("\tAMEC Slave Pre-State Common\n");

  // Check if we need to apply a voltage uplift to the Global Pstate table
  amec_slv_pstate_uplift_check();

  // Update the FW Worst Case sensors every tick
  amec_update_fw_sensors();

  // Update the fast core data sensors every tick
  amec_update_fast_core_data_sensors();

  // Update the sensors that come from the APSS every tick
  amec_update_apss_sensors();

  // Update the external voltage sensors
  amec_update_external_voltage();

  // Update critical sensors for WOF algorithm
  amec_update_wof_sensors();

  // Call the stream buffer recording function
  amec_analytics_sb_recording();

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

  // Over-subscription check
  amec_oversub_check();
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

      // Call helper function for the WOF algorithm. This should be done after
      // the frequency selection in amec_slv_freq_smh().
      amec_wof_helper();

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
  amec_update_proc_core_sensors(CORE_0);
  amec_update_proc_core_sensors(CORE_8);

  //-------------------------------------------------------
  // Update Centaur sensors (for this tick)
  //-------------------------------------------------------
  amec_update_centaur_sensors(CENTAUR_0);

  //-------------------------------------------------------
  // Update Sleep Count & Winkle Count Sensors
  //-------------------------------------------------------
  sensor_update(AMECSENSOR_PTR(SLEEPCNT2MSP0),
                __builtin_popcount( g_amec->proc[0].sleep_cnt));
  g_amec->proc[0].sleep_cnt  = 0;

  sensor_update(AMECSENSOR_PTR(WINKCNT2MSP0),
                __builtin_popcount(g_amec->proc[0].winkle_cnt));
  g_amec->proc[0].winkle_cnt = 0;

  //-------------------------------------------------------
  // Update vector sensors
  //-------------------------------------------------------
  sensor_vector_update(AMECSENSOR_PTR(TEMP2MSP0),    1);
  sensor_vector_update(AMECSENSOR_PTR(TEMP2MSP0PEAK),1);
  sensor_vector_update(AMECSENSOR_PTR(FREQA2MSP0),   1);
  sensor_vector_update(AMECSENSOR_PTR(IPS2MSP0),     1);
  sensor_vector_update(AMECSENSOR_PTR(UTIL2MSP0),    1);

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
  amec_update_proc_core_sensors(CORE_1);
  amec_update_proc_core_sensors(CORE_9);

  //-------------------------------------------------------
  // Update Centaur sensors (for this tick)
  //-------------------------------------------------------
  amec_update_centaur_sensors(CENTAUR_1);

  //-------------------------------------------------------
  // Update Proc Level Centaur/DIMM Temperature sensors
  //-------------------------------------------------------
  amec_update_centaur_temp_sensors();
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
  amec_update_proc_core_sensors(CORE_2);
  amec_update_proc_core_sensors(CORE_10);

  //-------------------------------------------------------
  // Update Centaur sensors (for this tick)
  //-------------------------------------------------------
  amec_update_centaur_sensors(CENTAUR_2);
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
  amec_update_proc_core_sensors(CORE_3);
  amec_update_proc_core_sensors(CORE_11);

  //-------------------------------------------------------
  // Update Centaur sensors (for this tick)
  //-------------------------------------------------------
  amec_update_centaur_sensors(CENTAUR_3);

  //-------------------------------------------------------
  // Perform amec_analytics (set amec_analytics_slot to 3)
  //-------------------------------------------------------
  amec_analytics_main();
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
  amec_update_proc_core_sensors(CORE_4);

  //-------------------------------------------------------
  // Update Centaur sensors (for this tick)
  //-------------------------------------------------------
  amec_update_centaur_sensors(CENTAUR_4);

  if ( IS_OCC_STATE_ACTIVE() )
  {
      // Execute the main WOF algorithm (Workload-Optimized Frequency
      // selection)
      amec_wof_main();
  }

  // Call the trace function for 2ms tracing if it has been configured via
  // Amester. If not configured, this call will return immediately.
  amec_tb_record(AMEC_TB_2MS);
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
  amec_update_proc_core_sensors(CORE_5);

  //-------------------------------------------------------
  // Update Centaur sensors (for this tick)
  //-------------------------------------------------------
  amec_update_centaur_sensors(CENTAUR_5);

  //-------------------------------------------------------
  // Update partition sensors for DPS algorithms (for this tick)
  //-------------------------------------------------------
  amec_dps_main();
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
  amec_update_proc_core_sensors(CORE_6);

  //-------------------------------------------------------
  // Update Centaur sensors (for this tick)
  //-------------------------------------------------------
  amec_update_centaur_sensors(CENTAUR_6);
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
  amec_update_proc_core_sensors(CORE_7);

  //-------------------------------------------------------
  // Update Centaur sensors (for this tick)
  //-------------------------------------------------------
  amec_update_centaur_sensors(CENTAUR_7);
}

// Function Specification
//
// Name: amec_slv_substate_6_0
//       amec_slv_substate_6_1
//       amec_slv_substate_6_2
//       amec_slv_substate_6_3
//       amec_slv_substate_6_4
//       amec_slv_substate_6_5
//       amec_slv_substate_6_6
//       amec_slv_substate_6_7
//
// Description: slave substate amec_slv_substate_6_0
//              slave substate amec_slv_substate_6_1
//              slave substate amec_slv_substate_6_2
//              slave substate amec_slv_substate_6_3
//              slave substate amec_slv_substate_6_4
//              slave substate amec_slv_substate_6_5
//              slave substate amec_slv_substate_6_6
//              slave substate amec_slv_substate_6_7
//
// End Function Specification
void amec_slv_substate_6_0(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 6.0\n");

    // Call processor-based thermal controller
    amec_controller_proc_thermal();
}

void amec_slv_substate_6_1(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 6.1\n");

    // Call controller on VRHOT signal from processor regulator
    amec_controller_vrhotproc();
}

void amec_slv_substate_6_2(void){AMEC_DBG("\tAMEC Slave State 6.2\n");}
void amec_slv_substate_6_3(void){AMEC_DBG("\tAMEC Slave State 6.3\n");}
void amec_slv_substate_6_4(void){AMEC_DBG("\tAMEC Slave State 6.4\n");}
void amec_slv_substate_6_5(void){AMEC_DBG("\tAMEC Slave State 6.5\n");}
void amec_slv_substate_6_6(void){AMEC_DBG("\tAMEC Slave State 6.6\n");}
void amec_slv_substate_6_7(void){AMEC_DBG("\tAMEC Slave State 6.7\n");}

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
    AMEC_DBG("\tAMEC Slave State 7.0\n");

    // Call memory thermal controller based on DIMM temperature
    amec_controller_dimm_thermal();

    // Call memory thermal controller based on Centaur temperature
    amec_controller_centaur_thermal();
}

void amec_slv_substate_7_1(void){AMEC_DBG("\tAMEC Slave State 7.1\n");}
void amec_slv_substate_7_2(void){AMEC_DBG("\tAMEC Slave State 7.2\n");}
void amec_slv_substate_7_3(void){AMEC_DBG("\tAMEC Slave State 7.3\n");}
void amec_slv_substate_7_4(void){AMEC_DBG("\tAMEC Slave State 7.4\n");}
void amec_slv_substate_7_5(void){AMEC_DBG("\tAMEC Slave State 7.5\n");}
void amec_slv_substate_7_6(void){AMEC_DBG("\tAMEC Slave State 7.6\n");}

void amec_slv_substate_7_7(void)
{
    /*------------------------------------------------------------------------*/
    /*  Local Variables                                                       */
    /*------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/
    AMEC_DBG("\tAMEC Slave State 7.7\n");

    // Call health monitor to check for processor error temperature conditions
    amec_health_check_proc_temp();
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
