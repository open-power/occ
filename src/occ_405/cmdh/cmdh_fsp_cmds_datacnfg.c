/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cmdh/cmdh_fsp_cmds_datacnfg.c $                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2023                        */
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
#include "proc_pstate.h"
#include <amec_data.h>
#include "amec_amester.h"
#include "amec_service_codes.h"
#include "amec_sys.h"
#include "memory.h"
#include <avsbus.h>
#include "pstates_occ.H"
#include <wof.h>
#include <i2c.h>

#define DATA_PCAP_VERSION_20       0x20

#define DATA_SYS_VERSION_30        0x30

#define DATA_APSS_VERSION20        0x20

#define DATA_THRM_THRES_VERSION_30 0x30
#define THRM_THRES_BASE_DATA_SZ_30 6

#define DATA_IPS_VERSION           0

#define DATA_MEM_CFG_VERSION_30    0x30

#define DATA_MEM_THROT_VERSION_40  0x40

#define DATA_VRM_FAULT_VERSION     0x01

#define DATA_AVSBUS_VERSION_30     0x30

// Hard codes for Rainier Vdd socket power capping
#define RAINIER_VDD_SOCKET_CAP_W          625
#define RAINIER_VDD_SOCKET_LOW_CAP_W      625  // no hysteresis
#define RAINIER_DELTA_CHIP_MHZ_PER_W_DEC   87  // 5 Pstates
#define RAINIER_DELTA_CHIP_MHZ_PER_W_INC   17  // 1 Pstate
#define RAINIER_NUM_TICKS_DEC_WAIT          5  // 2.5ms
#define RAINIER_NUM_TICKS_INC_WAIT          1  // 500us
#define RAINIER_PROPORTIONAL_CONTROL        0  // non-proportional

extern uint8_t G_occ_interrupt_type;

extern OCCPstateParmBlock_t G_oppb;   // OCC Pstate Parameters Block Structure
extern uint32_t G_first_proc_gpu_config;
extern uint32_t G_first_num_gpus_sys;
extern uint32_t G_curr_num_gpus_sys;
extern uint32_t G_curr_proc_gpu_config;
extern bool     G_gpu_config_done;
extern bool     G_gpu_monitoring_allowed;
extern task_t   G_task_table[TASK_END];
extern bool G_simics_environment;

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
    {DATA_MASK_AVSBUS_CONFIG,         DATA_FORMAT_AVSBUS_CONFIG},
    {DATA_MASK_SET_ROLE,              DATA_FORMAT_SET_ROLE},
    {DATA_MASK_MEM_CFG,               DATA_FORMAT_MEM_CFG},
    {DATA_MASK_THRM_THRESHOLDS,       DATA_FORMAT_THRM_THRESHOLDS},
    {DATA_MASK_PCAP_PRESENT,          DATA_FORMAT_POWER_CAP},
    {DATA_MASK_MEM_THROT,             DATA_FORMAT_MEM_THROT},
    {DATA_MASK_GPU,                   DATA_FORMAT_GPU},
};

cmdh_ips_config_data_t G_ips_config_data = {0};

bool G_mem_monitoring_allowed = FALSE;

uint8_t G_read_ocmb_num_8ms_ticks = 1;
uint32_t G_num_ocmb_reads_per_1000s = 3906;  // default assuming 200us update time (256ms reads)

// Will get set when receiving APSS config data
PWR_READING_TYPE G_pwr_reading_type = PWR_READING_TYPE_NONE;

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

errlHndl_t DATA_get_thrm_thresholds(cmdh_thrm_thresholds_v30_t **o_thrm_thresh)
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
// Description: Determine what config data should be requested from TMGT
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

        // Skip whenever we are trying to request pcap as a slave
        if((G_data_pri_table[i].format == DATA_FORMAT_POWER_CAP) &&
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

            case ADC_12V_STANDBY_CURRENT:
                l_adc_function = &G_sysConfigData.apss_adc_map.current_12v_stby;
                break;

            case ADC_GPU_0_0:
                l_adc_function = &G_sysConfigData.apss_adc_map.gpu[0][0];
                break;

            case ADC_GPU_0_1:
                l_adc_function = &G_sysConfigData.apss_adc_map.gpu[0][1];
                break;

            case ADC_GPU_0_2:
                l_adc_function = &G_sysConfigData.apss_adc_map.gpu[0][2];
                break;

            case ADC_GPU_1_0:
                l_adc_function = &G_sysConfigData.apss_adc_map.gpu[1][0];
                break;

            case ADC_GPU_1_1:
                l_adc_function = &G_sysConfigData.apss_adc_map.gpu[1][1];
                break;

            case ADC_GPU_1_2:
                l_adc_function = &G_sysConfigData.apss_adc_map.gpu[1][2];
                break;

            case ADC_PCIe_CURRENT:
                l_adc_function = &G_sysConfigData.apss_adc_map.pcie;
                break;

            case ADC_VPCIE_CURRENT_DCM0:
            case ADC_VPCIE_CURRENT_DCM1:
            case ADC_VPCIE_CURRENT_DCM2:
            case ADC_VPCIE_CURRENT_DCM3:
                l_adc_function = &G_sysConfigData.apss_adc_map.dcm_vpcie[i_func_id-ADC_VPCIE_CURRENT_DCM0];
                break;

            case ADC_VIO_CURRENT_DCM0:
            case ADC_VIO_CURRENT_DCM1:
            case ADC_VIO_CURRENT_DCM2:
            case ADC_VIO_CURRENT_DCM3:
                l_adc_function = &G_sysConfigData.apss_adc_map.dcm_vio[i_func_id-ADC_VIO_CURRENT_DCM0];
                break;

            case ADC_AVDD_CURRENT_TOTAL:
                l_adc_function = &G_sysConfigData.apss_adc_map.avdd_total;
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
    uint8_t l_proc  = G_pbax_id.chip_id;

    switch (i_adc->assignment)
    {
        case ADC_MEMORY_PROC_0:
        case ADC_MEMORY_PROC_1:
        case ADC_MEMORY_PROC_2:
        case ADC_MEMORY_PROC_3:
            if (l_proc == (i_adc->assignment - ADC_MEMORY_PROC_0))
            {
                AMECSENSOR_PTR(PWRMEM)->ipmi_sid = i_adc->ipmisensorId;
            }
            break;

        case ADC_GPU_0_0:
        case ADC_GPU_0_1:
        case ADC_GPU_0_2:
            if((i_adc->ipmisensorId != 0) && (l_proc == 0))
            {
                AMECSENSOR_PTR(PWRGPU)->ipmi_sid = i_adc->ipmisensorId;
            }
            break;

        case ADC_GPU_1_0:
        case ADC_GPU_1_1:
        case ADC_GPU_1_2:
            if((i_adc->ipmisensorId != 0) && (l_proc == 1))
            {
                AMECSENSOR_PTR(PWRGPU)->ipmi_sid = i_adc->ipmisensorId;
            }
            break;

        default:
            // None
            break;
    }

    //Write sensor ID to channel sensors.  If the assignment(function id) is 0, that means
    //the channel is not being utilized.
    if ((i_channel < MAX_APSS_ADC_CHANNELS) && (i_adc->assignment != ADC_RESERVED))
    {
        if ((i_adc->ipmisensorId == 0) && (G_occ_interrupt_type != FSP_SUPPORTED_OCC))
        {
            // Sensor IDs are not required and only used for BMC based systems
            CMDH_TRAC_INFO("apss_store_ipmi_sensor_id: No Sensor ID for channel %i.",i_channel);
            //We need to generate a generic sensor ID if we want channels with functionIDs but
            //no sensor IDs to be reported in the poll command.
        }

        //Only store sensor ids for power sensors.  voltage and gnd remote sensors do not report power used.
        if ((i_adc->assignment != ADC_12V_SENSE) &&
            (i_adc->assignment != ADC_GND_REMOTE_SENSE) &&
            (i_adc->assignment != ADC_12V_STANDBY_CURRENT))
        {
            AMECSENSOR_PTR(PWRAPSSCH00 + i_channel)->ipmi_sid = i_adc->ipmisensorId;
            CNFG_DBG("apss_store_ipmi_sensor_id: SID[0x%08X] stored as 0x%08X for channel %d",
                     i_adc->ipmisensorId, AMECSENSOR_PTR(PWRAPSSCH00 + i_channel)->ipmi_sid, i_channel);
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

            case PSU_FAN_DISABLE_N:
                l_gpio_function = &G_sysConfigData.apss_gpio_map.psu_fan_disable;
                break;

            case GPU_0_0_PRSNT_N:
            case GPU_0_1_PRSNT_N:
            case GPU_0_2_PRSNT_N:
            case GPU_1_0_PRSNT_N:
            case GPU_1_1_PRSNT_N:
            case GPU_1_2_PRSNT_N:
                l_gpio_function = &G_sysConfigData.apss_gpio_map.gpu[i_func_id-GPU_0_0_PRSNT_N];
                break;

            case NVDIMM_EPOW_N:
                l_gpio_function = &G_sysConfigData.apss_gpio_map.nvdimm_epow;
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
// Description: Configuration required for APSS
//
// End Function Specification
errlHndl_t data_store_apss_config_v20(const cmdh_apss_config_v20_t * i_cmd_ptr,
                                            cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t              l_err = NULL;
    uint16_t l_channel = 0, l_port = 0, l_pin = 0, l_num_channels = MAX_APSS_ADC_CHANNELS;

    // ADC channels info
    if(G_pwr_reading_type == PWR_READING_TYPE_2_CHANNEL)
       l_num_channels = 2;
    else
    {
       // This must be APSS type, however it is possible that xml and/or HTMGT doesn't support
       // indication of no APSS so we will default to none and then set to APSS if valid channel found
       G_pwr_reading_type = PWR_READING_TYPE_NONE;
    }

    for(l_channel=0;(l_channel < l_num_channels) && (NULL == l_err);l_channel++)
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

            // APSS is present if there is at least one channel with a valid assignment
            if( (i_cmd_ptr->adc[l_channel].assignment != ADC_RESERVED) &&
                (G_pwr_reading_type == PWR_READING_TYPE_NONE) )
            {
                G_pwr_reading_type = PWR_READING_TYPE_APSS;
            }
        }
        CNFG_DBG("data_store_apss_config_v20: Channel %d: FuncID[0x%02X] SID[0x%08X]",
                 l_channel, i_cmd_ptr->adc[l_channel].assignment, i_cmd_ptr->adc[l_channel].ipmisensorId);
        CNFG_DBG("data_store_apss_config_v20: Channel %d: GND[0x%02X] Gain[0x%08X] Offst[0x%08X]",
                 l_channel, G_sysConfigData.apss_cal[l_channel].gnd_select, G_sysConfigData.apss_cal[l_channel].gain,
                 G_sysConfigData.apss_cal[l_channel].offset);
    }

    if( (NULL == l_err) && (G_pwr_reading_type == PWR_READING_TYPE_APSS) ) // only APSS has GPIO config
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
            }

        }
    }

    return l_err;
}

// Function Specification
//
// Name:  data_store_apss_config
//
// Description: Configuration required for APSS
//
// End Function Specification
errlHndl_t data_store_apss_config(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                        cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t              l_err = NULL;
    bool                    l_invalid_data = FALSE;
    cmdh_apss_config_v20_t *l_cmd_ptr = (cmdh_apss_config_v20_t *)i_cmd_ptr;
    uint16_t                l_data_length = CMDH_DATALEN_FIELD_UINT16(l_cmd_ptr); //Command length
    uint32_t                l_v20_data_sz = sizeof(cmdh_apss_config_v20_t) - sizeof(cmdh_fsp_cmd_header_t);


    // Set to default values
    memset(&G_sysConfigData.apss_adc_map, SYSCFG_INVALID_ADC_CHAN, sizeof(G_sysConfigData.apss_adc_map));
    memset(&G_sysConfigData.apss_gpio_map, SYSCFG_INVALID_PIN, sizeof(G_sysConfigData.apss_gpio_map));

    // only version 0x20 supported and data length must be at least 4
    if( (l_cmd_ptr->version != DATA_APSS_VERSION20) || (l_data_length < 4))
    {
       l_invalid_data = TRUE;
    }
    else
    {
       // verify the data length and process the data based on power reading type
       // PWR_READING_TYPE_APSS --> full size of structure
       // PWR_READING_TYPE_2_CHANNEL --> 0x20 bytes (2 ADC channels, no GPIOs)
       // PWR_READING_TYPE_NONE --> 4 bytes (no channel or GPIO data)

       G_pwr_reading_type = l_cmd_ptr->type;

       if( ( (G_pwr_reading_type == PWR_READING_TYPE_2_CHANNEL) && (l_data_length == 0x20) ) ||
                ( (G_pwr_reading_type == PWR_READING_TYPE_APSS) && (l_v20_data_sz == l_data_length) ) )
       {
          l_err = data_store_apss_config_v20(l_cmd_ptr, o_rsp_ptr);

          if(!l_err)
          {
              // Turn on Vdd socket power capping to prevent Vdd VRM slow trip on Rainier
              // we know Rainier based on hard coded APSS config
              if( (G_apss_ch_to_function[2] == ADC_VDD_PROC_0) &&
                  (G_apss_ch_to_function[3] == ADC_VDD_PROC_1) &&
                  (G_apss_ch_to_function[4] == ADC_VCS_VIO_VPCIE_PROC_0) &&
                  (G_apss_ch_to_function[5] == ADC_VCS_VIO_VPCIE_PROC_1) )
              {
                 G_sysConfigData.vdd_socket_pcap_w = RAINIER_VDD_SOCKET_CAP_W;
                 G_sysConfigData.vdd_socket_low_w = RAINIER_VDD_SOCKET_LOW_CAP_W;
                 G_sysConfigData.delta_chip_mhz_per_watt_drop = RAINIER_DELTA_CHIP_MHZ_PER_W_DEC;
                 G_sysConfigData.delta_chip_mhz_per_watt_raise = RAINIER_DELTA_CHIP_MHZ_PER_W_INC;
                 G_sysConfigData.num_ticks_drop_wait = RAINIER_NUM_TICKS_DEC_WAIT;
                 G_sysConfigData.num_ticks_raise_wait = RAINIER_NUM_TICKS_INC_WAIT;
                 G_sysConfigData.socket_pcap_proportional_control = RAINIER_PROPORTIONAL_CONTROL;

                 // No total socket power capping
                 G_sysConfigData.total_socket_pcap_w = 0;
                 G_sysConfigData.total_socket_low_w  = 0;

                 CMDH_TRAC_IMP("data_store_apss_config: Enabling Vdd power cap[%04dW] drop[%04d mhz] raise[%04d mhz]",
                                G_sysConfigData.vdd_socket_pcap_w,
                                G_sysConfigData.delta_chip_mhz_per_watt_drop,
                                G_sysConfigData.delta_chip_mhz_per_watt_raise);
                 CMDH_TRAC_IMP("data_store_apss_config: Alg wait times decrease[%04d ticks] increase[%04d ticks] Proportional[%02d]",
                                G_sysConfigData.num_ticks_drop_wait,
                                G_sysConfigData.num_ticks_raise_wait,
                                G_sysConfigData.socket_pcap_proportional_control);
              }
          }
       }
       else if( (G_pwr_reading_type != PWR_READING_TYPE_NONE) || (l_data_length != 4) )
       {
          l_invalid_data = TRUE;
       }
    }

    if(l_invalid_data)
    {
        G_pwr_reading_type = PWR_READING_TYPE_NONE;
        CMDH_TRAC_ERR("data_store_apss_config: Invalid APSS Config Data packet. Given Version:0x%02X length:0x%04X",
                 l_cmd_ptr->version, l_data_length);

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
    else if(NULL == l_err)
    {
        // Change Data Request Mask to indicate we got this data
        G_data_cnfg->data_mask |= DATA_MASK_APSS_CONFIG;
        CMDH_TRAC_IMP("Got valid APSS Config data via TMGT; Pwr reading type = %d", G_pwr_reading_type);
    }

    return l_err;
}

// Function Specification
//
// Name:  data_store_avsbus_config
//
// Description: Configuration required to read power/current from AVS Bus
//
// End Function Specification
errlHndl_t data_store_avsbus_config(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t l_err = NULL;
    const uint16_t AVSBUS_V30_LENGTH = sizeof(cmdh_avsbus_v30_config_t) - sizeof(cmdh_fsp_cmd_header_t);
    bool l_invalid_data = FALSE;

    cmdh_avsbus_v30_config_t *l_cmd_ptr = (cmdh_avsbus_v30_config_t *)i_cmd_ptr;
    uint16_t l_data_length = CMDH_DATALEN_FIELD_UINT16(l_cmd_ptr);

    if ( ((DATA_AVSBUS_VERSION_30 == l_cmd_ptr->version) && (AVSBUS_V30_LENGTH == l_data_length)) )
    {
        // common code for all versions
        // Validate Vdd
        if ((l_cmd_ptr->vdd_bus >= 0) && (l_cmd_ptr->vdd_bus <= 2))
        {
            if ((l_cmd_ptr->vdd_rail >= 0) && (l_cmd_ptr->vdd_rail <= 15))
            {
                // may be getting Vdd from PGPE but this could be enabled with internal flag
                // for OCC to handle overflow, that checking will be done in amec_update_avsbus_sensors()
                G_avsbus_vdd_monitoring = TRUE;
                G_sysConfigData.avsbus_vdd.bus = l_cmd_ptr->vdd_bus;
                G_sysConfigData.avsbus_vdd.rail = l_cmd_ptr->vdd_rail;
                CNFG_DBG("data_store_avsbus_config: Vdd bus[%d] rail[%d]",
                         G_sysConfigData.avsbus_vdd.bus, G_sysConfigData.avsbus_vdd.rail);
            }
            else
            {
                CMDH_TRAC_ERR("data_store_avsbus_config: Invalid AVS Bus Vdd rail 0x%02X",
                              l_cmd_ptr->vdd_rail);
                l_invalid_data = TRUE;
            }
        }
        else
        {
            if (l_cmd_ptr->vdd_bus != 0xFF)
            {
                CMDH_TRAC_ERR("data_store_avsbus_config: Invalid AVS Bus Vdd bus 0x%02X",
                              l_cmd_ptr->vdd_bus);
                l_invalid_data = TRUE;
            }
            else
            {
                CMDH_TRAC_INFO("data_store_avsbus_config: Vdd will not be monitored via AVS Bus");
                G_avsbus_vdd_monitoring = FALSE;
            }
        }
    }
    else
    {
        CMDH_TRAC_ERR("data_store_avsbus_config: Invalid AVS Bus version/length (0x%02X/0x%04X))",
                      l_cmd_ptr->version, l_data_length);
        l_invalid_data = TRUE;
    }

    if(l_invalid_data)
    {
        if (G_simics_environment)
        {
            CMDH_TRAC_ERR("data_store_avsbus_config: Ignoring missing AVSBUS data in simics");
        }
        else
        {
            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
        }
        G_avsbus_vdd_monitoring = FALSE;

        CMDH_TRAC_ERR("Invalid VDD, Vdd temperature not monitored");
    }
    else
    {
        // Vdd Current roll over workaround is disabled
        G_sysConfigData.vdd_current_rollover_10mA = 0xFFFF;  // no rollover
        G_sysConfigData.vdd_max_current_10mA = 0xFFFF;

        // We can use vdd/vdn. Clear NO_VDD_VDN_READ mask
        set_clear_wof_disabled( CLEAR,
                                WOF_RC_INVALID_VDD_VDN,
                                ERC_WOF_INVALID_VDD_VDN );
        avsbus_init();
    }

    if(!l_err)
    {
        // If there were no errors, indicate that we got this data
        G_data_cnfg->data_mask |= DATA_MASK_AVSBUS_CONFIG;
        CMDH_TRAC_IMP("data_store_avsbus_config: Got valid AVS Bus data packet");
    }

    return l_err;

} // end data_store_avsbus_config()

// Function Specification
//
// Name:  data_store_gpu
//
// Description: GPU information
//
// End Function Specification
errlHndl_t data_store_gpu(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t l_err = NULL;
    uint8_t i = 0;
    uint8_t l_gpu_num = 0;
    cmdh_gpu_config_v2_t *l_cmd_ptr = (cmdh_gpu_config_v2_t *)i_cmd_ptr;
    uint16_t l_data_length = CMDH_DATALEN_FIELD_UINT16((&l_cmd_ptr->header));
    uint16_t l_gpu_data_length = 0;
    uint8_t  l_present_bit_mask = 0;  // Bit mask for present GPUs behind this OCC

    // parse data based on version. Version byte is located at same offset for all versions
    if(l_cmd_ptr->header.version == 1)
    {
        cmdh_gpu_config_t *l_cmd_ptr_v1 = (cmdh_gpu_config_t *)i_cmd_ptr;
        l_data_length = CMDH_DATALEN_FIELD_UINT16(l_cmd_ptr_v1);
        l_gpu_data_length = sizeof(cmdh_gpu_config_t) - sizeof(cmdh_fsp_cmd_header_t);
        if(l_gpu_data_length == l_data_length)
        {
            G_sysConfigData.total_non_gpu_max_pwr_watts = l_cmd_ptr_v1->total_non_gpu_max_pwr_watts;
            G_sysConfigData.total_proc_mem_pwr_drop_watts = l_cmd_ptr_v1->total_proc_mem_pwr_drop_watts;

            AMECSENSOR_PTR(TEMPGPU0)->ipmi_sid = l_cmd_ptr_v1->gpu0_temp_sid;
            AMECSENSOR_PTR(TEMPGPU0MEM)->ipmi_sid = l_cmd_ptr_v1->gpu0_mem_temp_sid;
            G_sysConfigData.gpu_sensor_ids[0]  = l_cmd_ptr_v1->gpu0_sid;

            AMECSENSOR_PTR(TEMPGPU1)->ipmi_sid = l_cmd_ptr_v1->gpu1_temp_sid;
            AMECSENSOR_PTR(TEMPGPU1MEM)->ipmi_sid = l_cmd_ptr_v1->gpu1_mem_temp_sid;
            G_sysConfigData.gpu_sensor_ids[1]  = l_cmd_ptr_v1->gpu1_sid;

            AMECSENSOR_PTR(TEMPGPU2)->ipmi_sid = l_cmd_ptr_v1->gpu2_temp_sid;
            AMECSENSOR_PTR(TEMPGPU2MEM)->ipmi_sid = l_cmd_ptr_v1->gpu2_mem_temp_sid;
            G_sysConfigData.gpu_sensor_ids[2]  = l_cmd_ptr_v1->gpu2_sid;

            G_data_cnfg->data_mask |= DATA_MASK_GPU;
            CMDH_TRAC_IMP("data_store_gpu: Got valid GPU data packet");
        }
        else
        {
            CMDH_TRAC_ERR("data_store_gpu: GPU version 1 invalid length Expected: 0x%04X  Received: 0x%04X",
                          l_gpu_data_length, l_data_length);
            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_CMD_LEN, &l_err);
        }
    } // if version 1
    else if(l_cmd_ptr->header.version == 2)
    {
        l_gpu_data_length = sizeof(cmdh_gpu_cfg_header_v2_t) - sizeof(cmdh_fsp_cmd_header_t);
        l_gpu_data_length += (l_cmd_ptr->header.num_data_sets * sizeof(cmdh_gpu_set_v2_t));

        if(l_gpu_data_length == l_data_length)
        {
            if( (l_cmd_ptr->header.gpu_i2c_engine == PIB_I2C_ENGINE_C) &&
                ((l_cmd_ptr->header.gpu_i2c_bus_voltage == 0) || (l_cmd_ptr->header.gpu_i2c_bus_voltage == 18)) )
            {
                G_sysConfigData.gpu_i2c_engine = l_cmd_ptr->header.gpu_i2c_engine;
                G_sysConfigData.gpu_i2c_bus_voltage = l_cmd_ptr->header.gpu_i2c_bus_voltage;
                CMDH_TRAC_IMP("data_store_gpu: I2C engine = 0x%02X I2C bus voltage = %d deci volts",
                               G_sysConfigData.gpu_i2c_engine, G_sysConfigData.gpu_i2c_bus_voltage);

                G_sysConfigData.total_non_gpu_max_pwr_watts = l_cmd_ptr->header.total_non_gpu_max_pwr_watts;
                G_sysConfigData.total_proc_mem_pwr_drop_watts = l_cmd_ptr->header.total_proc_mem_pwr_drop_watts;

                // Store the individual GPU data
                for(i=0; i<l_cmd_ptr->header.num_data_sets; i++)
                {
                    // Get the GPU number data is for
                    l_gpu_num = l_cmd_ptr->gpu_data[i].gpu_num;

                    if( (l_gpu_num >= 0) && (l_gpu_num < MAX_NUM_GPU_PER_DOMAIN) )
                    {
                        G_sysConfigData.gpu_i2c_info[l_gpu_num].port = l_cmd_ptr->gpu_data[i].i2c_port;
                        G_sysConfigData.gpu_i2c_info[l_gpu_num].address = l_cmd_ptr->gpu_data[i].i2c_addr;

                        // if port or i2c address is 0xFF the GPU will not be monitored
                        if( (G_sysConfigData.gpu_i2c_info[l_gpu_num].port != 0xFF) &&
                           (G_sysConfigData.gpu_i2c_info[l_gpu_num].address != 0xFF) )
                        {
                            CMDH_TRAC_IMP("data_store_gpu: GPU%d I2C port = 0x%02X address = 0x%02X", l_gpu_num,
                                           G_sysConfigData.gpu_i2c_info[l_gpu_num].port,
                                           G_sysConfigData.gpu_i2c_info[l_gpu_num].address);

                            AMECSENSOR_PTR(TEMPGPU0 + l_gpu_num)->ipmi_sid = l_cmd_ptr->gpu_data[i].gpu_temp_sid;
                            AMECSENSOR_PTR(TEMPGPU0MEM + l_gpu_num)->ipmi_sid = l_cmd_ptr->gpu_data[i].gpu_mem_temp_sid;
                            G_sysConfigData.gpu_sensor_ids[l_gpu_num]  = l_cmd_ptr->gpu_data[i].gpu_sid;

                            // If there is no APSS this data is giving GPU presence, mark this GPU as present
                            if(G_pwr_reading_type != PWR_READING_TYPE_APSS)
                            {
                                l_present_bit_mask |= (0x01 << l_gpu_num);
                            }
                        }
                        else
                        {
                            CMDH_TRAC_ERR("data_store_gpu: GPU%d NOT monitored Invalid I2C port = 0x%02X I2C address = 0x%02X",
                                           l_gpu_num, G_sysConfigData.gpu_i2c_info[l_gpu_num].port,
                                           G_sysConfigData.gpu_i2c_info[l_gpu_num].address);
                        }
                    }
                    else
                    {
                        // We got an invalid GPU number
                        CMDH_TRAC_ERR("data_store_gpu: Received invalid GPU number %d", l_gpu_num);
                        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                        break;
                    }
                } // for each GPU data set

                // if there is no APSS for GPU presence then this data is the GPU presence
                if(G_pwr_reading_type != PWR_READING_TYPE_APSS)
                {
                    if(l_err == NULL)
                    {
                        G_first_num_gpus_sys = l_cmd_ptr->header.total_num_gpus_system;
                        G_curr_num_gpus_sys = G_first_num_gpus_sys;
                        G_first_proc_gpu_config = l_present_bit_mask;
                        G_curr_proc_gpu_config = G_first_proc_gpu_config;
                        G_gpu_config_done = TRUE;

                        if(G_first_proc_gpu_config)
                        {
                            // GPUs are present enable monitoring
                            G_gpu_monitoring_allowed = TRUE;
                            G_task_table[TASK_ID_GPU_SM].flags = GPU_RTL_FLAGS;
                        }

                        CMDH_TRAC_IMP("data_store_gpu: This OCC GPUs present mask = 0x%02X Total number GPUs present in system = %d",
                                       G_first_proc_gpu_config, G_first_num_gpus_sys);

                        G_data_cnfg->data_mask |= DATA_MASK_GPU;
                        CMDH_TRAC_IMP("data_store_gpu: Got valid GPU data packet");
                    }
                    else
                    {
                        G_first_num_gpus_sys = 0;
                        G_curr_num_gpus_sys = 0;
                        G_first_proc_gpu_config = 0;
                        G_curr_proc_gpu_config = 0;
                        G_gpu_config_done = FALSE;
                    }
                }
                else if(l_err == NULL)
                {
                    G_data_cnfg->data_mask |= DATA_MASK_GPU;
                    CMDH_TRAC_IMP("data_store_gpu: Got valid GPU data packet");
                }
            }  // valid i2c engine and voltage
            else
            {
                // We got an invalid I2C Engine and/or voltage
                CMDH_TRAC_ERR("data_store_gpu: Received invalid I2C Engine/Voltage 0x%02X / %d",
                               l_cmd_ptr->header.gpu_i2c_engine, l_cmd_ptr->header.gpu_i2c_bus_voltage);
                cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
            }
        } // if length valid
        else
        {
            CMDH_TRAC_ERR("data_store_gpu: GPU version 2 invalid length Expected: 0x%04X  Received: 0x%04X",
                          l_gpu_data_length, l_data_length);
            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_CMD_LEN, &l_err);
        }
    } //else if version 2
    else
    {
        CMDH_TRAC_ERR("data_store_gpu: Invalid GPU version 0x%02X", l_cmd_ptr->header.version);
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
    }

    return l_err;

} // end data_store_gpu()

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

            // Allow APSS tasks to run on OCC master if APSS is present
            if(G_pwr_reading_type == PWR_READING_TYPE_APSS)
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
                if(G_pwr_reading_type == PWR_READING_TYPE_APSS)
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
        if(l_cmd_ptr->version == DATA_PCAP_VERSION_20)
        {
            // Copy power cap limits data into G_master_pcap_data
            cmdh_pcap_config_t * l_cmd2_ptr = (cmdh_pcap_config_t *)i_cmd_ptr;
            G_master_pcap_data.soft_min_pcap   = l_cmd2_ptr->pcap_config.soft_min_pcap;
            G_master_pcap_data.hard_min_pcap   = l_cmd2_ptr->pcap_config.hard_min_pcap;
            G_master_pcap_data.max_pcap        = l_cmd2_ptr->pcap_config.sys_max_pcap;
            G_master_pcap_data.oversub_pcap    = l_cmd2_ptr->pcap_config.qpd_pcap;
            G_master_pcap_data.system_pcap     = l_cmd2_ptr->pcap_config.sys_max_pcap;

            // NOTE: The customer power cap will be set via a separate command
            // from BMC/(H)TMGT or OPAL.
        }

        // The last byte in G_master_pcap_data is a counter that needs to be incremented.
        // It tells the master and slave code that there is new
        // pcap data.  This should not be incremented until
        // after the packet data has been copied into G_master_pcap_data.
        G_master_pcap_data.pcap_data_count++;

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
// Description: Store system configuration data from TMGT
//
// End Function Specification
errlHndl_t data_store_sys_config(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                       cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t                      l_err = NULL;

    // Cast the command to the struct for this format
    cmdh_sys_config_v30_t * l_cmd_ptr = (cmdh_sys_config_v30_t *)i_cmd_ptr;
    uint16_t                        l_data_length = 0;
    uint32_t                        l_sys_data_sz = 0;
    bool                            l_invalid_input = TRUE; //Assume bad input
    uint8_t                         l_coreIndex = 0;

    l_data_length = CMDH_DATALEN_FIELD_UINT16(l_cmd_ptr);

    // Check length and version
    if(l_cmd_ptr->version == DATA_SYS_VERSION_30)
    {
        l_sys_data_sz = sizeof(cmdh_sys_config_v30_t) - sizeof(cmdh_fsp_cmd_header_t);
        if(l_sys_data_sz == l_data_length)
        {
            l_invalid_input = FALSE;
        }
    }

    if(l_invalid_input)
    {
        CMDH_TRAC_ERR("data_store_sys_config: Invalid System Data packet! Version[0x%02X] Data_size[%u] Expected Size[%u]",
                 l_cmd_ptr->version,
                 l_data_length,
                 l_sys_data_sz);

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
    else  // version and length is valid, store the data
    {
        // Copy data that is common to all versions
        G_sysConfigData.system_type.byte    = l_cmd_ptr->sys_config.system_type;
        G_sysConfigData.backplane_huid      = l_cmd_ptr->sys_config.backplane_sid;
        G_sysConfigData.apss_huid           = l_cmd_ptr->sys_config.apss_sid;
        G_sysConfigData.proc_huid           = l_cmd_ptr->sys_config.proc_sid;
        G_sysConfigData.proc_freq_huid      = l_cmd_ptr->sys_config.proc_freq_sid;

        CNFG_DBG("data_store_sys_config: SystemType[0x%02X] BPSID[0x%08X] APSSSID[0x%08X] ProcSID[0x%08X]"
                 "ProcFreqSID[0x%08X]", G_sysConfigData.system_type.byte, G_sysConfigData.backplane_huid,
                 G_sysConfigData.apss_huid, G_sysConfigData.proc_huid, G_sysConfigData.proc_freq_huid);

        // Check if we have to disable WOF due to reset limit
        if (G_sysConfigData.system_type.wof_reset_limit)
        {
            CMDH_TRAC_INFO("WOF Disabled due to reset limit");
            set_clear_wof_disabled( SET,
                                    WOF_RC_RESET_LIMIT_REACHED,
                                    ERC_WOF_RESET_LIMIT_REACHED );
        }

        // Check to see if we have to disable WOF due to no mode set yet on PowerVM
        if( !G_sysConfigData.system_type.kvm &&
           (CURRENT_MODE() == OCC_MODE_NOCHANGE) )
        {
            set_clear_wof_disabled(SET,
                                   WOF_RC_MODE_NO_SUPPORT_MASK,
                                   ERC_WOF_MODE_NO_SUPPORT_MASK);
        }

        //Write core temp sensor ids
        //Core Temp sensors are always in sequence in the table
        for (l_coreIndex = 0; l_coreIndex < MAX_CORES; l_coreIndex++)
        {
            AMECSENSOR_PTR(TEMPPROCTHRMC0 + l_coreIndex)->ipmi_sid = l_cmd_ptr->sys_config.core_sid[l_coreIndex];

            CNFG_DBG("data_store_sys_config: Core[%d] TempSID[0x%08X]", l_coreIndex,
                     AMECSENSOR_PTR(TEMPPROCTHRMC0 + l_coreIndex)->ipmi_sid);
        }

        // Store the VRM Vdd Sensor IDs
        G_sysConfigData.vrm_vdd_huid      = l_cmd_ptr->sys_config.vrm_vdd_sid;
        AMECSENSOR_PTR(TEMPVDD)->ipmi_sid = l_cmd_ptr->sys_config.vrm_vdd_temp_sid;

        // Change Data Request Mask to indicate we got this data
        G_data_cnfg->data_mask |= DATA_MASK_SYS_CNFG;
        CMDH_TRAC_IMP("Got valid System Config data via TMGT for system type: 0x%02X", l_cmd_ptr->sys_config.system_type);
    }

    return l_err;

} // end data_store_sys_config()


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
    cmdh_thrm_thresholds_v30_t*     l_cmd_ptr = (cmdh_thrm_thresholds_v30_t*)i_cmd_ptr;
    uint8_t                         l_num_data_sets = 0;
    uint8_t                         l_dvfs = 0;
    uint8_t                         l_error = 0;
    uint8_t                         l_temperature = 0;
    bool                            l_invalid_input = TRUE; //Assume bad input
    bool                            l_data_valid = FALSE;


    do
    {
        l_data_length = CMDH_DATALEN_FIELD_UINT16(l_cmd_ptr);

        // Sanity checks on input data, break if:
        //  * data packet is smaller than the base size, OR
        //  * the version doesn't match what we expect,  OR
        //  * the actual data length does not match the expected data length, OR
        //  * the core and quad weights are both zero.
        if(l_cmd_ptr->version == DATA_THRM_THRES_VERSION_30)
        {
            l_num_data_sets = l_cmd_ptr->num_data_sets;
            l_exp_data_length = THRM_THRES_BASE_DATA_SZ_30 +
                (l_num_data_sets * sizeof(cmdh_thrm_thresholds_set_v30_t));

            if((l_exp_data_length == l_data_length) &&
               (l_data_length >= THRM_THRES_BASE_DATA_SZ_30) &&
               (l_cmd_ptr->proc_core_weight || l_cmd_ptr->proc_racetrack_weight || l_cmd_ptr->proc_L3_weight) )
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

        // clear all FRU types to 0xFF (not defined) to prevent errors when (H)TMGT doesn't send thresholds for a FRU type
        for(i=0; i<DATA_FRU_MAX; i++)
        {
            G_data_cnfg->thrm_thresh.data[i].fru_type = i;
            G_data_cnfg->thrm_thresh.data[i].dvfs     = 0xFF;
            G_data_cnfg->thrm_thresh.data[i].error    = 0xFF;
            G_data_cnfg->thrm_thresh.data[i].max_read_timeout = 0xFF;
        }

        // Store the base data
        G_data_cnfg->thrm_thresh.version          = l_cmd_ptr->version;
        G_data_cnfg->thrm_thresh.proc_core_weight = l_cmd_ptr->proc_core_weight;
        G_data_cnfg->thrm_thresh.proc_racetrack_weight = l_cmd_ptr->proc_racetrack_weight;
        G_data_cnfg->thrm_thresh.proc_L3_weight = l_cmd_ptr->proc_L3_weight;
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
                G_data_cnfg->thrm_thresh.data[l_frutype].max_read_timeout =
                    l_cmd_ptr->data[i].max_read_timeout;

                // Useful trace for debugging
                //CMDH_TRAC_INFO("data_store_thrm_thresholds: FRU_type[0x%.2X] T_control[%u] DVFS[%u] Error[%u]",
                //          G_data_cnfg->thrm_thresh.data[l_frutype].fru_type,
                //          G_data_cnfg->thrm_thresh.data[l_frutype].t_control,
                //          G_data_cnfg->thrm_thresh.data[l_frutype].dvfs,
                //          G_data_cnfg->thrm_thresh.data[l_frutype].error);
            }
            else if((l_frutype == DATA_FRU_PROC_DELTAS) || (l_frutype == DATA_FRU_PROC_IO_DELTAS))
            {
                // FRU data for ERROR and DVFS are deltas applied to VPD #V TDP value
                if(l_frutype == DATA_FRU_PROC_DELTAS)
                    l_temperature = G_oppb.tdp_sort_power_temp_0p5C / 2;

                else // DATA_FRU_PROC_IO_DELTAS
                    l_temperature = G_oppb.io_throttle_temp_0p5C / 2;

                // only save if we have a temperature from VPD
                if(l_temperature != 0)
                {
                    l_data_valid = TRUE;

                    // prevent overflow if applying negative delta
                    if( (l_cmd_ptr->data[i].dvfs > 0) || (l_temperature >= -l_cmd_ptr->data[i].dvfs) )
                        l_dvfs = l_temperature + l_cmd_ptr->data[i].dvfs;
                    else
                        l_dvfs = l_temperature;

                    if( (l_cmd_ptr->data[i].error > 0) || (l_temperature >= -l_cmd_ptr->data[i].error) )
                        l_error = l_temperature + l_cmd_ptr->data[i].error;
                    else
                        l_error = l_temperature;

                    CMDH_TRAC_INFO("data_store_thrm_thresholds: FRU_type[0x%02X] DVFS Delta[%d] Error Delta[%d]",
                                   l_frutype, l_cmd_ptr->data[i].dvfs, l_cmd_ptr->data[i].error);
                    CMDH_TRAC_INFO("data_store_thrm_thresholds: FRU_type[0x%02X] VPD Temp[%d] DVFS[%d] Error[%d]",
                                   l_frutype, l_temperature, l_dvfs, l_error);
                }
                else
                {
                    l_data_valid = FALSE;
                    CMDH_TRAC_ERR("data_store_thrm_thresholds: No VPD temp for FRU_type[0x%02X]", l_frutype);
                }

                if(l_data_valid)
                {
                    // mask off upper nibble used to indicate deltas
                    l_frutype &= 0x0F;

                    G_data_cnfg->thrm_thresh.data[l_frutype].fru_type = l_frutype;
                    G_data_cnfg->thrm_thresh.data[l_frutype].dvfs     = l_dvfs;
                    G_data_cnfg->thrm_thresh.data[l_frutype].error    = l_error;
                    G_data_cnfg->thrm_thresh.data[l_frutype].max_read_timeout =
                    l_cmd_ptr->data[i].max_read_timeout;
                }
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
// Description: Store the memory configuration for membufs and/or dimms. This data is
// sent to each OCC individually.
//
// End Function Specification
errlHndl_t data_store_mem_cfg(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t                      l_err = NULL;
    cmdh_mem_cfg_v30_t*             l_cmd_ptr = (cmdh_mem_cfg_v30_t*)i_cmd_ptr;
    uint16_t                        l_data_length = 0;
    uint16_t                        l_exp_data_length = 0;
    uint16_t                        l_ocmb_update_time_ms = 200;
    uint8_t                         l_num_mem_bufs = 0;
    uint8_t                         l_num_dimms = 0;
    uint8_t                         l_dts_num = 0;
    uint8_t                         l_memory_type = 0;
    uint8_t                         num_data_sets = 0;
    uint8_t                         l_i2c_engine;
    int                             i;
    bool                            l_i2c_memory = FALSE;

    do
    {
        l_data_length = CMDH_DATALEN_FIELD_UINT16((&l_cmd_ptr->header));

        // Clear all sensor IDs (hw and temperature)
        memset(G_sysConfigData.dimm_huids, 0, sizeof(G_sysConfigData.dimm_huids));
        int memctl, dimm;
        for(memctl=0; memctl < MAX_NUM_MEM_CONTROLLERS; ++memctl)
        {
            g_amec->proc[0].memctl[memctl].membuf.temp_sid = 0;

            for(dimm=0; dimm < MAX_NUM_DTS_PER_OCMB; ++dimm)
            {
                g_amec->proc[0].memctl[memctl].membuf.dimm_temps[dimm].temp_sid = 0;
            }
        }

        if(l_cmd_ptr->header.version == DATA_MEM_CFG_VERSION_30)
        {
            num_data_sets = l_cmd_ptr->header.num_data_sets;
            // Verify the actual data length matches the expected data length for this version
            l_exp_data_length = sizeof(cmdh_mem_cfg_header_v30_t) - sizeof(cmdh_fsp_cmd_header_t) +
                                (num_data_sets * sizeof(cmdh_mem_cfg_data_set_t));

            if(l_exp_data_length != l_data_length)
            {
                CMDH_TRAC_ERR("data_store_mem_cfg: Invalid mem config data packet: "
                              "data_length[%u] exp_length[%u] version[0x%02X] num_data_sets[%u]",
                              l_data_length,
                              l_exp_data_length,
                              l_cmd_ptr->header.version,
                              num_data_sets);

                cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                break;
            }

            l_ocmb_update_time_ms = l_cmd_ptr->header.update_time_ms;

            // Store the mem config data
            G_sysConfigData.ips_mem_pwr_ctl = l_cmd_ptr->header.ips_mem_pwr_ctl;
            G_sysConfigData.default_mem_pwr_ctl = l_cmd_ptr->header.default_mem_pwr_ctl;
        }
        else
        {
            CMDH_TRAC_ERR("data_store_mem_cfg: Invalid mem config data packet: version[0x%02X]",
                          l_cmd_ptr->header.version);

            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
            break;
        }

        if (num_data_sets > 0)
        {
            // Store the memory type
            l_memory_type = l_cmd_ptr->data_set[0].memory_type & OCMB_TYPE_TYPE_MASK;
            // verify this is a valid memory type, currently only OCM types are valid
            if(IS_OCM_MEM_TYPE(l_memory_type))
               G_sysConfigData.mem_type = l_memory_type;
            else
            {
                // un-supported memory type
                CMDH_TRAC_ERR("data_store_mem_cfg: Un-supported memory type 0x%02X", l_memory_type);
                cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                break;
            }

            // Store the hardware sensor ID and the temperature sensor ID
            for(i=0; i<num_data_sets; i++)
            {
                cmdh_mem_cfg_data_set_t* l_data_set;
                l_data_set = &l_cmd_ptr->data_set[i];
                l_memory_type = (l_data_set->memory_type & OCMB_TYPE_TYPE_MASK);

                // No mixing DDR4 and DDR5 behind a proc. is allowed.
                if( (IS_OCM_DDR4_MEM_TYPE(G_sysConfigData.mem_type) == IS_OCM_DDR4_MEM_TYPE(l_memory_type)) ||
                    (IS_OCM_DDR5_MEM_TYPE(G_sysConfigData.mem_type) == IS_OCM_DDR5_MEM_TYPE(l_memory_type)) )
                {
                    // Get the physical OCMB location from type
                    unsigned int l_membuf_num = l_data_set->memory_type;
                    l_membuf_num &= OCMB_TYPE_LOCATION_MASK;

                    // Validate the memory buffer num for this data set
                    if (l_membuf_num >= MAX_NUM_OCMBS)
                    {
                        CMDH_TRAC_ERR("data_store_mem_cfg: Invalid memory buffer num[0x%02X] for entry %d type/mem_buf[0x%02X] ",
                                      l_membuf_num, i, l_data_set->memory_type);
                        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                        break;
                    }

                    // process dimm info1 byte
                    if( (l_data_set->dimm_info1 == 0xFF) || (l_memory_type != MEM_TYPE_OCM_DDR4_I2C) )
                    {
                       // DIMM info1 is a DTS number
                       l_dts_num = l_data_set->dimm_info1;

                       // Validate the dts num for this data set
                       if(l_dts_num != 0xFF)
                       {
                           if( ((l_memory_type == MEM_TYPE_OCM_DDR4) && (l_dts_num >= NUM_DTS_PER_OCMB_DDR4)) ||
                               ((l_memory_type == MEM_TYPE_OCM_DDR5) && (l_dts_num >= NUM_DTS_PER_OCMB_DDR5)) )
                           {
                              CMDH_TRAC_ERR("data_store_mem_cfg: Invalid memory data for type 0x%02X "
                                            "(entry %d: type/mem_buf[0x%02X], dts[0x%02X])",
                                            l_memory_type, i, l_data_set->memory_type, l_dts_num);
                              cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                              break;
                           }
                       }

                       // membuf must be present if at least one membuf or dimm dts is used.
                       if(l_data_set->dimm_info2 != DATA_FRU_NOT_USED)
                       {
                           G_present_membufs |= MEMBUF0_PRESENT_MASK >> l_membuf_num;
                       }

                       if(l_dts_num == 0xFF) // sensors are for the Memory Buffer itself
                       {
                           // Store the hardware sensor ID
                           G_sysConfigData.membuf_huids[l_membuf_num] = l_data_set->hw_sensor_id;

                           // Store the temperature sensor ID
                           g_amec->proc[0].memctl[l_membuf_num].membuf.temp_sid = l_data_set->temp_sensor_id;

                           // Store the thermal sensor type
                           g_amec->proc[0].memctl[l_membuf_num].membuf.membuf_hottest.temp_fru_type = l_data_set->dimm_info2;

                           l_num_mem_bufs++;
                       }
                       else // individual DTS
                       {
                           // Track TMGT configured/requested DIMM sensors
                           if(l_data_set->dimm_info2 != DATA_FRU_NOT_USED)
                           {
                               G_dimm_configured_sensors.bytes[l_membuf_num] |= (DIMM_SENSOR0 >> l_dts_num);
                           }

                           // Store the hardware sensor ID
                           G_sysConfigData.dimm_huids[l_membuf_num][l_dts_num] = l_data_set->hw_sensor_id;

                           // Store the temperature sensor ID
                           g_amec->proc[0].memctl[l_membuf_num].membuf.dimm_temps[l_dts_num].temp_sid = l_data_set->temp_sensor_id;

                           // Store the temperature sensor fru type
                           // The 2 external temp sensors may be used for non-dimm fru type i.e. PMIC, mem controller...
                           // this fru type is coming from attributes setup by HWP during IPL and then read by (H)TMGT
                           if(l_data_set->dimm_info2 == DATA_FRU_DIMM)
                           {
                                 g_amec->proc[0].memctl[l_membuf_num].membuf.dimm_temps[l_dts_num].temp_fru_type = DATA_FRU_DIMM;
                                 g_amec->proc[0].memctl[l_membuf_num].membuf.dimm_temps[l_dts_num].dts_type_mask = OCM_DTS_TYPE_DIMM_MASK;
                           }
                           else if(l_data_set->dimm_info2 == DATA_FRU_MEMCTRL_DRAM)
                           {
                                 g_amec->proc[0].memctl[l_membuf_num].membuf.dimm_temps[l_dts_num].temp_fru_type = DATA_FRU_MEMCTRL_DRAM;
                                 g_amec->proc[0].memctl[l_membuf_num].membuf.dimm_temps[l_dts_num].dts_type_mask = OCM_DTS_TYPE_MEMCTRL_DRAM_MASK;
                           }
                           else if(l_data_set->dimm_info2 == DATA_FRU_PMIC)
                           {
                                 g_amec->proc[0].memctl[l_membuf_num].membuf.dimm_temps[l_dts_num].temp_fru_type = DATA_FRU_PMIC;
                                 g_amec->proc[0].memctl[l_membuf_num].membuf.dimm_temps[l_dts_num].dts_type_mask = OCM_DTS_TYPE_PMIC_MASK;
                           }
                           else if(l_data_set->dimm_info2 == DATA_FRU_MEMCTRL_EXT)
                           {
                                 g_amec->proc[0].memctl[l_membuf_num].membuf.dimm_temps[l_dts_num].temp_fru_type = DATA_FRU_MEMCTRL_EXT;
                                 g_amec->proc[0].memctl[l_membuf_num].membuf.dimm_temps[l_dts_num].dts_type_mask = OCM_DTS_TYPE_MEMCTRL_EXT_MASK;
                           }
                           else // sensor not used
                           {
                                 g_amec->proc[0].memctl[l_membuf_num].membuf.dimm_temps[l_dts_num].temp_fru_type = DATA_FRU_NOT_USED;
                                 g_amec->proc[0].memctl[l_membuf_num].membuf.dimm_temps[l_dts_num].dts_type_mask = 0;
                                 if (l_data_set->dimm_info2 != DATA_FRU_NOT_USED)
                                 {
                                       // not a valid fru type
                                       CMDH_TRAC_ERR("data_store_mem_cfg: Got invalid fru type[0x%02X] for mem buf[%d] dts[%d]",
                                                     l_data_set->dimm_info2, l_membuf_num, l_dts_num);
                                 }
                           }
                           l_num_dimms++;
                       } // else individual DTS sensor
                    } // end if on chip sensor or non-I2C OCMB Memory type
                    else if(l_memory_type == MEM_TYPE_OCM_DDR4_I2C)
                    {
                       // Sensor must be read via I2C

                        l_i2c_engine = l_data_set->dimm_info1;
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

                        // save engine from first entry and verify remaining
                        if(l_i2c_memory == FALSE)
                        {
                           l_i2c_memory = TRUE;
                           G_sysConfigData.dimm_i2c_engine = l_i2c_engine;
                        }
                        // Engine must be the same for all DIMMs
                        else if (l_i2c_engine != G_sysConfigData.dimm_i2c_engine)
                        {
                            CMDH_TRAC_ERR("data_store_mem_cfg: I2c engine mismatch. entry=%d, engine=%d, expected=%d",
                                          i,
                                          l_i2c_engine,
                                          G_sysConfigData.dimm_i2c_engine);
                            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                            break;
                        }

                        // save the rest of the data.  Only 1 DIMM per OCMB supported, make sure didn't
                        // already save a DIMM for this OCMB
                        if(G_dimm_configured_sensors.bytes[l_membuf_num])
                        {
                            CMDH_TRAC_ERR("data_store_mem_cfg: Received more than 1 DIMM for OCMB[0x%02X]",
                                      l_membuf_num);

                            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                            break;
                        }
                        else
                        {
                           // Store the hardware sensor ID
                           G_sysConfigData.dimm_huids[l_membuf_num][0] = l_data_set->hw_sensor_id;
                           // Store the temperature sensor ID
                           g_amec->proc[0].memctl[l_membuf_num].membuf.dimm_temps[0].temp_sid = l_data_set->temp_sensor_id;
                           // Store the i2c info
                           g_amec->proc[0].memctl[l_membuf_num].membuf.dimm_temps[0].i2c_port = l_data_set->dimm_info2;
                           g_amec->proc[0].memctl[l_membuf_num].membuf.dimm_temps[0].i2c_address = l_data_set->dimm_info3;
                           // set fru type to DIMM
                           g_amec->proc[0].memctl[l_membuf_num].membuf.dimm_temps[0].temp_fru_type = DATA_FRU_DIMM;
                           g_amec->proc[0].memctl[l_membuf_num].membuf.dimm_temps[0].dts_type_mask = OCM_DTS_TYPE_DIMM_MASK;
                           // mark this DIMM as present
                           G_dimm_configured_sensors.bytes[l_membuf_num] = DIMM_SENSOR0;
                        }
                    }
                    else
                    {
                        // un-supported memory type
                        CMDH_TRAC_ERR("data_store_mem_cfg: Un-supported memory type 0x%02X at index %d",
                                      l_memory_type, i);

                        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                        break;
                    }
                 }
                 else
                 {
                     // MISMATCH ON MEMORY TYPE!!
                     CMDH_TRAC_ERR("data_store_mem_cfg: Memory type mismatch at index %d (0x%02X vs 0x%02X)",
                                   i, G_sysConfigData.mem_type, l_data_set->memory_type);

                     cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                     break;
                 }
            } // for each data set

            // set memory type to I2C if there were i2c DIMMs detected
            if(l_i2c_memory)
            {
                // i2c is only supported with DDR4
                if(IS_OCM_DDR4_MEM_TYPE(G_sysConfigData.mem_type))
                {
                   if(G_sysConfigData.mem_type != MEM_TYPE_OCM_DDR4_I2C)
                   {
                       CMDH_TRAC_INFO("data_store_mem_cfg: Setting mem_type from 0x%02X to I2C 0x%02X",
                                       G_sysConfigData.mem_type, MEM_TYPE_OCM_DDR4_I2C);
                       G_sysConfigData.mem_type = MEM_TYPE_OCM_DDR4_I2C;
                   }
                }
                else
                {
                   CMDH_TRAC_ERR("data_store_mem_cfg: I2C detected with invalid memory type 0x%02X",
                                  G_sysConfigData.mem_type);
                   cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                }
            }
        } // else no data sets given
    } while(0);

    if(!l_err)
    {
        // indicate that we got this data
        G_data_cnfg->data_mask |= DATA_MASK_MEM_CFG;
        CMDH_TRAC_IMP("data_store_mem_cfg: Got valid mem cfg packet. type=0x%02X, #mem bufs=%d, #dimm=%d",
                      G_sysConfigData.mem_type, l_num_mem_bufs, l_num_dimms);

        // No errors so we can enable memory monitoring if the data indicates it should be enabled
        if(num_data_sets == 0)  // num data sets of 0 indicates memory monitoring disabled
        {
            CMDH_TRAC_IMP("Memory monitoring is not allowed (mem config data sets = 0), ");
        }
        else
        {
            // calculate how often memory will be read based on how often OCMB is updating
            // the cache line.  OCC shouldn't be reading faster than OCMB update time, but faster
            // than the deadman timer.
            // MAX deadman timer is 668 ms (for P10 l_max_num_8ms_ticks is 5)
            int l_max_dead_8ms_ticks = 640/(8*MAX_NUM_OCMBS);

            // task to read 1 OCMB is called every 8ms
            // divide by (8ms * #OCMBs) to get number of 8ms ticks to read 1 OCMB
            // Add  1 to ocmb sensor cache read is less frequent than ocmb update.
            G_read_ocmb_num_8ms_ticks = 1 + ((l_ocmb_update_time_ms) / (8*MAX_NUM_OCMBS));

            // i.e. 500ms OCMB update time gives 1 + (500)/(8*16) = 4 -> every 4 8ms calls (32ms) 1 OCMB read
            // each OCMB read 32ms*16= 512ms

            // don't allow 0
            if(G_read_ocmb_num_8ms_ticks == 0)
               G_read_ocmb_num_8ms_ticks = 1;

            // G_read_ocmb_num_8ms_ticks can't be greater than the deadman timer (668ms)
            if(G_read_ocmb_num_8ms_ticks > l_max_dead_8ms_ticks)
                G_read_ocmb_num_8ms_ticks = l_max_dead_8ms_ticks;

            // determine number of reads done in 1000s
            // using 1000 for better precision to calculate memory bandwidth sensors
            G_num_ocmb_reads_per_1000s = 1000000 / (G_read_ocmb_num_8ms_ticks * 8 * MAX_NUM_OCMBS);
            CMDH_TRAC_IMP("1 OCMB will be read every %dms Each OCMB read every %dms (%d reads/1000s)",
                          G_read_ocmb_num_8ms_ticks * 8,
                          G_read_ocmb_num_8ms_ticks * 8 * MAX_NUM_OCMBS,
                          G_num_ocmb_reads_per_1000s);

            // This notifies other code that we need to request the mem throttle packet
            // and we need to enable memory monitoring when we enter observation state
            G_mem_monitoring_allowed = TRUE;

            // Require the mem throt packet for going to active state
            SMGR_VALIDATE_DATA_ACTIVE_MASK |= DATA_MASK_MEM_THROT;

            CMDH_TRAC_IMP("Memory monitoring is allowed (mem config data sets = %d,"
                          " ips_mem_pwr_ctl = %d, default_mem_pwr_ctl = %d)",
                          num_data_sets, G_sysConfigData.ips_mem_pwr_ctl,
                          G_sysConfigData.default_mem_pwr_ctl);
        }
    }
    else
    {
        // Error with data don't enable memory monitoring
        CMDH_TRAC_IMP("data_store_mem_cfg: Memory monitoring not allowed due to error");
    }

    return l_err;

} // end data_store_mem_cfg()


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
    uint8_t                         i;
    uint16_t                        l_configured_membufs = 0;
    bool                            l_invalid_input = TRUE; //Assume bad input

    do
    {
        l_data_length = CMDH_DATALEN_FIELD_UINT16((&l_cmd_ptr->header));

        // Sanity checks on input data, break if:
        //  * the version doesn't match what we expect,  OR
        //  * the actual data length does not match the expected data length.
        if(l_cmd_ptr->header.version == DATA_MEM_THROT_VERSION_40)
        {
            l_exp_data_length = sizeof(cmdh_mem_throt_header_t) - sizeof(cmdh_fsp_cmd_header_t) +
                (l_cmd_ptr->header.num_data_sets * sizeof(cmdh_mem_throt_data_set_v40_t));

            if(l_exp_data_length == l_data_length)
            {
                l_invalid_input = FALSE;
            }
        }

        if(l_invalid_input)
        {
            CMDH_TRAC_ERR("data_store_mem_throt: Invalid mem throttle data packet: "
                          "data_length[%u] exp_length[%u] version[0x%02X] num_data_sets[%u]",
                          l_data_length,
                          l_exp_data_length,
                          l_cmd_ptr->header.version,
                          l_cmd_ptr->header.num_data_sets);
            cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
            break;
        }

        // Store the memory throttle settings
        for(i=0; i<l_cmd_ptr->header.num_data_sets; i++)
        {
            uint8_t mem_buf = 0xFF; // membuf Info Parameters

            mem_buf = l_cmd_ptr->data_set[i].mem_throt_info.membuf_num;

            // Validate parameters
            if(mem_buf >= MAX_NUM_OCMBS)
            {
                CMDH_TRAC_ERR("data_store_mem_throt: Invalid memory data for type 0x%02X "
                              "(entry %d: mem_buf[%d]",
                              G_sysConfigData.mem_type, i, mem_buf);
                cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                break;
            }

            // Save throttles, a 0 for any N value is an error
            if(l_cmd_ptr->data_set[i].min_n_per_mba == 0)
            {
                CMDH_TRAC_ERR("data_store_mem_throt: Min N per MBA is 0! mem_buf[%d]", mem_buf);
                cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                break;
            }
            G_sysConfigData.mem_throt_limits[mem_buf].min_n_per_mba = l_cmd_ptr->data_set[i].min_n_per_mba;

            if( (l_cmd_ptr->data_set[i].fmax_n_per_mba == 0) ||
                (l_cmd_ptr->data_set[i].fmax_n_per_chip == 0) )
            {
                CMDH_TRAC_ERR("data_store_mem_throt: 0 N value found for Fmax per_mba[%d] per_chip[%d] mem_buf[%d]",
                               l_cmd_ptr->data_set[i].fmax_n_per_mba,
                               l_cmd_ptr->data_set[i].fmax_n_per_chip,
                               mem_buf);
                cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                break;
            }
            G_sysConfigData.mem_throt_limits[mem_buf].fmax_n_per_mba = l_cmd_ptr->data_set[i].fmax_n_per_mba;
            G_sysConfigData.mem_throt_limits[mem_buf].fmax_n_per_chip = l_cmd_ptr->data_set[i].fmax_n_per_chip;

            if( (l_cmd_ptr->data_set[i].mode_disabled_n_per_mba == 0) ||
                (l_cmd_ptr->data_set[i].mode_disabled_n_per_chip == 0) )
            {
                CMDH_TRAC_ERR("data_store_mem_throt: 0 N value found for Mode Disabled per_mba[%d] per_chip[%d] mem_buf[%d]",
                               l_cmd_ptr->data_set[i].mode_disabled_n_per_mba,
                               l_cmd_ptr->data_set[i].mode_disabled_n_per_chip,
                               mem_buf);
                cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                break;
            }
            G_sysConfigData.mem_throt_limits[mem_buf].mode_disabled_n_per_mba = l_cmd_ptr->data_set[i].mode_disabled_n_per_mba;
            G_sysConfigData.mem_throt_limits[mem_buf].mode_disabled_n_per_chip = l_cmd_ptr->data_set[i].mode_disabled_n_per_chip;

            if( (l_cmd_ptr->data_set[i].ut_n_per_mba == 0) ||
                (l_cmd_ptr->data_set[i].ut_n_per_chip == 0) )
            {
                CMDH_TRAC_ERR("data_store_mem_throt: 0 N value found for Ultra Turbo per_mba[%d] per_chip[%d] mem_buf[%d]",
                               l_cmd_ptr->data_set[i].ut_n_per_mba,
                               l_cmd_ptr->data_set[i].ut_n_per_chip,
                               mem_buf);
                cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                break;
            }
            G_sysConfigData.mem_throt_limits[mem_buf].ut_n_per_mba = l_cmd_ptr->data_set[i].ut_n_per_mba;
            G_sysConfigData.mem_throt_limits[mem_buf].ut_n_per_chip = l_cmd_ptr->data_set[i].ut_n_per_chip;

            if( (l_cmd_ptr->data_set[i].oversub_n_per_mba == 0) ||
                (l_cmd_ptr->data_set[i].oversub_n_per_chip == 0) )
            {
                CMDH_TRAC_ERR("data_store_mem_throt: 0 N value found for Oversubscription per_mba[%d] per_chip[%d] mem_buf[%d]",
                               l_cmd_ptr->data_set[i].oversub_n_per_mba,
                               l_cmd_ptr->data_set[i].oversub_n_per_chip,
                               mem_buf);
                cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, ERRL_RC_INVALID_DATA, &l_err);
                break;
            }
            G_sysConfigData.mem_throt_limits[mem_buf].oversub_n_per_mba = l_cmd_ptr->data_set[i].oversub_n_per_mba;
            G_sysConfigData.mem_throt_limits[mem_buf].oversub_n_per_chip = l_cmd_ptr->data_set[i].oversub_n_per_chip;

            l_configured_membufs |= (0x1 << mem_buf);

	} // data_sets for loop
    } while(0);

    if(!l_err)
    {
        // If there were no errors, indicate that we got this data
        G_data_cnfg->data_mask |= DATA_MASK_MEM_THROT;
        CMDH_TRAC_IMP("data_store_mem_throt: Got valid mem throt packet. configured_mba_bitmap=0x%04x",
                 l_configured_membufs);

        // Update the configured mba bitmap
        G_configured_mbas = l_configured_membufs;
    }

    return l_err;

} // end data_store_mem_throt()


// Function Specification
//
// Name:  data_store_ips_config
//
// Description: Store IPS config data from TMGT
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
// Name:  data_store_socket_pwr_config
//
// Description: Store socket power configuration data from TMGT
//
// End Function Specification
errlHndl_t data_store_socket_pwr_config(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                              cmdh_fsp_rsp_t * o_rsp_ptr)
{
    errlHndl_t                      l_err = NULL;

    // Cast the command to the struct for this format
    cmdh_socket_pwr_config_t * l_cmd_ptr = (cmdh_socket_pwr_config_t *)i_cmd_ptr;
    uint16_t                        l_data_length = 0;
    uint32_t                        l_sys_data_sz = 0;
    bool                            l_invalid_input = TRUE; //Assume bad input
    bool                            l_supported = TRUE; //Assume socket capping supported
    bool                            l_log_no_support_error = FALSE;

    // Socket capping is only supported on Rainier, we know Rainier by APSS proc channel assignment
    // and no other P10 system matches this.  APSS config MUST be sent prior to this config packet
    if( (G_apss_ch_to_function[2] != ADC_VDD_PROC_0) ||
        (G_apss_ch_to_function[3] != ADC_VDD_PROC_1) ||
        (G_apss_ch_to_function[4] != ADC_VCS_VIO_VPCIE_PROC_0) ||
        (G_apss_ch_to_function[5] != ADC_VCS_VIO_VPCIE_PROC_1) )
    {
        // we will log an error later only if data is to enable a power cap
        l_supported = FALSE;
    }

    l_data_length = CMDH_DATALEN_FIELD_UINT16(l_cmd_ptr);

    // Check length and version
    if(l_cmd_ptr->version == 0)
    {
        l_sys_data_sz = sizeof(cmdh_socket_pwr_config_t) - sizeof(cmdh_fsp_cmd_header_t);
        if(l_sys_data_sz == l_data_length)
        {
            l_invalid_input = FALSE;
        }
    }

    if(l_invalid_input)
    {
        CMDH_TRAC_ERR("data_store_socket_pwr_config: Invalid Socket Power Data packet! Version[0x%02X] Data_size[%u] Expected Size[%u]",
                 l_cmd_ptr->version,
                 l_data_length,
                 l_sys_data_sz);

        /* @
         * @errortype
         * @moduleid    DATA_STORE_SOCKET_DATA
         * @reasoncode  INVALID_INPUT_DATA
         * @userdata1   data size
         * @userdata2   packet version
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     OCC recieved an invalid data packet from the FSP
         */
        l_err = createErrl(DATA_STORE_SOCKET_DATA,
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
    else  // version and length is valid, store the data
    {
        G_sysConfigData.vdd_socket_pcap_w     = l_cmd_ptr->socket_pwr_config.vdd_socket_pcap_w;
        G_sysConfigData.vdd_socket_low_w      = l_cmd_ptr->socket_pwr_config.vdd_socket_low_w;
        G_sysConfigData.total_socket_pcap_w   = l_cmd_ptr->socket_pwr_config.total_socket_pcap_w;
        G_sysConfigData.total_socket_low_w    = l_cmd_ptr->socket_pwr_config.total_socket_low_w;
        G_sysConfigData.delta_chip_mhz_per_watt_drop = l_cmd_ptr->socket_pwr_config.delta_chip_mhz_per_watt_drop;
        G_sysConfigData.delta_chip_mhz_per_watt_raise = l_cmd_ptr->socket_pwr_config.delta_chip_mhz_per_watt_raise;
        G_sysConfigData.num_ticks_drop_wait = l_cmd_ptr->socket_pwr_config.num_ticks_drop_wait;
        G_sysConfigData.num_ticks_raise_wait = l_cmd_ptr->socket_pwr_config.num_ticks_raise_wait;
        G_sysConfigData.socket_pcap_proportional_control = l_cmd_ptr->socket_pwr_config.proportional_control;

        // Change Data Request Mask to indicate we got this data
        G_data_cnfg->data_mask |= DATA_MASK_SOCKET_PCAP;
        if( (G_sysConfigData.vdd_socket_pcap_w) && (G_sysConfigData.vdd_socket_pcap_w != 0xFFFF) )
        {
           if(l_supported)
           {
              CMDH_TRAC_IMP("Vdd Socket Power Cap enabled [%d W] low[%d W] drop[%d mhz] raise[%d mhz]",
                             G_sysConfigData.vdd_socket_pcap_w,
                             G_sysConfigData.vdd_socket_low_w,
                             G_sysConfigData.delta_chip_mhz_per_watt_drop,
                             G_sysConfigData.delta_chip_mhz_per_watt_raise);
           }
           else
              l_log_no_support_error = TRUE;
        }
        else
        {
           CMDH_TRAC_IMP("Vdd Socket Power Cap 0x%04X disabled! last clip %d Mhz",
                          G_sysConfigData.vdd_socket_pcap_w,
                          g_amec->proc[0].pwr_votes.socket_vdd_pcap_clip_freq);

           g_amec->proc[0].pwr_votes.socket_vdd_pcap_clip_freq = 0xFFFF;
        }

        if( (G_sysConfigData.total_socket_pcap_w) && (G_sysConfigData.total_socket_pcap_w != 0xFFFF) )
        {
           if(l_supported)
           {
              CMDH_TRAC_IMP("Total Socket Power Cap enabled [%d W] low[%d W] drop[%d mhz] raise[%d mhz]",
                             G_sysConfigData.total_socket_pcap_w,
                             G_sysConfigData.total_socket_low_w,
                             G_sysConfigData.delta_chip_mhz_per_watt_drop,
                             G_sysConfigData.delta_chip_mhz_per_watt_raise);
           }
           else
              l_log_no_support_error = TRUE;
        }
        else
        {
           CMDH_TRAC_IMP("Total Socket Power Cap 0x%04X disabled! last clip %d Mhz",
                          G_sysConfigData.total_socket_pcap_w,
                          g_amec->proc[0].pwr_votes.socket_total_pcap_clip_freq);

           g_amec->proc[0].pwr_votes.socket_total_pcap_clip_freq = 0xFFFF;
        }

        if(l_log_no_support_error)
        {
           // Disable and make sure clips are unbounded
           G_sysConfigData.vdd_socket_pcap_w = 0xFFFF;
           G_sysConfigData.total_socket_pcap_w = 0xFFFF;
           g_amec->proc[0].pwr_votes.socket_vdd_pcap_clip_freq = 0xFFFF;
           g_amec->proc[0].pwr_votes.socket_total_pcap_clip_freq = 0xFFFF;

           // Power cap data comes from MRW, send back invalid config data error
           CMDH_TRAC_ERR("data_store_socket_pwr_config: Socket Power Capping not supported! APSS FIDs: ch2[0x%02X] ch3[0x%02X] ch4[0x%02X] ch5[0x%02X]",
                G_apss_ch_to_function[2],
                G_apss_ch_to_function[3],
                G_apss_ch_to_function[4],
                G_apss_ch_to_function[5]);

           /* @
            * @errortype
            * @moduleid    DATA_STORE_SOCKET_DATA
            * @reasoncode  INVALID_CONFIG_DATA
            * @userdata1   Vdd Pcap
            * @userdata2   total socket pcap
            * @userdata4   OCC_NO_EXTENDED_RC
            * @devdesc     Socket Power cappig not supported
            */
           l_err = createErrl(DATA_STORE_SOCKET_DATA,
                              INVALID_CONFIG_DATA,
                              OCC_NO_EXTENDED_RC,
                              ERRL_SEV_UNRECOVERABLE,
                              NULL,
                              DEFAULT_TRACE_SIZE,
                              (uint32_t)l_cmd_ptr->socket_pwr_config.vdd_socket_pcap_w,
                              (uint32_t)l_cmd_ptr->socket_pwr_config.total_socket_pcap_w);

           // Callout firmware
           addCalloutToErrl(l_err,
                            ERRL_CALLOUT_TYPE_COMPONENT_ID,
                            ERRL_COMPONENT_ID_FIRMWARE,
                            ERRL_CALLOUT_PRIORITY_HIGH);
        }
        else if(l_supported)
        {
           CMDH_TRAC_IMP("data_store_socket_pwr_config: Alg wait times freq decrease[%d ticks] freq increase[%d ticks] Proportional[%d]",
                          G_sysConfigData.num_ticks_drop_wait,
                          G_sysConfigData.num_ticks_raise_wait,
                          G_sysConfigData.socket_pcap_proportional_control);
        }
    }

    return l_err;

} // end data_store_socket_pwr_config()


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

        case DATA_FORMAT_AVSBUS_CONFIG:
            // Store AVSBUS settings so that OCC can retrieve the
            // voltage/current and initialize the AVS Bus for reading
            l_errlHndl = data_store_avsbus_config(i_cmd_ptr, o_rsp_ptr);
            if(NULL == l_errlHndl)
            {
                // Notify AMEC of the new data
                l_new_data = DATA_MASK_AVSBUS_CONFIG;
            }
            break;

        case DATA_FORMAT_GPU:
            // Store GPU information
            l_errlHndl = data_store_gpu(i_cmd_ptr, o_rsp_ptr);
            if(NULL == l_errlHndl)
            {
                // Notify AMEC of the new data
                l_new_data = DATA_MASK_GPU;
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
            // Store memory configuration for present membufs and/or dimms to monitor
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

        case DATA_FORMAT_SOCKET_PCAP:
            // Store the socket power cap data in G_sysConfigData
            l_errlHndl = data_store_socket_pwr_config(i_cmd_ptr, o_rsp_ptr);

            if(NULL == l_errlHndl)
            {
                // Set this in case AMEC needs to know about this
                l_new_data = DATA_MASK_SOCKET_PCAP;
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
        l_errlHndl = AMEC_data_change(l_new_data);
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

