/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/proc/proc_data.c $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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
#include "pgp_async.h"
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

//Global array of core data buffers
GPE_BUFFER(gpe_bulk_core_data_t G_core_data[MAX_NUM_FW_CORES+NUM_CORE_DATA_DOUBLE_BUF+NUM_CORE_DATA_EMPTY_BUF]) = {{{0}}};

//Global array of core data pointers
gpe_bulk_core_data_t * G_core_data_ptrs[MAX_NUM_FW_CORES] = { &G_core_data[0], &G_core_data[1],
   &G_core_data[2], &G_core_data[3], &G_core_data[4], &G_core_data[5], &G_core_data[6],
   &G_core_data[7], &G_core_data[8], &G_core_data[9], &G_core_data[10], &G_core_data[11] };

//Global structures for gpe get core data parms
GPE_BUFFER(GpeGetCoreDataParms G_low_cores_data_parms);
GPE_BUFFER(GpeGetCoreDataParms G_high_cores_data_parms);

//We will have separate bulk core data structure for low and high cores.
//Global low and high cores structures used for task data pointers.
bulk_core_data_task_t G_low_cores = { 0, 0, 5, &G_core_data[12] };
bulk_core_data_task_t G_high_cores = { 6, 6, 11, &G_core_data[13] };

//AMEC needs to know when data for a core has been collected.
uint32_t G_updated_core_mask = 0;

// Mask to indicate when an empath error has been detected and empath data
// should be ignored.  Core bits are cleared when empath data is collected
// without error.
uint32_t G_empath_error_core_mask = 0;

//Global G_present_cores is bitmask of all cores
//(1 = present, 0 = not present. Core 0 has the most significant bit)
uint32_t G_present_cores = 0;

//Global double buffering for fast core data collection.
GPE_BUFFER(gpe_fast_core_data_t G_fast_core_data[NUM_FAST_CORE_DATA_BUFF]) = {{0}};

//Pointer to the latest fast core data that will be used by AMEC code.
GPE_BUFFER(gpe_fast_core_data_t * G_read_fast_core_data_ptr) = { &G_fast_core_data[0] };

//Pointer used by GPE code to write fast core data.
GPE_BUFFER(gpe_fast_core_data_t * G_write_fast_core_data_ptr) = { &G_fast_core_data[1] };

//Globals structure for gpe get chip data fast parms.
//The Gpe parameter fields are set up each time before collect data start.
GPE_BUFFER(GpeGetChipDataFastParms G_chip_data_fast_parms);

//Pore flex request for GPE job. The initialization
//will be done one time during pore flex create.
PoreFlex G_fast_cores_req;

//Global G_present_hw_cores is bitmask of all hardware cores
//(1 = present, 0 = not present. Core 0 has the most significant bit)
uint32_t G_present_hw_cores = 0;

//OCC to HW core id mapping array
uint8_t G_occ2hw_core_id[MAX_NUM_HW_CORES] = { 1, 2, 3, 4, 5, 6, 9, 10, 11, 12, 13, 14, 0, 7, 8, 15 };

//HW to OCC core id mapping array
uint8_t G_hw2occ_core_id[MAX_NUM_HW_CORES] = { 12, 0, 1, 2, 3, 4, 5, 13, 14, 6, 7, 8, 9, 10, 11, 15 };

//Flag to keep tract of one time trace for GPE running case
//for task core data.
bool    G_queue_not_idle_traced = FALSE;

//Flag to keep tract of one time trace for GPE running case
//for Fast core data.
bool    G_fast_core_queue_not_idle_traced = FALSE;

// Global to track the maximum time elapsed between pore flex schedules of
// per core get_per_core_data tasks.  The array is indexed by core number.
uint32_t G_get_per_core_data_max_schedule_intervals[MAX_NUM_HW_CORES] = {0,};

// Declaration of debug functions
#ifdef PROC_DEBUG
void print_core_data_sensors(uint8_t core);
void print_core_status(uint8_t core);
void print_fast_core_data(void);
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

    errlHndl_t  l_err = NULL;       //Error handler
    tracDesc_t  l_trace = NULL;     //Temporary trace descriptor
    int         rc = 0;     //return code
    bulk_core_data_task_t * l_bulk_core_data_ptr = (bulk_core_data_task_t *)i_task->data_ptr;
    GpeGetCoreDataParms * l_parms = (GpeGetCoreDataParms *)(l_bulk_core_data_ptr->gpe_req.parameter);
    gpe_bulk_core_data_t  * l_temp = NULL;

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

            // The gpe_data collection code has to handle the workaround for
            // HW280375.  Two new flags have been added to the OHA_RO_STATUS_REG
            // image to indicate whether the EMPATH collection failed, and
            // whether it was due to an "expected" error that we can ignore
            // (we can ignore the data as well), or an "unexpected" error that
            // we will create an informational log one time.
            //
            // The "expected" errors are very rare in practice, in fact we may
            // never even see them unless running a specific type of workload.
            // If you want to test the handling of expected errors compile the
            // GPE code with -DINJECT_HW280375_ERRORS which will inject an error
            // approximately every 1024 samples
            //
            // To determine if the expected error has occurred inspect the
            // CoreDataOha element of the CoreData structure written by the GPE
            // core data job.  The OHA element contains the oha_ro_status_reg.
            // Inside the OHA status register is a 16 bit reserved field.
            // gpe_data.h defines two masks that can be applied against the
            // reserved field to check for these errors:
            // CORE_DATA_EXPECTED_EMPATH_ERROR
            // CORE_DATA_UNEXPECTED_EMPATH_ERROR
            // Also, a 4-bit PCB parity + error code is saved at bit position:
            // CORE_DATA_EMPATH_ERROR_LOCATION, formally the length is
            // specified by: CORE_DATA_EMPATH_ERROR_BITS
            gpe_bulk_core_data_t *l_core_data =
                    G_core_data_ptrs[l_bulk_core_data_ptr->current_core];

            // We will trace the errors, but only a certain number of
            // times, we will only log the unexpected error once.
#define OCC_EMPATH_ERROR_THRESH 10
            static uint32_t L_expected_emp_err_cnt = 0;
            static uint32_t L_unexpected_emp_err_cnt = 0;

            // Check the reserved field for the expected or the unexpected error flag
            if ((l_core_data->oha.oha_ro_status_reg.fields._reserved0 & CORE_DATA_EXPECTED_EMPATH_ERROR)
                ||
                (l_core_data->oha.oha_ro_status_reg.fields._reserved0 & CORE_DATA_UNEXPECTED_EMPATH_ERROR))
            {
                // Indicate empath error on current core
                G_empath_error_core_mask |=
                        CORE0_PRESENT_MASK >> (l_bulk_core_data_ptr->current_core);

                // Save the high and low order words of the OHA status reg
                uint32_t l_oha_reg_high = l_core_data->oha.oha_ro_status_reg.words.high_order;
                uint32_t l_oha_reg_low = l_core_data->oha.oha_ro_status_reg.words.low_order;

                // Handle each error case
                if ((l_core_data->oha.oha_ro_status_reg.fields._reserved0 & CORE_DATA_EXPECTED_EMPATH_ERROR)
                    &&
                    (L_expected_emp_err_cnt < OCC_EMPATH_ERROR_THRESH))
                {
                    L_expected_emp_err_cnt++;
                    TRAC_IMP("Expected empath collection error occurred %d time(s)! Core = %d",
                             L_expected_emp_err_cnt,
                             l_bulk_core_data_ptr->current_core);
                    TRAC_IMP("OHA status register: 0x%4.4x%4.4x",
                             l_oha_reg_high, l_oha_reg_low);
                }

                if ((l_core_data->oha.oha_ro_status_reg.fields._reserved0 & CORE_DATA_UNEXPECTED_EMPATH_ERROR)
                    &&
                    (L_unexpected_emp_err_cnt < OCC_EMPATH_ERROR_THRESH))
                {
                    L_unexpected_emp_err_cnt++;
                    TRAC_ERR("Unexpected empath collection error occurred %d time(s)! Core = %d",
                             L_unexpected_emp_err_cnt,
                             l_bulk_core_data_ptr->current_core);
                    TRAC_ERR("OHA status register: 0x%4.4x%4.4x",
                             l_oha_reg_high, l_oha_reg_low);

                    // Create and commit an informational error the first
                    // time this occurs.
                    if (L_unexpected_emp_err_cnt == 1)
                    {
                        TRAC_IMP("Logging unexpected empath collection error 1 time only.");
                        /*
                        * @errortype
                        * @moduleid    PROC_TASK_CORE_DATA_MOD
                        * @reasoncode  INTERNAL_HW_FAILURE
                        * @userdata1   OHA status reg high
                        * @userdata2   OHA status reg low
                        * @userdata4   ERC_PROC_CORE_DATA_EMPATH_ERROR
                        * @devdesc     An unexpected error occurred while
                        *              collecting core empath data.
                        */
                        l_err = createErrl(
                                PROC_TASK_CORE_DATA_MOD, //modId
                                INTERNAL_HW_FAILURE,     //reason code
                                ERC_PROC_CORE_DATA_EMPATH_ERROR, //Extended reason code
                                ERRL_SEV_INFORMATIONAL,  //Severity
                                NULL,                    //Trace
                                DEFAULT_TRACE_SIZE,      //Trace Size
                                l_oha_reg_high,          //userdata1
                                l_oha_reg_low);          //userdata2

                        commitErrl(&l_err);
                    }
                }
            }
        }

        // If the core is not present, then we need to point to the empty G_core_data
        // so that we don't use old/stale data from a leftover G_core_data
        if( !CORE_PRESENT(l_bulk_core_data_ptr->current_core))
        {
            G_core_data_ptrs[l_bulk_core_data_ptr->current_core] = &G_core_data[14];
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
            PROC_DBG("Schedule PoreFlex OCC core [%x] or HW core [%x]\n",
                      l_bulk_core_data_ptr->current_core, CORE_OCC2HW(l_bulk_core_data_ptr->current_core));

            //1. Setup the get core data parms
            l_parms->config =
                    ((uint64_t) CORE0_PRESENT_MASK_GPE) >> (CORE_OCC2HW(l_bulk_core_data_ptr->current_core));

            if( (cfam_id() == CFAM_CHIP_ID_MURANO_10)
                || (cfam_id() == CFAM_CHIP_ID_MURANO_11)
                || (cfam_id() == CFAM_CHIP_ID_MURANO_12) )
            {
                // Due to HW243646 & HW243942 fallout, we will not be collecting EX core
                // activity counter scoms until Murano DD1.3
                l_parms->select = GPE_GET_CORE_DATA_DTS_CPM | GPE_GET_CORE_DATA_PCB_SLAVE;
            }
            else
            {
                l_parms->select = GPE_GET_CORE_DATA_ALL;
            }

            l_parms->data = (uint32_t) l_bulk_core_data_ptr->core_data_ptr;

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
//                    TRAC_INFO("New max get_per_core_data interval: core=%d, interval(us)=%d",
//                              l_current_core, l_elapsed_us);
                }
                // Also sniff if the request has actually completed, it is checked above but
                // the schedule proceeds regardless which could be dangerous...
                if (!async_request_completed(&l_bulk_core_data_ptr->gpe_req.request))
                {
                    TRAC_ERR("Async get_per_core_data task for core=%d not complete!",
                             l_current_core);
                }
            }

            // The PCBS heartbeat timer may time out even though we are scheduling
            // the get_per_core_data task for each core every 2ms.  Reason at time
            // of this code change is still unknown but reproducible.  We are going
            // to getscom the PMSTATEHISTOCC register (PCBS_PMSTATEHISTOCC_REG) for
            // each configured core before scheduling the PORE job to try and make
            // sure the heartbeat doesn't expire.
            // Recent tests show the heartbeat is holding steady so the getscom is
            // commented out and incomplete, the core number needs to be converted
            // to a hardware core number if this code is activated.
            //uint64_t l_dont_care = 0;
            //uint32_t core = CORE_CHIPLET_ADDRESS(PCBS_PMSTATEHISTOCC_REG, ???);
            //getscom(address???, &l_dont_care);

            //2. Port flex schedule gpe_get_per_core_data
            //   Check pore_flex_schedule return code if error
            //   then request OCC reset.

            rc = pore_flex_schedule( &l_bulk_core_data_ptr->gpe_req );
            if( rc != 0 )
            {
                //Error in schedule gpe get core data
                TRAC_ERR("Failed PoreFlex schedule core [%x] \n", rc);

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
                        rc,                                 //userdata1
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
     errlHndl_t l_err = NULL;   //Error handler
     tracDesc_t l_trace = NULL; //Temporary trace descriptor
     int         rc = 0;    //Return code

     do
     {
    //Before RTLoop starts to collect data. We need to determine what cores
    //are present and configured. We have a register that has information
    //of all cores that have been deconfigured in the chip so we need to read that.

    G_present_hw_cores = in32(PMC_CORE_DECONFIGURATION_REG);

    //Inverse the bitmask of the deconfigured register to get present
    //cores and save it in the global present hardware cores.
    G_present_hw_cores = ~G_present_hw_cores & HW_CORES_MASK;

    //Convert hardware core numering to OCC core numering.
    G_present_cores = ((G_present_hw_cores & LO_CORES_MASK) << 1) |
                ((G_present_hw_cores & HI_CORES_MASK) << 3);

    PROC_DBG("G_present_hw_cores =[%x] and G_present_cores =[%x] \n",
            G_present_hw_cores, G_present_cores);

    //Initializes low cores data PoreFlex object
    rc = pore_flex_create(  &G_low_cores.gpe_req,   //gpe_req for the task
                            &G_pore_gpe0_queue,     //queue
                            gpe_get_per_core_data,  //entry point
                                                    //parm for the task
                            (uint32_t) &G_low_cores_data_parms,
                            SSX_WAIT_FOREVER,       // no timeout
                            NULL,                   //callback
                            NULL,                   //callback argument
                            0 );                    //options
    if( rc )
    {
        //If fail to create pore flex object then there is a problem.
        TRAC_ERR("Fail to create low cores poreFlex object[0x%x]", rc );

        /*
         * @errortype
         * @moduleid    PROC_CORE_INIT_MOD
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   pore_flex_create return code
         * @userdata4   ERC_LOW_CORE_PORE_FLEX_CREATE_FAILURE
         * @devdesc     Failure to create low cores poreflex object
         */
        l_err = createErrl(
                PROC_CORE_INIT_MOD,                     //modId
                SSX_GENERIC_FAILURE,                    //reasoncode
                ERC_LOW_CORE_PORE_FLEX_CREATE_FAILURE,  //Extended reason code
                ERRL_SEV_PREDICTIVE,                    //Severity
                l_trace,    //TODO: create l_trace      //Trace Buf
                DEFAULT_TRACE_SIZE,                     //Trace Size
                rc,                                     //userdata1
                0                                       //userdata2
        );

        // commit error log
        REQUEST_RESET(l_err);
        break;
    }

    //Initializes existing PoreFlex object for high cores data
    rc = pore_flex_create(  &G_high_cores.gpe_req,      //gpe_req for the task
                            &G_pore_gpe0_queue,         //queue
                            gpe_get_per_core_data,      //entry point
                                                        //parm for the task
                            (uint32_t) &G_high_cores_data_parms,
                            SSX_WAIT_FOREVER,           //no timeout
                            NULL,                       //callback
                            NULL,                       //callback argument
                            0 );                        //options
    if( rc )
    {
        //If fail to create pore flex object then there is a problem.
        TRAC_ERR("Fail to create high cores poreFlex object[0x%x]", rc );

        /*
         * @errortype
         * @moduleid    PROC_CORE_INIT_MOD
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   pore_flex_create return code
         * @userdata4   ERC_HIGH_CORE_PORE_FLEX_CREATE_FAILURE
         * @devdesc     Failure to create high core poreFlex object
         */
        l_err = createErrl(
                            PROC_CORE_INIT_MOD,                     //modId
                            SSX_GENERIC_FAILURE,                    //reasoncode
                            ERC_HIGH_CORE_PORE_FLEX_CREATE_FAILURE, //Extended reason code
                            ERRL_SEV_PREDICTIVE,                    //Severity
                            l_trace,    //TODO: create l_trace      //Trace Buf
                            DEFAULT_TRACE_SIZE,                     //Trace Size
                            rc,                                     //userdata1
                            0                                       //userdata2
        );

        // commit error log
        REQUEST_RESET(l_err);
        break;
    }

        //Initializes PoreFlex object for fast core data
        rc = pore_flex_create( &G_fast_cores_req,               //gpe_req for the task
                                &G_pore_gpe0_queue,             //queue
                                gpe_get_core_data_fast,         //entry point
                                                                //parm for the task
                                (uint32_t) &G_chip_data_fast_parms,
                               SSX_WAIT_FOREVER,                //no timeout
                                NULL,                           //callback
                                NULL,                           //callback argument
                                0 );                            //options
        if( rc )
        {
            //If fail to create pore flex object then there is a problem.
            TRAC_ERR("Fail to create fast cores poreFlex object[0x%x]", rc );

            /*
             * @errortype
             * @moduleid    PROC_CORE_INIT_MOD
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   pore_flex_create return code
             * @userdata4   ERC_FAST_CORE_PORE_FLEX_CREATE_FAILURE
             * @devdesc     Failure to create fast core data poreFlex object
             */
            l_err = createErrl(
                            PROC_CORE_INIT_MOD,                     //modId
                            SSX_GENERIC_FAILURE,                    //reasoncode
                            ERC_FAST_CORE_PORE_FLEX_CREATE_FAILURE, //Extended reason code
                            ERRL_SEV_PREDICTIVE,                    //Severity
                            l_trace, //TODO: create l_trace point   //Trace Buf
                            DEFAULT_TRACE_SIZE,                     //Trace Size
                            rc,                                     //userdata1
                            0                                       //userdata2
            );

            // commit error log
            REQUEST_RESET(l_err);
            break;
        }

    } while(0);

    // Initialize the core data control structures at the same time
    proc_core_data_control_init();

    return;
}

// Function Specification
//
// Name: task_fast_core_data
//
// Description: Collect fast core data for all configured cores on every tick.
//
// End Function Specification

void task_fast_core_data( task_t * i_task )
{

    errlHndl_t  l_err = NULL;   //Error handler
    tracDesc_t l_trace = NULL;  //Temporary trace descriptor
    int         rc = 0;         //Return code
    gpe_fast_core_data_t  * l_temp = NULL;
    uint32_t    l_pres_hw_cores;

    //poll the pmc deconfig register for newly deconfigured cores
    l_pres_hw_cores = (~in32(PMC_CORE_DECONFIGURATION_REG)) & HW_CORES_MASK;
    if(l_pres_hw_cores != G_present_hw_cores)
    {
        TRAC_IMP("Present cores changed. old_hw_mask[0x%04x] new_hw_mask[0x%04x]",
                G_present_hw_cores, l_pres_hw_cores);

        //update our global core presence masks
        G_present_hw_cores = l_pres_hw_cores;
        G_present_cores = ((l_pres_hw_cores & LO_CORES_MASK) << 1) |
                ((l_pres_hw_cores & HI_CORES_MASK) << 3);
    }

    do
    {
        //Check to see if the previous GPE request still running
        if( !(async_request_is_idle(&G_fast_cores_req.request)) )
        {
                //This should not happen unless there's a problem
                //Trace 1 time
                if( !G_fast_core_queue_not_idle_traced )
                {
                        TRAC_ERR("GPE is still running \n");
                        G_fast_core_queue_not_idle_traced = TRUE;
                }
                break;
        }

        //Check to see if the previosuly GPE request has been succeeded
        if( async_request_completed(&G_fast_cores_req.request) )
        {
            //If the previous GPE request succeeded then swap the
            //G_read_fast_core_data_ptr with the G_write_fast_core_data_ptr.

            PROC_DBG("Fast core data GPE request has been succeeded.\n");

            #ifdef  PROC_DEBUG
                print_fast_core_data();
            #endif

            l_temp = G_write_fast_core_data_ptr;
            G_write_fast_core_data_ptr = G_read_fast_core_data_ptr;
            G_read_fast_core_data_ptr = l_temp;
        }

    //Setup the get fast core data parms
    G_chip_data_fast_parms.config = (uint64_t) (((uint64_t) G_present_hw_cores) << 32);
    G_chip_data_fast_parms.select = GPE_GET_CORE_DATA_FAST_FREQ_TARGET;
    G_chip_data_fast_parms.data = (uint32_t) G_write_fast_core_data_ptr;

    //Port flex schedule gpe_get_core_data_fast
    //Check pore_flex_schedule return code if error
    //then request OCC reset.

    rc = pore_flex_schedule( &G_fast_cores_req );
    if( rc != 0 )
    {
        //Error in schedule gpe get fast core data
        TRAC_ERR("Failed PoreFlex schedule fast core data [%x] \n", rc);

        /*
         * @errortype
         * @moduleid    PROC_TASK_FAST_CORE_DATA_MOD
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   pore_flex_schedule return code
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     SSX PORE related failure
         */
        l_err = createErrl(
            PROC_TASK_FAST_CORE_DATA_MOD,           //modId
            SSX_GENERIC_FAILURE,                    //reasoncode
            OCC_NO_EXTENDED_RC,                     //Extended reason code
            ERRL_SEV_PREDICTIVE,                    //Severity
            l_trace, //TODO: create l_trace point   //Trace Buf
            DEFAULT_TRACE_SIZE,                     //Trace Size
            rc,                                     //userdata1
            0 );                                    //userdata2

        // commit error log
        REQUEST_RESET(l_err);
        break;
    }
    } while(0);

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
gpe_bulk_core_data_t * proc_get_bulk_core_data_ptr( const uint8_t i_occ_core_id )
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

// Function Specification
//
// Name: proc_get_fast_core_data_ptr
//
// Description: Returns a pointer to the most up-to-date fast core data
//
// End Function Specification
gpe_fast_core_data_t * proc_get_fast_core_data_ptr( void )
{
    //Returns a pointer to the most up-to-date fast core data.
    //Core data is organized is organized in hardware scheme.

    return G_read_fast_core_data_ptr;
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
    gpe_bulk_core_data_t * l_core_data = proc_get_bulk_core_data_ptr(core);

    if( l_core_data != NULL )
    {
        PROC_DBG("\n-------------------------------\n");
        PROC_DBG("Core [%x] Sensors Data \n", core);
        // TODO: Commented these out b/c they take too long to run in task.
        //       Consider moving this to an applet
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
    gpe_bulk_core_data_t * l_core_data = proc_get_bulk_core_data_ptr(core);

    if( l_core_data != NULL )
    {
        PROC_DBG("\n-------------------------\n");
        PROC_DBG("Core [%x] status \n", core);
        // TODO: Commented these out b/c they take too long to run in task.
        //       Consider moving this to an applet
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

// Function Specification
//
// Name: print_fast_core_data
//
// Description: Print out fast core data of the chip
//
// End Function Specification

void print_fast_core_data(void)
{
    gpe_fast_core_data_t * l_fast_core_data = proc_get_fast_core_data_ptr();

    if( l_fast_core_data != NULL )
    {
        PROC_DBG("\n---------------------------\n");
        // TODO: Commented these out b/c they take too long to run in task.
        //       Consider moving this to an applet
        //dumpHexString(&l_fast_core_data->tod, sizeof(l_fast_core_data->tod), "Fast Core Data TOD");
        //dumpHexString(&l_fast_core_data->core_data, sizeof(fast_core_data_t) * MAX_NUM_HW_CORES, "Fast Core Data");
        PROC_DBG("\n");
    }
    else
    {
        PROC_DBG("\n G_read_fast_core_data_ptr is NULL. This should not happen.\n");
    }
    return;
}
#endif

