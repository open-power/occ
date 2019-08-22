/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/occ_sys_config.h $                                */
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

#ifndef _occ_sys_config_h
#define _occ_sys_config_h

// SSX has Pstate defined in pgp_common.h file. Without below ifdef
// compiler complains about redefinition for OCC.
#ifdef OCC_FIRMWARE
#include "occhw_common.h"
#else
#endif
#include <state.h>
#include <apss.h>
#include <mem_structs.h>
#include <p10_hcd_memmap_base.H>

#define MAX_NUM_OCC              4
#define MAX_NUM_NODES            4
#define MAX_NUM_CORES           24
#define NUM_CORES_PER_QUAD       4
#define MAX_NUM_CHIP_MODULES     4
#define MAX_NUM_MEM_CONTROLLERS  16
#define MAX_NUM_OCMBS            16
#define MAX_NUM_CHOM_MEM_CTRL    8  // todo allow 16 RTC: 211908
#define NUM_PROC_VRMS            2
#define MAX_GPU_PRES_SIGNALS     6
#define GPU_PRES_SIGN_PER_OCC    3

#define MAX_NUM_MCU_PORTS        4

#define NUM_DIMMS_PER_MEM_CONTROLLER    8

#define NUM_PROC_CHIPS_PER_OCC          1
#define NUM_PORT_PAIRS_PER_MEM_BUF      2

#define NUM_MBAS_PER_OCMB               1
#define NUM_DIMMS_PER_OCMB              2
#define OCMB_TYPE_LOCATION_MASK (MAX_NUM_OCMBS-1)
#define IS_OCM_MEM_TYPE(type) (((type) & (~OCMB_TYPE_LOCATION_MASK)) == MEM_TYPE_OCM)

#define UPPER_LIMIT_PROC_FREQ_MHZ     6000

// System Structures
typedef union
{
    struct
    {
        uint8_t     kvm:              1;
        uint8_t     reserved:         3;
        uint8_t     report_dvfs_nom:  1;
        uint8_t     reserved_2:       1;
        uint8_t     non_redund_ps:    1;
        uint8_t     single:           1;
    };
    uint8_t byte;
} eSystemType;

// APSS Structures & Defines
#define SYSCFG_INVALID_ADC_CHAN      0xff
#define SYSCFG_INVALID_PIN           0xff

#define MAX_VRFAN_SIGNALS       4
#define MAX_ADC_IO_DOMAINS      3
#define MAX_ADC_FAN_DOMAINS     2
#define MAX_ADC_STORAGE_DOMAINS 2
#define MAX_CENT_EN_VCACHE      4
#define MAX_DOM_OC_LATCH        4
#define MAX_PROC_MEMBUF_CH      4
#define MAX_GPU_DOMAINS         2
#define MAX_NUM_GPU_PER_DOMAIN  3

//Returns non-0 if the specified GPU behind this OCC (not system wide) is present. Otherwise, returns zero.
// NOTE: This is looking at the first GPU config, GPU config should not change after first determined, if
// current GPU config is wanted to compare to the first check G_curr_proc_gpu_config
extern uint32_t G_first_proc_gpu_config;
#define GPU_PRESENT(occ_gpu_id) \
         ((0x01 << occ_gpu_id) & G_first_proc_gpu_config)


// List of all possible APSS Channel assignments (Function IDs)
// Each channel in the APSS will be associated with only one of these
// function ids for each system type as defined in the mrw.
// If a channel is assigned a 0x00, that means it's not assigned.
typedef enum
{
    ADC_RESERVED                = 0x00,
    ADC_MEMORY_PROC_0           = 0x01,
    ADC_MEMORY_PROC_1           = 0x02,
    ADC_MEMORY_PROC_2           = 0x03,
    ADC_MEMORY_PROC_3           = 0x04,
    ADC_VDD_PROC_0              = 0x05,
    ADC_VDD_PROC_1              = 0x06,
    ADC_VDD_PROC_2              = 0x07,
    ADC_VDD_PROC_3              = 0x08,
    ADC_VCS_VIO_VPCIE_PROC_0    = 0x09,
    ADC_VCS_VIO_VPCIE_PROC_1    = 0x0A,
    ADC_VCS_VIO_VPCIE_PROC_2    = 0x0B,
    ADC_VCS_VIO_VPCIE_PROC_3    = 0x0C,
    ADC_IO_A                    = 0x0D,
    ADC_IO_B                    = 0x0E,
    ADC_IO_C                    = 0x0F,
    ADC_FANS_A                  = 0x10,
    ADC_FANS_B                  = 0x11,
    ADC_STORAGE_A               = 0x12,
    ADC_STORAGE_B               = 0x13,
    ADC_12V_SENSE               = 0x14,
    ADC_GND_REMOTE_SENSE        = 0x15,
    ADC_TOTAL_SYS_CURRENT       = 0x16,
    ADC_MEM_CACHE               = 0x17,
    ADC_GPU_0_0                 = 0x18,  // ADC_GPU_x_y x=Proc, y=GPU (max of 3 GPUs/proc)
    ADC_MEMORY_PROC_0_0         = 0x19,
    ADC_MEMORY_PROC_0_1         = 0x1A,
    ADC_MEMORY_PROC_0_2         = 0x1B,
    ADC_12V_STANDBY_CURRENT     = 0x1C,
    ADC_GPU_0_1                 = 0x1D,
    ADC_GPU_0_2                 = 0x1E,
    ADC_GPU_1_0                 = 0x1F,
    ADC_GPU_1_1                 = 0x20,
    ADC_GPU_1_2                 = 0x21,
    ADC_GPU_VOLT2_0_0           = 0x22,
    ADC_GPU_VOLT2_0_1           = 0x23,
    ADC_GPU_VOLT2_1_0           = 0x24,
    ADC_GPU_VOLT2_1_1           = 0x25,
    ADC_VOLT_SENSE_2            = 0x26,
    ADC_TOTAL_SYS_CURRENT_2     = 0x27,
    NUM_ADC_ASSIGNMENT_TYPES    // This should always be the last member
} eApssAdcChannelAssignments;

typedef enum
{
    GPIO_RESERVED               = 0x0000,
    GPIO_FAN_WATCHDOG_ERROR     = 0x0001,
    GPIO_FAN_FULL_SPEED         = 0x0002,
    GPIO_FAN_ERROR              = 0x0003,
    GPIO_FAN_RESERVED           = 0x0004,
    GPIO_VR_HOT_MEM_PROC_0      = 0x0005,
    GPIO_VR_HOT_MEM_PROC_1      = 0x0006,
    GPIO_VR_HOT_MEM_PROC_2      = 0x0007,
    GPIO_VR_HOT_MEM_PROC_3      = 0x0008,
    GPIO_CENT_EN_VCACHE0        = 0x0009,
    GPIO_CENT_EN_VCACHE1        = 0x000A,
    GPIO_CENT_EN_VCACHE2        = 0x000B,
    GPIO_CENT_EN_VCACHE3        = 0x000C,
    CME_THROTTLE_N              = 0x000D,
    GND_OC_N                    = 0x000E,
    DOM_A_OC_LATCH              = 0x000F,
    DOM_B_OC_LATCH              = 0x0010,
    DOM_C_OC_LATCH              = 0x0011,
    DOM_D_OC_LATCH              = 0x0012,
    PSU_FAN_DISABLE_N           = 0x0013,
    GPU_0_0_PRSNT_N             = 0x0014, //GPU0_PRSNT_N
    GPU_0_1_PRSNT_N             = 0x0015, //GPU1_PRSNT_N
    GPU_0_2_PRSNT_N             = 0x0016, //GPU2_PRSNT_N
    GPU_1_0_PRSNT_N             = 0x0017, //GPU3_PRSNT_N
    GPU_1_1_PRSNT_N             = 0x0018, //GPU4_PRSNT_N
    GPU_1_2_PRSNT_N             = 0x0019, //GPU5_PRSNT_N
    NVDIMM_EPOW_N               = 0x001A,
    NUM_GPIO_ASSIGNMENT_TYPES    // This should always be the last member
} eApssGpioAssignments;

typedef struct
{
  // Gain and Offset, Stored in a exponent/mantissa format (like P7 PS Gain/Offset)
  uint32_t gain;        // Think "multiplier"
  uint32_t offset;      // Will probably always be 0, since APSS is so accurate
  uint8_t  gnd_select;
} apssCalibrationData_t;

typedef struct
{
  // Value stored will be APSS GPIO Number, if not present, set to INVALID = xFF.
  // Example:  vr_fan[0] = 4; (VR_FAN is APSS GPIO #4)
  uint8_t dom_oc_latch[MAX_DOM_OC_LATCH];
  uint8_t gnd_oc_n;
  uint8_t cme_throttle_n;
  uint8_t cent_en_vcache[MAX_CENT_EN_VCACHE];
  uint8_t vr_fan[MAX_VRFAN_SIGNALS];
  uint8_t fans_watchdog_error;
  uint8_t fans_full_speed;
  uint8_t fans_error;
  uint8_t fans_reserved;
  uint8_t psu_fan_disable;
  uint8_t gpu[MAX_GPU_PRES_SIGNALS];
  uint8_t nvdimm_epow;
} apssGpioPinData_t;

typedef struct
{
  // Value stored will be APSS ADC Channel Number, if rail is not present, set
  // to INVALID = xFF
  uint8_t memory[MAX_NUM_CHIP_MODULES][MAX_PROC_MEMBUF_CH];
  uint8_t vdd[MAX_NUM_CHIP_MODULES];
  uint8_t io[MAX_ADC_IO_DOMAINS];
  uint8_t fans[MAX_ADC_FAN_DOMAINS];
  uint8_t storage_media[MAX_ADC_STORAGE_DOMAINS];
  uint8_t total_current_12v;
  uint8_t vcs_vio_vpcie[MAX_NUM_CHIP_MODULES];
  uint8_t sense_12v;
  uint8_t remote_gnd;
  uint8_t mem_cache;
  uint8_t current_12v_stby;
  uint8_t gpu[MAX_GPU_DOMAINS][MAX_NUM_GPU_PER_DOMAIN];
  uint8_t sense_volt2;
  uint8_t total_current_volt2;
} apssAdcChannelData_t;

typedef struct
{
    uint8_t bus;
    uint8_t rail;
} avsbusData_t;

// Master/Slave Configuration
typedef struct
{
  // Bitmask that states if OCC is master capable
  // (i.e. 0x11 = Chip_Id 000 & Chip_Id 100 can both be masters)
  uint8_t is_master_capable;

  // Which Chip_Id should be initial master
  // (i.e. default_master = 0x00 for almost all cases)
  uint8_t default_master;

} masterCapability_t;


// System Frequency
typedef struct
{
    uint16_t table[OCC_MODE_COUNT];  // Table w/ freq for each mode
    uint8_t  update_count;           //
    uint8_t  _reserved;              // Align to 2 b/c we may use it in PBAX broadcast
} freqConfig_t;

// Power Cap Structures
typedef struct
{
    uint16_t current_pcap;     // Node power cap requested by customer (AEM) in 1W units
    uint16_t soft_min_pcap;    // Minimum node power cap allowed in 1W units
    uint16_t hard_min_pcap;    // Minimum guaranteed node power cap in 1W units
    uint16_t max_pcap;         // Maximum customer settable node power cap in 1W units
    uint16_t oversub_pcap;     // Node power cap to be used for oversubscripion in 1W units
    uint16_t system_pcap;      // Fixed node power cap required by the system in 1W units
    uint8_t  pcap_data_count;  // Used by OCC only.  Initialized to 0 and incremented by 1 with every new packet.
    uint8_t  source;           // source of PCAP value currently in use
} pcap_config_data_t;


// source of PCAP value
#define OUT_OF_BAND         1  // source is BMC or (H)TMGT
#define IN_BAND             2  // source is OPAL


// Memory Throttle settings
typedef struct
{
    uint16_t    min_n_per_mba;           //minimum value
    uint16_t    min_mem_power;           //Max mem Power @min (x0.01W)

    uint16_t    turbo_n_per_mba;         //max mba value for Turbo
    uint16_t    turbo_n_per_chip;        //Static per chip numerator @Turbo
    uint16_t    turbo_mem_power;         //max memory power @Turbo

    uint16_t    pcap_n_per_mba;         //max mba value for Power Cap
    uint16_t    pcap_n_per_chip;        //Static per chip numerator @PCAP
    uint16_t    pcap_mem_power;         //max memory power @PCAP

    uint16_t    nom_n_per_mba;           //max mba value for nominal mode
    uint16_t    nom_n_per_chip;          //chip setting for nominal mode
    uint16_t    nom_mem_power;           //max memory power @nominal

    uint16_t    reserved1;               //reserved
    uint16_t    reserved2;               //reserved
    uint16_t    reserved3;               //reserved
} mem_throt_config_data_t;

// Per GPU I2C Info
typedef struct
{
    uint8_t port;
    uint8_t address;
} gpuI2CInfo_t;

// Sys Config Structure

// Needs to be 128 bytes aligned so we can do a DMA to transfer this to SRAM from
// Main Memory.

// One global structure which is the same in all OCCs

// TODO: If we cannot dynamically change this structure and have it
// picked up by host boot & occ, then we will want to add reserved
// fields for each 'section' of the structure for future-proofing

typedef struct
{
  // ------------------------------------
  // Identification & Debug Flags
  // ------------------------------------

  // Gives us a version number of this structure for ease of debug
  uint8_t version;

  // Gives us a lot of debug flags that we can use to change OCC behavior
  // at startup.
  uint8_t debug_reserved[7];

  // ------------------------------------
  // System Configuration
  // ------------------------------------

  // Instead of system-type, lets try to send all system attributes
  // that matter instead of having tables in OCC code.
  eSystemType system_type;

  // Processor HUID - HUID for this OCC processor, used by OCC for processor error call out
  uint32_t proc_huid;

  // Backplane HUID - Used by OCC for system backplane error call out (i.e. VRM errors will call out backplane)
  uint32_t backplane_huid;

  // APSS HUID - Used by OCC for APSS error call out
  uint32_t apss_huid;

  // VRM Vdd HUID - Used by OCC for VRM Vdd error call out
  uint32_t vrm_vdd_huid;

  // Contains how many OCCs & how many proc modules are present.
  uint8_t sys_num_proc_present;

  // ------------------------------------
  // Max Frequency for each mode
  // ------------------------------------
  freqConfig_t sys_mode_freq;

  // ------------------------------------
  // APSS Configuration Data
  // ------------------------------------

  // Holds the gain/offset needed to translate from raw ADC to actual power
  // measurements
  apssCalibrationData_t apss_cal[MAX_APSS_ADC_CHANNELS];

  // Holds the mapping of GPIO signals to APSS pins
  apssGpioPinData_t apss_gpio_map;

  // Holds the mapping of ADC signals to APSS adc channels
  apssAdcChannelData_t apss_adc_map;

  // GPIO Port Mode
  uint8_t apssGpioPortsMode[MAX_APSS_GPIO_PORTS];

  // AVS Bus config
  avsbusData_t avsbus_vdd;
  avsbusData_t avsbus_vdn;
  uint16_t     proc_power_adder;
  uint32_t     vdd_current_rollover_10mA;
  uint32_t     vdd_max_current_10mA;

  // ------------------------------------
  // Power Cap Configuration Data updated by Slaves
  // ------------------------------------
  pcap_config_data_t pcap;

  // ------------------------------------
  // Master/Slave OCC Configuration Data
  // ------------------------------------

  // Information Needed to translate from PowerBus ID to PBAX Id (OCC Id)
  //   - Index = Powerbus Id will get you PBAX Id as value at that index
  //   - Example pob2pbax_chip[1] = 4; (PowerBus ID 001 = PBAX ID 100
  uint8_t pob2pbax_chip[MAX_NUM_OCC];
  uint8_t pob2pbax_node[MAX_NUM_NODES];

  // Bitmask that states if OCC is present
  // (i.e. 0x11 = Chip_Id 000 & Chip_Id 100 are only OCCs present)
  uint8_t is_occ_present;

  // Tells us which OCCs can be master & which OCC should start out as master
  masterCapability_t master_config;

  // ------------------------------------
  // Oversubscription Configuration Data
  // ------------------------------------

  // ppb_fmax as set by Master OCC.  Initialized to 0xFFFF
  uint32_t master_ppb_fmax;

  // --------------------------------------
  // Memory Configuration Data
  // --------------------------------------
  uint32_t membuf_huids[MAX_NUM_OCMBS];
  uint32_t dimm_huids[MAX_NUM_OCMBS][NUM_DIMMS_PER_OCMB];
  uint8_t mem_type;
  uint8_t ips_mem_pwr_ctl;     // IPS memory power control
  uint8_t default_mem_pwr_ctl; // default memory power control

  // --------------------------------------
  // Memory Throttle limits
  // --------------------------------------
  // This array holds throttle configuration parameters for memory

  // Throttle limits are layout:
  //   mem_throt_limits[membuf][mba]
  //           membuf=0-8, mba = 0/1 for mba01/mba23
  //              (only first two columns populated)
  mem_throt_config_data_t mem_throt_limits[MAX_NUM_MEM_CONTROLLERS][MAX_NUM_MCU_PORTS];

  // --------------------------------------
  // GPU Information for error callout and GPU power capping
  // --------------------------------------
  uint32_t gpu_sensor_ids[MAX_NUM_GPU_PER_DOMAIN];
  uint16_t total_non_gpu_max_pwr_watts;
  uint16_t total_proc_mem_pwr_drop_watts;
  uint8_t  psr;                  // power shifting ratio for power capping between GPU/Proc&mem
  uint8_t  gpu_i2c_engine;       // PIB I2CM engine for all GPUs
  uint8_t  gpu_i2c_bus_voltage;  // GPU I2C bus voltage (1 = 0.1V)
  gpuI2CInfo_t gpu_i2c_info[MAX_NUM_GPU_PER_DOMAIN];  // per GPU I2C info (port/address)

} occSysConfigData_t;  __attribute__ ((__aligned__ (128)))

extern occSysConfigData_t G_sysConfigData;


// Individual OCC Config Structure
// Needs to be 128 bytes aligned so we can do a DMA to transfer this to SRAM from
// Main Memory.

// One global structure per OCC (so there will be 8 up in main memory)

// TODO: If we cannot dynamically change this structure and have it
// picked up by host boot & occ, then we will want to add reserved
// fields for each 'section' of the structure for future-proofing

typedef struct
{
  int dummy;

} occModuleConfigData_t;  __attribute__ ((__aligned__ (128)))


// MASTER PCAP values.
extern pcap_config_data_t G_master_pcap_data;


#ifdef FSPLESS_SIMICS
void sysConfigFspLess(void);
#endif

#endif // _occ_sys_config_h
