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

// Used for OPAL
DMA_BUFFER( opal_table_t G_opal_table ) = {{0}};

//KVM throttle reason coming from the frequency voting box.
extern uint8_t G_amec_kvm_throt_reason;

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
        populate_pstate_to_opal_tbl();

        // copy sram image into mainstore HOMER
        populate_opal_tbl_to_mem();
        TRAC_IMP("proc_pstate_kvm_setup: RUNNING IN KVM MODE");

    }while(0);
}

// Function Specification
//
// Name:  populate_pstate_to_opal_tbl
//
// Description:
//
// End Function Specification
void populate_pstate_to_opal_tbl()
{
    uint8_t i = 0;

    memset(&G_opal_table, 0, sizeof(opal_table_t));

    G_opal_table.config.valid = 1;              // default 0x01
    G_opal_table.config.version = 1;            // default 0x01
    G_opal_table.config.throttle = NO_THROTTLE; // default 0x00
    G_opal_table.config.pmin = G_proc_pmin - 1;
    G_opal_table.config.pnominal = proc_freq2pstate(G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL]);
    G_opal_table.config.pmax = G_proc_pmax;
    const uint16_t l_entries = G_opal_table.config.pmin - G_opal_table.config.pmax + 1;
//    const uint8_t l_idx = l_entries-1;

    for (i = 0; i < l_entries; i++)
    {
        G_opal_table.data[i].pstate = (Pstate) G_proc_pmax + i;
        G_opal_table.data[i].flag = 0;          // default 0x00

// @TODO: what are parameters used to calculte evids in P9?
// Use it to calculate evids and complete OPAL Pstate table:

/*        if (i < l_gpst_ptr->entries)
        {
            G_opal_table.data[i].evid_vdd = l_gpst_ptr->pstate[i].fields.evid_vdd;
            G_opal_table.data[i].evid_vcs = l_gpst_ptr->pstate[i].fields.evid_vcs;
        }
        else
        {
            // leave the VDD & VCS Vids the same as the "Pstate Table Pmin"
            G_opal_table.data[i].evid_vdd = l_gpst_ptr->pstate[l_idx].fields.evid_vdd;
            G_opal_table.data[i].evid_vcs = l_gpst_ptr->pstate[l_idx].fields.evid_vcs;
        }
*/
        // calculate frequency for pstate
        G_opal_table.data[i].freq_khz = proc_pstate2freq(G_opal_table.data[i].pstate);
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

    // @TODO: Schedule a PGPE IPC to enable pstates. This will also tell
    // PGPE how to set PMCR mode register (OCC control pstates or OPAL).

    // the call back IPC function that gets executed after this IPC finishes
    // sets the G_proc_pstate_enabled flag to true, so that the OCC is then
    // able to switch to the active state.

    // Set up Key Globals for use by proc_freq2pstate functions
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
        l_ssxrc = bce_request_create(&pba_copy,                          // block copy object
                                     &G_pba_bcue_queue,                  // sram to mainstore copy engine
                                     OPAL_ADDRESS_HOMER,             // mainstore address
                                     (uint32_t) &G_opal_table,       // sram starting address
                                     (size_t) sizeof(G_opal_table),  // size of copy
                                     SSX_WAIT_FOREVER,                   // no timeout
                                     NULL,                               // call back
                                     NULL,                               // call back arguments
                                     ASYNC_REQUEST_BLOCKING              // callback mask
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
             * @devdesc     SSX BCE related failure
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
             * @devdesc     Failed to copy data by using DMA
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
// Name: proc_check_for_opal_updates
//
// Description: Checks if the opal table needs an update
//              and updates if necessary.
//
// End Function Specification
void proc_check_for_opal_updates()
{
    uint8_t l_latest_throttle_reason;

    //If safe state is requested then that overrides anything from amec
    if(isSafeStateRequested())
    {
        l_latest_throttle_reason = OCC_RESET;
    }
    else
    {
        l_latest_throttle_reason = G_amec_kvm_throt_reason;
    }

    //If the throttle reason changed, update it in the HOMER
    if(G_opal_table.config.throttle != l_latest_throttle_reason)
    {
        TRAC_INFO("proc_check_for_opal_updates: throttle reason changed to %d", l_latest_throttle_reason);
        G_opal_table.config.throttle = l_latest_throttle_reason;
        G_opal_table.config.version = 1; // default 0x01
        G_opal_table.config.valid = 1; //default 0x01
        populate_opal_tbl_to_mem();
    }
}
