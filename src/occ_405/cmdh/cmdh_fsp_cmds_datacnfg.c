/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cmdh/cmdh_fsp_cmds_datacnfg.c $                   */
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
#include "cmdh_fsp_cmds_datacnfg.h"
#include "errl.h"
#include "trac.h"
#include "rtls.h"
#include "dcom.h"
#include "occ_common.h"
#include "state.h"
#include "cmdh_fsp_cmds.h"
#include "cmdh_dbug_cmd.h"
//#include "gpsm.h"
#include "proc_pstate.h"
#include <amec_data.h>
#include "amec_amester.h"
#include "amec_service_codes.h"
#include "amec_sys.h"
#include <centaur_data.h>

#define FREQ_FORMAT_PWR_MODE_NUM   6
#define FREQ_FORMAT_BASE_DATA_SZ   (sizeof(cmdh_store_mode_freqs_t) - sizeof(cmdh_fsp_cmd_header_t))

#define FREQ_FORMAT_20_NUM_FREQS   6

#define DATA_FREQ_VERSION_20       0x20

#define DATA_PCAP_VERSION_20       0x20

#define DATA_SYS_VERSION_20        0x20

#define DATA_APSS_VERSION20        0x20

#define DATA_THRM_THRES_VERSION_20 0x20
#define THRM_THRES_BASE_DATA_SZ_20 5

#define DATA_IPS_VERSION           0

#define DATA_MEM_CFG_VERSION_20    0x20

#define DATA_MEM_THROT_VERSION_20  0x20

#define DATA_VOLT_UPLIFT_VERSION   0

extern uint8_t G_occ_interrupt_type;

extern uint32_t G_proc_fmin;
extern uint32_t G_proc_fmax;
extern uint32_t G_khz_per_pstate;

extern uint8_t G_proc_pmin;
extern uint8_t G_proc_pmax;

typedef struct data_req_table
{
   uint32_t mask;
   uint8_t  format;
} data_req_table_t;

data_cnfg_t G_data_cnfg_static_obj = {0};

data_cnfg_t * G_data_cnfg = &G_data_cnfg_static_obj;

const data_req_table_t G_data_pri_table[] =
{
    {DATA_MASK_SYS_CNFG,              DATA_FORMAT_SYS_CNFG}, //Need this first so we can use correct huid's for callouts
    {DATA_MASK_APSS_CONFIG,           DATA_FORMAT_APSS_CONFIG}, //need apss config data prior to role data
    {DATA_MASK_SET_ROLE,              DATA_FORMAT_SET_ROLE},
    {DATA_MASK_MEM_CFG,               DATA_FORMAT_MEM_CFG},
    {DATA_MASK_THRM_THRESHOLDS,       DATA_FORMAT_THRM_THRESHOLDS},
    {DATA_MASK_FREQ_PRESENT,          DATA_FORMAT_FREQ},
    {DATA_MASK_PCAP_PRESENT,          DATA_FORMAT_POWER_CAP},
    {DATA_MASK_MEM_THROT,             DATA_FORMAT_MEM_THROT},
};

// TODO: Temporarily saving this off here not sure
//        it it belongs somewhere more appropriate
cmdh_ips_config_data_t G_ips_config_data = {0};

bool G_mem_monitoring_allowed = FALSE;

// Global flag that indicates if VRMs are present. In the context of BMC-based
// systems, if OCC doesn't get any VRM thermal control thresholds (data packet
// 0x13), it should not attempt to talk to the VRMs. In the context of
// FSP-based systems, then we assume VRMs are always present.
uint8_t G_vrm_present = 1;

// Function Specification
//
// Name:  DATA_get_present_cnfgdata
//
// Description:  Accessor function for external access
//
// End Function Specification
uint32_t DATA_get_present_cnfgdata(void)
{
    return G_data_cnfg->data_mask;
}

errlHndl_t DATA_get_thrm_thresholds(cmdh_thrm_thresholds_t **o_thrm_thresh)
{
    errlHndl_t                  l_err = NULL;

    if(G_data_cnfg->data_mask & DATA_MASK_THRM_THRESHOLDS)
    {
        *o_thrm_thresh = &(G_data_cnfg->thrm_thresh);
    }
    else
    {
        CMDH_TRAC_ERR("DATA_get_thrm_thresholds: Thermal Threshold data is unavailable! data_mask[0x%X]",
                 G_data_cnfg->data_mask);
        /* @
         * @errortype
         * @moduleid    DATA_GET_THRM_THRESHOLDS
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   data mask showing which data OCC has received
         * @userdata4   ERC_CMDH_THRM_DATA_MISSING
         * @devdesc     Someone is asking for the thermal control threholds
         *              and OCC hasn't received them yet from the FSP!
         */
        l_err = createErrl(DATA_GET_THRM_THRESHOLDS,
                           INTERNAL_FAILURE,
                           ERC_CMDH_THRM_DATA_MISSING,
                           ERRL_SEV_PREDICTIVE,
                           NULL,
                           DEFAULT_TRACE_SIZE,
                           G_data_cnfg->data_mask,
                           0);
    }

    return l_err;
}

errlHndl_t DATA_get_ips_cnfg(cmdh_ips_config_data_t **o_ips_cnfg)
{
    errlHndl_t                  l_err = NULL;

    if(G_data_cnfg->data_mask & DATA_MASK_IPS_CNFG)
    {
        *o_ips_cnfg = &G_ips_config_data;
    }
    else
    {
        CMDH_TRAC_ERR("DATA_get_ips_cnfg: IPS Config data is unavailable! data_mask[0x%X]",
                 G_data_cnfg->data_mask);

        /* @
         * @errortype
         * @moduleid    DATA_GET_IPS_DATA
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   data mask showing which data OCC has received
         * @userdata4   ERC_CMDH_IPS_DATA_MISSING
         * @devdesc     Someone is asking for the Idle Power Save config data
         *              and OCC hasn't received them yet from the FSP!
         */
        l_err = createErrl(DATA_GET_IPS_DATA,
                           INTERNAL_FAILURE,
                           ERC_CMDH_IPS_DATA_MISSING,
                           ERRL_SEV_PREDICTIVE,
                           NULL,
                           DEFAULT_TRACE_SIZE,
                           G_data_cnfg->data_mask,
                           0);
    }

    return l_err;
}

// Function Specification
//
// Name:  DATA_request_cnfgdata
//
// Description: TODO -- Add description
//
// End Function Specification
uint8_t DATA_request_cnfgdata ()
{
    uint8_t                        l_req         = 0x00; // Data to request
    uint16_t                       i             = 0;
    uint16_t                       l_array_size  = 0;

    l_array_size = sizeof(G_data_pri_table) / sizeof(data_req_table_t);

    for(i=0;i<l_array_size;i++)
    {
        // Skip requesting memory throttle values if memory monitoring
        // is not being allowed by TMGT.
        if((G_data_pri_table[i].format == DATA_FORMAT_MEM_THROT) &&
            !G_mem_monitoring_allowed)
        {
            continue;
        }

        // Skip whenever we are trying to request pcap or freq as a slave
        if(((G_data_pri_table[i].format == DATA_FORMAT_POWER_CAP) ||
            (G_data_pri_table[i].format == DATA_FORMAT_FREQ)) &&
            (G_occ_role == OCC_SLAVE))
        {
            continue;
        }

        // Go through priority table and request first data found which has
        // not been provided
        if(!(G_data_cnfg->data_mask & G_data_pri_table[i].mask))
        {
            l_req = G_data_pri_table[i].format;
            break;
        }
    }

    return(l_req);
}

// Function Specification
//
// Name:  data_store_freq_data
//
// Description: Write all of the frequency points from TMGT into G_sysConfigData
//
// End Function Specification
errlHndl_t data_store_freq_data(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                cmdh_fsp_rsp_t       * o_rsp_ptr)
{
    errlHndl_t                      l_err = NULL;
    cmdh_store_mode_freqs_t*        l_cmdp = (cmdh_store_mode_freqs_t*)i_cmd_ptr;
    uint8_t*                        l_buf = ((uint8_t*)(l_cmdp)) + sizeof(cmdh_store_mode_freqs_t);
    uint16_t                        l_data_length;
    uint32_t                        l_mode_data_sz;
    uint16_t                        l_freq = 0;
    uint8_t                         l_count = FREQ_FORMAT_PWR_MODE_NUM;
    uint16_t                        l_table[OCC_MODE_COUNT] = {0};

    do
    {
        l_data_length  = CMDH_DATALEN_FIELD_UINT16(l_cmdp);
        l_mode_data_sz = l_data_length - FREQ_FORMAT_BASE_DATA_SZ;

        // Sanity Checks
        // If the datapacket is bigger than what we can store, OR
        // if the version doesn't equal what we expect, OR
        // if the expected data length does not agree with the actual data length
        if((l_data_length < FREQ_FORMAT_BASE_DATA_SZ) ||
           (l_cmdp->version != DATA_FREQ_VERSION_20) ||
           ((DATA_FREQ_VERSION_20 == l_cmdp->version) && (l_mode_data_sz != (FREQ_FORMAT_20_NUM_FREQS * 2))))
        {
            CMDH_TRAC_ERR("Invalid Frequency Data packet: data_length[%u] version[%u] l_count[%u] l_mode_data_sz[%u]",
                     l_data_length, l_cmdp->version, l_count, l_mode_data_sz);
            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
            break;
        }

        if(OCC_MASTER != G_occ_role)
        {
            // We want to ignore this cnfg data if we are not the master.
            CMDH_TRAC_INFO("Received a Frequncy Data Packet on a Slave OCC: Ignore!");
            break;
        }

        if(DATA_FREQ_VERSION_20 == l_cmdp->version) // Version 0x20
        {
            // 1) Nominal, 2) Turbo, 3) Minimum,
            // 4) Ultra Turbo, 5) Static PS, 6) FFO
            // store under the existing enums.

            // Bytes 3-4 Nominal Frequency Point
            l_freq = (l_buf[0] << 8 | l_buf[1]);
            l_table[OCC_MODE_NOMINAL] = l_freq;
            CMDH_TRAC_INFO("Nominal frequency = %d", l_freq);

            // Bytes 5-6 Turbo Frequency Point
            l_freq = (l_buf[2] << 8 | l_buf[3]);
            l_table[OCC_MODE_TURBO] = l_freq;
            CMDH_TRAC_INFO("Turbo frequency = %d", l_freq);

            // Bytes 7-8 Minimum Frequency Point
            l_freq = (l_buf[4] << 8 | l_buf[5]);
            l_table[OCC_MODE_MIN_FREQUENCY] = l_freq;
            G_proc_fmin = l_freq;
            CMDH_TRAC_INFO("Minimum frequency = %d", l_freq);

            // Bytes 9-10 Ultr Turbo Frequency Point
            l_freq = (l_buf[6] << 8 | l_buf[7]);
            if(l_freq)
            {
                G_proc_fmax = l_freq;
            }
            else // If Ultra Turbo Frequency Point = 0, Fmax = Turbo Frequency
            {
                G_proc_fmax = l_table[OCC_MODE_TURBO];
            }
            l_table[OCC_MODE_UTURBO] = l_freq;
            CMDH_TRAC_INFO("UT frequency = %d", l_freq);

            // Bytes 11-12 Static Power Save Frequency Point
            l_freq = (l_buf[8] << 8 | l_buf[9]);
            l_table[OCC_MODE_PWRSAVE] = l_freq;
            CMDH_TRAC_INFO("Static Power Save frequency = %d", l_freq);

            // Bytes 13-14 FFO Frequency Point
            l_freq = (l_buf[10] << 8 | l_buf[11]);
            if (l_freq != 0)
            {
                // Check and make sure that FFO freq is within valid range
                const uint16_t l_req_freq = l_freq;
                if (l_freq < G_proc_fmin)
                {
                    l_freq = G_proc_fmin;
                }
                else if (l_freq > G_proc_fmax)
                {
                    l_freq = G_proc_fmax;
                }

                // Log an error if we could not honor the requested FFO frequency, but keep going.
                if (l_req_freq != l_freq)
                {
                    TRAC_ERR("FFO Frequency out of range. requested %d, but using %d",
                             l_req_freq,  l_freq);
                    /* @
                     * @errortype
                     * @moduleid    DATA_STORE_FREQ_DATA
                     * @reasoncode  INVALID_INPUT_DATA
                     * @userdata1   requested frequency
                     * @userdata2   frequency used
                     * @userdata4   OCC_NO_EXTENDED_RC
                     * @devdesc     OCC recieved an invalid FFO frequency
                     */
                    l_err = createErrl(DATA_STORE_FREQ_DATA,
                                       INVALID_INPUT_DATA,
                                       OCC_NO_EXTENDED_RC,
                                       ERRL_SEV_INFORMATIONAL,
                                       NULL,
                                       DEFAULT_TRACE_SIZE,
                                       l_req_freq,
                                       l_freq);
                    commitErrl(&l_err);
                }
            }
        }
        l_table[OCC_MODE_FFO] = l_freq;
        CMDH_TRAC_INFO("FFO Frequency = %d", l_freq);

        // Calculate minimum Pstate:
        G_proc_pmin = G_proc_pmax + ((G_proc_fmax - G_proc_fmin)/G_khz_per_pstate);

        // inconsistent Frequency Points?
        if((l_table[OCC_MODE_UTURBO] < l_table[OCC_MODE_TURBO] && l_table[OCC_MODE_UTURBO]) ||
           l_table[OCC_MODE_TURBO]   < l_table[OCC_MODE_NOMINAL] ||
           l_table[OCC_MODE_NOMINAL] < l_table[OCC_MODE_PWRSAVE] ||
           l_table[OCC_MODE_PWRSAVE] < l_table[OCC_MODE_MIN_FREQUENCY])
        {
            CMDH_TRAC_ERR("Inconsistent Frequency points - UT-T=0x%x, NOM-PS=0x%x, MIN=0x%x",
                          (l_table[OCC_MODE_UTURBO] << 16)  + l_table[OCC_MODE_TURBO],
                          (l_table[OCC_MODE_NOMINAL] << 16) + l_table[OCC_MODE_PWRSAVE],
                          l_table[OCC_MODE_MIN_FREQUENCY]);
        }

    }while(0);



    // Change Data Request Mask to indicate we got this data
    if(!l_err && (G_occ_role == OCC_MASTER))
    {
        // Copy all of the frequency updates to the global and notify
        // dcom of the new frequenies.
        memcpy(G_sysConfigData.sys_mode_freq.table, l_table, sizeof(l_table));
        G_sysConfigData.sys_mode_freq.update_count++;
        G_data_cnfg->data_mask |= DATA_MASK_FREQ_PRESENT;
    }

    return l_err;
}

// Function Specification
//
// Name:  apss_store_adc_channel
//
// Description: Matches the functional ID (from MRW) to the APSS ADC channel
//
// End Function Specification
errlHndl_t apss_store_adc_channel(const eApssAdcChannelAssignments i_func_id, const uint8_t i_channel_num )
{
    errlHndl_t l_err = NULL;

    // Check function ID and channel number
    if ( (i_func_id >= NUM_ADC_ASSIGNMENT_TYPES) ||
         (i_channel_num >= MAX_APSS_ADC_CHANNELS) )
    {
        CMDH_TRAC_ERR("apss_store_adc_channel: Invalid function ID or channel number (id:0x%x, channel:%d)", i_func_id, i_channel_num);

        /* @
         * @errortype
         * @moduleid    DATA_STORE_APSS_DATA
         * @reasoncode  INVALID_INPUT_DATA
         * @userdata1   function ID
         * @userdata2   channel number
         * @userdata4   ERC_APSS_ADC_OUT_OF_RANGE_FAILURE
         * @devdesc     Invalid function ID or channel number
         */
        l_err = createErrl(DATA_STORE_APSS_DATA,
                           INVALID_INPUT_DATA,
                           ERC_APSS_ADC_OUT_OF_RANGE_FAILURE,
                           ERRL_SEV_UNRECOVERABLE,
                           NULL,
                           DEFAULT_TRACE_SIZE,
                           (uint32_t)i_func_id,
                           (uint32_t)i_channel_num);

        // Callout firmware
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);
    }
    else
    {
        uint8_t *l_adc_function=NULL;
        switch (i_func_id)
        {
            case ADC_RESERVED:
                // Do nothing
                break;

            case ADC_MEMORY_PROC_0:
            case ADC_MEMORY_PROC_1:
            case ADC_MEMORY_PROC_2:
            case ADC_MEMORY_PROC_3:
                l_adc_function = &G_sysConfigData.apss_adc_map.memory[i_func_id-ADC_MEMORY_PROC_0][0];
                break;

            case ADC_MEMORY_PROC_0_0:
                l_adc_function = &G_sysConfigData.apss_adc_map.memory[0][1];
                break;
            case ADC_MEMORY_PROC_0_1:
                l_adc_function = &G_sysConfigData.apss_adc_map.memory[0][2];
                break;
            case ADC_MEMORY_PROC_0_2:
                l_adc_function = &G_sysConfigData.apss_adc_map.memory[0][3];
                break;
            case ADC_VDD_PROC_0:
            case ADC_VDD_PROC_1:
            case ADC_VDD_PROC_2:
            case ADC_VDD_PROC_3:
                l_adc_function = &G_sysConfigData.apss_adc_map.vdd[i_func_id-ADC_VDD_PROC_0];
                break;

            case ADC_VCS_VIO_VPCIE_PROC_0:
            case ADC_VCS_VIO_VPCIE_PROC_1:
            case ADC_VCS_VIO_VPCIE_PROC_2:
            case ADC_VCS_VIO_VPCIE_PROC_3:
                l_adc_function = &G_sysConfigData.apss_adc_map.vcs_vio_vpcie[i_func_id-ADC_VCS_VIO_VPCIE_PROC_0];
                break;

            case ADC_IO_A:
            case ADC_IO_B:
            case ADC_IO_C:
                l_adc_function = &G_sysConfigData.apss_adc_map.io[i_func_id-ADC_IO_A];
                break;

            case ADC_FANS_A:
            case ADC_FANS_B:
                l_adc_function = &G_sysConfigData.apss_adc_map.fans[i_func_id-ADC_FANS_A];
                break;

            case ADC_STORAGE_A:
            case ADC_STORAGE_B:
                l_adc_function = &G_sysConfigData.apss_adc_map.storage_media[i_func_id-ADC_STORAGE_A];
                break;

            case ADC_12V_SENSE:
                l_adc_function = &G_sysConfigData.apss_adc_map.sense_12v;
                break;

            case ADC_GND_REMOTE_SENSE:
                l_adc_function = &G_sysConfigData.apss_adc_map.remote_gnd;
                break;

            case ADC_TOTAL_SYS_CURRENT:
                l_adc_function = &G_sysConfigData.apss_adc_map.total_current_12v;
                break;

            case ADC_MEM_CACHE:
                l_adc_function = &G_sysConfigData.apss_adc_map.mem_cache;
                break;

            case ADC_GPU_SENSE:
                l_adc_function = &G_sysConfigData.apss_adc_map.gpu;
                break;

            default:
                // It should never happen
                CMDH_TRAC_ERR("apss_store_adc_channel: Invalid function ID: 0x%x", i_func_id);
                break;
        }

        if(NULL != l_adc_function)
        {
            // Check if this function already have ADC channel assigned
            if( SYSCFG_INVALID_ADC_CHAN == *l_adc_function)
            {
                *l_adc_function = i_channel_num;
                G_apss_ch_to_function[i_channel_num] = i_func_id;
                CNFG_DBG("apss_store_adc_channel: func_id[0x%02X] stored as 0x%02X for channel %d",
                         i_func_id, G_apss_ch_to_function[i_channel_num], *l_adc_function);
            }
            else
            {

                CMDH_TRAC_ERR("apss_store_adc_channel: Function ID is duplicated (id:0x%x, channel:%d)", i_func_id, i_channel_num);

                /* @
                 * @errortype
                 * @moduleid    DATA_STORE_APSS_DATA
                 * @reasoncode  INVALID_INPUT_DATA
                 * @userdata1   function ID
                 * @userdata2   channel number
                 * @userdata4   ERC_APSS_ADC_DUPLICATED_FAILURE
                 * @devdesc     Function ID is duplicated
                 */
                l_err = createErrl(DATA_STORE_APSS_DATA,
                                   INVALID_INPUT_DATA,
                                   ERC_APSS_ADC_DUPLICATED_FAILURE,
                                   ERRL_SEV_UNRECOVERABLE,
                                   NULL,
                                   DEFAULT_TRACE_SIZE,
                                   (uint32_t)i_func_id,
                                   (uint32_t)i_channel_num);

                // Callout firmware
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                 ERRL_COMPONENT_ID_FIRMWARE,
                                 ERRL_CALLOUT_PRIORITY_HIGH);
            }
        }
    }

    return l_err;
}

// Function Specification
//
// Name:  apss_store_ipmi_sensor_id
//
// Description: Writes the given ipmi sensor ID provided by tmgt to the
//              associated power sensor.
//
// End Function Specification
void apss_store_ipmi_sensor_id(const uint16_t i_channel, const apss_cfg_adc_v20_t *i_adc)
{
    // Get current processor id.
    uint8_t l_proc  = G_pbax_id.module_id;

    switch (i_adc->assignment)
    {
        case ADC_RESERVED:
            // Do nothing; given channel is not utilized.
            break;
        case ADC_MEMORY_PROC_0:
        case ADC_MEMORY_PROC_1:
        case ADC_MEMORY_PROC_2:
        case ADC_MEMORY_PROC_3:
            if (l_proc == (i_adc->assignment - ADC_MEMORY_PROC_0))
            {
                AMECSENSOR_PTR(PWR250USMEM0)->ipmi_sid = i_adc->ipmisensorId;
            }
            break;
        case ADC_VDD_PROC_0:
        case ADC_VDD_PROC_1:
        case ADC_VDD_PROC_2:
        case ADC_VDD_PROC_3:
            if (l_proc == (i_adc->assignment - ADC_VDD_PROC_0))
            {
                AMECSENSOR_PTR(PWR250USVDD0)->ipmi_sid = i_adc->ipmisensorId;
            }

            break;

        case ADC_VCS_VIO_VPCIE_PROC_0:
        case ADC_VCS_VIO_VPCIE_PROC_1:
        case ADC_VCS_VIO_VPCIE_PROC_2:
        case ADC_VCS_VIO_VPCIE_PROC_3:
            if (l_proc == (i_adc->assignment - ADC_VCS_VIO_VPCIE_PROC_0))
            {
                AMECSENSOR_PTR(PWR250USVCS0)->ipmi_sid = i_adc->ipmisensorId;
            }
            break;

        case ADC_IO_A:
        case ADC_IO_B:
        case ADC_IO_C:
            if (i_adc->ipmisensorId != 0)
            {
                AMECSENSOR_PTR(PWR250USIO)->ipmi_sid = i_adc->ipmisensorId;
            }
            break;

        case ADC_FANS_A:
        case ADC_FANS_B:
            if (i_adc->ipmisensorId != 0)
            {
                AMECSENSOR_PTR(PWR250USFAN)->ipmi_sid = i_adc->ipmisensorId;
            }
            break;

        case ADC_STORAGE_A:
        case ADC_STORAGE_B:
            if (i_adc->ipmisensorId != 0)
            {
                AMECSENSOR_PTR(PWR250USSTORE)->ipmi_sid = i_adc->ipmisensorId;
            }
            break;

        case ADC_12V_SENSE:
            //None
            break;

        case ADC_GND_REMOTE_SENSE:
            //None
            break;

        case ADC_TOTAL_SYS_CURRENT:
            //None
            break;
        case ADC_MEM_CACHE:
            //None
            break;

        case ADC_GPU_SENSE:
            if (i_adc->ipmisensorId != 0)
            {
                AMECSENSOR_PTR(PWR250USGPU)->ipmi_sid = i_adc->ipmisensorId;
            }
            break;

        default:
            break;
    }

    //Write sensor ID to channel sensors.  If the assignment(function id) is 0, that means
    //the channel is not being utilized.
    if ((i_channel < MAX_APSS_ADC_CHANNELS) && (i_adc->assignment != ADC_RESERVED))
    {
        if (i_adc->ipmisensorId == 0)
        {
            CMDH_TRAC_ERR("apss_store_ipmi_sensor_id: Missing Sensor ID for channel %i.",i_channel);
            //We need to generate a generic sensor ID if we want channels with functionIDs but
            //no sensor IDs to be reported in the poll command.
        }

        //Only store sensor ids for power sensors.  12V sensor and gnd remote sensors do not report power used.
        if ((i_adc->assignment != ADC_12V_SENSE) && (i_adc->assignment != ADC_GND_REMOTE_SENSE))
        {
            AMECSENSOR_PTR(PWRAPSSCH0 + i_channel)->ipmi_sid = i_adc->ipmisensorId;
            CNFG_DBG("apss_store_ipmi_sensor_id: SID[0x%08X] stored as 0x%08X for channel %d",
                     i_adc->ipmisensorId, AMECSENSOR_PTR(PWRAPSSCH0 + i_channel)->ipmi_sid, i_channel);
        }
    }
}

// Function Specification
//
// Name:  apss_store_gpio_pin
//
// Description: Matches the functional ID (from MRW) to the APSS GPIO pin
//
// End Function Specification
errlHndl_t apss_store_gpio_pin(const eApssGpioAssignments i_func_id, const uint8_t i_gpio_num )
{
    errlHndl_t l_err = NULL;

    // Check function ID and channel number
    if ( (i_func_id >= NUM_GPIO_ASSIGNMENT_TYPES) ||
         ( i_gpio_num >= (MAX_APSS_GPIO_PORTS*NUM_OF_APSS_PINS_PER_GPIO_PORT) ) )
    {
        CMDH_TRAC_ERR("apss_store_gpio_pin: Invalid function ID or gpio number (id:0x%x, pin:%d)", i_func_id, i_gpio_num);

        /* @
         * @errortype
         * @moduleid    DATA_STORE_APSS_DATA
         * @reasoncode  INVALID_INPUT_DATA
         * @userdata1   function ID
         * @userdata2   gpio number
         * @userdata4   ERC_APSS_GPIO_OUT_OF_RANGE_FAILURE
         * @devdesc     Invalid function ID or gpio number
         */
        l_err = createErrl(DATA_STORE_APSS_DATA,
                           INVALID_INPUT_DATA,
                           ERC_APSS_GPIO_OUT_OF_RANGE_FAILURE,
                           ERRL_SEV_UNRECOVERABLE,
                           NULL,
                           DEFAULT_TRACE_SIZE,
                           (uint32_t)i_func_id,
                           (uint32_t)i_gpio_num);

        // Callout firmware
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);
    }
    else
    {
        uint8_t *l_gpio_function = NULL;
        switch (i_func_id)
        {
            case GPIO_RESERVED:
                // Do nothing
                break;

            case GPIO_FAN_WATCHDOG_ERROR:
                l_gpio_function = &G_sysConfigData.apss_gpio_map.fans_watchdog_error;
                break;

            case GPIO_FAN_FULL_SPEED:
                l_gpio_function = &G_sysConfigData.apss_gpio_map.fans_full_speed;
                break;

            case GPIO_FAN_ERROR:
                l_gpio_function = &G_sysConfigData.apss_gpio_map.fans_error;
                break;

            case GPIO_FAN_RESERVED:
                l_gpio_function = &G_sysConfigData.apss_gpio_map.fans_reserved;
                 break;

            case GPIO_VR_HOT_MEM_PROC_0:
            case GPIO_VR_HOT_MEM_PROC_1:
            case GPIO_VR_HOT_MEM_PROC_2:
            case GPIO_VR_HOT_MEM_PROC_3:
                l_gpio_function = &G_sysConfigData.apss_gpio_map.vr_fan[i_func_id-GPIO_VR_HOT_MEM_PROC_0];
                break;
            case GPIO_CENT_EN_VCACHE0:
            case GPIO_CENT_EN_VCACHE1:
            case GPIO_CENT_EN_VCACHE2:
            case GPIO_CENT_EN_VCACHE3:
                l_gpio_function = &G_sysConfigData.apss_gpio_map.cent_en_vcache[i_func_id-GPIO_CENT_EN_VCACHE0];
                break;

            case CME_THROTTLE_N:
                l_gpio_function = &G_sysConfigData.apss_gpio_map.cme_throttle_n;
                break;

            case GND_OC_N:
                l_gpio_function = &G_sysConfigData.apss_gpio_map.gnd_oc_n;
                break;

            case DOM_A_OC_LATCH:
            case DOM_B_OC_LATCH:
            case DOM_C_OC_LATCH:
            case DOM_D_OC_LATCH:
                l_gpio_function = &G_sysConfigData.apss_gpio_map.dom_oc_latch[i_func_id-DOM_A_OC_LATCH];
                break;
            default:
                // It should never happen
                CMDH_TRAC_ERR("apss_store_gpio_pin: Invalid function ID: 0x%x", i_func_id);
                break;
        }

        if(NULL != l_gpio_function)
        {
            // Check if this function already have ADC channel assigned
            if( SYSCFG_INVALID_PIN == *l_gpio_function)
            {
                *l_gpio_function = i_gpio_num;
                CNFG_DBG("apss_store_gpio_pin: func_id[0x%02X] is mapped to pin 0x%02X", i_func_id, *l_gpio_function);
            }
            else
            {
                CMDH_TRAC_ERR("apss_store_gpio_pin: Function ID is duplicated (id:0x%x, pin:%d)", i_func_id, i_gpio_num);

                /* @
                 * @errortype
                 * @moduleid    DATA_STORE_APSS_DATA
                 * @reasoncode  INVALID_INPUT_DATA
                 * @userdata1   function ID
                 * @userdata2   gpio number
                 * @userdata4   ERC_APSS_GPIO_DUPLICATED_FAILURE
                 * @devdesc     Invalid function ID or channel number
                 */
                l_err = createErrl(DATA_STORE_APSS_DATA,
                                   INVALID_INPUT_DATA,
                                   ERC_APSS_GPIO_DUPLICATED_FAILURE,
                                   ERRL_SEV_UNRECOVERABLE,
                                   NULL,
                                   DEFAULT_TRACE_SIZE,
                                   (uint32_t)i_func_id,
                                   (uint32_t)i_gpio_num);

                // Callout firmware
                addCalloutToErrl(l_err,
                                 ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                 ERRL_COMPONENT_ID_FIRMWARE,
                                 ERRL_CALLOUT_PRIORITY_HIGH);
            }
        }
    }

    return l_err;
}


// Function Specification
//
// Name:  data_store_apss_config_v20
//
// Description: TODO Add description
//
// End Function Specification
errlHndl_t data_store_apss_config_v20(const cmdh_apss_config_v20_t * i_cmd_ptr,    //New for P9
                                            cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t              l_err = NULL;

    uint16_t l_channel = 0, l_port = 0, l_pin = 0;

    // Set to default value
    memset(&G_sysConfigData.apss_adc_map, SYSCFG_INVALID_ADC_CHAN, sizeof(G_sysConfigData.apss_adc_map));
    memset(&G_sysConfigData.apss_gpio_map, SYSCFG_INVALID_PIN, sizeof(G_sysConfigData.apss_gpio_map));

    // ADC channels info
    for(l_channel=0;(l_channel < MAX_APSS_ADC_CHANNELS) && (NULL == l_err);l_channel++)
    {
        G_sysConfigData.apss_cal[l_channel].gnd_select = i_cmd_ptr->adc[l_channel].gnd_select;
        G_sysConfigData.apss_cal[l_channel].gain       = i_cmd_ptr->adc[l_channel].gain;
        G_sysConfigData.apss_cal[l_channel].offset     = i_cmd_ptr->adc[l_channel].offset;

        // Assign the ADC channels
        l_err = apss_store_adc_channel(i_cmd_ptr->adc[l_channel].assignment, l_channel);
        if (l_err == NULL)
        {
            //Write sensor IDs to the appropriate powr sensors.
            apss_store_ipmi_sensor_id(l_channel, &(i_cmd_ptr->adc[l_channel]));
        }
        CNFG_DBG("data_store_apss_config_v20: Channel %d: FuncID[0x%02X] SID[0x%08X]",
                 l_channel, i_cmd_ptr->adc[l_channel].assignment, i_cmd_ptr->adc[l_channel].ipmisensorId);
        CNFG_DBG("data_store_apss_config_v20: Channel %d: GND[0x%02X] Gain[0x%08X] Offst[0x%08X]",
                 l_channel, G_sysConfigData.apss_cal[l_channel].gnd_select, G_sysConfigData.apss_cal[l_channel].gain,
                 G_sysConfigData.apss_cal[l_channel].offset);
    }

    if(NULL == l_err)
    {

        // GPIO Ports
        for(l_port=0;(l_port < MAX_APSS_GPIO_PORTS) && (NULL == l_err);l_port++)
        {
            // GPIO mode
            G_sysConfigData.apssGpioPortsMode[l_port] = i_cmd_ptr->gpio[l_port].mode;

            // For each pin
            for(l_pin=0; (l_pin < NUM_OF_APSS_PINS_PER_GPIO_PORT) && (NULL == l_err);l_pin++)
            {
                // Assign the GPIO number
                l_err = apss_store_gpio_pin( i_cmd_ptr->gpio[l_port].assignment[l_pin],
                                             (l_port*NUM_OF_APSS_PINS_PER_GPIO_PORT)+l_pin);

                // TODO #cmdh_fsp_cmds_datacnfg_c_001
                // This is only needed for an ITE which would have an OC
                // sensor for a chassis power connector
                // Assign this global the correct pins when the oc pins are passed in.
                G_conn_oc_pins_bitmap = 0x0000;
            }

        }

        if(NULL == l_err)
        {
            // Change Data Request Mask to indicate we got this data
            G_data_cnfg->data_mask |= DATA_MASK_APSS_CONFIG;
            CMDH_TRAC_IMP("Got valid APSS Config data via TMGT");
        }
    }

    return l_err;
}

// Function Specification
//
// Name:  data_store_apss_config
//
// Description: TODO Add description
//
// End Function Specification
errlHndl_t data_store_apss_config(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                        cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t              l_err = NULL;
    // Temporarly cast to version 0x10 struct just to get version number.
    cmdh_apss_config_v20_t *l_cmd_ptr = (cmdh_apss_config_v20_t *)i_cmd_ptr;
    uint16_t                l_data_length = CMDH_DATALEN_FIELD_UINT16(l_cmd_ptr); //Command length
    uint32_t                l_v20_data_sz = sizeof(cmdh_apss_config_v20_t) - sizeof(cmdh_fsp_cmd_header_t);

    if( !( (l_cmd_ptr->version == DATA_APSS_VERSION20) && (l_v20_data_sz == l_data_length) )  )
    {
        CMDH_TRAC_ERR("data_store_apss_config: Invalid System Data packet. Given Version:0x%X",
                 l_cmd_ptr->version);

        /* @
         * @errortype
         * @moduleid    DATA_STORE_APSS_DATA
         * @reasoncode  INVALID_INPUT_DATA
         * @userdata1   data size
         * @userdata2   packet version
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     OCC recieved an invalid data packet from the FSP
         */
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
    }
    else // Version 0x20
    {
        l_err = data_store_apss_config_v20(l_cmd_ptr, o_rsp_ptr);
    }

    return l_err;
}

// Function Specification
//
// Name:   data_store_role
//
// Description: Tell the OCC if it should run as a master or slave.  HTMGT knows
//              which OCC is the master from the MRW.  To be the master OCC
//              requires a connection to the APSS.  Until an OCC is told a role
//              it should default to running as a slave
//
// End Function Specification
errlHndl_t data_store_role(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                 cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t l_errlHndl = NULL;
    uint8_t    l_new_role = OCC_SLAVE;
    uint8_t    l_old_role = G_occ_role;
    ERRL_RC    l_rc       = ERRL_RC_SUCCESS;

    // Cast the command to the struct for this format
    cmdh_set_role_t * l_cmd_ptr = (cmdh_set_role_t *)i_cmd_ptr;

    // Set the OCC role
    l_new_role = l_cmd_ptr->role;

    // Must be in standby state before we can change roles
    if ( CURRENT_STATE() == OCC_STATE_STANDBY )
    {
        if( OCC_MASTER == l_new_role )
        {
            G_occ_role = OCC_MASTER;

            // Run master initializations if we just became master
            extern void  master_occ_init(void);
            master_occ_init();

            // Turn off anything slave related since we are a master
            rtl_clr_run_mask_deferred(RTL_FLAG_NOTMSTR);
            rtl_set_run_mask_deferred(RTL_FLAG_MSTR);

            // Allow APSS tasks to run on OCC master
            rtl_clr_run_mask_deferred(RTL_FLAG_APSS_NOT_INITD);

            CMDH_TRAC_IMP("data_store_role: OCC Role set to Master via TMGT");

            // Change Data Request Mask to indicate we got this data
            G_data_cnfg->data_mask |= DATA_MASK_SET_ROLE;

            // Make sure return code is success
            l_rc = ERRL_RC_SUCCESS;
        }
        else if( (OCC_SLAVE == l_new_role ) ||
                 (OCC_BACKUP_MASTER == l_new_role))
        {
            if(OCC_MASTER == l_old_role)
            {
                G_occ_role = OCC_SLAVE;

                // Turn off anything master related since we are a slave
                rtl_clr_run_mask_deferred(RTL_FLAG_MSTR);
                rtl_set_run_mask_deferred(RTL_FLAG_NOTMSTR);

                // Slave code will automatically recognize we no longer
                // have a master.
            }

            // If this is a backup master occ, we need to be checking the APSS health
            if(OCC_BACKUP_MASTER == l_new_role)
            {
                l_errlHndl = initialize_apss();

                // Initialize APSS communication on the backup OCC (retries internally)
                if( NULL != l_errlHndl )
                {
                    // Don't request due to a backup apss failure. Just log the error.
                    CMDH_TRAC_ERR("data_store_role: APSS init applet returned error: l_rc: 0x%x", l_errlHndl->iv_reasonCode);
                    commitErrl(&l_errlHndl);
                }

                // Allow APSS tasks to run on OCC backup
                rtl_clr_run_mask_deferred(RTL_FLAG_APSS_NOT_INITD);
                CMDH_TRAC_IMP("data_store_role: OCC Role set to Backup Master via TMGT");
            }
            else
            {
                // NOTE: slave initialization is done on all
                //       OCC's during OCC initialization.
                CMDH_TRAC_IMP("data_store_role: OCC Role set to Slave via TMGT");
            }

            // Change Data Request Mask to indicate we got this data
            G_data_cnfg->data_mask |= DATA_MASK_SET_ROLE;

            // Make sure return code is success
            l_rc = ERRL_RC_SUCCESS;

        }
        else
        {
            CMDH_TRAC_ERR("data_store_role: OCC Role from FSP is not recognized by OCC. role = %d", l_new_role);

            l_rc = ERRL_RC_INVALID_DATA;

            /* @
             * @errortype
             * @moduleid    DATA_STORE_GENERIC_DATA
             * @reasoncode  INVALID_INPUT_DATA
             * @userdata1   Reason input data failed
             * @userdata2   Requested role
             * @userdata4   ERC_INVALID_INPUT_DATA
             * @devdesc     Bad config data passed to OCC
             */
            l_errlHndl = createErrl(DATA_STORE_GENERIC_DATA,      //modId
                                    INVALID_INPUT_DATA,           //reasoncode
                                    ERC_INVALID_INPUT_DATA,       //Extended reason code
                                    ERRL_SEV_INFORMATIONAL,       //Severity
                                    NULL,                         //Trace Buf
                                    DEFAULT_TRACE_SIZE,           //Trace Size
                                    l_rc,                         //userdata1
                                    l_new_role);                  //userdata2
        }
    }
    else
    {
        CMDH_TRAC_ERR("data_store_role: Role change requested while OCC is not in standby state. role=%d, state=%d",
                      l_new_role, CURRENT_STATE());

        l_rc = ERRL_RC_INVALID_STATE;

        /* @
         * @errortype
         * @moduleid    DATA_STORE_GENERIC_DATA
         * @reasoncode  INVALID_INPUT_DATA
         * @userdata1   Reason input data failed
         * @userdata2   current state
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Bad config data passed to OCC
         */
        l_errlHndl = createErrl(DATA_STORE_GENERIC_DATA,  //modId
                INVALID_INPUT_DATA,                       //reasoncode
                OCC_NO_EXTENDED_RC,                       //Extended reason code
                ERRL_SEV_INFORMATIONAL,                   //Severity
                NULL,                                     //Trace Buf
                DEFAULT_TRACE_SIZE,                       //Trace Size
                l_rc,                                     //userdata1
                CURRENT_STATE());                         //userdata2
    }

    if( ERRL_RC_SUCCESS != l_rc  )
    {
        // Send back an error response to TMGT
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, l_rc, &l_errlHndl);
    }

    return l_errlHndl;
}

// Function Specification
//
// Name:  data_store_power_cap
//
// Description: This function should only be run by MASTER OCC when
//              power cap data is received from TMGT.
//
// End Function Specification
errlHndl_t data_store_power_cap(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                        cmdh_fsp_rsp_t * i_rsp_ptr)
{
    errlHndl_t                      l_err = NULL;

    // Cast the command to the struct for this format
    cmdh_pcap_config_t * l_cmd_ptr = (cmdh_pcap_config_t *)i_cmd_ptr;
    uint16_t                        l_data_length = 0;
    uint32_t                        l_pcap_data_sz = 0;
    static uint8_t                  L_pcap_count = 0;
    bool                            l_invalid_input = TRUE; //Assume bad input

    l_data_length = CONVERT_UINT8_ARRAY_UINT16(l_cmd_ptr->data_length[0], l_cmd_ptr->data_length[1]);

    // Check version and length
    if(l_cmd_ptr->version == DATA_PCAP_VERSION_20)
    {
        l_pcap_data_sz = sizeof(cmdh_pcap_config_t) - sizeof(cmdh_fsp_cmd_header_t);
        if(l_pcap_data_sz == l_data_length)
        {
            l_invalid_input = FALSE;
        }
    }

    // This is the master OCC and packet data length and version are valid?
    // TMGT should never send this packet to a slave OCC.
    // if the OCC is not master, OR
    // if the version doesn't equal what we expect (0x20), OR
    // if the expected data length does not agree with the actual data length...
    if((OCC_MASTER != G_occ_role) || l_invalid_input)
    {
        CMDH_TRAC_ERR("data_store_power_cap: Invalid Pcap Data packet! OCC_role[%d] Version[0x%02X] Data_size[%u]",
                 G_occ_role, l_cmd_ptr->version, l_data_length);

        /* @
         * @errortype
         * @moduleid    DATA_STORE_PCAP_DATA
         * @reasoncode  INVALID_INPUT_DATA
         * @userdata1   data size
         * @userdata2   packet version (Bytes 0-1) / role (Bytes 2-3)
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     OCC recieved an invalid data packet from the FSP or OCC role is not MASTER
         */
        l_err = createErrl(DATA_STORE_PCAP_DATA,
                           INVALID_INPUT_DATA,
                           OCC_NO_EXTENDED_RC,
                           ERRL_SEV_UNRECOVERABLE,
                           NULL,
                           0,
                           l_data_length,
                           ((uint32_t)l_cmd_ptr->version)<<16 | G_occ_role);

        // Callout firmware
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

    }
    else
    {
        if(l_cmd_ptr->version == DATA_SYS_VERSION_20)
        {
            // Copy power cap data into G_master_pcap_data
            memcpy(&G_master_pcap_data, &l_cmd_ptr->pcap_config, sizeof(cmdh_pcap_config_data_t));
        }

        // The last byte in G_master_pcap_data is a counter that needs to be incremented.
        // It tells the master and slave code that there is new
        // pcap data.  This should not be incremented until
        // after the packet data has been copied into G_master_pcap_data.
        L_pcap_count++;
        G_master_pcap_data.pcap_data_count = L_pcap_count;

        // Change Data Request Mask to indicate we got the data
        // G_data_cnfg->data_mask |= DATA_MASK_PCAP_PRESENT;
        // will update data mask when slave code acquires data
        CMDH_TRAC_IMP("data store pcap: Got valid PCAP Config data via TMGT. Count:%i, Data Cfg mask[%x]",G_master_pcap_data.pcap_data_count, G_data_cnfg->data_mask);
    }
    return l_err;
}

// Function Specification
//
// Name:  data_store_sys_config
//
// Description: TODO Add description
//
// End Function Specification
errlHndl_t data_store_sys_config(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                       cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t                      l_err = NULL;

    // Cast the command to the struct for this format
    cmdh_sys_config_v20_t * l_cmd_ptr = (cmdh_sys_config_v20_t *)i_cmd_ptr;
    uint16_t                        l_data_length = 0;
    uint32_t                        l_sys_data_sz = 0;
    bool                            l_invalid_input = TRUE; //Assume bad input
    uint8_t                         l_coreIndex = 0;

    l_data_length = CMDH_DATALEN_FIELD_UINT16(l_cmd_ptr);

    // Check length and version
    if(l_cmd_ptr->version == DATA_SYS_VERSION_20)
    {
        l_sys_data_sz = sizeof(cmdh_sys_config_v20_t) - sizeof(cmdh_fsp_cmd_header_t);
        if(l_sys_data_sz == l_data_length)
        {
            l_invalid_input = FALSE;
        }
    }

    if(l_invalid_input)
    {
        CMDH_TRAC_ERR("data_store_sys_config: Invalid System Data packet! Version[0x%02X] Data_size[%u]",
                 l_cmd_ptr->version,
                 l_data_length);

        /* @
         * @errortype
         * @moduleid    DATA_STORE_SYS_DATA
         * @reasoncode  INVALID_INPUT_DATA
         * @userdata1   data size
         * @userdata2   packet version
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     OCC recieved an invalid data packet from the FSP
         */
        l_err = createErrl(DATA_STORE_SYS_DATA,
                           INVALID_INPUT_DATA,
                           OCC_NO_EXTENDED_RC,
                           ERRL_SEV_UNRECOVERABLE,
                           NULL,
                           DEFAULT_TRACE_SIZE,
                           l_data_length,
                           (uint32_t)l_cmd_ptr->version);

        // Callout firmware
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);
    }
    else
    {
        if(l_cmd_ptr->version == DATA_SYS_VERSION_20)
        {
            // Copy data
            G_sysConfigData.system_type.byte    = l_cmd_ptr->sys_config.system_type;
            G_sysConfigData.backplane_huid      = l_cmd_ptr->sys_config.backplane_sid;
            G_sysConfigData.apss_huid           = l_cmd_ptr->sys_config.apss_sid;
            G_sysConfigData.proc_huid           = l_cmd_ptr->sys_config.proc_sid;
            CNFG_DBG("data_store_sys_config: SystemType[0x%02X] BPSID[0x%08X] APSSSID[0x%08X] ProcSID[0x%08X]",
                     G_sysConfigData.system_type.byte, G_sysConfigData.backplane_huid, G_sysConfigData.apss_huid,
                     G_sysConfigData.proc_huid);

            //Write core temp and freq sensor ids
            //Core Temp and Freq sensors are always in sequence in the table
            for (l_coreIndex = 0; l_coreIndex < MAX_CORES; l_coreIndex++)
            {
                AMECSENSOR_PTR(TEMP4MSP0C0 + l_coreIndex)->ipmi_sid = l_cmd_ptr->sys_config.core_sid[(l_coreIndex * 2)];
                AMECSENSOR_PTR(FREQA4MSP0C0 + l_coreIndex)->ipmi_sid = l_cmd_ptr->sys_config.core_sid[(l_coreIndex * 2) + 1];
                CNFG_DBG("data_store_sys_config: Core[%d] TempSID[0x%08X] FreqSID[0x%08X]", l_coreIndex,
                         AMECSENSOR_PTR(TEMP4MSP0C0 + l_coreIndex)->ipmi_sid, AMECSENSOR_PTR(FREQA4MSP0C0 + l_coreIndex)->ipmi_sid);
            }
        }

        // Change Data Request Mask to indicate we got this data
        G_data_cnfg->data_mask |= DATA_MASK_SYS_CNFG;
        CMDH_TRAC_IMP("Got valid System Config data via TMGT for system type: 0x%02X", l_cmd_ptr->sys_config.system_type);
    }

    return l_err;
}

// Function Specification
//
// Name:   data_store_thrm_thresholds
//
// Description: Store the thermal control thresholds sent by TMGT. This data is
// sent to all OCCs.
//
// End Function Specification
errlHndl_t data_store_thrm_thresholds(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                            cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t                      l_err = NULL;
    uint16_t                        i = 0;
    uint16_t                        l_data_length = 0;
    uint16_t                        l_exp_data_length = 0;
    uint8_t                         l_frutype = 0;
    cmdh_thrm_thresholds_v20_t*     l_cmd_ptr = (cmdh_thrm_thresholds_v20_t*)i_cmd_ptr;
    uint8_t                         l_num_data_sets = 0;
    bool                            l_invalid_input = TRUE; //Assume bad input
    bool                            l_vrm_frutype = FALSE;

    do
    {
        l_data_length = CMDH_DATALEN_FIELD_UINT16(l_cmd_ptr);

        // Sanity checks on input data, break if:
        //  * data packet is smaller than the base size, OR
        //  * the version doesn't match what we expect,  OR
        //  * the actual data length does not match the expected data length, OR
        //  * the core and quad weights are both zero.
        if(l_cmd_ptr->version == DATA_THRM_THRES_VERSION_20)
        {
            l_num_data_sets = l_cmd_ptr->num_data_sets;
            l_exp_data_length = THRM_THRES_BASE_DATA_SZ_20 +
                (l_num_data_sets * sizeof(cmdh_thrm_thresholds_set_v20_t));

            if((l_exp_data_length == l_data_length) &&
               (l_data_length >= THRM_THRES_BASE_DATA_SZ_20) &&
               (l_cmd_ptr->proc_core_weight || l_cmd_ptr->proc_quad_weight))
            {
                l_invalid_input = FALSE;
            }
        }

        if(l_invalid_input)
        {
            CMDH_TRAC_ERR("data_store_thrm_thresholds: Invalid Thermal Control Threshold Data packet: data_length[%u] version[0x%02X] num_data_sets[%u]",
                     l_data_length,
                     l_cmd_ptr->version,
                     l_num_data_sets);
            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
            break;
        }

        // Store the base data
        G_data_cnfg->thrm_thresh.version          = l_cmd_ptr->version;
        G_data_cnfg->thrm_thresh.proc_core_weight = l_cmd_ptr->proc_core_weight;
        G_data_cnfg->thrm_thresh.proc_quad_weight = l_cmd_ptr->proc_quad_weight;
        G_data_cnfg->thrm_thresh.num_data_sets    = l_cmd_ptr->num_data_sets;

        // Store the FRU related data
        for(i=0; i<l_cmd_ptr->num_data_sets; i++)
        {
            // Get the FRU type
            l_frutype = l_cmd_ptr->data[i].fru_type;

            if((l_frutype >= 0) && (l_frutype < DATA_FRU_MAX))
            {
                // Copy FRU data
                G_data_cnfg->thrm_thresh.data[l_frutype].fru_type = l_frutype;
                G_data_cnfg->thrm_thresh.data[l_frutype].dvfs     = l_cmd_ptr->data[i].dvfs;
                G_data_cnfg->thrm_thresh.data[l_frutype].error    = l_cmd_ptr->data[i].error;
                G_data_cnfg->thrm_thresh.data[l_frutype].pm_dvfs  = l_cmd_ptr->data[i].pm_dvfs;
                G_data_cnfg->thrm_thresh.data[l_frutype].pm_error = l_cmd_ptr->data[i].pm_error;
                G_data_cnfg->thrm_thresh.data[l_frutype].max_read_timeout =
                    l_cmd_ptr->data[i].max_read_timeout;

                // Set a local flag if we get data for VRM FRU type
                if(l_frutype == DATA_FRU_VRM)
                {
                    l_vrm_frutype = TRUE;
                }

                // Useful trace for debugging
                //CMDH_TRAC_INFO("data_store_thrm_thresholds: FRU_type[0x%.2X] T_control[%u] DVFS[%u] Error[%u]",
                //          G_data_cnfg->thrm_thresh.data[l_frutype].fru_type,
                //          G_data_cnfg->thrm_thresh.data[l_frutype].t_control,
                //          G_data_cnfg->thrm_thresh.data[l_frutype].dvfs,
                //          G_data_cnfg->thrm_thresh.data[l_frutype].error);
            }
            else
            {
                // We got an invalid FRU type
                CMDH_TRAC_ERR("data_store_thrm_thresholds: Received an invalid FRU type[0x%.2X] max_FRU_number[0x%.2X]",
                              l_frutype,
                              DATA_FRU_MAX);
                cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                break;
            }
        }

        // Did we get data for VRM FRU type?
        if(l_vrm_frutype)
        {
            // Then, set a global variable so that OCC attempts to talk to
            // the VRMs
            G_vrm_present = 1;
        }
        else
        {
            // No VRM data was received, so do not attempt to talk to the VRMs.
            // Also, make the error count very high so that the health
            // monitor doesn't complain about VRHOT being asserted.
            G_vrm_present = 0;
            G_data_cnfg->thrm_thresh.data[DATA_FRU_VRM].error_count = 0xFF;
            CMDH_TRAC_IMP("data_store_thrm_thresholds: No VRM data was received! OCC won't attempt to talk to VRMs.");
        }

    } while(0);

    if(!l_err)
    {
        // If there were no errors, indicate that we got this data
        G_data_cnfg->data_mask |= DATA_MASK_THRM_THRESHOLDS;
        CMDH_TRAC_IMP("data_store_thrm_thresholds: Got valid Thermal Control Threshold data packet");
    }

    return l_err;
}


// Function Specification
//
// Name:   data_store_mem_cfg
//
// Description: Store the memory configuration for centaurs and/or dimms. This data is
// sent to each OCC individually.
//
// End Function Specification
errlHndl_t data_store_mem_cfg(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t                      l_err = NULL;
    cmdh_mem_cfg_v20_t*             l_cmd_ptr = (cmdh_mem_cfg_v20_t*)i_cmd_ptr;
    uint16_t                        l_data_length = 0;
    uint16_t                        l_exp_data_length = 0;
    uint8_t                         l_num_centaurs = 0;
    uint8_t                         l_num_dimms = 0;
    uint8_t                         l_i2c_engine;
    uint8_t                         l_i2c_port;
    uint8_t                         l_i2c_addr = 0;
    uint8_t                         l_dimm_num = 0;
    int                             i;

    do
    {
        l_data_length = CMDH_DATALEN_FIELD_UINT16((&l_cmd_ptr->header));

        // Clear all sensor IDs (hw and temperature)
        memset(G_sysConfigData.dimm_huids, 0, sizeof(G_sysConfigData.dimm_huids));
        int memctl, dimm;
        for(memctl=0; memctl < MAX_NUM_MEM_CONTROLLERS; ++memctl)
        {
            g_amec->proc[0].memctl[memctl].centaur.temp_sid = 0;
            for(dimm=0; dimm < NUM_DIMMS_PER_CENTAUR; ++dimm)
            {
                g_amec->proc[0].memctl[memctl].centaur.dimm_temps[dimm].temp_sid = 0;
            }
        }

        // Process data based on version
        if(l_cmd_ptr->header.version == DATA_MEM_CFG_VERSION_20)
        {
            // Verify the actual data length matches the expected data length for this version
            l_exp_data_length = sizeof(cmdh_mem_cfg_header_t) - sizeof(cmdh_fsp_cmd_header_t) +
                                (l_cmd_ptr->header.num_data_sets * sizeof(cmdh_mem_cfg_data_set_v20_t));

            if(l_exp_data_length != l_data_length)
            {
               CMDH_TRAC_ERR("data_store_mem_cfg: Invalid mem config data packet: data_length[%u] exp_length[%u] version[0x%02X] num_data_sets[%u]",
               l_data_length,
               l_exp_data_length,
               l_cmd_ptr->header.version,
               l_cmd_ptr->header.num_data_sets);

               cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
               break;
            }

            if (l_cmd_ptr->header.num_data_sets > 0)
            {
                // Store the memory type.  Memory must all be the same type, save from first and verify remaining
                G_sysConfigData.mem_type = l_cmd_ptr->data_set[0].memory_type;
                if(G_sysConfigData.mem_type == MEM_TYPE_NIMBUS)
                {
                    // Nimbus type -- dimm_info1 is I2C engine which must be the same
                    // save from first entry and verify remaining
                    G_sysConfigData.dimm_i2c_engine = l_cmd_ptr->data_set[0].dimm_info1;
                }
                else
                {
                    // TODO - CUMULUS not supported yet
                    //G_sysConfigData.mem_type = MEM_TYPE_CUMULUS;

                    CMDH_TRAC_ERR("data_store_mem_cfg: Invalid mem type 0x%02X in config data packet version[0x%02X] num_data_sets[%u]",
                                  l_cmd_ptr->data_set[0].memory_type,
                                  l_cmd_ptr->header.version,
                                  l_cmd_ptr->header.num_data_sets);

                    cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                    break;
                }

                // Store the hardware sensor ID and the temperature sensor ID
                for(i=0; i<l_cmd_ptr->header.num_data_sets; i++)
                {
                    cmdh_mem_cfg_v20_t*          l_cmd2_ptr = (cmdh_mem_cfg_v20_t*)i_cmd_ptr;
                    cmdh_mem_cfg_data_set_v20_t* l_data_set = &l_cmd2_ptr->data_set[i];

                    // Verify matching memory type and process based on memory type
                    if( (l_data_set->memory_type == G_sysConfigData.mem_type) &&
                        (l_data_set->memory_type == MEM_TYPE_NIMBUS) )
                    {
                        // Nimbus type:  dimm info is I2C Engine, I2C Port, I2C Address
                        l_i2c_engine = l_data_set->dimm_info1;
                        l_i2c_port = l_data_set->dimm_info2;
                        l_i2c_addr = l_data_set->dimm_info3;

                        // Validate the i2c info for this data set.  Any failure will result in error and
                        // memory monitoring disabled.

                        // Only support engine C, D, or E
                        if((l_i2c_engine != PIB_I2C_ENGINE_C) &&
                           (l_i2c_engine != PIB_I2C_ENGINE_D) &&
                           (l_i2c_engine != PIB_I2C_ENGINE_E))
                        {
                            CMDH_TRAC_ERR("data_store_mem_cfg: Invalid I2C engine. entry=%d, engine=%d",
                                          i,
                                          l_i2c_engine);
                            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                            break;
                        }

                        // Engine must be the same for all DIMMs
                        if (l_i2c_engine != G_sysConfigData.dimm_i2c_engine)
                        {
                            CMDH_TRAC_ERR("data_store_mem_cfg: I2c engine mismatch. entry=%d, engine=%d, expected=%d",
                                          i,
                                          l_i2c_engine,
                                          G_sysConfigData.dimm_i2c_engine);
                            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                            break;
                        }

                        // Port must be 0 or 1.
                        if((l_i2c_port != 0) && (l_i2c_port != 1))
                        {
                            CMDH_TRAC_ERR("data_store_mem_cfg: Invalid I2C port. entry=%d, port=%d",
                                          i,
                                          l_i2c_port);
                            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                            break;
                        }

                        // I2C addr must be 0x3z where z is even
                        if( ( (l_i2c_addr & 0xF0) != 0x30) ||
                            ( (l_i2c_addr & 0x01) != 0 ) )
                        {
                            CMDH_TRAC_ERR("data_store_mem_cfg: Invalid I2C address. entry=%d, addr=%d",
                                          i,
                                          l_i2c_addr);
                            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                            break;
                        }

                        // DIMM info is good for this DIMM, save the sensor IDs for the DIMM
                        // The location of the HW sensor ID in the 2D dimm_huids array is used to know i2c port
                        // and i2c address for reading the DIMM.  "centaur num" index is port "dimm num" is
                        // translated from i2c address, we already verified the i2c addr is 0x3z above
                        l_dimm_num = (l_i2c_addr & 0x0F) >> 1;

                        // Store the hardware sensor ID
                        G_sysConfigData.dimm_huids[l_i2c_port][l_dimm_num] = l_data_set->hw_sensor_id;

                        // Store the temperature sensor ID
                        g_amec->proc[0].memctl[l_i2c_port].centaur.dimm_temps[l_dimm_num].temp_sid =
                            l_data_set->temp_sensor_id;

                        l_num_dimms++;

                    }
                    else  // must be cumulus and the "memory type" byte is the centaur#
                    {
                        CMDH_TRAC_ERR("data_store_mem_cfg: Invalid mem type 0x%02X in config data packet entry %d (entry 0 type: 0x%02X)",
                                      l_data_set->memory_type, i, G_sysConfigData.mem_type);

                        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                        break;

#if 0
                        // TODO - CUMULUS not supported yet
                        // per spec for cumulus memory type is the centaur# and dimm info1 is DIMM#
                        uint8_t l_centaur_num = l_data_set->memory_type;
                        l_dimm_num = l_data_set->dimm_info1;

                        // Validate the centaur and dimm #'s for this data set
                        if( (l_centaur_num >= MAX_NUM_CENTAURS) ||
                            (l_dimm_num != 0xFF && l_dimm_num >= NUM_DIMMS_PER_CENTAUR) )
                        {
                            CMDH_TRAC_ERR("data_store_mem_cfg: Invalid dimm or centaur number. entry=%d, cent=%d, dimm=%d",
                                          i,
                                          l_centaur_num,
                                          l_dimm_num);
                            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                            break;
                        }

                        // Per the spec, if dimm_num = 0xFF then this is a centaur ID
                        if(l_dimm_num == 0xFF)
                        {
                            // Store the hardware sensor ID
                            G_sysConfigData.centaur_huids[l_centaur_num] = l_data_set->hw_sensor_id;

                            // Store the temperature sensor ID
                            g_amec->proc[0].memctl[l_centaur_num].centaur.temp_sid = l_data_set->temp_sensor_id;

                            l_num_centaurs++;
                        }
                        else
                        {
                            // Store the hardware sensor ID
                            G_sysConfigData.dimm_huids[l_centaur_num][l_dimm_num] = l_data_set->hw_sensor_id;

                            // Store the temperature sensor ID
                            g_amec->proc[0].memctl[l_centaur_num].centaur.dimm_temps[l_dimm_num].temp_sid =
                                l_data_set->temp_sensor_id;

                            l_num_dimms++;
                        }
#endif
                    } // Cumulus
                } // for each data set
            } // else no data sets given
        } // version 0x20
        else  // version not supported
        {
            CMDH_TRAC_ERR("data_store_mem_cfg: Invalid mem config data packet: version[0x%02X]",
                          l_cmd_ptr->header.version);

            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
        }
    } while(0);

    if(!l_err)
    {
        // If there were no errors, indicate that we got this data
        G_data_cnfg->data_mask |= DATA_MASK_MEM_CFG;
        CMDH_TRAC_IMP("data_store_mem_cfg: Got valid mem cfg packet. type=0x%02X, #cent=%d, #dimm=%d",
                      G_sysConfigData.mem_type, l_num_centaurs, l_num_dimms);

        // No errors so we can enable memory monitoring if the data indicates it should be enabled
        if(l_cmd_ptr->header.num_data_sets == 0)  // num data sets of 0 indicates memory monitoring disabled
        {
            CMDH_TRAC_IMP("Memory monitoring is not allowed (mem config data sets = 0)");
        }
        else
        {
            // This notifies other code that we need to request the mem throttle packet
            // and we need to enable memory monitoring when we enter observation state
            G_mem_monitoring_allowed = TRUE;

            // Require the mem throt packet for going to active state
            SMGR_VALIDATE_DATA_ACTIVE_MASK |= DATA_MASK_MEM_THROT;

            CMDH_TRAC_IMP("Memory monitoring is allowed (mem config data sets = %d)", l_cmd_ptr->header.num_data_sets);
        }
    }
    else
    {
        // Error with data don't enable memory monitoring
        CMDH_TRAC_IMP("data_store_mem_cfg: Memory monitoring not allowed due to error");
    }

    return l_err;
}


// Function Specification
//
// Name:   data_store_mem_throt
//
// Description: Store min/max mem throttle settings. This data is
// sent to each OCC individually.
//
// End Function Specification
errlHndl_t data_store_mem_throt(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                      cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t                      l_err = NULL;
    cmdh_mem_throt_t*               l_cmd_ptr = (cmdh_mem_throt_t*)i_cmd_ptr;
    uint16_t                        l_data_length = 0;
    uint16_t                        l_exp_data_length = 0;
    int                             i;
    uint16_t                        l_configured_mbas = 0;
    bool                            l_invalid_input = TRUE; //Assume bad input

    do
    {
        l_data_length = CMDH_DATALEN_FIELD_UINT16((&l_cmd_ptr->header));

        // Sanity checks on input data, break if:
        //  * the version doesn't match what we expect,  OR
        //  * the actual data length does not match the expected data length.
        if(l_cmd_ptr->header.version == DATA_MEM_THROT_VERSION_20)
        {
            l_exp_data_length = sizeof(cmdh_mem_throt_header_t) - sizeof(cmdh_fsp_cmd_header_t) +
                (l_cmd_ptr->header.num_data_sets * sizeof(cmdh_mem_throt_data_set_t));

            if(l_exp_data_length == l_data_length)
            {
                l_invalid_input = FALSE;
            }
        }

        if(l_invalid_input)
        {
            CMDH_TRAC_ERR("data_store_mem_throt: Invalid mem throttle data packet: data_length[%u] exp_length[%u] version[0x%02X] num_data_sets[%u]",
                     l_data_length,
                     l_exp_data_length,
                     l_cmd_ptr->header.version,
                     l_cmd_ptr->header.num_data_sets);
            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
            break;
        }

        if(l_cmd_ptr->header.version == DATA_MEM_THROT_VERSION_20)
        {
            if(MEM_TYPE_NIMBUS ==  G_sysConfigData.mem_type)
            {
                // Store the memory throttle settings
                for(i=0; i<l_cmd_ptr->header.num_data_sets; i++)
                {
                    nimbus_mem_throt_t* l_data_set = &l_cmd_ptr->data_set[i].nimbus;
                    mem_throt_config_data_t    l_temp_set;
                    uint16_t * l_n_ptr;

                    // Validate the validity of Nimbus:
                    // - MC num (0 for MC01, and 2 for MC23)
                    // - and Port Number (0-3)
                    if((l_data_set->mc_num != 0 && l_data_set->mc_num != 2) ||
                       l_data_set->port_num >= MAX_NUM_MCU_PORTS)
                    {
                        CMDH_TRAC_ERR("data_store_mem_throt: Invalid MC# or Port#. entry=%d, cent=%d, mba=%d",
                                      i,
                                      l_data_set->mc_num,
                                      l_data_set->port_num);
                        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                        break;
                    }

                    // Copy into a temporary buffer while we check for N values of 0
                    memcpy(&l_temp_set, &(l_data_set->min_n_per_port), sizeof(mem_throt_config_data_t));

                    // A 0 for any N value, or power value, is an error
                    for(l_n_ptr = &l_temp_set.min_n_per_mba; l_n_ptr <= &l_temp_set.ovs_mem_power; l_n_ptr++)
                    {
                        if(!(*l_n_ptr))
                        {
                            CMDH_TRAC_ERR("data_store_mem_throt: Memory Throttle N/Power value is 0! MC[%d] Port[%d]",
                                          l_data_set->mc_num, l_data_set->port_num);
                            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                            break;
                        }

                    }
                    if(l_err)
                    {
                        break;
                    }

                    memcpy(&G_sysConfigData.mem_throt_limits[l_data_set->mc_num][l_data_set->port_num],
                           &(l_data_set->min_n_per_port),
                           sizeof(mem_throt_config_data_t));

                    l_configured_mbas |= 1 << (((l_data_set->mc_num<<1) * 2) + l_data_set->port_num);
                }
            }
            else if (MEM_TYPE_CUMULUS ==  G_sysConfigData.mem_type)
            {
                // Store the memory throttle settings
                for(i=0; i<l_cmd_ptr->header.num_data_sets; i++)
                {
                    cumulus_mem_throt_t* l_data_set = &l_cmd_ptr->data_set[i].cumulus;
                    mem_throt_config_data_t    l_temp_set;
                    uint16_t * l_n_ptr;

                    // Validate the centaur and mba #'s for this data set
                    if(l_data_set->centaur_num >= MAX_NUM_CENTAURS ||
                       l_data_set->mba_num >= NUM_MBAS_PER_CENTAUR)
                    {
                        CMDH_TRAC_ERR("data_store_mem_throt: Invalid mba or centaur number. entry=%d, cent=%d, mba=%d",
                                      i,
                                      l_data_set->centaur_num,
                                      l_data_set->mba_num);
                        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                        break;
                    }

                    // Copy into a temporary buffer while we check for N values of 0
                    memcpy(&l_temp_set, &(l_data_set->min_n_per_mba), sizeof(mem_throt_config_data_t));

                    // A 0 for any N value is an error
                    for(l_n_ptr = &l_temp_set.min_n_per_mba; l_n_ptr <= &l_temp_set.ovs_mem_power; l_n_ptr++)
                    {
                        if(!(*l_n_ptr))
                        {
                            CMDH_TRAC_ERR("data_store_mem_throt: Memory Throttle N value is 0! cent[%d] mba[%d]",
                                          l_data_set->centaur_num, l_data_set->mba_num);
                            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                            break;
                        }

                    }
                    if(l_err)
                    {
                        break;
                    }

                    memcpy(&G_sysConfigData.mem_throt_limits[l_data_set->centaur_num][l_data_set->mba_num],
                           &(l_data_set->min_n_per_mba),
                           sizeof(mem_throt_config_data_t));

                    l_configured_mbas |= 1 << ((l_data_set->centaur_num * 2) + l_data_set->mba_num);

                }
            }
        }

    } while(0);

    if(!l_err)
    {
        // If there were no errors, indicate that we got this data
        G_data_cnfg->data_mask |= DATA_MASK_MEM_THROT;
        CMDH_TRAC_IMP("data_store_mem_throt: Got valid mem throt packet. configured_mba_bitmap=0x%04x",
                 l_configured_mbas);

        // Update the configured mba bitmap
// @TEMP @TODO: defined in centaur_control - Not ready yet
//        G_configured_mbas = l_configured_mbas;
    }
    return l_err;
}

// Function Specification
//
// Name:  data_store_ips_config
//
// Description: TODO Add description
//
// End Function Specification
errlHndl_t data_store_ips_config(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                       cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t          l_err = NULL;
    cmdh_ips_config_t   *l_cmd_ptr = (cmdh_ips_config_t *)i_cmd_ptr; // Cast the command to the struct for this format
    uint16_t            l_data_length = CMDH_DATALEN_FIELD_UINT16(l_cmd_ptr);
    uint32_t            l_ips_data_sz = sizeof(cmdh_ips_config_t) - sizeof(cmdh_fsp_cmd_header_t);


    // Check if this is the Master OCC. Check length and version
    if((OCC_MASTER != G_occ_role) ||
       (l_cmd_ptr->iv_version != DATA_IPS_VERSION) ||
       ( l_ips_data_sz != l_data_length) )
    {
        CMDH_TRAC_ERR("data_store_ips_config: Invalid IPS Data packet");

        /* @
         * @errortype
         * @moduleid    DATA_STORE_IPS_DATA
         * @reasoncode  INVALID_INPUT_DATA
         * @userdata1   data size
         * @userdata2   packet version
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     OCC recieved an invalid data packet from the FSP
         */
        l_err = createErrl(DATA_STORE_IPS_DATA,
                           INVALID_INPUT_DATA,
                           OCC_NO_EXTENDED_RC,
                           ERRL_SEV_UNRECOVERABLE,
                           NULL,
                           DEFAULT_TRACE_SIZE,
                           l_data_length,
                           (uint32_t)l_cmd_ptr->iv_version);

        // Callout firmware
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);
    }
    else
    {
        // Copy data somewhere
        memcpy(&G_ips_config_data, &l_cmd_ptr->iv_ips_config, sizeof(cmdh_ips_config_data_t));

        // Change Data Request Mask to indicate we got this data
        G_data_cnfg->data_mask |= DATA_MASK_IPS_CNFG;

        CMDH_TRAC_IMP("Got valid Idle Power Save Config data via TMGT: ipsEnabled[%d] Delay Time to enter IPS[%d], exit IPS[%d]. Utilization to enter IPS[%d], exit IPS[%d]",
                 l_cmd_ptr->iv_ips_config.iv_ipsEnabled,
                 l_cmd_ptr->iv_ips_config.iv_delayTimeforEntry,
                 l_cmd_ptr->iv_ips_config.iv_delayTimeforExit,
                 l_cmd_ptr->iv_ips_config.iv_utilizationForEntry,
                 l_cmd_ptr->iv_ips_config.iv_utilizationForExit );
    }

    return l_err;
}

// Function Specification
//
// Name:  data_store_volt_uplift
//
// Description: Store the Vdd and Vcd vid values sent by TMGT. This data
//              is sent to each OCC.
//
// End Function Specification
errlHndl_t data_store_volt_uplift(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                        cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t             l_err = NULL;
    cmdh_uplift_config_t   *l_cmd_ptr = (cmdh_uplift_config_t *)i_cmd_ptr; // Cast the command to the struct for this format
    uint16_t               l_actual_sz = CMDH_DATALEN_FIELD_UINT16(l_cmd_ptr);
    uint32_t               l_expected_sz = sizeof(cmdh_uplift_config_t) - sizeof(cmdh_fsp_cmd_header_t);
    int8_t                 l_vdd_delta = 0;
    int8_t                 l_vcs_delta = 0;

    // Check length and version
    if((l_cmd_ptr->version != DATA_VOLT_UPLIFT_VERSION) ||
       (l_actual_sz != l_expected_sz))
    {
        CMDH_TRAC_ERR("Invalid Vdd/Vcs Uplift Data packet Version[0x%02X] Size[%d] Expected[%d]",
                 l_cmd_ptr->version,
                 l_actual_sz,
                 l_expected_sz);

        /* @
         * @errortype
         * @moduleid    DATA_STORE_VOLT_UPLIFT
         * @reasoncode  INVALID_INPUT_DATA
         * @userdata1   data size
         * @userdata2   packet version
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     OCC recieved an invalid data packet from the FSP
         */
        l_err = createErrl(DATA_STORE_VOLT_UPLIFT,
                           INVALID_INPUT_DATA,
                           OCC_NO_EXTENDED_RC,
                           ERRL_SEV_UNRECOVERABLE,
                           NULL,
                           DEFAULT_TRACE_SIZE,
                           l_actual_sz,
                           (uint32_t)l_cmd_ptr->version);

        // Callout firmware
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);
    }
    else
    {
        // Check if the new uplift value for Vdd is zero. That means that they
        // are asking OCC to reset the Pstate table to its original state.
        if(l_cmd_ptr->vdd_vid_uplift == 0)
        {
            // Restore original state by using the current Vdd VID uplift
            l_vdd_delta = -(G_sysConfigData.vdd_vid_uplift_cur);
        }
        else
        {
            // Compute the delta uplift that needs to be applied to Vdd
            l_vdd_delta = -(l_cmd_ptr->vdd_vid_uplift) - G_sysConfigData.vdd_vid_uplift_cur;
        }

        // Check if the new uplift value for Vcs is zero. That means that they
        // are asking OCC to reset the Pstate table to its original state.
        if(l_cmd_ptr->vcs_vid_uplift == 0)
        {
            // Restore original state by using the current Vcs VID uplift
            l_vcs_delta = -(G_sysConfigData.vcs_vid_uplift_cur);
        }
        else
        {
            // Compute the delta uplift that needs to be applied to Vcs
            l_vcs_delta = -(l_cmd_ptr->vcs_vid_uplift) - G_sysConfigData.vcs_vid_uplift_cur;
        }

        // Store the new current Vdd and Vcs VID uplift values
        G_sysConfigData.vdd_vid_uplift_cur = -(l_cmd_ptr->vdd_vid_uplift);
        G_sysConfigData.vcs_vid_uplift_cur = -(l_cmd_ptr->vcs_vid_uplift);

        // Store the Vdd and Vcs VID deltas to be applied to the Pstate table
        G_sysConfigData.vdd_vid_delta = l_vdd_delta;
        G_sysConfigData.vcs_vid_delta = l_vcs_delta;

        // Change Data Request Mask to indicate we got this data
        G_data_cnfg->data_mask |= DATA_MASK_VOLT_UPLIFT;

        CMDH_TRAC_IMP("Got valid Vdd/Vcs Uplift Config data: Vdd_vid_delta[%d] Vcs_vid_delta[%d]",
                 G_sysConfigData.vdd_vid_delta,
                 G_sysConfigData.vcs_vid_delta);
    }

    return l_err;
}

// Function Specification
//
// Name:   DATA_store_cnfgdata
//
// Description: Process Set Configuration Data cmd based on format (type) byte
//
// End Function Specification
errlHndl_t DATA_store_cnfgdata (const cmdh_fsp_cmd_t * i_cmd_ptr,
                                cmdh_fsp_rsp_t       * o_rsp_ptr)
{
    errlHndl_t                      l_errlHndl = NULL;
    UINT32                          l_new_data = 0;
    ERRL_RC                         l_rc       = ERRL_RC_INTERNAL_FAIL;

    CMDH_TRAC_IMP("Data Config Packet Received Type: 0x%02x",i_cmd_ptr->data[0]);

    switch (i_cmd_ptr->data[0])
    {
        case DATA_FORMAT_FREQ:
            l_errlHndl = data_store_freq_data(i_cmd_ptr , o_rsp_ptr);
            if(NULL == l_errlHndl)
            {
                l_new_data = DATA_MASK_FREQ_PRESENT;
            }
            break;

        case DATA_FORMAT_SET_ROLE:
            // Initialze our role to either be a master of a slave
            // We must be in Standby State for this command to be
            // accepted.

            l_errlHndl = data_store_role(i_cmd_ptr, o_rsp_ptr);
            if(NULL == l_errlHndl)
            {
                // Set this in case AMEC needs to know about this
                l_new_data = DATA_MASK_SET_ROLE;
            }
            break;

        case DATA_FORMAT_APSS_CONFIG:
            // Initialze APSS settings so that OCC can correctly interpret
            // the data that it gets from the APSS
            l_errlHndl = data_store_apss_config(i_cmd_ptr, o_rsp_ptr);

            if(NULL == l_errlHndl)
            {
                // Set this in case AMEC needs to know about this
                l_new_data = DATA_MASK_APSS_CONFIG;
            }
            break;

        case DATA_FORMAT_POWER_CAP:
            // Store the pcap data in G_master_pcap_data
            l_errlHndl = data_store_power_cap(i_cmd_ptr, o_rsp_ptr);

            if(NULL == l_errlHndl)
            {
                // Set this in case AMEC needs to know about this
                l_new_data = DATA_MASK_PCAP_PRESENT;
            }
            break;

        case DATA_FORMAT_SYS_CNFG:
            // Store the system config data in G_sysConfigData
            l_errlHndl = data_store_sys_config(i_cmd_ptr, o_rsp_ptr);

            if(NULL == l_errlHndl)
            {
                // Set this in case AMEC needs to know about this
                l_new_data = DATA_MASK_SYS_CNFG;
            }
            break;

        case DATA_FORMAT_IPS_CNFG:
            // Store the system config data in G_sysConfigData
            l_errlHndl = data_store_ips_config(i_cmd_ptr, o_rsp_ptr);

            if(NULL == l_errlHndl)
            {
                // Set this in case AMEC needs to know about this
                l_new_data = DATA_MASK_IPS_CNFG;
            }
            break;

        case DATA_FORMAT_THRM_THRESHOLDS:
            // Store the thermal control thresholds sent by TMGT
            l_errlHndl = data_store_thrm_thresholds(i_cmd_ptr, o_rsp_ptr);

            if(NULL == l_errlHndl)
            {
                // Set this in case AMEC needs to know about this
                l_new_data = DATA_MASK_THRM_THRESHOLDS;
            }
            break;

        case DATA_FORMAT_MEM_CFG:
            // Store memory configuration for present centaurs and/or dimms to monitor
            l_errlHndl = data_store_mem_cfg(i_cmd_ptr, o_rsp_ptr);

            if(NULL == l_errlHndl)
            {
                l_new_data = DATA_MASK_MEM_CFG;
            }
            break;

        case DATA_FORMAT_MEM_THROT:
            // Store memory throttle limits
            l_errlHndl = data_store_mem_throt(i_cmd_ptr, o_rsp_ptr);

            if(NULL == l_errlHndl)
            {
                l_new_data = DATA_MASK_MEM_THROT;
            }
            break;

        case DATA_FORMAT_VOLT_UPLIFT:
            l_errlHndl = data_store_volt_uplift(i_cmd_ptr , o_rsp_ptr);
            if(NULL == l_errlHndl)
            {
                l_new_data = DATA_MASK_VOLT_UPLIFT;
            }
            break;

        case DATA_FORMAT_WOF_CORE_FREQ:
            // FIXME: Need to add proper support for this data packet
            break;

        case DATA_FORMAT_WOF_VRM_EFF:
            // FIXME: Need to add proper support for this data packet
            break;

        case DATA_FORMAT_CLEAR_ALL:
            // Make sure not in ACTIVE
            if(CURRENT_STATE() != OCC_STATE_ACTIVE)
            {
                // Clear all configuration data except for any data needed to support observation
                CMDH_TRAC_INFO("Clear all active configuration data");
                G_data_cnfg->data_mask &= SMGR_VALIDATE_DATA_OBSERVATION_MASK;

                // Clear the frequencies config data
                memset(&G_sysConfigData.sys_mode_freq.table[0], 0, sizeof(G_sysConfigData.sys_mode_freq.table));

            }
            else
            {
                CMDH_TRAC_ERR("Failed to clear all active configuration data because we are in ACTIVE state");
                l_rc = ERRL_RC_INVALID_STATE;
            }
            break;

        default:
            // Build Error Response packet, we are calling this here
            // to generate the error log, it will get called again, below but
            // that's ok, as long as we set l_rc here.
            l_rc = ERRL_RC_INVALID_DATA;
            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, l_rc, &l_errlHndl);
            break;
    }

    if((!l_errlHndl) && (l_new_data))
    {
        // Notify AMEC component of new data
        AMEC_data_change(l_new_data);
    }

    if(l_errlHndl)
    {
        // Build Error Response packet
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, l_rc, &l_errlHndl);
    }
    else
    {
        /// Build Response Packet - all formats return success with no data
        o_rsp_ptr->data_length[0] = 0;
        o_rsp_ptr->data_length[1] = 0;
        G_rsp_status = ERRL_RC_SUCCESS;
    }

    return(l_errlHndl);
}

