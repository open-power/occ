/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_sensors_fw.c $                          */
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

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <occ_common.h>
#include <ssx.h>
#include <errl.h>               // Error logging
#include "sensor.h"
#include "rtls.h"
#include "occ_sys_config.h"
#include "occ_service_codes.h"  // for SSX_GENERIC_FAILURE
#include "dcom.h"
#include "proc_data.h"
#include "amec_smh.h"
#include "amec_slave_smh.h"
#include <trac.h>
#include "amec_sys.h"
#include "sensor_enum.h"
#include "amec_service_codes.h"
#include <amec_sensors_fw.h>
#include "gpe_register_addresses.h"
#include "gpe_firmware_registers.h"

/******************************************************************************/
/* Globals                                                                    */
/******************************************************************************/
extern bool G_24x7_disabled;
bool G_log_gpe1_error = FALSE;

//*************************************************************************
// Code
//*************************************************************************

// Function Specification
//
// Name: amec_slv_update_smh_sensors
//
// Description: Update FW Sensors with Amec Slave Timings.
//
// End Function Specification
void amec_slv_update_smh_sensors(int i_smh_state, uint32_t i_duration)
{
    // Update the duration in the fw timing table
    G_fw_timing.amess_state = i_smh_state;
    G_fw_timing.amess_dur   = i_duration;
}


// Function Specification
//
// Name: amec_slv_update_gpe_sensors
//
// Description: Update FW Sensors with GPE Engine Timings. Called from
//              callback on GPE routine completion.
//
// End Function Specification
void amec_slv_update_gpe_sensors(uint8_t i_gpe_engine)
{
    // Update the duration in the fw timing table
    G_fw_timing.gpe_dur[i_gpe_engine] = DURATION_IN_US_UNTIL_NOW_FROM(G_fw_timing.rtl_start_gpe[i_gpe_engine]);
}

// Function Specification
//
// Name: task_gpe_timings
//
// Description: Kick off tasks to time GPEs
//
// Thread: RealTime Loop
//
// End Function Specification
#define MAX_CONSEC_TRACE 4
void task_gpe_timings(task_t * i_task)
{
    errlHndl_t l_err                = NULL;
    int rc                          = 0;
    int rc2                         = 0;
    static uint8_t L_idleTicks      = 0;
    static uint8_t L_busyTicks      = 0;
    static bool L_first_call        = TRUE;
    bool l_gpe0_idle                = TRUE;
    bool l_gpe1_idle                = TRUE;
    static uint8_t L_consec_trace_count[2] = {0};
    gpe_gpenxiramdbg_t xsr_sprg0 = {0};
    gpe_gpenxiramedr_t ir_edr = {0};
    gpe_gpenxidbgpro_t iar_xsr = {0};

    // ------------------------------------------------------
    // Kick off GPE programs to track WorstCase time in GPE
    // and update the sensors.
    // ------------------------------------------------------
    if(NULL != G_fw_timing.gpe0_timing_request)
    {
        //Check if GPE0 was able to complete the last GPE job within 1 tick
        l_gpe0_idle = async_request_is_idle(&G_fw_timing.gpe0_timing_request->request);

        if(l_gpe0_idle)
        {
            //reset the consecutive trace count
            L_consec_trace_count[0] = 0;

            //Now check if successful too.
            if( async_request_completed(&(G_fw_timing.gpe0_timing_request->request)) )
            {
                // GPEtickdur0 = duration of last tick's PORE-GPE0 duration
                sensor_update( AMECSENSOR_PTR(GPEtickdur0), G_fw_timing.gpe_dur[0]);
            }
            //This case is expected on the first call of the function.
            //After that, this should not happen.
            else if(!L_first_call)
            {
                //Note: FFDC for this case is gathered by each task
                //responsible for a GPE job.
                TRAC_INFO("GPE0 task idle but GPE0 task did not complete");
            }

            // Schedule the GPE Routines that will run and update the worst
            // case timings (via callback) after they complete.  These GPE
            // routines are the last GPE routines added to the queue
            // during the RTL tick.
            G_fw_timing.rtl_start_gpe[0] = G_fw_timing.rtl_start;
            rc  = gpe_request_schedule(G_fw_timing.gpe0_timing_request);
        }
        else
        {
            // Reset will eventually be requested due to not having power measurement
            // data after X ticks, but add some additional FFDC to the trace that
            // will tell us what GPE job is currently executing.
            INCREMENT_ERR_HISTORY(ERRH_GPE0_NOT_IDLE);

            if(L_consec_trace_count[0] < MAX_CONSEC_TRACE)
            {
               xsr_sprg0.fields.xsr = in32(GPE_GPE0XIXSR);
               xsr_sprg0.fields.sprg0 = in32(GPE_GPE0XISPRG0);
               ir_edr.fields.edr = in32(GPE_GPE0XIEDR);
               ir_edr.fields.ir = in32(GPE_GPE0XIIR);
               iar_xsr.fields.iar = in32(GPE_GPE0XIIAR);
               TRAC_ERR("GPE0 programs did not complete within one tick. "
                         "XSR[0x%08x]  IAR[0x%08x] IR[0x%08x] EDR[0x%08x] SPRG0[0x%08X]",
                         xsr_sprg0.fields.xsr, iar_xsr.fields.iar,
                         ir_edr.fields.ir, ir_edr.fields.edr, xsr_sprg0.fields.sprg0);
                L_consec_trace_count[0]++;
            }
        }

    }
    if(NULL != G_fw_timing.gpe1_timing_request)
    {
        //Check if GPE1 was able to complete the last GPE job within 1 tick
        l_gpe1_idle = async_request_is_idle(&G_fw_timing.gpe1_timing_request->request);

        if(l_gpe1_idle)
        {
            //Now check if successful too.
            if( async_request_completed(&(G_fw_timing.gpe1_timing_request->request)) )
            {
                // GPEtickdur1 = duration of last tick's PORE-GPE1 duration
                sensor_update( AMECSENSOR_PTR(GPEtickdur1), G_fw_timing.gpe_dur[1]);
            }
            //This case is expected on the first call of the function.
            //After that, this should not happen.
            else if(!L_first_call)
            {
                //Note: FFDC for this case is gathered by each task
                //responsible for a GPE job.
                TRAC_INFO("GPE1 task idle but GPE1 task did not complete");
            }

            // Schedule the GPE Routines that will run and update the worst
            // case timings (via callback) after they complete.  These GPE
            // routines are the last GPE routines added to the queue
            // during the RTL tick.
            G_fw_timing.rtl_start_gpe[1] = G_fw_timing.rtl_start;
            rc2 = gpe_request_schedule(G_fw_timing.gpe1_timing_request);

        }
        else
        {
            INCREMENT_ERR_HISTORY(ERRH_GPE1_NOT_IDLE);
            // Log error and request reset if GPE1 issue has gone on long enough to cause real issues
            // i.e. timeout collecting memory temperatures

            if( (L_consec_trace_count[1] < MAX_CONSEC_TRACE) || (G_log_gpe1_error) )
            {
                xsr_sprg0.fields.xsr = in32(GPE_GPE1XIXSR);
                xsr_sprg0.fields.sprg0 = in32(GPE_GPE1XISPRG0);
                ir_edr.fields.edr = in32(GPE_GPE1XIEDR);
                ir_edr.fields.ir = in32(GPE_GPE1XIIR);
                iar_xsr.fields.iar = in32(GPE_GPE1XIIAR);
                TRAC_ERR("GPE1 programs did not complete within one tick. "
                         "XSR[0x%08x]  IAR[0x%08x] IR[0x%08x] EDR[0x%08x] SPRG0[0x%08X]",
                         xsr_sprg0.fields.xsr, iar_xsr.fields.iar,
                         ir_edr.fields.ir, ir_edr.fields.edr, xsr_sprg0.fields.sprg0);
                L_consec_trace_count[1]++;

                if(G_log_gpe1_error)
                {
                  TRAC_ERR("GPE1 not idle causing timeouts, need to reset!!!");
                  /* @
                   * @errortype
                   * @moduleid    AMEC_UPDATE_FW_SENSORS
                   * @reasoncode  GPE_REQUEST_TASK_TIMEOUT
                   * @userdata1   0
                   * @userdata2   0
                   * @userdata4   ERC_AMEC_GPE1_TIMEOUT
                   * @devdesc     Tasks on GPE1 failing to complete
                   */
                  l_err = createErrl(AMEC_UPDATE_FW_SENSORS,             //modId
                                       GPE_REQUEST_TASK_TIMEOUT,           //reasoncode
                                       ERC_AMEC_GPE1_TIMEOUT,              //Extended reason code
                                       ERRL_SEV_UNRECOVERABLE,             //Severity
                                       NULL,                               //Trace Buf
                                       DEFAULT_TRACE_SIZE,                 //Trace Size
                                       0,                                  //userdata1
                                       0);                                 //userdata2

                  // Commit error log and request reset
                  REQUEST_RESET(l_err);
                }
            }
        }
    }
    if(rc || rc2)
    {
        /* @
         * @errortype
         * @moduleid    AMEC_UPDATE_FW_SENSORS
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   return code - gpe0
         * @userdata2   return code - gpe1
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Failure to schedule PORE-GPE poreFlex object for FW timing
         *              analysis.
         */
        l_err = createErrl(
            AMEC_UPDATE_FW_SENSORS,             //modId
            SSX_GENERIC_FAILURE,                //reasoncode
            OCC_NO_EXTENDED_RC,                 //Extended reason code
            ERRL_SEV_INFORMATIONAL,             //Severity
            NULL,                               //Trace Buf
            DEFAULT_TRACE_SIZE,                 //Trace Size
            rc,                                 //userdata1
            rc2);                               //userdata2

        // commit error log
        commitErrl( &l_err );
    }

    // Process GPEs timings and determine whether to disable/enable 24x7
    if(l_gpe0_idle && l_gpe1_idle)
    {
        // Reset how many consecutive ticks we were busy for.
        L_busyTicks = 0;
        L_idleTicks++;

        // If we were idle for a complete cycle thru all ticks, re-enable 24x7
        if( L_idleTicks >= MAX_NUM_TICKS )
        {
            G_24x7_disabled = FALSE;
        }
    }
    else
    {
        // Reset how many consecutive ticks we were idle for
        L_idleTicks = 0;
        L_busyTicks++;

        // disable 24x7 if it isn't already, this will stay disabled until we can run all
        // ticks consecutively
        if( !G_24x7_disabled )
        {
            G_24x7_disabled = TRUE;
            INCREMENT_ERR_HISTORY(ERRH_24X7_DISABLED);
        }
    }

    L_first_call = FALSE;
}


// Function Specification
//
// Name: amec_update_fw_sensors
//
// Description: Updates sensors related to the OCC FW Timings
//
// Thread: RealTime Loop
//
// End Function Specification
#define MAX_CONSEC_TRACE 4
void amec_update_fw_sensors(void)
{
    // ------------------------------------------------------
    // Update OCC Firmware Sensors from last tick
    // ------------------------------------------------------
    // RTLtickdur    = duration of last tick's RTL ISR (max = 250us)
    sensor_update( AMECSENSOR_PTR(RTLtickdur), G_fw_timing.rtl_dur);
    // AMEintdur     = duration of last tick's AMEC portion of RTL ISR
    sensor_update( AMECSENSOR_PTR(AMEintdur), G_fw_timing.ameint_dur);
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
