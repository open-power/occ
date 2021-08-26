/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/common.c $                                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2021                        */
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
#include "sensor.h"

uint8_t G_host_notifications_pending = 0;
extern uint16_t G_allow_trace_flags;
extern uint8_t G_occ_interrupt_type;
#define TICKS_TO_DELAY_CHECKSTOP_PROCESSING 4

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
    ocb_oisr1_t  l_oisr1_status;       // OCC Interrupt Source 1 Register

    static bool L_checkstop_traced    = false;
    uint8_t     l_reason_code         = 0;
    bool        l_create_errl         = false;
    static unsigned int L_delay_cstop = TICKS_TO_DELAY_CHECKSTOP_PROCESSING;
    sensor_t   *l_sensor              = NULL;

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
        l_oisr1_status.value = in32(OCB_OISR1); // read high order 32 bits of OISR1

        if (l_oisr1_status.fields.check_stop_ppc405 ||   // System Checkstop
            l_oisr1_status.fields.gpe0_error        ||   // GPE0 Halt
            l_oisr1_status.fields.gpe1_error)            // GPE1 Halt
        {
            if(l_oisr1_status.fields.check_stop_ppc405)
            {
                // For FSP systems, delay the system checkstop processing to allow NVDIMM procedure to run
                if ((G_occ_interrupt_type == FSP_SUPPORTED_OCC) && (L_delay_cstop > 0))
                {
                    if (L_delay_cstop == TICKS_TO_DELAY_CHECKSTOP_PROCESSING)
                    {
                        TRAC_IMP("task_misc_405_checks: System checkstop detected by RTL: OISR1[0x%08x] - delaying halt (tick=%d)",
                                 l_oisr1_status.value, CURRENT_TICK);
                    }
                    --L_delay_cstop;
                }
                else
                {
                    l_create_errl = true;
                }
            }
            else
            {
                // GPE0/GPE1 Halt
                l_create_errl = true;
            }
        }

        if(l_create_errl)
        {
            errlHndl_t l_err = NULL;

            ocb_oisr0_t l_oisr0_status; // OCC Interrupt Source 0 Register
            l_oisr0_status.value = in32(OCB_OISR0);

            if (l_oisr1_status.fields.gpe0_error)
            {
                TRAC_IMP("task_misc_405_checks: Frozen GPE0 detected by RTL: OISR1[0x%08x]",
                         l_oisr1_status.value);
                l_reason_code = OCC_GPE_HALTED;
            }

            if (l_oisr1_status.fields.gpe1_error)
            {
                TRAC_IMP("task_misc_405_checks: Frozen GPE1 detected by RTL: OISR1[0x%08x]",
                         l_oisr1_status.value);
                /*
                 * @errortype
                 * @moduleid    MAIN_SYSTEM_HALTED_MID
                 * @reasoncode  OCC_GPE_HALTED
                 * @userdata1   OCB_OISR1
                 * @userdata2   OCB_OISR0
                 * @devdesc     OCC detected frozen GPE
                 */
                l_reason_code = OCC_GPE_HALTED;
            }

            if (l_oisr1_status.fields.check_stop_ppc405)
            {
                TRAC_ERR("task_misc_405_checks: System checkstop detected by RTL: OISR1[0x%08x]",
                         l_oisr1_status.value);

                // requested by char team to add sensors for checkstop analysis
                l_sensor = getSensorByGsid(UV_AVG);
                TRAC_IMP("task_misc_405_checks UV_AVG: current[%d] min[%d] max[%d]",
                          l_sensor->sample, l_sensor->sample_min, l_sensor->sample_max);

                l_sensor = getSensorByGsid(OV_AVG);
                TRAC_IMP("task_misc_405_checks OV_AVG: current[%d] min[%d] max[%d]",
                          l_sensor->sample, l_sensor->sample_min, l_sensor->sample_max);

                l_sensor = getSensorByGsid(VOLTVDD);
                TRAC_IMP("task_misc_405_checks VOLTVDD: current[%d] min[%d] max[%d]",
                          l_sensor->sample, l_sensor->sample_min, l_sensor->sample_max);

                l_sensor = getSensorByGsid(CURVDD);
                TRAC_IMP("task_misc_405_checks CURVDD: current[%d] min[%d] max[%d]",
                          l_sensor->sample, l_sensor->sample_min, l_sensor->sample_max);

                l_sensor = getSensorByGsid(FREQA);
                TRAC_IMP("task_misc_405_checks FREQA: current[%d] min[%d] max[%d]",
                          l_sensor->sample, l_sensor->sample_min, l_sensor->sample_max);

                l_sensor = getSensorByGsid(DDSMIN);
                TRAC_IMP("task_misc_405_checks DDSMIN: current[%d] min[%d] max[%d]",
                          l_sensor->sample, l_sensor->sample_min, l_sensor->sample_max);

                l_sensor = getSensorByGsid(CEFFVDDRATIOADJ);
                TRAC_IMP("task_misc_405_checks CEFFVDDRATIOADJ: current[%d] min[%d] max[%d]",
                          l_sensor->sample, l_sensor->sample_min, l_sensor->sample_max);

                l_sensor = getSensorByGsid(TEMPPROCTHRM);
                TRAC_IMP("task_misc_405_checks TEMPPROCTHRM: current[%d] min[%d] max[%d]",
                          l_sensor->sample, l_sensor->sample_min, l_sensor->sample_max);

                /*
                 * @errortype
                 * @moduleid    MAIN_SYSTEM_HALTED_MID
                 * @reasoncode  OCC_SYSTEM_HALTED
                 * @userdata1   OCB_OISR1
                 * @userdata2   OCB_OISR0
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
                               l_oisr1_status.value,
                               l_oisr0_status.value);

            // add PGPE specific data
            addPgpeDataToErrl(l_err);

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
    static uint16_t L_traced_once = 0;

    if( (G_allow_trace_flags & ALLOW_OPAL_TRACE) && G_host_notifications_pending)
    {
        TRAC_INFO("notify_host(0x%02X) called (G_host_notifications_pending=0x%02X)", i_reason, G_host_notifications_pending);
    }

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
        else
        {
            INTR_TRAC_ERR("notify_host: G_host_notification_pending has unrecognized value: 0x%02X",
                          notifyReason);
            notifyReason = 0;
        }
    }

    // Build new value for register
    ocb_occmisc_t new_occmisc = {0};
    new_occmisc.fields.core_ext_intr = 1;
    switch(notifyReason)
    {
        case INTR_REASON_HTMGT_SERVICE_REQUIRED:
            new_occmisc.fields.ext_intr_reason = INTR_REASON_HTMGT_SERVICE_REQUIRED;
            break;
        case INTR_REASON_OPAL_SHARED_MEM_CHANGE:
            new_occmisc.fields.ext_intr_reason = INTR_REASON_OPAL_SHARED_MEM_CHANGE;
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
            if (current_occmisc.fields.ext_intr_reason != 0)
            {
                // clear external interrupt reason
                current_occmisc.value = 0;
                current_occmisc.fields.ext_intr_reason = (INTR_REASON_HTMGT_SERVICE_REQUIRED |
                                                          INTR_REASON_I2C_OWNERSHIP_CHANGE |
                                                          INTR_REASON_OPAL_SHARED_MEM_CHANGE);
                out32(OCB_OCCMISC_CLR, current_occmisc.value);
            }

            out32(OCB_OCCMISC_OR, new_occmisc.value);
            notify_success = true;

            if( (G_allow_trace_flags & ALLOW_OPAL_TRACE) ||
                ((L_traced_once & notifyReason) == 0) )
            {
                TRAC_INFO("notify_host: notification of reason 0x%02X has been sent",
                           notifyReason);
                L_traced_once |= notifyReason;
            }
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

// Called prior to logging any error related to the PGPE or Pstate control
// i.e. PGPE communication, maintaining power cap...
// During prolonged droop events the PGPE can be non-responsive and don't have frequency control so doing a pm reset will
// not help.  The PGPE will set a bit in the OCC FLAGS register to indicate when in this condition for the OCC to ignore errors
// Returns true if the error should be ignored
bool ignore_pgpe_error(void)
{
    static bool  L_last_ignore_error = false;
           bool  l_ignore_error      = false;
           ocb_occflg_t occ_flags   = {0};

    // Check if the bit to ignore errors is set in the OCC Flags register
    occ_flags.value = in32(OCB_OCCFLG0);

    if (occ_flags.fields.pm_reset_suppress == 1)
    {
        l_ignore_error = true;
    }

    // Trace if this is a change from the last time this was called
    if (L_last_ignore_error != l_ignore_error)
    {
        TRAC_ERR("ignore_pgpe_error: OCCFLG pm_reset_suppress was %d and is now %d", L_last_ignore_error, l_ignore_error);
        L_last_ignore_error = l_ignore_error;
    }

    return l_ignore_error;
}

