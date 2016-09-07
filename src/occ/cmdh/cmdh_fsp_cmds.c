/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/cmdh/cmdh_fsp_cmds.c $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
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
#include "cmdhDbugCmd.h"
#include "appletManager.h"
#include "gpsm.h"
#include "pstates.h"
#include "proc_pstate.h"
#include "gpe_data.h"
#include "centaur_data.h"
#include <amec_data.h>
#include "amec_amester.h"
#include "amec_service_codes.h"
#include "amec_freq.h"
#include "amec_sys.h"
#include "sensor.h"
#include "sensorQueryList.h"
#include "chom.h"
#include "amec_master_smh.h"
#include "thrm_thread.h"
#include <proc_data.h>
#include "amec_wof.h"
#include <centaur_data.h>

extern cent_sensor_flags_t G_dimm_temp_expired_bitmap;

// We need to have a small structure in non-Applet space to keep track
// of success & failures when running test applets.  This needs to be
// kept as small as possible.
typedef struct
{
  uint16_t    successful; // Number of Sucessful Test Applets
  uint16_t    failed;     // Number of Failed Test Applets
  uint16_t    total;      // Total Number of Test Applets
  SsxTimebase duration;   // Total Duration of Test Applets
  errlHndl_t  errlHndl;   // Holds the first failure's errlHndl
} testAppletStats_t;

testAppletStats_t G_testAppletStats = {0};

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
};

// Flag to indicate that new tunable parameter values need to be written
// (=0: no new values available; =1: new values need to be written; =2: restore defaults)
uint8_t G_mst_tunable_parameter_overwrite = 0;

//Reverse association of channel to function.
uint8_t G_apss_ch_to_function[MAX_APSS_ADC_CHANNELS] = {0};

extern thrm_fru_data_t      G_thrm_fru_data[DATA_FRU_MAX];

// Function Specification
//
// Name:  cmdh_tmgt_poll
//
// Description: TODO -- Add description
//
// End Function Specification
errlHndl_t cmdh_tmgt_poll (const cmdh_fsp_cmd_t * i_cmd_ptr,
                     cmdh_fsp_rsp_t * o_rsp_ptr)
{
    cmdh_poll_query_t *         l_poll_cmd  = (cmdh_poll_query_t *) i_cmd_ptr;
    errlHndl_t                  l_errlHndl  = NULL;
    ERRL_RC                     l_rc        = ERRL_RC_INTERNAL_FAIL;
    uint8_t                     k           = 0;

    do
    {
        if(l_poll_cmd->version == CMDH_POLL_BASE_VERSION)
        {
            cmdh_poll_resp_v0_t * l_poll_rsp = (cmdh_poll_resp_v0_t *) o_rsp_ptr;

            memset(l_poll_rsp,0,(size_t)sizeof(cmdh_poll_resp_v0_t));

            l_poll_rsp->status.word     = SMGR_validate_get_valid_states();
            l_poll_rsp->ext_status.word = 0;
            l_poll_rsp->ext_status.cooling_request = THRM_thread_get_cooling_request();

            l_poll_rsp->occ_pres_mask   = G_sysConfigData.is_occ_present;
            l_poll_rsp->config_data     = DATA_request_cnfgdata();
            l_poll_rsp->state           = CURRENT_STATE();
            l_poll_rsp->ips_status.word = 0;

            if( G_sysConfigData.system_type.kvm )
            {
                l_poll_rsp->mode = G_occ_external_req_mode_kvm;
            }
            else
            {
                l_poll_rsp->mode            = CURRENT_MODE();
            }

            l_poll_rsp->ext_status.dvfs_due_to_ot = 0;
            l_poll_rsp->ext_status.dvfs_due_to_pwr = 0;

            for ( k = 0; k < MAX_NUM_CORES; k++ )
            {
                uint32_t l_freq_reason = g_amec->proc[0].core[k].f_reason;
                if ( l_freq_reason & (AMEC_VOTING_REASON_PROC_THRM | AMEC_VOTING_REASON_VRHOT_THRM) )
                {
                    l_poll_rsp->ext_status.dvfs_due_to_ot = 1;
                }

                if ( l_freq_reason & (AMEC_VOTING_REASON_PPB | AMEC_VOTING_REASON_PMAX | AMEC_VOTING_REASON_PWR) )
                {
                    l_poll_rsp->ext_status.dvfs_due_to_pwr = 1;
                }
            }

            l_poll_rsp->ips_status.ips_enabled = G_ips_config_data.iv_ipsEnabled;
            l_poll_rsp->ips_status.ips_active = AMEC_mst_get_ips_active_status();


            l_poll_rsp->errl_id        = getOldestErrlID();
            l_poll_rsp->errl_address   = getErrlOCIAddrByID(l_poll_rsp->errl_id);
            l_poll_rsp->errl_length    = getErrlLengthByID(l_poll_rsp->errl_id);

            //If errl_id is not 0, then neither address or length should be zero.
            //This should not happen, but if it does tmgt will create an error log that
            //includes the data at the errl slot address given.
            //NOTE: One cause for a false errlog id is corruption of data in one errl slot
            //      due to writing data greater than the size of the previous slot.  For
            //      example writing the CallHome errorlog (3kb) into a regular sized (2kb) slot.
            if ( (l_poll_rsp->errl_id != 0) &&
                 ((l_poll_rsp->errl_address == 0) || (l_poll_rsp->errl_length == 0)))
            {
                TRAC_ERR("An error ID has been sent via poll but the address or size is 0. "
                         "ErrlId:0x%X, sz:0x%X, address:0x%X.",
                         l_poll_rsp->errl_id, l_poll_rsp->errl_length, l_poll_rsp->errl_address);
            }
            l_poll_rsp->data_length[0] = CONVERT_UINT16_UINT8_HIGH(CMDH_POLL_RESP_LEN_V0);
            l_poll_rsp->data_length[1] = CONVERT_UINT16_UINT8_LOW(CMDH_POLL_RESP_LEN_V0);
            l_rc                       = ERRL_RC_SUCCESS;
            l_poll_rsp->rc             = ERRL_RC_SUCCESS;

            // TODO: Clear flag indicating we sent a 'poll request' to TMGT
        }
        else if (l_poll_cmd->version == CMDH_POLL_VERSION10)
        {
            l_rc = cmdh_poll_v10(o_rsp_ptr);
        }
        else
        {
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
// Name:  cmdh_poll_v10
//
// Description: Used for version 0x10 poll calls from BMC/HTMGT.
//
// End Function Specification

ERRL_RC cmdh_poll_v10(cmdh_fsp_rsp_t * o_rsp_ptr)
{
    ERRL_RC                     l_rc  = ERRL_RC_INTERNAL_FAIL;
    uint8_t                     k = 0;
    cmdh_poll_sensor_db_t       l_sensorHeader;

    // Clear response buffer
    memset(o_rsp_ptr, 0, (size_t)sizeof(cmdh_fsp_rsp_t));

    // Set pointer to start of o_rsp_ptr
    cmdh_poll_resp_v10_fixed_t * l_poll_rsp = (cmdh_poll_resp_v10_fixed_t *) o_rsp_ptr;

    // Byte 1
    l_poll_rsp->status.word = SMGR_validate_get_valid_states();

    // Byte 2
    l_poll_rsp->ext_status.word = 0;

    //SET DVFS bits.
    for ( k = 0; k < MAX_NUM_CORES; k++ )
    {
        uint32_t l_freq_reason = g_amec->proc[0].core[k].f_reason;
        uint16_t l_freq_request = g_amec->proc[0].core[k].f_request;
        if ( l_freq_reason & (AMEC_VOTING_REASON_PROC_THRM | AMEC_VOTING_REASON_VRHOT_THRM) )
        {
            //If WOF is disabled   OR
            //If WOF is enabled AND freq is less than Turbo frequency,
            //set dvfs_due_to_ot bit.
            //NOTE: STURBO holds the turbo freq if wof is enabled.
            if ((g_amec->wof.enable_parm == 0) ||
                ( (g_amec->wof.enable_parm) &&
                  (l_freq_request < G_sysConfigData.sys_mode_freq.table[OCC_MODE_STURBO])))
            {
                l_poll_rsp->ext_status.dvfs_due_to_ot = 1;
            }
        }

        if ( l_freq_reason & (AMEC_VOTING_REASON_PPB | AMEC_VOTING_REASON_PMAX | AMEC_VOTING_REASON_PWR) )
        {
            //If WOF is disabled   OR
            //If WOF is enabled AND freq is less than Turbo frequency,
            //set dvfs_due_to_pwr bit.
            //NOTE: STURBO holds the turbo freq if wof is enabled.
            if ((g_amec->wof.enable_parm == 0) ||
                ( (g_amec->wof.enable_parm) &&
                  (l_freq_request < G_sysConfigData.sys_mode_freq.table[OCC_MODE_STURBO])))
            {
                l_poll_rsp->ext_status.dvfs_due_to_pwr = 1;
            }
        }
    }

    //If memory is being throttled due to OverTemp or due to Failure to read sensors set mthrot_due_to_ot bit.
    if (((g_amec->mem_throttle_reason == AMEC_MEM_VOTING_REASON_DIMM) ||
         (g_amec->mem_throttle_reason == AMEC_MEM_VOTING_REASON_CENT)))
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

    // Byte 6 System Mode and Byte 7 IPS Status
    if( G_sysConfigData.system_type.kvm )  // OPAL No mode or IPS support
    {
        l_poll_rsp->mode = 0;
        l_poll_rsp->ips_status.word = 0;
    }
    else  // PowerVM on BMC supports mode and IPS
    {
        l_poll_rsp->mode         = CURRENT_MODE();
        l_poll_rsp->ips_status.word = 0;
        l_poll_rsp->ips_status.ips_enabled = G_ips_config_data.iv_ipsEnabled;
        l_poll_rsp->ips_status.ips_active = AMEC_mst_get_ips_active_status();
    }

    // Byte 8:
    l_poll_rsp->errl_id         = getOldestErrlID();
    // Byte 9 - 12:
    l_poll_rsp->errl_address    = getErrlOCIAddrByID(l_poll_rsp->errl_id);
    // Byte 13 - 14:
    l_poll_rsp->errl_length     = getErrlLengthByID(l_poll_rsp->errl_id);

            //If errl_id is not 0, then neither address or length should be zero.
            //This should not happen, but if it does tmgt will create an error log that
            //includes the data at the errl slot address given that can be used for debug.
            //NOTE: One cause for a false errlog id is corruption of data in one errl slot
            //      due to writing data greater than the size of the previous slot.  For
            //      example writing the CallHome errorlog (3kb) into a regular sized (2kb) slot.
            //      Make sure to verify the order of the memory allocation for the errl slots.
    if ( (l_poll_rsp->errl_id != 0) &&
         ((l_poll_rsp->errl_address == 0) || (l_poll_rsp->errl_length == 0)))
    {
        TRAC_ERR("An error ID has been sent via poll but the address or size is 0. "
                 "ErrlId:0x%X, sz:0x%X, address:0x%X.",
                 l_poll_rsp->errl_id, l_poll_rsp->errl_length, l_poll_rsp->errl_address);
    }

    // Byte 15 - 16: reserved.
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
    uint16_t l_rsp_index = CMDH_POLL_RESP_LEN_V10;

    ////////////////////
    // TEMP sensors:
    // Generate datablock header for temp sensors and sensor data.
    memset((void*) &l_sensorHeader, 0, (size_t)sizeof(cmdh_poll_sensor_db_t));
    memcpy ((void *) &(l_sensorHeader.eyecatcher[0]), SENSOR_TEMP, 4);
    l_sensorHeader.format = 0x01;
    l_sensorHeader.length = sizeof(cmdh_poll_temp_sensor_t);
    l_sensorHeader.count  = 0;

    //Initialize to max number of possible temperature sensors.
    cmdh_poll_temp_sensor_t l_tempSensorList[MAX_NUM_CORES + MAX_NUM_MEM_CONTROLLERS + (MAX_NUM_MEM_CONTROLLERS * NUM_DIMMS_PER_CENTAUR)];
    memset(l_tempSensorList, 0x00, sizeof(l_tempSensorList));

    for (k=0; k<MAX_NUM_CORES; k++)
    {
        if(CORE_PRESENT(k))
        {
            l_tempSensorList[l_sensorHeader.count].id = G_amec_sensor_list[TEMP2MSP0C0 + k]->ipmi_sid;
            l_tempSensorList[l_sensorHeader.count].value = G_amec_sensor_list[TEMP2MSP0C0 + k]->sample;
            l_sensorHeader.count++;
        }
    }

    uint8_t l_cent, l_dimm = 0;
    for (l_cent=0; l_cent < MAX_NUM_MEM_CONTROLLERS; l_cent++)
    {
        if (CENTAUR_PRESENT(l_cent))
        {
            //Add entry for centaurs.
            l_tempSensorList[l_sensorHeader.count].id = g_amec->proc[0].memctl[l_cent].centaur.temp_sid;
            if (G_cent_timeout_logged_bitmap & (CENTAUR0_PRESENT_MASK >> l_cent))
            {
                l_tempSensorList[l_sensorHeader.count].value = 0xFFFF;
            }
            else
            {
                l_tempSensorList[l_sensorHeader.count].value = g_amec->proc[0].memctl[l_cent].centaur.centaur_hottest.cur_temp;
            }
            l_sensorHeader.count++;

            //Add entries for present dimms associated with current centaur l_cent.
            for(l_dimm=0; l_dimm < NUM_DIMMS_PER_CENTAUR; l_dimm++)
            {
                if (g_amec->proc[0].memctl[l_cent].centaur.dimm_temps[l_dimm].temp_sid != 0)
                {
                    l_tempSensorList[l_sensorHeader.count].id = g_amec->proc[0].memctl[l_cent].centaur.dimm_temps[l_dimm].temp_sid;
                    //If a dimm timed out long enough, we should return 0xFFFF for that sensor.
                    if (G_dimm_temp_expired_bitmap.bytes[l_cent] & (DIMM_SENSOR0 >> l_dimm))
                    {
                        l_tempSensorList[l_sensorHeader.count].value = 0xFFFF;
                    }
                    else
                    {
                        l_tempSensorList[l_sensorHeader.count].value = g_amec->proc[0].memctl[l_cent].centaur.dimm_temps[l_dimm].cur_temp;
                    }

                    l_sensorHeader.count++;

                }

            }
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
        uint8_t l_sensordataSz = l_sensorHeader.count * l_sensorHeader.length;
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
    l_sensorHeader.format = 0x01;
    l_sensorHeader.length = sizeof(cmdh_poll_freq_sensor_t);
    l_sensorHeader.count  = 0;

    cmdh_poll_freq_sensor_t l_freqSensorList[MAX_NUM_CORES];
    for (k=0; k<MAX_NUM_CORES; k++)
    {
        if(CORE_PRESENT(k))
        {
            l_freqSensorList[l_sensorHeader.count].id = G_amec_sensor_list[FREQA2MSP0C0 + k]->ipmi_sid;
            l_freqSensorList[l_sensorHeader.count].value = G_amec_sensor_list[FREQA2MSP0C0 + k]->sample;
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
        uint8_t l_sensordataSz = l_sensorHeader.count * l_sensorHeader.length;
        // Copy sensor data into response buffer.
        memcpy ((void *) &(o_rsp_ptr->data[l_rsp_index]), (void *)l_freqSensorList, l_sensordataSz);
        // Increment index into response buffer.
        l_rsp_index += l_sensordataSz;
    }

    /////////////////////
    // POWR Sensors:
    // Generate datablock header for power sensors and sensor data.  RETURNED by MASTER ONLY.
    if (G_occ_role == OCC_MASTER)
    {
        memset((void*) &l_sensorHeader, 0, (size_t)sizeof(cmdh_poll_sensor_db_t));
        memcpy ((void *) &(l_sensorHeader.eyecatcher[0]), SENSOR_POWR, 4);
        l_sensorHeader.format = 0x01;
        l_sensorHeader.length = sizeof(cmdh_poll_power_sensor_t);
        l_sensorHeader.count  = 0;

        // Generate sensor list.
        cmdh_poll_power_sensor_t l_pwrSensorList[MAX_APSS_ADC_CHANNELS];
        for (k = 0; k < MAX_APSS_ADC_CHANNELS; k++)
        {
            if (G_amec_sensor_list[PWRAPSSCH0 + k]->ipmi_sid != 0)
            {
                l_pwrSensorList[l_sensorHeader.count].id = G_amec_sensor_list[PWRAPSSCH0 + k]->ipmi_sid;
                l_pwrSensorList[l_sensorHeader.count].current = G_amec_sensor_list[PWRAPSSCH0 + k]->sample;
                l_pwrSensorList[l_sensorHeader.count].accumul =
                    (uint32_t)G_amec_sensor_list[PWRAPSSCH0 + k]->accumulator;
                l_pwrSensorList[l_sensorHeader.count].update_tag  = G_amec_sensor_list[PWRAPSSCH0 + k]->update_tag;
                l_sensorHeader.count++;
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
            uint8_t l_sensordataSz = l_sensorHeader.count * l_sensorHeader.length;
            // Copy sensor data into response buffer.
            memcpy ((void *) &(o_rsp_ptr->data[l_rsp_index]), (void *)l_pwrSensorList, l_sensordataSz);
            // Increment index into response buffer.
            l_rsp_index += l_sensordataSz;
        }
    }

    ////////////////////////
    // POWER CAPS:
    // Generate datablock header for power caps.  RETURNED by MASTER ONLY.
    if (G_occ_role == OCC_MASTER)
    {
        memset((void*) &l_sensorHeader, 0, (size_t)sizeof(cmdh_poll_sensor_db_t));
        memcpy ((void *) &(l_sensorHeader.eyecatcher[0]), SENSOR_CAPS, 4);
        l_sensorHeader.format = 0x01;
        l_sensorHeader.length = sizeof(cmdh_poll_pcaps_sensor_t);


        cmdh_poll_pcaps_sensor_t l_pcapData;
        l_pcapData.current = g_amec->pcap.active_node_pcap;
        l_pcapData.system = G_amec_sensor_list[PWR250US]->sample;
        l_pcapData.n = G_sysConfigData.pcap.oversub_pcap;
        l_pcapData.max = G_sysConfigData.pcap.max_pcap;
        l_pcapData.min = G_sysConfigData.pcap.hard_min_pcap;
        l_pcapData.user = G_sysConfigData.pcap.current_pcap;
        l_sensorHeader.count  = 1;

        // Copy header to response buffer.
        memcpy ((void *) &(o_rsp_ptr->data[l_rsp_index]), (void *)&l_sensorHeader, sizeof(l_sensorHeader));
        // Increment index into response buffer.
        l_rsp_index += sizeof(l_sensorHeader);

        uint8_t l_sensordataSz = l_sensorHeader.count * l_sensorHeader.length;
        // Copy sensor data into response buffer.
        memcpy ((void *) &(o_rsp_ptr->data[l_rsp_index]), (void *)&(l_pcapData), l_sensordataSz);
        // Increment index into response buffer.
        l_rsp_index += l_sensordataSz;

        l_poll_rsp->sensor_dblock_count +=1;

    }

    l_poll_rsp->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_rsp_index);
    l_poll_rsp->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_rsp_index);
    l_rc                       = ERRL_RC_SUCCESS;
    l_poll_rsp->rc             = ERRL_RC_SUCCESS;

    return l_rc;
}




// Function Specification
//
// Name:  cmdh_tmgt_query_fw
//
// Description: TODO -- Add description
//
// End Function Specification
void cmdh_tmgt_query_fw (const cmdh_fsp_cmd_t * i_cmd_ptr,
                               cmdh_fsp_rsp_t * o_rsp_ptr)
{
    cmdh_fw_resp_t *            l_fw_data = (cmdh_fw_resp_t *) o_rsp_ptr;

    l_fw_data->rc = ERRL_RC_SUCCESS;
    l_fw_data->data_length[0] = 0;
    l_fw_data->data_length[1] = CMDH_FW_QUERY_RESP_LEN;

    // Copy the buildname into the response
    memcpy( (void *) l_fw_data->fw_level,
            (void *) &G_occ_buildname[0],
            (size_t) CMDH_FW_QUERY_RESP_LEN);

    return;
}


// Function Specification
//
// Name:  cmdh_reset_prep_t
//
// Description: TODO -- Add description
//
// End Function Specification
errlHndl_t cmdh_reset_prep (const cmdh_fsp_cmd_t * i_cmd_ptr,
                            cmdh_fsp_rsp_t * o_rsp_ptr)
{
    cmdh_reset_prep_t *         l_cmd_ptr = (cmdh_reset_prep_t *) i_cmd_ptr;
    errlHndl_t                  l_errlHndl = NULL;
    ERRL_RC                     l_rc = ERRL_RC_SUCCESS;
    bool                        l_ffdc = FALSE;

    o_rsp_ptr->rc = ERRL_RC_SUCCESS;
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

        if (G_sysConfigData.system_type.kvm && isSafeStateRequested())
        {
            // Notify dcom thread to update sapphire table
            ssx_semaphore_post(&G_dcomThreadWakeupSem);
        }
        if (CURRENT_STATE() != OCC_STATE_STANDBY)
        {
            // Clear the inhibit bits set by the WOF function
            reset_wof_clear_inhibit();

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

    o_rsp_ptr->rc = l_rc;

    if(ERRL_RC_SUCCESS != l_rc)
    {
        // Build Error Response packet
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, l_rc, &l_errlHndl);
    }

    return l_errlHndl;
}


// Function Specification
//
// Name:  cmdh_tmgt_query_fw
//
// Description: TODO Add description
//
// End Function Specification
errlHndl_t cmdh_clear_elog (const   cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr)
{
    cmdh_clear_elog_query_t *l_cmd_ptr = (cmdh_clear_elog_query_t *) i_cmd_ptr;
    uint8_t l_SlotNum = ERRL_INVALID_SLOT;
    errlHndl_t l_err = INVALID_ERR_HNDL;
    errlHndl_t l_oci_address = INVALID_ERR_HNDL;

    o_rsp_ptr->data_length[0] = 0;
    o_rsp_ptr->data_length[1] = 0;

    // Get Errl Array index
    l_SlotNum = getErrSlotNumByErrId(l_cmd_ptr->elog_id);

    // Get ERRL address
    l_oci_address = (errlHndl_t)getErrSlotOCIAddr(l_SlotNum);

    if ((l_oci_address != NULL) &&
        (l_oci_address != INVALID_ERR_HNDL))
    {
        // clear only one Errl by ID
        l_err = deleteErrl(&l_oci_address);
    }

    if (l_err == NULL)
    {
        o_rsp_ptr->rc = ERRL_RC_SUCCESS;
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
// Name:  cmdh_dbug_get_trace
//
// Description: TODO Add description
//
// End Function Specification
void cmdh_dbug_get_trace (const cmdh_fsp_cmd_t * i_cmd_ptr,
                               cmdh_fsp_rsp_t * o_rsp_ptr)
{
    UINT l_rc = 0;
    UINT l_trace_buffer_size = CMDH_FSP_RSP_SIZE-CMDH_DBUG_FSP_RESP_LEN-8;  // tmgt reserved 8 bytes
    UINT16 l_trace_size = 0;
    cmdh_dbug_get_trace_query_t *l_get_trace_query_ptr = (cmdh_dbug_get_trace_query_t*) i_cmd_ptr;
    cmdh_dbug_get_trace_resp_t *l_get_trace_resp_ptr = (cmdh_dbug_get_trace_resp_t*) o_rsp_ptr;

    tracDesc_t l_trace_ptr = TRAC_get_td((char *)l_get_trace_query_ptr->comp);
    l_rc = TRAC_get_buffer_partial(l_trace_ptr, l_get_trace_resp_ptr->data,&l_trace_buffer_size);
    l_trace_size = l_trace_buffer_size;
    if(l_rc==0)
    {
        o_rsp_ptr->rc = ERRL_RC_SUCCESS;
        o_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_trace_size);
        o_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_trace_size);
    }
    else
    {
        o_rsp_ptr->rc = ERRL_RC_INTERNAL_FAIL;
        o_rsp_ptr->data_length[0] = 0;
        o_rsp_ptr->data_length[1] = 0;
    }
}

// Function Specification
//
// Name:  cmdh_dbug_get_ame_sensor
//
// Description: TODO Add description
//
// End Function Specification
void cmdh_dbug_get_ame_sensor (const cmdh_fsp_cmd_t * i_cmd_ptr,
                               cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                      l_rc = ERRL_RC_SUCCESS;
    uint16_t                     l_type = 0;
    uint16_t                     l_location = 0;
    uint16_t                     i = 0;
    uint16_t                     l_resp_data_length = 0;
    uint16_t                     l_num_of_sensors = CMDH_DBUG_MAX_NUM_SENSORS;
    cmdh_dbug_get_sensor_query_t *l_cmd_ptr = (cmdh_dbug_get_sensor_query_t*) i_cmd_ptr;
    cmdh_dbug_get_sensor_resp_t  *l_resp_ptr = (cmdh_dbug_get_sensor_resp_t*) o_rsp_ptr;
    sensorQueryList_t            l_sensor_list[CMDH_DBUG_MAX_NUM_SENSORS];
    sensor_t                     *l_sensor_ptr = NULL;
    errlHndl_t                   l_err = NULL;
    OCC_APLT_STATUS_CODES        l_status = 0;

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
        TRAC_INFO("dbug_get_ame_sensor: Type[0x%04x] Location[0x%04x]",
                  l_type,
                  l_location);

        // Initialize the Applet arguments
        querySensorListAppletArg_t l_applet_arg = {
            0,                      // i_startGsid - start with sensor 0x0000
            0,                      // i_present
            l_type,                 // i_type - passed by the caller
            l_location,             // i_loc - passed by the caller
            &l_num_of_sensors,      // io_numOfSensors
            l_sensor_list,          // o_sensors
            NULL                    // o_sensorInfoPtr
        };

        // Call the sensor query list applet
        runApplet(OCC_APLT_SNSR_QUERY,
                  &l_applet_arg,
                  TRUE,
                  NULL,
                  &l_err,
                  &l_status);

        if (NULL != l_err)
        {
            // Query failure, this should not happen
            TRAC_ERR("dbug_get_ame_sensor: Failed to run OCC_APLT_SNSR_QUERY applet. Error status is: 0x%x",
                     l_status);

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
                l_resp_ptr->sensor[i].gsid = l_sensor_list[i].gsid;
                l_resp_ptr->sensor[i].sample = l_sensor_list[i].sample;
                strcpy(l_resp_ptr->sensor[i].name, l_sensor_list[i].name);

                // Capture the min and max value for this sensor
                l_sensor_ptr = getSensorByGsid(l_sensor_list[i].gsid);
                if (l_sensor_ptr == NULL)
                {
                    TRAC_INFO("dbug_get_ame_sensor: Didn't find sensor with gsid[0x%.4X]. Min/Max values won't be accurate.",
                              l_sensor_list[i].gsid);

                    // Didn't find this sensor, just continue
                    continue;
                }
                l_resp_ptr->sensor[i].sample_min = l_sensor_ptr->sample_min;
                l_resp_ptr->sensor[i].sample_max = l_sensor_ptr->sample_max;
                l_resp_ptr->sensor[i].ipmi_sid = l_sensor_ptr->ipmi_sid;
            }
        }

    }while(0);

    // Populate the response data header
    l_resp_data_length = sizeof(cmdh_dbug_get_sensor_resp_t) -
        CMDH_DBUG_FSP_RESP_LEN;
    o_rsp_ptr->rc = l_rc;
    o_rsp_ptr->data_length[0] = ((uint8_t *)&l_resp_data_length)[0];
    o_rsp_ptr->data_length[1] = ((uint8_t *)&l_resp_data_length)[1];
}

// Function Specification
//
// Name:  cmdh_dbug_peek
//
// Description: TODO Add description
//
// End Function Specification
void cmdh_dbug_peek (const cmdh_fsp_cmd_t * i_cmd_ptr,
                           cmdh_fsp_rsp_t * o_rsp_ptr)
{
    cmdh_dbug_peek_t * l_cmd_ptr = (cmdh_dbug_peek_t*) i_cmd_ptr;
    uint32_t           l_len     = l_cmd_ptr->size;
    uint8_t            l_type    = l_cmd_ptr->type;
    uint32_t           l_addr    = l_cmd_ptr->oci_address;

    static Ppc405MmuMap       L_mmuMapHomer;
    static Ppc405MmuMap       L_mmuMapCommon;


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
            // Make sure we don't overflow our response buffer
            l_len = (l_len > CMDH_FSP_RSP_DATA_SIZE ) ? CMDH_FSP_RSP_DATA_SIZE : l_len;

            // TODO:  didn't do anything, respond with zero bytes
            l_len = 0;
            break;

        case 0x03:   // Invalidate Cache
            //dcache_invalidate( (void *) l_addr, l_len );
            l_len = 0;
            break;

        case 0x04:   // Flush Cache
            dcache_flush( (void *) l_addr, l_len );
            l_len = 0;
            break;

        case 0x05:   // MMU Map Mainstore
            // Map mainstore to oci space so that we can peek at it

            // HOMER Image
            ppc405_mmu_map(0x00000000, // Mainstore address (BAR0, offset 0)
                           0x00000000, // OCI address 0x0 (BAR0)
                           0x400000,   // Size = 4 MB
                           0,          // TLB hi flags
                           0,          // TLB lo flags
                           &L_mmuMapHomer); // map pointer

            // COMMON Image = Communal OCC Memory Map On Node
            ppc405_mmu_map(0x30000000, // Mainstore address (BAR3, offset 0)
                           0x30000000, // OCI address 0x30000000
                           0x00800000, // Size = 8 MB
                           0,          // TLB hi flags
                           0,          // TLB lo flags
                           &L_mmuMapCommon); // map pointer
            l_len = 0;
            break;
        case 0x06:   // MMU UnMap Mainstore
            // HOMER Image
            ppc405_mmu_unmap(&L_mmuMapHomer);

            // COMMON Image = Communal OCC Memory Map On Node
            ppc405_mmu_unmap(&L_mmuMapCommon);

            l_len = 0;
            break;

        default:
            // Didn't do anything, respond with zero bytes
            l_len = 0;
            break;
    }

    o_rsp_ptr->rc = ERRL_RC_SUCCESS;
    o_rsp_ptr->data_length[0] = CONVERT_UINT16_UINT8_HIGH(l_len);
    o_rsp_ptr->data_length[1] = CONVERT_UINT16_UINT8_LOW(l_len);
}


// Function Specification
//
// Name:  cmdh_get_elog
//
// Description: TODO Add description
//
// End Function Specification
errlHndl_t cmdh_get_elog (const cmdh_fsp_cmd_t * i_cmd_ptr,
                                cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t l_lastID = getOldestErrlID();
    uint8_t l_SlotNum = 0;
    errlHndl_t l_errlHndl = INVALID_ERR_HNDL;

    cmdh_get_elog_resp_t    *l_rsp_ptr = (cmdh_get_elog_resp_t *) o_rsp_ptr;

    l_rsp_ptr->data_length[0] = 0;
    l_rsp_ptr->data_length[1] = CMDH_GET_ELOG_RESP_LEN;

    // Get slot num of last-Get-Err-log, return ERRL_INVALID_SLOT if not found
    l_SlotNum = getErrSlotNumByErrId(l_lastID);

    if (l_SlotNum != ERRL_INVALID_SLOT)
    {
        // clear error handle
        l_errlHndl = NULL;

        l_rsp_ptr->rc = ERRL_RC_SUCCESS;

        // Get and return OCI address of last-Get-Err-log, return NULL if not found
        l_rsp_ptr->oci_address = getErrSlotOCIAddr(l_SlotNum);
        l_rsp_ptr->elog_id = l_lastID;
    }
    else
    {
        l_errlHndl = NULL;

        l_rsp_ptr->rc = ERRL_RC_CONDITIONAL_SUCCESS;

        // no data should be returned
        l_rsp_ptr->oci_address = 0;
        l_rsp_ptr->elog_id = 0;
    }

    return l_errlHndl;
}

// Function Specification
//
// Name:  cmdh_dbug_get_apss_data
//
// Description: TODO Add description
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
        memset(o_rsp_ptr, 0, sizeof(cmdh_dbug_apss_data_resp_t));
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
            l_resp_ptr->ApssCh[i].ipmi_sid = AMECSENSOR_PTR(PWRAPSSCH0 + i)->ipmi_sid;

            TRAC_IMP("DBG__APSS Ch[%02d]:  Raw[0x%04x], Offset[0x%08x], Gain[0x%08x],",
                     i, l_resp_ptr->ApssCh[i].raw, l_resp_ptr->ApssCh[i].offset, l_resp_ptr->ApssCh[i].gain);
            TRAC_IMP("                     Pwr[0x%04x], FuncID[0x%02x], IPMI_sensorID[0x%X]",
                     l_resp_ptr->ApssCh[i].calculated, l_resp_ptr->ApssCh[i].func, l_resp_ptr->ApssCh[i].ipmi_sid);
        }

    }while(0);

    // Populate the response data header
    l_resp_data_length = sizeof(cmdh_dbug_apss_data_resp_t) - CMDH_DBUG_FSP_RESP_LEN;
    o_rsp_ptr->rc = l_rc;
    o_rsp_ptr->data_length[0] = ((uint8_t *)&l_resp_data_length)[0];
    o_rsp_ptr->data_length[1] = ((uint8_t *)&l_resp_data_length)[1];
}

// Function Specification
//
// Name:  cmdh_dbug_dump_wof_data
//
// Description: Command called via OCC debug to return the wof data
//
// End Function Specification
void cmdh_dbug_dump_wof_data (const cmdh_fsp_cmd_t * i_cmd_ptr,
                              cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                      l_rc = ERRL_RC_SUCCESS;
    uint16_t                     i = 0;
    uint16_t                     l_resp_data_length = 0;
    cmdh_dbug_wof_data_resp_t    *l_resp_ptr = (cmdh_dbug_wof_data_resp_t*) o_rsp_ptr;

    do
    {
        memset(o_rsp_ptr, 0, sizeof(cmdh_dbug_wof_data_resp_t));
        // Do sanity check on the function inputs
        if ((NULL == i_cmd_ptr) || (NULL == o_rsp_ptr))
        {
            TRAC_ERR("cmdh_dbug_dump_wof_data failed; Null in or out ptr.");
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }

        //Populate response struct
        l_resp_ptr->wof_enable = g_amec->wof.enable_parm;
        l_resp_ptr->wof_vdd_eff = g_amec->wof.vdd_eff;
        l_resp_ptr->wof_cur_out = g_amec->wof.cur_out;
        l_resp_ptr->wof_loadline = g_amec->wof.loadline;
        l_resp_ptr->wof_v_chip = g_amec->wof.v_chip;
        l_resp_ptr->wof_iddq_i = g_amec->wof.iddq_i;
        l_resp_ptr->wof_iddq85c = g_amec->wof.iddq85c;
        l_resp_ptr->wof_iddq = g_amec->wof.iddq;
        l_resp_ptr->wof_ac = g_amec->wof.ac;
        l_resp_ptr->wof_ceff_tdp = g_amec->wof.ceff_tdp[G_wof_max_cores_per_chip];
        l_resp_ptr->wof_ceff = g_amec->wof.ceff;
        l_resp_ptr->wof_ceff_old = g_amec->wof.ceff_old;
        l_resp_ptr->wof_ceff_ratio = g_amec->wof.ceff_ratio;
        l_resp_ptr->wof_f_uplift = g_amec->wof.f_uplift;
        l_resp_ptr->wof_f_vote = g_amec->wof.f_vote;
        l_resp_ptr->wof_vote_vreg = g_amec->wof.vote_vreg;
        l_resp_ptr->wof_vote_vchip = g_amec->wof.vote_vchip;
        l_resp_ptr->wof_error = g_amec->wof.error;
        l_resp_ptr->wof_state = g_amec->wof.state;
        l_resp_ptr->wof_cores_on = g_amec->wof.cores_on;
        l_resp_ptr->tdp_rdp_factor = G_sysConfigData.wof_parms.tdp_rdp_factor;
        l_resp_ptr->opTurbo_vdd_100uv = G_sysConfigData.wof_parms.operating_points[TURBO].vdd_5mv * 5 * 10;
        l_resp_ptr->opTurbo_idd_500ma = G_sysConfigData.wof_parms.operating_points[TURBO].idd_500ma;
        l_resp_ptr->opTurbo_freq_mhz = G_sysConfigData.wof_parms.operating_points[TURBO].frequency_mhz;
        for (i=0; i < MAX_NUM_CORES; i++)
        {
            if (g_amec->proc[0].core_max_freq == g_amec->proc[0].core[i].f_request)
            {
                l_resp_ptr->proc_max_freq = g_amec->proc[0].core_max_freq;
                l_resp_ptr->proc_freq_reason = g_amec->proc[0].core[i].f_reason;
                l_resp_ptr->proc_reason_core = i;
                break;
            }
        }

        l_resp_ptr->sensor_temp2msp0    = AMECSENSOR_PTR(TEMP2MSP0)->sample;
        l_resp_ptr->sensor_pwr250usvdd0 = AMECSENSOR_PTR(PWR250USVDD0)->sample;
        l_resp_ptr->sensor_cur250usvdd0 = AMECSENSOR_PTR(CUR250USVDD0)->sample;
        l_resp_ptr->sensor_wof250usvdds = AMECSENSOR_PTR(WOF250USVDDS)->sample;

        for (i=0; i< CORE_IDDQ_MEASUREMENTS; i++)
        {
            memcpy(&(l_resp_ptr->iddq_vdd[i]), &(G_sysConfigData.iddq_table.iddq_vdd[i]), sizeof(IddqReading));
        }
    }while(0);

    // Populate the response data header
    l_resp_data_length = sizeof(cmdh_dbug_wof_data_resp_t) - CMDH_DBUG_FSP_RESP_LEN;
    o_rsp_ptr->rc = l_rc;
    o_rsp_ptr->data_length[0] = ((uint8_t *)&l_resp_data_length)[0];
    o_rsp_ptr->data_length[1] = ((uint8_t *)&l_resp_data_length)[1];
}

// Function Specification
//
// Name:  amec_dbug_dump_wof_uplift
//
// Description: Command called via OCC debug to return the uplift table used by WOF
//
// End Function Specification
void cmdh_dbug_dump_wof_uplift (const cmdh_fsp_cmd_t * i_cmd_ptr,
                              cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                     l_rc = ERRL_RC_SUCCESS;
    uint16_t                    l_resp_data_length = 0;
    cmdh_dbug_wof_uplift_resp_t *l_resp_ptr = (cmdh_dbug_wof_uplift_resp_t*) o_rsp_ptr;

    do
    {
        memset(o_rsp_ptr, 0, sizeof(cmdh_dbug_wof_uplift_resp_t));

        // Do sanity check on the function inputs
        if ((NULL == i_cmd_ptr) || (NULL == o_rsp_ptr))
        {
            TRAC_ERR("amec_dbug_dump_wof_uplift failed; Null in or out ptr.");
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }

        //Populate response struct
        l_resp_ptr->rowCnt = AMEC_WOF_UPLIFT_TBL_ROWS;
        l_resp_ptr->colCnt = AMEC_WOF_UPLIFT_TBL_CLMS;
        memcpy(&(l_resp_ptr->upliftTable[0]),
               &(G_amec_wof_uplift_table[0]),
               sizeof(G_amec_wof_uplift_table));
    }while(0);

    // Populate the response data header
    l_resp_data_length = sizeof(cmdh_dbug_wof_uplift_resp_t) - CMDH_DBUG_FSP_RESP_LEN;
    o_rsp_ptr->rc = l_rc;
    o_rsp_ptr->data_length[0] = ((uint8_t *)&l_resp_data_length)[0];
    o_rsp_ptr->data_length[1] = ((uint8_t *)&l_resp_data_length)[1];
}

// Function Specification
//
// Name:  amec_dbug_dump_wof_vrm_eff
//
// Description: Command called via OCC debug to return the vrm Eff table used by WOF
//
// End Function Specification
void cmdh_dbug_dump_wof_vrm_eff (const cmdh_fsp_cmd_t * i_cmd_ptr,
                              cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                     l_rc = ERRL_RC_SUCCESS;
    uint16_t                    l_resp_data_length = 0;
    cmdh_dbug_wof_vrm_eff_resp_t *l_resp_ptr = (cmdh_dbug_wof_vrm_eff_resp_t*) o_rsp_ptr;

    do
    {
        memset(o_rsp_ptr, 0, sizeof(cmdh_dbug_wof_vrm_eff_resp_t));

        // Do sanity check on the function inputs
        if ((NULL == i_cmd_ptr) || (NULL == o_rsp_ptr))
        {
            TRAC_ERR("amec_dbug_dump_wof_vrm_eff failed; Null in or out ptr.");
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }

        //Populate response struct
        l_resp_ptr->rowCnt = AMEC_WOF_VRM_EFF_TBL_ROWS;
        l_resp_ptr->colCnt = AMEC_WOF_VRM_EFF_TBL_CLMS;
        memcpy(&(l_resp_ptr->vrmEff[0]),
               &(G_amec_wof_vrm_eff_table[0]),
               sizeof(G_amec_wof_vrm_eff_table));
    }while(0);

    // Populate the response data header
    l_resp_data_length = sizeof(cmdh_dbug_wof_vrm_eff_resp_t) - CMDH_DBUG_FSP_RESP_LEN;
    o_rsp_ptr->rc = l_rc;
    o_rsp_ptr->data_length[0] = ((uint8_t *)&l_resp_data_length)[0];
    o_rsp_ptr->data_length[1] = ((uint8_t *)&l_resp_data_length)[1];
}

// Function Specification
//
// Name:  dbug_parse_cmd
//
// Description: TODO Add description
//
// End Function Specification
void cmdh_dbug_cmd (const cmdh_fsp_cmd_t * i_cmd_ptr,
                           cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                     l_rc = 0;
    uint8_t                     l_sub_cmd = 0;
    uint8_t                     l_block_num = 0;
    errl_generic_resp_t *       l_err_rsp_ptr =  (errl_generic_resp_t *) o_rsp_ptr;
    errlHndl_t                  l_errl = NULL;
    OCC_APLT_STATUS_CODES       l_status = OCC_APLT_SUCCESS;
    cmdhDbugCmdAppletArg_t      l_applet_args;

    // Sub Command for debug is always first byte of data
    l_sub_cmd = i_cmd_ptr->data[0];

    /// Tracing based on Debug Sub-Command
    switch (l_sub_cmd)
    {
        // ----------------------------------------------------
        // NOTE:  This for for TRACING only, any actual command
        //        handling goes in the switch statement below.
        // ----------------------------------------------------
        case DBUG_GET_TRACE:
        case DBUG_GET_AME_SENSOR:
            // Don't trace that we got these debug commands, they happen too
            // often, or are not destructive when they do occur.
            break;

        default:
            // Trace the rest of the debug commands.
            TRAC_INFO("Debug Command: Sub:0x%02x\n", l_sub_cmd);
            break;
    }

    // Act on Debug Sub-Command
    switch ( l_sub_cmd )
    {
        // ------------------------------------------------
        // Run debug sub-commands that **cannot** use applets
        // ------------------------------------------------
        case DBUG_GET_AME_SENSOR:
            // This can't use an applet because it needs to run
            // an applet to get the data
            cmdh_dbug_get_ame_sensor(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_FSP_ATTN:
            // This is so small that we don't need to run it from an
            // applet
            break;

        case DBUG_GET_TRACE:
            // Get trace buffer SRAM address
            cmdh_dbug_get_trace(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_CLEAR_TRACE:
            // Call clear trace function
            TRAC_reset_buf();
            o_rsp_ptr->rc = ERRL_RC_SUCCESS;
            break;

        case DBUG_PEEK:
            cmdh_dbug_peek(i_cmd_ptr, o_rsp_ptr);
            break;

        case DBUG_FLUSH_DCACHE:
            // This is so small that we don't need to run it from an
            // applet
            dcache_flush_all();
            break;

        case DBUG_GEN_CHOM_LOG:
            chom_force_gen_log();
            break;

        case 0xE0:  // Run Test Applet
            // TODO: Come in and clean this up later
#define TEST_APPLET_MAX_BLOCK_NUMBER 8
#define TEST_APPLET_START_APPLET     0xFF
#define TEST_APPLET_GET_STATS        0xFE
#define TEST_APPLET_RESET_STATS      0xFD

            // We need to waste the OCC SRAM space to start these applets, but
            // lets try to use up as little code as possible

            // Get the 2kB "Block Number" as passed in as the second data
            // byte of the command.
            l_block_num = i_cmd_ptr->data[1];

            // If the block number that was passed is a valid data block,
            // then copy the passed data into Main Memory
            if(l_block_num < TEST_APPLET_MAX_BLOCK_NUMBER)
            {
                BceRequest pba_copy;
                int l_ssxrc = SSX_OK;

                // Calculate Address in Main Memory where we will put this block
                uint32_t l_addr_in_mem = ( G_ApltAddressTable[OCC_APLT_TEST].iv_aplt_address
                                           + (l_block_num*2048));

                // Can't copy to last 1024 bytes of SRAM
                size_t l_size = (l_block_num == 0x07) ? 1024 : 2048;

                TRAC_INFO("uploading %d bytes of test applet block %d from 0x%08x to 0x%08x",
                          l_size, l_block_num, &i_cmd_ptr->data[124], l_addr_in_mem);

                // Set up copy request
                l_ssxrc = bce_request_create(
                        &pba_copy,                          // block copy object
                        &G_pba_bcue_queue,                  // mainstore to sram copy engine
                        l_addr_in_mem,                      // mainstore address
                        (uint32_t) &i_cmd_ptr->data[124],   // sram starting address
                        (size_t) l_size,                    // size of copy
                        SSX_SECONDS(5),                     // no timeout
                        NULL,                               // call back
                        NULL,                               // call back arguments
                        ASYNC_REQUEST_BLOCKING              // callback mask
                        );

                // Don't log error.  We are in a debug path, not product code.
                if(l_ssxrc != SSX_OK)
                {
                    TRAC_ERR("CMDH Test Applet: PBA request create failure rc=[%08X]",l_ssxrc);
                    l_rc = ERRL_RC_INTERNAL_FAIL;
                    break;
                }

                // Do actual copying
                l_ssxrc = bce_request_schedule(&pba_copy);

                // Don't log error.  We are in a debug path, not product code.
                if(l_ssxrc != SSX_OK)
                {
                    TRAC_ERR("CMDH Test Applet: PBA request schedule failure rc=[%08X]",l_ssxrc);
                    l_rc = ERRL_RC_INTERNAL_FAIL;
                    break;
                }
            }
            else if(l_block_num == TEST_APPLET_START_APPLET)
            {
                TRAC_INFO("Starting test applet");
                OCC_APLT_STATUS_CODES l_status = OCC_APLT_SUCCESS;
                errlHndl_t  l_errl = NULL;

                // ------------------------------------------
                // Start Applet, and return
                //     Applet will auto-checksum itself, and
                //     fail w/ errlHndl if it doesn't match
                // ------------------------------------------
                runApplet(OCC_APLT_TEST,        // Applet enum Name
                          NULL,                 // Applet arguments
                          FALSE,                 // Blocking call?
                          NULL,                 // Applet finished semaphore
                          &l_errl,              // Error log handle
                          &l_status);           // Error status

                if( (NULL != l_errl)  || (l_status !=  OCC_APLT_SUCCESS))
                {
                    TRAC_ERR("Test applet failure. status=%d", l_status);
                    l_rc = ERRL_RC_INTERNAL_FAIL;
                }
                else
                {
                    TRAC_INFO("Test applet started");
                }
            }
            else if(l_block_num == TEST_APPLET_GET_STATS)
            {
                TRAC_INFO("Get applet stats");
            }
            else if(l_block_num == TEST_APPLET_RESET_STATS)
            {
                TRAC_INFO("Resetting applet stats");

                // Reset Applet Test Stats
                G_testAppletStats.total      = 0;
                G_testAppletStats.duration   = 0;
                G_testAppletStats.failed     = 0;
                G_testAppletStats.errlHndl   = NULL;
                G_testAppletStats.successful = 0;
            }
            else
            {
                TRAC_ERR("Unrecognized applet command. l_block_num = %d", l_block_num);
                l_rc = ERRL_RC_INVALID_DATA;
            }
            break;

        // ------------------------------------------------
        // Run debug sub-commands that can use applets
        // ------------------------------------------------
        case DBUG_READ_SCOM:
        case DBUG_PUT_SCOM:
        case DBUG_POKE:
        case DBUG_SET_PEXE_EVENT:
        case DBUG_DUMP_THEMAL:
        case DBUG_DUMP_POWER:
        case DBUG_DUMP_RAW_AD:
        case DBUG_MEM_PWR_CTL:
        case DBUG_PERFCOUNT:
        case DBUG_TEST_INTF:
        case DBUG_SET_BUS_SPEED:
        case DBUG_FAN_CONTROL:
        case DBUG_INJECT_ERRL:
        case DBUG_IIC_READ:
        case DBUG_IIC_WRITE:
        case DBUG_GPIO_READ:
        case DBUG_CALCULATE_MAX_DIFF:
        case DBUG_FORCE_ELOG:
        case DBUG_SWITCH_PHASE:
        case DBUG_INJECT_ERR:
        case DBUG_VERIFY_V_F:
        case DBUG_DUMP_PPM_DATA:
        case DBUG_CENTAUR_SENSOR_CACHE:
        case DBUG_DUMP_PROC_DATA:

            l_applet_args.i_cmd_ptr  = (cmdh_fsp_cmd_t *) i_cmd_ptr;
            l_applet_args.io_rsp_ptr = o_rsp_ptr;

            runApplet(OCC_APLT_CMDH_DBUG,   // Applet enum Name
              &l_applet_args,               // Applet arguments
              TRUE,                 // Blocking call?
              NULL,                 // Applet finished semaphore
              &l_errl,              // Error log handle
              &l_status);           // Error status

              if( (NULL != l_errl)  || (l_status !=  OCC_APLT_SUCCESS))
              {
                 TRAC_ERR("Debug command applet returned error: l_status: 0x%x", l_status);
                 commitErrl( &l_errl );
              }
            break;

        case DBUG_DUMP_APSS_DATA:
            cmdh_dbug_get_apss_data(i_cmd_ptr, o_rsp_ptr);
            break;
        case DBUG_DUMP_WOF_DATA:
            cmdh_dbug_dump_wof_data(i_cmd_ptr, o_rsp_ptr);
            break;
        case DBUG_DUMP_WOF_UPLIFT_TBL:
            cmdh_dbug_dump_wof_uplift(i_cmd_ptr, o_rsp_ptr);
            break;
        case DBUG_DUMP_WOF_VRM_EFF_TBL:
            cmdh_dbug_dump_wof_vrm_eff(i_cmd_ptr, o_rsp_ptr);
            break;
        default:
            l_rc = ERRL_RC_INVALID_DATA; //should NEVER get here...
            break;
    } //end switch

    // We don't do errors in DBUG, as a safety check make sure the response is valid.
    if ( l_rc )
    {
        l_err_rsp_ptr->rc = l_rc;
        l_err_rsp_ptr->data_length[0] = 0;
        l_err_rsp_ptr->data_length[1] = 1;
    }

    return;
}


// Function Specification
//
// Name:  SMGR_base_setmodestate_cmdh
//
// Description: Set Mode and State command handler
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

    // SAPPHIRE only accepts DPS-FE mode. In case OCC gets other modes, it should accept the request
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

        // Can't make FFO mode change if FFO frequency never set
        if((l_cmd_ptr->occ_mode == OCC_MODE_FFO) && (!G_sysConfigData.sys_mode_freq.table[OCC_MODE_FFO]))
        {
            TRAC_ERR("FFO requested without FFO frequency set");
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }


        // -------------------------------------------------
        // Act on State & Mode Changes
        // -------------------------------------------------
        TRAC_INFO("SMS Mode=%d, State=%d\n",l_cmd_ptr->occ_mode, l_cmd_ptr->occ_state);

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
                TRAC_INFO("cmdh_tmgt_setmodestate: changed state from %d to %d, mode from %d to %d",
                          l_pre_state, G_occ_external_req_state, l_pre_mode, G_occ_external_req_mode);
                l_rc = ERRL_RC_SUCCESS;
                break;
            }
            else
            {
                // check time and break out if we reached limit
                if ( ((ssx_timebase_get() - l_start) > l_timeout))
                {
                    TRAC_ERR("cmdh_tmgt_setmodestate: time out waiting for all slave occ (expected:%d, passed:%d)",
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
// Description: TODO Add description
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

        // Protect IPMI from overflowing a buffer
        if(l_rsp_data_length > IPMI_MAX_MSG_SIZE)
        {
            TRAC_ERR("amester_entry_point returned too much data. Got back %d bytes, but we only support sending %d bytes to IPMI",
                     l_rsp_data_length, IPMI_MAX_MSG_SIZE);
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
                ERRL_SEV_INFORMATIONAL,              //Severity
                NULL,    //TODO: create trace       //Trace Buf
                DEFAULT_TRACE_SIZE,                 //Trace Size
                l_rsp_data_length,                  //userdata1
                IPMI_MAX_MSG_SIZE                   //userdata2
            );

            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }
        // Set response rc and length
        o_rsp_ptr->rc = ERRL_RC_SUCCESS;
        o_rsp_ptr->data_length[0] = ((uint8_t *)&l_rsp_data_length)[0];
        o_rsp_ptr->data_length[1] = ((uint8_t *)&l_rsp_data_length)[1];

    }while(0);

    if(l_rc)
    {
        l_err_resp_ptr->data_length[0] = 0;
        l_err_resp_ptr->data_length[1] = 1;
        l_err_resp_ptr->rc             = l_rc;

        if(l_errlHndl)
        {
            l_err_resp_ptr->log_id = l_errlHndl->iv_entryId;
        }
        else
        {
            l_err_resp_ptr->log_id = 0;
        }
    }

    return l_errlHndl;
}

// Function Specification
//
// Name: cmdh_tmgt_get_field_debug_data
//
// Description: TODO Add description
//
// End Function Specification
errlHndl_t cmdh_tmgt_get_field_debug_data(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                                cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint16_t                          i                 = 0;
    UINT                              l_rtLen           = 0;
    uint16_t                          l_num_of_sensors  = CMDH_FIELD_MAX_NUM_SENSORS;
    sensorQueryList_t                 l_sensor_list[CMDH_FIELD_MAX_NUM_SENSORS];
    sensor_t                          *l_sensor_ptr     = NULL;
    errlHndl_t                        l_err             = NULL;
    OCC_APLT_STATUS_CODES             l_status          = 0;
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
        l_resp_ptr->occ_node     = G_pob_id.node_id;
        l_resp_ptr->occ_id       = 0; // TODO: add occ id info
        l_resp_ptr->occ_role     = G_occ_role;

        // copy trace data
        l_rtLen = CMDH_FIELD_TRACE_DATA_SIZE;
        TRAC_get_buffer_partial(TRAC_get_td("ERR"), l_resp_ptr->trace_err, &l_rtLen);

        l_rtLen = CMDH_FIELD_TRACE_DATA_SIZE;
        TRAC_get_buffer_partial(TRAC_get_td("INF"), l_resp_ptr->trace_inf, &l_rtLen);

        // Initialize the Applet arguments
        // TODO: Set "present" to 0 for testing (since no sensor presented now)
        querySensorListAppletArg_t l_applet_arg = {
            0,                         // i_startGsid - start with sensor 0x0000
            0,                         // i_present
            (AMEC_SENSOR_TYPE_POWER|   // i_type
             AMEC_SENSOR_TYPE_TEMP),
            AMEC_SENSOR_LOC_ALL,       // i_loc
            &l_num_of_sensors,         // io_numOfSensors
            l_sensor_list,             // o_sensors
            NULL                       // o_sensorInfoPtr
        };

        // Call the sensor query list applet
        runApplet(OCC_APLT_SNSR_QUERY,
                  &l_applet_arg,
                  TRUE,
                  NULL,
                  &l_err,
                  &l_status);

        if (NULL != l_err)
        {
            // Query failure, this should not happen
            TRAC_ERR("get_field_debug_data: Failed to run OCC_APLT_SNSR_QUERY applet. Error status is: 0x%x",
                     l_status);

            // Commit error log
            commitErrl(&l_err);
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }
        else
        {
            TRAC_INFO("get_field_debug_data: Numbers of sensors found[%u]",
                      l_num_of_sensors);

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
        l_resp_ptr->rc             = l_rc;

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
// Name:  cmdh_get_cooling_request
//
// Description: TODO Add description
//
// End Function Specification
errlHndl_t cmdh_get_cooling_request(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t                  l_errlHndl = NULL;
    cmdh_get_cooling_resp_t     *l_resp_ptr = (cmdh_get_cooling_resp_t*) o_rsp_ptr;
    ERRL_RC                     l_rc = ERRL_RC_SUCCESS;
    uint16_t                    l_rsp_data_length = 0;
    uint16_t                    l_max_increment = 0;
    uint8_t                     l_fru_winner = DATA_FRU_MAX;
    uint8_t                     i = 0;
    uint8_t                     j = 0;

    do
    {
        // Command Length Check - make sure we at least have a version number
        if( CMDH_DATALEN_FIELD_UINT16(i_cmd_ptr) < CMDH_GET_COOLING_MIN_DATALEN)
        {
            l_rc = ERRL_RC_INVALID_CMD_LEN;
            break;
        }

        // Version Number Check
        if(i_cmd_ptr->data[0] != CMDH_GET_COOLING_VERSION)
        {
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }

        // Loop through all Zones
        for (i=0; i<THRM_MAX_NUM_ZONES; i++)
        {
            // For each Zone, determine which FRU is requesting the maximum
            // increment value
            l_max_increment = 0;
            l_fru_winner = DATA_FRU_MAX;
            for (j=0; j<DATA_FRU_MAX; j++)
            {
                if (G_thrm_fru_data[j].FanIncZone[i] > l_max_increment)
                {
                    l_max_increment = G_thrm_fru_data[j].FanIncZone[i];
                    l_fru_winner = j;
                }
            }
            l_resp_ptr->zone_data[i].increment_value = l_max_increment;
            l_resp_ptr->zone_data[i].comp_type = l_fru_winner;
            l_resp_ptr->zone_data[i].reason = 0x00; // Always hardcoded to 0x00
        }

        // Populate the response data header
        l_rsp_data_length = (sizeof(cmdh_get_cooling_resp_t) - CMDH_DBUG_FSP_RESP_LEN);
        l_resp_ptr->data_length[0] = ((uint8_t *)&l_rsp_data_length)[0];
        l_resp_ptr->data_length[1] = ((uint8_t *)&l_rsp_data_length)[1];
        l_resp_ptr->rc = l_rc;

    }while(0);

    if(ERRL_RC_SUCCESS != l_rc)
    {
        // Build Error Response packet
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, l_rc, &l_errlHndl);
    }

    return l_errlHndl;
}


// Function Specification
//
// Name:  cmdh_set_user_pcap
//
// Description: Implements the Set Use Power Cap command.
//
// End Function Specification
errlHndl_t cmdh_set_user_pcap(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t l_err = NULL;
    ERRL_RC  l_rc = ERRL_RC_SUCCESS;


    o_rsp_ptr->rc = ERRL_RC_SUCCESS;
    o_rsp_ptr->data_length[0] = 0;
    o_rsp_ptr->data_length[1] = 0;

    do
    {
        // Can't send this command to a slave
        if (OCC_SLAVE == G_occ_role)
        {
            l_rc = ERRL_RC_INVALID_CMD;
            break;
        }

        if (CMDH_DATALEN_FIELD_UINT16(i_cmd_ptr) != CMDH_SET_USER_PCAP_DATALEN)
        {
            l_rc = ERRL_RC_INVALID_CMD_LEN;
            break;
        }

        uint16_t l_pcap = CONVERT_UINT8_ARRAY_UINT16(i_cmd_ptr->data[0],
                                                     i_cmd_ptr->data[1]);

        //A value of 0 means this pcap has been deactivated, otherwise
        //make sure it's within the min & max.
        if ((l_pcap != 0) && (l_pcap < G_master_pcap_data.hard_min_pcap))
        {
            TRAC_ERR("User PCAP %d is below the minimum allowed (%d)",
                      l_pcap, G_master_pcap_data.hard_min_pcap);

            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }
        else if ((l_pcap > G_master_pcap_data.system_pcap) &&
                 (G_master_pcap_data.system_pcap != 0))
        {
            TRAC_ERR("User PCAP %d is above the maximum allowed (%d)",
                      l_pcap, G_master_pcap_data.system_pcap);

            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }
        else
        {
            G_master_pcap_data.current_pcap = l_pcap;

            //Indicate there is new PCAP data available
            G_master_pcap_data.pcap_data_count++;
        }

        TRAC_INFO("User selected power limit = %d",
                  G_master_pcap_data.current_pcap);

    } while (0);


    if (ERRL_RC_SUCCESS != l_rc)
    {
        // Build Error Response packet
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, l_rc, &l_err);
    }

    return l_err;
}
