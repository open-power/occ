/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_init.c $                                    */
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
#include <amec_sys.h>
#include <ssx.h>
#include <errl.h>               // Error logging
#include <rtls.h>
#include <occ_sys_config.h>
#include <occ_service_codes.h>  // for SSX_GENERIC_FAILURE
#include <trac.h>
#include "state.h"
#include "amec_service_codes.h"
#include <amec_sys.h>
#include <proc_data.h>
#include <sensor.h>

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
// Globals
//*************************************************************************
// We can initialize amec system structure to all zeros
amec_sys_t g_amec_sys = {0};

// Initialize g_amec to point to g_amec_sys
// We use this pointer to keep the amec code as similar to previous projects
amec_sys_t * g_amec = &g_amec_sys;

// GPE Request Structure that is used to measure the worst case GPE timings
PoreFlex G_gpe_nop_request[NUM_GPE_ENGINES];

extern PoreEntryPoint GPE_pore_nop;
extern void amec_slv_update_gpe_sensors(uint8_t i_gpe_engine);
extern void amec_slv_update_gpe_sensors(uint8_t i_gpe_engine);

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************
void amec_vectorize_core_sensor(sensor_t * l_sensor,
                                vectorSensor_t * l_vector,
                                const VECTOR_SENSOR_OP l_op,
                                uint16_t l_sensor_elem_array_gsid)
{
#define VECTOR_CREATE_FAILURE   1
#define VECTOR_ADD_ELEM_FAILURE 2

  int l_idx = 0;    // Used to index the for loops for vector create
  int l_rc  = 0;    // Indicates failure to add a sensor to vector
  uint16_t l_gsid  = 0xFFFF;
  errlHndl_t l_err = NULL;

  do
  {
    // Grab GSID for errl in case of failure
    l_gsid = l_sensor->gsid;

    // Vectorize the sensor
    sensor_vectorize(l_sensor,
        l_vector,
        l_op);

    // If vectorize worked, add elements to the vector sensor
    if(NULL != l_sensor->vector)
    {
      // Loop through cores
      for(l_idx = 0; l_idx < MAX_NUM_CORES; l_idx++)
      {
        // Add elements to the vector sensor
        sensor_vector_elem_add(l_sensor->vector,
                               l_idx,
                               AMECSENSOR_ARRAY_PTR(l_sensor_elem_array_gsid, l_idx));
        // If core is not present, disable this vector element
        if(!CORE_PRESENT(l_idx))
        {
          sensor_vector_elem_enable(l_sensor->vector,
                                    l_idx,
                                    0 /* Disable */);
        }
      }

      // Sanity check, we should have MAX_NUM_CORES entries in
      // vector sensor
      if(l_sensor->vector->size != MAX_NUM_CORES)
      {
        // Set l_rc and break out so that we can create an errl
        l_rc = VECTOR_ADD_ELEM_FAILURE;
        break;
      }
    }
    else
    {
      // Set l_rc and break out so that we can create an errl
      l_rc = VECTOR_CREATE_FAILURE;
      break;
    }
  }while(0);

  if(l_rc)
  {
    //If fail to create pore flex object then there is a problem.
    TRAC_ERR("Failed to vectorize sensor[0x%x, 0x%x]", l_gsid, l_rc );

    /* @
     * @errortype
     * @moduleid    AMEC_VECTORIZE_FW_SENSORS
     * @reasoncode  SSX_GENERIC_FAILURE
     * @userdata1   return code
     * @userdata2   gsid of failed sensor
     * @userdata4   OCC_NO_EXTENDED_RC
     * @devdesc     Firmware failure in call to vectorize sensor
     */
    l_err = createErrl(
        AMEC_VECTORIZE_FW_SENSORS,      //modId
        SSX_GENERIC_FAILURE,            //reasoncode
        OCC_NO_EXTENDED_RC,             //Extended reason code
        ERRL_SEV_UNRECOVERABLE,         //Severity
        NULL,//TODO: create trace       //Trace Buf
        DEFAULT_TRACE_SIZE,             //Trace Size
        l_rc,                           //userdata1
        l_gsid                          //userdata2
        );

    REQUEST_RESET(l_err);
  }
}

void amec_init_vector_sensors(void)
{

#define VECTOR_CREATE_FAILURE   1
#define VECTOR_ADD_ELEM_FAILURE 2

  //-----------------------------------------------------
  // TEMP2MSP0 Vector Sensor
  //-----------------------------------------------------
  amec_vectorize_core_sensor(AMECSENSOR_PTR(TEMP2MSP0),
      &g_amec_sys.proc[0].temp2ms_vector,
      VECTOR_OP_AVG,
      TEMP2MSP0C0);

  //-----------------------------------------------------
  // FREQA2MSP0 Vector Sensor
  //-----------------------------------------------------
  amec_vectorize_core_sensor(AMECSENSOR_PTR(FREQA2MSP0),
      &g_amec_sys.proc[0].freqa2ms_vector,
      VECTOR_OP_AVG,
      FREQA2MSP0C0);

  //-----------------------------------------------------
  // IPS2MSP0 Vector Sensor
  //-----------------------------------------------------
  amec_vectorize_core_sensor(AMECSENSOR_PTR(IPS2MSP0),
      &g_amec_sys.proc[0].ips2ms_vector,
      VECTOR_OP_AVG,
      IPS2MSP0C0);

  //-----------------------------------------------------
  // TEMP2MSP0PEAK Vector Sensor
  //-----------------------------------------------------
  amec_vectorize_core_sensor(AMECSENSOR_PTR(TEMP2MSP0PEAK),
      &g_amec_sys.proc[0].temp2mspeak_vector,
      VECTOR_OP_MAX,
      TEMP2MSP0C0);

  //-----------------------------------------------------
  // UTIL2MSP0 Vector Sensor
  //-----------------------------------------------------
  amec_vectorize_core_sensor(AMECSENSOR_PTR(UTIL2MSP0),
      &g_amec_sys.proc[0].util2ms_vector,
      VECTOR_OP_AVG,
      UTIL2MSP0C0);

//TODO: Re-enable with error checking when centaur support is added
#if 0
  int l_rc = 0, l_idx = 0, l_idx2 = 0;    // Used to index the for loops for vector create
  //-----------------------------------------------------
  // MEMSP2MSP0 Vector Sensor
  //-----------------------------------------------------
  sensor_vectorize(AMECSENSOR_PTR(MEMSP2MSP0),
      &g_amec_sys.proc[0].memsp2ms_vector,
      VECTOR_OP_MIN);

  for(l_idx=0; l_idx<MAX_NUM_MEM_CONTROLLERS; l_idx++)
  {
    for(l_idx2=0; l_idx2<NUM_PORT_PAIRS_PER_CENTAUR; l_idx2++)
    {
      sensor_vector_elem_add(AMECSENSOR_PTR(MEMSP2MSP0)->vector,
          l_idx,
          AMECSENSOR_2D_ARRAY_PTR(MEMSP2MSPM0C0P0,l_idx, l_idx2));
    }
  }
#endif
}

// Function Specification
//
// Name:  amec_init_gamec_struct
//
// Description:  Perform initialization of g_amec structure
//
// End Function Specification
void amec_init_gamec_struct(void)
{
  /*------------------------------------------------------------------------*/
  /*  Local Variables                                                       */
  /*------------------------------------------------------------------------*/
  uint16_t l_idx = 0;

  /*------------------------------------------------------------------------*/
  /*  Code                                                                  */
  /*------------------------------------------------------------------------*/

  // Defaul the frequency range to something safe
  g_amec->sys.fmin = 2000;
  g_amec->sys.fmax = 2000;
  g_amec->sys.max_speed = 1000;

  g_amec->sys.min_speed = 400;
  g_amec->sys.speed_step = 10;
  g_amec->sys.speed_step_limit = (uint16_t)((65535/4)/(g_amec->sys.speed_step));

  // Initialize thermal controller for processor
  g_amec->thermalproc.setpoint = 850; // change to 850 = 85.0 C
  g_amec->thermalproc.Pgain = 1000;
  g_amec->thermalproc.speed_request = 1000;
  g_amec->thermalproc.freq_request = -1; //unconstrained frequency vote
  g_amec->thermalproc.total_res = 0;

  // Initialize thermal controller based on DIMM temperatures
  g_amec->thermaldimm.setpoint = 850; //In 0.1 degrees C -> 850 = 85.0 C
  g_amec->thermaldimm.Pgain = 30000;
  g_amec->thermaldimm.speed_request = AMEC_MEMORY_MAX_STEP;

  // Initialize thermal controller based on Centaur temperatures
  g_amec->thermalcent.setpoint = 850; //In 0.1 degrees C -> 850 = 85.0 C
  g_amec->thermalcent.Pgain = 30000;
  g_amec->thermalcent.speed_request = AMEC_MEMORY_MAX_STEP;

  // Initialize controler based on VRHOT signal from processor regulator
  g_amec->vrhotproc.setpoint = 100;
  g_amec->vrhotproc.freq_request = -1;
  g_amec->vrhotproc.speed_request = 1000;

  // Initialize partition information
  amec_part_init();

  // Initialize performace counter
  for (l_idx=0; l_idx<MAX_NUM_CORES; l_idx++)
  {
    amec_core_perf_counter_ctor(&g_amec->proc[0].core[l_idx].core_perf, 0, l_idx);
  }

  //Initialize processor fields
  g_amec->proc[0].core_max_freq = G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO];

  //Initialize processor power votes
  g_amec->proc[0].pwr_votes.pmax_clip_freq = G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO];
  g_amec->proc[0].pwr_votes.apss_pmax_clip_freq = 0xFFFF;

  //Initialize stream buffer recording parameters
  g_amec->recordflag=0;      // Never enable recording until requested via Amester API call
  g_amec->r_cnt=0;           // Reset counter of 250us ticks
  g_amec->ptr_stream_buffer = &g_amec->stream_buffer[0];
  g_amec->stream_vector_mode=0;      // No recording yet
  g_amec->stream_vector_delay=0;     // Delay in msec before recording can begin
  g_amec->stream_vector_rate=0xff;   // Invalid setting: requires IPMI command to select initial rate

  //Initialize analytics parameters
  g_amec->analytics_group=45;        // Default to analytics Group 45
  g_amec->analytics_chip=0;          // Default to which chip to perform analytics on
  g_amec->analytics_bad_output_count=0;   // Number of frames to discard before recording analytics output
  g_amec->analytics_total_chips=MAX_NUM_CHIP_MODULES;  // Default to do all chips in the system
  g_amec->analytics_threadmode=1;    // Default is average of all N threads (may be altered with IPMI command)
  g_amec->analytics_threadcountmax=8;// Default is 8 threads per core (may be altered with IPMI command)
  g_amec->analytics_total_chips=4;   // For Tuleta force to only 2 DCM sockets, 4 chips
  g_amec->analytics_option=1;        // =0 means cycle through all chips, =1 means only work with analytics_chip
  g_amec->analytics_thermal_offset=0;// Reset offset to 0 for thermal output group
  g_amec->analytics_slot=4;          // Time slot associated with when the amec_analytics function is called (out of 8 slots)
  // Set entire averaging buffer to zero
  memset (&g_amec->g44_avg, 0, 4*(MAX_SENSORS_ANALYTICS*MAX_NUM_CHIP_MODULES));
  for(l_idx=0; l_idx<NUM_AMEC_FW_PROBES; l_idx++)
  {
     g_amec->ptr_probe250us[l_idx] = &g_amec->sys.pwr250us.sample;
     g_amec->size_probe250us[l_idx] = 2;     // Size of object pointed to by probe is 2 bytes
     g_amec->index_probe250us[l_idx] = 0;    // Initialize all offsets to 0 (used only if size > 2)
  }

//  g_amec->ptr_probe250us[1] = g_amec->proc[0].sleepcnt2ms.sample;
//  g_amec->ptr_probe250us[2] = &g_amec->g44_avg[(0*MSA)+49];
//  g_amec->ptr_probe250us[2] = g_amec->ptr_probe250us[2]+2;  // Point to low 16 bits of g44_avg
//  g_amec->ptr_probe250us[3] = &g_amec->proc[0].core[0].thread[0].util2ms_thread;
  g_amec->ptr_probe250us[1] = &g_amec->sys.pwr250us.sample;
  g_amec->ptr_probe250us[2] = &g_amec->r_cnt;
  g_amec->ptr_probe250us[2] = g_amec->ptr_probe250us[2]+2;  // Point to low 16 bits of r_cnt
  g_amec->ptr_probe250us[3] = &g_amec->r_cnt;
//  g_amec->ptr_probe250us[4] = &g_amec->testscom1;
//  g_amec->ptr_probe250us[5] = &g_amec->traffic_delay;       // holds loop delay for holding up memory traffic
//  g_amec->ptr_probe250us[6] = &g_amec->testscom1;
//  g_amec->ptr_probe250us[6] = g_amec->ptr_probe250us[6]+2; // Point to low 16 bits of testscom1
//  g_amec->ptr_probe250us[7] = &g_amec->task_centaur_data_count;

}

// Function Specification
//
// Name:  amec_slave_init
//
// Description:  Perform initialization of any/all AMEC Slave Functions
//
// End Function Specification
void amec_slave_init()
{
  errlHndl_t l_err = NULL;   // Error handler
  int         rc   = 0;      // Return code
  int         rc2  = 0;      // Return code

  // Set the GPE Request Pointers to NULL in case the create fails.
  G_fw_timing.gpe0_timing_request = NULL;
  G_fw_timing.gpe1_timing_request = NULL;

  // Initializes the GPE routine that will be used to measure the worst case
  // timings for GPE0
  rc  = pore_flex_create( &G_gpe_nop_request[0],        //gpe_req for the task
                          &G_pore_gpe0_queue,           //queue
                          (void *) GPE_pore_nop,        //entry point
                          (uint32_t)  NULL,             //parm for the task
                          SSX_WAIT_FOREVER,             //no timeout
                          (AsyncRequestCallback) amec_slv_update_gpe_sensors,    //callback
                          (void *) GPE_ENGINE_0,        //callback argument
                          ASYNC_CALLBACK_IMMEDIATE );   //options

  // Initializes the GPE routine that will be used to measure the worst case
  // timings for GPE1
  rc2 = pore_flex_create( &G_gpe_nop_request[1],        //gpe_req for the task
                          &G_pore_gpe1_queue,           //queue
                          (void *)GPE_pore_nop,         //entry point
                          (uint32_t) NULL,              //parm for the task
                          SSX_WAIT_FOREVER,             //no timeout
                          (AsyncRequestCallback) amec_slv_update_gpe_sensors,    //callback
                          (void *) GPE_ENGINE_1,        //callback argument
                          ASYNC_CALLBACK_IMMEDIATE );   //options

  // If we couldn't create the poreFlex objects, there must be a major problem
  // so we will log an error and halt OCC.
  if( rc || rc2 )
  {
    //If fail to create pore flex object then there is a problem.
    TRAC_ERR("Failed to create GPE duration poreFlex object[0x%x, 0x%x]", rc, rc2 );

    /* @
     * @errortype
     * @moduleid    AMEC_INITIALIZE_FW_SENSORS
     * @reasoncode  SSX_GENERIC_FAILURE
     * @userdata1   return code - gpe0
     * @userdata2   return code - gpe1
     * @userdata4   OCC_NO_EXTENDED_RC
     * @devdesc     Failure to create PORE-GPE poreFlex object for FW timing
     *              analysis.
     *
     */
    l_err = createErrl(
        AMEC_INITIALIZE_FW_SENSORS,         //modId
        SSX_GENERIC_FAILURE,                //reasoncode
        OCC_NO_EXTENDED_RC,                 //Extended reason code
        ERRL_SEV_PREDICTIVE,                //Severity
        NULL,    //TODO: create trace       //Trace Buf
        DEFAULT_TRACE_SIZE,                 //Trace Size
        rc,                                 //userdata1
        rc2                                 //userdata2
    );

    REQUEST_RESET(l_err);
  }
  else
  {
    // Everything was successful, so set FW timing pointers to these
    // GPE Request objects
    G_fw_timing.gpe0_timing_request = &G_gpe_nop_request[0];
    G_fw_timing.gpe1_timing_request = &G_gpe_nop_request[1];
  }

  // Initialize Vector Sensors for AMEC use
  amec_init_vector_sensors();

  // Initialize AMEC internal parameters
  amec_init_gamec_struct();
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
