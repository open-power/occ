/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cmdh/cmdh_mnfg_intf.c $                           */
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

#include "cmdh_mnfg_intf.h"
#include "cmdh_service_codes.h"
#include "cmdh_fsp_cmds.h"
#include "dcom.h"
#include "amec_oversub.h"
#include "amec_sys.h"
#include "sensor_query_list.h"
#include "amec_smh.h"
#include "amec_master_smh.h"
#include "centaur_data.h"
#include "centaur_control.h"

extern task_t G_task_table[TASK_END];
extern centaur_throttle_t G_centaurThrottleLimits[MAX_NUM_CENTAURS][NUM_MBAS_PER_CENTAUR];

// Function Specification
//
// Name:  cmdh_mnfg_run_stop_slew
//
// Description: This function handles the manufacturing command to start
// or stop frequency autoslewing.
//
// End Function Specification
uint8_t cmdh_mnfg_run_stop_slew(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                      cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                     l_rc = ERRL_RC_SUCCESS;
    uint16_t                    l_fmin = 0;
    uint16_t                    l_fmax = 0;
    uint16_t                    l_step_size = 0;
    uint16_t                    l_step_delay = 0;
    uint32_t                    l_temp = 0;
    mnfg_run_stop_slew_cmd_t    *l_cmd_ptr = (mnfg_run_stop_slew_cmd_t*) i_cmd_ptr;
    mnfg_run_stop_slew_rsp_t    *l_rsp_ptr = (mnfg_run_stop_slew_rsp_t*) o_rsp_ptr;

    do
    {
        // This command is only supported on Master OCC
        if (G_occ_role == OCC_SLAVE)
        {
            TRAC_ERR("cmdh_mnfg_run_stop_slew: Mnfg command not supported on Slave OCCs!");
            break;
        }

        // Do some basic input verification
        if ((l_cmd_ptr->action > MNFG_INTF_SLEW_STOP) ||
            (l_cmd_ptr->step_mode > MNFG_INTF_FULL_SLEW))
        {
            // Invalid values were passed by the user!
            TRAC_ERR("cmdh_mnfg_run_stop_slew: Invalid values were detected! action[0x%02x] step_mode[0x%02x]",
                     l_cmd_ptr->action,
                     l_cmd_ptr->step_mode);
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }

        // Are we stopping the auto-slew function?
        if (l_cmd_ptr->action == MNFG_INTF_SLEW_STOP)
        {
            // Collect the slew count
            l_rsp_ptr->slew_count = AMEC_MST_CUR_SLEW_COUNT();

            // Collect the frequency range used for the auto-slew
            l_rsp_ptr->fstart = AMEC_MST_CUR_MNFG_FMIN();
            l_rsp_ptr->fstop = AMEC_MST_CUR_MNFG_FMAX();

            TRAC_INFO("cmdh_mnfg_run_stop_slew: Auto-slewing has been stopped. Count[%u] fstart[%u] fstop[%u]",
                      AMEC_MST_CUR_SLEW_COUNT(),
                      AMEC_MST_CUR_MNFG_FMIN(),
                      AMEC_MST_CUR_MNFG_FMAX());

            // Send a signal to RTL to stop auto-slewing
            AMEC_MST_STOP_AUTO_SLEW();

            // We are done
            break;
        }

        // If we made it here, that means we are starting up a slew run
        // First, determine the Fmax and Fmin for the slew run
        if (l_cmd_ptr->bottom_mode == OCC_MODE_PWRSAVE)
        {
            // If bottom mode is Static Power Save, use the min frequency
            // available
            l_fmin = G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY];
        }
        else
        {
            l_fmin = G_sysConfigData.sys_mode_freq.table[l_cmd_ptr->bottom_mode];
        }
        l_fmax = G_sysConfigData.sys_mode_freq.table[l_cmd_ptr->high_mode];

        // Add the percentages to compute the min/max frequencies
        l_fmin = l_fmin + (l_fmin * l_cmd_ptr->bottom_percent)/100;
        l_fmax = l_fmax + (l_fmax * l_cmd_ptr->high_percent)/100;

        TRAC_INFO("cmdh_mnfg_run_stop_slew: We are about to start auto-slewing function");
        TRAC_INFO("cmdh_mnfg_run_stop_slew: bottom_mode[0x%.2X] freq[%u] high_mode[0x%.2X] freq[%u]",
                  l_cmd_ptr->bottom_mode,
                  l_fmin,
                  l_cmd_ptr->high_mode,
                  l_fmax);

        // Determine the frequency step size and the step delay
        if (l_cmd_ptr->step_mode == MNFG_INTF_FULL_SLEW)
        {
            l_step_size = l_fmax - l_fmin;

            // Disable step delays if full slew mode has been selected
            l_step_delay = 0;

            TRAC_INFO("cmdh_mnfg_run_stop_slew: Enabling full-slew mode with step_size[%u] step_delay[%u]",
                      l_step_size,
                      l_step_delay);
        }
        else
        {
            l_step_size = 20;  // TODO: Need to find out the step size from Pstate table

            // Translate the step delay to internal OCC ticks
            l_temp = (l_cmd_ptr->step_delay * 1000) / AMEC_US_PER_TICK;
            l_step_delay = (uint16_t) l_temp;

            TRAC_INFO("cmdh_mnfg_run_stop_slew: Enabling single-step mode with step_size[%u] step_delay[%u]",
                      l_step_size,
                      l_step_delay);
        }

        // Now, load the values for RTL consumption
        AMEC_MST_SET_MNFG_FMIN(l_fmin);
        AMEC_MST_SET_MNFG_FMAX(l_fmax);
        AMEC_MST_SET_MNFG_FSTEP(l_step_size);
        AMEC_MST_SET_MNFG_DELAY(l_step_delay);

        // Reset the slew-counter before we start auto-slewing
        AMEC_MST_CUR_SLEW_COUNT() = 0;

        // Wait a little bit for RTL to process above parameters
        ssx_sleep(SSX_MILLISECONDS(5));

        // Send a signal to RTL to start auto-slewing
        AMEC_MST_START_AUTO_SLEW();

        // We are auto-slewing now, populate the response packet
        l_rsp_ptr->slew_count = 0;
        l_rsp_ptr->fstart = l_fmin;
        l_rsp_ptr->fstop = l_fmax;

    }while(0);

    // Populate the response data packet
    G_rsp_status = l_rc;
    l_rsp_ptr->data_length[0] = 0;
    l_rsp_ptr->data_length[1] = MNFG_INTF_RUN_STOP_SLEW_RSP_SIZE;

    return l_rc;
}

// Function Specification
//
// Name:  cmdh_mnfg_mem_slew
//
// Description: This function handles the manufacturing command to start
// or stop memory autoslewing.
//
// End Function Specification
uint8_t cmdh_mnfg_mem_slew(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                 cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                l_rc = ERRL_RC_SUCCESS;
    mnfg_mem_slew_cmd_t    *l_cmd_ptr = (mnfg_mem_slew_cmd_t*) i_cmd_ptr;
    mnfg_mem_slew_rsp_t    *l_rsp_ptr = (mnfg_mem_slew_rsp_t*) o_rsp_ptr;

    do
    {

        // Do some basic input verification
        if (l_cmd_ptr->action > MNFG_INTF_SLEW_STOP)
        {
            // Invalid values were passed by the user!
            TRAC_ERR("cmdh_mnfg_mem_slew: Invalid value was detected! action[0x%02x]",
                     l_cmd_ptr->action);
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }

        // Are we stopping the auto-slew function?
        if (l_cmd_ptr->action == MNFG_INTF_SLEW_STOP)
        {
            // Send a signal to RTL to stop auto-slewing
            g_amec->mnfg_parms.mem_autoslew = FALSE;

            // Collect the slew count
            if(g_amec->mnfg_parms.mem_slew_counter > 0x0000FFFF)
            {
                l_rsp_ptr->slew_count = 0xFFFF;
            }
            else
            {
                l_rsp_ptr->slew_count = g_amec->mnfg_parms.mem_slew_counter;
            }

            // Zero out the slew count;
            g_amec->mnfg_parms.mem_slew_counter = 0;

            TRAC_INFO("cmdh_mnfg_mem_slew: Auto-slewing has been stopped. Count[%u]",
                      l_rsp_ptr->slew_count);

            // We are done
            break;
        }

        // If we made it here, that means we are starting up a slew run
        TRAC_INFO("cmdh_mnfg_mem_slew: We are about to start auto-slewing function");

// TEMP -- NOT SUPPORTED YET IN PHASE1
// when implementing see dimm/dimm.c - memory_init()
/*
        // Force activation of memory monitoring and control
        if(!rtl_task_is_runnable(TASK_ID_CENTAUR_CONTROL))
        {
            uint32_t l_cent, l_mba;

            // Only run initialization on an active OCC
            if(!IS_OCC_STATE_ACTIVE())
            {
                TRAC_ERR("cmdh_mnfg_mem_slew: OCC must be active to start mem slewing");
                l_rc = ERRL_RC_INVALID_STATE;
                break;
            }

            // Force all MBA's to be present
            G_configured_mbas = -1;

            TRAC_INFO("cmdh_mnfg_mem_slew: calling centaur_init()");
            centaur_init(); //no rc, handles errors internally

            // Check if centaur_init resulted in a reset
            // since we don't have a return code from centaur_init.
            if(isSafeStateRequested())
            {
                TRAC_ERR("cmdh_mnfg_mem_slew: OCC is being reset");
                l_rc = ERRL_RC_INTERNAL_FAIL;
                break;
            }

            for(l_cent = 0; l_cent < MAX_NUM_CENTAURS; l_cent++)
            {
                if(!CENTAUR_PRESENT(l_cent))
                {
                    continue;
                }

                for(l_mba = 0; l_mba < NUM_MBAS_PER_CENTAUR; l_mba++)
                {
                    mem_throt_config_data_t * l_throt_ptr =
                                   &G_sysConfigData.mem_throt_limits[l_cent][l_mba];

                    // Uses values seen on tuleta as defaults
                    l_throt_ptr->min_n_per_mba = 13;
                    l_throt_ptr->nom_n_per_mba = 72;
                    l_throt_ptr->nom_n_per_chip = 72;
                    l_throt_ptr->pcap1_n_per_mba = 72;
                    l_throt_ptr->pcap2_n_per_mba = 72;
                    l_throt_ptr->ovs_n_per_mba = 72;
                    l_throt_ptr->ovs_n_per_chip = 72;
                }

            }


            // Initialization was successful.
            // Set task flags to allow centaur control task to run and
            // also to prevent us from doing initialization again.
            G_task_table[TASK_ID_CENTAUR_DATA].flags = CENTAUR_DATA_RTL_FLAGS;
            G_task_table[TASK_ID_CENTAUR_CONTROL].flags = CENTAUR_CONTROL_RTL_FLAGS;
        }
*/

        // Zero out the slew count
        g_amec->mnfg_parms.mem_slew_counter = 0;

        // Send a signal to RTL to start memory auto-slewing
        g_amec->mnfg_parms.mem_autoslew = TRUE;

        // We are auto-slewing now, populate the response packet
        l_rsp_ptr->slew_count = 0;

        TRAC_INFO("cmdh_mnfg_mem_slew: memory slewing started.");

    }while(0);

    // Populate the response data packet
    G_rsp_status = l_rc;
    l_rsp_ptr->data_length[0] = 0;
    l_rsp_ptr->data_length[1] = MNFG_INTF_MEM_SLEW_RSP_SIZE;

    return l_rc;
}


// Function Specification
//
// Name:  cmdh_mnfg_emulate_oversub
//
// Description: This function handles the manufacturing command to emulate
// oversubscription.
//
// End Function Specification
uint8_t cmdh_mnfg_emulate_oversub(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                        cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                     l_rc = 0;
    mnfg_emul_oversub_cmd_t     *l_cmd_ptr = (mnfg_emul_oversub_cmd_t*) i_cmd_ptr;
    mnfg_emul_oversub_rsp_t     *l_rsp_ptr = (mnfg_emul_oversub_rsp_t*) o_rsp_ptr;

    do
    {
        // This command is only supported on Master OCC
        if (G_occ_role == OCC_SLAVE)
        {
            TRAC_ERR("cmdh_mnfg_emulate_oversub: Mnfg command not supported on Slave OCCs!");
            break;
        }

        switch (l_cmd_ptr->action)
        {
            case 0x00:
                TRAC_INFO("cmdh_mnfg_emulate_oversub: Disable oversubscription emulation");
                AMEC_INTF_GET_OVERSUBSCRIPTION_EMULATION() = 0;
                l_rsp_ptr->state = l_cmd_ptr->action;
                break;

            case 0x01:
                TRAC_INFO("cmdh_mnfg_emulate_oversub: Enable oversubscription emulation");
                AMEC_INTF_GET_OVERSUBSCRIPTION_EMULATION() = 1;
                l_rsp_ptr->state = l_cmd_ptr->action;
                break;

            case 0xFF:
                TRAC_INFO("cmdh_mnfg_emulate_oversub: Query oversubscription emulation");
                l_rsp_ptr->state = AMEC_INTF_GET_OVERSUBSCRIPTION_EMULATION();
                break;

            default:
                TRAC_INFO("cmdh_mnfg_emulate_oversub: Invalid oversubscription emulation action");
                l_rsp_ptr->state = AMEC_INTF_GET_OVERSUBSCRIPTION_EMULATION();
                break;
        }

    }while(0);

    // Populate the response data packet
    G_rsp_status = ERRL_RC_SUCCESS;
    l_rsp_ptr->data_length[0] = 0;
    l_rsp_ptr->data_length[1] = 1;

    return l_rc;
}

// Function Specification
//
// Name:  cmdh_mnfg_list_sensors
//
// Description: Returns a list of selected sensors
//
// End Function Specification
uint8_t cmdh_mnfg_list_sensors(const cmdh_fsp_cmd_t * i_cmd_ptr,
                           cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                         l_rc = ERRL_RC_SUCCESS;
    uint16_t                        l_type = 0;
    uint16_t                        l_location = 0;
    uint16_t                        l_start_gsid;
    uint16_t                        i = 0;
    uint16_t                        l_resp_data_length = 0;
    uint16_t                        l_datalength;
    uint16_t                        l_num_of_sensors = MFG_MAX_NUM_SENSORS + 1;
    cmdh_mfg_list_sensors_query_t   *l_cmd_ptr =
                                    (cmdh_mfg_list_sensors_query_t*) i_cmd_ptr;
    cmdh_mfg_list_sensors_resp_t    *l_resp_ptr =
                                    (cmdh_mfg_list_sensors_resp_t*) o_rsp_ptr;
    sensorQueryList_t               l_sensor_list[MFG_MAX_NUM_SENSORS + 1];
    errlHndl_t                      l_err = NULL;

    do
    {
        // Do sanity check on the function inputs
        if ((NULL == i_cmd_ptr) || (NULL == o_rsp_ptr))
        {
// TODO: THESE TRACES NEED TO BE VERIFIED
            TRAC_ERR("cmdh_mnfg_list_sensors: invalid pointers. cmd[0x%08x] rsp[0x%08x]",
                     (uint32_t) i_cmd_ptr, (uint32_t) o_rsp_ptr);
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }

        // Check packet data length
        l_datalength = CMDH_DATALEN_FIELD_UINT16(i_cmd_ptr);
        if(l_datalength < (sizeof(cmdh_mfg_list_sensors_query_t) -
                          sizeof(cmdh_fsp_cmd_header_t)))
        {
            TRAC_ERR("cmdh_mnfg_list_sensors: incorrect data length. exp[%d] act[%d]",
                     (sizeof(cmdh_mfg_list_sensors_query_t) -
                      sizeof(cmdh_fsp_cmd_header_t)),
                     l_datalength);
            l_rc = ERRL_RC_INVALID_CMD_LEN;
            break;
        }

        // Check version
        if(l_cmd_ptr->version != MFG_LIST_SENSOR_VERSION)
        {
            TRAC_ERR("cmdh_mnfg_list_sensors: incorrect version. exp[%d] act[%d]",
                     MFG_LIST_SENSOR_VERSION,
                     l_cmd_ptr->version);
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }

        // Capture user inputs
        l_type = l_cmd_ptr->type;
        l_location = l_cmd_ptr->location;
        l_start_gsid = l_cmd_ptr->start_gsid;

        TRAC_INFO("cmdh_mnfg_list_sensors: Type[0x%04x] Location[0x%04x]",
                  l_type,
                  l_location);

        // Initialize the sensor query arguments
        const querySensorListArg_t l_qsl_arg =
        {
            l_start_gsid,           // i_startGsid - passed by the caller
            l_cmd_ptr->present,     // i_present - passed by the caller
            l_type,                 // i_type - passed by the caller
            l_location,             // i_loc - passed by the caller
            &l_num_of_sensors,      // io_numOfSensors
            l_sensor_list,          // o_sensors
            NULL                    // o_sensorInfoPtr - not needed
        };

        // Get the list of sensors
        l_err = querySensorList(&l_qsl_arg);

        if (NULL != l_err)
        {
            // Query failure
            TRAC_ERR("cmdh_mnfg_list_sensors: Failed to query sensor list. Error status is: 0x%x",
                     l_err->iv_reasonCode);

            // Commit error log
            commitErrl(&l_err);
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }
        else
        {
            TRAC_INFO("cmdh_mnfg_list_sensors: Numbers of sensors found[%u]",
                      l_num_of_sensors);

            if (l_num_of_sensors > MFG_MAX_NUM_SENSORS)
            {
                // Got too many sensors back, need to truncate the list
                TRAC_INFO("cmdh_mnfg_list_sensors: Got too many sensors back[%u]. Truncating number of sensors to %u",
                          l_num_of_sensors,
                          MFG_MAX_NUM_SENSORS);

                l_num_of_sensors = MFG_MAX_NUM_SENSORS;
                l_resp_ptr->truncated = 1;
            }
            else
            {
                l_resp_ptr->truncated = 0;
            }

            // Clear out the sensor fields
            memset((void*) &(l_resp_ptr->sensor[0]), 0, (sizeof(cmdh_dbug_sensor_list_t)*l_num_of_sensors) );

            // Populate the response data packet
            l_resp_ptr->num_sensors = l_num_of_sensors;
            for (i=0; i<l_num_of_sensors; i++)
            {
                l_resp_ptr->sensor[i].gsid = l_sensor_list[i].gsid;
                l_resp_ptr->sensor[i].sample = l_sensor_list[i].sample;
                strcpy(l_resp_ptr->sensor[i].name, l_sensor_list[i].name);
            }
        }

    }while(0);

    // Populate the response data header
    l_resp_data_length = 2 + l_num_of_sensors * sizeof(cmdh_mfg_sensor_rec_t);
    G_rsp_status = l_rc;
    o_rsp_ptr->data_length[0] = ((uint8_t *)&l_resp_data_length)[0];
    o_rsp_ptr->data_length[1] = ((uint8_t *)&l_resp_data_length)[1];

    return l_rc;
}


// Function Specification
//
// Name:  cmdh_mnfg_get_sensor
//
// Description: Returns a list of selected sensors
//
// End Function Specification
uint8_t cmdh_mnfg_get_sensor(const cmdh_fsp_cmd_t * i_cmd_ptr,
                             cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                         l_rc = ERRL_RC_SUCCESS;
    uint16_t                        l_gsid;
    uint16_t                        l_resp_data_length = 0;
    uint16_t                        l_datalength;
    uint16_t                        l_num_of_sensors = 1;
    cmdh_mfg_get_sensor_query_t     *l_cmd_ptr =
                                    (cmdh_mfg_get_sensor_query_t*) i_cmd_ptr;
    cmdh_mfg_get_sensor_resp_t      *l_resp_ptr =
                                    (cmdh_mfg_get_sensor_resp_t*) o_rsp_ptr;
    sensor_info_t                   l_sensor_info;
    errlHndl_t                      l_err = NULL;
    sensor_t*                       l_sensor_ptr;

    do
    {
        // Do sanity check on the function inputs
        if ((NULL == i_cmd_ptr) || (NULL == o_rsp_ptr))
        {
// TODO: THESE TRACES NEED TO BE VERIFIED
            TRAC_ERR("cmdh_mnfg_get_sensor: invalid pointers. cmd[0x%08x] rsp[0x%08x]",
                     (uint32_t) i_cmd_ptr, (uint32_t) o_rsp_ptr);
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }

        // Check packet data length
        l_datalength = CMDH_DATALEN_FIELD_UINT16(i_cmd_ptr);
        if(l_datalength < (sizeof(cmdh_mfg_get_sensor_query_t) -
                          sizeof(cmdh_fsp_cmd_header_t)))
        {
            TRAC_ERR("cmdh_mnfg_get_sensor: incorrect data length. exp[%d] act[%d]",
                     (sizeof(cmdh_mfg_get_sensor_query_t) -
                      sizeof(cmdh_fsp_cmd_header_t)),
                     l_datalength);
            l_rc = ERRL_RC_INVALID_CMD_LEN;
            break;
        }

        // Check version
        if(l_cmd_ptr->version != MFG_LIST_SENSOR_VERSION)
        {
            TRAC_ERR("cmdh_mnfg_get_sensor: incorrect version. exp[%d] act[%d]",
                     MFG_GET_SENSOR_VERSION,
                     l_cmd_ptr->version);
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }

        // Capture user inputs
        l_gsid = l_cmd_ptr->gsid;

        TRAC_INFO("cmdh_mnfg_get_sensor: gsid[0x%04x]", l_gsid);

        // Initialize the sensor query arguments
        querySensorListArg_t l_qsl_arg =
        {
            l_gsid,                 // i_startGsid - passed by the caller
            0,                      // i_present - passed by the caller
            AMEC_SENSOR_TYPE_ALL,   // i_type
            AMEC_SENSOR_LOC_ALL,    // i_loc
            &l_num_of_sensors,      // io_numOfSensors
            NULL,                   // o_sensors - not needed
            &l_sensor_info          // o_sensorInfoPtr
        };

        // Get the sensor list
        l_err = querySensorList(&l_qsl_arg);

        if (NULL != l_err)
        {
            // Query failure
            TRAC_ERR("cmdh_mnfg_get_sensor: Failed to get sensor list. Error status is: 0x%x",
                     l_err->iv_reasonCode);

            // Commit error log
            commitErrl(&l_err);
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }
        else
        {
            l_resp_ptr->gsid = l_gsid;

            // Some of the response comes from the sensor
            l_sensor_ptr = getSensorByGsid(l_gsid);
            if (l_sensor_ptr == NULL)
            {
                TRAC_INFO("cmdh_mnfg_get_sensor: Didn't find sensor with gsid[0x%.4X]. Min/Max values won't be accurate.",
                          l_gsid);
                l_resp_ptr->sample = 0;
                l_resp_ptr->min = 0xFFFF;
                l_resp_ptr->max = 0;
                l_resp_ptr->accumulator = 0;
                l_resp_ptr->status = 0;
            }
            else
            {
                l_resp_ptr->sample = l_sensor_ptr->sample;
                l_resp_ptr->min = l_sensor_ptr->sample_min;
                l_resp_ptr->max = l_sensor_ptr->sample_max;
                // Truncate accumulator to 4 bytes (should not be used)
                l_resp_ptr->accumulator = (uint32_t)l_sensor_ptr->accumulator;
                l_resp_ptr->status = *(uint8_t*)(&l_sensor_ptr->status);
            }

            // The rest of the response comes from the sensor info
            memcpy(l_resp_ptr->name, l_sensor_info.name, sizeof(l_resp_ptr->name));
            memcpy(l_resp_ptr->units, l_sensor_info.sensor.units, sizeof(l_resp_ptr->units));
            l_resp_ptr->freq = l_sensor_info.sensor.freq;
            l_resp_ptr->scalefactor = l_sensor_info.sensor.scalefactor;
            l_resp_ptr->location = l_sensor_info.sensor.location;
            l_resp_ptr->type = l_sensor_info.sensor.type;
        }

    }while(0);

    // Populate the response data header
    l_resp_data_length = sizeof(cmdh_mfg_get_sensor_resp_t) -
                         sizeof(cmdh_fsp_rsp_header_t);
    G_rsp_status = l_rc;
    o_rsp_ptr->data_length[0] = ((uint8_t *)&l_resp_data_length)[0];
    o_rsp_ptr->data_length[1] = ((uint8_t *)&l_resp_data_length)[1];

    return l_rc;
}

// Function Specification
//
// Name:  cmdh_mnfg_test_parse
//
// Description:  This function parses the manufacturing commands sent via TMGT.
//
// End Function Specification
void cmdh_mnfg_test_parse (const cmdh_fsp_cmd_t * i_cmd_ptr,
                                 cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                     l_rc = 0;
    uint8_t                     l_sub_cmd = 0;
    errlHndl_t                  l_errl = NULL;

    // Sub-command is always first byte of data
    l_sub_cmd = i_cmd_ptr->data[0];

    TRAC_INFO("cmdh_mnfg_test_parse: Mnfg sub-command [0x%02x]", l_sub_cmd);

    switch (l_sub_cmd)
    {
        case MNFG_RUN_STOP_SLEW:
            l_rc = cmdh_mnfg_run_stop_slew(i_cmd_ptr, o_rsp_ptr);
            break;

        case MNFG_OVERSUB_EMULATION:
            l_rc = cmdh_mnfg_emulate_oversub(i_cmd_ptr, o_rsp_ptr);
            break;

        case MNFG_LIST_SENSORS:
            l_rc = cmdh_mnfg_list_sensors(i_cmd_ptr, o_rsp_ptr);
            break;

        case MNFG_GET_SENSOR:
            l_rc = cmdh_mnfg_get_sensor(i_cmd_ptr, o_rsp_ptr);
            break;

        case MNFG_MEMORY_SLEW:
            l_rc = cmdh_mnfg_mem_slew(i_cmd_ptr, o_rsp_ptr);
            break;

        case MNFG_RETRIEVE_EAR:
        case MNFG_SET_FMINMAX:
        case MNFG_CPM_STRESS_CALI:
        case MNFG_UV_CONTROL:
        case MNFG_FCHECK_CONTROL:
        default:
            // Should never get here...
            l_rc = ERRL_RC_INVALID_DATA;
            break;
    }

    // All errors in MNFG logged internally
    if (l_rc)
    {
        TRAC_ERR("Mfg command 0x%02x failed with rc = %d", l_sub_cmd, l_rc);
        // Build Error Response packet
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, l_rc, &l_errl);
    }

    return;
}

