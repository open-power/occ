/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/occ_sys_config.c $                                */
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

#include <occ_common.h>
#include <common_types.h>
#include <occ_sys_config.h>
#include <dimm.h>

// SysConfig Section Defines
#define SYSCFG_DEFAULT_VERSION       0xff

// APSS Section Defines
#define SYSCFG_ADC_1x_MULT           1000

// Master/Slave Section Defines
#define SYSCFG_MASTER_CAPABLE_000    0x01
#define SYSCFG_DEFAULT_MASTER_000    0x00
#define SYSCFG_ALL_OCCS_PRESENT      0xff
#define SYSCFG_ZERO_OCCS_PRESENT     0x00

// OCC System Configuration Data
//
// We will initialize everything to default values, so in that case that we
// can't read the data from mainstore, we will still be able to *do something*
// instead of crash.
occSysConfigData_t G_sysConfigData =
{
    .version        = SYSCFG_DEFAULT_VERSION,
    .debug_reserved = {0},

    // -----------------------------------------------------------
    // System Configuration Section Initializations
    // -----------------------------------------------------------
    .sys_num_proc_present = 4,  //TODO: placeholder

    // -----------------------------------------------------------
    // System maximum frequencies (in MHz) for each mode
    // -----------------------------------------------------------
    .sys_mode_freq.table = {
        [OCC_MODE_NOMINAL]       3500,
        [OCC_MODE_TURBO]         3700,
        [OCC_MODE_PWRSAVE]       3000,
        [OCC_MODE_MIN_FREQUENCY] 2575,
        [OCC_MODE_FFO]              0,
    },

    // -----------------------------------------------------------
    // APSS Section Initializations
    // -----------------------------------------------------------
    .apss_cal = {
        [0]  {.gain = SYSCFG_ADC_1x_MULT, .offset = 0 , .gnd_select = 0},
        [1]  {.gain = SYSCFG_ADC_1x_MULT, .offset = 0 , .gnd_select = 0},
        [2]  {.gain = SYSCFG_ADC_1x_MULT, .offset = 0 , .gnd_select = 0},
        [3]  {.gain = SYSCFG_ADC_1x_MULT, .offset = 0 , .gnd_select = 0},
        [4]  {.gain = SYSCFG_ADC_1x_MULT, .offset = 0 , .gnd_select = 0},
        [5]  {.gain = SYSCFG_ADC_1x_MULT, .offset = 0 , .gnd_select = 0},
        [6]  {.gain = SYSCFG_ADC_1x_MULT, .offset = 0 , .gnd_select = 0},
        [7]  {.gain = SYSCFG_ADC_1x_MULT, .offset = 0 , .gnd_select = 0},
        [8]  {.gain = SYSCFG_ADC_1x_MULT, .offset = 0 , .gnd_select = 0},
        [9]  {.gain = SYSCFG_ADC_1x_MULT, .offset = 0 , .gnd_select = 0},
        [10] {.gain = SYSCFG_ADC_1x_MULT, .offset = 0 , .gnd_select = 0},
        [11] {.gain = SYSCFG_ADC_1x_MULT, .offset = 0 , .gnd_select = 0},
        [12] {.gain = SYSCFG_ADC_1x_MULT, .offset = 0 , .gnd_select = 0},
        [13] {.gain = SYSCFG_ADC_1x_MULT, .offset = 0 , .gnd_select = 0},
        [14] {.gain = SYSCFG_ADC_1x_MULT, .offset = 0 , .gnd_select = 0},
        [15] {.gain = SYSCFG_ADC_1x_MULT, .offset = 0 , .gnd_select = 0},
    },

    .apss_gpio_map =  {
        .fans_watchdog_error = SYSCFG_INVALID_PIN,
        .fans_full_speed     = SYSCFG_INVALID_PIN,
        .fans_error          = SYSCFG_INVALID_PIN,
        .fans_reserved       = SYSCFG_INVALID_PIN,
        .vr_fan[0]           = SYSCFG_INVALID_PIN,
        .vr_fan[1]           = SYSCFG_INVALID_PIN,
        .vr_fan[2]           = SYSCFG_INVALID_PIN,
        .vr_fan[3]           = SYSCFG_INVALID_PIN,
        .cent_en_vcache[0]   = SYSCFG_INVALID_PIN,
        .cent_en_vcache[1]   = SYSCFG_INVALID_PIN,
        .cent_en_vcache[2]   = SYSCFG_INVALID_PIN,
        .cent_en_vcache[3]   = SYSCFG_INVALID_PIN,
        .cme_throttle_n      = SYSCFG_INVALID_PIN,
        .gnd_oc_n            = SYSCFG_INVALID_PIN,
        .dom_oc_latch[0]     = SYSCFG_INVALID_PIN,
        .dom_oc_latch[1]     = SYSCFG_INVALID_PIN,
        .dom_oc_latch[2]     = SYSCFG_INVALID_PIN,
        .dom_oc_latch[3]     = SYSCFG_INVALID_PIN,
    },

    .apss_adc_map =   {
        .memory[0][0]        = SYSCFG_INVALID_ADC_CHAN,
        .memory[0][1]        = SYSCFG_INVALID_ADC_CHAN,
        .memory[0][2]        = SYSCFG_INVALID_ADC_CHAN,
        .memory[0][3]        = SYSCFG_INVALID_ADC_CHAN,
        .memory[1][0]        = SYSCFG_INVALID_ADC_CHAN,
        .memory[1][1]        = SYSCFG_INVALID_ADC_CHAN,
        .memory[1][2]        = SYSCFG_INVALID_ADC_CHAN,
        .memory[1][3]        = SYSCFG_INVALID_ADC_CHAN,
        .memory[2][0]        = SYSCFG_INVALID_ADC_CHAN,
        .memory[2][1]        = SYSCFG_INVALID_ADC_CHAN,
        .memory[2][2]        = SYSCFG_INVALID_ADC_CHAN,
        .memory[2][3]        = SYSCFG_INVALID_ADC_CHAN,
        .memory[3][0]        = SYSCFG_INVALID_ADC_CHAN,
        .memory[3][1]        = SYSCFG_INVALID_ADC_CHAN,
        .memory[3][2]        = SYSCFG_INVALID_ADC_CHAN,
        .memory[3][3]        = SYSCFG_INVALID_ADC_CHAN,
        .vdd[0]              = SYSCFG_INVALID_ADC_CHAN,
        .vdd[1]              = SYSCFG_INVALID_ADC_CHAN,
        .vdd[2]              = SYSCFG_INVALID_ADC_CHAN,
        .vdd[3]              = SYSCFG_INVALID_ADC_CHAN,
        .io[0]               = SYSCFG_INVALID_ADC_CHAN,
        .io[1]               = SYSCFG_INVALID_ADC_CHAN,
        .io[2]               = SYSCFG_INVALID_ADC_CHAN,
        .fans[0]             = SYSCFG_INVALID_ADC_CHAN,
        .fans[1]             = SYSCFG_INVALID_ADC_CHAN,
        .storage_media[0]    = SYSCFG_INVALID_ADC_CHAN,
        .storage_media[1]    = SYSCFG_INVALID_ADC_CHAN,
        .vcs_vio_vpcie[0]    = SYSCFG_INVALID_ADC_CHAN,
        .vcs_vio_vpcie[1]    = SYSCFG_INVALID_ADC_CHAN,
        .vcs_vio_vpcie[2]    = SYSCFG_INVALID_ADC_CHAN,
        .vcs_vio_vpcie[3]    = SYSCFG_INVALID_ADC_CHAN,
        .total_current_12v   = SYSCFG_INVALID_ADC_CHAN,
        .sense_12v           = SYSCFG_INVALID_ADC_CHAN,
        .remote_gnd          = SYSCFG_INVALID_ADC_CHAN,
        .mem_cache           = SYSCFG_INVALID_ADC_CHAN,
        .current_12v_stby    = SYSCFG_INVALID_ADC_CHAN,
        .gpu[0][0]           = SYSCFG_INVALID_ADC_CHAN,
        .gpu[0][1]           = SYSCFG_INVALID_ADC_CHAN,
        .gpu[0][2]           = SYSCFG_INVALID_ADC_CHAN,
        .gpu[1][0]           = SYSCFG_INVALID_ADC_CHAN,
        .gpu[1][1]           = SYSCFG_INVALID_ADC_CHAN,
        .gpu[1][2]           = SYSCFG_INVALID_ADC_CHAN,
    },

    .apssGpioPortsMode =   {0, 0},

    .avsbus_vdd = {
        .bus = 0,
        .rail = 0,
    },
    .avsbus_vdn = {
        .bus = 0,
        .rail = 0,
    },

    // -----------------------------------------------------------
    // Power Cap Initializations
    // -----------------------------------------------------------
    .pcap = {
        .current_pcap  = 0,
        .soft_min_pcap = 0,
        .hard_min_pcap = 0,
        .max_pcap      = 0,
        .oversub_pcap  = 0,
        .system_pcap   = 0,
        .unthrottle    = 0,
        .source        = 0,
    },


    // -----------------------------------------------------------
    // Master/Slave Section Initializations
    // -----------------------------------------------------------
    .is_occ_present = SYSCFG_ZERO_OCCS_PRESENT,

    .master_config = {
        .is_master_capable = SYSCFG_MASTER_CAPABLE_000,
        .default_master    = SYSCFG_DEFAULT_MASTER_000,
    },

    // -----------------------------------------------------------
    // Oversubscription Initializations
    // -----------------------------------------------------------

    //Master ppb_fmax calculated by Master OCC's slave.
    .master_ppb_fmax = 0xFFFF,

    // -----------------------------------------------------------
    // Centaur/DIMM Initialization
    // -----------------------------------------------------------
    .centaur_huids = {0},
    .dimm_huids = {{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}},
    .mem_type = MEM_TYPE_NIMBUS,
    .dimm_i2c_engine = PIB_I2C_ENGINE_E,
    .ips_mem_pwr_ctl = MEM_PWR_CTL_OFF,
    .default_mem_pwr_ctl = MEM_PWR_CTL_OFF,

    // -------------------------------------------------------------------
    // Memory Throttle Limits Initialization (for both Nimbus and Cumulus)
    // -------------------------------------------------------------------
    .mem_throt_limits =
    {
        {{0}, {0}, {0}, {0}},
        {{0}, {0}, {0}, {0}},
        {{0}, {0}, {0}, {0}},
        {{0}, {0}, {0}, {0}},
        {{0}, {0}, {0}, {0}},
        {{0}, {0}, {0}, {0}},
        {{0}, {0}, {0}, {0}},
        {{0}, {0}, {0}, {0}}
    },
};


// OCC Module Configuration Data
//
// We will initialize everything to default values, so in that case that we
// can't read the data from mainstore, we will still be able to *do something*
// instead of crash.
occModuleConfigData_t G_occModuleConfigData = {
    0
};

// Power Configuration Data
pcap_config_data_t G_master_pcap_data =
{
    .current_pcap    = 0,
    .soft_min_pcap   = 0,
    .hard_min_pcap   = 0,
    .max_pcap        = 0,
    .oversub_pcap    = 0,
    .system_pcap     = 0,
    .unthrottle      = 0,
    .pcap_data_count = 0,
};

// Function Specification
//
// Name: sysConfigFspLess
//
// Description: Since we can have a OCC Simulation in Simics without
//              having a FSP, we need to have a way for OCC to automatically
//              set itself up the way FSP would.  This is done via default
//              config data, and this function.
//
// End Function Specification
#ifdef FSPLESS_SIMICS

#include "cmdh_fsp_cmds_datacnfg.h"
#include "dcom.h"
#include "state.h"
#include "mode.h"

void sysConfigFspLess(void)
{
    // ----------------------------------------------------
    // Set OCC Role based on Config Data
    // ----------------------------------------------------
    if( G_pbax_id.chip_id == G_sysConfigData.master_config.default_master )
    {
        G_occ_role = OCC_MASTER;

        // Run master initializations if we just became master
        extern void  master_occ_init(void);
        master_occ_init();

        // Turn off anything slave related since we are a master
        rtl_clr_run_mask_deferred(RTL_FLAG_NOTMSTR);
        rtl_set_run_mask_deferred(RTL_FLAG_MSTR);

        // Set Final Mode & State.  OCC will transition through as
        // all requirements for state/mode become available.
        G_occ_external_req_state = OCC_STATE_ACTIVE;
        G_occ_external_req_mode  = OCC_MODE_NOMINAL;
    }
    else
    {
        G_occ_role = OCC_SLAVE;

        // Turn off anything master related since we are a slave
        rtl_clr_run_mask_deferred(RTL_FLAG_MSTR);
        rtl_set_run_mask_deferred(RTL_FLAG_NOTMSTR);
    }

    // Trace our current fspless role
    if(OCC_MASTER == G_occ_role){
        TRAC_IMP("Our current FspLess role: Master");
    }
    else{
        TRAC_IMP("Our current FspLess role: Slave");
    }

    // ----------------------------------------------------
    // Mark available all data we have hardcoded and
    // correctly initialized.
    // ----------------------------------------------------
    extern data_cnfg_t * G_data_cnfg;
    G_data_cnfg->data_mask = (   DATA_MASK_FREQ_PRESENT
                               | DATA_MASK_SET_ROLE
                               | DATA_MASK_APSS_CONFIG
                               | DATA_MASK_PCAP_PRESENT );

}

#endif

