/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_oversub.c $                             */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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
#include "ssx.h"
#include <occ_common.h>
#include <trac.h>
#include <amec_sys.h>
#include <proc_pstate.h>
#include <dcom.h>
#include <occ_sys_config.h>
#include <amec_service_codes.h>
#include <pgpe_interface.h>

//*************************************************************************/
// Externs
//*************************************************************************/

//*************************************************************************/
// Macros
//*************************************************************************/

//*************************************************************************/
// Defines/Enums
//*************************************************************************/
#define OVERSUB_REASON_DELAY_4MS     16  // 4ms (unit is 250us)
#define OVERSUB_REASON_COUNT_TIMEOUT 2
#define OVERSUB_REASON_DETERMINED    1

//*************************************************************************/
// Structures
//*************************************************************************/

//*************************************************************************/
// Globals
//*************************************************************************/

//*************************************************************************/
// Function Prototypes
//*************************************************************************/

//*************************************************************************/
// Functions
//*************************************************************************/

// Function Specification
//
// Name: amec_oversub_isr
//
// Description: Oversubscription ISR
//
// Task Flags:
//
// Note: This function is only ever called from Critical interrupt context
//
// End Function Specification
void amec_oversub_isr(void)
{
    static uint32_t L_isr_count = 0;
    static uint8_t L_polarity = SSX_IRQ_POLARITY_ACTIVE_HIGH; // Default is high
    uint8_t l_cur_polarity = L_polarity;

    L_isr_count++;

    // SSX_IRQ_POLARITY_ACTIVE_HIGH means over-subscription is active
    if(L_polarity == SSX_IRQ_POLARITY_ACTIVE_HIGH)
    {
        // oversub only switches pcap in amec_pcap_calc
        // throttling only done if actually needed due to reaching power cap
        g_amec->oversub_status.oversubReasonLatchCount = OVERSUB_REASON_DELAY_4MS;

        // Set oversubPinLive and oversubActiveTime
        g_amec->oversub_status.oversubPinLive = 1;
        g_amec->oversub_status.oversubActiveTime = ssx_timebase_get();

        // Setup the IRQ
        ssx_irq_setup(OCCHW_IRQ_EXTERNAL_TRAP,
                      SSX_IRQ_POLARITY_ACTIVE_LOW,
                      SSX_IRQ_TRIGGER_LEVEL_SENSITIVE);
        L_polarity = SSX_IRQ_POLARITY_ACTIVE_LOW;

    }
    else  // over-subscription is inactive
    {
        // Clear oversubPinLive
        g_amec->oversub_status.oversubPinLive = 0;

        // Set oversubInActiveTime
        g_amec->oversub_status.oversubInactiveTime = ssx_timebase_get();

        // Setup the IRQ
        ssx_irq_setup(OCCHW_IRQ_EXTERNAL_TRAP,
                      SSX_IRQ_POLARITY_ACTIVE_HIGH,
                      SSX_IRQ_TRIGGER_LEVEL_SENSITIVE);

        L_polarity = SSX_IRQ_POLARITY_ACTIVE_HIGH;
    }

    // Trace Oversub Event, Polarity and ISR count
    TRAC_INFO("Oversub IRQ - Polarity (high active):%d, oversubPinLive: %d, count: %d)", l_cur_polarity, g_amec->oversub_status.oversubPinLive, L_isr_count);

}


// Function Specification
//
// Name: amec_oversub_check
//
// Description: Oversubscription check called in amec_slv_common_tasks_pre()
//
// Task Flags:
//
// End Function Specification
void amec_oversub_check(void)
{
    static BOOLEAN              L_prev_ovs_state = FALSE;  // oversub happened

    // oversubscription condition happened?
    if ( AMEC_INTF_GET_OVERSUBSCRIPTION() == TRUE )
    {
        if ( L_prev_ovs_state != TRUE)
        {
            L_prev_ovs_state = TRUE;

            TRAC_ERR("Oversubscription condition happened");
            /* @
             * @errortype
             * @moduleid    AMEC_SLAVE_CHECK_PERFORMANCE
             * @reasoncode  OVERSUB_ALERT
             * @userdata1   Previous OVS State
             * @userdata4   ERC_AMEC_SLAVE_OVS_STATE
             * @devdesc     Oversubscription condition happened
             */
            errlHndl_t l_errl = createErrl(AMEC_SLAVE_CHECK_PERFORMANCE,//modId
                                          OVERSUB_ALERT,                //reasoncode
                                          ERC_AMEC_SLAVE_OVS_STATE,     //Extended reason code
                                          ERRL_SEV_INFORMATIONAL,       //Severity
                                          NULL,                         //Trace Buf
                                          DEFAULT_TRACE_SIZE,           //Trace Size
                                          L_prev_ovs_state,             //userdata1
                                          0);                           //userdata2

            // set the mfg action flag (allows callout to be added to info error)
            setErrlActions(l_errl, ERRL_ACTIONS_MANUFACTURING_ERROR);

            // add the oversubscription symbolic callout
            addCalloutToErrl(l_errl,
                             ERRL_CALLOUT_TYPE_COMPONENT_ID,
                             ERRL_COMPONENT_ID_OVERSUBSCRIPTION,
                             ERRL_CALLOUT_PRIORITY_HIGH);

            // Commit Error
            commitErrl(&l_errl);
        }
    }
    else
    {
        L_prev_ovs_state = FALSE;
    }

    // Figure out the over-subscription reason
    if(g_amec->oversub_status.oversubReasonLatchCount > 1)
    {
        // TODO -- RTC 155565 -- Try to figure out why we throttled based on APSS GPIO pins
        if( g_amec->oversub_status.oversubReasonLatchCount == OVERSUB_REASON_COUNT_TIMEOUT)
        {
            g_amec->oversub_status.oversubReason = INDETERMINATE;
            g_amec->oversub_status.oversubReasonLatchCount = OVERSUB_REASON_DETERMINED;

            TRAC_INFO("Oversub (oversubReason: %d)", g_amec->oversub_status.oversubReason );
        }
        else
        {
            // If we can figure out why oversub happened, set oversubReason to valide enum value
            // then set oversubReasonLatchCount = 1

            // If we can't figure it out, decrease oversubReasonLatchCount and we will try again in 250us
            g_amec->oversub_status.oversubReasonLatchCount--; // The unit of oversubReasonLatchCount is 250us
        }
    }
}

/*----------------------------------------------------------------------------*/
/* End                                                                        */
/*----------------------------------------------------------------------------*/
