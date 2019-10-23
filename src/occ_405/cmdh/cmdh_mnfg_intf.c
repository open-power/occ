/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cmdh/cmdh_mnfg_intf.c $                           */
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

#include "cmdh_mnfg_intf.h"
#include "cmdh_service_codes.h"
#include "cmdh_fsp_cmds.h"
#include "dcom.h"
#include "amec_oversub.h"
#include "amec_sys.h"
#include "sensor_query_list.h"
#include "amec_smh.h"
#include "amec_master_smh.h"
#include <pgpe_shared.h>

// SSX Block Copy Request for copying mfg Pstate table from HOMER to SRAM
BceRequest G_mfg_pba_request;
DMA_BUFFER(mfg_read_pstate_table_t G_mfg_read_pstate_table) = {{0}};

extern task_t G_task_table[TASK_END];

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
            l_step_size = (uint16_t)G_mhz_per_pstate;

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

        // If the OCC is active (we can only run auto-slew in active state) the memory control
        // task must be running and there is no support (or need) to force activation of
        // memory monitoring and control

        if(!IS_OCC_STATE_ACTIVE())
        {
            TRAC_ERR("cmdh_mnfg_mem_slew: OCC must be active to start mem slewing");
            l_rc = ERRL_RC_INVALID_STATE;
            break;
        }
        if(!rtl_task_is_runnable(TASK_ID_MEMORY_CONTROL))
        {
            TRAC_ERR("cmdh_mnfg_mem_slew: memory control task not running");
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }

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
    uint16_t                        l_resp_data_length = 0;
    uint16_t                        l_datalength;
    uint16_t                        l_num_of_sensors = MFG_MAX_NUM_SENSORS + 1;
    cmdh_mfg_list_sensors_query_t   *l_cmd_ptr =
                                    (cmdh_mfg_list_sensors_query_t*) i_cmd_ptr;
    cmdh_mfg_list_sensors_resp_t    *l_resp_ptr =
                                    (cmdh_mfg_list_sensors_resp_t*) o_rsp_ptr;
    errlHndl_t                      l_err = NULL;

    do
    {
        // Do sanity check on the function inputs
        if ((NULL == i_cmd_ptr) || (NULL == o_rsp_ptr))
        {
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

        // Clear out the sensor fields in response buffer
        memset((void*) &(l_resp_ptr->sensor[0]), 0, (sizeof(cmdh_dbug_sensor_list_t)*MFG_MAX_NUM_SENSORS) );

        // Initialize the sensor query arguments
        // To take advantage of full 4K response buffer copy directly to response buffer
        // Cannot use a local to copy into first as stack size limits creating a local buffer of 4K
        const querySensorListArg_t l_qsl_arg =
        {
            l_start_gsid,                               // i_startGsid - passed by the caller
            l_cmd_ptr->present,                         // i_present - passed by the caller
            l_type,                                     // i_type - passed by the caller
            l_location,                                 // i_loc - passed by the caller
            &l_num_of_sensors,                          // io_numOfSensors
            (sensorQueryList_t*)&l_resp_ptr->sensor[0], // o_sensors
            NULL                                        // o_sensorInfoPtr - not needed
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
            l_num_of_sensors = 0;
            break;
        }
        else
        {
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
                TRAC_INFO("cmdh_mnfg_list_sensors: Numbers of sensors found[%u]",
                          l_num_of_sensors);

                l_resp_ptr->truncated = 0;
            }
        }

    }while(0);

    // Populate the response data header
    l_resp_ptr->num_sensors = l_num_of_sensors;
    l_resp_data_length = 2 + l_num_of_sensors * sizeof(sensorQueryList_t);
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
// Name:  cmdh_mnfg_request_quad_pstate
//
// Description: This function handles the manufacturing command to request
// a Pstate per Quad.
//
// End Function Specification
uint8_t cmdh_mnfg_request_quad_pstate(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                            cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                     l_rc = ERRL_RC_SUCCESS;
    uint16_t                    l_datalength = 0;
    uint16_t                    l_resp_data_length = 0;
    uint8_t                     l_pmin = 0xFF;
    uint8_t                     l_pmax = 0xFF;
    uint8_t                     l_pstate_request = 0xFF;
    uint8_t                     l_quad = 0;
    mnfg_quad_pstate_cmd_t     *l_cmd_ptr = (mnfg_quad_pstate_cmd_t*) i_cmd_ptr;
    mnfg_quad_pstate_rsp_t     *l_rsp_ptr = (mnfg_quad_pstate_rsp_t*) o_rsp_ptr;

    do
    {
        if(!IS_OCC_STATE_ACTIVE())
        {
            TRAC_ERR("cmdh_mnfg_request_quad_pstate: OCC must be active to request pstate");
            l_rc = ERRL_RC_INVALID_STATE;
            break;
        }

        if(G_sysConfigData.system_type.kvm)
        {
            TRAC_ERR("cmdh_mnfg_request_quad_pstate: Must be PowerVM to request pstate");
            l_rc = ERRL_RC_INVALID_CMD;
            break;
        }

        // Check command packet data length
        l_datalength = CMDH_DATALEN_FIELD_UINT16(i_cmd_ptr);
        if(l_datalength != (sizeof(mnfg_quad_pstate_cmd_t) -
                            sizeof(cmdh_fsp_cmd_header_t)))
        {
            TRAC_ERR("cmdh_mnfg_request_quad_pstate: incorrect data length. exp[%d] act[%d]",
                     (sizeof(mnfg_quad_pstate_cmd_t) -
                      sizeof(cmdh_fsp_cmd_header_t)),
                      l_datalength);
            l_rc = ERRL_RC_INVALID_CMD_LEN;
            break;
        }

        // Check version
        if(l_cmd_ptr->version != MFG_QUAD_PSTATE_VERSION)
        {
            TRAC_ERR("cmdh_mnfg_request_quad_pstate: incorrect version. exp[%d] act[%d]",
                     MFG_QUAD_PSTATE_VERSION,
                     l_cmd_ptr->version);
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }

        // only allow a Pstate within the current range based on mode
        l_pmin = proc_freq2pstate(g_amec->sys.fmin);
        l_pmax = proc_freq2pstate(g_amec->sys.fmax);

        // Process each quad Pstate request, clip any request to min/max
        // 0xFF has special meaning that OCC is in control
        for(l_quad = 0; l_quad < MAXIMUM_QUADS; l_quad++)
        {
            l_pstate_request = l_cmd_ptr->quad_pstate_in[l_quad];
            if(l_pstate_request != 0xFF)
            {
                // pmin is lowest frequency corresponding to highest pState value
                if(l_pstate_request > l_pmin)
                   l_pstate_request = l_pmin;

                // pmax is highest frequency corresponding to lowest pState value
                else if(l_pstate_request < l_pmax)
                   l_pstate_request = l_pmax;
            }
            // save the quad pState request for amec and return in rsp data
            g_amec->mnfg_parms.quad_pstate[l_quad] = l_pstate_request;
            l_rsp_ptr->quad_pstate_out[l_quad] = l_pstate_request;
            TRAC_INFO("cmdh_mnfg_request_quad_pstate: Quad %d Pstate in = 0x%02x Pstate out = 0x%02x",
                      l_quad,
                      l_cmd_ptr->quad_pstate_in[l_quad],
                      l_rsp_ptr->quad_pstate_out[l_quad]);
        }

    }while(0);

    // Populate the response data header
    G_rsp_status = l_rc;
    l_resp_data_length = sizeof(mnfg_quad_pstate_rsp_t) - sizeof(cmdh_fsp_rsp_header_t);
    l_rsp_ptr->data_length[0] = ((uint8_t *)&l_resp_data_length)[0];
    l_rsp_ptr->data_length[1] = ((uint8_t *)&l_resp_data_length)[1];

    return l_rc;
}

// Function Specification
//
// Name:  cmdh_mnfg_read_pstate_table
//
// Description: This function handles the manufacturing command to read
// the generated Pstate table from main memory 3K blocks at a time
//
// End Function Specification
uint8_t cmdh_mnfg_read_pstate_table(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                            cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                       l_rc = ERRL_RC_SUCCESS;
    uint16_t                      l_datalength = 0;
    uint16_t                      l_resp_data_length = 0;
    uint32_t                      block_offset = 0;
    uint32_t                      main_mem_address = 0;
    int                           l_ssxrc = SSX_OK;
    mnfg_read_pstate_table_cmd_t  *l_cmd_ptr = (mnfg_read_pstate_table_cmd_t*) i_cmd_ptr;

    do
    {
        // Check command packet data length
        l_datalength = CMDH_DATALEN_FIELD_UINT16(i_cmd_ptr);
        if(l_datalength != (sizeof(mnfg_read_pstate_table_cmd_t) -
                            sizeof(cmdh_fsp_cmd_header_t)))
        {
            TRAC_ERR("cmdh_mnfg_read_pstate_table: incorrect data length. exp[%d] act[%d]",
                     (sizeof(mnfg_read_pstate_table_cmd_t) -
                      sizeof(cmdh_fsp_cmd_header_t)),
                      l_datalength);
            l_rc = ERRL_RC_INVALID_CMD_LEN;
            break;
        }

        // Process request
        if(l_cmd_ptr->request == MFG_PSTATE_READ_REQUEST_QUERY)
        {
            memcpy(&o_rsp_ptr->data[0], &G_pgpe_header.generated_pstate_table_homer_offset, 4);
            memcpy(&o_rsp_ptr->data[4], &G_pgpe_header.generated_pstate_table_length, 4);
            l_resp_data_length = MFG_PSTATE_READ_QUERY_RSP_SIZE;

            TRAC_INFO("cmdh_mnfg_read_pstate_table: Query table memory offset[0x%08x] table length[%d]",
                     G_pgpe_header.generated_pstate_table_homer_offset,
                     G_pgpe_header.generated_pstate_table_length);
            break;
        }

        // Calculate the starting main memory address for block to read
        block_offset = MFG_PSTATE_READ_MAX_RSP_SIZE * l_cmd_ptr->request;
        if(block_offset > G_pgpe_header.generated_pstate_table_length)
        {
            TRAC_ERR("cmdh_mnfg_read_pstate_table: Block request %d out of range.  Pstate Table size %d",
                     l_cmd_ptr->request,
                     G_pgpe_header.generated_pstate_table_length);
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }

        main_mem_address = G_pgpe_header.generated_pstate_table_homer_offset + block_offset;

        // Copy Pstate table from main memory to SRAM
        // Set up a copy request
        l_ssxrc = bce_request_create(&G_mfg_pba_request,                  // block copy object
                                     &G_pba_bcde_queue,                   // mainstore to sram copy engine
                                     main_mem_address,                    // mainstore address
                                     (uint32_t)&G_mfg_read_pstate_table,  // sram starting address
                                     sizeof(mfg_read_pstate_table_t),     // size of copy
                                     SSX_SECONDS(1),                      // timeout
                                     NULL,                                // no call back
                                     NULL,                                // no call back arguments
                                     ASYNC_REQUEST_BLOCKING);             // blocking request

        if(l_ssxrc != SSX_OK)
        {
            TRAC_ERR("cmdh_mnfg_read_pstate_table: BCDE request create failure rc=[%08X]", -l_ssxrc);
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }

        // Do actual copying
        l_ssxrc = bce_request_schedule(&G_mfg_pba_request);

        if(l_ssxrc != SSX_OK)
        {
            TRAC_ERR("cmdh_mnfg_read_pstate_table: BCE request schedule failure rc=[%08X]", -l_ssxrc);
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }

        // Determine the rsp data length
        l_resp_data_length = MFG_PSTATE_READ_MAX_RSP_SIZE;

        if((block_offset + MFG_PSTATE_READ_MAX_RSP_SIZE) > G_pgpe_header.generated_pstate_table_length)
        {
            l_resp_data_length = G_pgpe_header.generated_pstate_table_length - block_offset;
        }

        // Copy to response buffer
        memcpy(o_rsp_ptr->data,
               &G_mfg_read_pstate_table,
               l_resp_data_length);

        TRAC_INFO("cmdh_mnfg_read_pstate_table: Read from main memory[0x%08x] block offset[%d] length[%d]",
                   main_mem_address,
                   block_offset,
                   l_resp_data_length);
    }while(0);

    // Populate the response data header
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
errlHndl_t cmdh_mnfg_test_parse (const cmdh_fsp_cmd_t * i_cmd_ptr,
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

        case MNFG_QUAD_PSTATE:
            l_rc = cmdh_mnfg_request_quad_pstate(i_cmd_ptr, o_rsp_ptr);
            break;

        case MNFG_READ_PSTATE_TABLE:
            l_rc = cmdh_mnfg_read_pstate_table(i_cmd_ptr, o_rsp_ptr);
            break;

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

    return l_errl;
}


