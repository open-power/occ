/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/proc/proc_data.c $                                */
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

#include "proc_data.h"
#include "occhw_async.h"
#include "threadSch.h"
#include "proc_data_service_codes.h"
#include "occ_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "rtls.h"
#include "apss.h"
#include "state.h"
#include "proc_data_control.h"
#include "pgpe_interface.h"
#include "cmdh_fsp.h"
#include "sensor.h"
#include "gpe_24x7_structs.h"

//Global array of core data buffers
GPE_BUFFER(CoreData G_core_data[MAX_NUM_FW_CORES+NUM_CORE_DATA_DOUBLE_BUF+NUM_CORE_DATA_EMPTY_BUF]) = {{{0}}};

// Pointers to the actual core data buffers
CoreData * G_core_data_ptrs[MAX_NUM_FW_CORES] = {0};

//Global structures for gpe get core data parms
GPE_BUFFER(ipc_core_data_parms_t G_low_cores_data_parms);
GPE_BUFFER(ipc_core_data_parms_t G_high_cores_data_parms);

//Globals for 24x7 collection
GPE_BUFFER(gpe_24x7_args_t G_24x7_parms);
GpeRequest G_24x7_request;
bool G_24x7_disabled = FALSE;

// IPC Gpe request structure for gathering nest dts temps
GpeRequest G_nest_dts_gpe_req;

// Global structure for nest dts temp collection
GPE_BUFFER(ipc_nest_dts_parms_t G_nest_dts_parms);

//We will have separate bulk core data structure for low and high cores.
//Global low and high cores structures used for task data pointers.
bulk_core_data_task_t G_low_cores = {
            0,                                  // Low core to start with (min)
            0,                                  // Low core to start with (current)
            CORE_MID_POINT - 1,                 // Low core to end with
            &G_core_data[MAX_NUM_FW_CORES]      // Pointer to holding area for low coredata
    };
bulk_core_data_task_t G_high_cores = {
            CORE_MID_POINT,                     // High core to start with (min)
            CORE_MID_POINT,                     // High core to start with (current)
            MAX_NUM_FW_CORES - 1,               // High core to end with
            &G_core_data[MAX_NUM_FW_CORES+1] }; // Pointer to holding area for high coredata

//Keeps track of if Nest DTS data has been collected in last 4ms
bool G_nest_dts_data_valid = FALSE;

//AMEC needs to know when data for a core has been collected.
uint32_t G_updated_core_mask = 0;

// Mask to indicate when an empath error has been detected and empath data
// should be ignored.  Core bits are cleared when empath data is collected
// without error.
uint32_t G_empath_error_core_mask = 0;

//AMEC needs to know cores that are offline
uint32_t G_core_offline_mask = 0;

//Global G_present_cores is bitmask of all cores
//(1 = present, 0 = not present. Core 0 has the most significant bit)
uint32_t G_present_cores = 0;

//Global G_present_hw_cores is bitmask of all hardware cores
//(1 = present, 0 = not present. Core 0 has the most significant bit)
uint32_t G_present_hw_cores = 0;

//Flag to keep track of one time trace for GPE running case
//for task core data.
bool    G_queue_not_idle_traced = FALSE;

// Global to track the maximum time elapsed between pore flex schedules of
// per core get_per_core_data tasks.  The array is indexed by core number.
uint32_t G_get_per_core_data_max_schedule_intervals[MAX_NUM_HW_CORES] = {0,};

// Declaration of debug functions
#ifdef PROC_DEBUG
void print_core_data_sensors(uint8_t core);
void print_core_status(uint8_t core);
#endif

extern bool G_smf_mode;

// Function Specification
//
// Name: task_core_data
//
// Description: Collect bulk core data for all cores in specified range.
//              The task used for core data collection will be split into two
//              individual task instances. The task function is the same but it needs
//              to gather data for different sets of cores.
//
//
// End Function Specification

void task_core_data( task_t * i_task )
{

    errlHndl_t  l_err = NULL;  // Error handler
    int         l_rc = 0;      // Return code
    CoreData  * l_temp = NULL; // Used for pointer swapping
    bulk_core_data_task_t * l_bulk_core_data_ptr = (bulk_core_data_task_t *)i_task->data_ptr;
    ipc_core_data_parms_t * l_parms = (ipc_core_data_parms_t*)(l_bulk_core_data_ptr->gpe_req.cmd_data);
    static uint32_t L_trace_core_failure = 0;

    do
    {
        //First, check to see if the previous GPE request still running
        //A request is considered idle if it is not attached to any of the
        //asynchronous request queues
        if( !(async_request_is_idle(&l_bulk_core_data_ptr->gpe_req.request)) )
        {
            //This should not happen unless there's a timing problem
            //Trace 1 time
            if( !G_queue_not_idle_traced )
            {
                INTR_TRAC_ERR("Previous core data task has not yet completed");
                G_queue_not_idle_traced = TRUE;
            }
            break;
        }

        //Need to complete collecting data for all assigned cores from previous interval
        //and tick 0 is the current tick before collect data again.
        if( (l_bulk_core_data_ptr->current_core == l_bulk_core_data_ptr->end_core)
            &&
            ((CURRENT_TICK & (MAX_NUM_TICKS - 1)) != 0) )
        {
            PROC_DBG("Data will not be collected, waiting for tick cycle to restart");
            break;
        }

        //Check to see if the previous GPE request has successfully completed
        //A request is not considered complete until both the engine job
        //has finished without error and any callback has run to completion.

        if( async_request_completed(&l_bulk_core_data_ptr->gpe_req.request) &&
            (l_parms->error.rc == 0) &&
            CORE_PRESENT(l_bulk_core_data_ptr->current_core) )
        {
            //If the previous GPE request succeeded then swap core_data_ptr
            //with the global one. The gpe routine will write new data into
            //a buffer that is not being accessed by the RTLoop code.

            PROC_DBG( "Swap core_data_ptr [%x] with the global one",
                     l_bulk_core_data_ptr->current_core );

            //debug only
#ifdef PROC_DEBUG
//            print_core_status(l_bulk_core_data_ptr->current_core);
            print_core_data_sensors(l_bulk_core_data_ptr->current_core);
#endif

            l_temp = l_bulk_core_data_ptr->core_data_ptr;
            l_bulk_core_data_ptr->core_data_ptr = G_core_data_ptrs[l_bulk_core_data_ptr->current_core];
            G_core_data_ptrs[l_bulk_core_data_ptr->current_core] = l_temp;

            //Core data has been collected so set the bit in global mask.
            //AMEC code will know which cores to update sensors for. AMEC is
            //responsible for clearing the bit later on.
            G_updated_core_mask |= (CORE0_PRESENT_MASK >> (l_bulk_core_data_ptr->current_core));

            // set  or clear the empath error mask
            if(l_temp->empathValid)
            {
                G_empath_error_core_mask &=
                    ~(CORE0_PRESENT_MASK >> (l_bulk_core_data_ptr->current_core));
            }
            else
            {
                G_empath_error_core_mask |=
                    (CORE0_PRESENT_MASK >> (l_bulk_core_data_ptr->current_core));
            }
        }

        // If the core is not present, then we need to point to the empty G_core_data
        // so that we don't use old/stale data from a leftover G_core_data
        if( !CORE_PRESENT(l_bulk_core_data_ptr->current_core))
        {
            G_core_data_ptrs[l_bulk_core_data_ptr->current_core] = &G_core_data[MAX_NUM_FW_CORES+NUM_CORE_DATA_DOUBLE_BUF+NUM_CORE_DATA_EMPTY_BUF-1];
        }
        else if(l_parms->error.rc != 0)
        {
            // Check if failure is due to being offline (in stop 2 or greater)
            if(l_parms->error.ffdc == PCB_ERROR_CHIPLET_OFFLINE)
            {
                // Mark core offline so it is ignored in control loops and to avoid health monitor logging error
                G_core_offline_mask |= (CORE0_PRESENT_MASK >> (l_bulk_core_data_ptr->current_core));
            }
            else if( !(L_trace_core_failure & (1 << l_bulk_core_data_ptr->current_core)) )
            {
               // trace error, if it continues health monitor will see and log error
               INTR_TRAC_ERR("task_core_data: core %d data collection failed RC[0x%08X] FFDC[0x%08X%08X]",
                              l_bulk_core_data_ptr->current_core, l_parms->error.rc,
                              (uint32_t)(l_parms->error.ffdc >> 32),
                              (uint32_t)l_parms->error.ffdc);
               L_trace_core_failure |= (1 << l_bulk_core_data_ptr->current_core);
            }
        }

        //Update current core
        if ( l_bulk_core_data_ptr->current_core >= l_bulk_core_data_ptr->end_core )
        {
            l_bulk_core_data_ptr->current_core = l_bulk_core_data_ptr->start_core;
        }
        else
        {
            l_bulk_core_data_ptr->current_core++;
        }

        //If core is not present then skip it. This task assigned to this core will
        //be idle during this time it would have collected the data.
        if( CORE_PRESENT(l_bulk_core_data_ptr->current_core) )
        {
            PROC_DBG("Schedule GpeRequest for core %d", l_bulk_core_data_ptr->current_core);

            //1. Setup the get core data parms
            l_parms->core_num = l_bulk_core_data_ptr->current_core;
            l_parms->data = (CoreData*) l_bulk_core_data_ptr->core_data_ptr;
            l_parms->error.error = 0;  // default no error
            l_parms->error.ffdc = 0;

            // Static array to record the last timestamp a get_per_core_data task was
            // scheduled for a core.
            static SsxTimebase L_last_get_per_core_data_scheduled_time[MAX_NUM_HW_CORES] = {0,};
            uint8_t l_current_core = l_bulk_core_data_ptr->current_core;
            SsxTimebase l_now = ssx_timebase_get();
            // If the last scheduled timestamp is 0, record time and continue to schedule
            if (L_last_get_per_core_data_scheduled_time[l_current_core] == 0)
            {
                L_last_get_per_core_data_scheduled_time[l_current_core] = l_now;
            }
            else
            {
                // Calculate elapsed time in usecs since last get_per_core_data
                // task for the current core was scheduled.
                uint32_t l_elapsed_us =
                        (uint32_t)((l_now - L_last_get_per_core_data_scheduled_time[l_current_core])/
                           (SSX_TIMEBASE_FREQUENCY_HZ/1000000));
                // Save the last scheduled timestamp
                L_last_get_per_core_data_scheduled_time[l_current_core] = l_now;
                // If the new elapsed time is greater than what is currently saved,
                // save the larger time.
                if (G_get_per_core_data_max_schedule_intervals[l_current_core] < l_elapsed_us)
                {
                    G_get_per_core_data_max_schedule_intervals[l_current_core] = l_elapsed_us;
                }
            }

            //2. Schedule the GPE Request to get the core data
            //   Check gpe_request_schedule return code for an error
            //   and if there is an error, request an OCC reset.

            l_rc = gpe_request_schedule( &l_bulk_core_data_ptr->gpe_req );
            if( l_rc != 0 )
            {
                // Error in schedule gpe get core data
                INTR_TRAC_ERR("Failed GpeRequest schedule core [RC:0x%08X]", l_rc);

                /*
                * @errortype
                * @moduleid    PROC_TASK_CORE_DATA_MOD
                * @reasoncode  SSX_GENERIC_FAILURE
                * @userdata1   gpe_request_schedule return code
                * @userdata4   OCC_NO_EXTENDED_RC
                * @devdesc     SSX IPC related failure
                */
                l_err = createErrl(
                        PROC_TASK_CORE_DATA_MOD,            //modId
                        SSX_GENERIC_FAILURE,                //reasoncode
                        OCC_NO_EXTENDED_RC,                 //Extended reason code
                        ERRL_SEV_PREDICTIVE,                //Severity
                        NULL,                               //Trace Buf
                        DEFAULT_TRACE_SIZE,                 //Trace Size
                        l_rc,                               //userdata1
                        0                                   //userdata2
                );

                // commit error log
                REQUEST_RESET(l_err);
                break;
            }
        }
    }
    while(0);

    return;
}

// Function Specification
//
// Name: proc_core_init
//
// Description: Initialize structure for collecting core data. It
//              needs to be run in occ main and before RTLoop started.
//
// End Function Specification
void proc_core_init( void )
{
    errlHndl_t  l_err = NULL;    // Error handler
    int         l_rc = 0;        // Return code
    uint8_t     i = 0;

    // Setup the array of CoreData pointers
    for( i = 0; i < MAX_NUM_FW_CORES; i++ )
    {
        G_core_data_ptrs[i] = &G_core_data[i];
    }

    do
    {
        // Before the real time loop begins collecting data, we need to determine which
        // cores are present and configured. The Core Configuration Status Register
        // has this information, so we will need to read it over OCI.
        uint64_t l_ccsr_val = in64(OCB_CCSR);
        MAIN_TRAC_INFO("proc_core_init: CCSR read 0x%08X%08X",
                       (uint32_t) (l_ccsr_val>>32), (uint32_t) l_ccsr_val);

        G_present_hw_cores = ((uint32_t) (l_ccsr_val >> 32)) & HW_CORES_MASK;
        G_present_cores = G_present_hw_cores;

        PROC_DBG("G_present_hw_cores =[%x] and G_present_cores =[%x]",
                G_present_hw_cores, G_present_cores);

        //Initializes the GpeRequest object for low core data collection
        l_rc = gpe_request_create(&G_low_cores.gpe_req,            // GpeRequest for the task
                                  &G_async_gpe_queue0,             // Queue
                                  IPC_ST_GET_CORE_DATA_FUNCID,     // Function ID
                                  &G_low_cores_data_parms,         // Task parameters
                                  SSX_WAIT_FOREVER,                // Timeout (none)
                                  NULL,                            // Callback
                                  NULL,                            // Callback arguments
                                  0 );                             // Options

        if( l_rc )
        {
            // If we failed to create the GpeRequest then there is a serious problem.
            MAIN_TRAC_ERR("proc_core_init: Failure creating the low core data GpeRequest. [RC=0x%08x]", l_rc );

            /*
             * @errortype
             * @moduleid    PROC_CORE_INIT_MOD
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   gpe_request_create return code
             * @userdata4   ERC_LOW_CORE_GPE_REQUEST_CREATE_FAILURE
             * @devdesc     Failure to create low cores GpeRequest object
             */
            l_err = createErrl(
                    PROC_CORE_INIT_MOD,                      //ModId
                    SSX_GENERIC_FAILURE,                     //Reasoncode
                    ERC_LOW_CORE_GPE_REQUEST_CREATE_FAILURE, //Extended reason code
                    ERRL_SEV_PREDICTIVE,                     //Severity
                    NULL,                                    //Trace Buf
                    DEFAULT_TRACE_SIZE,                      //Trace Size
                    l_rc,                                    //Userdata1
                    0                                        //Userdata2
            );

            CHECKPOINT_FAIL_AND_HALT(l_err);
            break;
        }

        //Initializes high cores data GpeRequest object
        l_rc = gpe_request_create(&G_high_cores.gpe_req,           // GpeRequest for the task
                                  &G_async_gpe_queue0,             // Queue
                                  IPC_ST_GET_CORE_DATA_FUNCID,     // Function ID
                                  &G_high_cores_data_parms,        // Task parameters
                                  SSX_WAIT_FOREVER,                // Timeout (none)
                                  NULL,                            // Callback
                                  NULL,                            // Callback arguments
                                  0 );                             // Options

        if( l_rc )
        {
            // If we failed to create the GpeRequest then there is a serious problem.
            MAIN_TRAC_ERR("proc_core_init: Failure creating the high core data GpeRequest. [RC=0x%08x]", l_rc );

            /*
             * @errortype
             * @moduleid    PROC_CORE_INIT_MOD
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   gpe_request_create return code
             * @userdata4   ERC_HIGH_CORE_GPE_REQUEST_CREATE_FAILURE
             * @devdesc     Failure to create high core GpeRequest object
             */
            l_err = createErrl(
                                PROC_CORE_INIT_MOD,                       //ModId
                                SSX_GENERIC_FAILURE,                      //Reasoncode
                                ERC_HIGH_CORE_GPE_REQUEST_CREATE_FAILURE, //Extended reason code
                                ERRL_SEV_PREDICTIVE,                      //Severity
                                NULL,                                     //Trace Buf
                                DEFAULT_TRACE_SIZE,                       //Trace Size
                                l_rc,                                     //Userdata1
                                0                                         //Userdata2
            );

            CHECKPOINT_FAIL_AND_HALT(l_err);
            break;
        }

        //Initialize 24x7 data collection GpeRequest object
        l_rc = gpe_request_create(&G_24x7_request,           // GpeRequest for the task
                                  &G_async_gpe_queue1,       // Queue for GPE1
                                  IPC_ST_24_X_7_FUNCID,      // Function ID
                                  &G_24x7_parms,             // Task parameters
                                  SSX_WAIT_FOREVER,          // Timeout (none)
                                  NULL,                      // Callback
                                  NULL,                      // Callback arguments
                                  ASYNC_CALLBACK_IMMEDIATE); // Options

        if( l_rc )
        {
            // If we failed to create the GpeRequest then there is a serious problem.
            MAIN_TRAC_ERR("proc_core_init: Failure creating 24x7 GpeRequest. [RC=0x%08x]", l_rc );

            /*
             * @errortype
             * @moduleid    PROC_CORE_INIT_MOD
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   gpe_request_create return code
             * @userdata4   ERC_24X7_GPE_CREATE_FAILURE
             * @devdesc     Failure to create 24x7 GpeRequest object
             */
            l_err = createErrl(
                                PROC_CORE_INIT_MOD,                       //ModId
                                SSX_GENERIC_FAILURE,                      //Reasoncode
                                ERC_24X7_GPE_CREATE_FAILURE,              //Extended reason code
                                ERRL_SEV_PREDICTIVE,                      //Severity
                                NULL,                                     //Trace Buf
                                DEFAULT_TRACE_SIZE,                       //Trace Size
                                l_rc,                                     //Userdata1
                                0                                         //Userdata2
            );

            CHECKPOINT_FAIL_AND_HALT(l_err);
            break;
        }


    } while(0);

    // Initialize the core data control at the same time
    init_pgpe_ipcs();

    return;
}

// Function Specification
//
// Name: proc_get_bulk_core_data_ptr
//
// Description: Returns a pointer to the most up-to-date bulk core data for
//              the core associated with the specified OCC core id. Returns
//              NULL for core ID outside the range of 0 to 11.
//
// End Function Specification
CoreData * proc_get_bulk_core_data_ptr( const uint8_t i_occ_core_id )
{
    //The caller needs to send in a valid OCC core id. Since type is uchar
    //so there is no need to check for case less than 0.
    //If core id is invalid then returns NULL.

    if( i_occ_core_id < MAX_NUM_FW_CORES  )
    {
        //Returns a pointer to the most up-to-date bulk core data.
        return G_core_data_ptrs[i_occ_core_id];
    }
    else
    {
        //Core id outside the range
        TRAC_ERR("proc_get_bulk_core_data_ptr: Invalid OCC core id [0x%x]", i_occ_core_id);
        return( NULL );
    }
}

// Function Specification
//
// Name: task_nest_dts
//
// Description: Collect nest DTS readings and save off the average.
//
// End Function Specification
void task_nest_dts( task_t * i_task )
{
    errlHndl_t  l_err = NULL;  // Error handler
    int         l_rc = 0;      // Return code
    ipc_nest_dts_parms_t * l_parms = (ipc_nest_dts_parms_t*)(G_nest_dts_gpe_req.cmd_data);
    uint16_t    l_avg = 0;
    uint16_t    l_nestDtsTemp = 0;
    uint8_t     l_nestDtsCnt = 0;  // Number of valid Nest DTSs
    uint8_t     k = 0;
    bool        l_nestDtsValid = FALSE;
    static bool L_scheduled = FALSE;
    static bool L_idle_trace = FALSE;
    static bool L_incomplete_trace = FALSE;

    do
    {
        //First, check to see if the previous GPE request still running
        //A request is considered idle if it is not attached to any of the
        //asynchronous request queues
        if( !(async_request_is_idle(&G_nest_dts_gpe_req.request)) )
        {
            //This should not happen unless there's a timing problem, trace once
            if( !L_idle_trace )
            {
                INTR_TRAC_ERR("task_nest_dts: request is not idle");
                L_idle_trace = TRUE;
            }
            break;
        }

        // Don't update the dts temp sensor until we've run at least once
        if (L_scheduled)
        {
            // Check that the previous request has finished and an error did not occur
            if ((ASYNC_REQUEST_STATE_COMPLETE == G_nest_dts_gpe_req.request.completion_state) &&
                (0 == G_nest_dts_parms.error.error))
            {
                for (k = 0; k < NEST_DTS_COUNT; k++)
                {
                    // check valid and temperature for current nest DTS being processed
                    l_nestDtsValid = l_parms->data.sensor[k].fields.valid;
                    // temperature is only 8 bits of reading field
                    l_nestDtsTemp = (l_parms->data.sensor[k].fields.reading & 0xFF);

                    //Hardware bug workaround:  Module test will detect bad DTS and write coefficients
                    //to force a reading of 0 or negative to indicate the DTS is bad.
                    //Ignore any DTS that is not valid or marked bad
                    if( (l_nestDtsValid) && ( (l_nestDtsTemp & DTS_INVALID_MASK) != DTS_INVALID_MASK) &&
                        (l_nestDtsTemp != 0) )
                    {
                        l_avg += l_nestDtsTemp;
                        l_nestDtsCnt++;
                    }
                } //for loop

                if(l_nestDtsCnt)
                {
                    // Calculate the average of the valid Nest DTS temps
                    l_avg = l_avg / l_nestDtsCnt;

                    // Mark the data as valid and update sensor
                    G_nest_dts_data_valid = TRUE;
                    sensor_update(AMECSENSOR_PTR(TEMPNEST), l_avg);
                }
                else
                {
                    // No valid nest DTS, Mark the data as invalid
                    G_nest_dts_data_valid = FALSE;
                }
            } // if request completed without error
            else
            {
                // Async request not finished, mark data invalid
                G_nest_dts_data_valid = FALSE;

                // Trace only once
                if (!L_incomplete_trace)
                {
                    INTR_TRAC_ERR("task_nest_dts: task did not complete in time or an error occurred [rc=0x%08X]",
                                  G_nest_dts_parms.error.rc);
                    L_incomplete_trace = TRUE;
                }
            }
        }

        // Schedule the nest DTS task
        l_rc = gpe_request_schedule( &G_nest_dts_gpe_req );
        if( l_rc != 0 )
        {
            // Error scheduling nest DTS task
            INTR_TRAC_ERR("Failed to schedule nest DTS task [RC:0x%08X]", l_rc);

            /*
             * @errortype
             * @moduleid    PROC_TASK_NEST_DTS_MOD
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   gpe_request_schedule return code
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     SSX IPC related failure
             */
            l_err = createErrl(
                        PROC_TASK_NEST_DTS_MOD,             //modId
                        SSX_GENERIC_FAILURE,                //reasoncode
                        OCC_NO_EXTENDED_RC,                 //Extended reason code
                        ERRL_SEV_PREDICTIVE,                //Severity
                        NULL,                               //Trace Buf
                        DEFAULT_TRACE_SIZE,                 //Trace Size
                        l_rc,                               //userdata1
                        0                                   //userdata2
                    );

            // commit error log
            REQUEST_RESET(l_err);
            break;
        }
        else
        {
            L_scheduled = TRUE;
        }

    } while(0);
}

// Function Specification
//
// Name: nest_dts_init
//
// Description: Setup the GpeRequest object for collecting nest DTS readings.
//
// End Function Specification
void nest_dts_init(void)
{
    errlHndl_t l_err = NULL;    // Error handler
    int        l_rc = 0;        // Return code

    //Initializes the GpeRequest object for nest dts temp collection
    l_rc = gpe_request_create(&G_nest_dts_gpe_req,             // GpeRequest for the task
                              &G_async_gpe_queue0,             // Queue
                              IPC_ST_GET_NEST_DTS_FUNCID,      // Function ID
                              &G_nest_dts_parms,               // Task parameters
                              SSX_WAIT_FOREVER,                // Timeout (none)
                              NULL,                            // Callback
                              NULL,                            // Callback arguments
                              0 );                             // Options


    if( l_rc )
    {
        // If we failed to create the GpeRequest then there is a serious problem.
        MAIN_TRAC_ERR("nest_dts_init: Failure creating the nest dts GpeRequest. [RC=0x%08x]", l_rc );

        /*
         * @errortype
         * @moduleid    PROC_NEST_DTS_INIT_MOD
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   gpe_request_create return code
         * @userdata4   ERC_NEST_DTS_GPE_REQUEST_CREATE_FAILURE
         * @devdesc     Failure to create nest dts GpeRequest object
         */
        l_err = createErrl(
                            PROC_NEST_DTS_INIT_MOD,                  //ModId
                            SSX_GENERIC_FAILURE,                     //Reasoncode
                            ERC_NEST_DTS_GPE_REQUEST_CREATE_FAILURE, //Extended reason code
                            ERRL_SEV_PREDICTIVE,                     //Severity
                            NULL,                                    //Trace Buf
                            DEFAULT_TRACE_SIZE,                      //Trace Size
                            l_rc,                                    //Userdata1
                            0                                        //Userdata2
        );

        CHECKPOINT_FAIL_AND_HALT(l_err);
    }
}

// Function Specification
//
// Name:  task_24x7
//
// Description: Called every tick while active/obs state to do 24x7 data collection
//
// End Function Specification
void task_24x7(task_t * i_task)
{
    static uint8_t L_numTicks = 0x00;  // never called since OCC started
    static bool    L_idle_trace = FALSE;
    static bool    L_logged_disable = FALSE;

    // Schedule 24x7 task if it hasn't been disabled
    if( (!G_24x7_disabled) && !(G_internal_flags & INT_FLAG_DISABLE_24X7))
    {
        // Schedule 24x7 task if idle
        if (!async_request_is_idle(&G_24x7_request.request))
        {
            if(!L_idle_trace)
            {
                INTR_TRAC_ERR("task_24x7: request not idle");
                L_idle_trace = TRUE;
            }
            L_numTicks++;
        }
        else
        {
            if(L_idle_trace)
            {
                INTR_TRAC_INFO("task_24x7: previously was not idle and is now idle after %d ticks", L_numTicks);
                L_idle_trace = FALSE;
            }
            // Clear errors and init parameters for GPE task
            G_24x7_parms.error.error = 0;
            G_24x7_parms.numTicksPassed = L_numTicks;
            if (L_logged_disable)
            {
                INTR_TRAC_INFO("task_24x7: schedule re-enabled");
                L_logged_disable = FALSE;
            }

            int l_rc = gpe_request_schedule(&G_24x7_request);
            if (0 == l_rc)
            {
                L_numTicks = 1;  // next time called will be 1 tick later
            }
            else
            {
                errlHndl_t l_err = NULL;
                INTR_TRAC_ERR("task_24x7: schedule failed w/rc=0x%08X (%d us)",
                              l_rc, (int) ((ssx_timebase_get())/(SSX_TIMEBASE_FREQUENCY_HZ/1000000)));
                /*
                 * @errortype
                 * @moduleid    PROC_24X7_MOD
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   gpe_request_schedule return code
                 * @userdata4   ERC_24X7_GPE_SCHEDULE_FAILURE
                 * @devdesc     Failure to schedule 24x7 GpeRequest
                 */
                l_err = createErrl(
                                   PROC_24X7_MOD,                            //ModId
                                   SSX_GENERIC_FAILURE,                      //Reasoncode
                                   ERC_24X7_GPE_SCHEDULE_FAILURE,            //Extended reason code
                                   ERRL_SEV_PREDICTIVE,                      //Severity
                                   NULL,                                     //Trace Buf
                                   DEFAULT_TRACE_SIZE,                       //Trace Size
                                   l_rc,                                     //Userdata1
                                   0                                         //Userdata2
                                  );

                // Request reset since this should never happen.
                REQUEST_RESET(l_err);
            }
        }
    }  // !G_24x7_disabled
    else
    {
        if (! L_logged_disable)
        {
            INTR_TRAC_INFO("task_24x7: not scheduled due to disable");
            L_logged_disable = TRUE;
        }
        // 24x7 is disabled INC number ticks so 24x7 knows how many ticks it was disabled for
        L_numTicks++;
    }

    return;
} // end task_24x7()


#ifdef PROC_DEBUG
//uncomment to dump hex strings under simics. slows down simulation significantly
//#define PROC_DEBUG_DUMP

// Function Specification
//
// Name: print_core_data_sensors
//
// Description: Print out sensors data of a specified core in the chip
//
// End Function Specification

void print_core_data_sensors(uint8_t core)
{
    CoreData * l_core_data = proc_get_bulk_core_data_ptr(core);

    if( l_core_data != NULL )
    {
        PROC_DBG("-------------------------------");
        PROC_DBG("Core [%d] Sensors Data", core);
        PROC_DBG("Sensor Core[0] reading: 0x%04X [Valid:%d][Spare:%d][Trip:%d]",
                 l_core_data->dts.core[0].fields.reading, l_core_data->dts.core[0].fields.valid,
                 l_core_data->dts.core[0].fields.spare, l_core_data->dts.core[0].fields.thermal_trip);
        PROC_DBG("Sensor Core[1] reading: 0x%04X [Valid:%d][Spare:%d][Trip:%d]",
                 l_core_data->dts.core[1].fields.reading, l_core_data->dts.core[1].fields.valid,
                 l_core_data->dts.core[1].fields.spare, l_core_data->dts.core[1].fields.thermal_trip);
        PROC_DBG("Sensor Cache   reading: 0x%04X [Valid:%d][Spare:%d][Trip:%d]",
                 l_core_data->dts.cache.fields.reading, l_core_data->dts.cache.fields.valid,
                 l_core_data->dts.cache.fields.spare, l_core_data->dts.cache.fields.thermal_trip);
        PROC_DBG("Sensor RaceTrack reading: 0x%04x [Valid:%d][Spare:%d][Trip:%d]",
                 l_core_data->dts.racetrack.fields.reading, l_core_data->dts.racetrack.fields.valid,
                 l_core_data->dts.racetrack.fields.space, l_core_data->dts.racetrack.fields.thermal_trip);
    }
    else
    {
        PROC_DBG("G_core_data_ptrs[%x] is NULL. This should not happen.", core);
    }
    return;
}

// Function Specification
//
// Name: print_core_status
//
// Description: Print out information of a specified core in the chip
//
// End Function Specification

void print_core_status(uint8_t core)
{
    CoreData * l_core_data = proc_get_bulk_core_data_ptr(core);

    if( l_core_data != NULL )
    {
        PROC_DBG("-------------------------");
        PROC_DBG("Core [%x] status", core);

#ifdef PROC_DEBUG_DUMP
        dumpHexString(&l_core_data->core_tod, sizeof(l_core_data->core_tod), "Core TOD");
        dumpHexString(&l_core_data->core_raw_cycles, sizeof(l_core_data->core_raw_cycles), "Core Raw Cycles");
        dumpHexString(&l_core_data->core_run_cycles, sizeof(l_core_data->core_run_cycles), "Run Cycles");
        dumpHexString(&l_core_data->core_dispatch, sizeof(l_core_data->core_dispatch), "Core Dispatch");
        dumpHexString(&l_core_data->core_completion, sizeof(l_core_data->core_completion), "Core Completion");
        dumpHexString(&l_core_data->core_workrate, sizeof(l_core_data->core_workrate), "Core Workrate");
        dumpHexString(&l_core_data->core_mem_hler_a, sizeof(l_core_data->core_mem_hler_a), "Mem A");
        dumpHexString(&l_core_data->core_mem_hler_b, sizeof(l_core_data->core_mem_hler_b), "Mem B");
        dumpHexString(&l_core_data->mem_tod, sizeof(l_core_data->mem_tod), "Mem TOD");
        dumpHexString(&l_core_data->mem_raw_cycles, sizeof(l_core_data->mem_raw_cycles), "Mem Raw Cycles");
#endif
    }
    else
    {
        PROC_DBG("G_core_data_ptrs[%x] is NULL. This should not happen.", core);
    }
    return;
}

#endif

