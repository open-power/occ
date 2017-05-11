/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/common.c $                                            */
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

// Description: Common 405 functions

#include <common.h>
#include <trac.h>
#include <errl.h>
#include <occ_service_codes.h>

uint8_t G_host_notifications_pending = 0;


// Function Specification
//
// Name: task_misc_405_checks
//
// Description: Perform periodic checks on the 405 including:
//              - host notifications
//              - checkstops
//
// End Function Specification
void task_misc_405_checks(task_t *i_self)
{
    if (G_host_notifications_pending != 0)
    {
        notify_host(G_host_notifications_pending);
    }

    // Check for checkstops
    ocb_oisr0_t  l_oisr0_status;       // OCC Interrupt Source 0 Register

    static bool L_checkstop_traced    = false;
    uint8_t     l_reason_code         = 0;

    do
    {
        // This check is disabled once a checkstop or frozen GPE is detected
        if(L_checkstop_traced)
        {
            break;
        }
        // Look for a frozen GPE, a sign that the chip has stopped working or
        // halted.  This check also looks for an interrupt status flag that
        // indicates if the system has check-stopped.
        l_oisr0_status.value = in32(OCB_OISR0); // read high order 32 bits of OISR0


        if (l_oisr0_status.fields.check_stop_ppc405 ||   // System Checkstop
            l_oisr0_status.fields.gpe0_error        ||   // GPE0 Halt
            l_oisr0_status.fields.gpe1_error)            // GPE1 Halt
        {
            errlHndl_t l_err = NULL;

            if (l_oisr0_status.fields.gpe0_error)
            {
                TRAC_IMP("task_misc_405_checks: Frozen GPE0 detected by RTL: OISR0[0x%08x]",
                         l_oisr0_status.value);
                l_reason_code = OCC_GPE_HALTED;
            }

            if (l_oisr0_status.fields.gpe1_error)
            {
                TRAC_IMP("task_misc_405_checks: Frozen GPE1 detected by RTL: OISR0[0x%08x]",
                         l_oisr0_status.value);
                /*
                 * @errortype
                 * @moduleid    MAIN_SYSTEM_HALTED_MID
                 * @reasoncode  OCC_GPE_HALTED
                 * @userdata1   OCB_OISR0
                 * @devdesc     OCC detected frozen GPE
                 */
                l_reason_code = OCC_GPE_HALTED;
            }

            if (l_oisr0_status.fields.check_stop_ppc405)
            {
                TRAC_IMP("task_misc_405_checks: System checkstop detected by RTL: OISR0[0x%08x]",
                         l_oisr0_status.value);
                /*
                 * @errortype
                 * @moduleid    MAIN_SYSTEM_HALTED_MID
                 * @reasoncode  OCC_SYSTEM_HALTED
                 * @userdata1   OCB_OISR0
                 * @devdesc     OCC detected system checkstop
                 */
                l_reason_code = OCC_SYSTEM_HALTED;
            }

            L_checkstop_traced = true;

            l_err = createErrl(MAIN_SYSTEM_HALTED_MID,
                               l_reason_code,
                               OCC_NO_EXTENDED_RC,
                               ERRL_SEV_INFORMATIONAL,
                               NULL,
                               DEFAULT_TRACE_SIZE,
                               l_oisr0_status.value,
                               0 );

            // The commit code will check for the frozen GPE0 and system
            // checkstop conditions and take appropriate actions.
            commitErrl(&l_err);
        }
    }
    while(0);

} // end task_misc_405_checks()


// Trigger interrupt to the host with the specified reason (OCCMISC/core_ext_intr)
// Returns true if notification was sent, false if interrupt already outstanding
// If notifcation was not sent, G_host_notifications_pending will be set
// so that it is retried during next tick.
bool notify_host(const ext_intr_reason_t i_reason)
{
    bool notify_success = false;
    //TRAC_INFO("notify_host(0x%02X) called (G_host_notifications_pending=0x%02X)", i_reason, G_host_notifications_pending);

    // Use input reason unless there are outstanding notifications pending
    uint8_t notifyReason = i_reason;
    if (G_host_notifications_pending)
    {
        // Add reason to pending notifications
        notifyReason = G_host_notifications_pending | i_reason;
        // Determine next outstanding notifcation (highest priority first)
        if (notifyReason & INTR_REASON_OPAL_SHARED_MEM_CHANGE)
        {
            notifyReason = INTR_REASON_OPAL_SHARED_MEM_CHANGE;
        }
        else if (notifyReason & INTR_REASON_HTMGT_SERVICE_REQUIRED)
        {
            notifyReason = INTR_REASON_HTMGT_SERVICE_REQUIRED;
        }
        else if (notifyReason & INTR_REASON_I2C_OWNERSHIP_CHANGE)
        {
            notifyReason = INTR_REASON_I2C_OWNERSHIP_CHANGE;
        }
        else
        {
            INTR_TRAC_ERR("notify_host: G_host_notification_pending has unrecognized value: 0x%02X",
                          notifyReason);
            notifyReason = i_reason;
        }
    }

    // Build new value for register
    ocb_occmisc_t new_occmisc = {0};
    new_occmisc.fields.core_ext_intr = 1;
    switch(notifyReason)
    {
        case INTR_REASON_HTMGT_SERVICE_REQUIRED:
            new_occmisc.fields.ext_intr_service_required = 1;
            break;
        case INTR_REASON_I2C_OWNERSHIP_CHANGE:
            new_occmisc.fields.ext_intr_i2c_change = 1;
            break;
        case INTR_REASON_OPAL_SHARED_MEM_CHANGE:
            new_occmisc.fields.ext_intr_shmem_change = 1;
            break;
        default:
            INTR_TRAC_ERR("notify_host: Invalid reason specified: 0x%02X", notifyReason);
            new_occmisc.value = 0;
            break;
    }

    if (new_occmisc.value != 0)
    {
        // Check if we can send interrupt to host (no other interrupts outstanding)
        ocb_occmisc_t current_occmisc;
        current_occmisc.value = in32(OCB_OCCMISC);
        if (current_occmisc.fields.core_ext_intr == 0)
        {
            if (current_occmisc.fields.ext_intr_service_required ||
                current_occmisc.fields.ext_intr_i2c_change ||
                current_occmisc.fields.ext_intr_shmem_change)
            {
                // clear external interrupt reason
                current_occmisc.value = 0;
                current_occmisc.fields.ext_intr_service_required = 1;
                current_occmisc.fields.ext_intr_i2c_change = 1;
                current_occmisc.fields.ext_intr_shmem_change = 1;
                out32(OCB_OCCMISC_CLR, current_occmisc.value);
            }

            out32(OCB_OCCMISC_OR, new_occmisc.value);
            notify_success = true;
            TRAC_INFO("notify_host: notification of reason 0x%02X has been sent", notifyReason);
            G_host_notifications_pending &= ~notifyReason;
        }
        else
        {
            // Host already has an interrupt outstanding, resend notification later
            if ((G_host_notifications_pending & i_reason) == 0)
            {
                TRAC_INFO("notify_host: OCCMISC/core_ext_intr not clear yet (register=0x%08X, reason=0x%02X)",
                          current_occmisc.value, i_reason);
                G_host_notifications_pending |= i_reason;
            }
        }
    }

    return notify_success;
}


