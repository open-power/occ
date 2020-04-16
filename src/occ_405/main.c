/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/main.c $                                          */
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

#include "ssx.h"
#include "ssx_io.h"
#include "ipc_api.h"                //ipc base interfaces
#include "occhw_async.h"            //async (GPE, OCB, etc) interfaces
#include "simics_stdio.h"
#include <thread.h>
#include <sensor.h>
#include <threadSch.h>
#include <errl.h>
#include <apss.h>
#include <trac.h>
#include <occ_service_codes.h>
#include <occ_sys_config.h>
#include <timer.h>
#include <dcom.h>
#include <rtls.h>
#include <proc_data.h>
#include <dpss.h>
#include <state.h>
#include <amec_sys.h>
#include <cmdh_fsp.h>
#include <proc_pstate.h>
#include <chom.h>
#include <homer.h>
#include <amec_health.h>
#include <amec_freq.h>
#include <pss_service_codes.h>
#include "occhw_shared_data.h"
#include <pgpe_shared.h>
#include <gpe_register_addresses.h>
#include <pstates_occ.H>
#include <wof.h>
#include "pgpe_service_codes.h"
#include <common.h>
#include "p10_hcd_memmap_occ_sram.H"
#include "pstate_pgpe_occ_api.h"
#include "misc_scom_addresses.h"

// timeout, as number of WOF ticks, for PGPE VRT command
uint8_t G_max_vrt_chances = MAX_VRT_CHANCES_EVERY_TICK;
// timeout, as number of WOF ticks, for PGPE WOF Control command
uint8_t G_max_wof_control_chances = MAX_WOF_CONTROL_CHANCES_EVERY_TICK;

uint32_t G_max_ceff_ratio = MAX_CEFF_RATIO;

extern uint32_t __ssx_boot; // Function address is 32 bits
extern uint32_t G_occ_phantom_critical_count;
extern uint32_t G_occ_phantom_noncritical_count;
extern uint8_t G_occ_interrupt_type;
extern uint8_t G_occ_role;
extern volatile pstateStatus G_proc_pstate_status;

extern GpeRequest G_meas_start_request;
extern GpeRequest G_meas_cont_request;
extern GpeRequest G_meas_complete_request;
extern apss_start_args_t    G_gpe_start_pwr_meas_read_args;
extern apss_continue_args_t G_gpe_continue_pwr_meas_read_args;
extern apss_complete_args_t G_gpe_complete_pwr_meas_read_args;
extern uint32_t G_present_cores;

extern bool G_smf_mode;

extern PWR_READING_TYPE  G_pwr_reading_type;

IMAGE_HEADER (G_mainAppImageHdr,__ssx_boot,MAIN_APP_ID,ID_NUM_INVALID);

ppmr_header_t G_ppmr_header;       // PPMR Header layout format
pgpe_header_data_t G_pgpe_header;  // PGPE Header layout format
OCCPstateParmBlock_t G_oppb;       // OCC Pstate Parameters Block Structure
extern uint16_t G_proc_fmax_mhz;
extern volatile int G_ss_pgpe_rc;

// Buffer to hold the wof header
DMA_BUFFER(temp_bce_request_buffer_t G_temp_bce_buff) = {{0}};

// Set main thread timer for one second
#define MAIN_THRD_TIMER_SLICE ((SsxInterval) SSX_SECONDS(1))

// Size of "Reserved" section in OCC-PGPE shared SRAM
#define OCC_PGPE_SRAM_RESERVED_SZ 7


// Define location for data shared with GPEs
gpe_shared_data_t G_shared_gpe_data __attribute__ ((section (".gpe_shared")));

// SIMICS printf/printk
SimicsStdio G_simics_stdout;
SimicsStdio G_simics_stderr;

//  Critical /non Critical Stacks
uint8_t G_noncritical_stack[NONCRITICAL_STACK_SIZE];
uint8_t G_critical_stack[CRITICAL_STACK_SIZE];

//NOTE: Three semaphores are used so that if in future it is decided
// to move health monitor and FFDC into it's own threads, then
// it can be done easily without more changes.
// Semaphores for the health monitor functions
SsxSemaphore G_hmonSem;
// Semaphores for the FFDC functions
SsxSemaphore G_ffdcSem;
// Timer for posting health monitor and FFDC semaphore
SsxTimer G_mainThrdTimer;

// Variable holding main thread loop count
uint32_t G_mainThreadLoopCounter = 0x0;

// Global flag indicating FIR collection is required
bool G_fir_collection_required = FALSE;

// Global flag indicating we are running on Simics
bool G_simics_environment = FALSE;

// OCC frequency in MHz
uint32_t G_occ_frequency_mhz;

extern uint8_t g_trac_inf_buffer[];
extern uint8_t g_trac_imp_buffer[];
extern uint8_t g_trac_err_buffer[];

/*
 * Function Specification
 *
 * Name: check_runtime_environment
 *
 * Description: Determines whether we are running in Simics or on real HW.
 *
 * End Function Specification
 */
void check_runtime_environment(void)
{
    uint64_t flags;
    flags = in64(OCB_OCCFLG0);

    // NOTE: The lower 32 bits of this register have no latches on
    //       physical hardware and will return 0s, so we can use
    //       a backdoor hack in Simics to set bit 63, telling the
    //       firmware what environment it's running in.
    G_simics_environment = ( 0 != (flags & 0x0000000000000001) ) ? TRUE : FALSE;
    if (G_simics_environment)
    {
        // slow down RTL for Simics
        G_mics_per_tick = SIMICS_MICS_PER_TICK;
        G_dcom_tx_apss_wait_time = SIMICS_MICS_PER_TICK * 4 / 10;
    }
}

/*
 * Function Specification
 *
 * Name: populate_sys_mode_freq_table
 *
 * Description: Populate table with frequency used for each mode
 *              Called one time during init after OPPB is read
 *              Prior to being called it has been verified that
 *              G_oppb.frequency_min_khz < G_oppb.frequency_max_khz
 *              and both aren't 0
 *
 * End Function Specification
 */
void populate_sys_mode_freq_table(void)
{
    errlHndl_t  l_err = NULL;
    int         i = 0;
    int         l_max_freq_index = 0;
    uint32_t    l_max_freq = 0;

    // VPD curve fit points
    for(i = OCC_FREQ_PT_VPD_CF0; i <= OCC_FREQ_PT_VPD_LAST_CF; i++)
    {
        // OPPB operating_points has NUM_PV_POINTS make sure we don't blow past the end of OPPB
        if(i < NUM_PV_POINTS)
        {
            G_sysConfigData.sys_mode_freq.table[i] = G_oppb.operating_points[i].frequency_mhz;

            // should be increasing by frequency but just trace if it isn't may need to do something more later
            if(G_sysConfigData.sys_mode_freq.table[i] > l_max_freq)
            {
                l_max_freq = G_sysConfigData.sys_mode_freq.table[i];
                l_max_freq_index = i;
            }
            else
            {
                MAIN_TRAC_ERR("populate_sys_mode_freq_table: CF%d freq %dMHz < %dMHz previous pt",
                               i, G_sysConfigData.sys_mode_freq.table[i], l_max_freq);
            }

        }
        else
        {
            // this is indication of code mismatch stop copying over curve fit points
            MAIN_TRAC_ERR("populate_sys_mode_freq_table: mismatch in num CF freq points OCC[%d] OPPB[%d]",
                          OCC_FREQ_PT_VPD_LAST_CF+1, NUM_PV_POINTS);
            break;
        }
    }

    // Set the maximum frequency for this chip
    // use the VPD operating point not Pstate 0 (G_oppb.frequency_max_khz) which
    // represents the highest Fmax across all chips in the system here we want what
    // this chip is capable of
    if(l_max_freq_index != VPD_PV_FMAX)
    {
        MAIN_TRAC_ERR("populate_sys_mode_freq_table: max freq %d found at VPD index %d",
                       l_max_freq, l_max_freq_index);

        G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ] = l_max_freq;
    }
    else
    {
        G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ] = G_oppb.operating_points[VPD_PV_FMAX].frequency_mhz;
    }
    // sanity check that the chip's Fmax is equal to or lower than Pstate 0 frequency
    if(G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ] > (G_oppb.frequency_max_khz / 1000) )
    {
        // Impossible to set anything higher than Pstate 0! Clip max to Pstate 0 frequency
        MAIN_TRAC_ERR("populate_sys_mode_freq_table: chip Fmax[%dMHz] is higher than Pstate0[%dMHz]",
                       G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ],
                       G_oppb.frequency_max_khz / 1000);
        G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ] = G_oppb.frequency_max_khz / 1000;
    }

    // set the minimum frequency allowed.  Don't use VPD as this may be different
    // than VPD power save point due to uplift calculated by the PGPE
    G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ] = G_oppb.frequency_min_khz / 1000;
    // sanity check if the chip's Fmin is higher need to uplift the min to the chip's min
    if(G_oppb.operating_points[VPD_PV_PSAV].frequency_mhz > (G_oppb.frequency_min_khz / 1000) )
    {
        MAIN_TRAC_ERR("populate_sys_mode_freq_table: chip min[%dMHz] is higher than sys min[%dMHz]",
                       G_oppb.operating_points[VPD_PV_PSAV].frequency_mhz,
                       G_oppb.frequency_min_khz / 1000);
        G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ] = G_oppb.operating_points[VPD_PV_PSAV].frequency_mhz;
    }

    // WOF base frequency
    if(G_oppb.tdp_wof_base_frequency_mhz)
    {
        G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE] = G_oppb.tdp_wof_base_frequency_mhz;
        // sanity check that this isn't higher than max
        if(G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE] > G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ])
        {
            MAIN_TRAC_ERR("populate_sys_mode_freq_table: WOF Base freq[%dMHz] higher than max[%dMHz]",
                           G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE],
                           G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ]);
            G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE] = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ];
        }
        // sanity check that this isn't lower than min
        if(G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE] < G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ])
        {
            MAIN_TRAC_ERR("populate_sys_mode_freq_table: WOF Base freq[%dMHz] lower than min[%dMHz]",
                           G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE],
                           G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ]);
            G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE] = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ];
        }
    }
    else
    {
        // WOF base frequency is 0, use first non-zero operating point +2 above min if none found use the minimum
        i = VPD_PV_PSAV + 2;
        do
        {
            if(G_oppb.operating_points[i].frequency_mhz)
            {
              MAIN_TRAC_ERR("populate_sys_mode_freq_table: WOF Base freq is 0 using CF%d freq %dMHz",
                                  i, G_oppb.operating_points[i].frequency_mhz);
                G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE] = G_oppb.operating_points[i].frequency_mhz;
                break;
            }
            i++;
        }while(i < NUM_PV_POINTS);
        if(i == NUM_PV_POINTS)
        {
            MAIN_TRAC_ERR("populate_sys_mode_freq_table: WOF Base freq is 0 and failed to find non-zero point using min freq %dMHz",
                          G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ]);
            G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE] = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ];
        }
    }

    // Power management disabled frequency in VPD this is called "fixed frequency"
    // and is defined to be the same as P9 legacy turbo this differs from WOF base
    // in that WOF base may be a higher frequency and is not guaranteed for all environments
    if(G_oppb.fixed_freq_mode_frequency_mhz)
    {
        G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DISABLED] = G_oppb.fixed_freq_mode_frequency_mhz;
        // sanity check that this isn't higher than max
        if(G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DISABLED] > G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ])
        {
            MAIN_TRAC_ERR("populate_sys_mode_freq_table: Disabled freq[%dMHz] higher than max[%dMHz]",
                           G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DISABLED],
                           G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ]);
            G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DISABLED] = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ];
        }
        // sanity check that this isn't lower than min
        if(G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DISABLED] < G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ])
        {
            MAIN_TRAC_ERR("populate_sys_mode_freq_table: Disabled freq[%dMHz] lower than min[%dMHz]",
                           G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DISABLED],
                           G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ]);
            G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DISABLED] = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ];
        }
    }
    else
    {
        // Disabled frequency is 0 just use WOF base
        MAIN_TRAC_ERR("populate_sys_mode_freq_table: Disabled freq is 0 using WOF base freq %dMHz",
                          G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE]);

        G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DISABLED] = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE];
    }

    // Ultra Turbo VPD point
    G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_VPD_UT] = G_oppb.operating_points[VPD_PV_UT].frequency_mhz;

    if(G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_VPD_UT] == 0)
    {
      MAIN_TRAC_ERR("populate_sys_mode_freq_table: UT freq is 0 using WOF base freq %dMHz for UT",
                       G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE]);
        G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_VPD_UT] = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE];
        MAIN_TRAC_INFO("WOF Disabled due to 0 UT value.");
        set_clear_wof_disabled( SET,
                                WOF_RC_UTURBO_IS_ZERO,
                                ERC_WOF_UTURBO_IS_ZERO );

    }

    // sanity check that UT isn't higher than max
    else if(G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_VPD_UT] >
            G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ])
    {
        MAIN_TRAC_ERR("populate_sys_mode_freq_table: UT freq[%dMHz] higher than max[%dMHz]",
                       G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_VPD_UT],
                       G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ]);
        G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_VPD_UT] = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ];
    }
    // Copy over UT frequency into G_proc_fmax_mhz
    // can only go higher than UT if Fmax mode is enabled
    G_proc_fmax_mhz = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_VPD_UT];

    MAIN_TRAC_IMP("populate_sys_mode_freq_table: freq points min[%dMHz], WOF Base[%dMHz], UT[%dMHz], max[%dMHz]",
                   G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ],
                   G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_WOF_BASE],
                   G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_VPD_UT],
                   G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ]);

    // set frequency for all other power management modes
    G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_PWR_SAVE] = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MIN_FREQ];
    G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DYN_PERF] = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_VPD_UT];
    G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_MAX_PERF] = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_VPD_UT];
    G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_FMAX] = G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MAX_FREQ];

    MAIN_TRAC_IMP("populate_sys_mode_freq_table: Mode freq disabled[%dMHz], Psav[%dMHz], Perf[%dMHz], Fmax[%dMHz]",
                   G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_DISABLED],
                   G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_PWR_SAVE],
                   G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_MAX_PERF],
                   G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_MODE_FMAX]);

    // set freq for bottom throttle point to 1Mhz this will translate into pstate for the most throttled settin
    G_sysConfigData.sys_mode_freq.table[OCC_FREQ_PT_BOTTOM_THROTTLE] = 1;

    // Set the frequency range for amec, at this point there is no mode set
    l_err = amec_set_freq_range(OCC_MODE_NOCHANGE);
    if(l_err)
    {
        // Commit log
        commitErrl(&l_err);
    }
} // end populate_sys_mode_freq_table()

/*
 * Function Specification
 *
 * Name: read_wof_header
 *
 * Description: Read WOF Tables header and populate global variables
 *              needed for WOF.  Must be called after pgpe header is read.
 *
 * End Function Specification
 */
void read_wof_header(void)
{
    int l_ssxrc = SSX_OK;
    bool l_error = false;

    // Read wof tables address, and wof tables len
    if(G_pgpe_header.wof_tables_addr == 0)
    {
        MAIN_TRAC_ERR("read_wof_header(): WOF tables address is 0, WOF is disabled");
        l_error = TRUE;
    }
    else
    {
        g_amec->static_wof_data.vrt_tbls_main_mem_addr = HOMER_BASE_ADDRESS + G_pgpe_header.wof_tables_addr;
        g_amec->static_wof_data.vrt_tbls_len           = G_pgpe_header.wof_tables_length;

        MAIN_TRAC_INFO("read_wof_header: WOF Tables Main Memory Address[0x%08X], Length[0x%08X] ",
                       g_amec->static_wof_data.vrt_tbls_main_mem_addr,
                       g_amec->static_wof_data.vrt_tbls_len);

        if (g_amec->static_wof_data.vrt_tbls_main_mem_addr%128 != 0)
        {
            MAIN_TRAC_ERR("read_wof_header: WOF tables address is NOT"
                    " 128-byte aligned, WOF is disabled");
            l_error = TRUE;
        }
        else
        {
            do
            {
                // use block copy engine to read WOF header
                BceRequest l_wof_header_req;

                // Create request
                l_ssxrc = bce_request_create(&l_wof_header_req,              // block copy object
                                             &G_pba_bcde_queue,              // main to sram copy engine
                                             g_amec->static_wof_data.vrt_tbls_main_mem_addr, // mainstore address
                                             (uint32_t) &G_temp_bce_buff,    // SRAM start address
                                             MIN_BCE_REQ_SIZE,               // size of copy
                                             SSX_WAIT_FOREVER,               // no timeout
                                             NULL,                           // no call back
                                             NULL,                           // no call back args
                                             ASYNC_REQUEST_BLOCKING);        // blocking request
                if(l_ssxrc != SSX_OK)
                {
                    MAIN_TRAC_ERR("read_wof_header: BCDE request create failure rc=[%08X]", -l_ssxrc);
                    l_error = TRUE;
                    break;
                }

                // Do the actual copy
                l_ssxrc = bce_request_schedule(&l_wof_header_req);
                if(l_ssxrc != SSX_OK)
                {
                    MAIN_TRAC_ERR("read_wof_header: BCE request schedule failure rc=[%08X]", -l_ssxrc);
                    l_error = TRUE;
                    break;
                }

                // Copy the data into Global WOF header struct
                memcpy(&g_amec->static_wof_data.wof_header,
                       G_temp_bce_buff.data,
                       sizeof(wof_header_data_t));

                // verify the validity of the magic number
                if( (WOF_TABLES_MAGIC_NUMBER != g_amec->static_wof_data.wof_header.magic_number) ||
                    (WOF_TABLES_VERSION != g_amec->static_wof_data.wof_header.version) )
                {
                    MAIN_TRAC_ERR("read_wof_header: Invalid WOF Magic number[0x%08X] or version[0x%02X]. Address[0x%08X]. WOF disabled",
                                  g_amec->static_wof_data.wof_header.magic_number,
                                  g_amec->static_wof_data.wof_header.version,
                                  g_amec->static_wof_data.vrt_tbls_main_mem_addr);
                    l_error = TRUE;
                    break;
                }

                MAIN_TRAC_INFO("read_wof_header: valid WOF Magic No[0x%08X] and version[0x%02X]",
                                g_amec->static_wof_data.wof_header.magic_number,
                                g_amec->static_wof_data.wof_header.version);

                MAIN_TRAC_INFO("read_wof_header: VRT block size[%d] VRT Header size[%d]  VRT Data size[%d]",
                                g_amec->static_wof_data.wof_header.vrt_block_size,
                                g_amec->static_wof_data.wof_header.vrt_blck_hdr_sz,
                                g_amec->static_wof_data.wof_header.vrt_data_size);

                MAIN_TRAC_INFO("read_wof_header: OCS Mode[%d] Core Count[%d]",
                                g_amec->static_wof_data.wof_header.ocs_mode,
                                g_amec->static_wof_data.wof_header.core_count);

                // Initialize wof init state to zero
                g_amec->wof.wof_init_state  = WOF_DISABLED;

                // Default to no overrides
                g_amec->wof.vcs_override_index = WOF_VRT_IDX_NO_OVERRIDE;
                g_amec->wof.vdd_override_index = WOF_VRT_IDX_NO_OVERRIDE;
                g_amec->wof.io_pwr_override_index = WOF_VRT_IDX_NO_OVERRIDE;
                g_amec->wof.ambient_override_index = WOF_VRT_IDX_NO_OVERRIDE;
                g_amec->wof.v_ratio_override_index = WOF_VRT_IDX_NO_OVERRIDE;

                // Initialize OCS increase/decrease amounts to one step
                g_amec->wof.ocs_increase_ceff = g_amec->static_wof_data.wof_header.vdd_step;
                g_amec->wof.ocs_decrease_ceff = g_amec->static_wof_data.wof_header.vdd_step;

                // calculate max ceff ratio from header info
                G_max_ceff_ratio = ( g_amec->static_wof_data.wof_header.vdd_start +
                                    ( g_amec->static_wof_data.wof_header.vdd_step *
                                     (g_amec->static_wof_data.wof_header.vdd_size - 1) ) );

            }while( 0 );

        } // else 128 aligned address read WOF header

    } // else wof address !0

    // Check for errors and log, if any
    if ( l_error )
    {
        // We were unable to get the WOF header thus WOF should not run.
        set_clear_wof_disabled( SET,
                                WOF_RC_NO_WOF_HEADER_MASK,
                                ERC_WOF_NO_WOF_HEADER_MASK );
    }
} // end read_wof_header()

/*
 * Function Specification
 *
 * Name: read_pgpe_header
 *
 * Description: Initialize PGPE image header entry in DTLB,
 *              Read PGPE image header, lookup shared SRAM address and size,
 *              Populate global variables, including G_pgpe_beacon_address.
 *
 * Returns: TRUE if read was successful, else FALSE
 *
 * End Function Specification
 */
bool read_pgpe_header(void)
{
    uint32_t l_reasonCode = 0;
    uint32_t l_extReasonCode = OCC_NO_EXTENDED_RC;
    uint32_t userdata1 = 0;
    uint32_t userdata2 = 0;
    uint32_t pgpe_shared_magic_number = 0;
    uint8_t  l_bit_mask = 0;
    int      i = 0;

    MAIN_TRAC_INFO("read_pgpe_header(0x%08X)", PGPE_HEADER_ADDR);
    // Copy the header into Global PGPE header struct
    memcpy(&G_pgpe_header.magic_number,
           (const void*)PGPE_HEADER_ADDR,
           sizeof(pgpe_header_data_t));
    do
    {
        // verify the validity of the magic number
        MAIN_TRAC_IMP("PGPE HEADER magic number[0x%08X%08X]", WORD_HIGH(G_pgpe_header.magic_number), WORD_LOW(G_pgpe_header.magic_number));

        if( (0x46414B455F484452 == G_pgpe_header.magic_number) || // "FAKE_HDR"
            (PGPE_MAGIC_NUMBER_10 == G_pgpe_header.magic_number) )
        {
            // we can't run without shared SRAM or PGPE beacon address
            if ((G_pgpe_header.beacon_sram_addr == 0) ||
                (G_pgpe_header.shared_sram_addr == 0))
            {
                MAIN_TRAC_ERR("read_pgpe_header: Zero Shared SRAM Address[0x%08x] or PGPE Beacon Address[0x%08x]",
                              G_pgpe_header.shared_sram_addr,
                              G_pgpe_header.beacon_sram_addr);
                /*
                 * @errortype
                 * @moduleid    READ_PGPE_HEADER
                 * @reasoncode  SSX_GENERIC_FAILURE
                 * @userdata1   lower word of beacon sram address
                 * @userdata2   lower word of shared sram address
                 * @userdata4   ERC_PGPE_INVALID_ADDRESS
                 * @devdesc     Invalid sram addresses from PGPE header
                 */
                l_reasonCode = SSX_GENERIC_FAILURE;
                l_extReasonCode = ERC_PGPE_INVALID_ADDRESS;
                userdata1 = WORD_LOW(G_pgpe_header.beacon_sram_addr);
                userdata2 = WORD_LOW(G_pgpe_header.shared_sram_addr);
                break;
            }

            MAIN_TRAC_IMP("read_pgpe_header: PGPE Beacon Address[0x%08X]", G_pgpe_header.beacon_sram_addr);

            // we have non-zero shared SRAM address now verify shared SRAM version
            // OCC-PGPE shared SRAM version is first word of shared SRAM
            pgpe_shared_magic_number = in32(G_pgpe_header.shared_sram_addr);
            if(pgpe_shared_magic_number == OPS_MAGIC_NUMBER_P10)
            {
                MAIN_TRAC_IMP("read_pgpe_header: Valid PGPE Shared SRAM address[0x%08X] and Magic Number[0x%08X]",
                               G_pgpe_header.shared_sram_addr, pgpe_shared_magic_number);

                const HcodeOCCSharedData_t *shared_sram = (HcodeOCCSharedData_t*)G_pgpe_header.shared_sram_addr;
                // Initialization for handling hcode errors
                const uint32_t hcode_elog_table_addr = G_pgpe_header.shared_sram_addr + shared_sram->error_log_offset;
                hcode_error_table_t hcode_etable;
                hcode_etable.dw0.value = in64(hcode_elog_table_addr);
                if (HCODE_ELOG_TABLE_MAGIC_NUMBER == hcode_etable.dw0.fields.magic_word)
                {
                    G_hcode_elog_table_slots = hcode_etable.dw0.fields.total_log_slots;
                    G_hcode_elog_table = (hcode_elog_entry_t*)(hcode_elog_table_addr + 8);
                    MAIN_TRAC_IMP("read_pgpe_header: Valid HCODE Elog version[0x%08X], HCODE Elog Table [0x%08X] (%d slots)",
                                  hcode_etable.dw0.fields.magic_word,
                                  G_hcode_elog_table,
                                  G_hcode_elog_table_slots);
                }
                else
                {
                    G_hcode_elog_table_slots = 0;
                    G_hcode_elog_table = 0;
                    MAIN_TRAC_ERR("read_pgpe_header: HCODE Elog version[0x%08X] NOT valid! No HCODE Elog support!",
                                  hcode_etable.dw0.fields.magic_word);
                }

                // Initialization for WOF data
                g_amec->static_wof_data.occ_values_sram_addr = G_pgpe_header.shared_sram_addr + shared_sram->occ_data_offset;
                g_amec->static_wof_data.pgpe_values_sram_addr = G_pgpe_header.shared_sram_addr + shared_sram->pgpe_data_offset;
                g_amec->static_wof_data.xgpe_values_sram_addr = G_pgpe_header.shared_sram_addr + shared_sram->xgpe_data_offset;
                MAIN_TRAC_IMP("read_pgpe_header: Produced WOF values SRAM address OCC[0x%08X] PGPE[0x%08X] XGPE[0x%08X]",
                               g_amec->static_wof_data.occ_values_sram_addr,
                               g_amec->static_wof_data.pgpe_values_sram_addr,
                               g_amec->static_wof_data.xgpe_values_sram_addr);

                g_amec->static_wof_data.xgpe_iddq_activity_sram_addr = G_pgpe_header.shared_sram_addr + shared_sram->iddq_data_offset;
                g_amec->static_wof_data.iddq_activity_sample_depth = shared_sram->iddq_activity_sample_depth;
                // sanity check sample depth must be non-zero and a power of 2 (exactly 1 bit set)
                if(__builtin_popcount(g_amec->static_wof_data.iddq_activity_sample_depth) == 1)
                {
                    l_bit_mask = 0x01;
                    for(i=0; i<8; i++)
                    {
                        if( g_amec->static_wof_data.iddq_activity_sample_depth & (l_bit_mask << i) )
                        {
                            // set the number of bits to shift in order to do a divide by activity depth to calcualte
                            // percentages in the WOF calculations
                            g_amec->static_wof_data.iddq_activity_divide_bit_shift = i;
                            break;
                        }
                    }
                }
                else
                {
                    // Invalid IDDQ activity sample depth can't run WOF
                    MAIN_TRAC_ERR("read_pgpe_header: IDDQ Activity Sample Depth[0x%02X] NOT valid! No WOF support!",
                                  g_amec->static_wof_data.iddq_activity_sample_depth);
                    if(!G_simics_environment)
                    set_clear_wof_disabled( SET,
                                            WOF_RC_INVALID_IDDQ_SAMPLE_DEPTH,
                                            ERC_WOF_INVALID_IDDQ_SAMPLE_DEPTH );
                    else
                    {
                        g_amec->static_wof_data.iddq_activity_sample_depth = 8;  // hard code due to simics
                        g_amec->static_wof_data.iddq_activity_divide_bit_shift = 3;
                        MAIN_TRAC_IMP("read_pgpe_header: In SIMICS setting IDDQ Activity Sample Depth to 8 and allowing WOF");
                    }
                }

                MAIN_TRAC_IMP("read_pgpe_header: IDDQ Activity SRAM addr[0x%08X] depth[0x%02X] bit shift[%d]",
                              g_amec->static_wof_data.xgpe_iddq_activity_sram_addr,
                              g_amec->static_wof_data.iddq_activity_sample_depth,
                              g_amec->static_wof_data.iddq_activity_divide_bit_shift);

                g_amec->static_wof_data.pstate_tbl_sram_addr   = G_pgpe_header.occ_pstate_table_sram_addr;

                MAIN_TRAC_IMP("read_pgpe_header: Pstate table SRAM Address[0x%08X]",
                              g_amec->static_wof_data.pstate_tbl_sram_addr);

                // Read in WOF tables header
                read_wof_header();
                CHECKPOINT(WOF_IMAGE_HEADER_READ);
            }
            else
            {
                // PGPE Shared SRAM magic number not supported
                MAIN_TRAC_ERR("read_pgpe_header: Invalid PGPE Shared SRAM Magic number. Addr[0x%08X], Magic Number[0x%08X]",
                               G_pgpe_header.shared_sram_addr, pgpe_shared_magic_number);


                /* @
                 * @errortype
                 * @moduleid    READ_PGPE_HEADER
                 * @reasoncode  INVALID_MAGIC_NUMBER
                 * @userdata1   OCC PGPE Shared SRAM magic number
                 * @userdata2   0
                 * @userdata4   ERC_OPS_INVALID_MAGIC_NUMBER
                 * @devdesc     Invalid magic number in OCC PGPE Shared SRAM
                 */
                l_reasonCode = INVALID_MAGIC_NUMBER;
                l_extReasonCode = ERC_OPS_INVALID_MAGIC_NUMBER;
                userdata1 = pgpe_shared_magic_number;
                userdata2 = 0;
                break;
            }
        }  // if valid PGPE Header magic number
        else
        {
            // The Magic number is invalid .. Invalid or corrupt PGPE image header
            MAIN_TRAC_ERR("read_pgpe_header: Invalid PGPE Magic number. Address[0x%08X], Magic Number[0x%08X%08X]",
                          PGPE_HEADER_ADDR, WORD_HIGH(G_pgpe_header.magic_number), WORD_LOW(G_pgpe_header.magic_number));
            /* @
             * @errortype
             * @moduleid    READ_PGPE_HEADER
             * @reasoncode  INVALID_MAGIC_NUMBER
             * @userdata1   High order 32 bits of retrieved PGPE magic number
             * @userdata2   Low order 32 bits of retrieved PGPE magic number
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Invalid magic number in PGPE header
             */
            l_reasonCode = INVALID_MAGIC_NUMBER;
            userdata1 = WORD_HIGH(G_pgpe_header.magic_number);
            userdata2 = WORD_LOW(G_pgpe_header.magic_number);
            break;
        }
    } while (0);

    if ( l_reasonCode )
    {
        errlHndl_t l_errl = createErrl(READ_PGPE_HEADER,         //modId
                                       l_reasonCode,             //reasoncode
                                       l_extReasonCode,          //Extended reason code
                                       ERRL_SEV_UNRECOVERABLE,   //Severity
                                       NULL,                     //Trace Buf
                                       DEFAULT_TRACE_SIZE,       //Trace Size
                                       userdata1,                //userdata1
                                       userdata2);               //userdata2

        // Callout firmware
        addCalloutToErrl(l_errl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        REQUEST_RESET(l_errl);
        return FALSE;
    }

    // Success
    return TRUE;

} // end read_pgpe_header()


/*
 * Function Specification
 *
 * Name: read_ppmr_header
 *
 * Description: read PPMR image header and validate magic number
 *              Only magic number and OPPB offset/length are used from PPMR
 *
 * Returns: TRUE if read was successful, else FALSE
 *
 * End Function Specification
 */
bool read_ppmr_header(void)
{
    int l_ssxrc = SSX_OK;
    uint32_t l_reasonCode = 0;
    uint32_t l_extReasonCode = OCC_NO_EXTENDED_RC;
    uint32_t userdata1 = 0;
    uint32_t userdata2 = 0;

    MAIN_TRAC_INFO("read_ppmr_header(0x%08X)", PPMR_ADDRESS_HOMER);

    do{
        // use block copy engine to read the PPMR header
        BceRequest pba_copy;

        // Set up a copy request
        l_ssxrc = bce_request_create(&pba_copy,                           // block copy object
                                     &G_pba_bcde_queue,                   // mainstore to sram copy engine
                                     PPMR_ADDRESS_HOMER,                  // mainstore address
                                     (uint32_t) &G_ppmr_header,           // sram starting address
                                     (size_t) sizeof(G_ppmr_header),      // size of copy
                                     SSX_WAIT_FOREVER,                    // no timeout
                                     NULL,                                // no call back
                                     NULL,                                // no call back arguments
                                     ASYNC_REQUEST_BLOCKING);             // blocking request

        if(l_ssxrc != SSX_OK)
        {
            MAIN_TRAC_ERR("read_ppmr_header: BCDE request create failure rc=[%08X]", -l_ssxrc);
            /*
             * @errortype
             * @moduleid    READ_PPMR_HEADER
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   RC for BCE block-copy engine
             * @userdata2   Internal function checkpoint
             * @userdata4   ERC_BCE_REQUEST_CREATE_FAILURE
             * @devdesc     Failed to create BCDE request
             */
            l_reasonCode = SSX_GENERIC_FAILURE;
            l_extReasonCode = ERC_BCE_REQUEST_CREATE_FAILURE;

            userdata1 = (uint32_t)(-l_ssxrc);
            break;
        }

        // Do actual copying
        l_ssxrc = bce_request_schedule(&pba_copy);

        if(l_ssxrc != SSX_OK)
        {
            MAIN_TRAC_ERR("read_ppmr_header: BCE request schedule failure rc=[%08X]", -l_ssxrc);
            /*
             * @errortype
             * @moduleid    READ_PPMR_HEADER
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   RC for BCE block-copy engine
             * @userdata4   ERC_BCE_REQUEST_SCHEDULE_FAILURE
             * @devdesc     Failed to read PPMR data by using BCDE
             */
            l_reasonCode = SSX_GENERIC_FAILURE;
            l_extReasonCode = ERC_BCE_REQUEST_SCHEDULE_FAILURE;

            userdata1 = (uint32_t)(-l_ssxrc);
            break;
        }

        if (PPMR_MAGIC_NUMBER_10 != G_ppmr_header.magic_number)
        {
            MAIN_TRAC_ERR("read_ppmr_header: Invalid PPMR Magic number: 0x%08X%08X",
                          WORD_HIGH(G_ppmr_header.magic_number), WORD_LOW(G_ppmr_header.magic_number));
            /* @
             * @errortype
             * @moduleid    READ_PPMR_HEADER
             * @reasoncode  INVALID_MAGIC_NUMBER
             * @userdata1   High order 32 bits of retrieved PPMR magic number
             * @userdata2   Low order 32 bits of retrieved PPMR magic number
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Invalid magic number in PPMR header
             */
            l_reasonCode = INVALID_MAGIC_NUMBER;
            userdata1 = WORD_HIGH(G_ppmr_header.magic_number);
            userdata2 = WORD_LOW(G_ppmr_header.magic_number);
            break;
        }

    } while (0);

    if ( l_reasonCode )
    {
        errlHndl_t l_errl = createErrl(READ_PPMR_HEADER,         //modId
                                       l_reasonCode,             //reasoncode
                                       l_extReasonCode,          //Extended reason code
                                       ERRL_SEV_UNRECOVERABLE,   //Severity
                                       NULL,                     //Trace Buf
                                       DEFAULT_TRACE_SIZE,       //Trace Size
                                       userdata1,                //userdata1
                                       userdata2);               //userdata2

        // Callout firmware
        addCalloutToErrl(l_errl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        REQUEST_RESET(l_errl);

        return FALSE;
    }

    // Success
    return TRUE;
}

/*
 * Function Specification
 *
 * Name: read_oppb_params
 *
 * Description: Read the OCC Pstates Parameter Block,
 *              and initializa Pstates Global Variables.
 *
 * Returns: TRUE if read was successful, else FALSE
 *
 * End Function Specification
 */
bool read_oppb_params()
{
    int l_ssxrc = SSX_OK;
    uint32_t l_reasonCode = 0;
    uint32_t l_extReasonCode = OCC_NO_EXTENDED_RC;
    uint32_t userdata1 = 0;
    uint32_t userdata2 = 0;
    const uint32_t oppb_address = PPMR_ADDRESS_HOMER + G_ppmr_header.oppb_offset;

    MAIN_TRAC_INFO("read_oppb_params address[0x%08X] length[0x%08X]", oppb_address, sizeof(OCCPstateParmBlock_t));

    do{
        // use block copy engine to read the OPPB header
        BceRequest pba_copy;

        // Set up a copy request
        l_ssxrc = bce_request_create(&pba_copy,                           // block copy object
                                     &G_pba_bcde_queue,                   // mainstore to sram copy engine
                                     oppb_address,                        // mainstore address
                                     (uint32_t) &G_oppb,                  // sram starting address
                                     (size_t) sizeof(OCCPstateParmBlock_t), // size of copy
                                     SSX_WAIT_FOREVER,                    // no timeout
                                     NULL,                                // no call back
                                     NULL,                                // no call back arguments
                                     ASYNC_REQUEST_BLOCKING);             // blocking request

        if(l_ssxrc != SSX_OK)
        {
            MAIN_TRAC_ERR("read_oppb_params: BCDE request create failure rc=[%08X]", -l_ssxrc);
            /*
             * @errortype
             * @moduleid    READ_OPPB_PARAMS
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   RC for BCE block-copy engine
             * @userdata4   ERC_BCE_REQUEST_CREATE_FAILURE
             * @devdesc     Failed to create BCDE request
             */
            l_reasonCode = SSX_GENERIC_FAILURE;
            l_extReasonCode = ERC_BCE_REQUEST_CREATE_FAILURE;
            userdata1 = (uint32_t)(-l_ssxrc);
            break;
        }

        // Do actual copying
        l_ssxrc = bce_request_schedule(&pba_copy);

        if(l_ssxrc != SSX_OK)
        {
            MAIN_TRAC_ERR("read_oppb_params: BCE request schedule failure rc=[%08X]", -l_ssxrc);
            /*
             * @errortype
             * @moduleid    READ_OPPB_PARAMS
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   RC for BCE block-copy engine
             * @userdata4   ERC_BCE_REQUEST_SCHEDULE_FAILURE
             * @devdesc     Failed to read OPPB data by using BCDE
             */
            l_reasonCode = SSX_GENERIC_FAILURE;
            l_extReasonCode = ERC_BCE_REQUEST_SCHEDULE_FAILURE;
            userdata1 = (uint32_t)(-l_ssxrc);
            break;
        }

        if (OPPB_MAGIC_NUMBER_10 != G_oppb.magic.value)
        {
            // The magic number is invalid .. Invalid or corrupt OPPB image header
            MAIN_TRAC_ERR("read_oppb_params: Invalid OPPB magic number: 0x%08X%08X",
                          WORD_HIGH(G_oppb.magic.value), WORD_LOW(G_oppb.magic.value));
            /* @
             * @errortype
             * @moduleid    READ_OPPB_PARAMS
             * @reasoncode  INVALID_MAGIC_NUMBER
             * @userdata1   High order 32 bits of retrieved OPPB magic number
             * @userdata2   Low order 32 bits of retrieved OPPB magic number
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Invalid magic number in OPPB header
             */
            l_reasonCode = INVALID_MAGIC_NUMBER;
            userdata1 = WORD_HIGH(G_oppb.magic.value);
            userdata2 = WORD_LOW(G_oppb.magic.value);
            break;
        }

        MAIN_TRAC_INFO("read_oppb_params: G_oppb.attr.pstates_enabled=0x%02X, wof_enabled=0x%02X, dds_enabled=0x%02X",
                        G_oppb.attr.fields.pstates_enabled, G_oppb.attr.fields.wof_enabled, G_oppb.attr.fields.dds_enabled);

        MAIN_TRAC_INFO("read_oppb_params: G_oppb.operating_points[0]=%4d, %4d, %4d, %4d Mhz",
                        G_oppb.operating_points[0].frequency_mhz,
                        G_oppb.operating_points[1].frequency_mhz,
                        G_oppb.operating_points[2].frequency_mhz,
                        G_oppb.operating_points[3].frequency_mhz);
        MAIN_TRAC_INFO("read_oppb_params: G_oppb.operating_points[4]=%4d, %4d, %4d, %4d Mhz",
                        G_oppb.operating_points[4].frequency_mhz,
                        G_oppb.operating_points[5].frequency_mhz,
                        G_oppb.operating_points[6].frequency_mhz,
                        G_oppb.operating_points[7].frequency_mhz);

        // Validate frequencies
        // frequency_min_khz frequency_max_khz frequency_step_khz pstate_min
        if ((G_oppb.frequency_min_khz == 0) || (G_oppb.frequency_max_khz == 0) ||
            (G_oppb.frequency_step_khz == 0) ||
            (G_oppb.pstate_min == 0) || (G_oppb.pstate_min > 0xFF) ||
            (G_oppb.frequency_min_khz > G_oppb.frequency_max_khz))
        {
            // Frequency data is invalid
            MAIN_TRAC_ERR("read_oppb_params: Invalid frequency data: min[%d], max[%d], step[%d] kHZ, pmin[0x%02X]",
                          G_oppb.frequency_min_khz, G_oppb.frequency_max_khz,
                          G_oppb.frequency_step_khz, G_oppb.pstate_min);
            /* @
             * @errortype
             * @moduleid    READ_OPPB_PARAMS
             * @reasoncode  INVALID_FREQUENCY
             * @userdata1   min / max frequency (MHz)
             * @userdata2   step freq (MHz) / pstate min
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Invalid frequency in OPPB header
             */
            l_reasonCode = INVALID_FREQUENCY;
            userdata1 = ((G_oppb.frequency_min_khz/1000) << 16) | (G_oppb.frequency_max_khz/1000);
            userdata2 = ((G_oppb.frequency_step_khz/1000) << 16) | G_oppb.pstate_min;
            break;
        }

        MAIN_TRAC_IMP("read_oppb_params: PGPE Frequency data: min[%d], max[%d], step[%d] kHz, pState min[0x%02X]",
                          G_oppb.frequency_min_khz, G_oppb.frequency_max_khz,
                          G_oppb.frequency_step_khz, G_oppb.pstate_min);

        // frequency points are valid, now populate our internal frequency table
        populate_sys_mode_freq_table();

        // verify pstate_max_throttle is defined and larger than min
        if (G_oppb.pstate_max_throttle >= G_oppb.pstate_min)
        {
            uint32_t l_steps = 0;
            uint32_t l_max_throt_freq = proc_pstate2freq(G_oppb.pstate_max_throttle, &l_steps);
            MAIN_TRAC_IMP("read_oppb_params:  pstate_max_throttle[0x%02X]/%dkHz(%d steps)  Fmin Pstate[0x%02X]",
                          G_oppb.pstate_max_throttle, l_max_throt_freq, l_steps,
                          G_oppb.pstate_min);
        }
        else
        {
            MAIN_TRAC_ERR("read_oppb_params: invalid pstate_max_throttle[0x%02X] Fmin Pstate[0x%02X]",
                          G_oppb.pstate_max_throttle,
                          G_oppb.pstate_min);
            /* @
             * @errortype
             * @moduleid    READ_OPPB_PARAMS
             * @reasoncode  INVALID_PSTATE
             * @userdata1   max throttle pstate
             * @userdata2   Fmin pstate
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     Invalid max throttle pstate in OPPB header
             */
            l_reasonCode = INVALID_PSTATE;
            userdata1 = G_oppb.pstate_max_throttle;
            userdata2 = G_oppb.pstate_min;
        }

        // Confirm whether we have wof support
        if(!(G_oppb.attr.fields.wof_enabled))
        {
            MAIN_TRAC_INFO("OPPB has WOF disabled.");
            set_clear_wof_disabled( SET,
                                    WOF_RC_OPPB_WOF_DISABLED,
                                    ERC_WOF_OPPB_WOF_DISABLED );
        }
        else
        {
            if(G_oppb.vdd_vret_mv)
            {
                g_amec->static_wof_data.Vdd_vret_p1mv = G_oppb.vdd_vret_mv * 10;
                g_amec->static_wof_data.Vdd_vret_index = get_voltage_index(g_amec->static_wof_data.Vdd_vret_p1mv);
            }
            else
            {
                MAIN_TRAC_ERR("read_oppb_params: vdd_vret_mv is 0!");
                set_clear_wof_disabled( SET,
                                        WOF_RC_OPPB_WOF_DISABLED,
                                        ERC_WOF_OPPB_WOF_DISABLED );
                break;
            }

            g_amec->static_wof_data.altitude_temp_adj_degCpMm = G_oppb.altitude_temp_adj_degCpMm;

            MAIN_TRAC_INFO("OPPB has WOF enabled(%d) Vdd Vret[%d]100uv  Vdd Vret index[%d] altitude_temp_adj_degCpMm[%d] ",
                           G_oppb.attr.fields.wof_enabled,
                           g_amec->static_wof_data.Vdd_vret_p1mv,
                           g_amec->static_wof_data.Vdd_vret_index,
                           g_amec->static_wof_data.altitude_temp_adj_degCpMm);
        }

    } while (0);

    if (l_reasonCode)
    {
        errlHndl_t l_errl = createErrl(READ_OPPB_PARAMS,         //modId
                                       l_reasonCode,             //reasoncode
                                       l_extReasonCode,          //Extended reason code
                                       ERRL_SEV_UNRECOVERABLE,   //Severity
                                       NULL,                     //Trace Buf
                                       DEFAULT_TRACE_SIZE,       //Trace Size
                                       userdata1,                //userdata1
                                       userdata2);               //userdata2

        // Callout firmware
        addCalloutToErrl(l_errl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        REQUEST_RESET(l_errl);

        return FALSE;
    }

    // Used by amec for pcap calculation could use G_oppb.frequency_step_khz
    // in PCAP calculations instead, but using a separate varaible speeds
    // up PCAP related calculations significantly, by eliminating slow
    // division operations.
    G_mhz_per_pstate = G_oppb.frequency_step_khz/1000;

    TRAC_INFO("read_oppb_params: OCC Pstates Parameter Block read successfully");

    // Success
    return TRUE;

} // end read_oppb_params()


/*
 * Function Specification
 *
 * Name: read_hcode_headers
 *
 * Description: Read and save hcode header data
 *
 * End Function Specification
 */
void read_hcode_headers()
{
    do
    {
        CHECKPOINT(READ_HCODE_HEADERS);

        if (read_ppmr_header() == FALSE) break;
        CHECKPOINT(PPMR_IMAGE_HEADER_READ);

        // If there are no configured cores, skip reading these headers
        // since they will no longer be used.
        if(G_present_cores == 0)
        {
            TRAC_INFO("read_hcode_headers: No configured cores detected."
                      " Skipping OPPB, PGPE, and WOF headers");
            set_clear_wof_disabled( SET,
                                    WOF_RC_NO_CONFIGURED_CORES,
                                    ERC_WOF_NO_CONFIGURED_CORES );
            G_proc_pstate_status = PSTATES_DISABLED;
        }
        else
        {
            // Read OCC pstates parameter block
            if (read_oppb_params() == FALSE) break;
            CHECKPOINT(OPPB_IMAGE_HEADER_READ);

            // Read PGPE header file, extract OCC/PGPE Shared SRAM address and size,
            if (read_pgpe_header() == FALSE) break;
            CHECKPOINT(PGPE_IMAGE_HEADER_READ);

            set_clear_wof_disabled( CLEAR,
                                    WOF_RC_NO_CONFIGURED_CORES,
                                    ERC_WOF_NO_CONFIGURED_CORES );
        }

    } while(0);
}

/*
 * Function Specification
 *
 * Name: gpe_reset
 *
 * Description: Force a GPE to start executing instructions at the reset vector
 * Only supports GPE0 and GPE1
 *
 * End Function Specification
 */
void gpe_reset(uint32_t instance_id)
{
#define XCR_CMD_HRESET      0x60000000
#define XCR_CMD_TOGGLE_XSR  0x40000000
#define XCR_CMD_RESUME      0x20000000

    uint32_t l_gpe_sram_addr = GPE0_SRAM_BASE_ADDR;

    if(1 == instance_id)
    {
        l_gpe_sram_addr = GPE1_SRAM_BASE_ADDR;

    }

    out32(GPE_GPENIVPR(instance_id), l_gpe_sram_addr);

    out32(GPE_GPENXIXCR(instance_id), XCR_CMD_HRESET);
    out32(GPE_GPENXIXCR(instance_id), XCR_CMD_TOGGLE_XSR);
    out32(GPE_GPENXIXCR(instance_id), XCR_CMD_TOGGLE_XSR);
    out32(GPE_GPENXIXCR(instance_id), XCR_CMD_RESUME);
}

/*
 * Set up share_gpe_data struct
 */
void set_shared_gpe_data()
{
    uint32_t sram_addr;

    sram_addr = in32(GPE_GPENIVPR(OCCHW_INST_ID_GPE0));
    if(0 != sram_addr)
    {
        sram_addr += GPE_DEBUG_PTR_OFFSET;

        G_shared_gpe_data.gpe0_tb_ptr =
            *((uint32_t *)(sram_addr + PK_TRACE_PTR_OFFSET));
        G_shared_gpe_data.gpe0_tb_sz =
            *((uint32_t *)(sram_addr + PK_TRACE_SIZE_OFFSET));
    }

    sram_addr = in32(GPE_GPENIVPR(OCCHW_INST_ID_GPE1));
    if(0 != sram_addr)
    {
        sram_addr += GPE_DEBUG_PTR_OFFSET;

        G_shared_gpe_data.gpe1_tb_ptr =
            *((uint32_t *)(sram_addr + PK_TRACE_PTR_OFFSET));
        G_shared_gpe_data.gpe1_tb_sz =
            *((uint32_t *)(sram_addr + PK_TRACE_SIZE_OFFSET));
    }
}

/*
 * Function Specification
 *
 * Name: occ_ipc_setup
 *
 * Description: Initialzes IPC (Inter Process Communication) that is used
 *              to communicate with GPEs.
 *              This will also start GPE0 and GPE1.
 * NOTE:  SGPE and PGPE are started prior to the OCC 405 during the IPL.
 *
 * End Function Specification
 */
void occ_ipc_setup()
{
    int         l_rc;
    errlHndl_t  l_err;

    do
    {
        // install our IPC interrupt handler (this disables our cbufs)
        l_rc = ipc_init();
        if(l_rc)
        {
            MAIN_TRAC_ERR("ipc_init failed with rc=0x%08x", l_rc);
            break;
        }

        // enable IPC's
        l_rc = ipc_enable();
        if(l_rc)
        {
            MAIN_TRAC_ERR("ipc_enable failed with rc = 0x%08x", l_rc);
            break;
        }

        MAIN_TRAC_INFO("Calling IPC disable on all GPE's");
        // disable all of the GPE cbufs.  They will enable them once
        // they are ready to communicate.
        ipc_disable(OCCHW_INST_ID_GPE0);
        ipc_disable(OCCHW_INST_ID_GPE1);

        MAIN_TRAC_INFO("IPC initialization completed");

        // start GPE's 0 and 1
        MAIN_TRAC_INFO("Starting GPE0");
        gpe_reset(OCCHW_INST_ID_GPE0);

        MAIN_TRAC_INFO("Starting GPE1");
        gpe_reset(OCCHW_INST_ID_GPE1);

        MAIN_TRAC_INFO("GPE's taken out of reset");

        set_shared_gpe_data();

    }while(0);

    if(l_rc)
    {
        // Log single error for all error cases, just look at trace to see where it failed.
        /* @
         * @moduleid   OCC_IPC_SETUP
         * @reasonCode SSX_GENERIC_FAILURE
         * @severity   ERRL_SEV_UNRECOVERABLE
         * @userdata1  IPC return code
         * @userdata4  OCC_NO_EXTENDED_RC
         * @devdesc    Firmware failure initializing IPC
         */
        l_err = createErrl( OCC_IPC_SETUP,             // i_modId,
                            SSX_GENERIC_FAILURE,       // i_reasonCode,
                            OCC_NO_EXTENDED_RC,
                            ERRL_SEV_UNRECOVERABLE,
                            NULL,                      // tracDesc_t i_trace,
                            DEFAULT_TRACE_SIZE,        //Trace Size
                            l_rc,                      // i_userData1,
                            0);                        // i_userData2

        //Callout firmware
        addCalloutToErrl(l_err,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        commitErrl(&l_err);
    }
}



/*
 * Function Specification
 *
 * Name: hmon_routine
 *
 * Description: Runs various routines that check the health of the OCC
 *
 * End Function Specification
 */
void hmon_routine()
{
    static uint32_t L_critical_phantom_count = 0;
    static uint32_t L_noncritical_phantom_count = 0;
    static bool L_c_phantom_logged = FALSE;
    static bool L_nc_phantom_logged = FALSE;
    bool l_log_phantom_error = FALSE;

    //use MAIN debug traces
    MAIN_DBG("HMON routine processing...");

    //Check if we've had any phantom interrupts
    if(L_critical_phantom_count != G_occ_phantom_critical_count)
    {
        L_critical_phantom_count = G_occ_phantom_critical_count;
        MAIN_TRAC_INFO("hmon_routine: critical phantom irq occurred! count[%d]", L_critical_phantom_count);

        //log a critical phantom error once
        if(!L_c_phantom_logged)
        {
            L_c_phantom_logged = TRUE;
            l_log_phantom_error = TRUE;
        }
    }
    if(L_noncritical_phantom_count != G_occ_phantom_noncritical_count)
    {
        L_noncritical_phantom_count = G_occ_phantom_noncritical_count;
        MAIN_TRAC_INFO("hmon_routine: non-critical phantom irq occurred! count[%d]", L_noncritical_phantom_count);

        //log a non-critical phantom error once
        if(!L_nc_phantom_logged)
        {
            L_nc_phantom_logged = TRUE;
            l_log_phantom_error = TRUE;
        }
    }

    if(l_log_phantom_error)
    {
        /* @
         * @errortype
         * @moduleid    HMON_ROUTINE_MID
         * @reasoncode  INTERNAL_FAILURE
         * @userdata1   critical count
         * @userdata2   non-critical count
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     interrupt with unknown source was detected
         */
        errlHndl_t l_err = createErrl(HMON_ROUTINE_MID,             //modId
                                      INTERNAL_FAILURE,             //reasoncode
                                      OCC_NO_EXTENDED_RC,           //Extended reason code
                                      ERRL_SEV_INFORMATIONAL,       //Severity
                                      NULL,                         //Trace Buf
                                      DEFAULT_TRACE_SIZE,           //Trace Size
                                      L_critical_phantom_count,     //userdata1
                                      L_noncritical_phantom_count); //userdata2
        // Commit Error
        commitErrl(&l_err);
    }

    //if we are in observation, characterization, or activate state, then monitor the processor
    //and VRM Vdd temperatures for timeout conditions
    if( (IS_OCC_STATE_OBSERVATION() || IS_OCC_STATE_ACTIVE() || IS_OCC_STATE_CHARACTERIZATION()) &&
        (!SMGR_is_state_transitioning()) )
    {
        amec_health_check_proc_timeout();
        amec_health_check_vrm_vdd_temp_timeout();
    }

    //if we are in observation, characterization, or active state with memory temperature data
    // being collected then monitor the temperature collections for overtemp and timeout conditions
    if( (IS_OCC_STATE_OBSERVATION() || IS_OCC_STATE_ACTIVE() || IS_OCC_STATE_CHARACTERIZATION()) &&
        (rtl_task_is_runnable(TASK_ID_MEMORY_DATA)) && (!SMGR_is_state_transitioning()) )
    {
        // Check for memory buffer timeout and overtemp errors
        amec_health_check_membuf_timeout();
        amec_health_check_membuf_temp();

        // Check for DIMM timeout and overtemp errors
        amec_health_check_dimm_timeout();
        amec_health_check_dimm_temp();
    }
}



/*
 * Function Specification
 *
 * Name: master_occ_init
 *
 * Description: Master OCC specific initialization.
 *
 * End Function Specification
 */
void master_occ_init()
{

    errlHndl_t l_err = NULL;

    // Only do APSS initialization if APSS is present
    if(G_pwr_reading_type == PWR_READING_TYPE_APSS)
    {
       l_err = initialize_apss();

       if( (NULL != l_err))
       {
           MAIN_TRAC_ERR("master_occ_init: Error initializing APSS");
           // commit & delete. CommitErrl handles NULL error log handle
           REQUEST_RESET(l_err);
       }
    }

    // Reinitialize the PBAX Queues
    dcom_initialize_pbax_queues();
}

/*
 * Function Specification
 *
 * Name: slave_occ_init
 *
 * Description: Slave OCC specific initialization.
 *
 * End Function Specification
 */
void slave_occ_init()
{
    // Init the DPSS oversubscription IRQ handler
    MAIN_DBG("Initializing Oversubscription IRQ...");

    errlHndl_t l_errl = dpss_oversubscription_irq_initialize();

    if( l_errl )
    {
        // Trace and commit error
        MAIN_TRAC_ERR("Initialization of Oversubscription IRQ handler failed");

        // commit log
        commitErrl( &l_errl );
    }
    else
    {
        MAIN_TRAC_INFO("Oversubscription IRQ initialized");
    }

    //Set up doorbell queues
    dcom_initialize_pbax_queues();

    // Run AMEC Slave Init Code
    amec_slave_init();

    // Initialize SMGR State Semaphores
    extern SsxSemaphore G_smgrModeChangeSem;
    ssx_semaphore_create(&G_smgrModeChangeSem, 1, 1);

    // Initialize SMGR Mode Semaphores
    extern SsxSemaphore G_smgrStateChangeSem;
    ssx_semaphore_create(&G_smgrStateChangeSem, 1, 1);
}

/*
 * Function Specification
 *
 * Name: mainThrdTimerCallback
 *
 * Description: Main thread timer to post semaphores handled by main thread
 *
 * End Function Specification
 */
void mainThrdTimerCallback(void * i_argPtr)
{
    int l_rc = SSX_OK;
    do
    {
        // Post health monitor semaphore
        l_rc = ssx_semaphore_post( &G_hmonSem );

        if ( l_rc != SSX_OK )
        {
            MAIN_TRAC_ERR("Failure posting HlTH monitor semaphore: rc: 0x%x", l_rc);
            break;
        }

        MAIN_DBG("posted hmonSem");

        // Post FFDC semaphore
        l_rc = ssx_semaphore_post( &G_ffdcSem );

        if ( l_rc != SSX_OK )
        {
            MAIN_TRAC_ERR("Failure posting FFDC semaphore: rc: 0x%x", l_rc);
            break;
        }

        MAIN_DBG("posted ffdcSem");

    }while(FALSE);

    // create error on failure posting semaphore
    if( l_rc != SSX_OK)
    {
        /* @
         * @errortype
         * @moduleid    MAIN_THRD_TIMER_MID
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   Create hmon and ffdc semaphore rc
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     SSX semaphore related failure
         */

        errlHndl_t l_err = createErrl(MAIN_THRD_TIMER_MID,          //modId
                                      SSX_GENERIC_FAILURE,          //reasoncode
                                      OCC_NO_EXTENDED_RC,           //Extended reason code
                                      ERRL_SEV_UNRECOVERABLE,       //Severity
                                      NULL,                         //Trace Buf
                                      DEFAULT_TRACE_SIZE,           //Trace Size
                                      l_rc,                         //userdata1
                                      0);                           //userdata2

        // Commit Error
        REQUEST_RESET(l_err);
    }
}

/*
 * Function Specification
 *
 * Name: initMainThrdSemAndTimer
 *
 * Description: Helper function to create semaphores handled by main thread. It also
 *              creates and schedules timer used for posting main thread semaphores
 *
 *
 * End Function Specification
 */
void initMainThrdSemAndTimer()
{
    // create the health monitor Semaphore, starting at 0 with a max count of 0
    // NOTE: Max count of 0 is used becuase there is possibility that
    // semaphore can be posted more than once without any semaphore activity
    int l_hmonSemRc = ssx_semaphore_create(&G_hmonSem, 0, 0);
    // create FFDC Semaphore, starting at 0 with a max count of 0
    // NOTE: Max count of 0 is used becuase there is possibility that
    // semaphore can be posted more than once without any semaphore activity
    int l_ffdcSemRc = ssx_semaphore_create(&G_ffdcSem, 0, 0);
    //create main thread timer
    int l_timerRc = ssx_timer_create(&G_mainThrdTimer,mainThrdTimerCallback,0);

    //check for errors creating the timer
    if(l_timerRc == SSX_OK)
    {
        //schedule the timer so that it runs every MAIN_THRD_TIMER_SLICE
        l_timerRc = ssx_timer_schedule(&G_mainThrdTimer,      // Timer
                                       1,                     // time base
                                       MAIN_THRD_TIMER_SLICE);// Timer period
    }
    else
    {
        MAIN_TRAC_ERR("Error creating main thread timer: RC: %d", l_timerRc);
    }

    // Failure creating semaphore or creating/scheduling timer, create
    // and log error.
    if (( l_hmonSemRc != SSX_OK ) ||
        ( l_ffdcSemRc != SSX_OK ) ||
        ( l_timerRc != SSX_OK))
    {
        MAIN_TRAC_ERR("Semaphore/timer create failure: "
                 "hmonSemRc: 0x08%x, ffdcSemRc: 0x%08x, l_timerRc: 0x%08x",
                 -l_hmonSemRc,-l_ffdcSemRc, l_timerRc );

        /* @
         * @errortype
         * @moduleid    MAIN_THRD_SEM_INIT_MID
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   Create health monitor semaphore rc
         * @userdata2   Timer create/schedule rc
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     SSX semaphore related failure
         */

        errlHndl_t l_err = createErrl(MAIN_THRD_SEM_INIT_MID,       //modId
                                      SSX_GENERIC_FAILURE,          //reasoncode
                                      OCC_NO_EXTENDED_RC,           //Extended reason code
                                      ERRL_SEV_UNRECOVERABLE,       //Severity
                                      NULL,                         //Trace Buf
                                      DEFAULT_TRACE_SIZE,           //Trace Size
                                      l_hmonSemRc,                  //userdata1
                                      l_timerRc);                   //userdata2

        CHECKPOINT_FAIL_AND_HALT(l_err);
    }
}

/*
 * Function Specification
 *
 * Name: Main_thread_routine
 *
 * Description: Main thread handling OCC initialization and thernal, health
 *              monitor and FFDC function semaphores
 *
 * End Function Specification
 */
void Main_thread_routine(void *private)
{
    CHECKPOINT(MAIN_THREAD_STARTED);

    MAIN_TRAC_INFO("Main Thread Started ... " );


    // NOTE: At present, we are not planning to use any config data from
    // mainstore. OCC Role will be provided by FSP after FSP communication
    // So instead of doing config_data_init, we will directly use
    // dcom_initialize_roles. If in future design changes, we will make
    // change to use config_data_init at that time.
    // Default role initialization and determine OCC/Chip Id

    dcom_initialize_roles();
    CHECKPOINT(ROLES_INITIALIZED);

    // Sensor Initialization
    // All Master & Slave Sensor are initialized here, it is up to the
    // rest of the firmware if it uses them or not.
    sensor_init_all();
    CHECKPOINT(SENSORS_INITIALIZED);

    //Initialize structures for collecting core data.
    //It needs to run before RTLoop starts, as gpe request initialization
    //needs to be done before task to collect core data starts.
    proc_core_init();
    CHECKPOINT(PROC_CORE_INITIALIZED);

    // Initialize structures for collecting nest dts data.
    // Needs to run before RTL to initialize the gpe request
    nest_dts_init();
    CHECKPOINT(NEST_DTS_INITIALIZED);

    // Run slave OCC init on all OCCs. Master-only initialization will be
    // done after determining actual role. By default all OCCs are slave.
    slave_occ_init();
    CHECKPOINT(SLAVE_OCC_INITIALIZED);

    // Read hcode headers (PPMR, OCC pstate parameter block, PGPE, WOF)
    read_hcode_headers();

    // SIMICS specific initialization
    if (G_simics_environment)
    {
        // TEMP Hack to enable Active State, until PGPE is ready
        G_proc_pstate_status = PSTATES_ENABLED;

        if(G_oppb.frequency_max_khz == 0)
        {
            MAIN_TRAC_INFO("Main_thread_routine: Could not load all hcode_headers\n");
            CHECKPOINT(OPPB_IMAGE_HEADER_READ);

            // Temp hack to Set up Key Globals for use by proc_freq2pstate functions
            G_oppb.frequency_max_khz  = 4322500;
            G_oppb.frequency_min_khz  = 2028250;
            G_oppb.frequency_step_khz = 16667;
            G_oppb.pstate_min         = PMAX +
                ((G_oppb.frequency_max_khz - G_oppb.frequency_min_khz)/G_oppb.frequency_step_khz);

            G_proc_fmax_mhz   = G_oppb.frequency_max_khz / 1000;


            // Set globals used by amec for pcap calculation
            // could have used G_oppb.frequency_step_khz in PCAP calculations
            // instead, but using a separate varaible speeds up PCAP related
            // calculations significantly, by eliminating division operations.
            G_mhz_per_pstate = G_oppb.frequency_step_khz/1000;

            TRAC_INFO("Main_thread_routine: Pstate Key globals initialized to default values");
        }
    }

    // Initialize watchdog timers. This needs to be right before
    // start rtl to make sure timer doesn't timeout. This timer is being
    // reset from the rtl task.

    MAIN_TRAC_INFO("Initializing watchdog timers.");
    initWatchdogTimers();
    CHECKPOINT(WATCHDOG_INITIALIZED);

    // Initialize Real time Loop Timer Interrupt
    rtl_ocb_init();
    CHECKPOINT(RTL_TIMER_INITIALIZED);

    // Initialize semaphores and timer for handling health monitor and
    // FFDC functions.
    initMainThrdSemAndTimer();
    CHECKPOINT(SEMS_AND_TIMERS_INITIALIZED);

    //Initialize the thread scheduler.
    //Other thread initialization is done here so that don't have to handle
    // blocking commnad handler thread as FSP might start communicating
    // through cmd handler thread in middle of the initialization.
    initThreadScheduler();

    int l_ssxrc = SSX_OK;
    // initWatchdogTimers called before will start running the timer but
    // the interrupt handler will just restart the timer until we use this
    // enable switch to actually start the watchdog function.
//    ENABLE_WDOG;


    while (TRUE)
    {
        // Count each loop so the watchdog can tell the main thread is
        // running.
        G_mainThreadLoopCounter++;

        // Flush the loop counter and trace buffers on each loop, this makes
        // debug easier if the cmd interface doesn't respond
        dcache_flush_line(&G_mainThreadLoopCounter);
        dcache_flush(g_trac_inf_buffer, TRACE_BUFFER_SIZE);
        dcache_flush(g_trac_imp_buffer, TRACE_BUFFER_SIZE);
        dcache_flush(g_trac_err_buffer, TRACE_BUFFER_SIZE);

        if( l_ssxrc == SSX_OK)
        {
            // Wait for health monitor semaphore
            l_ssxrc = ssx_semaphore_pend(&G_hmonSem,SSX_WAIT_FOREVER);

            if( l_ssxrc != SSX_OK)
            {
                MAIN_TRAC_ERR("health monitor Semaphore pending failure RC[0x%08X]",
                         -l_ssxrc );
            }
            else
            {
                // call health monitor routine
                hmon_routine();
            }
        }

        if( l_ssxrc == SSX_OK)
        {
            // Wait for FFDC semaphore
            l_ssxrc = ssx_semaphore_pend(&G_ffdcSem,SSX_WAIT_FOREVER);

            if( l_ssxrc != SSX_OK)
            {
                MAIN_TRAC_ERR("FFDC Semaphore pending failure RC[0x%08X]",-l_ssxrc );
            }
            else
            {
                // Only Master OCC will log call home data
                if (OCC_MASTER == G_occ_role)
                {
                    chom_main();
                }
            }
        }

        if( l_ssxrc != SSX_OK)
        {
            /* @
             * @errortype
             * @moduleid    MAIN_THRD_ROUTINE_MID
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   semaphore pending return code
             * @userdata4   OCC_NO_EXTENDED_RC
             * @devdesc     SSX semaphore related failure
             */

            errlHndl_t l_err = createErrl(MAIN_THRD_ROUTINE_MID,        //modId
                                          SSX_GENERIC_FAILURE,          //reasoncode
                                          OCC_NO_EXTENDED_RC,           //Extended reason code
                                          ERRL_SEV_UNRECOVERABLE,       //Severity
                                          NULL,                         //Trace Buf
                                          DEFAULT_TRACE_SIZE,           //Trace Size
                                          -l_ssxrc,                     //userdata1
                                          0);                           //userdata2

            REQUEST_RESET(l_err);
        }

        // Check to make sure that the start_suspend PGPE job did not fail
        if( (G_proc_pstate_status == PSTATES_FAILED) &&
            (FALSE == isSafeStateRequested()) &&
            (CURRENT_STATE() != OCC_STATE_SAFE) &&
            (CURRENT_STATE() != OCC_STATE_STANDBY) &&
            (!SMGR_is_state_transitioning()) )
        {
            MAIN_TRAC_ERR("Pstate start/suspend command failed PGPE RC[0x%04X]", G_ss_pgpe_rc);
            /* @
             * @errortype
             * @moduleid    MAIN_THRD_ROUTINE_MID
             * @reasoncode  PGPE_FAILURE
             * @userdata1   start_suspend rc
             * @userdata2   0
             * @userdata4   ERC_PGPE_START_SUSPEND_FAILURE
             * @devdesc     PGPE returned an error in response to start_suspend
             */
            errlHndl_t l_err = createPgpeErrl(MAIN_THRD_ROUTINE_MID,                  // modId
                                              PGPE_FAILURE,                           // reasoncode
                                              ERC_PGPE_START_SUSPEND_FAILURE,         // Extended reason code
                                              ERRL_SEV_UNRECOVERABLE,                 // Severity
                                              G_ss_pgpe_rc,                           // userdata1
                                              0                                       // userdata2
                                             );

            REQUEST_RESET(l_err);
        }

    } // while loop
}

/*
 * Function Specification
 *
 * Name: main
 *
 * Description: Entry point of the OCC application
 *
 * End Function Specification
 */
int main(int argc, char **argv)
{
    int l_ssxrc = 0;
    int l_ssxrc2 = 0;

    // First, check what environment we are running on (Simics vs. HW).
    check_runtime_environment();

    CHECKPOINT_INIT();
    CHECKPOINT(MAIN_STARTED);

    homer_rc_t l_homerrc = HOMER_SUCCESS;
    homer_rc_t l_homerrc2 = HOMER_SUCCESS;

    imageHdr_t* l_headerPtr = (imageHdr_t*)SRAM_START_ADDRESS_405;

    // Set to whatever is in the header. Frequency will be overwritten
    // below if necessary.
    uint32_t l_tb_freq_hz = (l_headerPtr->occ_frequency * 1000000) / 4;
    uint32_t l_homer_version = 0;

    // Get the homer version
    l_homerrc = homer_hd_map_read_unmap(HOMER_VERSION,
                                        &l_homer_version,
                                        &l_ssxrc);

    // Get proc_pb_frequency from HOMER host data and calculate the timebase
    // frequency for the OCC. Pass the timebase frequency to ssx_initialize.
    // The passed value must be in Hz. The occ 405 runs at 1/4 the proc
    // frequency so the passed value is 1/4 of the proc_pb_frequency from the
    // HOMER, ie. if the MRW says that proc_pb_frequency is 2400 MHz, then
    // pass 600000000 (600MHz)

    // The offset from the start of the HOMER is 0x000C0000, we will need to
    // create a temporary mapping to this section of the HOMER with ppc405_mmu_map
    // (at address 0x800C0000) read the value, convert it, and then unmap.

    // Don't do a version check before reading the OCC freq, it's present in
    // all HOMER versions.
    l_homerrc2 = homer_hd_map_read_unmap(HOMER_OCC_FREQ,
                                        &G_occ_frequency_mhz,
                                        &l_ssxrc2);

    if (((HOMER_SUCCESS == l_homerrc2) || (HOMER_SSX_UNMAP_ERR == l_homerrc2)) &&
        (G_occ_frequency_mhz != 0))
    {
        // Data is in Mhz upon return and needs to be converted to Hz and then
        // quartered.
        l_tb_freq_hz = G_occ_frequency_mhz * (1000000 / 4);
    }
    else
    {
        l_tb_freq_hz = PPC405_TIMEBASE_HZ;
        G_occ_frequency_mhz = (l_tb_freq_hz * 4) / 1000000;
    }

    CHECKPOINT(SSX_STARTING);

    // Initialize SSX Stacks.  This also reinitializes the time base to 0
    ssx_initialize((SsxAddress)G_noncritical_stack,
                   NONCRITICAL_STACK_SIZE,
                   (SsxAddress)G_critical_stack,
                   CRITICAL_STACK_SIZE,
                   0,
                   l_tb_freq_hz);

    // The GPEs are configured to use the OCB_OTBR as a timebase.
    // This counter runs at (OCC freq)/64.  The 405 timebase frequency runs at
    // (OCC freq)/4, so divide this by 16 to get the gpe timebase frequency.
    G_shared_gpe_data.occ_freq_div = l_tb_freq_hz/16;

    CHECKPOINT(SSX_INITIALIZED);
    // TRAC_XXX needs ssx services, traces can only be done after ssx_initialize
    TRAC_init_buffers();

    CHECKPOINT(TRACE_INITIALIZED);

    MAIN_TRAC_INFO("Inside OCC Main");

    check_runtime_environment();
    if (G_simics_environment == FALSE)
    {
        MAIN_TRAC_INFO("Currently not running in Simics environment");
    }
    else
    {
        MAIN_TRAC_INFO("Currently running in Simics environment");
    }

    // Trace what happened before ssx initialization
    MAIN_TRAC_INFO("HOMER accessed, rc=%d, version=0x%08X, ssx_rc=%d",
                   l_homerrc, l_homer_version, l_ssxrc);

    MAIN_TRAC_INFO("HOMER accessed, rc=%d, occ_freq=%u Hz, ssx_rc=%d",
                   l_homerrc2, l_tb_freq_hz, l_ssxrc2);

    // Handle any errors from the version access
    homer_log_access_error(l_homerrc,
                           l_ssxrc,
                           l_homer_version);

    // Handle any errors from the OCC freq access
    homer_log_access_error(l_homerrc2,
                           l_ssxrc2,
                           l_tb_freq_hz);

    // Time to access any initialization data needed from the HOMER (besides the
    // nest frequency which was required above to enable SSX and tracing).
    CHECKPOINT(HOMER_ACCESS_INITS);

    // Get OCC interrupt type from HOMER host data area. This will tell OCC
    // which interrupt to Host it should be using.
    uint32_t l_occ_int_type = 0;
    l_homerrc = homer_hd_map_read_unmap(HOMER_INT_TYPE,
                                        &l_occ_int_type,
                                        &l_ssxrc);

    if ((HOMER_SUCCESS == l_homerrc) || (HOMER_SSX_UNMAP_ERR == l_homerrc))
    {
        G_occ_interrupt_type = (uint8_t) l_occ_int_type;
    }
    else
    {
        // if HOMER host data read fails, assume the FSP communication
        // path as the default
        G_occ_interrupt_type = FSP_SUPPORTED_OCC;
        //G_occ_interrupt_type = PSIHB_INTERRUPT;
    }

    MAIN_TRAC_INFO("HOMER accessed, rc=%d, host interrupt type=%d, ssx_rc=%d",
                   l_homerrc, l_occ_int_type, l_ssxrc);

    // Handle any errors from the interrupt type access
    homer_log_access_error(l_homerrc,
                           l_ssxrc,
                           l_occ_int_type);

    // enable IPC and start GPEs
    CHECKPOINT(INITIALIZING_IPC);

    occ_ipc_setup();

    CHECKPOINT(IPC_INITIALIZED);

    // Create and resume main thread
    int l_rc = createAndResumeThreadHelper(&Main_thread,
                                           Main_thread_routine,
                                           (void *)0,
                                           (SsxAddress)main_thread_stack,
                                           THREAD_STACK_SIZE,
                                           THREAD_PRIORITY_2);


    if( SSX_OK != l_rc)
    {
        MAIN_TRAC_ERR("Failure creating/resuming main thread: rc: 0x%x", -l_rc);
        /* @
         * @errortype
         * @moduleid    MAIN_MID
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   return code
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Firmware internal error creating thread
         */
        errlHndl_t l_err = createErrl(MAIN_MID,                 //modId
                                      SSX_GENERIC_FAILURE,      //reasoncode
                                      OCC_NO_EXTENDED_RC,       //Extended reason code
                                      ERRL_SEV_UNRECOVERABLE,   //Severity
                                      NULL,                     //Trace Buf
                                      DEFAULT_TRACE_SIZE,       //Trace Size
                                      -l_rc,                    //userdata1
                                      0);                       //userdata2
        // Commit Error log
        REQUEST_RESET(l_err);
    }

    // Enter SSX Kernel
    ssx_start_threads();

    return 0;
}
