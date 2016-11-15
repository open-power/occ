/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/proc/proc_pstate.c $                              */
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
#include "scom.h"
#include "homer.h"
#include <amec_freq.h>
#include <common.h>
#include <amec_oversub.h>
#include <amec_sys.h>

//OPAL processor and memory throttle reason coming from the frequency voting boxes.
extern opal_proc_voting_reason_t G_amec_opal_proc_throt_reason;
extern opal_mem_voting_reason_t  G_amec_opal_mem_throt_reason;

// Holds Fmax for ease of proc_freq2pstate calculation
uint32_t G_proc_fmax = 0;

// Holds Fmin for ease of proc_freq2pstate calculation
uint32_t G_proc_fmin = 0;

// Holds frequency steps between consequtive Pstates
uint32_t G_khz_per_pstate = 0;

// Holds Pmax for ease of proc_freq2pstate calculation
uint8_t   G_proc_pmax = 0;

// Holds Pmin for ease of proc_freq2pstate calculation
uint8_t   G_proc_pmin = 0;

// Holds a flag indicating whether the pstates have been enabled.
// initialized to FALSE, turns TRUE only after the PGPE IPC that
// enable pstates completes successfully.
bool G_proc_pstate_enabled = FALSE;

// OPAL Dynamic data, updated whenever any OCC G_opal_table.dynamic parameter change
DMA_BUFFER( opal_dynamic_table_t G_opal_dynamic_table ) = {{0}};

// OPAL Static data, updated once at transition to active state
DMA_BUFFER( opal_static_table_t  G_opal_static_table )  = {{0}};


// Function Specification
//
// Name:  proc_is_hwpstate_enabled
//
// Description:  Checks OCC to see if Pstate HW Mode is enabled.
//
// End Function Specification
bool proc_is_hwpstate_enabled(void)
{
    return ( G_proc_pstate_enabled ? TRUE : FALSE);
}

// Function Specification
//
// Name:  proc_pstate2freq
//
// Description:  Convert Pstate to Frequency in kHz
//
// End Function Specification
uint32_t proc_pstate2freq(Pstate i_pstate)
{
    // If passed in Pstate is lower than Pmin, just use Pmin
    if(i_pstate < G_proc_pmin)
    {
        i_pstate = G_proc_pmin;
    }

    // If passed in Pstate is greater than Pmax, just use Pmax
    else if (i_pstate > G_proc_pmax)
    {
        i_pstate = G_proc_pmax;
    }

    // Calculate Frequency in kHz based on Pstate
    return (G_proc_fmin + (G_proc_pmin - i_pstate) * G_khz_per_pstate);
}

// Function Specification
//
// Name:  proc_freq2pstate
//
// Description:  Convert Frequency to Nearest Pstate
//
// End Function Specification
Pstate proc_freq2pstate(uint32_t i_freq_mhz)
{
    int8_t   l_pstate = PSTATE_MIN;
    int8_t   l_temp_pstate = 0;
    int32_t  l_temp_freq = 0;
    uint32_t l_freq_khz = 0;

    do
    {
        // Freq Units need to be in kHz, not Mhz for the following calculations
        l_freq_khz = i_freq_mhz * 1000;

        // Make sure that we don't ever get a frequency below the min Freq from
        // def file
        if(i_freq_mhz < G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY])
        {
            l_freq_khz =  G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY] * 1000;
        }

        if(l_freq_khz < G_proc_fmax)
        {
            // First, calculate the delta between passed in freq, and Pmin
            l_temp_freq = l_freq_khz - G_proc_fmin;

            // Check if the passed in frequency is lower than Minimum Frequency
            if(l_freq_khz <= G_proc_fmin)
            {
                // We need to substract a full step (minus 1) to make sure we
                // are keeping things safe
                l_temp_freq -= (G_khz_per_pstate - 1);
            }

            // Next, calculate how many Pstate steps there are in that delta
            l_temp_pstate = l_temp_freq / (int32_t) G_khz_per_pstate;

            // Lastly, calculate Pstate, by adding delta Pstate steps to Pmin
            l_pstate = G_proc_pmin - l_temp_pstate;
        }
        else
        {
            // Freq is higher than maximum frequency -- return Pmax
            l_pstate = G_proc_pmax;
        }
    }
    while(0);

    return (Pstate) l_pstate;
}

// Function Specification
//
// Name:  proc_pstate_kvm_setup
//
// Description: Get everything set up for KVM mode
//
// End Function Specification
void proc_pstate_kvm_setup()
{
    do
    {
        //only run this in KVM mode
        if(!G_sysConfigData.system_type.kvm)
        {
            TRAC_ERR("proc_pstate_kvm_setup: called in a non OPAL system");
            break;
        }

        // Initialize the opal table in SRAM (sets valid bit)
        populate_static_opal_data();

        // copy sram image into mainstore HOMER
        populate_opal_tbl_to_mem();
        TRAC_IMP("proc_pstate_kvm_setup: RUNNING IN KVM MODE");

    }while(0);
}

// Function Specification
//
// Name:  populate_dynamic_opal_data
//
// Description: populate the dynamic data entries in the OPAL table
//
// End Function Specification
void populate_dynamic_opal_data()
{
    memset(&G_opal_dynamic_table, 0, sizeof(G_opal_dynamic_table));

    // Dynamic OPAL runtime data
    G_opal_dynamic_table.dynamic.occ_state            = CURRENT_STATE();

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
    G_opal_dynamic_table.dynamic.power_shift_ratio    = 50; // @TODO: Power Shift Ratio is not Implemented yet RTC:133825
    G_opal_dynamic_table.dynamic.power_cap_type       = G_master_pcap_data.source;
    G_opal_dynamic_table.dynamic.min_power_cap        = G_master_pcap_data.soft_min_pcap;
    G_opal_dynamic_table.dynamic.max_power_cap        = G_master_pcap_data.max_pcap;
    G_opal_dynamic_table.dynamic.current_power_cap    = G_master_pcap_data.current_pcap;
}

// Function Specification
//
// Name:  populate_static_opal_data
//
// Description: populate the static configuration entries,
//              the generated pstates table, and maximum pstates
//              for all possible number of active cores.
//
// End Function Specification
void populate_static_opal_data()
{
    memset(&G_opal_dynamic_table, 0, sizeof(G_opal_dynamic_table));


    // Static OPAL configuration data
    G_opal_static_table.config.valid    = 1;
    G_opal_static_table.config.version  = 0x90;
    G_opal_static_table.config.occ_role = G_occ_role;
    G_opal_static_table.config.pmin     = pmin_rail();
    G_opal_static_table.config.pnominal = proc_freq2pstate(G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL]);
    G_opal_static_table.config.pturbo   = proc_freq2pstate(G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO]);
    G_opal_static_table.config.puturbo  = proc_freq2pstate(G_sysConfigData.sys_mode_freq.table[OCC_MODE_UTURBO]);

    // TODO - RTC:130201 generate pstates table & Max pstates for var #cores
}

// Function Specification
//
// Name:  populate_opal_tbl_to_mem
//
// Description:
//
// End Function Specification
void populate_opal_tbl_to_mem()
{
    int l_ssxrc = SSX_OK;
    uint32_t l_reasonCode = 0;
    uint32_t l_extReasonCode = 0;

    do
    {
        BceRequest pba_copy;
        // Set up copy request
        l_ssxrc = bce_request_create(
                       &pba_copy,                             // block copy object
                       &G_pba_bcue_queue,                     // sram to mainstore copy engine
                       OPAL_DYNAMIC_ADDRESS_HOMER,            // mainstore address
                       (uint32_t) &G_opal_dynamic_table,      // sram starting address
                       (size_t) sizeof(G_opal_dynamic_table), // size of copy
                       SSX_WAIT_FOREVER,                      // no timeout
                       NULL,                                  // call back
                       NULL,                                  // call back arguments
                       ASYNC_REQUEST_BLOCKING                 // callback mask
                       );

        if(l_ssxrc != SSX_OK)
        {
            TRAC_ERR("populate_opal_tbl_to_mem: PBA request create failure rc=[%08X]", -l_ssxrc);
            /*
             * @errortype
             * @moduleid    MAIN_STATE_TRANSITION_MID
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   RC for PBA block-copy engine
             * @userdata4   ERC_BCE_REQUEST_CREATE_FAILURE
             * @devdesc     Failed to create BCUE request
             */
            l_reasonCode = SSX_GENERIC_FAILURE;
            l_extReasonCode = ERC_BCE_REQUEST_CREATE_FAILURE;
            break;
        }

        // Do actual copying
        l_ssxrc = bce_request_schedule(&pba_copy);

        if(l_ssxrc != SSX_OK)
        {
            TRAC_ERR("populate_opal_tbl_to_mem: PBA request schedule failure rc=[%08X]", -l_ssxrc);
            /*
             * @errortype
             * @moduleid    MAIN_STATE_TRANSITION_MID
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   RC for PBA block-copy engine
             * @userdata4   ERC_BCE_REQUEST_SCHEDULE_FAILURE
             * @devdesc     Failed to copy OPAL data by using BCUE
             */
            l_reasonCode = SSX_GENERIC_FAILURE;
            l_extReasonCode = ERC_BCE_REQUEST_SCHEDULE_FAILURE;
            break;
        }
    } while(0);

    if ( l_ssxrc != SSX_OK )
    {
        errlHndl_t l_errl = createErrl(MAIN_STATE_TRANSITION_MID,    //modId
                                       l_reasonCode,                 //reasoncode
                                       l_extReasonCode,              //Extended reason code
                                       ERRL_SEV_UNRECOVERABLE,       //Severity
                                       NULL,                         //Trace Buf
                                       0,                            //Trace Size
                                       -l_ssxrc,                     //userdata1
                                       0);                           //userdata2

        // Callout firmware
        addCalloutToErrl(l_errl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        commitErrl(&l_errl);
    }
}

// Function Specification
//
// Name:   proc_pstate_initialize
//
// Description:  initialize Global variables required for
//               the proc_freq2pstate function.
//
// End Function Specification
void proc_pstate_initialize(void)
{
    // RTC:130201
    // @TODO: Schedule a PGPE IPC to enable pstates. This will also tell
    // PGPE how to set PMCR mode register (OCC control pstates or OPAL).

    // the call back IPC function that gets executed after this IPC finishes
    // sets the G_proc_pstate_enabled flag to true, so that the OCC is then
    // able to switch to the active state.

    // @TODO: FIXME - TEMP Hack to enable Active State, until PGPE is ready
    G_proc_pstate_enabled = 1;


    // Set up Key Globals for use by proc_freq2pstate functions
    // @TODO - FIXME ==> move to initialization code, reading from parameter block
    G_proc_fmax = 4322500;
    G_proc_fmin = 2028250;
    G_khz_per_pstate = 33250;
    G_proc_pmax = 0;
    G_proc_pmin = G_proc_pmax + ((G_proc_fmax - G_proc_fmin)/G_khz_per_pstate);

    // Set globals used by amec for pcap calculation
    // could have used G_khz_per_pstate in PCAP calculations
    // instead, but using a separate varaible speeds up PCAP related
    // calculations significantly, by eliminating division operations.
    G_mhz_per_pstate = G_khz_per_pstate/1000;

    TRAC_INFO("proc_pstate_initialize: Pstate Key globals initialized to default values");

}

// Function Specification
//
// Name: check_for_opal_updates
//
// Description: Checks if the opal table needs an update
//              and updates if necessary.
//
// End Function Specification
void check_for_opal_updates(void)
{
    bool throttle_change = false;

    // if throttle status change, update OPAL and notify host
    if( (G_opal_dynamic_table.dynamic.proc_throt_status != G_amec_opal_proc_throt_reason) ||         // PROC throttle states changed
        ((G_opal_dynamic_table.dynamic.proc_throt_status != OCC_RESET) && isSafeStateRequested()) ||  // OCC reset requested & OPAL not updated
        (G_opal_dynamic_table.dynamic.mem_throt_status  != G_amec_opal_mem_throt_reason) )           // Mem throttle status changed
    {
        throttle_change = true;
        update_dynamic_opal_data();
    }

    // else, if a dynamic OPAL parameter changed, update OPAL dynamic table and OPAL data in memory
    else if(G_opal_dynamic_table.dynamic.occ_state         != CURRENT_STATE()                  ||
            G_opal_dynamic_table.dynamic.quick_power_drop  != AMEC_INTF_GET_OVERSUBSCRIPTION() ||
            G_opal_dynamic_table.dynamic.power_shift_ratio != 50  || // @TODO: Power Shift Ratio is not Implemented yet RTC:133825
            G_opal_dynamic_table.dynamic.power_cap_type    != G_master_pcap_data.source        ||
            G_opal_dynamic_table.dynamic.min_power_cap     != G_master_pcap_data.soft_min_pcap ||
            G_opal_dynamic_table.dynamic.max_power_cap     != G_master_pcap_data.max_pcap      ||
            G_opal_dynamic_table.dynamic.current_power_cap != G_master_pcap_data.current_pcap  )
    {
        update_dynamic_opal_data();
    }

    // A throttle status change, notify host after copying dynamic OPAL data
    if(throttle_change)
    {
        notify_host(INTR_REASON_OPAL_SHARED_MEM_CHANGE);
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
    // Initialize the opal table in SRAM (sets valid bit)
    populate_dynamic_opal_data();

    // copy sram image into mainstore HOMER
    populate_opal_tbl_to_mem();
    TRAC_IMP("update_dynamic_opal_data: update dynamic OPAL data");


}


// Function Specification
//
// Name: pmin_rail
//
// Description: returns the smaller pstate (for narrower set of pstates) of:
//                    1) the min pstate from the PGPE pstate table
//                and 2) the TMGT configured min pstate
//
// End Function Specification
uint8_t pmin_rail(void)
{
    uint8_t  configMinPstate = proc_freq2pstate(G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY]);

    if(configMinPstate < G_proc_pmin)
    {
        return configMinPstate;
    }
    else
    {
        return G_proc_pmin;
    }
}
