/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/proc/proc_pstate.c $                              */
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

#include "ssx.h"
#include "proc_data_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "rtls.h"
#include "dcom.h"
#include "occ_common.h"
#include "state.h"
#include "cmdh_fsp_cmds.h"
#include "proc_data.h"
#include "proc_pstate.h"
#include "homer.h"
#include <amec_freq.h>
#include <common.h>
#include <amec_oversub.h>
#include <amec_sys.h>
#include <pstate_pgpe_occ_api.h>
#include <pstates_occ.H>

//OPAL processor and memory throttle reason coming from the frequency voting boxes.
extern opal_proc_voting_reason_t G_amec_opal_proc_throt_reason;
extern opal_mem_voting_reason_t  G_amec_opal_mem_throt_reason;

//Global OCC Pstate Parameters Block Structure
extern OCCPstateParmBlock_t G_oppb;

//Trace flags
extern uint16_t G_allow_trace_flags;

// Indicates if we have determined GPU presence
extern bool G_gpu_config_done;
// GPU Present bit mask
extern uint32_t G_first_proc_gpu_config;

//Holds Fmax for ease of proc_freq2pstate calculation = max(fturbo,futurbo)
uint16_t G_proc_fmax_mhz;

uint8_t         G_desired_pstate[MAXIMUM_QUADS];

// A global variable indicating whether the pstates have been enabled.
// initialized to PSTATES_DISABLED, turns to PSTATES_ENABLED only after
// the PGPE IPC that enable pstates completes successfully. While the IPC
// task is still running, this variable be set to PSTATES_IN_TRANSITION
volatile pstateStatus G_proc_pstate_status = PSTATES_DISABLED;

// A Global parameter indicating the owner of the PMCR.
volatile PMCR_OWNER G_proc_pmcr_owner = PMCR_OWNER_HOST;

// OPAL Dynamic data, updated whenever any OCC G_opal_table.dynamic parameter change
// Since this is happening multiple times need to keep track of it being scheduled
DMA_BUFFER( opal_dynamic_table_t G_opal_dynamic_table ) = {{0}};
bool G_opal_dynamic_bce_req_scheduled = false;
BceRequest G_opal_dynamic_bce_req;
#define OPAL_DYNAMIC_UPDATE_BCE_RETRIES 2

// OPAL Static data, updated once at transition to active state
DMA_BUFFER( opal_static_table_t  G_opal_static_table )  = {{0}};
BceRequest G_opal_static_bce_req;



volatile uint8_t G_opal_table_update_state = OPAL_TABLE_UPDATE_IDLE;

// Function Specification
//
// Name:  proc_is_hwpstate_enabled
//
// Description:  Checks OCC to see if Pstate HW Mode is enabled.
//
// End Function Specification
bool proc_is_hwpstate_enabled(void)
{
    return ( G_proc_pstate_status == PSTATES_ENABLED ? TRUE : FALSE);
}

// Function Specification
//
// Name:  proc_pstate2freq
//
// Description:  Convert Pstate to Frequency in kHz
//
// End Function Specification
uint32_t proc_pstate2freq(Pstate_t i_pstate)
{
    // The higher the pstate number, the lower the frequency:
    // If passed in Pstate is lower than Pmin (higher pstate value),
    // just use Pmin.
    if(i_pstate > G_oppb.pstate_min)
    {
        i_pstate = G_oppb.pstate_min;
    }

    // Calculate Frequency in kHz based on Pstate
    return ( G_oppb.frequency_max_khz - (i_pstate * G_oppb.frequency_step_khz));
}

// Function Specification
//
// Name:  proc_freq2pstate
//
// Description:  Convert Frequency to Nearest Pstate
//
// End Function Specification
Pstate_t proc_freq2pstate(uint32_t i_freq_mhz)
{
    int8_t   l_pstate = 0;
    int8_t   l_temp_pstate = 0;
    int32_t  l_temp_freq_khz = 0;
    uint32_t l_freq_khz = 0;

    do
    {
        // Freq Units need to be in kHz, not Mhz for the following calculations
        l_freq_khz = i_freq_mhz * 1000;

        // Return Pmin if the frequency is below or equal to the min Freq for the lowest Pstate
        if(l_freq_khz <= G_oppb.frequency_min_khz )
        {
            l_pstate = G_oppb.pstate_min;
            break;
        }

        if(i_freq_mhz < G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY])
        {
            l_freq_khz =  G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY] * 1000;
        }

        if(l_freq_khz < G_oppb.frequency_max_khz)
        {
            // First, calculate the delta between passed in freq, and Pmax
            l_temp_freq_khz = G_oppb.frequency_max_khz - l_freq_khz;

            // Next, calculate how many Pstate steps there are in that delta
            l_temp_pstate = l_temp_freq_khz / (int32_t) G_oppb.frequency_step_khz;

            // Higher Pstates are lower frequency, add number of steps to the highest frequency Pstate
            l_pstate = PMAX + l_temp_pstate;

            // As an extra safety check make sure the calculated Pstate is not out of the PGPE range
            // this should never happen!
            if(l_pstate > G_oppb.pstate_min)
            {
                TRAC_ERR("proc_freq2pstate: Invalid calculated Pstate[%d] for freq[%dkHz] PGPE min Pstate[%d] freq[%dkHz]",
                          l_pstate, l_freq_khz, G_oppb.pstate_min, G_oppb.frequency_min_khz);
                l_pstate = G_oppb.pstate_min;
            }
        }
        else
        {
            // Freq is higher than or equal to the maximum frequency -- return Pmax
            l_pstate = PMAX;
        }
    }
    while(0);

    return (Pstate_t) l_pstate;
}

// Function Specification
//
// Name:  proc_pstate_kvm_setup
//
// Description: Copy Pstate table to OPAL shared memory this should only be called once
//    when going to active state
//
// End Function Specification
void proc_pstate_kvm_setup()
{
    TRAC_IMP("proc_pstate_kvm_setup: populate static OPAL data");

    // Initialize the opal table in SRAM (sets valid bit)
    populate_opal_static_data();

    // copy sram image into mainstore HOMER
    populate_opal_tbl_to_mem(OPAL_STATIC);
}

// Function Specification
//
// Name:  opal_table_bce_callback
//
// Description: Callback function for populate_opal_tbl_to_mem() BCE request
//              NO TRACING OR CALLING FUNCTIONS THAT TRACE ALLOWED
//
// End Function Specification
void opal_table_bce_callback( void )
{
    // If the BCE that just finished was for a dynamic table update notify host
    if(G_opal_table_update_state == OPAL_TABLE_UPDATE_DYNAMIC_COPY)
    {
        G_opal_table_update_state = OPAL_TABLE_UPDATE_NOTIFY_HOST;
    }
}

// Function Specification
//
// Name:  populate_opal_dynamic_data
//
// Description: populate the dynamic data entries in the OPAL table
//
// End Function Specification
void populate_opal_dynamic_data()
{
    memset(&G_opal_dynamic_table, 0, sizeof(G_opal_dynamic_table));

    // Dynamic OPAL runtime data
    G_opal_dynamic_table.dynamic.occ_state            = CURRENT_STATE();

    // Add GPUs presence if this is a system that has GPUs OCC would monitor
    if(G_gpu_config_done)
    {
        // set minor version to indicate OCC determined GPU presence, this is so
        // OPAL can tell the difference between no GPUs present and fw that didn't support
        G_opal_dynamic_table.dynamic.dynamic_minor_version = DYNAMIC_MINOR_V_GPU_PRESENCE;
        G_opal_dynamic_table.dynamic.gpus_present = G_first_proc_gpu_config;
    }

    //If safe state is requested then that overrides anything from amec
    if(isSafeStateRequested())
    {
        G_opal_dynamic_table.dynamic.proc_throt_status = OCC_RESET;
    }
    else
    {
        G_opal_dynamic_table.dynamic.proc_throt_status = G_amec_opal_proc_throt_reason;
    }

    G_opal_dynamic_table.dynamic.mem_throt_status     = G_amec_opal_mem_throt_reason;
    G_opal_dynamic_table.dynamic.quick_power_drop     = AMEC_INTF_GET_OVERSUBSCRIPTION();
    G_opal_dynamic_table.dynamic.power_shift_ratio    = G_sysConfigData.psr;
    G_opal_dynamic_table.dynamic.power_cap_type       = G_sysConfigData.pcap.source;
    G_opal_dynamic_table.dynamic.min_power_cap        = G_sysConfigData.pcap.hard_min_pcap;
    G_opal_dynamic_table.dynamic.max_power_cap        = G_sysConfigData.pcap.max_pcap;
    G_opal_dynamic_table.dynamic.current_power_cap    = g_amec->pcap.active_node_pcap;
    G_opal_dynamic_table.dynamic.soft_min_power_cap   = G_sysConfigData.pcap.soft_min_pcap;
}

// Function Specification
//
// Name:  populate_opal_static_data
//
// Description: populate the static configuration entries,
//              the generated pstates table, and maximum pstates
//              for all possible number of active cores.
//
// End Function Specification
void populate_opal_static_data()
{
    // clear all entries of the OPAL static table
    memset(&G_opal_static_table, 0, sizeof(G_opal_static_table));

    populate_opal_static_config_data();
    populate_opal_static_pstates_data();
}



// Function Specification
//
// Name:  populate_opal_static_config_data
//
// Description: populate the static configuration entries,
//
// End Function Specification
void populate_opal_static_config_data(void)
{
    // Static OPAL configuration data
    G_opal_static_table.config.valid    = 1;
    G_opal_static_table.config.version  = 0x90;
    G_opal_static_table.config.occ_role = G_occ_role;
    G_opal_static_table.config.pmin     = proc_freq2pstate(g_amec->sys.fmin);
    G_opal_static_table.config.pnominal = proc_freq2pstate(G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL]);
    G_opal_static_table.config.pturbo   = proc_freq2pstate(G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO]);
    G_opal_static_table.config.puturbo  = proc_freq2pstate(G_proc_fmax_mhz);
}

// Function Specification
//
// Name:  populate_opal_static_data
//
// Description: populate the generated pstates table, and maximum
//              pstates for all possible number of active cores.
//
// End Function Specification
void populate_opal_static_pstates_data(void)
{
    uint16_t i; // loop variable
    for (i=0; i <= G_oppb.pstate_min; i++)
    {
        G_opal_static_table.pstates[i].pstate   = i;  // pstate number
        G_opal_static_table.pstates[i].flag     = 0;  // flag is reserved for future use
        G_opal_static_table.pstates[i].freq_khz = proc_pstate2freq(i); // pstate's frequency
    }

    for (i=0; i<MAX_NUM_CORES; i++)
    {
        // TODO - RTC:170582 fix entries for WOF systems
        G_opal_static_table.max_pstate[i] = G_opal_static_table.config.puturbo;
    }
}

// Function Specification
//
// Name:  populate_opal_tbl_to_mem
//
// Description: use the upload copy engine to copy OPAL
//              OPAL table's static/dynamic entries to main memory.
//
// End Function Specification
void populate_opal_tbl_to_mem(opalDataType opal_data_type)
{
    int l_ssxrc = SSX_OK;
    uint32_t l_reasonCode = 0;
    uint32_t l_extReasonCode = 0;

    // Set up copy request for type of data being updated.  NOTE:  only DYNAMIC uses the callback
    if(opal_data_type == OPAL_STATIC)
    {
        // static is only updated when going active and should be blocking to make sure
        // this complets before reporting back that the state change finished
        l_ssxrc = bce_request_create(&G_opal_static_bce_req,                // block copy object
                                     &G_pba_bcue_queue,                     // sram to mainstore copy engine
                                     OPAL_STATIC_ADDRESS_HOMER,             // mainstore address
                                     (uint32_t) &G_opal_static_table,       // sram starting address
                                     (size_t) sizeof(G_opal_static_table),  // size of copy
                                     SSX_SECONDS(2),                        // timeout
                                     NULL,                                  // no call back
                                     NULL,                                  // call back arguments
                                     ASYNC_REQUEST_BLOCKING);
    }
    else if(opal_data_type == OPAL_DYNAMIC)
    {
        // dynamic data can be updated while active and should NOT be blocking
        l_ssxrc = bce_request_create(&G_opal_dynamic_bce_req,               // block copy object
                                     &G_pba_bcue_queue,                     // sram to mainstore copy engine
                                     OPAL_DYNAMIC_ADDRESS_HOMER,            // mainstore address
                                     (uint32_t) &G_opal_dynamic_table,      // sram starting address
                                     (size_t) sizeof(G_opal_dynamic_table), // size of copy
                                     SSX_WAIT_FOREVER,                      // no timeout
              (AsyncRequestCallback) opal_table_bce_callback,               // call back
                                     NULL,                                  // call back arguments
                                     ASYNC_CALLBACK_IMMEDIATE);
    }
    else
    {
        TRAC_ERR("populate_opal_tbl_to_mem: Invalid OPAL Table data type");
        return;
    }

    do
    {
        if(l_ssxrc != SSX_OK)
        {
            TRAC_ERR("populate_opal_tbl_to_mem: PBA request create failure rc=[%08X]",
                     -l_ssxrc);
            /*
             * @errortype
             * @moduleid    PROC_POP_OPAL_TBL_TO_MEM_MOD
             * @reasoncode  OPAL_TABLE_UPDATE_ERROR
             * @userdata1   RC for PBA block-copy engine
             * @userdata4   ERC_BCE_REQUEST_CREATE_FAILURE
             * @devdesc     Failed to create BCUE request
             */
            l_reasonCode = OPAL_TABLE_UPDATE_ERROR;
            l_extReasonCode = ERC_BCE_REQUEST_CREATE_FAILURE;
            break;
        }

        // Do actual copying
        if(opal_data_type == OPAL_STATIC)
        {
            l_ssxrc = bce_request_schedule(&G_opal_static_bce_req);
        }
        else
        {
            l_ssxrc = bce_request_schedule(&G_opal_dynamic_bce_req);
        }

        if(l_ssxrc != SSX_OK)
        {
            TRAC_ERR("populate_opal_tbl_to_mem: PBA request schedule failure rc=[%08X]",
                     -l_ssxrc);
            /*
             * @errortype
             * @moduleid    PROC_POP_OPAL_TBL_TO_MEM_MOD
             * @reasoncode  OPAL_TABLE_UPDATE_ERROR
             * @userdata1   RC for PBA block-copy engine
             * @userdata4   ERC_BCE_REQUEST_SCHEDULE_FAILURE
             * @devdesc     Failed to copy OPAL data by using BCUE
             */
            l_reasonCode = OPAL_TABLE_UPDATE_ERROR;
            l_extReasonCode = ERC_BCE_REQUEST_SCHEDULE_FAILURE;
            break;
        }
    } while(0);

    if ( l_ssxrc != SSX_OK )
    {
        // set back to idle since callback won't happen
        if( (opal_data_type == OPAL_DYNAMIC) &&
            (G_opal_table_update_state == OPAL_TABLE_UPDATE_DYNAMIC_COPY) )
        {
            G_opal_table_update_state = OPAL_TABLE_UPDATE_IDLE;
        }

        // data in main mem only matters for OPAL so only log error if OPAL
        if(G_sysConfigData.system_type.kvm)
        {
            errlHndl_t l_errl = createErrl(PROC_POP_OPAL_TBL_TO_MEM_MOD,  //modId
                                           l_reasonCode,                  //reasoncode
                                           l_extReasonCode,               //Extended reason code
                                           ERRL_SEV_UNRECOVERABLE,        //Severity
                                           NULL,                          //Trace Buf
                                           0,                             //Trace Size
                                           -l_ssxrc,                      //userdata1
                                           0);                            //userdata2

            // Callout firmware
            addCalloutToErrl(l_errl,
                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                             ERRL_COMPONENT_ID_FIRMWARE,
                             ERRL_CALLOUT_PRIORITY_HIGH);

            // Callout processor
            addCalloutToErrl(l_errl,
                             ERRL_CALLOUT_TYPE_HUID,
                             G_sysConfigData.proc_huid,
                             ERRL_CALLOUT_PRIORITY_MED);

            commitErrl(&l_errl);
        }
    }
    else if(opal_data_type == OPAL_DYNAMIC)
    {
        G_opal_dynamic_bce_req_scheduled = true;
    }
}


// Function Specification
//
// Name: check_for_opal_updates
//
// Description: Checks if any of the dynamic fields in the opal shared memory
//              needs to be updated and updates if necessary.
//
// End Function Specification
void check_for_opal_updates(void)
{
    bool           dynamic_data_change = false;
    bool           l_log_crit_error = false;
    static uint8_t L_num_bce_checks = 0;
    static bool    L_first_throttle_trace = true;
    static bool    L_first_unthrottle_trace = true;
    static bool    L_first_mem_trace = true;

    // check if BCE for previous change finished and now need to notify host
    if(G_opal_table_update_state == OPAL_TABLE_UPDATE_NOTIFY_HOST)
    {
         // regardless of if we notify host we are done with this change
         G_opal_table_update_state = OPAL_TABLE_UPDATE_IDLE;

         if(G_sysConfigData.system_type.kvm)  // only notify if OPAL
         {
              notify_host(INTR_REASON_OPAL_SHARED_MEM_CHANGE);
         }
    }

    // check if previous change is not complete
    else if( (G_opal_table_update_state == OPAL_TABLE_UPDATE_DYNAMIC_COPY) ||
             (G_opal_dynamic_bce_req_scheduled && !(async_request_is_idle(&G_opal_dynamic_bce_req.request))) )
    {
         if(L_num_bce_checks <= OPAL_DYNAMIC_UPDATE_BCE_RETRIES)
         {
             L_num_bce_checks++;
         }
         else
         {
             TRAC_ERR("check_for_opal_updates: BCE not idle %u times, done retrying", L_num_bce_checks);
             l_log_crit_error = true;
         }
    }

    else if(G_opal_table_update_state == OPAL_TABLE_UPDATE_BCE_FAIL)
    {
         // BCE failed re-populate the data and retry the BCE if under retry count
         if(L_num_bce_checks <= OPAL_DYNAMIC_UPDATE_BCE_RETRIES)
         {
             dynamic_data_change = true;
         }
         else
         {
             TRAC_ERR("check_for_opal_updates: BCE failed %u times, done retrying", L_num_bce_checks);
             l_log_crit_error = true;
         }
    }

    else  // check if any of the data changed
    {
        // check if processor throttle status changed if going to safe state check for reset status
        // else just check for any change since not in safe state
        if( isSafeStateRequested() )
        {
            if(G_opal_dynamic_table.dynamic.proc_throt_status != OCC_RESET)
            {
                dynamic_data_change = true;
                TRAC_INFO("check_for_opal_updates: safe state processor throttle status change - 0x%02X->0x%02X",
                           G_opal_dynamic_table.dynamic.proc_throt_status, G_amec_opal_proc_throt_reason);
            }
        }
        else if(G_opal_dynamic_table.dynamic.proc_throt_status != G_amec_opal_proc_throt_reason)
        {
            dynamic_data_change = true;

            // Only want to trace the first time we throttle, and the first
            // time we unthrottle. If ALLOW_OPAL_TRACE is set, trace every time
            bool l_trace = false;
            if( (G_allow_trace_flags & ALLOW_OPAL_TRACE) ||
                (L_first_throttle_trace) )
            {
                l_trace = true;
                L_first_throttle_trace = false;
            }
            else if( (G_amec_opal_proc_throt_reason == 0) &&
                     (L_first_unthrottle_trace) )
            {
                    l_trace = true;
                    L_first_unthrottle_trace = false;
            }

            if(l_trace)
            {
                TRAC_INFO("check_for_opal_updates: "
                        "processor throttle status change - 0x%02X->0x%02X",
                        G_opal_dynamic_table.dynamic.proc_throt_status,
                        G_amec_opal_proc_throt_reason);
            }
        }

        // check if memory throttle status or Quick Power Drop changed
        if( (G_opal_dynamic_table.dynamic.mem_throt_status != G_amec_opal_mem_throt_reason) ||
            (G_opal_dynamic_table.dynamic.quick_power_drop != AMEC_INTF_GET_OVERSUBSCRIPTION()) )
        {
            dynamic_data_change = true;

            if( (G_allow_trace_flags & ALLOW_OPAL_TRACE  ) ||
                (L_first_mem_trace) ||
                (G_opal_dynamic_table.dynamic.quick_power_drop !=
                 AMEC_INTF_GET_OVERSUBSCRIPTION()) )
            {
               TRAC_INFO("check_for_opal_updates:"
                       " memory throttle status - 0x%02X->0x%02X"
                       " QPD - 0x%02X->0x%02X",
                       G_opal_dynamic_table.dynamic.mem_throt_status,
                       G_amec_opal_mem_throt_reason,
                       G_opal_dynamic_table.dynamic.quick_power_drop,
                       AMEC_INTF_GET_OVERSUBSCRIPTION());
                L_first_mem_trace = false;
            }
        }

        // check for OCC state change
        if(G_opal_dynamic_table.dynamic.occ_state != CURRENT_STATE())
        {
            dynamic_data_change = true;
            TRAC_INFO("check_for_opal_updates: OCC state change 0x%02X->0x%02X",
                       G_opal_dynamic_table.dynamic.occ_state, CURRENT_STATE());
        }

        // check for change in power cap data must look at slave copy
        // do NOT use G_master_pcap_data as that is not populated on slaves
        if( (G_opal_dynamic_table.dynamic.min_power_cap      != G_sysConfigData.pcap.hard_min_pcap) ||
            (G_opal_dynamic_table.dynamic.max_power_cap      != G_sysConfigData.pcap.max_pcap)      ||
            (G_opal_dynamic_table.dynamic.soft_min_power_cap != G_sysConfigData.pcap.soft_min_pcap) ||
            (G_opal_dynamic_table.dynamic.power_shift_ratio  != G_sysConfigData.psr)                ||
            (G_opal_dynamic_table.dynamic.power_cap_type     != G_sysConfigData.pcap.source)        ||
            (G_opal_dynamic_table.dynamic.current_power_cap  != g_amec->pcap.active_node_pcap) )
        {
            dynamic_data_change = true;
            TRAC_INFO("check_for_opal_updates: soft min Pcap = 0x%04X->0x%04X hard min Pcap = 0x%04X->0x%04X",
                       G_opal_dynamic_table.dynamic.soft_min_power_cap, G_sysConfigData.pcap.soft_min_pcap,
                       G_opal_dynamic_table.dynamic.min_power_cap, G_sysConfigData.pcap.hard_min_pcap);

            TRAC_INFO("check_for_opal_updates: max Pcap = 0x%04X->0x%04X active Pcap = 0x%04X->0x%04X",
                       G_opal_dynamic_table.dynamic.max_power_cap, G_sysConfigData.pcap.max_pcap,
                       G_opal_dynamic_table.dynamic.current_power_cap, g_amec->pcap.active_node_pcap);

            TRAC_INFO("check_for_opal_updates: Pcap PSR = %u->%u Pcap source = %u->%u",
                       G_opal_dynamic_table.dynamic.power_shift_ratio, G_sysConfigData.psr,
                       G_opal_dynamic_table.dynamic.power_cap_type, G_sysConfigData.pcap.source);
        }

        // check if GPU presence was determined
        if( (G_opal_dynamic_table.dynamic.dynamic_minor_version != DYNAMIC_MINOR_V_GPU_PRESENCE) &&
            (G_gpu_config_done) )
        {
            dynamic_data_change = true;
            TRAC_INFO("check_for_opal_updates: GPU presence determined = 0x%04X",
                       G_first_proc_gpu_config);
        }
    }  // else check for changes

    // If there was a change copy to main memory and notify host when BCE finishes
    if(dynamic_data_change)
    {
        G_opal_table_update_state = OPAL_TABLE_UPDATE_DYNAMIC_COPY;
        update_dynamic_opal_data();

        // if the BCE schedule fails the state will go back to IDLE, retry next time called
        if(G_opal_table_update_state == OPAL_TABLE_UPDATE_IDLE)
        {
            G_opal_table_update_state = OPAL_TABLE_UPDATE_BCE_FAIL;
            L_num_bce_checks++;
        }
        else
        {
            L_num_bce_checks = 0;
        }
    }
    else if(l_log_crit_error)
    {
         // stop trying to update dynamic data, this only really matters on OPAL systems so
         // only log the error if OPAL
         G_opal_table_update_state = OPAL_TABLE_UPDATE_CRITICAL_ERROR;

         if(G_sysConfigData.system_type.kvm)
         {
             // Create and commit error
             /* @
              * @errortype
              * @moduleid    PROC_CHECK_FOR_OPAL_UPDATES_MOD
              * @reasoncode  OPAL_TABLE_UPDATE_ERROR
              * @userdata1   0
              * @userdata2   0
              * @userdata4   ERC_GENERIC_TIMEOUT
              * @devdesc     BCE request failure to update dynamic opal table
              */
             errlHndl_t l_errl = createErrl(PROC_CHECK_FOR_OPAL_UPDATES_MOD, // Module ID
                                            OPAL_TABLE_UPDATE_ERROR,         // Reason code
                                            ERC_GENERIC_TIMEOUT,             // Extended reason code
                                            ERRL_SEV_UNRECOVERABLE,          // Severity
                                            NULL,                            // Trace Buffers
                                            DEFAULT_TRACE_SIZE,              // Trace Size
                                            0,                               // Userdata1
                                            0);                              // Userdata2

            // Callout firmware
            addCalloutToErrl(l_errl,
                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                             ERRL_COMPONENT_ID_FIRMWARE,
                             ERRL_CALLOUT_PRIORITY_HIGH);

            // Callout processor
            addCalloutToErrl(l_errl,
                             ERRL_CALLOUT_TYPE_HUID,
                             G_sysConfigData.proc_huid,
                             ERRL_CALLOUT_PRIORITY_MED);

             commitErrl(&l_errl);
         }
    }

}


// Function Specification
//
// Name: update_dynamic_opal_data
//
// Description: update dynamic opal data in SRAM and
//              copy it over to OPAL space in main memory.
//
// End Function Specification
void update_dynamic_opal_data (void)
{

    // Initialize the dynamic opal table in SRAM
    populate_opal_dynamic_data();

    // copy sram image into mainstore HOMER
    populate_opal_tbl_to_mem(OPAL_DYNAMIC);
}
