/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/proc/proc_data_control.c $                            */
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
#include "gpe_control.h"
#include "occ_sys_config.h"
#include <amec_sys.h>

// Pore flex request for GPE job. The initialization will be done one time
// during pore flex create.
PoreFlex G_core_data_control_req;

// Global double buffering for core data control
GPE_BUFFER(PcbsPstateRegs G_core_data_control_a[MAX_NUM_HW_CORES]) = {{{0}}};
GPE_BUFFER(PcbsPstateRegs G_core_data_control_b[MAX_NUM_HW_CORES]) = {{{0}}};

// Pointer to the core data control that will be used by GPE engine.
GPE_BUFFER(PcbsPstateRegs * G_core_data_control_gpewrite_ptr) = { &G_core_data_control_a[0] };

// Pointer to the core data control that will be written to by the OCC FW.
GPE_BUFFER(PcbsPstateRegs * G_core_data_control_occwrite_ptr) = { &G_core_data_control_b[0] };

// The Gpe parameter fields are set up each time before the GPE starts.
GPE_BUFFER(GpeSetPstatesParms G_core_data_control_parms);

// Function Specification
//
// Name: proc_set_pstate
//
// Description: Function to demonstrate setting Pstates to all cores
//              Should only be run from RTL
//
// End Function Specification
void proc_set_pstate_all(Pstate i_pstate)
{
  uint8_t l_chiplet = 0;

  for(; l_chiplet<MAX_NUM_HW_CORES; l_chiplet++)
  {
     set_chiplet_pstate(G_core_data_control_occwrite_ptr,
                        l_chiplet,
                        i_pstate,
                        i_pstate);
  }

  PROC_DBG("Setting Pstates to %d\n",i_pstate);
}


// Function Specification
//
// Name: proc_set_core_pstate
//
// Description: Function to demonstrate setting Pstates to all cores
//              Should only be run from RTL
//
// End Function Specification
void proc_set_core_pstate(Pstate i_pstate, uint8_t i_core)
{
  set_chiplet_pstate(G_core_data_control_occwrite_ptr,
                        CORE_OCC2HW(i_core),
                        i_pstate,
                        i_pstate);
}

// Function Specification
//
// Name: proc_set_core_bounds
//
// Description: Function to set core pmin/pmax
//              Should only be run from RTL
//
// End Function Specification
void proc_set_core_bounds(Pstate i_pmin, Pstate i_pmax, uint8_t i_core)
{
    Pstate l_pmax;
    uint8_t l_hw_core = CORE_OCC2HW(i_core);

    //don't allow pmax to be set lower than pmin
    if(i_pmax < i_pmin)
    {
        l_pmax = i_pmin;
    }
    else
    {
        l_pmax = i_pmax;
    }

    set_chiplet_pmax(G_core_data_control_occwrite_ptr,
                     l_hw_core,
                     l_pmax);
    set_chiplet_pmin(G_core_data_control_occwrite_ptr,
                     l_hw_core,
                     i_pmin);
}


// Function Specification
//
// Name: proc_core_data_control_init
//
// Description: Initializations needed for core data control task
//
// End Function Specification
void proc_core_data_control_init( void )
{
    errlHndl_t l_err = NULL;   //Error handler
    tracDesc_t l_trace = NULL; //Temporary trace descriptor
    int         rc = 0; //Return code

    do
    {
        //Initializes PoreFlex object for fast core data
        rc = pore_flex_create( &G_core_data_control_req,  //gpe_req for the task
                &G_pore_gpe0_queue,                       //queue
                gpe_set_pstates,                          //entry point
                (uint32_t) &G_core_data_control_parms,    //parm for the task
                SSX_WAIT_FOREVER,                         //no timeout
                NULL,                                     //callback
                NULL,                                     //callback argument
                0 );                                      //options
        if( rc )
        {
            //If fail to create pore flex object then there is a problem.
            TRAC_ERR("Fail to create core control poreFlex object[0x%x]", rc );

            /*
             * @errortype
             * @moduleid    PROC_CORE_INIT_MOD
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   pore_flex_create return code
             * @userdata4   ERC_PROC_CONTROL_INIT_FAILURE
             * @devdesc     Failure to create poreflex object
             */
            l_err = createErrl(
                    PROC_CORE_INIT_MOD,                     //modId
                    SSX_GENERIC_FAILURE,                    //reasoncode
                    ERC_PROC_CONTROL_INIT_FAILURE,          //Extended reason code
                    ERRL_SEV_PREDICTIVE,                    //Severity
                    l_trace,    //TODO: create l_trace      //Trace Buf
                    DEFAULT_TRACE_SIZE,                     //Trace Size
                    rc,                                     //userdata1
                    0                                       //userdata2
                    );

            // commit error log
            REQUEST_RESET(l_err); //$gm006
            break;
        }

    } while(0);
}

// Function Specification
//
// Name: task_core_data_control
//
// Description: Control core actuation for all configured cores on every tick.
//
// End Function Specification
void task_core_data_control( task_t * i_task )
{
    errlHndl_t       l_err   = NULL;     //Error handler
    tracDesc_t       l_trace = NULL;     //Temporary trace descriptor
    int              rc      = 0;        //Return code
    PcbsPstateRegs * l_temp  = NULL;

    do
    {
        //Check to see if the previous GPE request still running
        if( !(async_request_is_idle(&G_core_data_control_req.request)) )
        {
            break;
        }

        //Check to see if the previosuly GPE request has been succeeded
        if( async_request_completed(&G_core_data_control_req.request) )
        {
            //If the previous GPE request succeeded then swap the
            //gpewrite ptr with the occwrite ptr.
            l_temp = G_core_data_control_occwrite_ptr;
            G_core_data_control_gpewrite_ptr = l_temp;

            //Signal that frequency vote has been applied (WOF depends on this)
            g_amec->proc[0].core_max_freq_actual = g_amec->proc[0].core_max_freq;
        }

        //Setup the core data control parms
        G_core_data_control_parms.config = (uint64_t) (((uint64_t) G_present_hw_cores) << 32);
        if(G_sysConfigData.system_type.kvm)
        {
            //Set the chiplet bounds (pmax/pmin) only on sapphire
            G_core_data_control_parms.select = GPE_SET_PSTATES_PMBR;
        }
        else
        {
            //Set the chiplet pstate request on non-sapphire systems
            G_core_data_control_parms.select = GPE_SET_PSTATES_PMCR;
        }

        G_core_data_control_parms.regs = (uint32_t) G_core_data_control_gpewrite_ptr;

        rc = pore_flex_schedule( &G_core_data_control_req );
        if( rc != 0 )
        {
            TRAC_ERR("Failed PoreFlex schedule core data control [%x] \n", rc);

            /*
             * @errortype
             * @moduleid    PROC_TASK_CORE_DATA_MOD
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   pore_flex_schedule return code
             * @userdata4   ERC_PROC_CONTROL_TASK_FAILURE
             * @devdesc     Failure to schedule poreflex object
             */
            l_err = createErrl(
                    PROC_TASK_CORE_DATA_MOD,                //modId
                    SSX_GENERIC_FAILURE,                    //reasoncode
                    ERC_PROC_CONTROL_TASK_FAILURE,          //Extended reason code
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
    } while(0);

    return;
}
