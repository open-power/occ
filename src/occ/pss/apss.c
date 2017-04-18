/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/pss/apss.c $                                              */
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

#include "ssx.h"

#include <trac_interface.h>
#include <apss.h>
#include <occ_common.h>
#include <comp_ids.h>
#include <pss_service_codes.h>
#include <occ_service_codes.h>
#include <trac.h>
#include <state.h>
#include <occ_sys_config.h>
#include <dcom.h>
#include "pss_constants.h"

// Threshold for calling out the redundant APSS
#define MAX_BACKUP_FAILURES  8

// Configure both GPIOs (directoin/drive/interrupts): All Input, All 1's, No Interrupts
const apssGpioConfigStruct_t G_gpio_config[2] = { {0x00, 0xFF, 0x00}, {0x00, 0xFF, 0x00} };

// Configure streaming of: 16 ADCs, 2 GPIOs
const apssCompositeConfigStruct_t G_apss_composite_config = { 16, 2 };

// Power Measurements (read from APSS every RealTime loop)
apssPwrMeasStruct_t G_apss_pwr_meas = { {0} };

GPE_BUFFER(initGpioArgs_t G_gpe_apss_initialize_gpio_args);
GPE_BUFFER(setCompositeModeArgs_t G_gpe_apss_set_composite_mode_args);

uint64_t G_gpe_apss_time_start;
uint64_t G_gpe_apss_time_end;

// Flag for requesting APSS recovery when OCC detects all zeroes or data out of sync
bool G_apss_recovery_requested = FALSE;

GPE_BUFFER(apss_start_args_t    G_gpe_start_pwr_meas_read_args);
GPE_BUFFER(apss_continue_args_t G_gpe_continue_pwr_meas_read_args);
GPE_BUFFER(apss_complete_args_t G_gpe_complete_pwr_meas_read_args);

PoreEntryPoint GPE_apss_start_pwr_meas_read;
PoreEntryPoint GPE_apss_continue_pwr_meas_read;
PoreEntryPoint GPE_apss_complete_pwr_meas_read;

// Up / down counter for redundant apss failures
uint32_t G_backup_fail_count = 0;

// Used to tell slave inbox that pwr meas is complete
volatile bool G_ApssPwrMeasCompleted = FALSE;

// Function Specification
//
// Name: dumpHexString
//
// Description: TODO Add description
//
// End Function Specification
#if ( (!defined(NO_TRAC_STRINGS)) && defined(TRAC_TO_SIMICS) )

// Utility function to dump hex data to screen
void dumpHexString(const void *i_data, const unsigned int len, const char *string)
{
  unsigned int i, j;
  char text[17];
  uint8_t *data = (uint8_t*)i_data;
  unsigned int l_len = len;

  text[16] = '\0';
  if (string != NULL)
  {
    printf("%s\n", string);
  }

  if (len > 0x0800) l_len = 0x800;
  for(i = 0; i < l_len; i++)
  {
    if (i % 16 == 0)
    {
      if (i > 0) printf("   \"%s\"\n", text);
      printf("   %04x:",i);
    }
    if (i % 4 == 0) printf(" ");

    printf("%02x",(int)data[i]);
    if (isprint(data[i])) text[i%16] = data[i];
    else text[i%16] = '.';
  }
  if ((i%16) != 0) {
    text[i%16] = '\0';
    for(j = (i % 16); j < 16; ++j) {
      printf("  ");
      if (j % 4 == 0) printf(" ");
    }
  }
  printf("   \"%s\"\n", text);
  return;
}
#endif

// Function Specification
//
// Name: do_apss_recovery
//
// Description: Collect FFDC and attempt recovery for APSS failures
//
// End Function Specification
void do_apss_recovery(void)
{
#define PSS_START_COMMAND 0x8000000000000000ull
#define PSS_RESET_COMMAND 0x4000000000000000ull
    int             l_scom_rc           = 0;
    uint32_t        l_scom_addr;
    uint64_t        l_spi_adc_ctrl0;
    uint64_t        l_spi_adc_ctrl1;
    uint64_t        l_spi_adc_ctrl2;
    uint64_t        l_spi_adc_status;
    uint64_t        l_spi_adc_reset;
    uint64_t        l_spi_adc_wdata;


    TRAC_ERR("detected invalid power data[%08x%08x]",
             (uint32_t)(G_gpe_continue_pwr_meas_read_args.meas_data[0] >> 32),
             (uint32_t)(G_gpe_continue_pwr_meas_read_args.meas_data[0] & 0x00000000ffffffffull));

    do
    {
        // Collect SPI ADC FFDC data
        l_scom_addr = SPIPSS_ADC_RESET_REG;
        l_scom_rc = _getscom(l_scom_addr, &l_spi_adc_reset, SCOM_TIMEOUT);
        if(l_scom_rc)
        {
            break;
        }
        l_scom_addr = SPIPSS_ADC_CTRL_REG0;
        l_scom_rc = _getscom(l_scom_addr, &l_spi_adc_ctrl0, SCOM_TIMEOUT);
        if(l_scom_rc)
        {
            break;
        }
        l_scom_addr = SPIPSS_ADC_CTRL_REG1;
        l_scom_rc = _getscom(l_scom_addr, &l_spi_adc_ctrl1, SCOM_TIMEOUT);
        if(l_scom_rc)
        {
            break;
        }
        l_scom_addr = SPIPSS_ADC_CTRL_REG2;
        l_scom_rc = _getscom(l_scom_addr, &l_spi_adc_ctrl2, SCOM_TIMEOUT);
        if(l_scom_rc)
        {
            break;
        }
        l_scom_addr = SPIPSS_ADC_STATUS_REG;
        l_scom_rc = _getscom(l_scom_addr, &l_spi_adc_status, SCOM_TIMEOUT);
        if(l_scom_rc)
        {
            break;
        }
        l_scom_addr = SPIPSS_ADC_WDATA_REG;
        l_scom_rc = _getscom(l_scom_addr, &l_spi_adc_wdata, SCOM_TIMEOUT);
        if(l_scom_rc)
        {
            break;
        }

        TRAC_ERR("70000[%08x] 70001[%08x] 70002[%08x] 70003|70005[%08x] 70010[%08x]",
                (uint32_t)(l_spi_adc_ctrl0 >> 32),
                (uint32_t)(l_spi_adc_ctrl1 >> 32),
                (uint32_t)(l_spi_adc_ctrl2 >> 32),
                (uint32_t)((l_spi_adc_status >> 32) | (l_spi_adc_reset >> 48)), // Stuff reset register in lower 16 bits
                (uint32_t)(l_spi_adc_wdata >> 32));

        // Special error handling on OCC backup. Keep an up/down counter of
        // fail/success and log predictive error when we reach the limit.
        if(G_occ_role == OCC_SLAVE)
        {
            if(G_backup_fail_count < MAX_BACKUP_FAILURES)
            {
                // Increment the up/down counter
                G_backup_fail_count++;
            }
            else
            {
                //We're logging the error so stop running apss tasks
                rtl_stop_task(TASK_ID_APSS_START);
                rtl_stop_task(TASK_ID_APSS_CONT);
                rtl_stop_task(TASK_ID_APSS_DONE);

                TRAC_INFO("Redundant APSS has exceeded failure threshold.  Logging Error");

                /*
                 * @errortype
                 * @moduleid    PSS_MID_DO_APSS_RECOVERY
                 * @reasoncode  REDUNDANT_APSS_GPE_FAILURE
                 * @userdata1   0
                 * @userdata2   0
                 * @userdata4   OCC_NO_EXTENDED_RC
                 * @devdesc     Redundant APSS failure.  Power Management Redundancy Lost.
                 */
                errlHndl_t l_err = createErrl(PSS_MID_DO_APSS_RECOVERY,
                                              REDUNDANT_APSS_GPE_FAILURE,
                                              OCC_NO_EXTENDED_RC,
                                              ERRL_SEV_PREDICTIVE,
                                              NULL,
                                              DEFAULT_TRACE_SIZE,
                                              0,
                                              0);

                // APSS callout
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_HUID,
                                 G_sysConfigData.apss_huid,
                                 ERRL_CALLOUT_PRIORITY_HIGH);
                // Processor callout
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_HUID,
                                 G_sysConfigData.proc_huid,
                                 ERRL_CALLOUT_PRIORITY_LOW);
                // Backplane callout
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_HUID,
                                 G_sysConfigData.backplane_huid,
                                 ERRL_CALLOUT_PRIORITY_LOW);
                // Firmware callout
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                 ERRL_COMPONENT_ID_FIRMWARE,
                                 ERRL_CALLOUT_PRIORITY_MED);

                commitErrl(&l_err);

                break;
            }
        }

        TRAC_INFO("Starting APSS recovery.  fail_count=%d", G_backup_fail_count);

        // Reset the ADC engine
        l_scom_addr = SPIPSS_ADC_RESET_REG;
        l_scom_rc = _putscom(l_scom_addr, PSS_RESET_COMMAND, SCOM_TIMEOUT);
        if(l_scom_rc)
        {
            break;
        }

        // Zero out the reset register
        l_scom_rc = _putscom(l_scom_addr, 0, SCOM_TIMEOUT);
        if(l_scom_rc)
        {
            break;
        }

        // Attempt recovery by sending the apss
        // command that was set up earlier by initialization GPE
        l_scom_addr = SPIPSS_P2S_COMMAND_REG;
        l_scom_rc = _putscom(l_scom_addr, PSS_START_COMMAND, SCOM_TIMEOUT);
        if(l_scom_rc)
        {
            break;
        }
    }while(0);

    // Just trace it if we get a scom failure trying to collect FFDC
    if(l_scom_rc)
    {
        TRAC_ERR("apss recovery scom failure. addr=0x%08x, rc=0x%08x", l_scom_addr, l_scom_rc);
    }
}

// Note: The complete request must be global, since it must stick around until after the
//       GPE program has completed (in order to do the callback).
PoreFlex G_meas_start_request;
// Function Specification
//
// Name:  task_apss_start_pwr_meas
//
// Description: Start the GPE program to request power measurement data from APSS
//              If previous call had failed, commit the error and request reset
//
// Task Flags: RTL_FLAG_MSTR, RTL_FLAG_OBS, RTL_FLAG_ACTIVE, RTL_FLAG_APSS_PRES
//
// End Function Specification
void task_apss_start_pwr_meas(struct task *i_self)
{
    int             l_rc                = 0;
    static bool     L_scheduled         = FALSE;
    static bool     L_ffdc_collected    = FALSE;

    // Create/schedule GPE_start_pwr_meas_read (non-blocking)
    APSS_DBG("GPE_start_pwr_meas_read started\n");

    do
    {
        if (!async_request_is_idle(&G_meas_start_request.request))
        {
            TRAC_ERR("task_apss_start_pwr_meas: request is not idle.");
            break;
        }

        // Check if we need to try recovering the apss
        if(G_apss_recovery_requested)
        {
            // Do recovery then wait until next tick to do anything more.
            do_apss_recovery();
            break;
        }

        if (L_scheduled)
        {
            if ((ASYNC_REQUEST_STATE_COMPLETE != G_meas_start_request.request.completion_state) ||
                (0 != G_gpe_start_pwr_meas_read_args.error.error))
            {
                //error should only be non-zero in the case where the GPE timed out waiting for
                //the APSS master to complete the last operation.  Just keep retrying until
                //DCOM resets us due to not having valid power data.
                TRAC_ERR("task_apss_start_pwr_meas: request is not complete or failed with an error(rc:0x%08X, ffdc:0x%08X%08X). " \
                        "CompletionState:0x%X.",
                         G_gpe_start_pwr_meas_read_args.error.rc,
                         G_gpe_start_pwr_meas_read_args.error.ffdc >> 32,
                         G_gpe_start_pwr_meas_read_args.error.ffdc,
                         G_meas_start_request.request.completion_state);

                // Collect FFDC and log error once.
                if (!L_ffdc_collected)
                {
                    errlHndl_t l_err = NULL;

                    /*
                     * @errortype
                     * @moduleid    PSS_MID_APSS_START_MEAS
                     * @reasoncode  APSS_GPE_FAILURE
                     * @userdata1   GPE returned rc code
                     * @userdata4   ERC_APSS_COMPLETE_FAILURE
                     * @devdesc     Failure getting power measurement data from APSS
                     */
                    l_err = createErrl(PSS_MID_APSS_START_MEAS,   // i_modId
                                       APSS_GPE_FAILURE,          // i_reasonCode
                                       ERC_APSS_COMPLETE_FAILURE,
                                       ERRL_SEV_INFORMATIONAL,
                                       NULL,
                                       DEFAULT_TRACE_SIZE,
                                       G_gpe_start_pwr_meas_read_args.error.rc,
                                       0);

                    addUsrDtlsToErrl(l_err,
                                     (uint8_t*)&G_meas_start_request.ffdc,
                                     sizeof(G_meas_start_request.ffdc),
                                     ERRL_STRUCT_VERSION_1,
                                     ERRL_USR_DTL_BINARY_DATA);

                    // Commit Error
                    commitErrl(&l_err);

                      // Set to true so that we don't log this error again.
                    L_ffdc_collected = TRUE;
                }
            }
        }

        // Clear these out prior to starting the GPE (GPE only sets them)
        G_gpe_start_pwr_meas_read_args.error.error = 0;
        G_gpe_start_pwr_meas_read_args.error.ffdc = 0;

        // Submit the next request
        l_rc = pore_flex_schedule(&G_meas_start_request);
        if (0 != l_rc)
        {
            errlHndl_t l_err = NULL;

            TRAC_ERR("task_apss_start_pwr_meas: schedule failed w/rc=0x%08X (%d us)", l_rc,
                     (int) ((ssx_timebase_get())/(SSX_TIMEBASE_FREQUENCY_HZ/1000000)));

            /*
             * @errortype
             * @moduleid    PSS_MID_APSS_START_MEAS
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   GPE shedule returned rc code
             * @userdata2   0
             * @userdata4   ERC_APSS_SCHEDULE_FAILURE
             * @devdesc     task_apss_start_pwr_meas schedule failed
             */
            l_err = createErrl(PSS_MID_APSS_START_MEAS,
                               SSX_GENERIC_FAILURE,
                               ERC_APSS_SCHEDULE_FAILURE,
                               ERRL_SEV_PREDICTIVE,
                               NULL,
                               DEFAULT_TRACE_SIZE,
                               l_rc,
                               0);

            // Request reset since this should never happen.
            REQUEST_RESET(l_err);
            L_scheduled = FALSE;
            break;
        }

        L_scheduled = TRUE;


    }while (0);


    APSS_DBG("GPE_start_pwr_meas_read finished w/rc=0x%08X\n", G_gpe_start_pwr_meas_read_args.error.rc);
    APSS_DBG_HEXDUMP(&G_gpe_start_pwr_meas_read_args, sizeof(G_gpe_start_pwr_meas_read_args), "G_gpe_start_pwr_meas_read_args");
    G_ApssPwrMeasCompleted = FALSE;  // Will complete when 3rd task is complete.
    G_gpe_apss_time_start = ssx_timebase_get();


} // end task_apss_start_pwr_meas()


// Note: The complete request must be global, since it must stick around until after the
//       GPE program has completed (in order to do the callback).
PoreFlex G_meas_cont_request;
// Function Specification
//
// Name:  task_apss_continue_pwr_meas
//
// Description: Start GPE to collect 1st block of power measurement data and request
//              the 2nd block
//              If previous call had failed, commit the error and request reset
//
// Task Flags: RTL_FLAG_MSTR, RTL_FLAG_OBS, RTL_FLAG_ACTIVE, RTL_FLAG_APSS_PRES
//
// End Function Specification
void task_apss_continue_pwr_meas(struct task *i_self)
{
    int         l_rc                = 0;
    static bool L_scheduled         = FALSE;
    static bool L_ffdc_collected    = FALSE;

    // Create/schedule GPE_apss_continue_pwr_meas_read (non-blocking)
    APSS_DBG("Calling task_apss_continue_pwr_meas.\n");

    do
    {
        if (!async_request_is_idle(&G_meas_cont_request.request))
        {
            TRAC_ERR("task_apss_continue_pwr_meas: request is not idle.");
            break;
        }

        //Don't run anything if apss recovery is in progress
        if(G_apss_recovery_requested)
        {
            break;
        }

        if (L_scheduled)
        {
            if ((ASYNC_REQUEST_STATE_COMPLETE != G_meas_cont_request.request.completion_state) ||
                (0 != G_gpe_continue_pwr_meas_read_args.error.error))
            {
                //error should only be non-zero in the case where the GPE timed out waiting for
                //the APSS master to complete the last operation.  Just keep retrying until
                //DCOM resets us due to not having valid power data.
                TRAC_ERR("task_apss_continue_pwr_meas: request is not complete or failed with an error(rc:0x%08X, ffdc:0x%08X%08X). " \
                        "CompletionState:0x%X.",
                         G_gpe_continue_pwr_meas_read_args.error.rc,
                         G_gpe_continue_pwr_meas_read_args.error.ffdc >> 32,
                         G_gpe_continue_pwr_meas_read_args.error.ffdc,
                         G_meas_cont_request.request.completion_state);


                // Collect FFDC and log error once.
                if (!L_ffdc_collected)
                {
                    errlHndl_t l_err = NULL;

                    /*
                     * @errortype
                     * @moduleid    PSS_MID_APSS_CONT_MEAS
                     * @reasoncode  APSS_GPE_FAILURE
                     * @userdata1   GPE returned rc code
                     * @userdata2   0
                     * @userdata4   ERC_APSS_COMPLETE_FAILURE
                     * @devdesc     Failure getting power measurement data from APSS
                     */
                    l_err = createErrl(PSS_MID_APSS_CONT_MEAS,   // i_modId
                                       APSS_GPE_FAILURE,          // i_reasonCode
                                       ERC_APSS_COMPLETE_FAILURE,
                                       ERRL_SEV_INFORMATIONAL,
                                       NULL,
                                       DEFAULT_TRACE_SIZE,
                                       G_gpe_continue_pwr_meas_read_args.error.rc,
                                       0);

                    addUsrDtlsToErrl(l_err,
                                     (uint8_t*)&G_meas_cont_request.ffdc,
                                     sizeof(G_meas_cont_request.ffdc),
                                     ERRL_STRUCT_VERSION_1,
                                     ERRL_USR_DTL_BINARY_DATA);

                    // Commit Error
                    commitErrl(&l_err);

                      // Set to true so that we don't log this error again.
                    L_ffdc_collected = TRUE;
                }
            }
        }

        // Clear these out prior to starting the GPE (GPE only sets them)
        G_gpe_continue_pwr_meas_read_args.error.error = 0;
        G_gpe_continue_pwr_meas_read_args.error.ffdc = 0;

        // Submit the next request
        l_rc = pore_flex_schedule(&G_meas_cont_request);
        if (0 != l_rc)
        {
            errlHndl_t l_err = NULL;

            TRAC_ERR("task_apss_cont_pwr_meas: schedule failed w/rc=0x%08X (%d us)", l_rc,
                     (int) ((ssx_timebase_get())/(SSX_TIMEBASE_FREQUENCY_HZ/1000000)));

            /*
             * @errortype
             * @moduleid    PSS_MID_APSS_CONT_MEAS
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   GPE shedule returned rc code
             * @userdata2   0
             * @userdata4   ERC_APSS_SCHEDULE_FAILURE
             * @devdesc     task_apss_continue_pwr_meas schedule failed
             */
            l_err = createErrl(PSS_MID_APSS_CONT_MEAS,
                               SSX_GENERIC_FAILURE,
                               ERC_APSS_SCHEDULE_FAILURE,
                               ERRL_SEV_PREDICTIVE,
                               NULL,
                               DEFAULT_TRACE_SIZE,
                               l_rc,
                               0);

            // Request reset since this should never happen.
            REQUEST_RESET(l_err);
            L_scheduled = FALSE;
            break;
        }

        L_scheduled = TRUE;

    }while (0);

    APSS_DBG("task_apss_continue_pwr_meas: finished w/rc=0x%08X\n", G_gpe_continue_pwr_meas_read_args.error.rc);
    APSS_DBG_HEXDUMP(&G_gpe_continue_pwr_meas_read_args, sizeof(G_gpe_continue_pwr_meas_read_args), "G_gpe_continue_pwr_meas_read_args");

} // end task_apss_continue_pwr_meas

// Function Specification
//
// Name:  reformat_meas_data
//
// Description: Extract measurement from GPE programs into G_apss_pwr_meas structure
//              This function is called when the GPE completes the final measurement
//              collection for this loop.
//
// End Function Specification
#define APSS_ADC_SEQ_MASK 0xf000f000f000f000ull
#define APSS_ADC_SEQ_CHECK 0x0000100020003000ull
void reformat_meas_data()
{
    APSS_DBG("GPE_complete_pwr_meas_read finished w/rc=0x%08X\n", G_gpe_complete_pwr_meas_read_args.error.rc);
    APSS_DBG_HEXDUMP(&G_gpe_complete_pwr_meas_read_args, sizeof(G_gpe_complete_pwr_meas_read_args), "G_gpe_complete_pwr_meas_read_args");

    do
    {
        // Make sure complete was successful
        if (G_gpe_complete_pwr_meas_read_args.error.error)
        {
            break;
        }

        // Check that the first 4 sequence nibbles are 0, 1, 2, 3 in the ADC data
        if (((G_gpe_continue_pwr_meas_read_args.meas_data[0] & APSS_ADC_SEQ_MASK) != APSS_ADC_SEQ_CHECK) ||
             !(G_gpe_continue_pwr_meas_read_args.meas_data[0] & ~APSS_ADC_SEQ_MASK))
        {
            // Recovery will begin on the next tick
            G_apss_recovery_requested = TRUE;
            break;
        }

        // Decrement up/down fail counter for backup on success.
        if(G_backup_fail_count)
        {
            G_backup_fail_count--;
        }

        // Don't do the copy unless this is the master OCC
        if(G_occ_role == OCC_MASTER)
        {

            // Fail every 16 seconds
            APSS_DBG("Populate meas data:\n");

            // Merge continue/complete data into a single buffer
            const uint16_t l_continue_meas_length = sizeof(G_gpe_continue_pwr_meas_read_args.meas_data);
            const uint16_t l_complete_meas_length = sizeof(G_gpe_complete_pwr_meas_read_args.meas_data);
            uint8_t l_buffer[l_continue_meas_length+l_complete_meas_length];
            memcpy(&l_buffer[                     0], G_gpe_continue_pwr_meas_read_args.meas_data, l_continue_meas_length);
            memcpy(&l_buffer[l_continue_meas_length], G_gpe_complete_pwr_meas_read_args.meas_data, l_complete_meas_length);
            APSS_DBG_HEXDUMP(l_buffer, sizeof(l_buffer), "l_buffer");

            // Copy measurements into correct struction locations (based on composite config)
            uint16_t l_index = 0;
            memcpy(G_apss_pwr_meas.adc, &l_buffer[l_index], (G_apss_composite_config.numAdcChannelsToRead * 2));
            l_index += (G_apss_composite_config.numAdcChannelsToRead * 2);
            memcpy(G_apss_pwr_meas.gpio, &l_buffer[l_index], (G_apss_composite_config.numGpioPortsToRead * 2));
            // TOD is always located at same offset
            memcpy(&G_apss_pwr_meas.tod, &l_buffer[l_continue_meas_length+l_complete_meas_length-8], 8);

            APSS_DBG("...into structure: (%d ADC, %d GPIO)\n", G_apss_composite_config.numAdcChannelsToRead,
                G_apss_composite_config.numGpioPortsToRead);
            APSS_DBG_HEXDUMP(&G_apss_pwr_meas, sizeof(G_apss_pwr_meas), "G_apss_pwr_meas");
        }

        // Mark apss pwr meas completed and valid
        G_ApssPwrMeasCompleted = TRUE;
        G_gpe_apss_time_end = ssx_timebase_get();
        APSS_DBG("APSS Completed - %d\n",(int) ssx_timebase_get());
  }while(0);
}


// Note: The complete request must be global, since it must stick around until after the
//       GPE program has completed (in order to do the callback).
PoreFlex G_meas_complete_request;

// Function Specification
//
// Name:  task_apss_complete_pwr_meas
//
// Description: Start GPE to collect 2nd block of power measurement data and TOD.
//              If previous call had failed, commit the error and request reset
//
// Task Flags: RTL_FLAG_MSTR, RTL_FLAG_OBS, RTL_FLAG_ACTIVE, RTL_FLAG_APSS_PRES
//
// End Function Specification
void task_apss_complete_pwr_meas(struct task *i_self)
{
    int         l_rc                = 0;
    static bool L_scheduled         = FALSE;
    static bool L_ffdc_collected    = FALSE;

    // Create/schedule GPE_apss_complete_pwr_meas_read (non-blocking)
    APSS_DBG("Calling task_apss_complete_pwr_meas.\n");

    do
    {
        if (!async_request_is_idle(&G_meas_complete_request.request))
        {
            TRAC_ERR("task_apss_complete_pwr_meas: request is not idle.");
            break;
        }

        if(G_apss_recovery_requested)
        {
            // Allow apss measurement to proceed on next tick
            G_apss_recovery_requested = FALSE;
            break;
        }


        if (L_scheduled)
        {
            if ((ASYNC_REQUEST_STATE_COMPLETE != G_meas_complete_request.request.completion_state) ||
                (0 != G_gpe_complete_pwr_meas_read_args.error.error))
            {
                // Error should only be non-zero in the case where the GPE timed out waiting for
                // the APSS master to complete the last operation. Just keep retrying until
                // DCOM resets us due to not having valid power data.
                TRAC_ERR("task_apss_complete_pwr_meas: request is not complete or failed with an error(rc:0x%08X, ffdc:0x%08X%08X). " \
                        "CompletionState:0x%X.",
                         G_gpe_complete_pwr_meas_read_args.error.rc,
                         G_gpe_complete_pwr_meas_read_args.error.ffdc >> 32,
                         G_gpe_complete_pwr_meas_read_args.error.ffdc,
                         G_meas_complete_request.request.completion_state);

                // Collect FFDC and log error once.
                if (!L_ffdc_collected)
                {
                    errlHndl_t l_err = NULL;

                    /*
                     * @errortype
                     * @moduleid    PSS_MID_APSS_COMPLETE_MEAS
                     * @reasoncode  APSS_GPE_FAILURE
                     * @userdata1   GPE returned rc code
                     * @userdata2   0
                     * @userdata4   ERC_APSS_COMPLETE_FAILURE
                     * @devdesc     Failure getting power measurement data from APSS
                     */
                    l_err = createErrl(PSS_MID_APSS_COMPLETE_MEAS,   // i_modId
                                       APSS_GPE_FAILURE,          // i_reasonCode
                                       ERC_APSS_COMPLETE_FAILURE,
                                       ERRL_SEV_INFORMATIONAL,
                                       NULL,
                                       DEFAULT_TRACE_SIZE,
                                       G_gpe_complete_pwr_meas_read_args.error.rc,
                                       0);

                    addUsrDtlsToErrl(l_err,
                                     (uint8_t*)&G_meas_complete_request.ffdc,
                                     sizeof(G_meas_complete_request.ffdc),
                                     ERRL_STRUCT_VERSION_1,
                                     ERRL_USR_DTL_BINARY_DATA);

                    // Commit Error
                    commitErrl(&l_err);

                    // Set to true so that we don't log this error again.
                    L_ffdc_collected = TRUE;
                }
            }
        }

        // Clear these out prior to starting the GPE (GPE only sets them)
        G_gpe_complete_pwr_meas_read_args.error.error = 0;
        G_gpe_complete_pwr_meas_read_args.error.ffdc = 0;

        // Submit the next request
        l_rc = pore_flex_schedule(&G_meas_complete_request);
        if (0 != l_rc)
        {
            errlHndl_t l_err = NULL;

            TRAC_ERR("task_apss_complete_pwr_meas: schedule failed w/rc=0x%08X (%d us)", l_rc,
                     (int) ((ssx_timebase_get())/(SSX_TIMEBASE_FREQUENCY_HZ/1000000)));

            /*
             * @errortype
             * @moduleid    PSS_MID_APSS_COMPLETE_MEAS
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   GPE shedule returned rc code
             * @userdata2   0
             * @userdata4   ERC_APSS_SCHEDULE_FAILURE
             * @devdesc     task_apss_complete_pwr_meas schedule failed
             */
            l_err = createErrl(PSS_MID_APSS_COMPLETE_MEAS,
                               SSX_GENERIC_FAILURE,
                               ERC_APSS_SCHEDULE_FAILURE,
                               ERRL_SEV_PREDICTIVE,
                               NULL,
                               DEFAULT_TRACE_SIZE,
                               l_rc,
                               0);

            // Request reset since this should never happen.
            REQUEST_RESET(l_err);
            L_scheduled = FALSE;
            break;
        }

        L_scheduled = TRUE;


    }while (0);

    APSS_DBG("task_apss_complete_pwr_meas: finished w/rc=0x%08X\n", G_gpe_complete_pwr_meas_read_args.error.rc);
    APSS_DBG_HEXDUMP(&G_gpe_complete_pwr_meas_read_args, sizeof(G_gpe_complete_pwr_meas_read_args), "G_gpe_complete_pwr_meas_read_args");


} // end task_apss_complete_pwr_meas

bool apss_gpio_get(uint8_t i_pin_number, uint8_t *o_pin_value)
{
    bool l_valid = FALSE;

    if( (i_pin_number != SYSCFG_INVALID_PIN) && (o_pin_value != NULL) )
    {
        // Check if G_dcom_slv_inbox_rx is valid.
        // The default value is all 0, so check if it's no-zero
        bool l_dcom_data_valid = FALSE;
        int i=0;
        for(;i<sizeof(G_dcom_slv_inbox_rx);i++)
        {
            if( ((char*)&G_dcom_slv_inbox_rx)[i] != 0 )
            {
              l_dcom_data_valid = TRUE;
              break;
            }
        }

        if( l_dcom_data_valid == TRUE)
        {
            uint8_t l_gpio_port = i_pin_number/NUM_OF_APSS_PINS_PER_GPIO_PORT;
            uint8_t l_gpio_mask = 0x1 << i_pin_number % NUM_OF_APSS_PINS_PER_GPIO_PORT;
            l_valid = TRUE;
            if( G_dcom_slv_inbox_rx.gpio[l_gpio_port] & l_gpio_mask )
            {
                *o_pin_value = 1;
            }
            else
            {
                *o_pin_value = 0;
            }
        }
    }
    return l_valid;
}

