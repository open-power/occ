/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/proc/proc_data.c $                                */
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

#include "proc_data.h"
#include "occhw_async.h"
#include "threadSch.h"
#include "pmc_register_addresses.h"
#include "proc_data_service_codes.h"
#include "occ_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "rtls.h"
#include "apss.h"
#include "state.h"
#include "proc_data_control.h"
#include "core_data.h"
//Global array of core data buffers
GPE_BUFFER(CoreData G_core_data[MAX_NUM_FW_CORES+NUM_CORE_DATA_DOUBLE_BUF+NUM_CORE_DATA_EMPTY_BUF]) = {{{0}}};

// Pointers to the actual core data buffers
CoreData * G_core_data_ptrs[MAX_NUM_FW_CORES] = {0};

// TEMP / TODO -- Only needed until we get the HWP for getting CORE_STATUS
// Used to get GPE0 to do a getscom of the Core Config Status Reg
GPE_BUFFER(ipc_scom_op_t G_core_stat_scom_op);

//Global structures for gpe get core data parms
GPE_BUFFER(ipc_core_data_parms_t G_low_cores_data_parms);
GPE_BUFFER(ipc_core_data_parms_t G_high_cores_data_parms);

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

//AMEC needs to know when data for a core has been collected.
uint32_t G_updated_core_mask = 0;

// Mask to indicate when an empath error has been detected and empath data
// should be ignored.  Core bits are cleared when empath data is collected
// without error.
uint32_t G_empath_error_core_mask = 0;

//Global G_present_cores is bitmask of all cores
//(1 = present, 0 = not present. Core 0 has the most significant bit)
uint32_t G_present_cores = 0;

//Global G_present_hw_cores is bitmask of all hardware cores
//(1 = present, 0 = not present. Core 0 has the most significant bit)
uint32_t G_present_hw_cores = 0;

//OCC to HW core id mapping array
uint8_t G_occ2hw_core_id[MAX_NUM_HW_CORES] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };

//HW to OCC core id mapping array
uint8_t G_hw2occ_core_id[MAX_NUM_HW_CORES] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };

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
    const trace_descriptor_array_t *l_trace = NULL; // Temporary trace descriptor
    bulk_core_data_task_t * l_bulk_core_data_ptr = (bulk_core_data_task_t *)i_task->data_ptr;
    ipc_core_data_parms_t * l_parms = (ipc_core_data_parms_t*)(l_bulk_core_data_ptr->gpe_req.cmd_data);

    INTR_TRAC_INFO("task_core_data: core %d", l_bulk_core_data_ptr->current_core);

    do
    {
        //First, check to see if the previous GPE request still running
        //A request is considered idle if it is not attached to any of the
        //asynchronous request queues
        if( !(async_request_is_idle(&l_bulk_core_data_ptr->gpe_req.request)) )
        {
            //This should not happen unless there's a problem
            //Trace 1 time
            if( !G_queue_not_idle_traced )
            {
                TRAC_ERR("Core data GPE is still running \n");
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
            PROC_DBG("Not collect data. Need to wait for tick.\n");
            break;
        }

        //Check to see if the previously GPE request has successfully completed
        //A request is not considered complete until both the engine job
        //has finished without error and any callback has run to completion.

        if( async_request_completed(&l_bulk_core_data_ptr->gpe_req.request)
            &&
            CORE_PRESENT(l_bulk_core_data_ptr->current_core) )
        {
            //If the previous GPE request succeeded then swap core_data_ptr
            //with the global one. The gpe routine will write new data into
            //a buffer that is not being accessed by the RTLoop code.

            PROC_DBG( "Swap core_data_ptr [%x] with the global one\n",
                     l_bulk_core_data_ptr->current_core );

            //debug only
#ifdef PROC_DEBUG
            print_core_status(l_bulk_core_data_ptr->current_core);
            print_core_data_sensors(l_bulk_core_data_ptr->current_core);
#endif

            l_temp = l_bulk_core_data_ptr->core_data_ptr;
            l_bulk_core_data_ptr->core_data_ptr =
                    G_core_data_ptrs[l_bulk_core_data_ptr->current_core];
            G_core_data_ptrs[l_bulk_core_data_ptr->current_core] = l_temp;

            //Core data has been collected so set the bit in global mask.
            //AMEC code will know which cores to update sensors for. AMEC is
            //responsible for clearing the bit later on.
            G_updated_core_mask |= CORE0_PRESENT_MASK >> (l_bulk_core_data_ptr->current_core);

            // Presumptively clear the empath error mask
            G_empath_error_core_mask &=
                    ~(CORE0_PRESENT_MASK >> (l_bulk_core_data_ptr->current_core));
        }

        // If the core is not present, then we need to point to the empty G_core_data
        // so that we don't use old/stale data from a leftover G_core_data
        if( !CORE_PRESENT(l_bulk_core_data_ptr->current_core))
        {
            G_core_data_ptrs[l_bulk_core_data_ptr->current_core] = &G_core_data[MAX_NUM_FW_CORES+NUM_CORE_DATA_DOUBLE_BUF+NUM_CORE_DATA_EMPTY_BUF-1];
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
            PROC_DBG("Schedule GpeRequest for core %d\n", l_bulk_core_data_ptr->current_core);

            //1. Setup the get core data parms
            l_parms->core_num = l_bulk_core_data_ptr->current_core;
            l_parms->data = (CoreData*) l_bulk_core_data_ptr->core_data_ptr;

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
                    TRAC_INFO("New max get_per_core_data interval: core=%d, interval(us)=%d",
                              l_current_core, l_elapsed_us);
                }
                // Also sniff if the request has actually completed, it is checked above but
                // the schedule proceeds regardless which could be dangerous...
                if (!async_request_completed(&l_bulk_core_data_ptr->gpe_req.request))
                {
                    TRAC_ERR("Async get_per_core_data task for core=%d not complete!",
                             l_current_core);
                }
            }

            //2. Schedule the GPE Request to get the core data
            //   Check gpe_request_schedule return code for an error
            //   and if there is an error, request an OCC reset.

            l_rc = gpe_request_schedule( &l_bulk_core_data_ptr->gpe_req );
            if( l_rc != 0 )
            {
                // Error in schedule gpe get core data
                TRAC_ERR("Failed GpeRequest schedule core [RC:0x%08X] \n", l_rc);

                /*
                * @errortype
                * @moduleid    PROC_TASK_CORE_DATA_MOD
                * @reasoncode  SSX_GENERIC_FAILURE
                * @userdata1   pore_flex_schedule return code
                * @userdata4   OCC_NO_EXTENDED_RC
                * @devdesc     SSX PORE related failure
                */
                l_err = createErrl(
                        PROC_TASK_CORE_DATA_MOD,            //modId
                        SSX_GENERIC_FAILURE,                //reasoncode
                        OCC_NO_EXTENDED_RC,                 //Extended reason code
                        ERRL_SEV_PREDICTIVE,                //Severity
                        l_trace, //TODO: create l_trace     //Trace Buf
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
//                needs to be run in occ main and before RTLoop started.
//
// End Function Specification

void proc_core_init( void )
{
    errlHndl_t l_err = NULL;    // Error handler
    int        l_rc = 0;        // Return code
    const trace_descriptor_array_t *l_trace = NULL;  // Temporary trace descriptor
    uint8_t i = 0;
    GpeRequest l_req;

    // Setup the array of CoreData pointers
    for( i = 0; i < MAX_NUM_FW_CORES; i++ )
    {
        G_core_data_ptrs[i] = &G_core_data[i];
    }

    do
    {
        // Before the real time loop begins collecting data, we need to determine which
        // cores are present and configured. The Core Configuration Status Register
        // has this information, but only the GPEs can read this via a scom.

        // TEMP/TODO: For now, we will use a generic IPC command to do this and other
        //            scoms until the HW team can give us the hardware procedures.
        G_core_stat_scom_op.addr = 0x6c090;
        G_core_stat_scom_op.size = 8;
        G_core_stat_scom_op.data = 0xFFFFFF00;
        G_core_stat_scom_op.read = TRUE;

        //Initializes the GpeRequest object for reading the configuration status register
        l_rc = gpe_request_create(&l_req,                          // GpeRequest for the task
                                  &G_async_gpe_queue0,             // Queue
                                  IPC_ST_SCOM_OPERATION,           // Function ID
                                  &G_core_stat_scom_op,            // Task parameters
                                  SSX_WAIT_FOREVER,                // Timeout (none)
                                  NULL,                            // Callback
                                  NULL,                            // Callback arguments
                                  0 );                             // Options
        // Schedule the request
        //l_rc = gpe_request_schedule(&l_req);

        MAIN_TRAC_INFO("proc_core_init: generic scom read back 0x%08X%08X", G_core_stat_scom_op.data>>32, G_core_stat_scom_op.data);

        // TODO: Store the present cores here
        G_present_hw_cores = G_core_stat_scom_op.data & HW_CORES_MASK;

        G_present_cores = G_present_hw_cores;

        PROC_DBG("G_present_hw_cores =[%x] and G_present_cores =[%x] \n",
                G_present_hw_cores, G_present_cores);

        //Initializes the GpeRequest object for low core data collection
        l_rc = gpe_request_create(&G_low_cores.gpe_req,            // GpeRequest for the task
                                  &G_async_gpe_queue0,             // Queue
                                  IPC_ST_CORE_DATA_LOW_FUNCID,     // Function ID
                                  &G_low_cores_data_parms,         // Task parameters
                                  SSX_WAIT_FOREVER,                // Timeout (none)
                                  NULL,                            // Callback
                                  NULL,                            // Callback arguments
                                  0 );                             // Options

        if( l_rc )
        {
            // If we failed to create the GpeRequest then there is a serious problem.
            MAIN_TRAC_ERR("Failure creating the low core data GpeRequest. [RC=0x%08x]", l_rc );

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
                    l_trace,                                 //Trace Buf
                    DEFAULT_TRACE_SIZE,                      //Trace Size
                    l_rc,                                    //Userdata1
                    0                                        //Userdata2
            );

            // commit error log
            REQUEST_RESET(l_err);
            break;
        }

        //Initializes high cores data GpeRequest object
        l_rc = gpe_request_create(&G_high_cores.gpe_req,            // GpeRequest for the task
                                  &G_async_gpe_queue0,             // Queue
                                  IPC_ST_CORE_DATA_HIGH_FUNCID,    // Function ID
                                  &G_high_cores_data_parms,         // Task parameters
                                  SSX_WAIT_FOREVER,                // Timeout (none)
                                  NULL,                            // Callback
                                  NULL,                            // Callback arguments
                                  0 );                             // Options

        if( l_rc )
        {
            // If we failed to create the GpeRequest then there is a serious problem.
            MAIN_TRAC_ERR("Failure creating the high core data GpeRequest. [RC=0x%08x]", l_rc );

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
                                l_trace,                                  //Trace Buf
                                DEFAULT_TRACE_SIZE,                       //Trace Size
                                l_rc,                                       //Userdata1
                                0                                         //Userdata2
            );

            // commit error log
            REQUEST_RESET(l_err);
            break;
        }

    } while(0);

    // Initialize the core data control structures at the same time
// TEMP/TODO: Needs to be re-enabled when the data control task is enabled
//    proc_core_data_control_init();

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

    if( i_occ_core_id <= 11 )
    {
        //Returns a pointer to the most up-to-date bulk core data.
        return G_core_data_ptrs[i_occ_core_id];
    }
    else
    {
        //Core id outside the range
        TRAC_ERR("Invalid OCC core id [0x%x]", i_occ_core_id);
        return( NULL );
    }
}

#ifdef PROC_DEBUG
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
        PROC_DBG("\n-------------------------------\n");
        PROC_DBG("Core [%x] Sensors Data \n", core);
        // TODO: Commented these out b/c they take too long to run in task.
        //dumpHexString(&l_core_data->sensors_tod, sizeof(l_core_data->sensors_tod), "Sensor TOD");
        //dumpHexString(&l_core_data->sensors_v0, sizeof(l_core_data->sensors_v0), "Sensor VO");
        //dumpHexString(&l_core_data->sensors_v1, sizeof(l_core_data->sensors_v1), "Sensor V1");
        //dumpHexString(&l_core_data->sensors_v8, sizeof(l_core_data->sensors_v8), "Sensor V8");
        //dumpHexString(&l_core_data->sensors_v9, sizeof(l_core_data->sensors_v9), "Sensor V9");
        PROC_DBG("\n");
    }
    else
    {
        PROC_DBG("\n G_core_data_ptrs[%x] is NULL. This should not happen.\n", core);
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
        PROC_DBG("\n-------------------------\n");
        PROC_DBG("Core [%x] status \n", core);
        // TODO: Commented these out b/c they take too long to run in task.
        //dumpHexString(&l_core_data->core_tod, sizeof(l_core_data->core_tod), "Core TOD");
        //dumpHexString(&l_core_data->core_raw_cycles, sizeof(l_core_data->core_raw_cycles), "Core Raw Cycles");
        //dumpHexString(&l_core_data->core_run_cycles, sizeof(l_core_data->core_run_cycles), "Run Cycles");
        //dumpHexString(&l_core_data->core_dispatch, sizeof(l_core_data->core_dispatch), "Core Dispatch");
        //dumpHexString(&l_core_data->core_completion, sizeof(l_core_data->core_completion), "Core Completion");
        //dumpHexString(&l_core_data->core_workrate, sizeof(l_core_data->core_workrate), "Core Workrate");
        //dumpHexString(&l_core_data->core_spurr, sizeof(l_core_data->core_spurr), "Core Spurr");
        //dumpHexString(&l_core_data->core_mem_hler_a, sizeof(l_core_data->core_mem_hler_a), "Mem A");
        //dumpHexString(&l_core_data->core_mem_hler_b, sizeof(l_core_data->core_mem_hler_b), "Mem B");
        //dumpHexString(&l_core_data->mem_tod, sizeof(l_core_data->mem_tod), "Mem TOD");
        //dumpHexString(&l_core_data->mem_raw_cycles, sizeof(l_core_data->mem_raw_cycles), "Mem Raw Cycles");
        PROC_DBG("\n");
    }
    else
    {
        PROC_DBG("\n G_core_data_ptrs[%x] is NULL. This should not happen.\n", core);
    }
    return;
}

#endif

