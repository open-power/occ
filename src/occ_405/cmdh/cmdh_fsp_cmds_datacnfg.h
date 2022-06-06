/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/cmdh/cmdh_fsp_cmds_datacnfg.h $                   */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2020                        */
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

#ifndef _CMDH_FSP_CMDS_DATACNFG_H
#define _CMDH_FSP_CMDS_DATACNFG_H

#include "ssx.h"
#include "cmdh_service_codes.h"
#include "errl.h"
#include "trac.h"
#include "rtls.h"
#include "occ_common.h"
#include "state.h"
#include "cmdh_fsp.h"
#include "cmdh_fsp_cmds.h"
#include "apss.h"
#include "occ_sys_config.h"

// Enum of the various CnfgData command formats that
// are sent to OCC over the TMGT<->OCC interface.
typedef enum
{
   DATA_FORMAT_SET_ROLE              = 0x03,
   DATA_FORMAT_APSS_CONFIG           = 0x04,
   DATA_FORMAT_MEM_CFG               = 0x05,
   DATA_FORMAT_POWER_CAP             = 0x07,
   DATA_FORMAT_SYS_CNFG              = 0x0f,
   DATA_FORMAT_IPS_CNFG              = 0x11,
   DATA_FORMAT_MEM_THROT             = 0x12,
   DATA_FORMAT_THRM_THRESHOLDS       = 0x13,
   DATA_FORMAT_AVSBUS_CONFIG         = 0x14,
   DATA_FORMAT_GPU                   = 0x15,
   DATA_FORMAT_SOCKET_PCAP           = 0x17,
} eConfigDataFormatVersion;

// Enum of the various Cnfg Data Masks that are used
// to signal that OCC has received cnfg data
typedef enum
{
   DATA_MASK_SET_ROLE              = 0x00000004,
   DATA_MASK_APSS_CONFIG           = 0x00000008,
   DATA_MASK_PCAP_PRESENT          = 0x00000010,
   DATA_MASK_SYS_CNFG              = 0x00000020,
   DATA_MASK_AVSBUS_CONFIG         = 0x00000040,
   DATA_MASK_THRM_THRESHOLDS       = 0x00000080,
   DATA_MASK_IPS_CNFG              = 0x00000100,
   DATA_MASK_MEM_CFG               = 0x00000200,
   DATA_MASK_MEM_THROT             = 0x00000400,
   DATA_MASK_GPU                   = 0x00000800,
   DATA_MASK_SOCKET_PCAP           = 0x00001000,
} eConfigDataPriorityMask;

typedef enum
{
    DATA_FRU_PROC               = 0x00,  // Processor (hottest core temperature)
    DATA_FRU_MEMBUF             = 0x01,  // Internal memory controller sensor
    DATA_FRU_DIMM               = 0x02,
    DATA_FRU_MEMCTRL_DRAM       = 0x03,  // Memory controller + DIMM
    DATA_FRU_GPU                = 0x04,
    DATA_FRU_GPU_MEM            = 0x05,
    DATA_FRU_VRM_VDD            = 0x06,  // This is an actual temperature reading for VRM Vdd
    DATA_FRU_PMIC               = 0x07,
    DATA_FRU_MEMCTRL_EXT        = 0x08,  // External memory controller sensor
    DATA_FRU_PROC_IO            = 0x09,  // Processor IO
    DATA_FRU_MAX                = 0x0A,
// special FRU types not counted in the MAX
    DATA_FRU_PROC_DELTAS        = 0xF0,  // DVFS/ERROR fields are deltas applied to VPD value for DATA_FRU_PROC
    DATA_FRU_PROC_IO_DELTAS     = 0xF9,  // DVFS/ERROR fields are deltas applied to VPD value for DATA_FRU_PROC_IO
} eConfigDataFruType;

// For OCM the mapping of sensors to fru type comes in mem config data from (H)TMGT and some may not be used
// OCC should ignore readings from all sesnors marked as not used
#define DATA_FRU_NOT_USED       0xFF

typedef enum
{
    PWR_READING_TYPE_APSS       = 0x00,
    PWR_READING_TYPE_2_CHANNEL  = 0x02,
    PWR_READING_TYPE_NONE       = 0xFF,
} PWR_READING_TYPE;

extern PWR_READING_TYPE  G_pwr_reading_type;

// Set OCC Role Masks
#define OCC_ROLE_MASTER_MASK        0x01

// Bit mask in poll for indicating if OCC thinks
// it is running in simulation.
#define OCC_SIMICS_ENVIRONMENT      0x08

// Bit mask in poll for indicating OCC owns PMCR
#define OCC_PMCR_OWNER_POLL_STATUS_MASK 0x10

// Used by TMGT to send OCC the frequencies for each mode.
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t format;
    uint8_t version;
}cmdh_store_mode_freqs_t;

// Used by TMGT to tell OCC if it is a master or not
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t format;
    uint8_t role;
    uint8_t reserved[2];
}cmdh_set_role_t;

// Used by TMGT to send OCC the APSS ADC Config Data
typedef struct  __attribute__ ((packed))
{
  uint8_t  assignment;
  uint32_t ipmisensorId;
  uint8_t  gnd_select;
  uint32_t gain;
  uint32_t offset;
} apss_cfg_adc_v20_t;  //New for P9

// Used by TMGT to send OCC the GPIO Config Data
typedef struct  __attribute__ ((packed))
{
  uint8_t  mode;
  uint8_t  __reserved;
  uint8_t  assignment[8];
} apss_cfg_gpio_t;

// Used by TMGT to send OCC the full APSS config data.
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t              format;
    uint8_t              version;
    uint8_t              type;
    uint8_t              reserved;
    apss_cfg_adc_v20_t   adc[MAX_APSS_ADC_CHANNELS];
    apss_cfg_gpio_t      gpio[MAX_APSS_GPIO_PORTS];
}cmdh_apss_config_v20_t; //New for P9

// Used by TMGT to send OCC the AVS Bus config data.
// Version 0x30 AVS Bus config data.
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t  format;
    uint8_t  version;
    uint8_t  vdd_bus;
    uint8_t  vdd_rail;
    uint8_t  reserved1[4];
}cmdh_avsbus_v30_config_t;

// Used by TMGT to send OCC GPU data.
// Header data for GPU version 2 cfg packet
typedef struct __attribute__ ((packed))
{
    struct   cmdh_fsp_cmd_header;
    uint8_t  format;
    uint8_t  version;
    uint16_t total_non_gpu_max_pwr_watts;
    uint16_t total_proc_mem_pwr_drop_watts;
    uint8_t  total_num_gpus_system;
    uint8_t  gpu_i2c_engine;
    uint8_t  gpu_i2c_bus_voltage;
    uint8_t  num_data_sets;
}cmdh_gpu_cfg_header_v2_t;

typedef struct __attribute__ ((packed))
{
    uint8_t  gpu_num;          // number 0...2 for GPU data is for
    uint8_t  i2c_port;         // I2C port for this GPU
    uint8_t  i2c_addr;         // I2C slave address for this GPU
    uint8_t  reserved;
    uint32_t gpu_temp_sid;     // GPU Core Temperature Sensor ID
    uint32_t gpu_mem_temp_sid; // GPU Memory Temperature Sensor ID
    uint32_t gpu_sid;          // GPU Sensor ID for callout
}cmdh_gpu_set_v2_t;

typedef struct __attribute__ ((packed))
{
    cmdh_gpu_cfg_header_v2_t  header;
    cmdh_gpu_set_v2_t         gpu_data[MAX_NUM_GPU_PER_DOMAIN];
}cmdh_gpu_config_v2_t;

typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t  format;
    uint8_t  version;
    uint16_t total_non_gpu_max_pwr_watts;
    uint16_t total_proc_mem_pwr_drop_watts;
    uint16_t reserved;
    uint32_t gpu0_temp_sid;     // GPU0 Temperature Sensor ID
    uint32_t gpu0_mem_temp_sid; // GPU0 Memory Temperature Sensor ID
    uint32_t gpu0_sid;          // GPU0 Sensor ID for callout
    uint32_t gpu1_temp_sid;     // GPU1 Temperature Sensor ID
    uint32_t gpu1_mem_temp_sid; // GPU1 Memory Temperature Sensor ID
    uint32_t gpu1_sid;          // GPU1 Sensor ID for callout
    uint32_t gpu2_temp_sid;     // GPU2 Temperature Sensor ID
    uint32_t gpu2_mem_temp_sid; // GPU2 Memory Temperature Sensor ID
    uint32_t gpu2_sid;          // GPU2 Sensor ID for callout
}cmdh_gpu_config_t;

// Used by TMGT to send OCC the PCAP config data.
typedef struct __attribute__ ((packed))
{
    uint16_t soft_min_pcap; // Min node power cap allowed in 1W units (1WU)
    uint16_t hard_min_pcap; // Min guaranteed node power cap in 1WU
    uint16_t sys_max_pcap;  // Max system (max user settable) power cap in 1WU
    uint16_t qpd_pcap;      // Quick Power Drop (Oversubscription) power cap in 1WU
} cmdh_pcap_config_data_t;

// Used by TMGT to send OCC the PCAP config data.
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t              format;
    uint8_t              version;
    cmdh_pcap_config_data_t   pcap_config;
}cmdh_pcap_config_t;

// Used by TMGT to send OCC the Socket Power config data.
typedef struct __attribute__ ((packed))
{
    uint16_t vdd_socket_pcap_w; // power cap for socket Vdd power in 1W for Vdd socket power to be above before freq is lowered
    uint16_t vdd_socket_low_w;  // Lower threshold in 1W for Vdd socket power to fall below before frequency is raised
    uint16_t total_socket_pcap_w; // power cap for total socket power in 1W for total socket power to be above before freq is lowered
    uint16_t total_socket_low_w;  // Lower threshold in 1W for total socket power to fall below before frequency is raised
    uint16_t delta_chip_mhz_per_watt_drop;  // Number of MHz to decrease freq for every watt over pcap
    uint16_t delta_chip_mhz_per_watt_raise; // Number of MHz to increase freq for every watt under pcap
    uint16_t num_ticks_drop_wait; // number of ticks to wait before decreasing freq again when over pcap to allow power reading
    uint16_t num_ticks_raise_wait; // number of ticks to wait before increasing freq again when under pcap to allow power reading
    uint8_t  proportional_control; // 0=non-proportional control, raise/drop will be 1W step size.  Non-zero proportional control
} cmdh_socket_pwr_config_data_t;

typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t              format;
    uint8_t              version;
    cmdh_socket_pwr_config_data_t   socket_pwr_config;
}cmdh_socket_pwr_config_t;

typedef struct __attribute__ ((packed))
{
    uint8_t     system_type;                // General system type
    uint32_t    proc_sid;                   // Processor Sensor ID
    uint32_t    proc_freq_sid;              // Processor Frequency Sensor ID
    uint32_t    core_sid[MAX_CORES];        // 32 cores. 4 bytes for each temp sensor ID
    uint32_t    backplane_sid;              // Backplane Sensor ID
    uint32_t    apss_sid;                   // APSS Sensor ID
    uint32_t    vrm_vdd_sid;                // VRM Vdd Sensor ID for hw callout
    uint32_t    vrm_vdd_temp_sid;           // VRM Vdd Temperature sensor ID
    uint32_t    _reserved[2];
} cmdh_sys_config_data_v30_t;

typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t                     format;
    uint8_t                     version;
    cmdh_sys_config_data_v30_t  sys_config;
}cmdh_sys_config_v30_t;

// Used by TMGT to send OCC the IPS config data.
typedef struct __attribute__ ((packed))
{
    uint8_t     iv_ipsEnabled;          // Idle Power Save Enabled (0 or 1)
    uint16_t    iv_delayTimeforEntry;   // Delay Time in seconds to enter Idle Power
    uint8_t     iv_utilizationForEntry; // Utilization threshold percentage to enter Idle Power
    uint16_t    iv_delayTimeforExit;    // Delay Time in seconds to exit Idle Power
    uint8_t     iv_utilizationForExit;  // Utilization threshold percentage to exit Idle Power

} cmdh_ips_config_data_t;

// Used by TMGT to send OCC the IPS config data.
typedef struct __attribute__ ((packed))
{
    struct                  cmdh_fsp_cmd_header;
    uint8_t                 iv_format;
    uint8_t                 iv_version;
    cmdh_ips_config_data_t  iv_ips_config;
}cmdh_ips_config_t;

extern cmdh_ips_config_data_t   G_ips_config_data;

typedef struct __attribute__ ((packed))
{
    uint8_t              fru_type;  // eConfigDataFruType

    //  dvfs and error fields define the temperature trigger
    //  points for DVFS/Throttling and error/FRU callouts, respectively.
    //  a 0xFF entry indicates that the coresponding temperature trigger
    //  point is undefined.
    int8_t              dvfs;              // OS Controlled sys/PowerVM nominal
    int8_t              error;             // OS Controlled sys/PowerVM nominal

    // Max time (s) allowed without having new temp readings before throttling
    uint8_t              max_read_timeout;

    uint8_t              _reserved[2];
}cmdh_thrm_thresholds_set_v30_t;

typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t              format;
    uint8_t              version;
    uint8_t              proc_core_weight;
    uint8_t              proc_racetrack_weight;
    uint8_t              proc_L3_weight;
    uint8_t              num_data_sets;
    cmdh_thrm_thresholds_set_v30_t data[DATA_FRU_MAX + 2]; // allow for extra delta thresholds
}cmdh_thrm_thresholds_v30_t;

// Header data for mem cfg packet
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t                 format;
    uint8_t                 version;
    uint16_t                update_time_ms;       // time in ms that the cache line is updated
    uint8_t                 default_mem_pwr_ctl;  // default memory power control
    uint8_t                 ips_mem_pwr_ctl;      // Idle Power Save memory power control
    uint8_t                 num_data_sets;
}cmdh_mem_cfg_header_v30_t;

// Config packet definition used to send
// sensor mappings for membufs and dimms

typedef struct __attribute__ ((packed))
{
    uint32_t                   hw_sensor_id;
    uint32_t                   temp_sensor_id;
    uint8_t                    memory_type;
    uint8_t                    dimm_info1;  // Mem DTS 0 or 1
    uint8_t                    dimm_info2;  // Temperature type
    uint8_t                    dimm_info3;  // Reserved
}cmdh_mem_cfg_data_set_t;

typedef struct __attribute__ ((packed))
{
    cmdh_mem_cfg_header_v30_t   header;
    cmdh_mem_cfg_data_set_t data_set[1];
}cmdh_mem_cfg_v30_t;

// Header data for mem throttle packet
typedef struct __attribute__ ((packed))
{
    struct cmdh_fsp_cmd_header;
    uint8_t                    format;
    uint8_t                    version;
    uint8_t                    num_data_sets;
}cmdh_mem_throt_header_t;


// Provides memory throttle min and max values
typedef struct __attribute__ ((packed))
{
    uint8_t   membuf_num;            // Physical memory buffer#
    uint8_t   _reserved;
} cmdh_mem_throt_info_t;

typedef struct __attribute__ ((packed))
{
    cmdh_mem_throt_info_t mem_throt_info; // membuf information header

    uint16_t    min_n_per_mba;            // Lowest per MBA allowed numerator

    uint16_t    mode_disabled_n_per_mba;  // Static per MBA numerator when modes are disabled
    uint16_t    mode_disabled_n_per_chip; // Static per chip numerator when modes are disabled

    uint16_t    ut_n_per_mba;             // Static per MBA numerator for ultra turbo modes
    uint16_t    ut_n_per_chip;            // Static per chip numerator for ultra turbo modes

    uint16_t    fmax_n_per_mba;           // Static per MBA N for fmax mode
    uint16_t    fmax_n_per_chip;          // Static per chip N for fmax mode

    uint16_t    oversub_n_per_mba;        // Static per MBA N when in oversubscription
    uint16_t    oversub_n_per_chip;       // Static per chip N when in oversubscription

    uint16_t    reserved1;                // reserved
} cmdh_mem_throt_data_set_v40_t;


// Config packet definition used by TMGT to
// send mem throttle min/max settings.
typedef struct __attribute__ ((packed))
{
    cmdh_mem_throt_header_t          header;
    cmdh_mem_throt_data_set_v40_t    data_set[1];
} cmdh_mem_throt_t;

// Used to mark present the config data TMGT has sent us.
typedef struct data_cnfg
{
    uint32_t                  data_mask;
    cmdh_thrm_thresholds_v30_t    thrm_thresh;
} data_cnfg_t;

errlHndl_t DATA_store_cnfgdata (const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * i_rsp_ptr);

uint8_t DATA_request_cnfgdata ();

uint32_t DATA_get_present_cnfgdata ();

errlHndl_t DATA_get_thrm_thresholds(cmdh_thrm_thresholds_v30_t **o_thrm_thresh);

errlHndl_t DATA_get_ips_cnfg(cmdh_ips_config_data_t **o_ips_cnfg);

#endif

