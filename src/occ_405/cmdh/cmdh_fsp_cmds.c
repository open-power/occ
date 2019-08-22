/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cmdh/cmdh_fsp_cmds.c $                            */
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

#include "ssx.h"
#include "cmdh_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "rtls.h"
#include "dcom.h"
#include "occ_common.h"
#include "state.h"
#include "cmdh_fsp_cmds.h"
#include "proc_pstate.h"
#include <amec_data.h>
#include "amec_amester.h"
#include "amec_service_codes.h"
#include "amec_freq.h"
#include "amec_sys.h"
#include "sensor.h"
#include "sensor_query_list.h"
#include "amec_master_smh.h"
#include <proc_data.h>
#include "homer.h"
#include <avsbus.h>
#include "wof.h"
#include "sensor_main_memory.h"
#include "memory.h"
#include "memory_data.h"
#include "gpu.h"

extern dimm_sensor_flags_t G_dimm_temp_expired_bitmap;
extern uint32_t G_first_proc_gpu_config;
extern uint32_t G_first_sys_gpu_config;
extern bool G_vrm_vdd_temp_expired;
extern bool G_reset_prep;
extern uint16_t G_amester_max_data_length;
extern uint8_t G_occ_interrupt_type;

extern opal_proc_voting_reason_t G_amec_opal_proc_throt_reason;

extern bool G_htmgt_notified_of_error;
extern bool G_smf_mode;

// This table contains tunable parameter information that can be exposed to
// customers (only Master OCC should access/control this table)
cmdh_tunable_param_table_t G_mst_tunable_parameter_table[CMDH_DEFAULT_TUNABLE_PARAM_NUM] =
{
    {1,   "Utilization threshold for increasing frequency",              3,  0,  980,  0,    1000},
    {2,   "Utilization threshold for decreasing frequency",              3,  0,  980,  0,    1000},
    {3,   "Number of samples for computing utilization statistics",      4,  0,  16,   1,    1024},
    {4,   "Step size for going up in frequency",                         3,  0,  8,    1,    1000},
    {5,   "Step size for going down in frequency",                       3,  0,  8,    1,    1000},
    {6,   "Delta percentage for determining active cores",               2,  0,  18,   0,    100 },
    {7,   "Utilization threshold to determine active cores with slack",  3,  0,  980,  0,    1000},
    {8,   "Enable/disable frequency delta between cores",                0,  0,  0,    0,    1   },
    {9,   "Maximum frequency delta between cores",                       2,  0,  10,   10,   100 },
    {10,  "Enable/disable Workload Optimized Frequency",                 0,  0,  1,    0,    1   },
};

// The first two columns of this table are the default tunable parameter values
// and mutipliers.
cmdh_tunable_param_table_ext_t G_mst_tunable_parameter_table_ext[CMDH_DEFAULT_TUNABLE_PARAM_NUM] =
{
    {980, 10,  9800},
    {980, 10,  9800},
    {16,  1,   16  },
    {8,   1,   8   },
    {8,   1,   8   },
    {18,  100, 1800},
    {980, 10,  980 },
    {0,   1,   0   },
    {10,  1,   10  },
    {1,   1,   1   },
};

// Flag to indicate that new tunable parameter values need to be written
// (=0: no new values available; =1: new values need to be written; =2: restore defaults)
uint8_t G_mst_tunable_parameter_overwrite = 0;

//Reverse association of channel to function.
uint8_t G_apss_ch_to_function[MAX_APSS_ADC_CHANNELS] = {0};

ERRL_RC cmdh_poll_v20 (cmdh_fsp_rsp_t * i_rsp_ptr);

#define MAX_CONSECUTIVE_HCODE_ELOGS 2

// Function Specification
//
// Name:  cmdh_tmgt_poll
//
// Description: Poll the OCC for OCC status, OCCs present
//              system mode, error log ID, etc.
//
// End Function Specification
errlHndl_t cmdh_tmgt_poll (const cmdh_fsp_cmd_t * i_cmd_ptr,
                           cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t                  l_errlHndl  = NULL;
    cmdh_poll_query_t *         l_poll_cmd  = (cmdh_poll_query_t *) i_cmd_ptr;
    ERRL_RC                     l_rc        = ERRL_RC_INTERNAL_FAIL;

    do
    {
        if (l_poll_cmd->version == CMDH_POLL_VERSION20)
        {
            l_rc = cmdh_poll_v20(o_rsp_ptr);
            G_rsp_status = l_rc;
        }
        else
        {
            CMDH_TRAC_ERR("cmdh_tmgt_poll: Invalid version 0x%02X", l_poll_cmd->version);
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }
    } while(0);

    if(l_rc)
    {
        // Build Error Response packet
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, l_rc, &l_errlHndl);
    }

    return l_errlHndl;
}


// Function Specification
//
// Name:  cmdh_poll_v20
//
// Description: Used for version 0x20 poll calls from BMC/HTMGT.
//
// End Function Specification
ERRL_RC cmdh_poll_v20(cmdh_fsp_rsp_t * o_rsp_ptr)
{
    ERRL_RC                     l_rc  = ERRL_RC_INTERNAL_FAIL;
    int                         k = 0, l_max_sensors = 0;
    int                         l_err_hist_idx = 0, l_sens_list_idx = 0;
    cmdh_poll_sensor_db_t       l_sensorHeader;
    static unsigned int         L_num_hcode_elogs = 0;

    // Set pointer to start of o_rsp_ptr
    cmdh_poll_resp_v20_fixed_t * l_poll_rsp = (cmdh_poll_resp_v20_fixed_t *) o_rsp_ptr;

    // Byte 1
    l_poll_rsp->status.word = SMGR_validate_get_valid_states();

    // Byte 2
    l_poll_rsp->ext_status.word = 0;

    //SET DVFS bits
    for ( k = 0; k < MAX_NUM_CORES; k++ )
    {
        uint32_t l_freq_reason = g_amec->proc[0].core[k].f_reason;
        if ( l_freq_reason & AMEC_VOTING_REASON_PROC_THRM )
        {
            // only set DVFS bit if throttling below frequency to report throttling
            if(G_amec_opal_proc_throt_reason == CPU_OVERTEMP)
            {
               l_poll_rsp->ext_status.dvfs_due_to_ot = 1;
            }
        }

        if ( l_freq_reason & AMEC_VOTING_REASON_VDD_THRM )
        {
            // only set DVFS bit if throttling below frequency to report throttling
            if(G_amec_opal_proc_throt_reason == VDD_OVERTEMP)
            {
               l_poll_rsp->ext_status.dvfs_due_to_vdd_ot = 1;
            }
        }

        if ( l_freq_reason & (AMEC_VOTING_REASON_PPB | AMEC_VOTING_REASON_PMAX | AMEC_VOTING_REASON_PWR) )
        {
            // only set DVFS bit if throttling below frequency to report throttling
            if(G_amec_opal_proc_throt_reason == POWERCAP)
            {
               l_poll_rsp->ext_status.dvfs_due_to_pwr = 1;
            }
        }
    }

    //If memory is being throttled due to OverTemp or due to Failure to read sensors set mthrot_due_to_ot bit.
    if (((g_amec->mem_throttle_reason == AMEC_MEM_VOTING_REASON_DIMM) ||
         (g_amec->mem_throttle_reason == AMEC_MEM_VOTING_REASON_MEMBUF)))
    {
        l_poll_rsp->ext_status.mthrot_due_to_ot = 1;
    }

    //If we are in oversubscription, set the N_power bit.
    if( AMEC_INTF_GET_OVERSUBSCRIPTION() )
    {
        l_poll_rsp->ext_status.n_power = 1;
    }

    // Byte 3
    l_poll_rsp->occ_pres_mask   = G_sysConfigData.is_occ_present;
    // Byte 4
    l_poll_rsp->config_data     = DATA_request_cnfgdata();
    // Byte 5
    l_poll_rsp->state           = CURRENT_STATE();
    // Byte 6
    l_poll_rsp->mode            = CURRENT_MODE();
    // Byte 7
    l_poll_rsp->ips_status.word = 0;
    l_poll_rsp->ips_status.ips_enabled = G_ips_config_data.iv_ipsEnabled;
    l_poll_rsp->ips_status.ips_active = AMEC_mst_get_ips_active_status();
    // Error Log:
    bool check_405_elogs = true;
    // if (405 has no elogs) OR (have not hit max consecutive hcode elogs)
    if ((getOldestErrlID() == 0) || (L_num_hcode_elogs < MAX_CONSECUTIVE_HCODE_ELOGS))
    {
        // Check for HCODE errors
        hcode_elog_entry_t elog_entry;
        unsigned int index = 0;
        for (; index < G_hcode_elog_table_slots; ++index)
        {
            elog_entry.dw0.value = in64(&G_hcode_elog_table[index]);
            if (elog_entry.dw0.value != 0)
            { // Found HCODE elog
                if (elog_entry.dw0.fields.errlog_src != ERRL_SOURCE_405)
                {
                    ++L_num_hcode_elogs;
                    // Byte 8:
                    l_poll_rsp->errl_id         = elog_entry.dw0.fields.errlog_id;
                    // Byte 9 - 12:
                    l_poll_rsp->errl_address    = elog_entry.dw0.fields.errlog_addr;
                    // Byte 13 - 14:
                    l_poll_rsp->errl_length     = elog_entry.dw0.fields.errlog_len;
                    // Byte 15:
                    l_poll_rsp->errl_source     = elog_entry.dw0.fields.errlog_src;
                    check_405_elogs = false;
                    break;
                }
                else
                {
                    TRAC_ERR("cmdh_poll_v20: ignoring HCODE error with 405 source (id:0x%02X, len:0x%04X, address:0x%08X)",
                             elog_entry.dw0.fields.errlog_id, elog_entry.dw0.fields.errlog_len, elog_entry.dw0.fields.errlog_addr);
                    // Zero out error log entry in list so hcode can reuse
                    out64(&G_hcode_elog_table[index], 0);
                    G_htmgt_notified_of_error = false;
                }
            }
        }
    }
    if (check_405_elogs)
    { // No, HCODE errors, check/add any 405 elog
        L_num_hcode_elogs = 0;
        // Byte 8:
        l_poll_rsp->errl_id         = getOldestErrlID();
        // Byte 9 - 12:
        l_poll_rsp->errl_address    = getErrlOCIAddrByID(l_poll_rsp->errl_id);
        // Byte 13 - 14:
        l_poll_rsp->errl_length     = getErrlLengthByID(l_poll_rsp->errl_id);
        // Byte 15:
        l_poll_rsp->errl_source     = ERRL_SOURCE_405;
    }
    //If errl_id is not 0, then neither address or length should be zero.
    //This should not happen, but if it does TMGT will create an error log that
    //includes the data at the errl slot address given that can be used for debug.
    //NOTE: One cause for a false errlog id is corruption of data in one errl slot
    //      due to writing data greater than the size of the previous slot.  For
    //      example writing the CallHome errorlog (3kb) into a regular sized (2kb) slot.
    //      Make sure to verify the order of the memory allocation for the errl slots.
    if ( (l_poll_rsp->errl_id != 0) &&
         ((l_poll_rsp->errl_address == 0) || (l_poll_rsp->errl_length == 0)))
    {
        TRAC_ERR("cmdh_poll_v20: error log sent with bad data "
                 "(id:0x%02X, source:0x%02X, len:0x%04X, address:0x%08X)",
                 l_poll_rsp->errl_id, l_poll_rsp->errl_source, l_poll_rsp->errl_length, l_poll_rsp->errl_address);
    }

    // Byte 16: GPU Configuration
    l_poll_rsp->gpu_presence = (uint8_t)G_first_proc_gpu_config;

    // Byte 17 - 32 (16 bytes): OCC level
    memcpy( (void *) l_poll_rsp->occ_level, (void *) &G_occ_buildname[0], 16);

    // Byte 33 - 38:
    char l_sensor_ec[6] = "SENSOR";
    memcpy( (void *) l_poll_rsp->sensor_ec, (void *) &l_sensor_ec[0], (size_t) sizeof(l_sensor_ec));
    // Byte 39:
    l_poll_rsp->sensor_dblock_count = 0;
    // Byte 40:
    l_poll_rsp->sensor_dblock_version = 0x01;  //Currently only 0x01 is supported.

    //l_rsp_index is used as an index into o_rsp_ptr
    uint16_t l_rsp_index = CMDH_POLL_RESP_LEN_V20;

    ////////////////////
    // TEMP sensors:
    // Generate datablock header for temp sensors and sensor data.

    // Set up the header
    memset((void*) &l_sensorHeader, 0, (size_t)sizeof(cmdh_poll_sensor_db_t));
    memcpy ((void *) &(l_sensorHeader.eyecatcher[0]), SENSOR_TEMP, 4);
    l_sensorHeader.format = 0x02;
    l_sensorHeader.length = sizeof(cmdh_poll_temp_sensor_t);
    l_sensorHeader.count  = 0;

    //Initialize to max number of possible temperature sensors.
    l_max_sensors = MAX_NUM_CORES + MAX_NUM_MEM_CONTROLLERS + (MAX_NUM_MEM_CONTROLLERS * NUM_DIMMS_PER_OCMB) + (MAX_NUM_GPU_PER_DOMAIN * 2);
    l_max_sensors++;  // +1 for VRM
    cmdh_poll_temp_sensor_t l_tempSensorList[l_max_sensors];
    memset(l_tempSensorList, 0x00, sizeof(l_tempSensorList));

    // Add the core temperatures
    for (k=0; k<MAX_NUM_CORES; k++)
    {
        if(CORE_PRESENT(k))
        {
            l_tempSensorList[l_sensorHeader.count].id = G_amec_sensor_list[TEMPPROCTHRMC0 + k]->ipmi_sid;
            l_tempSensorList[l_sensorHeader.count].fru_type = DATA_FRU_PROC;
            l_tempSensorList[l_sensorHeader.count].value = (G_amec_sensor_list[TEMPPROCTHRMC0 + k]->sample) & 0xFF;
            l_sensorHeader.count++;
        }
    }

    // Add the memory temperatures
    uint8_t l_membuf, l_dimm = 0;
    static bool l_traced_missing_sid = FALSE;
    for (l_membuf=0; l_membuf < MAX_NUM_MEM_CONTROLLERS; l_membuf++)
    {
        if (MEMBUF_PRESENT(l_membuf))
        {
            //Add entry for membufs.
            uint32_t l_temp_sid = g_amec->proc[0].memctl[l_membuf].membuf.temp_sid;
            l_tempSensorList[l_sensorHeader.count].id = l_temp_sid;
            l_tempSensorList[l_sensorHeader.count].fru_type = DATA_FRU_MEMBUF;
            if (G_membuf_timeout_logged_bitmap & (MEMBUF0_PRESENT_MASK >> l_membuf))
            {
                l_tempSensorList[l_sensorHeader.count].value = 0xFF;
            }
            else
            {
                l_tempSensorList[l_sensorHeader.count].value = (g_amec->proc[0].memctl[l_membuf].membuf.membuf_hottest.cur_temp) & 0xFF;
            }

            l_sensorHeader.count++;

            //Add entries for present dimms associated with current memory buffer l_membuf.
            for(l_dimm=0; l_dimm < NUM_DIMMS_PER_OCMB; l_dimm++)
            {
                l_temp_sid = g_amec->proc[0].memctl[l_membuf].membuf.dimm_temps[l_dimm].temp_sid;

                if((FSP_SUPPORTED_OCC == G_occ_interrupt_type) && (l_temp_sid == 0) && (g_amec->proc[0].memctl[l_membuf].membuf.dimm_temps[l_dimm].cur_temp != 0))
                {
                    if (!l_traced_missing_sid)
                    {
                        CMDH_TRAC_ERR("cmdh_poll_v20: DIMM%04X sensor not defined but temperature was non-zero", ((l_membuf << 8)|l_dimm));
                        l_traced_missing_sid = TRUE;
                    }
                    l_temp_sid = 1 + l_dimm; // If sid is zero them make up a sid for FSP
                }

                if (l_temp_sid != 0)
                {
                    l_tempSensorList[l_sensorHeader.count].id = l_temp_sid;
                    l_tempSensorList[l_sensorHeader.count].fru_type = DATA_FRU_DIMM;
                    //If a dimm timed out long enough, we should return 0xFFFF for that sensor.
                    if (G_dimm_temp_expired_bitmap.bytes[l_membuf] & (DIMM_SENSOR0 >> l_dimm))
                    {
                        l_tempSensorList[l_sensorHeader.count].value = 0xFF;
                    }
                    else
                    {
                        l_tempSensorList[l_sensorHeader.count].value = (g_amec->proc[0].memctl[l_membuf].membuf.dimm_temps[l_dimm].cur_temp & 0xFF);
                    }

                    l_sensorHeader.count++;
                }
            }
        }
    }

    if (G_avsbus_vdd_monitoring)
    {
        // Add Vdd temp
        const sensor_t *tempvdd = getSensorByGsid(TEMPVDD);
        if (tempvdd != NULL)
        {
            l_tempSensorList[l_sensorHeader.count].id = AMECSENSOR_PTR(TEMPVDD)->ipmi_sid;
            l_tempSensorList[l_sensorHeader.count].fru_type = DATA_FRU_VRM_VDD;
            if (G_vrm_vdd_temp_expired)
            {
                l_tempSensorList[l_sensorHeader.count].value = 0xFF;
            }
            else
            {
                l_tempSensorList[l_sensorHeader.count].value = tempvdd->sample & 0xFF;
            }
            l_sensorHeader.count++;
        }
    }

    // Add GPU temperatures
    for (k=0; k<MAX_NUM_GPU_PER_DOMAIN; k++)
    {
        if(GPU_PRESENT(k))
        {
            // GPU core temperature
            l_tempSensorList[l_sensorHeader.count].id = G_amec_sensor_list[TEMPGPU0 + k]->ipmi_sid;
            l_tempSensorList[l_sensorHeader.count].fru_type = DATA_FRU_GPU;
            if(g_amec->gpu[k].status.coreTempFailure)
            {
               // failed to read core temperature return 0xFF
               l_tempSensorList[l_sensorHeader.count].value = 0xFF;
            }
            else if(g_amec->gpu[k].status.coreTempNotAvailable)
            {
               // core temperature not available return 0
               l_tempSensorList[l_sensorHeader.count].value = 0;
            }
            else
            {
               // have a good core temperature return the reading
               l_tempSensorList[l_sensorHeader.count].value = (G_amec_sensor_list[TEMPGPU0 + k]->sample) & 0xFF;
            }
            l_sensorHeader.count++;

            // GPU memory temperature
            l_tempSensorList[l_sensorHeader.count].id = G_amec_sensor_list[TEMPGPU0MEM + k]->ipmi_sid;
            l_tempSensorList[l_sensorHeader.count].fru_type = DATA_FRU_GPU_MEM;
            if(g_amec->gpu[k].status.memTempFailure)
            {
               // failed to read memory temperature return 0xFF
               l_tempSensorList[l_sensorHeader.count].value = 0xFF;
            }
            else if(g_amec->gpu[k].status.memTempNotAvailable)
            {
               // memory temperature not available return 0
               l_tempSensorList[l_sensorHeader.count].value = 0;
            }
            else
            {
               // have a good memory temperature return the reading
               l_tempSensorList[l_sensorHeader.count].value = (G_amec_sensor_list[TEMPGPU0MEM + k]->sample) & 0xFF;
            }
            l_sensorHeader.count++;
        }
    }

    // Copy header first.
    memcpy ((void *) &(o_rsp_ptr->data[l_rsp_index]), (void *)&l_sensorHeader, sizeof(l_sensorHeader));
    // Increment index into response buffer.
    l_rsp_index += sizeof(l_sensorHeader);
    l_poll_rsp->sensor_dblock_count +=1;
    // Write data to resp buffer if any.
    if (l_sensorHeader.count)
    {
        int l_sensordataSz = l_sensorHeader.count * l_sensorHeader.length;
        // Copy sensor data into response buffer.
        memcpy ((void *) &(o_rsp_ptr->data[l_rsp_index]), (void *)l_tempSensorList, l_sensordataSz);
        // Increment index into response buffer.
        l_rsp_index += l_sensordataSz;

    }

    ///////////////////
    // FREQ Sensors:
    // Generate datablock header for freq sensors and sensor data.
    memset((void*) &l_sensorHeader, 0, (size_t)sizeof(cmdh_poll_sensor_db_t));
    memcpy ((void *) &(l_sensorHeader.eyecatcher[0]), SENSOR_FREQ, 4);
    l_sensorHeader.format = 0x02;
    l_sensorHeader.length = sizeof(cmdh_poll_freq_sensor_t);
    l_sensorHeader.count  = 0;

    cmdh_poll_freq_sensor_t l_freqSensorList[MAX_NUM_CORES];
    for (k=0; k<MAX_NUM_CORES; k++)
    {
        if(CORE_PRESENT(k))
        {
            l_freqSensorList[l_sensorHeader.count].id = G_amec_sensor_list[FREQAC0 + k]->ipmi_sid;
            l_freqSensorList[l_sensorHeader.count].value = G_amec_sensor_list[FREQAC0 + k]->sample;
            l_sensorHeader.count++;
        }
    }

    // Copy header to response buffer.
    memcpy ((void *) &(o_rsp_ptr->data[l_rsp_index]), (void *)&l_sensorHeader, sizeof(l_sensorHeader));
    //Increment index into response buffer.
    l_rsp_index += sizeof(l_sensorHeader);
    l_poll_rsp->sensor_dblock_count +=1;
    // Write data to outbuffer if any.
    if (l_sensorHeader.count)
    {
        int l_sensordataSz = l_sensorHeader.count * l_sensorHeader.length;
        // Copy sensor data into response buffer.
        memcpy ((void *) &(o_rsp_ptr->data[l_rsp_index]), (void *)l_freqSensorList, l_sensordataSz);
        // Increment index into response buffer.
        l_rsp_index += l_sensordataSz;
    }

    /////////////////////
    // POWR Sensors:
    // Generate datablock header for power sensors and sensor data.
    // If APSS is present return format version 0x02 by MASTER ONLY.
    // If no APSS present return format version 0xA0 by all OCCs.
    if ( (G_occ_role == OCC_MASTER) && (G_pwr_reading_type == PWR_READING_TYPE_APSS) )
    {
        memset((void*) &l_sensorHeader, 0, (size_t)sizeof(cmdh_poll_sensor_db_t));
        memcpy ((void *) &(l_sensorHeader.eyecatcher[0]), SENSOR_POWR, 4);
        l_sensorHeader.format = 0x02;
        l_sensorHeader.length = sizeof(cmdh_poll_power_sensor_t);
        l_sensorHeader.count  = 0;

        // Generate sensor list.
        cmdh_poll_power_sensor_t l_pwrSensorList[MAX_APSS_ADC_CHANNELS];
        for (k = 0; k < MAX_APSS_ADC_CHANNELS; k++)
        {
            const uint8_t function_id = G_apss_ch_to_function[k];
            if ((function_id != ADC_12V_SENSE) &&
                (function_id != ADC_GND_REMOTE_SENSE) &&
                (function_id != ADC_12V_STANDBY_CURRENT) &&
                (function_id != ADC_VOLT_SENSE_2))
            {
                // only return channel power for GPU if the GPU is present
                bool include = FALSE;
                if  ( (function_id == ADC_GPU_0_0) ||
                      ((function_id >= ADC_GPU_0_1) && (function_id <= ADC_GPU_VOLT2_1_1)) )
                {
                    // GPU channel: include if has a non-zero reading or if GPU is present
                    if ( ( G_amec_sensor_list[PWRAPSSCH0 + k]->sample > 0) ||
                         ( ((ADC_GPU_0_0 == function_id) || (ADC_GPU_VOLT2_0_0 == function_id)) &&
                           (G_first_sys_gpu_config & 0x01) ) ||
                         ( ((ADC_GPU_0_1 == function_id) || (ADC_GPU_VOLT2_0_1 == function_id)) &&
                           (G_first_sys_gpu_config & 0x02) ) ||
                         ( ( ADC_GPU_0_2 == function_id) &&
                           (G_first_sys_gpu_config & 0x04) ) ||
                         ( ((ADC_GPU_1_0 == function_id) || (ADC_GPU_VOLT2_1_0 == function_id)) &&
                           (G_first_sys_gpu_config & 0x08) ) ||
                         ( ((ADC_GPU_1_1 == function_id) || (ADC_GPU_VOLT2_1_1 == function_id)) &&
                           (G_first_sys_gpu_config & 0x10) ) ||
                         ( ( ADC_GPU_1_2 == function_id) &&
                           (G_first_sys_gpu_config & 0x20) ) )
                    {
                        // GPU is present
                        include = TRUE;
                    }
                }
                else
                {
                    // non-GPU channel
                    include = TRUE;
                }
                if (include)
                {
                    l_pwrSensorList[l_sensorHeader.count].id = G_amec_sensor_list[PWRAPSSCH0 + k]->ipmi_sid;
                    l_pwrSensorList[l_sensorHeader.count].function_id = function_id;
                    l_pwrSensorList[l_sensorHeader.count].apss_channel = k;
                    l_pwrSensorList[l_sensorHeader.count].reserved = 0;
                    l_pwrSensorList[l_sensorHeader.count].current = G_amec_sensor_list[PWRAPSSCH0 + k]->sample;
                    l_pwrSensorList[l_sensorHeader.count].accumul = G_amec_sensor_list[PWRAPSSCH0 + k]->accumulator;
                    l_pwrSensorList[l_sensorHeader.count].update_tag  = G_amec_sensor_list[PWRAPSSCH0 + k]->update_tag;
                    l_sensorHeader.count++;
                }
            }
        }

        // Copy header to response buffer.
        memcpy ((void *) &(o_rsp_ptr->data[l_rsp_index]), (void *)&l_sensorHeader, sizeof(l_sensorHeader));
        // Increment index into response buffer.
        l_rsp_index += sizeof(l_sensorHeader);
        l_poll_rsp->sensor_dblock_count +=1;
        // Write data to resp buffer if any.
        if (l_sensorHeader.count)
        {
            uint16_t l_sensordataSz = l_sensorHeader.count * l_sensorHeader.length;
            // Copy sensor data into response buffer.
            memcpy ((void *) &(o_rsp_ptr->data[l_rsp_index]), (void *)l_pwrSensorList, l_sensordataSz);
            // Increment index into response buffer.
            l_rsp_index += l_sensordataSz;
        }
    }

    else if (G_pwr_reading_type != PWR_READING_TYPE_APSS)
    {
        memset((void*) &l_sensorHeader, 0, (size_t)sizeof(cmdh_poll_sensor_db_t));
        memcpy ((void *) &(l_sensorHeader.eyecatcher[0]), SENSOR_POWR, 4);
        l_sensorHeader.format = 0xA0;
        l_sensorHeader.length = sizeof(cmdh_poll_power_no_apss_sensor_t);
        l_sensorHeader.count  = 1;

        cmdh_poll_power_no_apss_sensor_t l_pwrData;
        memset((void*) &l_pwrData, 0, (size_t)sizeof(cmdh_poll_power_no_apss_sensor_t));

        // if there is a non-APSS chip for system power fill in system power else return 0's
        if(G_pwr_reading_type != PWR_READING_TYPE_NONE)
        {
            l_pwrData.sys_pwr_id = G_amec_sensor_list[PWRSYS]->ipmi_sid;
            l_pwrData.sys_pwr_update_time = G_mics_per_tick; // system power is read every tick
            l_pwrData.sys_pwr_current = G_amec_sensor_list[PWRSYS]->sample;
            l_pwrData.sys_pwr_update_tag = G_amec_sensor_list[PWRSYS]->update_tag;
            l_pwrData.sys_pwr_accumul = G_amec_sensor_list[PWRSYS]->accumulator;
        }

        // Proc power is from AVS bus, return readings if reading Vdd and Vdn else return 0's
        if( (G_avsbus_vdd_monitoring) && (G_avsbus_vdn_monitoring) )
        {
            // when no APSS present proc readings are updated based on AVS timing use PWRVDD/N timing (2 ticks)
            l_pwrData.proc_pwr_update_time = G_mics_per_tick * 2;
            l_pwrData.proc_pwr_current = G_amec_sensor_list[PWRPROC]->sample;
            l_pwrData.proc_pwr_update_tag = G_amec_sensor_list[PWRPROC]->update_tag;
            l_pwrData.proc_pwr_accumul = G_amec_sensor_list[PWRPROC]->accumulator;
            l_pwrData.vdd_pwr_current = G_amec_sensor_list[PWRVDD]->sample;
            l_pwrData.vdd_pwr_update_tag = G_amec_sensor_list[PWRVDD]->update_tag;
            l_pwrData.vdd_pwr_accumul = G_amec_sensor_list[PWRVDD]->accumulator;
            l_pwrData.vdn_pwr_current = G_amec_sensor_list[PWRVDN]->sample;
            l_pwrData.vdn_pwr_update_tag = G_amec_sensor_list[PWRVDN]->update_tag;
            l_pwrData.vdn_pwr_accumul = G_amec_sensor_list[PWRVDN]->accumulator;
        }

        // Copy header to response buffer.
        memcpy ((void *) &(o_rsp_ptr->data[l_rsp_index]),
                (void *)&l_sensorHeader, sizeof(l_sensorHeader));
        // Increment index into response buffer.
        l_rsp_index += sizeof(l_sensorHeader);

        // Copy sensor data into response buffer.
        memcpy ((void *) &(o_rsp_ptr->data[l_rsp_index]),
                (void *)&(l_pwrData), sizeof(cmdh_poll_power_no_apss_sensor_t));
        // Increment index into response buffer.
        l_rsp_index += sizeof(cmdh_poll_power_no_apss_sensor_t);

        l_poll_rsp->sensor_dblock_count +=1;
    }

    ////////////////////////
    // POWER CAPS:
    // Generate datablock header for power caps.  RETURNED by MASTER ONLY.
    if (G_occ_role == OCC_MASTER)
    {
        memset((void*) &l_sensorHeader, 0, (size_t)sizeof(cmdh_poll_sensor_db_t));
        memcpy ((void *) &(l_sensorHeader.eyecatcher[0]), SENSOR_CAPS, 4);
        l_sensorHeader.format = 0x03;
        l_sensorHeader.length = sizeof(cmdh_poll_pcaps_sensor_t);
        l_sensorHeader.count  = 1;

        cmdh_poll_pcaps_sensor_t l_pcapData;
        memset((void*) &l_pcapData, 0, (size_t)sizeof(cmdh_poll_pcaps_sensor_t));

        // Return 0's for power cap section if there is no system power reading
        // OCC can't support power capping without knowing the system power
        if(G_pwr_reading_type != PWR_READING_TYPE_NONE)
        {
            if ((G_sysConfigData.system_type.non_redund_ps == false) ||
                (! AMEC_INTF_GET_OVERSUBSCRIPTION()))
            {
                l_pcapData.current = g_amec->pcap.active_node_pcap;
            }
            // else OCC is not running pcap algorithim so leave current cap as 0
            l_pcapData.system = G_amec_sensor_list[PWRSYS]->sample;
            l_pcapData.n = G_sysConfigData.pcap.oversub_pcap;
            l_pcapData.max = G_sysConfigData.pcap.max_pcap;
            l_pcapData.hard_min = G_sysConfigData.pcap.hard_min_pcap;
            l_pcapData.soft_min = G_sysConfigData.pcap.soft_min_pcap;
            l_pcapData.user = G_sysConfigData.pcap.current_pcap;
            l_pcapData.source = G_sysConfigData.pcap.source;
        }

        // Copy header to response buffer.
        memcpy ((void *) &(o_rsp_ptr->data[l_rsp_index]),
                (void *)&l_sensorHeader, sizeof(l_sensorHeader));
        // Increment index into response buffer.
        l_rsp_index += sizeof(l_sensorHeader);

        // Copy sensor data into response buffer.
        memcpy ((void *) &(o_rsp_ptr->data[l_rsp_index]),
                (void *)&(l_pcapData), sizeof(cmdh_poll_pcaps_sensor_t));
        // Increment index into response buffer.
        l_rsp_index += sizeof(cmdh_poll_pcaps_sensor_t);

        l_poll_rsp->sensor_dblock_count +=1;
    }

    ///////////////////
    // EXTN Sensors:
    // Generate datablock header for freq sensors and sensor data.
    memset((void*) &l_sensorHeader, 0, (size_t)sizeof(cmdh_poll_sensor_db_t));
    memcpy ((void *) &(l_sensorHeader.eyecatcher[0]), SENSOR_EXTN, 4);
    l_sensorHeader.format = 0x01;
    l_sensorHeader.length = sizeof(cmdh_poll_extn_sensor_t);
    l_sensorHeader.count  = 0;

    cmdh_poll_extn_sensor_t l_extnSensorList[MAX_EXTN_SENSORS] = {{0}};
    l_extnSensorList[l_sensorHeader.count].name = EXTN_NAME_FMIN;
    uint16_t freq = G_sysConfigData.sys_mode_freq.table[OCC_MODE_MIN_FREQUENCY];
    l_extnSensorList[l_sensorHeader.count].data[0] = proc_freq2pstate(freq);
    l_extnSensorList[l_sensorHeader.count].data[1] = CONVERT_UINT16_UINT8_HIGH(freq);
    l_extnSensorList[l_sensorHeader.count].data[2] = CONVERT_UINT16_UINT8_LOW(freq);
    l_sensorHeader.count++;
    l_extnSensorList[l_sensorHeader.count].name = EXTN_NAME_FNOM;
    freq = G_sysConfigData.sys_mode_freq.table[OCC_MODE_NOMINAL];
    l_extnSensorList[l_sensorHeader.count].data[0] = proc_freq2pstate(freq);
    l_extnSensorList[l_sensorHeader.count].data[1] = CONVERT_UINT16_UINT8_HIGH(freq);
    l_extnSensorList[l_sensorHeader.count].data[2] = CONVERT_UINT16_UINT8_LOW(freq);
    l_sensorHeader.count++;
    l_extnSensorList[l_sensorHeader.count].name = EXTN_NAME_FTURBO;
    freq = G_sysConfigData.sys_mode_freq.table[OCC_MODE_TURBO];
    if (freq > 0)
    {
        l_extnSensorList[l_sensorHeader.count].data[0] = proc_freq2pstate(freq);
        l_extnSensorList[l_sensorHeader.count].data[1] = CONVERT_UINT16_UINT8_HIGH(freq);
        l_extnSensorList[l_sensorHeader.count].data[2] = CONVERT_UINT16_UINT8_LOW(freq);
    }
    l_sensorHeader.count++;
    l_extnSensorList[l_sensorHeader.count].name = EXTN_NAME_FUTURBO;
    freq = G_sysConfigData.sys_mode_freq.table[OCC_MODE_UTURBO];
    if (freq > 0)
    {
        l_extnSensorList[l_sensorHeader.count].data[0] = proc_freq2pstate(freq);
        l_extnSensorList[l_sensorHeader.count].data[1] = CONVERT_UINT16_UINT8_HIGH(freq);
        l_extnSensorList[l_sensorHeader.count].data[2] = CONVERT_UINT16_UINT8_LOW(freq);
    }
    l_sensorHeader.count++;

    l_extnSensorList[l_sensorHeader.count].name = EXTN_NAME_CLIP;
    // get Pstate for the current minimum maximum frequency OCC is allowing
    // actual frequency is driven down by the lowest max frequency across all cores
    freq = g_amec->proc[0].core_min_freq;
    if (freq > 0)
    {
        l_extnSensorList[l_sensorHeader.count].data[0] = proc_freq2pstate(freq);
    }
    else
    {
        l_extnSensorList[l_sensorHeader.count].data[0] = 0xFF;
    }

    // current counter will be 0 if not currently clipping
    l_extnSensorList[l_sensorHeader.count].data[1] = g_amec->proc[0].current_clip_count;
    // clip history reason
    l_extnSensorList[l_sensorHeader.count].data[2] = CONVERT_UINT32_UINT8_UPPER_HIGH(g_amec->proc[0].chip_f_reason_history);
    l_extnSensorList[l_sensorHeader.count].data[3] = CONVERT_UINT32_UINT8_UPPER_LOW(g_amec->proc[0].chip_f_reason_history);
    l_extnSensorList[l_sensorHeader.count].data[4] = CONVERT_UINT32_UINT8_LOWER_HIGH(g_amec->proc[0].chip_f_reason_history);
    l_extnSensorList[l_sensorHeader.count].data[5] = CONVERT_UINT32_UINT8_LOWER_LOW(g_amec->proc[0].chip_f_reason_history);
    l_sensorHeader.count++;

    // WOF clip info from PGPE
    l_extnSensorList[l_sensorHeader.count].name = EXTN_NAME_WOFC;
    if(g_amec->wof.wof_disabled)
    {
        // WOF disabled put 0xFF for clip followed by WOF disable reason
        l_extnSensorList[l_sensorHeader.count].data[0] = 0xFF;
        l_extnSensorList[l_sensorHeader.count].data[1] = 0x00;
        l_extnSensorList[l_sensorHeader.count].data[2] = CONVERT_UINT32_UINT8_UPPER_HIGH(g_amec->wof.wof_disabled);
        l_extnSensorList[l_sensorHeader.count].data[3] = CONVERT_UINT32_UINT8_UPPER_LOW(g_amec->wof.wof_disabled);
        l_extnSensorList[l_sensorHeader.count].data[4] = CONVERT_UINT32_UINT8_LOWER_HIGH(g_amec->wof.wof_disabled);
        l_extnSensorList[l_sensorHeader.count].data[5] = CONVERT_UINT32_UINT8_LOWER_LOW(g_amec->wof.wof_disabled);
    }
    else
    {
        // WOF is enabled return WOF information from PGPE
        // These are read from PGPE shared SRAM every WOF cycle
        l_extnSensorList[l_sensorHeader.count].data[0] = g_amec->wof.f_clip_ps;
        l_extnSensorList[l_sensorHeader.count].data[1] = CONVERT_UINT16_UINT8_HIGH(g_amec->wof.v_clip);
        l_extnSensorList[l_sensorHeader.count].data[2] = CONVERT_UINT16_UINT8_LOW(g_amec->wof.v_clip);
        l_extnSensorList[l_sensorHeader.count].data[3] = CONVERT_UINT16_UINT8_HIGH(g_amec->wof.v_ratio);
        l_extnSensorList[l_sensorHeader.count].data[4] = CONVERT_UINT16_UINT8_LOW(g_amec->wof.v_ratio);
    }
    l_sensorHeader.count++;

    // add any non-0 error history counts
    for(l_err_hist_idx=0; l_err_hist_idx < ERR_HISTORY_SIZE; l_err_hist_idx++)
    {
        if(G_error_history[l_err_hist_idx])
        {
            if(l_sens_list_idx == 0)
            {
                // first one to add fill in name
                l_extnSensorList[l_sensorHeader.count].name = EXTN_NAME_ERRHIST;
                l_extnSensorList[l_sensorHeader.count].data[l_sens_list_idx] = l_err_hist_idx;
                l_sens_list_idx++;
                l_extnSensorList[l_sensorHeader.count].data[l_sens_list_idx] = G_error_history[l_err_hist_idx];
                l_sens_list_idx++;
            }
            else if(l_sens_list_idx < 5)  // room in current extended error history sensor?
            {
                l_extnSensorList[l_sensorHeader.count].data[l_sens_list_idx] = l_err_hist_idx;
                l_sens_list_idx++;
                l_extnSensorList[l_sensorHeader.count].data[l_sens_list_idx] = G_error_history[l_err_hist_idx];
                l_sens_list_idx++;
            }
            else // no room start another extended error history sensor
            {
                l_sensorHeader.count++;
                l_extnSensorList[l_sensorHeader.count].name = EXTN_NAME_ERRHIST;
                l_sens_list_idx = 0;
                l_extnSensorList[l_sensorHeader.count].data[l_sens_list_idx] = l_err_hist_idx;
                l_sens_list_idx++;
                l_extnSensorList[l_sensorHeader.count].data[l_sens_list_idx] = G_error_history[l_err_hist_idx];
                l_sens_list_idx++;
            }
        }
    }
    if(l_sens_list_idx)
    {
        l_sensorHeader.count++;
    }


    // Copy header to response buffer.
    memcpy ((void *) &(o_rsp_ptr->data[l_rsp_index]), (void *)&l_sensorHeader, sizeof(l_sensorHeader));
    //Increment index into response buffer.
    l_rsp_index += sizeof(l_sensorHeader);
    l_poll_rsp->sensor_dblock_count +=1;
    // Write data to outbuffer if any.
    if (l_sensorHeader.count)
    {
        int l_sensordataSz = l_sensorHeader.count * l_sensorHeader.length;
        // Copy sensor data into response buffer.
        memcpy ((void *) &(o_rsp_ptr->data[l_rsp_index]), (void *)l_extnSensorList, l_sensordataSz);
        // Increment index into response buffer.
        l_rsp_index += l_sensordataSz;
    }


    l_poll_rsp->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_rsp_index);
    l_poll_rsp->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_rsp_index);
    l_rc                       = ERRL_RC_SUCCESS;
    // Response status is returned (must be written to rsp buffer last)

    return l_rc;
}



// Function Specification
//
// Name:  cmdh_reset_prep_t
//
// Description: Process reset prep command
//
// End Function Specification
errlHndl_t cmdh_reset_prep (const cmdh_fsp_cmd_t * i_cmd_ptr,
                                  cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t                  l_errlHndl = NULL;
    cmdh_reset_prep_t *         l_cmd_ptr = (cmdh_reset_prep_t *) i_cmd_ptr;
    ERRL_RC                     l_rc = ERRL_RC_SUCCESS;
    bool                        l_ffdc = FALSE;

    G_rsp_status = ERRL_RC_SUCCESS;
    o_rsp_ptr->data_length[0] = 0;
    o_rsp_ptr->data_length[1] = 0;

    do
    {
        // Command Length Check - make sure we at least have a version number
        if( CMDH_DATALEN_FIELD_UINT16(i_cmd_ptr) < CMDH_RESET_PREP_MIN_DATALEN)
        {
            l_rc = ERRL_RC_INVALID_CMD_LEN;
            break;
        }

        // Version Number Check
        if(l_cmd_ptr->version != CMDH_RESET_PREP_VERSION)
        {
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }

        TRAC_IMP("cmdh_reset_prep: Prep for reset command received! reason[0x%.2X]",
                 l_cmd_ptr->reason);

        G_reset_prep = true;

        // Command Handling
        switch( l_cmd_ptr->reason )
        {
            case CMDH_PREP_NONFAILURE:
                // No FFDC Error Log Needed
                l_rc = ERRL_RC_SUCCESS;
                break;

            case CMDH_PREP_FAILON_THISOCC:
                l_ffdc = TRUE;
                l_rc = ERRL_RC_SUCCESS;
                break;

            case CMDH_PREP_FAILON_OTHEROCC:
                // If OCC is master, we may want to generate FFDC log
                if (G_occ_role == OCC_MASTER)
                {
                    l_ffdc = TRUE;
                }
                l_rc = ERRL_RC_SUCCESS;
                break;

            case CMDH_PREP_FAILON_OTHERNODE:
                // No FFDC Error Log Needed
                l_rc = ERRL_RC_SUCCESS;
                break;

            case CMDH_PREP_POWER_OFF:
                // System powering off, stop DCOM and other tasks that still run in standby
                rtl_stop_task(TASK_ID_DCOM_WAIT_4_MSTR);
                rtl_stop_task(TASK_ID_DCOM_RX_INBX);
                rtl_stop_task(TASK_ID_DCOM_TX_INBX);
                rtl_stop_task(TASK_ID_DCOM_RX_OUTBX);
                rtl_stop_task(TASK_ID_DCOM_TX_OUTBX);
                rtl_stop_task(TASK_ID_DCOM_PARSE_FW_MSG);
                rtl_stop_task(TASK_ID_MISC_405_CHECKS);
                rtl_stop_task(TASK_ID_POKE_WDT);

                l_rc = ERRL_RC_SUCCESS;
                break;


            default:
                l_rc = ERRL_RC_INVALID_DATA;
                break;
        }

        // Generate FFDC error log if required
        if (TRUE == l_ffdc)
        {
            /* @
             * @errortype
             * @moduleid    DATA_GET_RESET_PREP_ERRL
             * @reasoncode  PREP_FOR_RESET
             * @userdata1   reset reason
             * @userdata2   0
             * @userdata4   0
             * @devdesc     Generate error log for ResetPrep command
             */
            l_errlHndl = createErrl(
                DATA_GET_RESET_PREP_ERRL,           //modId
                PREP_FOR_RESET,                     //reasoncode
                OCC_NO_EXTENDED_RC,                 //Extended reason code
                ERRL_SEV_INFORMATIONAL,             //Severity
                NULL,                               //Trace Buf
                CMDH_RESET_PREP_TRACE_SIZE,         //Trace Size
                l_cmd_ptr->reason,                  //userdata1
                0                                   //userdata2
                );

            // commit error log
            if (l_errlHndl != NULL)
            {
                commitErrl(&l_errlHndl);
            }
        }

        if (G_sysConfigData.system_type.kvm && isSafeStateRequested() &&
            (l_cmd_ptr->reason != CMDH_PREP_POWER_OFF))
        {
            // Notify dcom thread to update opal table
            ssx_semaphore_post(&G_dcomThreadWakeupSem);
        }
        if (CURRENT_STATE() != OCC_STATE_STANDBY)
        {
            // Put OCC in stand-by state
            l_errlHndl = SMGR_set_state(OCC_STATE_STANDBY);
        }

        if(l_errlHndl)
        {
            // Commit error log for the failed transition
            commitErrl(&l_errlHndl);
            TRAC_ERR("cmdh_reset_prep: Failed to transition to stand-by state!");
            l_rc = ERRL_RC_INTERNAL_FAIL;
        }
        else
        {
            // Prevent the OCC from going back to the original state it was
            // prior to the reset prep command
            if (G_occ_role == OCC_MASTER)
            {
                G_occ_external_req_state = OCC_STATE_STANDBY;
            }
        }

    } while(0);

    G_rsp_status = l_rc;

    if(ERRL_RC_SUCCESS != l_rc)
    {
        // Build Error Response packet
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, l_rc, &l_errlHndl);
    }
    return l_errlHndl;
}

// Function Specification
//
// Name:  cmdh_clear_elog
//
// Description: Clear elog and free up entry
//
// End Function Specification
errlHndl_t cmdh_clear_elog (const   cmdh_fsp_cmd_t * i_cmd_ptr,
                            cmdh_fsp_rsp_t * o_rsp_ptr)
{
    cmdh_clear_elog_query_t *l_cmd_ptr = (cmdh_clear_elog_query_t *) i_cmd_ptr;
    cmdh_clear_elog_version_t *l_cmd_version_ptr = (cmdh_clear_elog_version_t *) i_cmd_ptr;
    ERRL_RC                  l_rc = ERRL_RC_SUCCESS;
    uint16_t                 l_data_length = CMDH_DATALEN_FIELD_UINT16(l_cmd_ptr);
    uint8_t                  l_elog_id = 0;
    uint8_t                  l_elog_source = ERRL_SOURCE_INVALID;

    int l_SlotNum = ERRL_INVALID_SLOT;
    errlHndl_t l_err = NULL;
    errlHndl_t l_oci_address = INVALID_ERR_HNDL;

    o_rsp_ptr->data_length[0] = 0;
    o_rsp_ptr->data_length[1] = 0;
    do
    {
       // must support old and new version 1 until (H)TMGT removes support for old version
       // old version didn't have a version number, use data length to determine version
       if(l_data_length == CLEAR_ELOG_V0_CMD_LEN)
       {
          l_elog_id = l_cmd_ptr->elog_id;
          // version 0 only supported elogs from the 405
          l_elog_source = ERRL_SOURCE_405;
       }
       else if( (l_data_length == CLEAR_ELOG_V1_CMD_LEN) &&
                (l_cmd_version_ptr->version == 1) )
       {
          l_elog_id = l_cmd_version_ptr->elog_id;
          l_elog_source = l_cmd_version_ptr->elog_source;
       }

       else
       {
          CMDH_TRAC_ERR("cmdh_clear_elog: Invalid version 0x%02X or data length 0x%02X",
                          l_cmd_version_ptr->version, l_data_length);
          l_rc = ERRL_RC_INVALID_DATA;
          break;
       }

       // process the clear command based on elog source
       switch(l_elog_source)
       {
           case ERRL_SOURCE_405:
               // Get Errl Array index
               l_SlotNum = getErrSlotNumByErrId(l_elog_id);

               // Get ERRL address
               l_oci_address = (errlHndl_t)getErrSlotOCIAddr(l_SlotNum);

               if ((l_oci_address != NULL) &&
                   (l_oci_address != INVALID_ERR_HNDL))
               {
                   // clear only one Errl by ID
                   l_err = deleteErrl(&l_oci_address);
               }
               else
               {
                   CMDH_TRAC_ERR("cmdh_clear_elog: 405 error log ID[0x%02X] not found", l_elog_id);
                   l_rc = ERRL_RC_INVALID_DATA;
               }
               break;

           default: // non-405 error log
               {
                   unsigned int index = 0;
                   for (; index < G_hcode_elog_table_slots; ++index)
                   {
                       hcode_elog_entry_t elog_entry;
                       elog_entry.dw0.value = in64(&G_hcode_elog_table[index]);
                       if ((elog_entry.dw0.fields.errlog_id == l_elog_id) && (elog_entry.dw0.fields.errlog_src == l_elog_source))
                       {
                           CMDH_TRAC_INFO("cmdh_clear_elog: Clearing HCODE elog id 0x%02X from source 0x%02X",
                                          l_elog_id, l_elog_source);

                           // Zero out error log entry in list so hcode can reuse
                           out64(&G_hcode_elog_table[index], 0);
                           break;
                       }
                   }
                   if (index == G_hcode_elog_table_slots)
                   {
                       // Did not find matching entry in hcode table for non-405 error
                       CMDH_TRAC_ERR("cmdh_clear_elog: Could not find elog id 0x%02X with source 0x%02X",
                                     l_elog_id, l_elog_source);
                       l_rc = ERRL_RC_INVALID_DATA;
                   }
                   G_htmgt_notified_of_error = false;
               }
               break;
       }
    }while(0);

    if ( (l_err == NULL) && (l_rc == ERRL_RC_SUCCESS) )
    {
        G_rsp_status = ERRL_RC_SUCCESS;
    }
    else
    {
        /// Build Error Response packet
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
    }

    return l_err;
}

// Function Specification
//
// Name:  SMGR_base_setmodestate_cmdh
//
// Description: Process set mode and state command
//
// End Function Specification
errlHndl_t cmdh_tmgt_setmodestate(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                  cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t                      l_errlHndl     = NULL;
    smgr_setmodestate_v0_query_t*   l_cmd_ptr      = (smgr_setmodestate_v0_query_t *)i_cmd_ptr;
    ERRL_RC                         l_rc           = ERRL_RC_INTERNAL_FAIL;
    SsxInterval                     l_timeout      = SSX_SECONDS(15);
    SsxTimebase                     l_start        = ssx_timebase_get();
    OCC_STATE                       l_pre_state    = CURRENT_STATE();
    OCC_MODE                        l_pre_mode     = CURRENT_MODE();

    // OPAL only accepts DPS-FE mode. In case OCC gets other modes, it should accept the request
    // and keep reporting back that it is in that mode.
    if(G_sysConfigData.system_type.kvm)
    {
        l_pre_mode  = G_occ_external_req_mode_kvm;
    }

    do
    {
        // -------------------------------------------------
        // Check Command & Function Inputs
        // -------------------------------------------------
        // Function Inputs Sanity Check
        if( (NULL == i_cmd_ptr) || (NULL == o_rsp_ptr) )
        {
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }

        // Command Version Check
        if(l_cmd_ptr->version != SMGR_SMS_CMD_VERSION)
        {
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }

        // Command Length Check
        if( CMDH_DATALEN_FIELD_UINT16(i_cmd_ptr) !=
                (sizeof(smgr_setmodestate_v0_query_t) - sizeof(cmdh_fsp_cmd_header_t)))
        {
            l_rc = ERRL_RC_INVALID_CMD_LEN;
            break;
        }

        // Can't send this command to a slave
        if( OCC_SLAVE == G_occ_role )
        {
            l_rc = ERRL_RC_INVALID_CMD;
            break;
        }

        // Verify that the state and mode are correct
        if (!OCC_STATE_IS_VALID(l_cmd_ptr->occ_state) || !OCC_MODE_IS_VALID(l_cmd_ptr->occ_mode))
        {
            CMDH_TRAC_ERR("Invalid state and/or mode! State=0x%0X Mode=0x%0X",
                          l_cmd_ptr->occ_state, l_cmd_ptr->occ_mode);
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }

        // Verify not in safe state
        if ((TRUE == isSafeStateRequested()) || (CURRENT_STATE() == OCC_STATE_SAFE))
        {
            CMDH_TRAC_ERR("OCC in safe state, rejecting mode change request");
            l_rc = ERRL_RC_INVALID_STATE;
            break;
        }

        // -------------------------------------------------
        // Act on State & Mode Changes
        // -------------------------------------------------
        CMDH_TRAC_INFO("SMS Mode=%d, State=%d",l_cmd_ptr->occ_mode, l_cmd_ptr->occ_state);

        G_occ_external_req_mode  = l_cmd_ptr->occ_mode;
        G_occ_external_req_state = l_cmd_ptr->occ_state;

        // We need to wait and see if all Slaves correctly make it to state/mode.
        do
        {
            uint8_t l_slv_idx = 0;
            uint8_t l_occ_passed_num = 0;
            uint8_t l_occ_num = G_occ_num_present;
            uint8_t l_occ_bitmap_present = G_sysConfigData.is_occ_present;
            uint8_t l_occ_bitmap_succeeded = 0;

            for(l_slv_idx=0; l_slv_idx < MAX_OCCS; l_slv_idx++)
            {
                // Check if the occ exists
                if( (0x01<<l_slv_idx) & l_occ_bitmap_present )
                {
                    // Check if occ reaches the requested state/mode
                    if( ( (G_dcom_slv_outbox_rx[l_slv_idx].occ_fw_mailbox[0] == G_occ_external_req_state)
                          || (G_occ_external_req_state == OCC_STATE_NOCHANGE)                             ) &&
                        ( (G_dcom_slv_outbox_rx[l_slv_idx].occ_fw_mailbox[1] == G_occ_external_req_mode)
                          || (G_occ_external_req_mode == OCC_MODE_NOCHANGE)                               )     )
                    {
                        l_occ_bitmap_succeeded |= (0x01<<l_slv_idx);
                        l_occ_passed_num++;
                    }
                }
            }

            if(l_occ_num <= l_occ_passed_num)
            {
                // This means that all present OCCs have reached the desired state/mode
                CMDH_TRAC_INFO("cmdh_tmgt_setmodestate: changed state from %d to %d, mode from %d to %d",
                          l_pre_state, G_occ_external_req_state, l_pre_mode, G_occ_external_req_mode);
                l_rc = ERRL_RC_SUCCESS;
                break;
            }
            else
            {
                // check time and break out if we reached limit
                if ( ((ssx_timebase_get() - l_start) > l_timeout))
                {
                    CMDH_TRAC_ERR("cmdh_tmgt_setmodestate: time out waiting for all slave occ (expected:%d, passed:%d)",
                                  l_occ_num, l_occ_passed_num);
                    /* @
                     * @errortype
                     * @moduleid    CMDH_GENERIC_CMD_FAILURE
                     * @reasoncode  INTERNAL_FAILURE
                     * @userdata1   OCC present bitmap
                     * @userdata2   OCC succeeded bitmap
                     * @userdata4   OCC_NO_EXTENDED_RC
                     * @devdesc     Timed out trying to reach requested power mode/state
                     */
                    l_errlHndl = createErrl(
                            CMDH_GENERIC_CMD_FAILURE,           //modId
                            INTERNAL_FAILURE,                   //reasoncode
                            OCC_NO_EXTENDED_RC,                 //Extended reason code
                            ERRL_SEV_UNRECOVERABLE,             //Severity
                            NULL,                               //Trace Buf
                            DEFAULT_TRACE_SIZE,                 //Trace Size
                            l_occ_bitmap_present,               //userdata1
                            l_occ_bitmap_succeeded              //userdata2
                            );
                    addCalloutToErrl(l_errlHndl,
                                     ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                     ERRL_COMPONENT_ID_FIRMWARE,
                                     ERRL_CALLOUT_PRIORITY_HIGH);
                    l_rc = ERRL_RC_INTERNAL_FAIL;
                    break;
                }
                else
                {
                    // Give OCCs a chance to get to active state. This
                    // timeout is arbitrary, but there's no point in making
                    // it too small.
                    ssx_sleep(SSX_MILLISECONDS(100));
                }
            }

        }while( 1 );

    }while(0);

    if(l_rc)
    {
        // Build Error Response packet
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, l_rc, &l_errlHndl);
    }

    return l_errlHndl;
}

// Function Specification
//
// Name:  cmdh_amec_pass_through
//
// Description: Process Amester pass-through command
//
// End Function Specification
errlHndl_t cmdh_amec_pass_through(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                  cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t                      l_errlHndl    = NULL;
    IPMIMsg_t                       l_IPMImsg;
    uint8_t                         l_rc          = 0;
    uint16_t                        l_rsp_data_length = CMDH_FSP_RSP_DATA_SIZE;
    errl_generic_resp_t*            l_err_resp_ptr = (errl_generic_resp_t*)o_rsp_ptr;
    static bool L_traced_reject = FALSE;

    if (G_smf_mode == false)
    {
        do
        {
            // Function Inputs Sanity Check
            if( (NULL == i_cmd_ptr) || (NULL == o_rsp_ptr) )
            {
                l_rc = ERRL_RC_INTERNAL_FAIL;
                break;
            }

            // Byte0 is ipmi command number
            l_IPMImsg.u8Cmd = i_cmd_ptr->data[0];

            //set the ipmi command data size, byte0 and byte1 is ipmi header
            l_IPMImsg.u8CmdDataLen = CONVERT_UINT8_ARRAY_UINT16( i_cmd_ptr->data_length[0],
                                                                 i_cmd_ptr->data_length[1])
                - AMEC_AME_CMD_HEADER_SZ;

            // Set the ipmi command data buffer
            l_IPMImsg.au8CmdData_ptr = (uint8_t *)&i_cmd_ptr->data[AMEC_AME_CMD_HEADER_SZ];

            // Call the amester entry point
            l_rc = amester_entry_point( &l_IPMImsg,
                                        &l_rsp_data_length,
                                        o_rsp_ptr->data);

            if(COMPCODE_NORMAL != l_rc)
            {
                TRAC_ERR("amester_entry_point failured, rc (ipmi completion code) = %d", l_rc);

                // Just put the rc in the return packet and return success
                l_rsp_data_length = 1;
                o_rsp_ptr->data[0] = l_rc;
                l_rc = ERRL_RC_SUCCESS;
            }

            // Protect from overflowing buffer
            if(l_rsp_data_length > G_amester_max_data_length)
            {
                TRAC_ERR("amester_entry_point returned too much data. Got back %d bytes, but we only support sending %d bytes to IPMI",
                         l_rsp_data_length, G_amester_max_data_length);
                /* @
                 * @errortype
                 * @moduleid    AMEC_AMESTER_INTERFACE
                 * @reasoncode  INTERNAL_FAILURE
                 * @userdata1   response data length
                 * @userdata2   max data length
                 * @userdata4   OCC_NO_EXTENDED_RC
                 * @devdesc     amester_entry_point returned too much data.
                 */
                l_errlHndl = createErrl(
                                        AMEC_AMESTER_INTERFACE,             //modId
                                        INTERNAL_FAILURE,                   //reasoncode
                                        OCC_NO_EXTENDED_RC,                 //Extended reason code
                                        ERRL_SEV_INFORMATIONAL,             //Severity
                                        NULL,                               //Trace Buf
                                        DEFAULT_TRACE_SIZE,                 //Trace Size
                                        l_rsp_data_length,                  //userdata1
                                        G_amester_max_data_length           //userdata2
                                       );

                l_rc = ERRL_RC_INTERNAL_FAIL;
                break;
            }
            // Set response rc and length
            G_rsp_status = ERRL_RC_SUCCESS;
            o_rsp_ptr->data_length[0] = ((uint8_t *)&l_rsp_data_length)[0];
            o_rsp_ptr->data_length[1] = ((uint8_t *)&l_rsp_data_length)[1];

        }while(0);

        if(l_rc)
        {
            l_err_resp_ptr->data_length[0] = 0;
            l_err_resp_ptr->data_length[1] = 1;
            G_rsp_status = l_rc;

            if(l_errlHndl)
            {
                l_err_resp_ptr->log_id = l_errlHndl->iv_entryId;
            }
            else
            {
                l_err_resp_ptr->log_id = 0;
            }
        }
    }
    else
    {
        if (!L_traced_reject)
        {
            TRAC_ERR("cmdh_amec_pass_through: Amester not supported in SMF mode");
            L_traced_reject = TRUE;
        }
        // Return error to TMGT w/no log
        G_rsp_status = ERRL_RC_NO_SUPPORT_IN_SMF_MODE;
        o_rsp_ptr->data_length[0] = 0;
        o_rsp_ptr->data_length[1] = 1;
        o_rsp_ptr->data[0] = 0x00; // no error log
    }

    return l_errlHndl;
}


// Function Specification
//
// Name: cmdh_tmgt_get_field_debug_data
//
// Description: Process get field debug data command
//
// End Function Specification
errlHndl_t cmdh_tmgt_get_field_debug_data(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                          cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t                        l_err             = NULL;
    uint16_t                          i                 = 0;
    UINT                              l_rtLen           = 0;
    uint16_t                          l_num_of_sensors  = CMDH_FIELD_MAX_NUM_SENSORS;
    sensorQueryList_t                 l_sensor_list[CMDH_FIELD_MAX_NUM_SENSORS];
    sensor_t                          *l_sensor_ptr     = NULL;
    cmdh_get_field_debug_data_resp_t  *l_resp_ptr       = (cmdh_get_field_debug_data_resp_t*) o_rsp_ptr;
    uint16_t                          l_rsp_data_length = 0;
    ERRL_RC                           l_rc              = ERRL_RC_SUCCESS;

    do
    {
        // Function Inputs Sanity Check
        if( (NULL == i_cmd_ptr) || (NULL == o_rsp_ptr) )
        {
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }

        // Add occ infomation so that we know where the debug data from
        l_resp_ptr->occ_node     = G_pbax_id.node_id;

        l_resp_ptr->occ_id       =  G_pbax_id.chip_id;
        l_resp_ptr->occ_role     = G_occ_role;

        // copy trace data
        l_rtLen = CMDH_FIELD_TRACE_DATA_SIZE;
        TRAC_get_buffer_partial(TRAC_get_td("ERR"), l_resp_ptr->trace_err, &l_rtLen);

        l_rtLen = CMDH_FIELD_TRACE_DATA_SIZE;
        TRAC_get_buffer_partial(TRAC_get_td("INF"), l_resp_ptr->trace_inf, &l_rtLen);

        querySensorListArg_t l_qsl_arg = {
            0,                         // i_startGsid - start with sensor 0x0000
            0,                         // i_present
            (AMEC_SENSOR_TYPE_POWER|   // i_type
             AMEC_SENSOR_TYPE_TEMP),
            AMEC_SENSOR_LOC_ALL,       // i_loc
            &l_num_of_sensors,         // io_numOfSensors
            l_sensor_list,             // o_sensors
            NULL                       // o_sensorInfoPtr
        };

        // Get sensor list
        l_err = querySensorList(&l_qsl_arg);

        if (NULL != l_err)
        {
            // Query failure, this should not happen
            TRAC_ERR("get_field_debug_data: Failed to get sensor list. Error status is: 0x%x",
                     l_err->iv_reasonCode);

            // Commit error log
            commitErrl(&l_err);
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }
        else
        {
            // Populate the response data packet
            l_resp_ptr->num_sensors = l_num_of_sensors;
            for (i=0; i<l_num_of_sensors; i++)
            {
                l_resp_ptr->sensor[i].gsid = l_sensor_list[i].gsid;
                l_resp_ptr->sensor[i].sample = l_sensor_list[i].sample;
                strcpy(l_resp_ptr->sensor[i].name, l_sensor_list[i].name);

                // Capture the min and max value for this sensor
                l_sensor_ptr = getSensorByGsid(l_sensor_list[i].gsid);
                if (l_sensor_ptr == NULL)
                {
                    TRAC_INFO("get_field_debug_data: Unable to find sensor with gsid[0x%.4X]. Values won't be accurate.",
                              l_sensor_list[i].gsid);

                    // Didn't find this sensor, just continue
                    continue;
                }
                l_resp_ptr->sensor[i].sample_min = l_sensor_ptr->sample_min;
                l_resp_ptr->sensor[i].sample_max = l_sensor_ptr->sample_max;
            }
        }
        // -------------------------------------------------
        // Build Response Packet
        // -------------------------------------------------
        // Populate the response data header
        l_rsp_data_length = (sizeof(cmdh_get_field_debug_data_resp_t) - CMDH_DBUG_FSP_RESP_LEN);
        l_resp_ptr->data_length[0] = ((uint8_t *)&l_rsp_data_length)[0];
        l_resp_ptr->data_length[1] = ((uint8_t *)&l_rsp_data_length)[1];
        G_rsp_status = l_rc;

    } while(0);

    if (l_rc)
    {
        // Build Error Response packet
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, l_rc, &l_err);
    }
    return l_err;
}

// Function Specification
//
// Name:  cmdh_set_user_pcap_common
//
// Description: Implements the common part of Set Use Power Cap cmd from inband or out of band
//
// End Function Specification
uint8_t cmdh_set_user_pcap_common(uint16_t i_pcap,
                                  uint8_t  i_source)
{
    uint8_t  l_rc = ERRL_RC_SUCCESS;

    do
    {
        // Can't send this command to a slave
        if (OCC_SLAVE == G_occ_role)
        {
            TRAC_ERR("From source %d User PCAP %d must be sent to master OCC",
                      i_source, i_pcap);
            l_rc = ERRL_RC_INVALID_CMD;
            break;
        }

        //A value of 0 means this pcap has been deactivated, otherwise
        //make sure it's within the min & max.
        if ((i_pcap != 0) && (i_pcap < G_master_pcap_data.soft_min_pcap))
        {
            TRAC_ERR("From source %d User PCAP %d is below the minimum allowed (%d)",
                      i_source, i_pcap, G_master_pcap_data.soft_min_pcap);

            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }
        else if ((i_pcap > G_master_pcap_data.system_pcap) &&
                 (G_master_pcap_data.system_pcap != 0))
        {
            TRAC_ERR("From source %d User PCAP %d is above the maximum allowed (%d)",
                      i_source, i_pcap, G_master_pcap_data.system_pcap);

            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }
        else
        {
            G_master_pcap_data.current_pcap = i_pcap;

            //Indicate there is new PCAP data available
            G_master_pcap_data.pcap_data_count++;
            // if user pcap was just disabled set source to 0 (no user pcap)
            if(i_pcap == 0)
            {
               G_master_pcap_data.source = 0;
            }
            else
            {
               G_master_pcap_data.source = i_source;
            }
        }

        TRAC_INFO("User selected power limit = %d set from source %d",
                  G_master_pcap_data.current_pcap, i_source);

    } while (0);

    return l_rc;
}

// Function Specification
//
// Name:  cmdh_set_user_pcap
//
// Description: Implements the Set Use Power Cap command from out of band interface
//
// End Function Specification
errlHndl_t cmdh_set_user_pcap(const cmdh_fsp_cmd_t * i_cmd_ptr,
                              cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t l_err = NULL;
    ERRL_RC  l_rc = ERRL_RC_SUCCESS;


    G_rsp_status = ERRL_RC_SUCCESS;
    o_rsp_ptr->data_length[0] = 0;
    o_rsp_ptr->data_length[1] = 0;

    if (CMDH_DATALEN_FIELD_UINT16(i_cmd_ptr) != CMDH_SET_USER_PCAP_DATALEN)
    {
        TRAC_ERR("cmdh_set_user_pcap: Invalid command length %u, expected %u ",
                  CMDH_DATALEN_FIELD_UINT16(i_cmd_ptr), CMDH_SET_USER_PCAP_DATALEN);
        l_rc = ERRL_RC_INVALID_CMD_LEN;
    }
    else
    {
        uint16_t l_pcap = CONVERT_UINT8_ARRAY_UINT16(i_cmd_ptr->data[0],
                                                     i_cmd_ptr->data[1]);
        l_rc = cmdh_set_user_pcap_common(l_pcap, OUT_OF_BAND);
    }

    if (ERRL_RC_SUCCESS != l_rc)
    {
        // Build Error Response packet
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, l_rc, &l_err);
    }

    return l_err;
}

// Function Specification
//
// Name:  cmdh_clear_sensor_data
//
// Description: Implements the Clear sensor data command
//
// End Function Specification
uint8_t cmdh_clear_sensor_data(const uint16_t  i_cmd_data_length,
                               const uint8_t*  i_cmd_data_ptr,
                               const uint16_t  i_max_rsp_data_length,
                                     uint16_t* o_rsp_data_length,
                                     uint8_t*  o_rsp_data_ptr)
{
    uint8_t  l_rc = ERRL_RC_SUCCESS;
    cmdh_clear_sensor_cmd_data_t *l_cmd_ptr = (cmdh_clear_sensor_cmd_data_t *) i_cmd_data_ptr;
    cmdh_clear_sensor_rsp_data_t *l_rsp_ptr = (cmdh_clear_sensor_rsp_data_t*) o_rsp_data_ptr;
    *o_rsp_data_length = 0;

    do
    {
        // Command Length Check
        if( i_cmd_data_length != sizeof(cmdh_clear_sensor_cmd_data_t) )
        {
            TRAC_ERR("cmdh_clear_sensor_data: Invalid command length %u, expected %u ",
                     i_cmd_data_length, sizeof(cmdh_clear_sensor_cmd_data_t));
            l_rc = ERRL_RC_INVALID_CMD_LEN;
            break;
        }
        // Make sure there is enough room in response buffer
        if( sizeof(cmdh_clear_sensor_rsp_data_t) > i_max_rsp_data_length )
        {
            TRAC_ERR("cmdh_clear_sensor_data: Response size %u is larger than buffer size %u ",
                     sizeof(cmdh_clear_sensor_rsp_data_t), i_max_rsp_data_length);
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }
        // Check that the owner(s) to clear sensors for are valid
        if( (l_cmd_ptr->sensor_owner_id & ~(VALID_CLEAR_SENSOR_OWNER_MASK) ) ||
            (l_cmd_ptr->sensor_owner_id == 0) )
        {
            TRAC_ERR("cmdh_clear_sensor_data: Invalid sensor owners = 0x%02X",
                     l_cmd_ptr->sensor_owner_id);
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }

        // clear min/max fields of all sensors for the given owner(s)
        sensor_t *l_sensor_ptr = NULL;
        uint16_t i = 0;
        for( i = 0;i < G_amec_sensor_count; i++)
        {
            l_sensor_ptr = getSensorByGsid(i);
            sensor_clear_minmax(l_sensor_ptr, l_cmd_ptr->sensor_owner_id);
        }

        // copy the owner_id to the response buffer and set the rsp length
        l_rsp_ptr->sensor_owner_id = l_cmd_ptr->sensor_owner_id;
        *o_rsp_data_length = (uint16_t) sizeof(cmdh_clear_sensor_rsp_data_t);
        TRAC_INFO("cmdh_clear_sensor_data: Sensor min/max cleared for owners = 0x%02X",
                   l_rsp_ptr->sensor_owner_id);
    } while (0);

    return l_rc;
}

// Function Specification
//
// Name:  cmdh_set_pcap_inband
//
// Description: Implements setting a power cap from the inband interface
//
// End Function Specification
uint8_t cmdh_set_pcap_inband(const uint16_t  i_cmd_data_length,
                             const uint8_t*  i_cmd_data_ptr,
                             const uint16_t  i_max_rsp_data_length,
                                   uint16_t* o_rsp_data_length,
                                   uint8_t*  o_rsp_data_ptr)
{
    uint8_t  l_rc = ERRL_RC_SUCCESS;
    cmdh_set_inband_pcap_cmd_data_t *l_cmd_ptr = (cmdh_set_inband_pcap_cmd_data_t *) i_cmd_data_ptr;
    cmdh_set_inband_pcap_rsp_data_t *l_rsp_ptr = (cmdh_set_inband_pcap_rsp_data_t*) o_rsp_data_ptr;
    *o_rsp_data_length = 0;

    do
    {
        // Command Length Check
        if( i_cmd_data_length != sizeof(cmdh_set_inband_pcap_cmd_data_t) )
        {
            TRAC_ERR("cmdh_set_pcap_inband: Invalid command length %u, expected %u ",
                     i_cmd_data_length, sizeof(cmdh_set_inband_pcap_cmd_data_t));
            l_rc = ERRL_RC_INVALID_CMD_LEN;
            break;
        }

        // Make sure there is enough room in response buffer
        if( sizeof(cmdh_set_inband_pcap_rsp_data_t) > i_max_rsp_data_length )
        {
            TRAC_ERR("cmdh_set_pcap_inband: Response size %u is larger than buffer size %u ",
                     sizeof(cmdh_set_inband_pcap_rsp_data_t), i_max_rsp_data_length);
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }

        uint16_t l_pcap = CONVERT_UINT8_ARRAY_UINT16(l_cmd_ptr->power_cap[0],
                                                     l_cmd_ptr->power_cap[1]);
        l_rc = cmdh_set_user_pcap_common(l_pcap, IN_BAND);

        // if successful copy the power cap to the response buffer and set the rsp length
        if(l_rc == ERRL_RC_SUCCESS)
        {
           l_rsp_ptr->power_cap[0] = l_cmd_ptr->power_cap[0];
           l_rsp_ptr->power_cap[1] = l_cmd_ptr->power_cap[1];
           *o_rsp_data_length = (uint16_t) sizeof(cmdh_set_inband_pcap_rsp_data_t);
        }
    } while (0);

    return l_rc;
}

// Function Specification
//
// Name:  cmdh_write_psr
//
// Description: Implements the Write Power Shifting Ratio command
//
// End Function Specification
uint8_t cmdh_write_psr(const uint16_t  i_cmd_data_length,
                       const uint8_t*  i_cmd_data_ptr,
                       const uint16_t  i_max_rsp_data_length,
                             uint16_t* o_rsp_data_length,
                             uint8_t*  o_rsp_data_ptr)
{
    uint8_t  l_rc = ERRL_RC_SUCCESS;
    cmdh_write_psr_cmd_data_t *l_cmd_ptr = (cmdh_write_psr_cmd_data_t *) i_cmd_data_ptr;
    cmdh_write_psr_rsp_data_t *l_rsp_ptr = (cmdh_write_psr_rsp_data_t*) o_rsp_data_ptr;
    *o_rsp_data_length = 0;

    do
    {
        // Command Length Check
        if( i_cmd_data_length != sizeof(cmdh_write_psr_cmd_data_t) )
        {
            TRAC_ERR("cmdh_write_psr: Invalid command length %u, expected %u ",
                     i_cmd_data_length, sizeof(cmdh_write_psr_cmd_data_t));
            l_rc = ERRL_RC_INVALID_CMD_LEN;
            break;
        }
        // Make sure there is enough room in response buffer
        if( sizeof(cmdh_write_psr_rsp_data_t) > i_max_rsp_data_length )
        {
            TRAC_ERR("cmdh_write_psr: Response size %u is larger than buffer size %u ",
                     sizeof(cmdh_write_psr_rsp_data_t), i_max_rsp_data_length);
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }
        // Verify PSR is within range 0-100%
        if(l_cmd_ptr->psr > 100)
        {
            TRAC_ERR("cmdh_write_psr: Invalid PSR %u",
                     l_cmd_ptr->psr);
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }

        // PSR is valid
        TRAC_INFO("cmdh_write_psr: PSR changed from %u to %u", G_sysConfigData.psr, l_cmd_ptr->psr);
        G_sysConfigData.psr = l_cmd_ptr->psr;

        // copy the PSR to the response buffer and set the rsp length
        l_rsp_ptr->psr = l_cmd_ptr->psr;
        *o_rsp_data_length = (uint16_t) sizeof(cmdh_write_psr_rsp_data_t);
    } while (0);

    return l_rc;
}

// Function Specification
//
// Name:  cmdh_select_sensor_groups
//
// Description: Implements the Select sensor Groups command to select
//               sensor types that will be copied to main memory
//
// End Function Specification
uint8_t cmdh_select_sensor_groups(const uint16_t  i_cmd_data_length,
                                  const uint8_t*  i_cmd_data_ptr,
                                  const uint16_t  i_max_rsp_data_length,
                                        uint16_t* o_rsp_data_length,
                                        uint8_t*  o_rsp_data_ptr)
{
    uint8_t  l_rc = ERRL_RC_SUCCESS;
    uint16_t l_sensor_groups = 0;
    cmdh_select_sensor_groups_cmd_data_t *l_cmd_ptr = (cmdh_select_sensor_groups_cmd_data_t *) i_cmd_data_ptr;
    cmdh_select_sensor_groups_rsp_data_t *l_rsp_ptr = (cmdh_select_sensor_groups_rsp_data_t*) o_rsp_data_ptr;
    *o_rsp_data_length = 0;
    do
    {
        // Command Length Check
        if( i_cmd_data_length != sizeof(cmdh_select_sensor_groups_cmd_data_t) )
        {
            TRAC_ERR("cmdh_select_sensor_groups: Invalid command length %u, expected %u ",
                     i_cmd_data_length, sizeof(cmdh_select_sensor_groups_cmd_data_t));
            l_rc = ERRL_RC_INVALID_CMD_LEN;
            break;
        }
        // Make sure there is enough room in response buffer
        if( sizeof(cmdh_select_sensor_groups_rsp_data_t) > i_max_rsp_data_length )
        {
            TRAC_ERR("cmdh_select_sensor_groups: Response size %u is larger than buffer size %u ",
                     sizeof(cmdh_select_sensor_groups_rsp_data_t), i_max_rsp_data_length);
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }
        // Check that the sensor group(s) to select are valid
        // 0 is valid and means not to copy any sensors to main memory
        l_sensor_groups = CONVERT_UINT8_ARRAY_UINT16(l_cmd_ptr->sensor_groups[0],
                                                     l_cmd_ptr->sensor_groups[1]);
        if(l_sensor_groups & ~(VALID_SET_SENSOR_GROUPS_MASK))
        {
            TRAC_ERR("cmdh_select_sensor_groups: Invalid sensor groups = 0x%04X",
                     l_sensor_groups);
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }

        // Loop thru 16 bits to check all possible sensor types
        uint8_t l_bit = 0;
        uint16_t l_sensor_type = 0;
        bool l_enabled = false;
        for(l_bit = 0; l_bit < 16; l_bit++)
        {
           l_sensor_type = 0x0001 << l_bit;
           // only set eanbled for sensor types that are valid
           if( l_sensor_type & (VALID_SET_SENSOR_GROUPS_MASK) )
           {
              // type is valid now set enabled based on sensor groups selected
              l_enabled = (l_sensor_groups & l_sensor_type) ? true : false;
              main_mem_sensors_set_enabled(l_sensor_type, l_enabled);
           }
        }

        // copy the sensor groups to the response buffer and set the rsp length
        l_rsp_ptr->sensor_groups[0] = l_cmd_ptr->sensor_groups[0];
        l_rsp_ptr->sensor_groups[1] = l_cmd_ptr->sensor_groups[1];
        *o_rsp_data_length = (uint16_t) sizeof(cmdh_select_sensor_groups_rsp_data_t);
        TRAC_INFO("cmdh_select_sensor_groups: Sensor groups 0x%04X selected",
                   l_sensor_groups);
    } while (0);

    return l_rc;
}
