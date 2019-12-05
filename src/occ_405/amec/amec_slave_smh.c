/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_slave_smh.c $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2019                        */
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
#include <memory.h>
#include <amec_sensors_ocmb.h>
#include <amec_sensors_fw.h>
#include <amec_freq.h>
#include <amec_data.h>
#include <amec_amester.h>
#include <amec_oversub.h>
#include <amec_health.h>
#include <common.h>
#include <occhw_async.h>
#include <wof.h>
#include <pgpe_interface.h>
#include <state.h>                      // For CURRENT_STATE(), OCC_STATE_*
#include <cmdh_fsp_cmds_datacnfg.h>     // For DATA_get_present_cnfgdata()
#include <sensor_main_memory.h>         // For main_mem_sensors_*()
#include <sensor_inband_cmd.h>          // For inband_command_*()

//*************************************************************************/
// Externs
//*************************************************************************/
extern dcom_slv_inbox_t G_dcom_slv_inbox_rx;
extern opal_proc_voting_reason_t G_amec_opal_proc_throt_reason;
extern uint16_t G_proc_fmax_mhz;
extern GpeRequest G_wof_vrt_req;
extern bool G_pgpe_shared_sram_V_I_readings;

//*************************************************************************
// Macros
//*************************************************************************/

//*************************************************************************/
// Defines/Enums
//*************************************************************************/
smh_state_t G_amec_slv_state = {AMEC_INITIAL_STATE,
                                AMEC_INITIAL_STATE,
                                AMEC_INITIAL_STATE};

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
// Each substate runs every 64th tick time = 64 * MICS_PER_TICK
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
// AMEC Slave State 2 Substate Table
// --------------------------------------------------------
// Each substate runs every 64th tick time = 64 * MICS_PER_TICK
//
// No Substates
//
const smh_tbl_t amec_slv_state_2_substate_table[AMEC_SMH_STATES_PER_LVL] =
{
  {amec_slv_substate_2_even, NULL}, // Substate 2.0
  {NULL,                     NULL}, // Substate 2.1 (not used)
  {amec_slv_substate_2_even, NULL}, // Substate 2.2
  {NULL,                     NULL}, // Substate 2.3 (not used)
  {amec_slv_substate_2_even, NULL}, // Substate 2.4
  {NULL,                     NULL}, // Substate 2.5 (not used)
  {amec_slv_substate_2_even, NULL}, // Substate 2.6
  {NULL,                     NULL}, // Substate 2.7 (not used)
};

// --------------------------------------------------------
// AMEC Slave State 3 Substate Table
// --------------------------------------------------------
// Each substate runs every 64th tick time = 64 * MICS_PER_TICK
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
// Each substate runs every 64th tick time = 64 * MICS_PER_TICK
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
// AMEC Slave State 6 Substate Table
// --------------------------------------------------------
// Each substate runs every 64th tick time = 64 * MICS_PER_TICK
//
// No Substates
//
const smh_tbl_t amec_slv_state_6_substate_table[AMEC_SMH_STATES_PER_LVL] =
{
  {amec_slv_substate_6_all, NULL},  // Substate 6.0
  {amec_slv_substate_6_all, NULL},  // Substate 6.1
  {amec_slv_substate_6_all, NULL},  // Substate 6.2
  {amec_slv_substate_6_all, NULL},  // Substate 6.3
  {amec_slv_substate_6_all, NULL},  // Substate 6.4
  {amec_slv_substate_6_all, NULL},  // Substate 6.5
  {amec_slv_substate_6_all, NULL},  // Substate 6.6
  {amec_slv_substate_6_all, NULL},  // Substate 6.7
};

// --------------------------------------------------------
// AMEC Slave State 7 Substate Table
// --------------------------------------------------------
// Each substate runs every 64th tick time = 64 * MICS_PER_TICK
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
// Each function inside this state table runs once every 8th tick
//
const smh_tbl_t amec_slv_state_table[AMEC_SMH_STATES_PER_LVL] =
{
  {amec_slv_state_0, NULL},
  {amec_slv_state_1, amec_slv_state_1_substate_table},
  {amec_slv_state_2, amec_slv_state_2_substate_table},
  {amec_slv_state_3, amec_slv_state_3_substate_table},
  {amec_slv_state_4, NULL},
  {amec_slv_state_5, amec_slv_state_5_substate_table},
  {amec_slv_state_6, amec_slv_state_6_substate_table},
  {amec_slv_state_7, amec_slv_state_7_substate_table},
};


// This sets up the function pointer that will be called to update the
// fw timings when the AMEC Slave State Machine finishes.
smh_state_timing_t G_amec_slv_state_timings = {amec_slv_update_smh_sensors};

//*************************************************************************/
// Structures
//*************************************************************************/

//*************************************************************************/
// Globals
//*************************************************************************/
extern bool G_gpu_config_done;

//*************************************************************************/
// Function Prototypes
//*************************************************************************/

//*************************************************************************/
// Functions
//*************************************************************************/


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
    uint32_t    l_pmax_rail_freq = g_amec->proc[0].pwr_votes.apss_pmax_clip_freq;
    static bool L_lower_pmax_rail = FALSE;
    static bool L_raise_pmax_rail = TRUE;

    /*------------------------------------------------------------------------*/
    /*  Code                                                                  */
    /*------------------------------------------------------------------------*/

    do
    {
        // Don't do this during a state transition
        if( SMGR_is_state_transitioning() )
        {
            break;
        }

        if (G_apss_lower_pmax_rail == TRUE)
        {
            if (L_lower_pmax_rail == FALSE)
            {
                // Lower the Pmax_rail to nominal
                l_pmax_rail_freq = G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL];

                // Let the slave voting box handle the clip
                // to take in account all reasons for clip changes
                TRAC_INFO("amec_slv_check_apss_fail: Updating apss pmax clip freq to nominal");

                L_lower_pmax_rail = TRUE;
                L_raise_pmax_rail = FALSE;
            }
        }
        else
        {
            if (L_raise_pmax_rail == FALSE)
            {
                // Raise the Pmax rail back
                l_pmax_rail_freq = G_proc_fmax_mhz;

                // Let the slave voting box handle the clip
                // to take in account all reasons for clip changes
                TRAC_INFO("amec_slv_check_apss_fail: Updating apss pmax clip freq to fmax");

                L_lower_pmax_rail = FALSE;
                L_raise_pmax_rail = TRUE;
            }
        }

        // Store the frequency vote for the voting box
        g_amec->proc[0].pwr_votes.apss_pmax_clip_freq = l_pmax_rail_freq;

    } while(0);
}


// Function Specification
//
// Name: amec_slv_update_main_mem_sensors
//
// Description: Initializes and updates the main memory sensors.
//
// End Function Specification
void amec_slv_update_main_mem_sensors(void)
{
    // Don't initialize/update main memory sensors during a state transition
    if (SMGR_is_state_transitioning())
    {
        return;
    }

    // If main memory sensors have not been initialized
    if (!G_main_mem_sensors_initialized)
    {
        // Make sure OCC role has been set.  Role required for initialization.
        if (DATA_get_present_cnfgdata() & DATA_MASK_SET_ROLE)
        {
            // Initialize main memory sensors.  Write static sensor data to main
            // memory.  Must call multiple times due to BCE copy size
            // limitations.  Sets G_main_mem_sensors_initialized to true when
            // all initialization is done.
            main_mem_sensors_init();
        }
    }
    else
    {
        // Main memory sensors have been initialized.  Check if the OCC is in
        // the proper state to update the sensor readings in main memory.
        if ((CURRENT_STATE() == OCC_STATE_OBSERVATION)    ||
            (CURRENT_STATE() == OCC_STATE_ACTIVE)         ||
            (CURRENT_STATE() == OCC_STATE_CHARACTERIZATION))
        {
            // Update main memory sensors.  Write dynamic sensor readings data
            // to main memory.
            main_mem_sensors_update();
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
    AMEC_DBG("\tAMEC Slave Pre-State Common\n");

    // Update the FW Worst Case sensors every tick
    amec_update_fw_sensors();

    // Update the sensors that come from the APSS every tick
    // If sensors were not updated due to EPOW event, skip remaining tasks
    if (amec_update_apss_sensors())
    {
        // Read the AVS Bus sensors (Vdd / Vdn)
        amec_update_avsbus_sensors();

        // Over-subscription check
        amec_oversub_check();
    }
}

// Function Specification
//
// Name: amec_slv_common_tasks_post
//
// Description: Runs all the functions that need to run post-AMEC-State-Machine
//              This function will run every tick.
//
// Thread: RealTime Loop
//
// End Function Specification
void amec_slv_common_tasks_post(void)
{
  static bool L_active_1tick = FALSE;

  AMEC_DBG("\tAMEC Slave Post-State Common\n");

  // Only execute if OCC is in the active state
  if ( IS_OCC_STATE_ACTIVE() )
  {
      // wait 1 tick after going active to allow pwr sensors to be updated
      if(L_active_1tick)
      {
        // Check if we need to change Pmax_clip register setting due to not
        // getting any APSS data from Master
        amec_slv_check_apss_fail();

        // Call amec_power_control
        amec_power_control();

        // Call the OCC slave's processor voting box
        amec_slv_proc_voting_box();

        // Call the frequency state machine
        amec_slv_freq_smh();

        // Call the OCC slave's memory voting box
        amec_slv_mem_voting_box();

        // Call the OCC slave's performance check
        amec_slv_check_perf();

        //-------------------------------------------------------
        // Run WOF Algorithm if we are getting readings from PGPE
        // Else WOF will run in slave state 4
        //-------------------------------------------------------
        if(G_pgpe_shared_sram_V_I_readings)
        {
            call_wof_main();
        }

        // Call the every tick trace recording if it has been configured via Amester.
        // If not configured, this call will return immediately.
        amec_tb_record(AMEC_TB_EVERY_TICK);
      }
      else
      {
         L_active_1tick = TRUE;
      }

  }
  // if an OPAL system & just transitioned to CHAR or OBS state, set proc mnfg override
  else if ( (IS_OCC_STATE_CHARACTERIZATION() || IS_OCC_STATE_OBSERVATION()) &&
            (MANUFACTURING_OVERRIDE != G_amec_opal_proc_throt_reason)       &&
            (G_sysConfigData.system_type.kvm == true) )
  {
      G_amec_opal_proc_throt_reason = MANUFACTURING_OVERRIDE;
      ssx_semaphore_post(&G_dcomThreadWakeupSem);
  }
  // Check if an inband command is being processed
  if(G_inband_occ_cmd_state != INBAND_OCC_CMD_NONE)
  {
      // call inband command handler if in active, char or obs state
      if ( IS_OCC_STATE_ACTIVE() || IS_OCC_STATE_OBSERVATION() ||
           IS_OCC_STATE_CHARACTERIZATION() )
      {
         inband_command_handler();
      }
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
  static uint8_t L_membuf_addr = 0; // used to handle reading 16 OCMBs one per slave state over 8 slave states
  //-------------------------------------------------------
  // Update memory buffer sensors (for this tick)
  //-------------------------------------------------------
  if(MEM_TYPE_OCM == G_sysConfigData.mem_type)
  {
      amec_update_ocmb_sensors(MEMBUF_0+L_membuf_addr);
      if(L_membuf_addr)
          L_membuf_addr = 0;
      else
          L_membuf_addr = 8;
  }

  //-------------------------------------------------------
  // Update vector sensors
  //-------------------------------------------------------
  sensor_vector_update(AMECSENSOR_PTR(TEMPPROCAVG),  1);
  sensor_vector_update(AMECSENSOR_PTR(TEMPPROCTHRM), 1);
  sensor_vector_update(AMECSENSOR_PTR(FREQA),        1);
  sensor_vector_update(AMECSENSOR_PTR(IPS),     1);
  sensor_vector_update(AMECSENSOR_PTR(UTIL),         1);

  // Call the trace function for every 8th tick tracing if it has been configured via
  // Amester. If not configured, this call will return immediately.
  amec_tb_record(AMEC_TB_EVERY_8TH_TICK);
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
    static uint8_t L_membuf_addr = 0; // used to handle reading 16 OCMBs one per slave state over 8 slave states
    //-------------------------------------------------------
    // Update memory buffer sensors (for this tick)
    //-------------------------------------------------------
    if(MEM_TYPE_OCM == G_sysConfigData.mem_type)
    {
        amec_update_ocmb_sensors(MEMBUF_1+L_membuf_addr);
        if(L_membuf_addr)
            L_membuf_addr = 0;
        else
        {
            L_membuf_addr = 8;
            // update Proc level sensors
            amec_update_ocmb_temp_sensors();
        }
    }
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
  static uint8_t L_membuf_addr = 0; // used to handle reading 16 OCMBs one per slave state over 8 slave states
  //-------------------------------------------------------
  // Update memory buffer sensors (for this tick)
  //-------------------------------------------------------
  if(MEM_TYPE_OCM == G_sysConfigData.mem_type)
  {
      amec_update_ocmb_sensors(MEMBUF_2+L_membuf_addr);
      if(L_membuf_addr)
          L_membuf_addr = 0;
      else
          L_membuf_addr = 8;
  }

  // Call VRM Vdd thermal controller
  amec_controller_vrm_vdd_thermal();
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
  static uint8_t L_membuf_addr = 0; // used to handle reading 16 OCMBs one per slave state over 8 slave states
  //-------------------------------------------------------
  // Update memory buffer sensors (for this tick)
  //-------------------------------------------------------
  if(MEM_TYPE_OCM == G_sysConfigData.mem_type)
  {
      amec_update_ocmb_sensors(MEMBUF_3+L_membuf_addr);
      if(L_membuf_addr)
          L_membuf_addr = 0;
      else
          L_membuf_addr = 8;
  }
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
  static uint8_t L_membuf_addr = 0; // used to handle reading 16 OCMBs one per slave state over 8 slave states
  //-------------------------------------------------------
  // Update memory buffer sensors (for this tick)
  //-------------------------------------------------------
  if(MEM_TYPE_OCM == G_sysConfigData.mem_type)
  {
      amec_update_ocmb_sensors(MEMBUF_4+L_membuf_addr);
      if(L_membuf_addr)
          L_membuf_addr = 0;
      else
          L_membuf_addr = 8;
  }

  //-------------------------------------------------------
  // Run WOF Algorithm if we are NOT getting readings from PGPE
  // Else WOF will run every tick with new PGPE readings
  //-------------------------------------------------------
  if(!G_pgpe_shared_sram_V_I_readings)
  {
      call_wof_main();
  }

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
  static uint8_t L_membuf_addr = 0; // used to handle reading 16 OCMBs one per slave state over 8 slave states
  //-------------------------------------------------------
  // Update memory buffer sensors (for this tick)
  //-------------------------------------------------------
  if(MEM_TYPE_OCM == G_sysConfigData.mem_type)
  {
      amec_update_ocmb_sensors(MEMBUF_5+L_membuf_addr);
      if(L_membuf_addr)
          L_membuf_addr = 0;
      else
          L_membuf_addr = 8;
  }

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
  static uint8_t L_membuf_addr = 0; // used to handle reading 16 OCMBs one per slave state over 8 slave states
  //-------------------------------------------------------
  // Update memory buffer sensors (for this tick)
  //-------------------------------------------------------
  if(MEM_TYPE_OCM == G_sysConfigData.mem_type)
  {
      amec_update_ocmb_sensors(MEMBUF_6+L_membuf_addr);
      if(L_membuf_addr)
          L_membuf_addr = 0;
      else
          L_membuf_addr = 8;
  }
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
  static uint8_t L_membuf_addr = 0; // used to handle reading 16 OCMBs one per slave state over 8 slave states
  //-------------------------------------------------------
  // Update memory buffer sensors (for this tick)
  //-------------------------------------------------------
  if(MEM_TYPE_OCM == G_sysConfigData.mem_type)
  {
      amec_update_ocmb_sensors(MEMBUF_7+L_membuf_addr);
      if(L_membuf_addr)
          L_membuf_addr = 0;
      else
          L_membuf_addr = 8;
  }
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
// Name: amec_slv_substate_2_even
//
// Description: even numbered slave substates
//              gives state 2 substate function to be called every 16th tick
//              Time = 16 * MICS_PER_TICK
//              odd substates of state 2 are not currently used
//
// End Function Specification
void amec_slv_substate_2_even(void)
{
    AMEC_DBG("\tAMEC Slave State 2 even substate\n");
    amec_slv_update_main_mem_sensors();
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
// Name: amec_slv_substate_6_all
//
// Description: Called for every substate of state 6
//              gives state 6 substate function to be called every 8th tick
//              Time = 8 * MICS_PER_TICK
//
// End Function Specification
void amec_slv_substate_6_all(void)
{
    AMEC_DBG("\tAMEC Slave State 6 substate\n");
    inband_command_check();
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

    // Call memory thermal controller based on DIMM temperature
    amec_controller_dimm_thermal();

    if(MEM_TYPE_OCM == G_sysConfigData.mem_type)
    {
        // Call memory thermal controller based on membuf temperature
        amec_controller_membuf_thermal();
    }
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
