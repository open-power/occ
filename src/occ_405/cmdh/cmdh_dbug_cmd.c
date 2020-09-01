/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cmdh/cmdh_dbug_cmd.c $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2020                        */
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
#include <occ_service_codes.h>  // For reason code
#include <errl.h>               // For error handle
#include <trac.h>               // For traces
#include <state.h>
#include <cmdh_dbug_cmd.h>
#include <cmdh_fsp.h>
#include <cmdh_fsp_cmds.h>
#include <memory.h>
#include <memory_data.h>
#include <proc_data.h>
#include <apss.h>
#include <gpe_export.h>
#include "sensor_query_list.h"
#include "amec_sys.h"
#include "dcom.h"
#include "chom.h"
#include "wof.h"
#include "gpu.h"
#include "pstates_occ.H"

//*************************************************************************/
// Externs
//*************************************************************************/
extern gpe_shared_data_t G_shared_gpe_data;
extern OCCPstateParmBlock_t G_oppb;

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
uint16_t G_allow_trace_flags = 0x0000;
uint32_t G_internal_flags    = INT_FLAG_DISABLE_OC_WOF_OFF | INT_FLAG_DISABLE_24X7;
extern bool G_smf_mode;
uint64_t G_inject_dimm = 0;
uint32_t G_inject_dimm_trace[MAX_NUM_OCMBS][NUM_DIMMS_PER_OCMB] = {{0}};

// SSX Block Copy Request for copying data from HOMER to SRAM
BceRequest G_debug_pba_request;
DMA_BUFFER(uint8_t G_debug_dma_buffer[CMDH_DEBUG_DMA_READ_SIZE]) = {0};

// list too large for stack, must be defined in memory
sensorQueryList_t G_sensor_list[CMDH_DBUG_MAX_NUM_SENSORS];

//*************************************************************************/
// Function Prototypes
//*************************************************************************/

//*************************************************************************/
// Functions
//*************************************************************************/

// Function Specification
//
// Name:  cmdh_dbug_get_trace
//
// Description: Process get trace command
//
// End Function Specification
void cmdh_dbug_get_trace (const cmdh_fsp_cmd_t * i_cmd_ptr,
                          cmdh_fsp_rsp_t * o_rsp_ptr)
{
    UINT l_rc = 0;
    UINT l_trace_buffer_size = CMDH_FSP_RSP_DATA_SIZE;
    UINT16 l_trace_size = 0;
    cmdh_dbug_get_trace_query_t *l_get_trace_query_ptr = (cmdh_dbug_get_trace_query_t*) i_cmd_ptr;
    cmdh_dbug_get_trace_resp_t *l_get_trace_resp_ptr = (cmdh_dbug_get_trace_resp_t*) o_rsp_ptr;

    if (memcmp((char *)l_get_trace_query_ptr->comp, "GP", 2) == 0)
    {
        // Return a GPE0/GPE1 trace buffer
        if (l_get_trace_query_ptr->comp[2] == '0')
        {
            if (G_shared_gpe_data.gpe0_tb_ptr != 0)
            {
                l_trace_size = G_shared_gpe_data.gpe0_tb_sz;
                memcpy(l_get_trace_resp_ptr->data, (uint8_t*)G_shared_gpe_data.gpe0_tb_ptr, (size_t)l_trace_size);
            }
        }
        else if (l_get_trace_query_ptr->comp[2] == '1')
        {
            if (G_shared_gpe_data.gpe0_tb_ptr != 0)
            {
                l_trace_size = G_shared_gpe_data.gpe1_tb_sz;
                memcpy(l_get_trace_resp_ptr->data, (uint8_t*)G_shared_gpe_data.gpe1_tb_ptr, (size_t)l_trace_size);
            }
        }
        else l_rc = 255;
    }
    else
    {
        // Return a 405 trace buffer
        const trace_descriptor_array_t* l_trace_ptr = TRAC_get_td((char *)l_get_trace_query_ptr->comp);
        l_rc = TRAC_get_buffer_partial(l_trace_ptr, l_get_trace_resp_ptr->data,&l_trace_buffer_size);
        l_trace_size = l_trace_buffer_size;
    }
    if(l_rc==0)
    {
        G_rsp_status = ERRL_RC_SUCCESS;
        o_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_trace_size);
        o_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_trace_size);
    }
    else
    {
        G_rsp_status = ERRL_RC_INTERNAL_FAIL;
        o_rsp_ptr->data_length[0] = 0;
        o_rsp_ptr->data_length[1] = 0;
    }
}

// Function Specification
//
// Name:  cmdh_dbug_get_ame_sensor
//
// Description: Process get sensor data command
//
// End Function Specification
void cmdh_dbug_get_ame_sensor (const cmdh_fsp_cmd_t * i_cmd_ptr,
                               cmdh_fsp_rsp_t * o_rsp_ptr)
{
    int                          l_rc = ERRL_RC_SUCCESS;
    bool                         l_clear = FALSE;
    uint16_t                     l_type = 0;
    uint16_t                     l_location = 0;
    uint16_t                     i = 0;
    uint16_t                     l_resp_data_length = 0;
    uint16_t                     l_num_of_sensors = CMDH_DBUG_MAX_NUM_SENSORS;
    cmdh_dbug_get_sensor_query_t *l_cmd_ptr = (cmdh_dbug_get_sensor_query_t*) i_cmd_ptr;
    cmdh_dbug_get_sensor_resp_t  *l_resp_ptr = (cmdh_dbug_get_sensor_resp_t*) o_rsp_ptr;
    sensor_t                     *l_sensor_ptr = NULL;
    errlHndl_t                   l_err = NULL;

    do
    {
        // Do sanity check on the function inputs
        if ((NULL == i_cmd_ptr) || (NULL == o_rsp_ptr))
        {
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }

        // Capture user inputs
        l_type = l_cmd_ptr->type;
        l_location = l_cmd_ptr->location;
        if(l_cmd_ptr->sub_cmd == DBUG_GET_AND_CLEAR_AME_SENSOR)
            l_clear = TRUE;

        TRAC_INFO("dbug_get_ame_sensor: Type[0x%04x] Location[0x%04x] Clear?[%d]",
                  l_type,
                  l_location,
                  l_clear);

        // Initialize the arguments to query sensor list
        querySensorListArg_t l_qsl_arg = {
            0,                      // i_startGsid - start with sensor 0x0000
            0,                      // i_present
            l_type,                 // i_type - passed by the caller
            l_location,             // i_loc - passed by the caller
            &l_num_of_sensors,      // io_numOfSensors
            G_sensor_list,          // o_sensors
            NULL                    // o_sensorInfoPtr
        };

        // Get the sensors
        l_err = querySensorList(&l_qsl_arg);

        if (NULL != l_err)
        {
            // Query failure, this should not happen
            TRAC_ERR("dbug_get_ame_sensor: Failed to query sensors. Error status is: 0x%x",
                     l_err->iv_reasonCode);

            // Commit error log
            commitErrl(&l_err);
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }
        else
        {
            TRAC_INFO("dbug_get_ame_sensor: Numbers of sensors found[%u]",
                      l_num_of_sensors);

            if (l_num_of_sensors > CMDH_DBUG_MAX_NUM_SENSORS)
            {
                // Got too many sensors back, need to truncate the list
                TRAC_INFO("dbug_get_ame_sensor: Got too many sensors back[%u]. Truncating number of sensors to %u",
                          l_num_of_sensors,
                          CMDH_DBUG_MAX_NUM_SENSORS);

                l_num_of_sensors = CMDH_DBUG_MAX_NUM_SENSORS;
            }

            // Populate the response data packet
            l_resp_ptr->num_sensors = l_num_of_sensors;
            for (i=0; i<l_num_of_sensors; i++)
            {
                l_resp_ptr->sensor[i].gsid = G_sensor_list[i].gsid;
                l_resp_ptr->sensor[i].sample = G_sensor_list[i].sample;
                strcpy(l_resp_ptr->sensor[i].name, G_sensor_list[i].name);

                // Capture the min and max value for this sensor
                l_sensor_ptr = getSensorByGsid(G_sensor_list[i].gsid);
                if (l_sensor_ptr == NULL)
                {
                    TRAC_INFO("dbug_get_ame_sensor: Didn't find sensor with gsid[0x%.4X]. Min/Max values won't be accurate.",
                              G_sensor_list[i].gsid);

                    // Didn't find this sensor, just continue
                    continue;
                }
                l_resp_ptr->sensor[i].sample_min = l_sensor_ptr->sample_min;
                l_resp_ptr->sensor[i].sample_max = l_sensor_ptr->sample_max;

                if(l_clear)
                {
                    sensor_reset(l_sensor_ptr);
                }
            }
        }

    }while(0);

    // Populate the response data header (with actual number of sensors collected)
    l_resp_data_length = sizeof(l_num_of_sensors) + (l_num_of_sensors * sizeof(cmdh_dbug_sensor_list_t));
    G_rsp_status = l_rc;
    o_rsp_ptr->data_length[0] = ((uint8_t *)&l_resp_data_length)[0];
    o_rsp_ptr->data_length[1] = ((uint8_t *)&l_resp_data_length)[1];
} // end cmdh_dbug_get_ame_sensor()


// Function Specification
//
// Name:  cmdh_dbug_peek
//
// Description: Process peek debug command
//
// End Function Specification
void cmdh_dbug_peek (const cmdh_fsp_cmd_t * i_cmd_ptr,
                     cmdh_fsp_rsp_t * o_rsp_ptr)
{
    cmdh_dbug_peek_t * l_cmd_ptr = (cmdh_dbug_peek_t*) i_cmd_ptr;
    uint32_t           l_len     = l_cmd_ptr->size;
    uint8_t            l_type    = l_cmd_ptr->type;
    uint32_t           l_addr    = l_cmd_ptr->address;
    int                l_ssxrc   = SSX_OK;
    static bool L_traced_reject = FALSE;

    if (G_smf_mode == false)
    {
        switch(l_type)
        {
            case 0x01:    // OCI Direct Read
                // Make sure we don't overflow our response buffer
                l_len = (l_len > CMDH_FSP_RSP_DATA_SIZE ) ? CMDH_FSP_RSP_DATA_SIZE : l_len;

                // Read the data
                memcpy( (void *) &o_rsp_ptr->data[0],
                        (void *) l_addr,
                        (size_t) l_len );
                break;

            case 0x02:    // DMA Read
                // Make sure address is 128 byte aligned required for block copy
                if(l_addr % 128)
                {
                    TRAC_ERR("cmdh_dbug_peek: Address 0x%08X is not 128 byte aligned", l_addr);
                    // no error handling for debug just respond with 0 bytes
                    l_len = 0;
                }
                else
                {
                    // only 1 length is supported by the OCC
                    // ignore input length and set length to CMDH_DEBUG_DMA_READ_SIZE
                    l_len = CMDH_DEBUG_DMA_READ_SIZE;

                    // Copy data from main memory to SRAM
                    // Set up a copy request
                    l_ssxrc = bce_request_create(&G_debug_pba_request,                // block copy object
                                                 &G_pba_bcde_queue,                   // mainstore to sram copy engine
                                                 l_addr,                              // mainstore address
                                                 (uint32_t)&G_debug_dma_buffer,       // sram starting address
                                                 l_len,                               // size of copy
                                                 SSX_SECONDS(1),                      // timeout
                                                 NULL,                                // no call back
                                                 NULL,                                // no call back arguments
                                                 ASYNC_REQUEST_BLOCKING);             // blocking request

                    if(l_ssxrc != SSX_OK)
                    {
                        TRAC_ERR("cmdh_dbug_peek: BCDE request create failure rc=[%08X]", -l_ssxrc);
                        // no error handling for debug just respond with 0 bytes
                        l_len = 0;
                    }
                    else
                    {
                        // Do actual copying
                        l_ssxrc = bce_request_schedule(&G_debug_pba_request);

                        if(l_ssxrc != SSX_OK)
                        {
                            TRAC_ERR("cmdh_dbug_peek: BCE request schedule failure rc=[%08X]", -l_ssxrc);
                            // no error handling for debug just respond with 0 bytes
                            l_len = 0;
                        }
                        else
                        {
                            // Copy to response buffer
                            memcpy((void *) &o_rsp_ptr->data[0],
                                   &G_debug_dma_buffer,
                                   l_len);
                        }
                    }
                } // else address is 128 byte aligned

                break;

            case 0x03:   // Invalidate Cache
                //dcache_invalidate( (void *) l_addr, l_len );
                l_len = 0;
                break;

            case 0x04:   // Flush Cache
                dcache_flush( (void *) l_addr, l_len );
                l_len = 0;
                break;

            default:
                // Didn't do anything, respond with zero bytes
                l_len = 0;
                break;
        }

        G_rsp_status = ERRL_RC_SUCCESS;
        o_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_len);
        o_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_len);
    }
    else
    {
        if (!L_traced_reject)
        {
            TRAC_ERR("cmdh_dbug_peek: Peek not supported in SMF mode");
            L_traced_reject = TRUE;
        }

        // Return error to TMGT w/no log
        G_rsp_status = ERRL_RC_NO_SUPPORT_IN_SMF_MODE;
        o_rsp_ptr->data_length[0] = 0;
        o_rsp_ptr->data_length[1] = 1;
        o_rsp_ptr->data[0] = 0x00; // no error log
    }

} // end cmdh_dbug_peek()


// Function Specification
//
// Name:  cmdh_dbug_get_apss_data
//
// Description: Process APSS data request
//
// End Function Specification
void cmdh_dbug_get_apss_data (const cmdh_fsp_cmd_t * i_cmd_ptr,
                              cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                      l_rc = ERRL_RC_SUCCESS;
    uint16_t                     i = 0;
    uint16_t                     l_resp_data_length = 0;
    cmdh_dbug_apss_data_resp_t  *l_resp_ptr = (cmdh_dbug_apss_data_resp_t*) o_rsp_ptr;

    do
    {
        // Do sanity check on the function inputs
        if ((NULL == i_cmd_ptr) || (NULL == o_rsp_ptr))
        {
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }

        //Get the data for each channel individually and write it to
        for (i = 0; i < MAX_APSS_ADC_CHANNELS; i++)
        {
            l_resp_ptr->ApssCh[i].gain = G_sysConfigData.apss_cal[i].gain;
            l_resp_ptr->ApssCh[i].offset = G_sysConfigData.apss_cal[i].offset;
            l_resp_ptr->ApssCh[i].raw = G_dcom_slv_inbox_rx.adc[i];
            l_resp_ptr->ApssCh[i].calculated = AMECSENSOR_PTR(PWRAPSSCH0 + i)->sample;
            l_resp_ptr->ApssCh[i].func = G_apss_ch_to_function[i];

            TRAC_IMP("DBG__APSS Ch[%02d]:  Raw[0x%04x], Offset[0x%08x], Gain[0x%08x],",
                      i, l_resp_ptr->ApssCh[i].raw, l_resp_ptr->ApssCh[i].offset, l_resp_ptr->ApssCh[i].gain);
            TRAC_IMP("                     Pwr[0x%04x], FuncID[0x%02x]",
                      l_resp_ptr->ApssCh[i].calculated, l_resp_ptr->ApssCh[i].func);
        }

    }while(0);

    // Populate the response data header
    l_resp_data_length = sizeof(cmdh_dbug_apss_data_resp_t) - CMDH_DBUG_FSP_RESP_LEN;
    G_rsp_status = l_rc;
    o_rsp_ptr->data_length[0] = ((uint8_t *)&l_resp_data_length)[0];
    o_rsp_ptr->data_length[1] = ((uint8_t *)&l_resp_data_length)[1];
}


// Function Specification
//
// Name: cmdh_dbug_dump_ame_sensor
//
// Description: Returns all fields (static and dynamic) for one sensor
//
// End Function Specification
void cmdh_dbug_dump_ame_sensor(const cmdh_fsp_cmd_t * i_cmd_ptr,
                               cmdh_fsp_rsp_t * o_rsp_ptr)
{
    const cmdh_dbug_dump_ame_sensor_cmd_t * l_cmd_ptr = (cmdh_dbug_dump_ame_sensor_cmd_t*) i_cmd_ptr;
    cmdh_dbug_dump_ame_sensor_rsp_t *       l_rsp_ptr = (cmdh_dbug_dump_ame_sensor_rsp_t*) o_rsp_ptr;
    uint8_t                                 l_rc = ERRL_RC_SUCCESS;    // Assume succeeds
    uint16_t                                l_resp_data_length = 0;

    // Make sure command and response pointer are valid
    if ((l_cmd_ptr == NULL) || (l_rsp_ptr == NULL))
    {
        l_rc = ERRL_RC_INTERNAL_FAIL;
    }
    else
    {
        // Make sure sensor gsid is valid
        uint16_t l_gsid = l_cmd_ptr->gsid;
        if (l_gsid >= G_amec_sensor_count)
        {
            l_rc = ERRL_RC_INVALID_DATA;
        }
        else
        {
            // Copy static sensor fields into response struct
            memcpy(&(l_rsp_ptr->sensor_info), &(G_sensor_info[l_gsid]), sizeof(sensor_info_t));
            l_resp_data_length += sizeof(sensor_info_t);

            // Copy dynamic sensor fields into response struct
            memcpy(&(l_rsp_ptr->sensor), G_amec_sensor_list[l_gsid], sizeof(sensor_t));
            l_resp_data_length += sizeof(sensor_t);
        }
    }

    // Populate the response data header
    if (l_rsp_ptr != NULL)
    {
        l_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_resp_data_length);
        l_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_resp_data_length);
    }
    G_rsp_status = l_rc;
}

// Function Specification
//
// Name: cmdh_dbug_wof_control
//
// Description: Sets the specified bit or clears all of them of wof_disabled
//
// End Function Specification
void cmdh_dbug_wof_control( const cmdh_fsp_cmd_t * i_cmd_ptr,
                            cmdh_fsp_rsp_t * o_rsp_ptr )
{
    const cmdh_dbug_wof_control_cmd_t * l_cmd_ptr = (cmdh_dbug_wof_control_cmd_t*) i_cmd_ptr;
    cmdh_dbug_wof_control_rsp_t * l_rsp_ptr = (cmdh_dbug_wof_control_rsp_t*) o_rsp_ptr;
    uint8_t  l_rc = ERRL_RC_SUCCESS;
    uint16_t l_resp_data_length = sizeof(g_amec->wof.wof_disabled);

     // Do sanity check on the function inputs
    if ((NULL == l_cmd_ptr) || (NULL == l_rsp_ptr))
    {
        l_rc = ERRL_RC_INTERNAL_FAIL;
    }
    else
    {
        // Process action
        if( l_cmd_ptr->action == SET )
        {
            g_amec->wof.wof_disabled |= l_cmd_ptr->wof_rc;
        }
        else if( l_cmd_ptr->action == CLEAR )
        {
            if(g_amec->wof.wof_disabled & WOF_RC_NO_WOF_HEADER_MASK)
            {
                TRAC_INFO("DEBUG - No WOF header present in memory."
                          " Cannot enable WOF!");
                g_amec->wof.wof_disabled = WOF_RC_NO_WOF_HEADER_MASK;
            }
            else
            {
                g_amec->wof.wof_disabled = 0x00000000;
            }
        }
        // Fill in response data
        l_rsp_ptr->wof_disabled = g_amec->wof.wof_disabled;
    }

    TRAC_INFO("DEBUG - wof_disabled: 0x%08x", g_amec->wof.wof_disabled);

    // Fill in response data length
    if( l_rsp_ptr != NULL )
    {
        l_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_resp_data_length);
        l_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_resp_data_length);
    }
    G_rsp_status = l_rc;
    return;
}

// Function Specification
//
// Name: cmdh_dbug_dump_wof_data
//
// Description: Dumps out the contents of g_amec_sys.wof
//
// End Function Specification
void cmdh_dbug_dump_wof_data( const cmdh_fsp_cmd_t * i_cmd_ptr,
                              cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint16_t l_datalen = sizeof(amec_wof_t);

    // Fill in response data
    memcpy((void*)&(o_rsp_ptr->data[0]),
           (void*)&(g_amec->wof),
           l_datalen);

    // Fill in response data length
    o_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_datalen);
    o_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_datalen);
    G_rsp_status = ERRL_RC_SUCCESS;
    return;
}

void cmdh_dbug_force_wof_reset( const cmdh_fsp_cmd_t * i_cmd_ptr,
                                cmdh_fsp_rsp_t * o_rsp_ptr)
{
    TRAC_INFO("DEBUG - Forcing WOF Reset");
    set_clear_wof_disabled( SET,
                            WOF_RC_RESET_DEBUG_CMD,
                            ERC_WOF_RESET_DEBUG_CMD );
    G_rsp_status = ERRL_RC_SUCCESS;
}

void cmdh_dbug_dump_oppb( const cmdh_fsp_cmd_t * i_cmd_ptr,
                                 cmdh_fsp_rsp_t * o_rsp_ptr)
{
    // Put important contents used by OCC for WOF into INFO trace buffer
    print_oppb();

    // return the full OPPB (up to max return size) in the response buffer
    uint16_t l_len = (sizeof(G_oppb) > CMDH_FSP_RSP_DATA_SIZE ) ? CMDH_FSP_RSP_DATA_SIZE : sizeof(G_oppb);

    memcpy((void*)&(o_rsp_ptr->data[0]),
           (void*)&(G_oppb),
           l_len);

    // Fill in the response data length
    o_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_len);
    o_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_len);

    G_rsp_status = ERRL_RC_SUCCESS;
}

// Function Specification
//
// Name: cmdh_dbug_dump_static_wof_data
//
// Description: Dumps out the contents of g_amec_sys.static_wof_data
//
// End Function Specification
void cmdh_dbug_dump_static_wof_data( const cmdh_fsp_cmd_t * i_cmd_ptr,
                                     cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint16_t l_datalen = sizeof(amec_static_wof_t);

    // Fill in response data
    memcpy((void*)&(o_rsp_ptr->data[0]),
           (void*)&(g_amec->static_wof_data),
           l_datalen);

    TRAC_INFO("WOF STATIC DATA - vrt_tbls_main_mem_addr[0x%08X] vrt_tbls_len[0x%08X]",
               g_amec->static_wof_data.vrt_tbls_main_mem_addr,
               g_amec->static_wof_data.vrt_tbls_len);

    TRAC_INFO("WOF STATIC DATA - occ_values_sram_addr[0x%08X] pgpe_values_sram_addr[0x%08X]   xgpe_values_sram_addr[0x%08X]  pstate_tbl_sram_addr[0x%08X]",
               g_amec->static_wof_data.occ_values_sram_addr,
               g_amec->static_wof_data.pgpe_values_sram_addr,
               g_amec->static_wof_data.xgpe_values_sram_addr,
               g_amec->static_wof_data.pstate_tbl_sram_addr);


    // Fill in response data length
    o_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_datalen);
    o_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_datalen);
    G_rsp_status = ERRL_RC_SUCCESS;
    return;
}

// Function Specification
//
// Name: cmdh_dbug_dump_wof_off_data
//
// Description: Dumps out the contents of g_amec->oc_wof_off
//
// End Function Specification
void cmdh_dbug_dump_wof_off_data( const cmdh_fsp_cmd_t * i_cmd_ptr,
                                        cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint16_t l_datalen = sizeof(oc_wof_off_t) + 1;

    // Fill in response data
    o_rsp_ptr->data[0] = g_amec->wof.ocs_dirty;

    memcpy((void*)&(o_rsp_ptr->data[1]),
           (void*)&(g_amec->oc_wof_off),
           sizeof(oc_wof_off_t));

    TRAC_INFO("WOF OFF DATA - decrease_pstate[%d] increase_pstate[%d] pstate_max[0x%02X]",
               g_amec->oc_wof_off.decrease_pstate,
               g_amec->oc_wof_off.increase_pstate,
               g_amec->oc_wof_off.pstate_max);

    TRAC_INFO("WOF OFF DATA - dirty bits[0x%02X] pstate_request[0x%02X] freq_request[0x%04X]",
               g_amec->wof.ocs_dirty,
               g_amec->oc_wof_off.pstate_request,
               g_amec->oc_wof_off.freq_request);


    // Fill in response data length
    o_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_datalen);
    o_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_datalen);
    G_rsp_status = ERRL_RC_SUCCESS;
    return;
}

// Function Specification
//
// Name: cmdh_dbug_wof_ocs
//
// Description: Writes data related to OCS support
//
// End Function Specification
void cmdh_dbug_wof_ocs( const cmdh_fsp_cmd_t * i_cmd_ptr,
                              cmdh_fsp_rsp_t * o_rsp_ptr)
{
    const cmdh_dbug_wof_ocs_cmd_t * l_cmd_ptr = (cmdh_dbug_wof_ocs_cmd_t*) i_cmd_ptr;
    cmdh_dbug_wof_ocs_rsp_t * l_rsp_ptr = (cmdh_dbug_wof_ocs_rsp_t*) o_rsp_ptr;
    uint8_t  l_rc = ERRL_RC_SUCCESS;
    uint16_t l_resp_data_length = sizeof(g_amec->wof.ocs_increase_ceff) + sizeof(g_amec->wof.ocs_decrease_ceff);

     // Do sanity check on the function inputs
    if ((NULL == l_cmd_ptr) || (NULL == l_rsp_ptr))
    {
        l_rc = ERRL_RC_INTERNAL_FAIL;
    }
    else if(l_cmd_ptr->pstates)  // values are Pstates used when WOF is off
    {
        // Save the OCS up and down addrs
        g_amec->oc_wof_off.increase_pstate = l_cmd_ptr->ceff_up_amount;
        g_amec->oc_wof_off.decrease_pstate = l_cmd_ptr->ceff_down_amount;

        TRAC_INFO("DEBUG - OCS Pstate Adders written up[%d] down[%d]",
                   g_amec->oc_wof_off.increase_pstate, g_amec->oc_wof_off.decrease_pstate);

        // Fill in response data
        l_rsp_ptr->ceff_up_amount = g_amec->oc_wof_off.increase_pstate;
        l_rsp_ptr->ceff_down_amount = g_amec->oc_wof_off.decrease_pstate;
    }
    else  // values are ceff values used when WOF is on
    {
        // Save the OCS up and down addrs
        g_amec->wof.ocs_increase_ceff = l_cmd_ptr->ceff_up_amount;
        g_amec->wof.ocs_decrease_ceff = l_cmd_ptr->ceff_down_amount;

        TRAC_INFO("DEBUG - OCS Ceff Adders written up[%d] down[%d]",
                   g_amec->wof.ocs_increase_ceff, g_amec->wof.ocs_decrease_ceff);

        // Fill in response data
        l_rsp_ptr->ceff_up_amount = g_amec->wof.ocs_increase_ceff;
        l_rsp_ptr->ceff_down_amount = g_amec->wof.ocs_decrease_ceff;
    }

    // Fill in response data length
    if( l_rsp_ptr != NULL )
    {
        l_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_resp_data_length);
        l_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_resp_data_length);
    }
    G_rsp_status = l_rc;
    return;
}


// Function Specification
//
// Name: cmdh_dbug_allow_trace
//
// Description: Set/Clear flags that allow/prevent certain traces to appear
//
// End Function Specification
void cmdh_dbug_allow_trace( const cmdh_fsp_cmd_t * i_cmd_ptr,
                            cmdh_fsp_rsp_t * o_rsp_ptr )
{
    const cmdh_dbug_allow_trace_cmd_t * l_cmd_ptr =
                                 (cmdh_dbug_allow_trace_cmd_t*)i_cmd_ptr;
    cmdh_dbug_allow_trace_rsp_t * l_rsp_ptr =
                                 (cmdh_dbug_allow_trace_rsp_t*)o_rsp_ptr;

    uint8_t l_rc = ERRL_RC_SUCCESS;
    uint16_t l_resp_data_length = sizeof(G_allow_trace_flags);

    if((NULL == l_cmd_ptr) || (NULL == l_rsp_ptr))
    {
        l_rc = ERRL_RC_INTERNAL_FAIL;
    }
    else
    {
        if( l_cmd_ptr->action == SET )
        {
            G_allow_trace_flags |= l_cmd_ptr->trace_flags;
        }
        else
        {
            G_allow_trace_flags = 0x0000;
        }
    }

    TRAC_INFO("DEBUG - allow_trace FLAGS 0x%04x Action: %d",
            G_allow_trace_flags,
            l_cmd_ptr->action);

    // fill in response data length
    if( l_rsp_ptr != NULL )
    {
        l_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_resp_data_length);
        l_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_resp_data_length);
    }
    G_rsp_status = l_rc;
    return;
}


/// Function Specification
//
// Name: cmdh_dbug_dimm_inject
//
// Description: Set/Clear internal debug flags
//
// End Function Specification
void cmdh_dbug_dimm_inject( const cmdh_fsp_cmd_t * i_cmd_ptr,
                               cmdh_fsp_rsp_t * o_rsp_ptr )
{
    const cmdh_dbug_dimm_inject_cmd_t * l_cmd_ptr =
                                 (cmdh_dbug_dimm_inject_cmd_t*)i_cmd_ptr;
    cmdh_dbug_dimm_inject_rsp_t * l_rsp_ptr =
                                 (cmdh_dbug_dimm_inject_rsp_t*)o_rsp_ptr;

    uint8_t l_rc = ERRL_RC_SUCCESS;
    // confirm inject data length (ignore sub_command byte)
    const uint16_t inject_length = CMDH_DATALEN_FIELD_UINT16(l_cmd_ptr) - 1;

    if((NULL == l_cmd_ptr) || (NULL == l_rsp_ptr))
    {
        l_rc = ERRL_RC_INTERNAL_FAIL;
    }
    // Command Length Check
    else if ((inject_length != 0) && (inject_length != MAX_NUM_OCMBS))
    {
        TRAC_ERR("cmdh_dbug_dimm_inject: Invalid inject data length %u (expected %u)",
                 inject_length, MAX_NUM_OCMBS);
        l_rc = ERRL_RC_INVALID_CMD_LEN;
    }
    else
    {
        if (inject_length == MAX_NUM_OCMBS)
        {
            TRAC_INFO("cmdh_dbug_dimm_inject: updating DIMM inject mask from 0x%08X.%08X to 0x%08X.%08X",
                      G_inject_dimm >> 32, G_inject_dimm & 0xFFFFFFFF,
                      l_cmd_ptr->inject_mask >> 32, l_cmd_ptr->inject_mask & 0xFFFFFFFF);
            G_inject_dimm = l_cmd_ptr->inject_mask;

            unsigned int l_membuf;
            for(l_membuf = 0; l_membuf < MAX_NUM_OCMBS; l_membuf++)
            {
                uint8_t dimms = (G_inject_dimm >> (l_membuf*8)) & 0xFF;
                if (dimms != 0)
                {
                    unsigned int k;
                    for(k=0; k < NUM_DIMMS_PER_OCMB; k++)
                    {
                        if (dimms & (1 << k))
                        {
                            if(!MEMBUF_SENSOR_ENABLED(l_membuf, k))
                            {
                                TRAC_ERR("cmdh_dbug_dimm_inject: membuf%d DIMM%d is not enabled", l_membuf, k);
                            }
                        }
                    }
                }
            }
        }
        // else just return current values

        // Return the current DIMM inject mask
        if( l_rsp_ptr != NULL )
        {
            l_rsp_ptr->data_length[0] = 0x00;
            l_rsp_ptr->data_length[1] = MAX_NUM_OCMBS;
            memcpy(&o_rsp_ptr->data[0], &G_inject_dimm, MAX_NUM_OCMBS);
        }
    }
    G_rsp_status = l_rc;
    return;
}



// Function Specification
//
// Name: cmdh_dbug_internal_flags
//
// Description: Set/Clear internal debug flags
//
// End Function Specification
void cmdh_dbug_internal_flags( const cmdh_fsp_cmd_t * i_cmd_ptr,
                               cmdh_fsp_rsp_t * o_rsp_ptr )
{
    const cmdh_dbug_internal_flags_cmd_t * l_cmd_ptr =
                                 (cmdh_dbug_internal_flags_cmd_t*)i_cmd_ptr;
    cmdh_dbug_internal_flags_rsp_t * l_rsp_ptr =
                                 (cmdh_dbug_internal_flags_rsp_t*)o_rsp_ptr;

    uint8_t l_rc = ERRL_RC_SUCCESS;
    const unsigned int flag_size = sizeof(G_internal_flags);
    // data length (ignore sub_command byte)
    const uint16_t data_length = CMDH_DATALEN_FIELD_UINT16(l_cmd_ptr) - 1;

    if ((NULL == l_cmd_ptr) || (NULL == l_rsp_ptr))
    {
        l_rc = ERRL_RC_INTERNAL_FAIL;
    }
    else if ((data_length != 0) && (data_length != flag_size))
    {
        TRAC_ERR("cmdh_dbug_internal_flags: Invalid internal flags length %u (expected %u)",
                 data_length, flag_size);
        l_rc = ERRL_RC_INVALID_CMD_LEN;
    }
    else
    {
        if (data_length == flag_size)
        {
            // if disabling WOF off OC protection clear the clip
            if( (l_cmd_ptr->flags & INT_FLAG_DISABLE_OC_WOF_OFF) &&
                !(G_internal_flags & INT_FLAG_DISABLE_OC_WOF_OFF) )
            {
                TRAC_INFO("DEBUG - Disabling WOF off OC protection current pstate clip[0x%02X]",
                          g_amec->oc_wof_off.pstate_request);
                g_amec->oc_wof_off.pstate_request = 0;
                g_amec->oc_wof_off.freq_request = 0xFFFF;
            }

            TRAC_INFO("DEBUG - updating internal flags from 0x%08X to 0x%08X",
                      G_internal_flags, l_cmd_ptr->flags);

            G_internal_flags = l_cmd_ptr->flags;
        }

        // always respond with the current flag value
        if( l_rsp_ptr != NULL )
        {
            l_rsp_ptr->data_length[0] = 0x00;
            l_rsp_ptr->data_length[1] = flag_size;
            // Fill in response data
            memcpy(&o_rsp_ptr->data[0], &G_internal_flags, flag_size);
        }
    }

    G_rsp_status = l_rc;
    return;
}



// Function Specification
//
// Name: cmdh_dbug_clear_ame_sensor
//
// Description: Clears minimum and maximum fields in one sensor.
//              Returns all dynamic sensor fields after the clear.
//
// End Function Specification
void cmdh_dbug_clear_ame_sensor(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                cmdh_fsp_rsp_t * o_rsp_ptr)
{
    const cmdh_dbug_clear_ame_sensor_cmd_t * l_cmd_ptr = (cmdh_dbug_clear_ame_sensor_cmd_t*) i_cmd_ptr;
    cmdh_dbug_clear_ame_sensor_rsp_t *       l_rsp_ptr = (cmdh_dbug_clear_ame_sensor_rsp_t*) o_rsp_ptr;
    uint8_t                                  l_rc = ERRL_RC_SUCCESS;    // Assume succeeds
    uint16_t                                 l_resp_data_length = 0;

    // Make sure command and response pointer are valid
    if ((l_cmd_ptr == NULL) || (l_rsp_ptr == NULL))
    {
        l_rc = ERRL_RC_INTERNAL_FAIL;
    }
    else
    {
        // Make sure sensor gsid is valid
        uint16_t l_gsid = l_cmd_ptr->gsid;
        if (l_gsid >= G_amec_sensor_count)
        {
            l_rc = ERRL_RC_INVALID_DATA;
        }
        else
        {
            // Clear specified min/max fields in sensor
            AMEC_SENSOR_CLEAR_TYPE l_clear_type = (AMEC_SENSOR_CLEAR_TYPE) l_cmd_ptr->clear_type;
            sensor_clear_minmax(G_amec_sensor_list[l_gsid], l_clear_type);

            // Copy dynamic sensor fields (after clear) into response struct
            memcpy(&(l_rsp_ptr->sensor), G_amec_sensor_list[l_gsid], sizeof(sensor_t));
            l_resp_data_length += sizeof(sensor_t);
        }
    }

    // Populate the response data header
    if (l_rsp_ptr != NULL)
    {
        l_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_resp_data_length);
        l_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_resp_data_length);
    }
    G_rsp_status = l_rc;
}

void cmdh_dump_gpu_timings(void)
{
    extern gpuTimingTable_t G_gpu_tick_times;
    int i = 0;

    for( ; i < MAX_NUM_GPU_PER_DOMAIN; i++)
    {
        TRAC_INFO("=======================================GPU%d===================================================", i);
        TRAC_INFO("|                          Max           Avg           1s count     100ms count  <100ms count|");
        TRAC_INFO("| Core Temperatures        %-5d msecs   %-5d msecs   %-5d        %-5d        %-5d",
                  G_gpu_tick_times.coretemp[i].max,
                  G_gpu_tick_times.coretemp[i].avg,
                  G_gpu_tick_times.coretemp[i].count_1s,
                  G_gpu_tick_times.coretemp[i].count_100ms,
                  G_gpu_tick_times.coretemp[i].count_lt100ms);
        TRAC_INFO("| Mem  Temperatures        %-5d msecs   %-5d msecs   %-5d        %-5d        %-5d",
                  G_gpu_tick_times.memtemp[i].max,
                  G_gpu_tick_times.memtemp[i].avg,
                  G_gpu_tick_times.memtemp[i].count_1s,
                  G_gpu_tick_times.memtemp[i].count_100ms,
                  G_gpu_tick_times.memtemp[i].count_lt100ms);
        TRAC_INFO("| Check Driver Loaded      %-5d msecs   %-5d msecs   %-5d        %-5d        %-5d",
                  G_gpu_tick_times.checkdriver[i].max,
                  G_gpu_tick_times.checkdriver[i].avg,
                  G_gpu_tick_times.checkdriver[i].count_1s,
                  G_gpu_tick_times.checkdriver[i].count_100ms,
                  G_gpu_tick_times.checkdriver[i].count_lt100ms);
        TRAC_INFO("| Mem  Capabilities        %-5d msecs   %-5d msecs   %-5d        %-5d        %-5d",
                  G_gpu_tick_times.capabilities[i].max,
                  G_gpu_tick_times.capabilities[i].avg,
                  G_gpu_tick_times.capabilities[i].count_1s,
                  G_gpu_tick_times.capabilities[i].count_100ms,
                  G_gpu_tick_times.capabilities[i].count_lt100ms);
        TRAC_INFO("| Read Power Policy        %-5d msecs   %-5d msecs   %-5d        %-5d        %-5d",
                  G_gpu_tick_times.getpcap[i].max,
                  G_gpu_tick_times.getpcap[i].avg,
                  G_gpu_tick_times.getpcap[i].count_1s,
                  G_gpu_tick_times.getpcap[i].count_100ms,
                  G_gpu_tick_times.getpcap[i].count_lt100ms);
        TRAC_INFO("| Set Power Cap            %-5d msecs   %-5d msecs   %-5d        %-5d        %-5d",
                  G_gpu_tick_times.setpcap[i].max,
                  G_gpu_tick_times.setpcap[i].avg,
                  G_gpu_tick_times.setpcap[i].count_1s,
                  G_gpu_tick_times.setpcap[i].count_100ms,
                  G_gpu_tick_times.setpcap[i].count_lt100ms);
        TRAC_INFO("==============================================================================================", i);
    }
}

// Function Specification
//
// Name:  dbug_err_inject
//
// Description: Injects an error
//
// End Function Specification
void dbug_err_inject(const cmdh_fsp_cmd_t * i_cmd_ptr,
                           cmdh_fsp_rsp_t * i_rsp_ptr)
{
    errlHndl_t l_err;
    cmdh_dbug_inject_errl_query_t *l_cmd_ptr = (cmdh_dbug_inject_errl_query_t*) i_cmd_ptr;

    i_rsp_ptr->data_length[0] = 0;
    i_rsp_ptr->data_length[1] = 0;
    G_rsp_status = ERRL_RC_SUCCESS;

    if(!strncmp(l_cmd_ptr->comp, "RST", OCC_TRACE_NAME_SIZE))
    {
        l_err = createErrl(CMDH_DBUG_MID,     //modId
                           INTERNAL_FAILURE,             //reasoncode
                           OCC_NO_EXTENDED_RC,           //Extended reason code
                           ERRL_SEV_PREDICTIVE,          //Severity
                           NULL,                         //Trace Buf
                           DEFAULT_TRACE_SIZE,           //Trace Size
                           0xff,                         //userdata1
                           0);                           //userdata2

        if (INVALID_ERR_HNDL == l_err)
        {
            G_rsp_status = ERRL_RC_INTERNAL_FAIL;
        }

        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_HUID,         //callout type (HUID/CompID)
                         G_sysConfigData.proc_huid,      //callout data
                         ERRL_CALLOUT_PRIORITY_HIGH);    //priority

        REQUEST_RESET(l_err);
    }
    else
    {
        ERRL_SEVERITY l_severity = ERRL_SEV_UNRECOVERABLE;
        if(strncmp(l_cmd_ptr->comp, "INF", 4) == 0)
        {
            l_severity = ERRL_SEV_INFORMATIONAL;
        }

        l_err = createErrl(CMDH_DBUG_MID,     //modId
                           INTERNAL_FAILURE,             //reasoncode
                           OCC_NO_EXTENDED_RC,           //Extended reason code
                           l_severity,                   //Severity
                           TRAC_get_td(l_cmd_ptr->comp), //Trace Buf
                           DEFAULT_TRACE_SIZE,           //Trace Size
                           0xff,                         //userdata1
                           0);                           //userdata2

        if (INVALID_ERR_HNDL == l_err)
        {
            G_rsp_status = ERRL_RC_INTERNAL_FAIL;
        }

        // Commit Error log
        commitErrl(&l_err);
    }

    if (G_rsp_status == ERRL_RC_INTERNAL_FAIL)
    {
        TRAC_ERR("cmdh_dbug_inject_errl: Fail creating ERR Log");
    }
    else
    {
        TRAC_INFO("cmdh_dbug_inject_errl: inject errl for COMP : %c%c%c",
                  l_cmd_ptr->comp[0], l_cmd_ptr->comp[1], l_cmd_ptr->comp[2]);
    }

    return;
}

// Function Specification
//
// Name:  dbug_membuf_dump
//
// Description: Injects an error
//
// End Function Specification
void dbug_membuf_dump(const cmdh_fsp_cmd_t * i_cmd_ptr,
                             cmdh_fsp_rsp_t * i_rsp_ptr)
{
    uint16_t l_datalen = 0;
    uint8_t l_jj=0;

    // Determine the size of the data we are returning
    l_datalen = (sizeof(OcmbMemData) * MAX_NUM_MEM_CONTROLLERS);

    // Fill out the response with the data we are returning
    for(l_jj=0; l_jj < MAX_NUM_MEM_CONTROLLERS; l_jj++)
    {
        OcmbMemData * l_sensor_cache_ptr = get_membuf_data_ptr(l_jj);

        memcpy((void *)&(i_rsp_ptr->data[l_jj*sizeof(OcmbMemData)]),
               (void *)l_sensor_cache_ptr,
               sizeof(OcmbMemData));
    }

    // Fill out the rest of the response data
    i_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_datalen);
    i_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_datalen);
    G_rsp_status              = ERRL_RC_SUCCESS;
    return;
}

// Function Specification
//
// Name:  dbug_apss_dump
//
// Description: Dumps the APSS power measurement raw ADC / GPIO data
//
// End Function Specification
void dbug_apss_dump(const cmdh_fsp_cmd_t * i_cmd_ptr,
                          cmdh_fsp_rsp_t * i_rsp_ptr)
{
    uint16_t l_datalen = 0;

    // Determine the size of the data we are returning
    l_datalen = (sizeof(apssPwrMeasStruct_t));

    memcpy((void *) &(i_rsp_ptr->data[0]),
           (void *) &G_apss_pwr_meas,
           sizeof(apssPwrMeasStruct_t));

    // Fill out the rest of the response data
    i_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_datalen);
    i_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_datalen);
    G_rsp_status              = ERRL_RC_SUCCESS;

    return;
}

// Function Specification
//
// Name:  dbug_clear_errh
//
// Description: Clear all error history counters
//
// End Function Specification
void dbug_clear_errh(const cmdh_fsp_cmd_t * i_cmd_ptr,
                           cmdh_fsp_rsp_t * i_rsp_ptr)
{
    uint16_t l_datalen = sizeof(G_error_history);

    // Copy the current error history counters to the response buffer befor clearing
    memcpy((void *) &(i_rsp_ptr->data[0]),
           (void *) &(G_error_history[0]),
           l_datalen);

    memset(G_error_history, 0x00, l_datalen);

    // Fill out the rest of the response data
    i_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_datalen);
    i_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_datalen);
    G_rsp_status              = ERRL_RC_SUCCESS;

    return;
}


// Function Specification
//
// Name: cmdh_dbug_write_sensor
//
// Description: Write the given value to the given gsid
//
// End Function Specification
void cmdh_dbug_write_sensor(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                cmdh_fsp_rsp_t * o_rsp_ptr)
{
    const cmdh_dbug_write_sensor_cmd_t *l_cmd_ptr = (cmdh_dbug_write_sensor_cmd_t*) i_cmd_ptr;
    cmdh_dbug_write_sensor_resp_t      *l_rsp_ptr = (cmdh_dbug_write_sensor_resp_t*) o_rsp_ptr;
    uint8_t                             l_rc = ERRL_RC_SUCCESS;    // Assume succeeds
    sensor_t                           *l_sensor_ptr = NULL;
    uint16_t                            l_rsp_data_length = 0;

    // Make sure command and response pointer are valid
    if ((l_cmd_ptr == NULL) || (l_rsp_ptr == NULL))
    {
        l_rc = ERRL_RC_INTERNAL_FAIL;
    }
    else
    {
        // Make sure sensor gsid is valid
        l_sensor_ptr = getSensorByGsid(l_cmd_ptr->gsid);
        if (l_sensor_ptr == NULL)
        {
            TRAC_INFO("dbug_write_sensor: Didn't find sensor with gsid[0x%04X]", l_cmd_ptr->gsid);
            l_rc = ERRL_RC_INVALID_DATA;
        }
        else
        {
            // Set the given value to the sensor
            sensor_update(l_sensor_ptr, l_cmd_ptr->value);

            // update response buffer
            strcpy(l_rsp_ptr->name, G_sensor_info[l_cmd_ptr->gsid].name);
            l_rsp_ptr->gsid = l_cmd_ptr->gsid;
            l_rsp_ptr->value = l_cmd_ptr->value;
            l_rsp_data_length = sizeof(cmdh_dbug_write_sensor_resp_t) - sizeof(struct cmdh_fsp_rsp_header);
            TRAC_INFO("dbug_write_sensor: Wrote %d to sensor gsid 0x%04x",
                      l_cmd_ptr->value,
                      l_cmd_ptr->gsid);
        }
    }

    o_rsp_ptr->data_length[0] = ((uint8_t *)&l_rsp_data_length)[0];
    o_rsp_ptr->data_length[1] = ((uint8_t *)&l_rsp_data_length)[1];
    G_rsp_status = l_rc;
}


// Function Specification
//
// Name:  dbug_proc_data_dump
//
// Description: Dumps the processor core data
//
// End Function Specification
void dbug_proc_data_dump(const cmdh_fsp_cmd_t * i_cmd_ptr,
                               cmdh_fsp_rsp_t * i_rsp_ptr)
{
    uint16_t l_datalen = 0;
    uint8_t l_jj=0;

    // Determine the size of the data we are returning
    l_datalen = (sizeof(CoreData) * MAX_NUM_CORES);

    // Fill out the response with the data we are returning
    for(l_jj=0; l_jj < MAX_NUM_CORES; l_jj++)
    {
        CoreData * l_core_data_ptr =
            proc_get_bulk_core_data_ptr(l_jj);

        memcpy((void *) &(i_rsp_ptr->data[l_jj*sizeof(CoreData)]),
               (void *) l_core_data_ptr,
               sizeof(CoreData));
    }

    // Fill out the rest of the response data
    i_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_datalen);
    i_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_datalen);
    G_rsp_status              = ERRL_RC_SUCCESS;
    return;
}

// Function Specification
//
// Name:  dbug_parse_cmd
//
// Description: Process debug commands
//              Error logs are not generated for debug commands
//
// End Function Specification
void cmdh_dbug_cmd (const cmdh_fsp_cmd_t * i_cmd_ptr,
                    cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t l_err = NULL;

    // Sub Command for debug is always first byte of data
    uint8_t l_sub_cmd = i_cmd_ptr->data[0];

    /// Tracing based on Debug Sub-Command
    switch (l_sub_cmd)
    {
        // ----------------------------------------------------
        // NOTE:  This for for TRACING only, any actual command
        //        handling goes in the switch statement below.
        // ----------------------------------------------------
        case DBUG_GET_TRACE:
        case DBUG_GET_AME_SENSOR:
        case DBUG_WRITE_SENSOR:
        case DBUG_GET_AND_CLEAR_AME_SENSOR:
            // Don't trace that we got these debug commands, they happen too
            // often, or are not destructive when they do occur.
            break;

        default:
            // Trace the rest of the debug commands.
            TRAC_INFO("Debug Command: Sub:0x%02x", l_sub_cmd);

            break;
    }

    // Act on Debug Sub-Command
    switch ( l_sub_cmd )
    {
        case DBUG_DUMP_WOF_DATA:
            cmdh_dbug_dump_wof_data(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_FORCE_WOF_RESET:
            cmdh_dbug_force_wof_reset(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_GET_TRACE:
            // Get trace buffer SRAM address
            cmdh_dbug_get_trace(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_CLEAR_TRACE:
            // Call clear trace function
            TRAC_reset_buf();
            break;

        case DBUG_ALLOW_TRACE:
            cmdh_dbug_allow_trace( i_cmd_ptr, o_rsp_ptr );
            break;

        case DBUG_GET_AME_SENSOR:
        case DBUG_GET_AND_CLEAR_AME_SENSOR:
            cmdh_dbug_get_ame_sensor(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_DUMP_GPU_TIMINGS:
            cmdh_dump_gpu_timings();
            break;

        case DBUG_PEEK:
            cmdh_dbug_peek(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_DUMP_STATIC_WOF_DATA:
            cmdh_dbug_dump_static_wof_data(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_DUMP_WOF_OFF_DATA:
            cmdh_dbug_dump_wof_off_data(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_DUMP_RAW_AD:
             dbug_apss_dump(i_cmd_ptr, o_rsp_ptr);
             break;

        case DBUG_CLEAR_ERRH:
             dbug_clear_errh(i_cmd_ptr, o_rsp_ptr);
             break;

        case DBUG_WRITE_SENSOR:
            cmdh_dbug_write_sensor(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_INJECT_PGPE_ERRL:
            TRAC_ERR("cmdh_dbug_cmd: Creating PGPE Error Log");
            l_err = createPgpeErrl(CMDH_DBUG_MID,            //modId
                                   PGPE_FAILURE,             //reasoncode
                                   OCC_NO_EXTENDED_RC,       //Extended reason code
                                   ERRL_SEV_UNRECOVERABLE,   //Severity
                                   0xff,                     //userdata1
                                   0xff);                    //userdata2

            if (INVALID_ERR_HNDL == l_err)
            {
                TRAC_ERR("cmdh_dbug_cmd: Failed to create PGPE Error Log");
                G_rsp_status = ERRL_RC_INTERNAL_FAIL;
            }
            else
            {
                // Commit Error log
                commitErrl(&l_err);
            }
            break;

        case DBUG_INJECT_ERRL:
            dbug_err_inject(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_DIMM_INJECT:
            cmdh_dbug_dimm_inject( i_cmd_ptr, o_rsp_ptr );
            break;

        case DBUG_INTERNAL_FLAGS:
            cmdh_dbug_internal_flags( i_cmd_ptr, o_rsp_ptr );
            break;

        case DBUG_FLUSH_DCACHE:
            dcache_flush_all();
            break;

        case DBUG_MEMBUF_SENSOR_CACHE:
             dbug_membuf_dump(i_cmd_ptr, o_rsp_ptr);
             break;

        case DBUG_DUMP_PROC_DATA:
             dbug_proc_data_dump(i_cmd_ptr, o_rsp_ptr);
             break;

        case DBUG_GEN_CHOM_LOG:
            chom_force_gen_log();
            break;

        case DBUG_DUMP_APSS_DATA:
            cmdh_dbug_get_apss_data(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_DUMP_AME_SENSOR:
            cmdh_dbug_dump_ame_sensor(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_CLEAR_AME_SENSOR:
            cmdh_dbug_clear_ame_sensor(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_WOF_CONTROL:
            cmdh_dbug_wof_control(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_WOF_OCS:
            cmdh_dbug_wof_ocs(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_DUMP_OPPB:
            cmdh_dbug_dump_oppb(i_cmd_ptr, o_rsp_ptr);
            break;

        default:
            TRAC_ERR("cmdh_dbug_cmd: Debug sub command 0x%02X not valid", l_sub_cmd);
            G_rsp_status = ERRL_RC_INVALID_DATA;
            break;
    } //end switch

    return;
}
