/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_init.c $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2020                        */
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

//*************************************************************************/
// Externs
//*************************************************************************/

//*************************************************************************/
// Macros
//*************************************************************************/

//*************************************************************************/
// Defines/Enums
//*************************************************************************/

//*************************************************************************/
// Structures
//*************************************************************************/

//*************************************************************************/
// Globals
//*************************************************************************/
// We can initialize amec system structure to all zeros
amec_sys_t g_amec_sys = {0};

// Initialize g_amec to point to g_amec_sys
// We use this pointer to keep the amec code as similar to previous projects
amec_sys_t * g_amec = &g_amec_sys;

// GPE Request Structure that is used to measure the worst case GPE timings
GpeRequest G_gpe_nop_request[NUM_GPE_ENGINES];

extern void amec_slv_update_gpe_sensors(uint8_t i_gpe_engine);
extern void amec_slv_update_gpe_sensors(uint8_t i_gpe_engine);

//*************************************************************************/
// Function Prototypes
//*************************************************************************/

//*************************************************************************/
// Functions
//*************************************************************************/
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
        NULL,                           //Trace Buf
        DEFAULT_TRACE_SIZE,             //Trace Size
        l_rc,                           //userdata1
        l_gsid                          //userdata2
        );

    REQUEST_RESET(l_err);
  }
}

void amec_vectorize_quad_sensor(sensor_t * l_sensor,
                                vectorSensor_t * l_vector,
                                const VECTOR_SENSOR_OP l_op,
                                uint16_t l_sensor_elem_array_gsid)
{
#define VECTOR_CREATE_FAILURE   1
#define VECTOR_ADD_ELEM_FAILURE 2

  int l_idx = 0;    // Used to index the for loops for vector create
  int l_rc  = 0;    // Indicates failure to add a sensor to vector
  uint16_t l_gsid  = l_sensor->gsid; // Grab GSID for errl in case of failure
  errlHndl_t l_err = NULL;

  do
  {
    // Vectorize the sensor
    sensor_vectorize(l_sensor,
        l_vector,
        l_op);

    // If vectorize worked, add elements to the vector sensor
    if(NULL != l_sensor->vector)
    {
      // Loop through quads
      for(l_idx = 0; l_idx < MAXIMUM_QUADS; l_idx++)
      {
        // Add elements to the vector sensor
        sensor_vector_elem_add(l_sensor->vector,
                               l_idx,
                               AMECSENSOR_ARRAY_PTR(l_sensor_elem_array_gsid, l_idx));
      }

      // Sanity check, we should have MAXIMUM_QUADS entries in
      // vector sensor
      if(l_sensor->vector->size != MAXIMUM_QUADS)
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
    TRAC_ERR("amec_vectorize_quad_sensor: Failed to vectorize sensor[0x%x, 0x%x]", l_gsid, l_rc );

    /* @
     * @errortype
     * @moduleid    AMEC_VECTORIZE_QUAD_SENSORS
     * @reasoncode  SSX_GENERIC_FAILURE
     * @userdata1   return code
     * @userdata2   gsid of failed sensor
     * @userdata4   OCC_NO_EXTENDED_RC
     * @devdesc     Firmware failure in call to vectorize sensor
     */
    l_err = createErrl(
        AMEC_VECTORIZE_QUAD_SENSORS,    //modId
        SSX_GENERIC_FAILURE,            //reasoncode
        OCC_NO_EXTENDED_RC,             //Extended reason code
        ERRL_SEV_UNRECOVERABLE,         //Severity
        NULL,                           //Trace Buf
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
  // TEMPPROCAVG Vector Sensor
  //-----------------------------------------------------
  amec_vectorize_core_sensor(AMECSENSOR_PTR(TEMPPROCAVG),
      &g_amec_sys.proc[0].temp4ms_vector,
      VECTOR_OP_AVG,
      TEMPPROCTHRMC0);

  //-----------------------------------------------------
  // TEMPRTAVG Vector Sensor
  //-----------------------------------------------------
  amec_vectorize_quad_sensor(AMECSENSOR_PTR(TEMPRTAVG),
      &g_amec_sys.proc[0].temprt_vector,
      VECTOR_OP_AVG,
      TEMPQ0);

  //-----------------------------------------------------
  // FREQA Vector Sensor
  //-----------------------------------------------------
  amec_vectorize_core_sensor(AMECSENSOR_PTR(FREQA),
      &g_amec_sys.proc[0].freqa_vector,
      VECTOR_OP_AVG,
      FREQAC0);

  //-----------------------------------------------------
  // IPS Vector Sensor
  //-----------------------------------------------------
  amec_vectorize_core_sensor(AMECSENSOR_PTR(IPS),
      &g_amec_sys.proc[0].ips4ms_vector,
      VECTOR_OP_AVG,
      IPSC0);

  //-----------------------------------------------------
  // TEMPPROCTHRM Vector Sensor
  //-----------------------------------------------------
  amec_vectorize_core_sensor(AMECSENSOR_PTR(TEMPPROCTHRM),
      &g_amec_sys.proc[0].tempprocthermal_vector,
      VECTOR_OP_MAX,
      TEMPPROCTHRMC0);

  //-----------------------------------------------------
  // UTIL Vector Sensor
  //-----------------------------------------------------
  amec_vectorize_core_sensor(AMECSENSOR_PTR(UTIL),
      &g_amec_sys.proc[0].util_vector,
      VECTOR_OP_AVG,
      UTILC0);

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
  uint16_t l_idx = 0;

  // Default the frequency range to something safe
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

  // Initialize thermal controller for VRM Vdd
  g_amec->thermalvdd.setpoint = 850; // 850 = 85.0 C
  g_amec->thermalvdd.Pgain = 1000;
  g_amec->thermalvdd.speed_request = 1000;
  g_amec->thermalvdd.freq_request = -1; //unconstrained frequency vote
  g_amec->thermalvdd.total_res = 0;

  // Initialize thermal controller based on DIMM temperatures
  g_amec->thermaldimm.setpoint = 850; //In 0.1 degrees C -> 850 = 85.0 C
  g_amec->thermaldimm.Pgain = 30000;
  g_amec->thermaldimm.speed_request = AMEC_MEMORY_MAX_STEP;

  // Initialize thermal controller based on membuf temperatures
  g_amec->thermalmembuf.setpoint = 850; //In 0.1 degrees C -> 850 = 85.0 C
  g_amec->thermalmembuf.Pgain = 30000;
  g_amec->thermalmembuf.speed_request = AMEC_MEMORY_MAX_STEP;

  // Initialize thermal controller based on temperature sensor covering both mem ctrl and DIMM
  g_amec->thermalmcdimm.setpoint = 850; //In 0.1 degrees C -> 850 = 85.0 C
  g_amec->thermalmcdimm.Pgain = 30000;
  g_amec->thermalmcdimm.speed_request = AMEC_MEMORY_MAX_STEP;

  // Initialize thermal controller based on PMIC temperatures
  g_amec->thermalpmic.setpoint = 850; //In 0.1 degrees C -> 850 = 85.0 C
  g_amec->thermalpmic.Pgain = 30000;
  g_amec->thermalpmic.speed_request = AMEC_MEMORY_MAX_STEP;

  // Initialize thermal controller based on external mem controller temperatures
  g_amec->thermalmcext.setpoint = 850; //In 0.1 degrees C -> 850 = 85.0 C
  g_amec->thermalmcext.Pgain = 30000;
  g_amec->thermalmcext.speed_request = AMEC_MEMORY_MAX_STEP;

  // Initialize component power caps
  g_amec->pcap.active_proc_pcap = 0;
  g_amec->pcap.active_mem_level = 0;

  // Initialize partition information
  amec_part_init();

  // Initialize performace counter
  for (l_idx=0; l_idx<MAX_NUM_CORES; l_idx++)
  {
    amec_core_perf_counter_ctor(&g_amec->proc[0].core[l_idx].core_perf, 0, l_idx);
  }

  //Initialize processor fields
  g_amec->proc[0].core_max_freq = 0xFFFF;

  //Initialize processor power votes
  g_amec->proc[0].pwr_votes.pmax_clip_freq = 0xFFFF;
  g_amec->proc[0].pwr_votes.apss_pmax_clip_freq = 0xFFFF;
  g_amec->proc[0].pwr_votes.proc_pcap_nom_vote = 0xFFFF;
  g_amec->proc[0].pwr_votes.proc_pcap_vote = 0xFFFF;
  g_amec->proc[0].pwr_votes.ppb_fmax = 0xFFFF;
  g_amec->proc[0].pwr_votes.nom_pcap_fmin = 0;

  //Initialize stream buffer recording parameters
  g_amec->recordflag=0;      // Never enable recording until requested via Amester API call
  g_amec->r_cnt=0;           // Reset counter of 250us ticks
  g_amec->ptr_stream_buffer = &g_amec->stream_buffer[0];
  g_amec->stream_vector_mode=0;      // No recording yet
  g_amec->stream_vector_delay=0;     // Delay in msec before recording can begin
  g_amec->stream_vector_rate=0xff;   // Invalid setting: requires IPMI command to select initial rate

  // Initialize the current_mem_pwr_ctl to indicate that memory power control is not supported
  // update memory control registers only if new ips/default memory power control is different
  g_amec->sys.current_mem_pwr_ctl = MEM_PWR_CTL_NO_SUPPORT;

  // Initialize wof_disabled
  g_amec->wof.wof_disabled = WOF_RC_OCC_WOF_DISABLED;

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
  rc  = gpe_request_create( &G_gpe_nop_request[0],         //gpe_req for the task
                            &G_async_gpe_queue0,           //GPE0 queue
                            IPC_ST_GPE0_NOP,               //Function ID
                            NULL,                          //parm for the task
                            SSX_WAIT_FOREVER,              //no timeout
                            (AsyncRequestCallback)
                             amec_slv_update_gpe_sensors,  //callback
                            (void *) GPE_ENGINE_0,         //callback argument
                            ASYNC_CALLBACK_IMMEDIATE );    //options

  // Initializes the GPE routine that will be used to measure the worst case
  // timings for GPE1
  rc2 = gpe_request_create( &G_gpe_nop_request[1],         //gpe_req for the task
                            &G_async_gpe_queue1,           //GPE1 queue
                            IPC_ST_GPE1_NOP,               //Function ID
                            NULL,                          //parm for the task
                            SSX_WAIT_FOREVER,              //no timeout
                            (AsyncRequestCallback)
                            amec_slv_update_gpe_sensors,   //callback
                            (void *) GPE_ENGINE_1,         //callback argument
                            ASYNC_CALLBACK_IMMEDIATE );    //options

  // If we couldn't create the GpeRequest objects, there must be a major problem
  // so we will log an error and halt OCC.
  if( rc || rc2 )
  {
    //If fail to create GpeRequest object then there is a problem.
    TRAC_ERR("Failed to create GPE duration GpeRequest object[0x%x, 0x%x]", rc, rc2);

    /* @
     * @errortype
     * @moduleid    AMEC_INITIALIZE_FW_SENSORS
     * @reasoncode  SSX_GENERIC_FAILURE
     * @userdata1   return code - gpe0
     * @userdata2   return code - gpe1
     * @userdata4   OCC_NO_EXTENDED_RC
     * @devdesc     Failure to create GpeRequest object for FW timing
     *              analysis.
     *
     */
    l_err = createErrl(
        AMEC_INITIALIZE_FW_SENSORS,         //modId
        SSX_GENERIC_FAILURE,                //reasoncode
        OCC_NO_EXTENDED_RC,                 //Extended reason code
        ERRL_SEV_PREDICTIVE,                //Severity
        NULL,                               //Trace Buf
        DEFAULT_TRACE_SIZE,                 //Trace Size
        rc,                                 //userdata1
        rc2                                 //userdata2
    );

    CHECKPOINT_FAIL_AND_HALT(l_err);
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
