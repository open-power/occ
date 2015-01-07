/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/amec/amec_sensors_fw.c $                              */
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

/******************************************************************************/
/* Globals                                                                    */
/******************************************************************************/

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
    G_fw_timing.gpe_dur[i_gpe_engine] = DURATION_IN_US_UNTIL_NOW_FROM(G_fw_timing.rtl_start_gpe);
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
    errlHndl_t l_err                = NULL;
    int rc                          = 0;
    int rc2                         = 0;
    static bool l_first_call        = TRUE;
    bool l_gpe0_idle, l_gpe1_idle;
    static int L_consec_trace_count = 0;

    // ------------------------------------------------------
    // Update OCC Firmware Sensors from last tick
    // ------------------------------------------------------
    int l_last_state = G_fw_timing.amess_state;
    // RTLtickdur    = duration of last tick's RTL ISR (max = 250us)
    sensor_update( AMECSENSOR_PTR(RTLtickdur), G_fw_timing.rtl_dur);
    // AMEintdur     = duration of last tick's AMEC portion of RTL ISR
    sensor_update( AMECSENSOR_PTR(AMEintdur), G_fw_timing.ameint_dur);
    // AMESSdurX     = duration of last tick's AMEC state
    if(l_last_state >= NUM_AMEC_SMH_STATES)
    {
        // Sanity check.  Trace this out, even though it should never happen.
        TRAC_INFO("AMEC State Invalid, Sensor Not Updated");
    }
    else
    {
        // AMESSdurX = duration of last tick's AMEC state
        sensor_update( AMECSENSOR_ARRAY_PTR(AMESSdur0, l_last_state),  G_fw_timing.amess_dur);
    }

    // ------------------------------------------------------
    // Kick off GPE programs to track WorstCase time in GPE
    // and update the sensors.
    // ------------------------------------------------------
    if( (NULL != G_fw_timing.gpe0_timing_request)
        && (NULL != G_fw_timing.gpe1_timing_request) )
    {
        //Check if both GPE engines were able to complete the last GPE job on
        //the queue within 1 tick.
        l_gpe0_idle = async_request_is_idle(&G_fw_timing.gpe0_timing_request->request);
        l_gpe1_idle = async_request_is_idle(&G_fw_timing.gpe1_timing_request->request);
        if(l_gpe0_idle && l_gpe1_idle)
        {
            //reset the consecutive trace count
            L_consec_trace_count = 0;

            //Both GPE engines finished on time. Now check if they were
            //successful too.
            if( async_request_completed(&(G_fw_timing.gpe0_timing_request->request))
                && async_request_completed(&(G_fw_timing.gpe1_timing_request->request)) )
            {
                // GPEtickdur0 = duration of last tick's PORE-GPE0 duration
                sensor_update( AMECSENSOR_PTR(GPEtickdur0), G_fw_timing.gpe_dur[0]);
                // GPEtickdur1 = duration of last tick's PORE-GPE1 duration
                sensor_update( AMECSENSOR_PTR(GPEtickdur1), G_fw_timing.gpe_dur[1]);
            }
            else
            {
                //This case is expected on the first call of the function.
                //After that, this should not happen.
                if(!l_first_call)
                {
                    //Note: FFDC for this case is gathered by each task
                    //responsible for a GPE job.
                    TRAC_INFO("GPE task idle but GPE task did not complete");
                }
                l_first_call = FALSE;
            }

            // Update Time used to measure GPE duration.
            G_fw_timing.rtl_start_gpe = G_fw_timing.rtl_start;

            // Schedule the GPE Routines that will run and update the worst
            // case timings (via callback) after they complete.  These GPE
            // routines are the last GPE routines added to the queue
            // during the RTL tick.
            rc  = pore_flex_schedule(G_fw_timing.gpe0_timing_request);
            rc2 = pore_flex_schedule(G_fw_timing.gpe1_timing_request);

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
        }
        else if(L_consec_trace_count < MAX_CONSEC_TRACE)
        {
            uint64_t l_dbg0;
            uint64_t l_dbg1;
            uint64_t l_status;

            // Reset will eventually be requested due to not having power measurement
            // data after X ticks, but add some additional FFDC to the trace that
            // will tell us what GPE job is currently executing.
            if(!l_gpe0_idle)
            {
                l_dbg1 = in64(PORE_GPE0_DBG1);
                l_dbg0 = in64(PORE_GPE0_DBG0);
                l_status = in64(PORE_GPE0_STATUS);
                TRAC_ERR("GPE0 programs did not complete within one tick. DBG0[0x%08x%08x] DBG1[0x%08x%08x]",
                         (uint32_t)(l_dbg0 >> 32),
                         (uint32_t)(l_dbg0 & 0x00000000ffffffffull),
                         (uint32_t)(l_dbg1 >> 32),
                         (uint32_t)(l_dbg1 & 0x00000000ffffffffull));
                TRAC_ERR("Additional GPE0 debug data: STATUS[0x%08x%08x]",
                         (uint32_t)(l_status >> 32),
                         (uint32_t)(l_status & 0x00000000ffffffffull));
            }
            if(!l_gpe1_idle)
            {
                l_dbg1 = in64(PORE_GPE1_DBG1);
                l_dbg0 = in64(PORE_GPE1_DBG0);
                l_status = in64(PORE_GPE1_STATUS);
                TRAC_ERR("GPE1 programs did not complete within one tick. DBG0[0x%08x%08x] DBG1[0x%08x%08x]",
                         (uint32_t)(l_dbg0 >> 32),
                         (uint32_t)(l_dbg0 & 0x00000000ffffffffull),
                         (uint32_t)(l_dbg1 >> 32),
                         (uint32_t)(l_dbg1 & 0x00000000ffffffffull));
                TRAC_ERR("Additional GPE1 debug data: STATUS[0x%08x%08x]",
                         (uint32_t)(l_status >> 32),
                         (uint32_t)(l_status & 0x00000000ffffffffull));
            }
            L_consec_trace_count++;
        }
    }
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
