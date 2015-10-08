/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/pss/dpss_init.c $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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
#include <common_types.h>       // imageHdr_t declaration and image header macro
#include <errl.h>               // For error handle
#include <dpss.h>
#include <trac.h>               // For traces
#include <occ_sys_config.h>
#include <occ_service_codes.h>  // for SSX_GENERIC_FAILURE
#include <pss_service_codes.h>
#include <state.h>

//*************************************************************************/
// Externs
//*************************************************************************/
extern PoreEntryPoint GPE_dpss_send_command_stream;
extern PoreFlex G_dpss_read_status_request;
extern gpeDpssCommandStream_t G_gpe_dpss_read_status;

//*************************************************************************/
// Macros
//*************************************************************************/

//*************************************************************************/
// Defines/Enums
//*************************************************************************/

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
// Name:  dpss_initialize
//
// Description: Initializes DPSS fans and oversubscription interrupt
//   If errHndl is returned, caller should call this function again,
//   one time only, in the hopes that it will work on the retry.
//
// End Function Specification
errlHndl_t dpss_initialize(void)
{
    errlHndl_t l_err = NULL;
    PoreFlex request;
    uint8_t i, l_idx_limit;  // index counter and limit value

    // Build command data
    // Structures from this array we will be passed into our GPE program one-by-one
    gpeDpssCommandStream_t l_gpe_dpss_stream_dpss_config[] = {

        { .dpss_msg_stream = {
              .command[0] = DPSS_CMD_SET_MIN_FANS  | G_sysConfigData.dpss_fan.min_fans,
              .command[1] = DPSS_CMD_SET_PWM_DELAY | G_sysConfigData.dpss_fan.pwm_delay_reg,
              .command[2] = DPSS_CMD_SET_PWM_STEP  | G_sysConfigData.dpss_fan.pwm_step_reg,
              .command[3] = 0x0000,
              .response = {0}
          },
        },

        { .dpss_msg_stream = {
              .command[0] = DPSS_CMD_SET_FAN_PPR_0 | G_sysConfigData.dpss_fan.fan_ppr[0],
              .command[1] = DPSS_CMD_SET_FAN_PPR_1 | G_sysConfigData.dpss_fan.fan_ppr[1],
              .command[2] = 0x0000,
              .command[3] = 0x0000,
              .response = {0}
          },
        },

        // Fan hysterisis: write low byte, then high byte
        { .dpss_msg_stream = {
              .command[0] = DPSS_CMD_SET_FAN_HYST_1_LO | G_sysConfigData.dpss_fan.fan_hysterisis[1],
              .command[1] = DPSS_CMD_SET_FAN_HYST_0_HI | G_sysConfigData.dpss_fan.fan_hysterisis[0],
              .command[2] = 0x0000,
              .command[3] = 0x0000,
              .response = {0}
          },
        },

        // Fan hysterisis: write low byte, then high byte
        { .dpss_msg_stream = {
              .command[0] = DPSS_CMD_SET_FAN_HYST_3_LO | G_sysConfigData.dpss_fan.fan_hysterisis[3],
              .command[1] = DPSS_CMD_SET_FAN_HYST_2_HI | G_sysConfigData.dpss_fan.fan_hysterisis[2],
              .command[2] = DPSS_CMD_SET_FAN_MODE      | G_sysConfigData.dpss_fan.fan_mode,
              .command[3] = 0x0000,
              .response = {0}
          },
        },

        { .dpss_msg_stream = {
              .command[0] = DPSS_CMD_SET_FAN_MASK      | G_sysConfigData.dpss_fan.fan_mask,
              .command[1] = DPSS_CMD_SET_FAN_HYST_MASK | G_sysConfigData.dpss_fan.fan_hyst_mask,
              .command[2] = DPSS_CMD_SET_MAX_FAN_PWM   | G_sysConfigData.dpss_fan.max_fan_pwm,
              .command[3] = 0x0000,
              .response = {0}
          },
        },

        { .dpss_msg_stream = {
              .command[0] = DPSS_CMD_SET_MIN_PWM  | G_sysConfigData.dpss_fan.min_pwm,
              .command[1] = DPSS_CMD_SET_SPI_FFS  | G_sysConfigData.dpss_fan.spi_ffs,
              .command[2] = DPSS_CMD_SET_SPIS_INT_MASK   | G_sysConfigData.dpss_spis_int_mask,
              .command[3] = 0x0000,
              .response = {0}
          },
        },

        { .dpss_msg_stream = {
              .command[0] = DPSS_CMD_SET_END_COUNT       | G_sysConfigData.dpss_fan.end_count_reg,
              .command[1] = DPSS_CMD_SET_FAN_WARN_COUNT  | G_sysConfigData.dpss_fan.fan_warning_cnt,
              .command[2] = 0x0000,
              .command[3] = 0x0000,
              .response = {0}
          },
        },
    };

    // Calculate the array size dynamically so we don't have to update it if the array changes.
    l_idx_limit = sizeof(l_gpe_dpss_stream_dpss_config) / sizeof(gpeDpssCommandStream_t);

    for( i = 0; i < l_idx_limit; i++) {
        // Clear the error reporting parts of the argument structure.
        l_gpe_dpss_stream_dpss_config[i].gpe_error.error = 0;
        l_gpe_dpss_stream_dpss_config[i].gpe_error.ffdc  = 0;

        // Create GPE program request
        DPSS_DEBUG_PRINTF(("%s: Calling GPE_dpss_send_command_stream for index %d\n", __FUNCTION__, i));
        pore_flex_create(&request,                                    // request
                         &pore_gpe0_queue,                            // queue
                         (void*)GPE_dpss_send_command_stream,         // GPE entry_point
                         (uint32_t)&l_gpe_dpss_stream_dpss_config[i], // GPE argument_ptr
                         NULL,                                        // callback
                         NULL,                                        // callback arg
                         ASYNC_REQUEST_BLOCKING);                     // options

        // Schedule the request to be executed
        // Because our GPE structures are not in non-cacheable RAM (they are in the .init section instead),
        // we have to flush the dcache before making the pore flex request, then invalidate the dcache afterward.
        dcache_flush(&l_gpe_dpss_stream_dpss_config[i], sizeof(l_gpe_dpss_stream_dpss_config[i]));
        pore_flex_schedule(&request);
        dcache_invalidate((void *)(&l_gpe_dpss_stream_dpss_config[i]), sizeof(l_gpe_dpss_stream_dpss_config[i]));

        DPSS_DEBUG_PRINTF(("%s: GPE_dpss_send_command_stream for index %d returned: 0x%08X\n", __FUNCTION__, i, l_gpe_dpss_stream_dpss_config[i].gpe_error.rc));
        DPSS_DEBUG_PRINTF(("\tcommand[0] = 0x%04x, response[0] = 0x%04x\n", l_gpe_dpss_stream_dpss_config[i].dpss_msg_stream.command[0], l_gpe_dpss_stream_dpss_config[i].dpss_msg_stream.response[0]));
        DPSS_DEBUG_PRINTF(("\tcommand[1] = 0x%04x, response[1] = 0x%04x\n", l_gpe_dpss_stream_dpss_config[i].dpss_msg_stream.command[1], l_gpe_dpss_stream_dpss_config[i].dpss_msg_stream.response[1]));
        DPSS_DEBUG_PRINTF(("\tcommand[2] = 0x%04x, response[2] = 0x%04x\n", l_gpe_dpss_stream_dpss_config[i].dpss_msg_stream.command[2], l_gpe_dpss_stream_dpss_config[i].dpss_msg_stream.response[2]));
        DPSS_DEBUG_PRINTF(("\tcommand[3] = 0x%04x, response[3] = 0x%04x\n", l_gpe_dpss_stream_dpss_config[i].dpss_msg_stream.command[3], l_gpe_dpss_stream_dpss_config[i].dpss_msg_stream.response[3]));

        // Check for errors and invalid DPSS responses.
        // (Valid DPSS responses should be an echo of the cmd & data passed in).
        if ( (l_gpe_dpss_stream_dpss_config[i].gpe_error.rc != 0) ||
             (l_gpe_dpss_stream_dpss_config[i].dpss_msg_stream.response[0] != l_gpe_dpss_stream_dpss_config[i].dpss_msg_stream.command[0]) ||
             (l_gpe_dpss_stream_dpss_config[i].dpss_msg_stream.response[1] != l_gpe_dpss_stream_dpss_config[i].dpss_msg_stream.command[1]) ||
             (l_gpe_dpss_stream_dpss_config[i].dpss_msg_stream.response[2] != l_gpe_dpss_stream_dpss_config[i].dpss_msg_stream.command[2]) )
        {
            DPSS_DEBUG_PRINTF(("%s: ...Failed with error.\n", __FUNCTION__));

            /*@
             * @moduleid   PSS_MID_DPSS_INIT
             * @reasonCode INTERNAL_HW_FAILURE
             * @severity   ERRL_SEV_UNRECOVERABLE
             * @userdata1  GPE error return code
             * @userdata2  GPE error ffdc
             * @userdata4  OCC_NO_EXTENDED_RC
             * @devdesc    GPE command failed to initialize the DPSS
             */
            l_err = createErrl(PSS_MID_DPSS_INIT,       // i_modId,
                               INTERNAL_HW_FAILURE,     // i_reasonCode,
                               OCC_NO_EXTENDED_RC,      // extended reason code // @nh001a
                               ERRL_SEV_UNRECOVERABLE,
                               NULL,                    // tracDesc_t i_trace,
                               0x0000,                  // i_traceSz,
                               l_gpe_dpss_stream_dpss_config[i].gpe_error.rc,          // i_userData1,
                               l_gpe_dpss_stream_dpss_config[i].gpe_error.ffdc >> 32); // i_userData2

            // Put extra debug info into local data struct
            addUsrDtlsToErrl(l_err,
                             (uint8_t*)&l_gpe_dpss_stream_dpss_config[i],
                             sizeof(l_gpe_dpss_stream_dpss_config[i]),
                             ERRL_STRUCT_VERSION_1,
                             ERRL_USR_DTL_TRACE_DATA);
            break;
        }
        else {
            DPSS_DEBUG_PRINTF(("%s: ...Success!\n", __FUNCTION__));
        }
    }

    return l_err;
}

// Function Specification
//
// Name:  dpssInitApplet (old name is start_dpss)
//
// Description:
// Entry-point for enabling DPSS functionality.
// Initializes the DPSS chip.  Starts the "DPSS Read Status" RTLS task.
//
// End Function Specification
errlHndl_t dpssInitApplet(void * i_arg)
{
    // Init DPSS
    TRAC_INFO("Initializing DPSS registers...");

    errlHndl_t l_errl = dpss_initialize();

    if (l_errl)
    {
        // init failed, attempt one more time before giving up
        TRAC_ERR("dpss_initialize failed! (retrying)...");

        // Convert the error severity to info and log it.
        setErrlSevToInfo(l_errl);
        commitErrl( &l_errl );

        l_errl = dpss_initialize();

        if (l_errl)
        {
            TRAC_ERR("dpss_initialize failed again!  OCC will be reset.");
            // Log the error with its original unrecoverable severity
            commitErrl( &l_errl );
            REQUEST_RESET();
        }
    }

    if (!l_errl)
    {
        TRAC_INFO("...DPSS initialized.");
        TRAC_INFO("Enabling DPSS Read Status RTLS task.");

        // Init the global DPSS read-status PORE flex request.
        // None of these values is expected to change.
        pore_flex_create(&G_dpss_read_status_request,                 // request
                         &pore_gpe0_queue,                            // queue
                         (void*)GPE_dpss_send_command_stream,         // GPE entry_point
                         (uint32_t)&G_gpe_dpss_read_status,           // GPE argument_ptr
                         NULL,                                        // callback
                         NULL,                                        // callback arg
                         0);                                          // options  DO NOT set this to ASYNC_REQUEST_BLOCKING

        // Make this task runnable.
        rtl_start_task(TASK_ID_DPSS_RD_STATUS);
    }

    return l_errl;
}

