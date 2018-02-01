/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/main.c $                                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2018                        */
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
//#include "heartbeat.h"
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
#include <centaur_data.h>
#include <dpss.h>
#include <state.h>
#include <amec_sys.h>
#include <cmdh_fsp.h>
#include <proc_pstate.h>
//#include <vrm.h>
#include <chom.h>
#include <homer.h>
#include <amec_health.h>
#include <amec_freq.h>
#include <pss_service_codes.h>
#include <dimm.h>
#include "occhw_shared_data.h"
#include <pgpe_shared.h>
#include <gpe_register_addresses.h>
#include <p9_pstates_occ.h>
#include <wof.h>
#include "pgpe_service_codes.h"
#include <common.h>
#include "p9_memmap_occ_sram.h"

// Used to indicate if OCC was started during IPL, in which case OCC's only
// job is to look for checkstops. This flag is set by hostboot in OCC's header
// in SRAM
bool G_ipl_time = false;

extern uint32_t __ssx_boot; // Function address is 32 bits
extern uint32_t G_occ_phantom_critical_count;
extern uint32_t G_occ_phantom_noncritical_count;
extern uint8_t G_occ_interrupt_type;
extern uint8_t G_occ_role;
extern pstateStatus G_proc_pstate_status;

extern GpeRequest G_meas_start_request;
extern GpeRequest G_meas_cont_request;
extern GpeRequest G_meas_complete_request;
extern apss_start_args_t    G_gpe_start_pwr_meas_read_args;
extern apss_continue_args_t G_gpe_continue_pwr_meas_read_args;
extern apss_complete_args_t G_gpe_complete_pwr_meas_read_args;
extern uint32_t G_present_cores;
extern uint32_t G_proc_fmin_khz;
extern uint32_t G_proc_fmax_khz;
extern wof_header_data_t G_wof_header;

extern uint32_t G_khz_per_pstate;

extern uint8_t G_proc_pmin;
extern uint8_t G_proc_pmax;

extern PWR_READING_TYPE  G_pwr_reading_type;

IMAGE_HEADER (G_mainAppImageHdr,__ssx_boot,MAIN_APP_ID,ID_NUM_INVALID);

ppmr_header_t G_ppmr_header;       // PPMR Header layout format
pgpe_header_data_t G_pgpe_header;  // Selected fields from PGPE Header
OCCPstateParmBlock G_oppb;         // OCC Pstate Parameters Block Structure
extern uint16_t G_proc_fmax_mhz;   // max(turbo,uturbo) frequencies
extern int G_ss_pgpe_rc;

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

// Nest frequency in MHz
uint32_t G_nest_frequency_mhz;

extern uint8_t g_trac_inf_buffer[];
extern uint8_t g_trac_imp_buffer[];
extern uint8_t g_trac_err_buffer[];

void create_tlb_entry(uint32_t address, uint32_t size);

//Macro creates a 'bridge' handler that converts the initial fast-mode to full
//mode interrupt handler


FIR_HEAP_BUFFER(uint8_t G_fir_heap[FIR_HEAP_SECTION_SIZE]);
FIR_PARMS_BUFFER(uint8_t G_fir_data_parms[FIR_PARMS_SECTION_SIZE]);
uint32_t G_fir_master = FIR_OCC_NOT_FIR_MASTER;
bool G_fir_collection_request_created = FALSE;
GpeRequest G_fir_collection_request;

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
    flags = in64(OCB_OCCFLG);

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

        // Same comment as above about lower 32-bits. Bit 62 can be toggled
        // on or off (in Simics) to indicate for which APSS specification the
        // model is configured.
        G_shared_gpe_data.spipss_spec_p9 =
            (0 != (flags & 0x0000000000000002) ) ? FALSE : TRUE;

    }
    else
    {
        G_shared_gpe_data.spipss_spec_p9 = 0;
    }
}

/*
 * Function Specification
 *
 * Name: create_tlb_entry
 *
 * Description: Creates a TLB entry in the 405 processor to access PGPE space.
 *              The TLB entry has read only access, and is cache-inhibited.
 *              This call takes care of pages alignment
 *              and adds additional pages in case the requested
 *              space crosses page boundaries.
 *
 *              will result in panic if the TLB entry allocation
 *              fails.
 *
 * End Function Specification
 */

#define PAGE_SIZE PPC405_PAGE_SIZE_MIN
#define PAGE_ALIGNED_ADDRESS(addr)     (addr & ~((uint32_t)(PAGE_SIZE-1)))
#define PAGE_ALIGNED_SIZE(sz)          ((uint32_t)PAGE_SIZE*(((uint32_t)sz/PAGE_SIZE)+1))

void create_tlb_entry(uint32_t address, uint32_t size)
{
#if PPC405_MMU_SUPPORT
    int      l_rc = SSX_OK;
    uint32_t tlb_entry_address, tlb_entry_size;   // address and size that guarantee page alignment

    tlb_entry_address = PAGE_ALIGNED_ADDRESS(address);

    if ((address + (size%PPC405_PAGE_SIZE_MIN)) >=
        (tlb_entry_address + PPC405_PAGE_SIZE_MIN))
    {
        tlb_entry_size = PAGE_ALIGNED_SIZE(size+PPC405_PAGE_SIZE_MIN);
    }
    else
    {
        tlb_entry_size = PAGE_ALIGNED_SIZE(size);
    }

    // define DTLB for page aligned address and size
    l_rc = ppc405_mmu_map(tlb_entry_address,
                          tlb_entry_address,
                          tlb_entry_size,
                          0,
                          TLBLO_I,         //Read-only, Cache-inhibited
                          NULL);
    if(l_rc == SSX_OK)
    {
        MAIN_TRAC_IMP("Created TLB entry for Address[0x%08x]"
                      "TLB Page Address[0x%08x], TLB Entry size[0x%08x]",
                      address, tlb_entry_address, tlb_entry_size);
    }
    else
    {
        MAIN_TRAC_ERR("Failed to create TLB entry,"
                      "TLB Page Address[0x%08x], TLB Entry size[0x%08x], rc[0x%08x]",
                      tlb_entry_address, tlb_entry_size, l_rc);

        /* @
         * @errortype
         * @moduleid    CREATE_TLB_ENTRY
         * @reasoncode  SSX_GENERIC_FAILURE
         * @userdata1   ppc405_mmu_map return code
         * @userdata2   address for which a TLB entry is created
         * @userdata4   ERC_TLB_ENTRY_CREATION_FAILURE
         * @devdesc     SSX semaphore related failure
         */
        errlHndl_t l_err = createErrl(CREATE_TLB_ENTRY,               //modId
                                      SSX_GENERIC_FAILURE,            //reasoncode
                                      ERC_TLB_ENTRY_CREATION_FAILURE, //Extended reason code
                                      ERRL_SEV_UNRECOVERABLE,         //Severity
                                      NULL,                           //Trace Buf
                                      DEFAULT_TRACE_SIZE,             //Trace Size
                                      l_rc,                           //userdata1
                                      address);                       //userdata2

        REQUEST_RESET(l_err);
    }
#endif
}

/*
 * Function Specification
 *
 * Name: externalize_oppb
 *
 * Description: Saves the relevant content from the G_oppb into g_amec
 *              for WOF calculations.
 *
 * End Function Specification
 */
void externalize_oppb( void )
{

    g_amec->wof.good_quads_per_sort = G_oppb.iddq.good_quads_per_sort;
    g_amec->wof.good_normal_cores_per_sort = G_oppb.iddq.good_normal_cores_per_sort;
    g_amec->wof.good_caches_per_sort = G_oppb.iddq.good_caches_per_sort;

    int i = 0;
    for( i = 0; i < MAXIMUM_QUADS; i++)
    {
        g_amec->wof.good_normal_cores[i] = G_oppb.iddq.good_normal_cores[i];
        g_amec->wof.good_caches[i] = G_oppb.iddq.good_caches[i];
        g_amec->wof.allGoodCoresCachesOn[i] = G_oppb.iddq.ivdd_all_good_cores_on_caches_on[i];
        g_amec->wof.allCoresCachesOff[i] = G_oppb.iddq.ivdd_all_cores_off_caches_off[i];
        g_amec->wof.coresOffCachesOn[i] = G_oppb.iddq.ivdd_all_good_cores_off_good_caches_on[i];
        g_amec->wof.quad1CoresCachesOn[i] = G_oppb.iddq.ivdd_quad_good_cores_on_good_caches_on[0][i];
        g_amec->wof.quad2CoresCachesOn[i] = G_oppb.iddq.ivdd_quad_good_cores_on_good_caches_on[1][i];
        g_amec->wof.quad3CoresCachesOn[i] = G_oppb.iddq.ivdd_quad_good_cores_on_good_caches_on[2][i];
        g_amec->wof.quad4CoresCachesOn[i] = G_oppb.iddq.ivdd_quad_good_cores_on_good_caches_on[3][i];
        g_amec->wof.quad5CoresCachesOn[i] = G_oppb.iddq.ivdd_quad_good_cores_on_good_caches_on[4][i];
        g_amec->wof.quad6CoresCachesOn[i] = G_oppb.iddq.ivdd_quad_good_cores_on_good_caches_on[5][i];
        g_amec->wof.ivdn[i] = G_oppb.iddq.ivdn[i];
        g_amec->wof.allCoresCachesOnT[i] = G_oppb.iddq.avgtemp_all_good_cores_on[i];
        g_amec->wof.allCoresCachesOffT[i] = G_oppb.iddq.avgtemp_all_cores_off_caches_off[i];
        g_amec->wof.coresOffCachesOnT[i] = G_oppb.iddq.avgtemp_all_good_cores_off[i];
        g_amec->wof.quad1CoresCachesOnT[i] = G_oppb.iddq.avgtemp_quad_good_cores_on[0][i];
        g_amec->wof.quad2CoresCachesOnT[i] = G_oppb.iddq.avgtemp_quad_good_cores_on[1][i];
        g_amec->wof.quad3CoresCachesOnT[i] = G_oppb.iddq.avgtemp_quad_good_cores_on[2][i];
        g_amec->wof.quad4CoresCachesOnT[i] = G_oppb.iddq.avgtemp_quad_good_cores_on[3][i];
        g_amec->wof.quad5CoresCachesOnT[i] = G_oppb.iddq.avgtemp_quad_good_cores_on[4][i];
        g_amec->wof.quad6CoresCachesOnT[i] = G_oppb.iddq.avgtemp_quad_good_cores_on[5][i];
        g_amec->wof.avgtemp_vdn[i] = G_oppb.iddq.avgtemp_vdn[i];

    }
}

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

    // Get OCCPstateParmBlock out to Amester
    externalize_oppb();

    // Read active quads address, wof tables address, and wof tables len
    g_amec->wof.req_active_quads_addr   = G_pgpe_header.requested_active_quad_sram_addr;
    g_amec->wof.vfrt_tbls_main_mem_addr = PPMR_ADDRESS_HOMER+WOF_TABLES_OFFSET;
    g_amec->wof.vfrt_tbls_len           = G_pgpe_header.wof_tables_length;
    g_amec->wof.pgpe_wof_state_addr     = G_pgpe_header.wof_state_address;
    g_amec->wof.pstate_tbl_sram_addr    = G_pgpe_header.occ_pstate_table_sram_addr;

    MAIN_TRAC_INFO("read_wof_header() 0x%08X", g_amec->wof.vfrt_tbls_main_mem_addr);

    // Read in quad state addresses here once
    g_amec->wof.quad_state_0_addr = G_pgpe_header.actual_quad_status_sram_addr;
    g_amec->wof.quad_state_1_addr = g_amec->wof.quad_state_0_addr +
                                    sizeof(uint64_t); //skip quad state 0

    // Set some of the fields in the pgpe header struct for next set of calcs
    G_pgpe_header.wof_tables_addr = g_amec->wof.vfrt_tbls_main_mem_addr;

    if (G_pgpe_header.wof_tables_addr != 0 &&
        G_pgpe_header.wof_tables_addr%128 == 0)
    {
        do
        {
            // use block copy engine to read WOF header
            BceRequest l_wof_header_req;

            // Create request
            l_ssxrc = bce_request_create(&l_wof_header_req,              // block copy object
                                         &G_pba_bcde_queue,              // main to sram copy engine
                                         g_amec->wof.vfrt_tbls_main_mem_addr,// mainstore address
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
            memcpy(&G_wof_header,
                   G_temp_bce_buff.data,
                   sizeof(wof_header_data_t));


            // verify the validity of the magic number
            uint32_t magic_number = in32(G_pgpe_header.wof_tables_addr);
            MAIN_TRAC_INFO("read_wof_header() Magic No: 0x%08X", magic_number);
            if(WOF_MAGIC_NUMBER == magic_number)
            {
                // Make sure the header is reporting a valid number of quads i.e. 1 or 6
                if( (G_wof_header.active_quads_size != ACTIVE_QUAD_SZ_MIN) &&
                    (G_wof_header.active_quads_size != ACTIVE_QUAD_SZ_MAX) )
                {
                    MAIN_TRAC_ERR("read_wof_header: Invalid number of active quads!"
                                  " Expected: 1 or 6, Actual %d, WOF disabled",
                                  G_wof_header.active_quads_size );
                    l_error = TRUE;
                    break;
                }
            }
            else
            {
                MAIN_TRAC_ERR("read_wof_header: Invalid WOF Magic number. Address[0x%08X], Magic Number[0x%08X], WOF disabled",
                              G_pgpe_header.wof_tables_addr, magic_number);
                l_error = TRUE;
                break;
            }

            MAIN_TRAC_INFO("MAIN: VFRT block size %d", G_wof_header.vfrt_block_size);
             // Make wof header data visible to amester
            g_amec->wof.version              = G_wof_header.version;
            // TODO: RTC 174543 - Read vfrt blck size from header once correct
            //                    in SRAM
            g_amec->wof.vfrt_block_size      = 256;
            g_amec->wof.vfrt_blck_hdr_sz     = G_wof_header.vfrt_blck_hdr_sz;
            g_amec->wof.vfrt_data_size       = G_wof_header.vfrt_data_size;
            g_amec->wof.active_quads_size    = G_wof_header.active_quads_size;
            g_amec->wof.core_count           = G_wof_header.core_count;
            g_amec->wof.vdn_start            = G_wof_header.vdn_start;
            g_amec->wof.vdn_step             = G_wof_header.vdn_step;
            g_amec->wof.vdn_size             = G_wof_header.vdn_size;
            g_amec->wof.vdd_start            = G_wof_header.vdd_start;
            g_amec->wof.vdd_step             = G_wof_header.vdd_step;
            g_amec->wof.vdd_size             = G_wof_header.vdd_size;
            g_amec->wof.vratio_start         = G_wof_header.vratio_start;
            g_amec->wof.vratio_step          = G_wof_header.vratio_step;
            g_amec->wof.vratio_size          = G_wof_header.vratio_size;
            g_amec->wof.fratio_start         = G_wof_header.fratio_start;
            g_amec->wof.fratio_step          = G_wof_header.fratio_step;
            g_amec->wof.fratio_size          = G_wof_header.fratio_size;
            memcpy(g_amec->wof.vdn_percent, G_wof_header.vdn_percent, 16);
            g_amec->wof.socket_power_w       = G_wof_header.socket_power_w;
            g_amec->wof.nest_freq_mhz        = G_wof_header.nest_freq_mhz;
            g_amec->wof.nom_freq_mhz         = G_wof_header.nom_freq_mhz;
            g_amec->wof.rdp_capacity         = G_wof_header.rdp_capacity;
            g_amec->wof.wof_tbls_src_tag     = G_wof_header.wof_tbls_src_tag;
            g_amec->wof.package_name_hi      = G_wof_header.package_name_hi;
            g_amec->wof.package_name_lo      = G_wof_header.package_name_lo;

            // Initialize wof init state to zero
            g_amec->wof.wof_init_state  = WOF_DISABLED;

        }while( 0 );

        // Check for errors and log, if any
        if ( l_error )
        {
            // We were unable to get the WOF header thus it should not be run.
            set_clear_wof_disabled( SET, WOF_RC_NO_WOF_HEADER_MASK );
        }
    }
    else
    {
        // We were unable to get the WOF header thus it should not be run.
        MAIN_TRAC_ERR("read_wof_header(): WOF header address is 0 or NOT"
                " 128-byte aligned, WOF is disabled");
        set_clear_wof_disabled( SET, WOF_RC_NO_WOF_HEADER_MASK );
    }
} // end read_wof_header()

/*
 * Function Specification
 *
 * Name: read_pgpe_header
 *
 * Description: Initialize PGPE image header entry in DTLB,
 *              Read PGPE image header, lookup shared SRAM address and size,
 *              Initialize OCC/PGPE shared SRAM entry in the DTLB,
 *              Populate global variables, including G_pgpe_peacon_address.
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
    uint64_t magic_number = 0;

    MAIN_TRAC_INFO("read_pgpe_header(0x%08X)", PGPE_HEADER_ADDR);
    do
    {
        // verify the validity of the magic number
        magic_number = in64(PGPE_HEADER_ADDR);
        if (PGPE_MAGIC_NUMBER_10 == magic_number)
        {
            G_pgpe_header.shared_sram_addr = in32(PGPE_HEADER_ADDR + PGPE_SHARED_SRAM_ADDR_OFFSET);
            G_pgpe_header.shared_sram_length = in32(PGPE_HEADER_ADDR + PGPE_SHARED_SRAM_LEN_OFFSET);
            G_pgpe_header.generated_pstate_table_homer_offset = in32(PGPE_HEADER_ADDR + PGPE_GENERATED_PSTATE_TBL_ADDR_OFFSET);
            G_pgpe_header.generated_pstate_table_length = in32(PGPE_HEADER_ADDR + PGPE_GENERATED_PSTATE_TBL_SZ_OFFSET);
            G_pgpe_header.occ_pstate_table_sram_addr = in32(PGPE_HEADER_ADDR + PGPE_OCC_PSTATE_TBL_ADDR_OFFSET);
            G_pgpe_header.occ_pstate_table_length = in32(PGPE_HEADER_ADDR + PGPE_OCC_PSTATE_TBL_SZ_OFFSET);
            G_pgpe_header.beacon_sram_addr = in32(PGPE_HEADER_ADDR + PGPE_BEACON_ADDR_OFFSET);
            G_pgpe_header.actual_quad_status_sram_addr = in32(PGPE_HEADER_ADDR + PGPE_ACTUAL_QUAD_STATUS_ADDR_OFFSET);
            G_pgpe_header.wof_state_address = in32(PGPE_HEADER_ADDR + PGPE_WOF_STATE_ADDR_OFFSET);
            G_pgpe_header.requested_active_quad_sram_addr = in32(PGPE_HEADER_ADDR + PGPE_REQUESTED_ACTIVE_QUAD_ADDR_OFFSET);
            G_pgpe_header.wof_tables_addr = in32(PGPE_HEADER_ADDR + PGPE_WOF_TBLS_ADDR_OFFSET);
            G_pgpe_header.wof_tables_length = in32(PGPE_HEADER_ADDR + PGPE_WOF_TBLS_LEN_OFFSET);

            MAIN_TRAC_IMP("Shared SRAM Address[0x%08x], PGPE Beacon Address[0x%08x]",
                          G_pgpe_header.shared_sram_addr, G_pgpe_header.beacon_sram_addr);
            MAIN_TRAC_IMP("WOF Tables Main Memory Address[0x%08x], Len[0x%08x], "
                          "Req Active Quads Address[0x%08x], "
                          "WOF State address[0x%08x]",
                          G_pgpe_header.wof_tables_addr,
                          G_pgpe_header.wof_tables_length,
                          G_pgpe_header.requested_active_quad_sram_addr,
                          G_pgpe_header.wof_state_address);
            if ((G_pgpe_header.beacon_sram_addr == 0) ||
                (G_pgpe_header.shared_sram_addr == 0))
            {
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
        }
        else
        {
            // The Magic number is invalid .. Invalid or corrupt PGPE image header
            MAIN_TRAC_ERR("read_pgpe_header: Invalid PGPE Magic number. Address[0x%08X], Magic Number[0x%08X%08X]",
                          PGPE_HEADER_ADDR, WORD_HIGH(magic_number), WORD_LOW(magic_number));
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
            userdata1 = WORD_HIGH(magic_number);
            userdata2 = WORD_LOW(magic_number);
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
    // create a DTLB entry for the PPMR image header
    create_tlb_entry(PPMR_ADDRESS_HOMER, sizeof(ppmr_header_t));

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

    MAIN_TRAC_INFO("read_oppb_params(0x%08X)", oppb_address);
    create_tlb_entry(oppb_address, sizeof(OCCPstateParmBlock));

    do{
        // use block copy engine to read the OPPB header
        BceRequest pba_copy;

        // Set up a copy request
        l_ssxrc = bce_request_create(&pba_copy,                           // block copy object
                                     &G_pba_bcde_queue,                   // mainstore to sram copy engine
                                     oppb_address,                        // mainstore address
                                     (uint32_t) &G_oppb,                  // sram starting address
                                     (size_t) sizeof(OCCPstateParmBlock), // size of copy
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

        if (OPPB_MAGIC_NUMBER_10 != G_oppb.magic)
        {
            // The magic number is invalid .. Invalid or corrupt OPPB image header
            MAIN_TRAC_ERR("read_oppb_header: Invalid OPPB magic number: 0x%08X%08X",
                          WORD_HIGH(G_oppb.magic), WORD_LOW(G_oppb.magic));
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
            userdata1 = WORD_HIGH(G_oppb.magic);
            userdata2 = WORD_LOW(G_oppb.magic);
            break;
        }

        // Validate frequencies
        // frequency_min_khz frequency_max_khz frequency_step_khz pstate_min
        if ((G_oppb.frequency_min_khz == 0) || (G_oppb.frequency_max_khz == 0) ||
            (G_oppb.frequency_step_khz == 0) || (G_oppb.pstate_min == 0) ||
            (G_oppb.frequency_min_khz > G_oppb.frequency_max_khz))
        {
            // The magic number is invalid .. Invalid or corrupt OPPB image header
            MAIN_TRAC_ERR("read_oppb_header: Invalid frequency data: min[%d], max[%d], step[%d], pmin[%d] kHz",
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

        MAIN_TRAC_IMP("read_oppb_header: PGPE Frequency data: min[%dkHz], max[%dkHz], step[%dkHz], pState min[%d]",
                          G_oppb.frequency_min_khz, G_oppb.frequency_max_khz,
                          G_oppb.frequency_step_khz, G_oppb.pstate_min);
        // Confirm whether we have wof support
        if(!(G_oppb.wof.wof_enabled))
        {
            MAIN_TRAC_INFO("OPPB has WOF disabled.(%d)",
                    G_oppb.wof.wof_enabled);
            set_clear_wof_disabled( SET, WOF_RC_OPPB_WOF_DISABLED );
        }
        else
        {
            MAIN_TRAC_INFO("OPPB has WOF enabled(%d)",
                    G_oppb.wof.wof_enabled);
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

    // Copy over max frequency into G_proc_fmax_mhz
    G_proc_fmax_mhz   = G_oppb.frequency_max_khz / 1000;


    // Used by amec for pcap calculation could use G_oppb.frequency_step_khz
    // in PCAP calculationsinstead, but using a separate varaible speeds
    // up PCAP relatedcalculations significantly, by eliminating slow
    // division operations.
    G_mhz_per_pstate = G_oppb.frequency_step_khz/1000;

    TRAC_INFO("read_oppb_params: OCC Pstates Parameter Block read successfully");

    // Success
    return TRUE;
}


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
                      " Skipping read_oppb_params(), read_pgpe_header(),"
                      " and read_wof_header()");
            set_clear_wof_disabled( SET, WOF_RC_NO_CONFIGURED_CORES );
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

            // Extract important WOF data into global space
            read_wof_header();
            CHECKPOINT(WOF_IMAGE_HEADER_READ);
            set_clear_wof_disabled( CLEAR, WOF_RC_NO_CONFIGURED_CORES );
        }

        // PGPE Beacon is not implemented in simics
        if (!G_simics_environment)
        {
            // define DTLB for OCC/PGPE shared SRAM, which enables access
            // to OCC-PGPE Shared SRAM space, including pgpe_beacon
            create_tlb_entry(G_pgpe_header.shared_sram_addr, G_pgpe_header.shared_sram_length);
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
        sram_addr += GPE_DEBUG_PTRS_OFFSET;

        G_shared_gpe_data.gpe0_tb_ptr =
            *((uint32_t *)(sram_addr + PK_TRACE_PTR_OFFSET));
        G_shared_gpe_data.gpe0_tb_sz =
            *((uint32_t *)(sram_addr + PK_TRACE_SIZE_OFFSET));
    }

    sram_addr = in32(GPE_GPENIVPR(OCCHW_INST_ID_GPE1));
    if(0 != sram_addr)
    {
        sram_addr += GPE_DEBUG_PTRS_OFFSET;

        G_shared_gpe_data.gpe1_tb_ptr =
            *((uint32_t *)(sram_addr + PK_TRACE_PTR_OFFSET));
        G_shared_gpe_data.gpe1_tb_sz =
            *((uint32_t *)(sram_addr + PK_TRACE_SIZE_OFFSET));
    }

    sram_addr = in32(GPE_GPENIVPR(OCCHW_INST_ID_GPE2));
    if(0 != sram_addr)
    {
        sram_addr += PGPE_DEBUG_PTRS_OFFSET;

        G_shared_gpe_data.pgpe_tb_ptr =
            *((uint32_t *)(sram_addr + PK_TRACE_PTR_OFFSET));
        G_shared_gpe_data.pgpe_tb_sz =
            *((uint32_t *)(sram_addr + PK_TRACE_SIZE_OFFSET));
    }

    sram_addr = in32(GPE_GPENIVPR(OCCHW_INST_ID_GPE3));
    if(0 != sram_addr)
    {
        sram_addr += SGPE_DEBUG_PTRS_OFFSET;

        G_shared_gpe_data.sgpe_tb_ptr =
            *((uint32_t *)(sram_addr + PK_TRACE_PTR_OFFSET));
        G_shared_gpe_data.sgpe_tb_sz =
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

        if (!G_ipl_time)
        {
            set_shared_gpe_data();
        }

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
    if( (IS_OCC_STATE_OBSERVATION() || IS_OCC_STATE_ACTIVE() || IS_OCC_STATE_CHARACTERIZATION()) && (!SMGR_is_state_transitioning()) )
    {
        amec_health_check_proc_timeout();
        amec_health_check_vrm_vdd_temp_timeout();
    }

    //if we are in observation, characterization, or active state with memory temperature data
    // being collected then monitor the temperature collections for overtemp and timeout conditions
    if( (IS_OCC_STATE_OBSERVATION() || IS_OCC_STATE_ACTIVE() || IS_OCC_STATE_CHARACTERIZATION()) &&
       (rtl_task_is_runnable(TASK_ID_DIMM_SM)) && (!SMGR_is_state_transitioning()) )
    {
        // For Cumulus systems only, check for centaur timeout and overtemp errors
        if (MEM_TYPE_CUMULUS ==  G_sysConfigData.mem_type)
        {
            amec_health_check_cent_timeout();
            amec_health_check_cent_temp();
        }

        // For both Nimbus and Cumulus systems, check for rdimm-modules/centaur-dimm
        // timeout and overtemp
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

    if(!G_ipl_time)
    {
        // Sensor Initialization
        // All Master & Slave Sensor are initialized here, it is up to the
        // rest of the firmware if it uses them or not.
        sensor_init_all();
        CHECKPOINT(SENSORS_INITIALIZED);

        // SPIVID Initialization must be done before Pstates
        // All SPIVID inits are done by Hostboot, remove this section.

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
    } //G_ipl_time

    if (G_simics_environment)
    {
        extern pstateStatus G_proc_pstate_status;
        // TEMP Hack to enable Active State, until PGPE is ready
        G_proc_pstate_status = PSTATES_ENABLED;


        // Temp hack to Set up Key Globals for use by proc_freq2pstate functions
        //G_oppb.frequency_max_khz  = 4322500;
        //G_oppb.frequency_min_khz  = 2028250;
        G_oppb.frequency_max_khz  = 2600000;
        G_oppb.frequency_min_khz  = 2000000;
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
    else
    {
        if(!G_ipl_time)
        {
            // Read hcode headers (PPMR, OCC pstate parameter block, PGPE, WOF)
            read_hcode_headers();
        }
    }

    // Initialize watchdog timers. This needs to be right before
    // start rtl to make sure timer doesn't timeout. This timer is being
    // reset from the rtl task.

    MAIN_TRAC_INFO("Initializing watchdog timers.");
    initWatchdogTimers();
    CHECKPOINT(WATCHDOG_INITIALIZED);

    if(!G_ipl_time)
    {
        // Initialize Real time Loop Timer Interrupt
        rtl_ocb_init();
        CHECKPOINT(RTL_TIMER_INITIALIZED);
    }

    // Initialize semaphores and timer for handling health monitor and
    // FFDC functions.
    initMainThrdSemAndTimer();
    CHECKPOINT(SEMS_AND_TIMERS_INITIALIZED);

    if(!G_ipl_time)
    {
        //Initialize the thread scheduler.
        //Other thread initialization is done here so that don't have to handle
        // blocking commnad handler thread as FSP might start communicating
        // through cmd handler thread in middle of the initialization.
        initThreadScheduler();
    }

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


        // This task is not auto-scheduled during IPL, so run it manually
        if(G_ipl_time)
        {
            task_misc_405_checks(NULL);
        }

        static bool L_fir_collection_completed = FALSE;
        uint32_t l_rc = 0;

        // Look for FIR collection flag and status
        if (G_fir_collection_required && !L_fir_collection_completed)
        {
            TRAC_IMP("fir data collection starting");
            // If this OCC is the FIR master and PNOR access is allowed perform
            // FIR collection
            if (OCC_IS_FIR_MASTER())
            {
                //Need to schedule a task on GPE to start fir collection
                if(!G_fir_collection_request_created) //Only need to create request once
                {
                    TRAC_IMP("fir data collection: creating gpe request");
                    l_rc = gpe_request_create(&G_fir_collection_request,//Request
                                              &G_async_gpe_queue0,     //Queue
                                              IPC_ST_FIR_COLLECTION,   //Function ID
                                              NULL,                    //GPE arguments
                                              SSX_SECONDS(5),          //Timeout
                                              NULL,                    //Callback function
                                              NULL,                    //Callback args
                                              ASYNC_REQUEST_BLOCKING); //Options
                    if(l_rc == 0)
                    {
                        G_fir_collection_request_created = TRUE;
                        TRAC_IMP("fir data collection: scheduling gpe request");
                        l_rc = gpe_request_schedule(&G_fir_collection_request);
                        if(l_rc != SUCCESS)
                        {
                            TRAC_IMP("failed to schedule fir data collection"
                                     " job. RC = %x", (uint32_t)l_rc);
                        }

                        L_fir_collection_completed = TRUE;
                        G_fir_collection_required = FALSE;
                    }
                }
            }

            // Error reporting is skipped while FIR collection is required so we
            // don't get reset in flight.  If anyone is listening send the
            // error alert now.
            // If this system is using PSIHB complex, send an interrupt to Host so that
            // Host can inform HTMGT to collect the error log
            if (G_occ_interrupt_type == PSIHB_INTERRUPT)
            {
                notify_host(INTR_REASON_HTMGT_SERVICE_REQUIRED);
            }
            TRAC_IMP("fir data collection done");
        }

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
                if(!G_ipl_time)
                {
                    // call health monitor routine
                    hmon_routine();
                }
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
                if(!G_ipl_time)
                {
                    // Only Master OCC will log call home data
                    if (OCC_MASTER == G_occ_role)
                    {
                        chom_main();
                    }
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
            (CURRENT_STATE() != OCC_STATE_STANDBY) )
        {
            /* @
             * @errortype
             * @moduleid    MAIN_THRD_ROUTINE_MID
             * @reasoncode  PGPE_FAILURE
             * @userdata1   start_suspend rc
             * @userdata2   0
             * @userdata4   ERC_PGPE_START_SUSPEND_FAILURE
             * @devdesc     PGPE returned an error in response to start_suspend
             */
            errlHndl_t l_err = createErrl(
                        MAIN_THRD_ROUTINE_MID,                  // modId
                        PGPE_FAILURE,                           // reasoncode
                        ERC_PGPE_START_SUSPEND_FAILURE,         // Extended reason code
                        ERRL_SEV_UNRECOVERABLE,                 // Severity
                        NULL,                                   // Trace Buf
                        DEFAULT_TRACE_SIZE,                     // Trace Size
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

    // ----------------------------------------------------
    // Initialize TLB for Linear Window access here so we
    // can write checkpoints into the fsp response buffer.
    // ----------------------------------------------------

#if PPC405_MMU_SUPPORT
    l_ssxrc = ppc405_mmu_map(
            OSD_ADDR,
            OSD_ADDR,
//          OSD_ADDR | 0x18000000,
            OSD_TOTAL_SHARED_DATA_BYTES,
            0,
            TLBLO_WR | TLBLO_I,
            NULL
            );

    if(l_ssxrc != SSX_OK)
    {
        //failure means we can't talk to FSP.
        SSX_PANIC(0x01000001);
    }

#if TRAC_TO_SIMICS
    l_ssxrc = ppc405_mmu_map(
            SIMICS_STDIO_BASE,
            SIMICS_STDIO_BASE,
            1024,
            0,
            TLBLO_WR | TLBLO_I,
            NULL
            );

    if(l_ssxrc != SSX_OK)
    {
        //failure means we can't talk to FSP.
        SSX_PANIC(0x01000001);
    }
#endif  /* TRAC_TO_SIMICS */

    l_ssxrc = ppc405_mmu_map(
            CMDH_OCC_RESPONSE_BASE_ADDRESS,
            CMDH_OCC_RESPONSE_BASE_ADDRESS,
            CMDH_FSP_RSP_SIZE,
            0,
            TLBLO_WR | TLBLO_I,
            NULL
            );

    if(l_ssxrc != SSX_OK)
    {
        //failure means we can't talk to FSP.
        SSX_PANIC(0x01000001);
    }

    l_ssxrc = ppc405_mmu_map(
            CMDH_LINEAR_WINDOW_BASE_ADDRESS,
            CMDH_LINEAR_WINDOW_BASE_ADDRESS,
            CMDH_FSP_CMD_SIZE,
            0,
            TLBLO_I,
            NULL
            );

    if(l_ssxrc != SSX_OK)
    {
        //failure means we can't talk to FSP.
        SSX_PANIC(0x01000002);
    }

    l_ssxrc = ppc405_mmu_map(
            TRACE_BUFFERS_START_ADDR,
            TRACE_BUFFERS_START_ADDR,
            ALL_TRACE_BUFFERS_SZ,
            0,
            TLBLO_WR | TLBLO_I,
            NULL
            );

    if(l_ssxrc != SSX_OK)
    {
        //failure means there will be no trace data for debug
        SSX_PANIC(0x01000003);
    }

    // Setup the TLB for writing to the FIR parms section
    l_ssxrc = ppc405_mmu_map(FIR_PARMS_SECTION_BASE_ADDRESS,
                             FIR_PARMS_SECTION_BASE_ADDRESS,
                             FIR_PARMS_SECTION_SIZE,
                             0,
                             TLBLO_WR | TLBLO_I,
                             NULL);

    if (l_ssxrc != SSX_OK)
    {
        // Panic, this section is required for FIR collection on checkstops
        SSX_PANIC(0x01000003);
    }

    // Setup the TLB for writing to the FIR heap section
    l_ssxrc = ppc405_mmu_map(FIR_HEAP_SECTION_BASE_ADDRESS,
                             FIR_HEAP_SECTION_BASE_ADDRESS,
                             FIR_HEAP_SECTION_SIZE,
                             0,
                             TLBLO_WR | TLBLO_I,
                             NULL);

    if (l_ssxrc != SSX_OK)
    {
        // Panic, this section is required for FIR collection on checkstops
        SSX_PANIC(0x01000004);
    }
#endif

    CHECKPOINT_INIT();
    CHECKPOINT(MAIN_STARTED);

    homer_rc_t l_homerrc = HOMER_SUCCESS;
    homer_rc_t l_homerrc2 = HOMER_SUCCESS;

    imageHdr_t* l_headerPtr = (imageHdr_t*)SRAM_START_ADDRESS_405;
    G_ipl_time = l_headerPtr->occ_flags.flag_bits.ipl_time_flag;

    // Set to whatever is in the header. Frequency will be overwritten
    // below if necessary.
    uint32_t l_tb_freq_hz = (l_headerPtr->nest_frequency * 1000000) / 4;
    uint32_t l_homer_version = 0;

    if(!G_ipl_time)
    {
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

        // Don't do a version check before reading the nest freq, it's present in
        // all HOMER versions.
        l_homerrc2 = homer_hd_map_read_unmap(HOMER_NEST_FREQ,
                                            &G_nest_frequency_mhz,
                                            &l_ssxrc2);

        if ((HOMER_SUCCESS == l_homerrc2) || (HOMER_SSX_UNMAP_ERR == l_homerrc2))
        {
            // Data is in Mhz upon return and needs to be converted to Hz and then
            // quartered.
            l_tb_freq_hz = G_nest_frequency_mhz * (1000000 / 4);
        }
        else
        {
            l_tb_freq_hz = PPC405_TIMEBASE_HZ;
            G_nest_frequency_mhz = (l_tb_freq_hz * 4) / 1000000;
        }
    } //!G_ipl_time
    else
    {
        MAIN_TRAC_INFO("freq = %d", l_tb_freq_hz);
        G_nest_frequency_mhz = l_tb_freq_hz;
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
    // This counter runs at (nest freq)/64.  The 405 timebase frequency runs at
    // (nest freq)/4, so divide this by 16 to get the gpe timebase frequency.
    G_shared_gpe_data.nest_freq_div = l_tb_freq_hz/16;

    CHECKPOINT(SSX_INITIALIZED);
    // TRAC_XXX needs ssx services, traces can only be done after ssx_initialize
    TRAC_init_buffers();

    CHECKPOINT(TRACE_INITIALIZED);

    MAIN_TRAC_INFO("Inside OCC Main");

    if(G_ipl_time)
    {
        MAIN_TRAC_INFO("ipl flag is set");
    }
    else
    {
        MAIN_TRAC_INFO("ipl flag is NOT set");
    }

    if (G_simics_environment == FALSE)
    {
        MAIN_TRAC_INFO("Currently not running in Simics environment");
    }
    else
    {
        MAIN_TRAC_INFO("Currently running in Simics environment");
        if(G_shared_gpe_data.spipss_spec_p9)
        {
            MAIN_TRAC_INFO("Using P9 Spec for APSS data gathering");
        }
        else
        {
            MAIN_TRAC_INFO("Using P8 Spec for APSS data gathering");
        }
    }

    if(!G_ipl_time)
    {
        // Trace what happened before ssx initialization
        MAIN_TRAC_INFO("HOMER accessed, rc=%d, version=%d, ssx_rc=%d",
                        l_homerrc, l_homer_version, l_ssxrc);

        MAIN_TRAC_INFO("HOMER accessed, rc=%d, nest_freq=%d, ssx_rc=%d",
                        l_homerrc2, l_tb_freq_hz, l_ssxrc2);

        // Handle any errors from the version access
        homer_log_access_error(l_homerrc,
                               l_ssxrc,
                               l_homer_version);

        // Handle any errors from the nest freq access
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

        // Get the FIR Master indicator
        uint32_t l_fir_master = FIR_OCC_NOT_FIR_MASTER;
        l_homerrc = homer_hd_map_read_unmap(HOMER_FIR_MASTER,
                                            &l_fir_master,
                                            &l_ssxrc);

        if (((HOMER_SUCCESS == l_homerrc) || (HOMER_SSX_UNMAP_ERR == l_homerrc))
            &&
            (FIR_OCC_IS_FIR_MASTER == l_fir_master))
        {
            OCC_SET_FIR_MASTER(FIR_OCC_IS_FIR_MASTER);
        }
        else
        {
            OCC_SET_FIR_MASTER(FIR_OCC_NOT_FIR_MASTER);
        }

        TRAC_IMP("HOMER accessed, rc=%d, FIR master=%d, ssx_rc=%d",
                  l_homerrc, l_fir_master, l_ssxrc);

        // Handle any errors from the FIR master access
        homer_log_access_error(l_homerrc,
                               l_ssxrc,
                               l_fir_master);

        // If this OCC is the FIR master read in the FIR collection parms
        if (OCC_IS_FIR_MASTER())
        {
            // Read the FIR parms buffer
            l_homerrc = homer_hd_map_read_unmap(HOMER_FIR_PARMS,
                                                &G_fir_data_parms[0],
                                                &l_ssxrc);

            TRAC_IMP("HOMER accessed, rc=%d, FIR parms buffer 0x%x, ssx_rc=%d",
                      l_homerrc, &G_fir_data_parms[0], l_ssxrc);

            // Handle any errors from the FIR master access
            homer_log_access_error(l_homerrc,
                                   l_ssxrc,
                                   (uint32_t)&G_fir_data_parms[0]);
        }
    }//G_ipl_time
    else
    {
        // No access to HOMER. Set this occ as FIR master
        TRAC_IMP("I am the FIR master");
        OCC_SET_FIR_MASTER(FIR_OCC_IS_FIR_MASTER);
    }

    //Set the fir_heap and fir_params pointer in the shared buffer
    G_shared_gpe_data.fir_heap_buffer_ptr   = (uint32_t)G_fir_heap;
    G_shared_gpe_data.fir_params_buffer_ptr = (uint32_t)G_fir_data_parms;

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

