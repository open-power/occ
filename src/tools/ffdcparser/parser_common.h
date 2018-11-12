/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/tools/ffdcparser/parser_common.h $                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2018                             */
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

// Constants
#define MAXIMUM_QUADS 6
#define CORE_IDDQ_MEASUREMENTS 6
#define MAX_NUM_CORES 24

// WOF data struct
typedef struct __attribute__ ((packed))
{
    uint32_t wof_disabled;
    uint8_t  version;
    uint16_t vfrt_block_size;
    uint16_t vfrt_blck_hdr_sz;
    uint16_t vfrt_data_size;
    uint8_t  active_quads_size;
    uint8_t  core_count;
    uint16_t vdn_start;
    uint16_t vdn_step;
    uint16_t vdn_size;
    uint16_t vdd_start;
    uint16_t vdd_step;
    uint16_t vdd_size;
    uint16_t vratio_start;
    uint16_t vratio_step;
    uint16_t vratio_size;
    uint16_t fratio_start;
    uint16_t fratio_step;
    uint16_t fratio_size;
    uint16_t vdn_percent[8];
    uint16_t socket_power_w;
    uint16_t nest_freq_mhz;
    uint16_t nom_freq_mhz;
    uint16_t rdp_capacity;
    uint64_t wof_tbls_src_tag;
    uint64_t package_name_hi;
    uint64_t package_name_lo;
    uint16_t vdd_step_from_start;
    uint16_t vdn_step_from_start;
    uint8_t quad_step_from_start;
    uint32_t v_core_100uV[MAXIMUM_QUADS];
    uint32_t core_pwr_on;
    uint8_t cores_on_per_quad[MAXIMUM_QUADS];
    uint32_t voltvddsense_sensor;
    uint16_t tempprocthrmc[MAX_NUM_CORES];
    uint16_t tempnest_sensor;
    uint16_t tempq[MAXIMUM_QUADS];
    uint16_t curvdd_sensor;
    uint16_t curvdn_sensor;
    uint16_t voltvdn_sensor;
    uint8_t  quad_x_pstates[MAXIMUM_QUADS];
    uint8_t quad_v_idx[MAXIMUM_QUADS];
    uint8_t  quad_ivrm_states;
    uint32_t idc_vdd;
    uint32_t idc_vdn;
    uint32_t idc_quad;
    uint32_t iac_vdd;
    uint32_t iac_vdn;
    uint32_t iac_tdp_vdd;
    uint16_t v_ratio;
    uint16_t f_ratio;
    uint16_t v_clip;
    uint8_t f_clip_ps;
    uint32_t f_clip_freq;
    uint32_t ceff_tdp_vdd;
    uint32_t ceff_vdd;
    uint32_t ceff_ratio_vdd;
    uint32_t ceff_tdp_vdn;
    uint32_t ceff_vdn;
    uint32_t ceff_ratio_vdn;
    uint8_t chip_volt_idx;
    uint32_t all_cores_off_iso;
    uint32_t all_good_caches_on_iso;
    uint32_t all_caches_off_iso;
    uint32_t quad_good_cores_only[MAXIMUM_QUADS];
    uint16_t quad_on_cores[MAXIMUM_QUADS];
    uint16_t quad_bad_off_cores[MAXIMUM_QUADS];
    uint8_t req_active_quad_update;
    uint8_t prev_req_active_quads;
    uint8_t num_active_quads;
    uint32_t curr_ping_pong_buf;
    uint32_t next_ping_pong_buf;
    uint32_t curr_vfrt_main_mem_addr;
    uint32_t next_vfrt_main_mem_addr;
    uint32_t vfrt_tbls_main_mem_addr;
    uint32_t vfrt_tbls_len;
    uint8_t wof_init_state;
    uint32_t quad_state_0_addr;
    uint32_t quad_state_1_addr;
    uint32_t pgpe_wof_state_addr;
    uint32_t req_active_quads_addr;
    uint16_t core_leakage_percent;
    uint32_t pstate_tbl_sram_addr;
    uint32_t gpe_req_rc;
    uint32_t control_ipc_rc;
    uint8_t vfrt_callback_error;
    uint8_t pgpe_wof_off;
    uint8_t pgpe_wof_disabled;
    uint32_t vfrt_mm_offset;
    uint8_t wof_vfrt_req_rc;
    uint32_t c_ratio_vdd_volt;
    uint32_t c_ratio_vdd_freq;
    uint32_t c_ratio_vdn_volt;
    uint32_t c_ratio_vdn_freq;
    uint8_t vfrt_state;
    uint32_t all_cores_off_before;
    uint8_t good_quads_per_sort;
    uint8_t good_normal_cores_per_sort;
    uint8_t good_caches_per_sort;
    uint8_t good_normal_cores[MAXIMUM_QUADS];
    uint8_t good_caches[MAXIMUM_QUADS];
    uint16_t allGoodCoresCachesOn[CORE_IDDQ_MEASUREMENTS];
    uint16_t allCoresCachesOff[CORE_IDDQ_MEASUREMENTS];
    uint16_t coresOffCachesOn[CORE_IDDQ_MEASUREMENTS];
    uint16_t quad1CoresCachesOn[CORE_IDDQ_MEASUREMENTS];
    uint16_t quad2CoresCachesOn[CORE_IDDQ_MEASUREMENTS];
    uint16_t quad3CoresCachesOn[CORE_IDDQ_MEASUREMENTS];
    uint16_t quad4CoresCachesOn[CORE_IDDQ_MEASUREMENTS];
    uint16_t quad5CoresCachesOn[CORE_IDDQ_MEASUREMENTS];
    uint16_t quad6CoresCachesOn[CORE_IDDQ_MEASUREMENTS];
    uint16_t ivdn[CORE_IDDQ_MEASUREMENTS];
    uint8_t allCoresCachesOnT[CORE_IDDQ_MEASUREMENTS];
    uint8_t allCoresCachesOffT[CORE_IDDQ_MEASUREMENTS];
    uint8_t coresOffCachesOnT[CORE_IDDQ_MEASUREMENTS];
    uint8_t quad1CoresCachesOnT[CORE_IDDQ_MEASUREMENTS];
    uint8_t quad2CoresCachesOnT[CORE_IDDQ_MEASUREMENTS];
    uint8_t quad3CoresCachesOnT[CORE_IDDQ_MEASUREMENTS];
    uint8_t quad4CoresCachesOnT[CORE_IDDQ_MEASUREMENTS];
    uint8_t quad5CoresCachesOnT[CORE_IDDQ_MEASUREMENTS];
    uint8_t quad6CoresCachesOnT[CORE_IDDQ_MEASUREMENTS];
    uint8_t avgtemp_vdn[CORE_IDDQ_MEASUREMENTS];
} wof_data_t;

// Thread struct
typedef struct __attribute__((packed,aligned(4))) thread_dump
{
    uint8_t     len;
    uint8_t     pri;
    uint8_t     state;
    uint8_t     flags;
    uint32_t    timer;
    uint32_t    sem;
    uint32_t    srr0;
    uint32_t    srr1;
    uint32_t    srr2;
    uint32_t    srr3;
    uint32_t    lr;
    uint32_t    stack_trace[8];
} thread_dump_t;

// FFDC struct
typedef struct __attribute__((packed,aligned(4))) ffdc
{
    uint8_t     seq;        // Sequence Number (0x00 for FFDC)
    uint8_t     cmd;        // Command (0x00 for FFDC)
    uint8_t     excp;       // Exception Code
    uint16_t    len;        // FFDC data length
    uint8_t     reserved;   // (0x00 for FFDC)
    uint16_t    ckpt;       // Checkpoint (usually 0x0F00 for FFDC)
    uint32_t    ssx_panic;  // SSX Panic Code
    uint32_t    panic_addr; // Address of panic instruction
    uint32_t    lr;         // Link Register
    uint32_t    msr;        // Machine Status Register
    uint32_t    cr;         // Condition Register
    uint32_t    ctr;        // Count Register
    uint32_t    gpr[32];    // GPR0 - GPR31
    uint32_t    evpr;       // Exception Vector Prefix Register
    uint32_t    xer;        // Fixedpoint Exception Register
    uint32_t    esr;        // Exception Syndrome Register
    uint32_t    dear;       // Bad Address
    uint32_t    srr0;       // Return Address for Non-Crit Interrupts
    uint32_t    srr1;       // MSR at time of non-crit interrupt
    uint32_t    srr2;       // Return Address for Crit Interrupts
    uint32_t    srr3;       // MSR at time of crit interrupt
    uint32_t    mcsr;       // Machine Check Syndrome Register
    uint32_t    pid;        // Process ID Register
    uint32_t    zpr;        // Zone Protection Register
    uint32_t    usprg0;     // User SPR General Purpose Register
    uint32_t    sprg[8];    // SPRG0 - SPRG7
    uint32_t    tcr;        // Timer Control Register
    uint32_t    tsr;        // Timer Status Register
    uint32_t    dbcr0;      // Debug Control Register0
    uint32_t    dbcr1;      // Debug Control Register1
    uint32_t    dbsr;       // Debug Status Register
    uint32_t    ocb_oisr0;
    uint32_t    ocb_oisr1;
    uint32_t    ocb_occmisc;
    uint32_t    ocb_ohtmcr;
    uint32_t    ocb_oimr0;
    uint32_t    ocb_oimr1;
    uint32_t    ocb_oitr0;
    uint32_t    ocb_oitr1;
    uint32_t    ocb_oiepr0;
    uint32_t    ocb_oiepr1;
    uint32_t    ocb_oehdr;
    uint32_t    ocb_ocicfg;
    uint32_t    ocb_onisr0;
    uint32_t    ocb_onisr1;
    uint32_t    ocb_ocisr0;
    uint32_t    ocb_ocisr1;
    uint32_t    ocb_occflg;
    uint32_t    ocb_occhbr;
    uint32_t    ssx_timebase;
    char        buildname[16];
    uint64_t    occlfir;
    uint64_t    pbafir;
    uint32_t    cores_deconf;
    thread_dump_t   main;
    thread_dump_t   cmdh;
    thread_dump_t   dcom;
    uint32_t        stack_trace[8];
    uint32_t        eye_catcher;
} ffdc_t;

uint64_t get_uint64(FILE* i_fhndl)
{
    int      i = 0;
    uint64_t ret = 0;
    uint8_t  byte = 0;

    for(i = 7; i >= 0; i--)
    {
        byte = fgetc(i_fhndl);
        if(EOF != byte)
        {
            ret |= ((uint64_t)byte << (i*8));
        }
    }

    return ret;
}

uint32_t get_uint32(FILE* i_fhndl)
{
    int      i = 0;
    uint32_t ret = 0;
    uint8_t  byte = 0;

    for(i = 3; i >= 0; i--)
    {
        byte = fgetc(i_fhndl);
        if(EOF != byte)
        {
            ret |= (byte << (i*8));
        }
    }

    return ret;
}

uint16_t get_uint16(FILE* i_fhndl)
{
    int      i = 0;
    uint16_t ret = 0;
    uint8_t  byte = 0;

    for(i = 1; i >= 0; i--)
    {
        byte = fgetc(i_fhndl);
        if(EOF != byte)
        {
            ret |= (byte << (i*8));
        }
    }

    return ret;
}
