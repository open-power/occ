/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe_ocmb.c $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2018                        */
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
#include "gpe_membuf.h"
#include "ppe42_scom.h"
#include "pk.h"
#include "ppe42_msr.h"
#include "occhw_pba_common.h"
#include "p9a_misc_scom_addresses.h"
#include "p9a_firmware_registers.h"
#include "ocmb_register_addresses.h"
#include "ocmb_firmware_registers.h"
#include "ocmb_mem_data.h"

const uint32_t AXONE_MCFGPR[OCCHW_N_MC_CHANNEL] =
{
    P9A_MI_0_MCFGPR0,
    P9A_MI_0_MCFGPR1,
    P9A_MI_1_MCFGPR0,
    P9A_MI_1_MCFGPR1,
    P9A_MI_2_MCFGPR0,
    P9A_MI_2_MCFGPR1,
    P9A_MI_3_MCFGPR0,
    P9A_MI_3_MCFGPR1
};

const uint32_t AXONE_MCSYNC[OCCHW_N_MC_PORT] =
{
    P9A_MI_0_MCSYNC,
    P9A_MI_1_MCSYNC,
    P9A_MI_2_MCSYNC,
    P9A_MI_3_MCSYNC
};

// This table was taken from ekb p9a_omi_setup_bars.C
const int NUM_EXT_MASKS = 20;
const uint8_t EXT_MASK_REORDER[][9] =   // Workbook table 7
{
    // B     6   7   8   9   10  11  12  13
    { 0x00, 15, 18, 16, 17, 21, 20, 19, 14  },
    { 0x04, 15, 18, 16, 17, 21, 20, 14, 19  },
    { 0x06, 15, 18, 16, 17, 21, 14, 20, 19  },
    { 0x07, 15, 18, 16, 17, 14, 21, 20, 19  },
    { 0x80, 15, 18, 16, 17, 21, 20, 19, 14  },
    { 0x84, 15, 18, 16, 17, 21, 20, 14, 19  },
    { 0x86, 15, 18, 16, 17, 21, 14, 20, 19  },
    { 0x87, 15, 18, 16, 17, 14, 21, 20, 19  },
    { 0xC0, 15, 21, 17, 18, 20, 19, 14, 16  },
    { 0xC4, 15, 21, 17, 18, 20, 14, 19, 16  },
    { 0xC6, 15, 21, 17, 18, 14, 20, 19, 16  },
    { 0xC7, 15, 14, 17, 18, 21, 20, 19, 16  },
    { 0xE0, 15, 20, 18, 21, 19, 14, 17, 16  },
    { 0xE4, 15, 20, 18, 21, 14, 19, 17, 16  },
    { 0xE6, 15, 14, 18, 21, 20, 19, 17, 16  },
    { 0xE7, 15, 21, 18, 14, 20, 19, 17, 16  },
    { 0xF0, 15, 19, 21, 20, 14, 18, 17, 16  },
    { 0xF4, 15, 14, 21, 20, 19, 18, 17, 16  },
    { 0xF6, 15, 20, 21, 14, 19, 18, 17, 16  },
    { 0xF7, 15, 20, 14, 21, 19, 18, 17, 16  }
};

int inband_scom_setup(MemBufConfiguration_t* i_config,
                            uint32_t i_membuf_instance,
                            uint32_t i_scom_address,
                            uint32_t *o_oci_addr);

int membuf_get_scom(MemBufConfiguration_t* i_config,
                     int i_membuf_instance,
                     uint32_t i_scom_address,
                     uint64_t* o_data);

int membuf_put_scom(MemBufConfiguration_t* i_config,
                     int i_membuf_instance,
                     uint32_t i_scom_address,
                     uint64_t i_data);

void swap_u32(uint32_t * data32)
{
    uint32_t val = *data32;
    val = ((val << 8) & 0xff00ff00) | ((val >> 8) & 0x00ff00ff);
    *data32 = (val << 16) | (val >> 16);
}


int ocmb_check_sensor_cache_enabled(MemBufConfiguration_t * i_config,
                                         int i_instance)
{
    int rc = 0;
    mmio_merrctl_t merrctl;

    rc = membuf_get_scom(i_config,
                         i_instance,
                         MMIO_MERRCTL,
                         &(merrctl.value));
    if(rc)
    {
        PK_TRACE("OCMB MERRCTL getscom failed. rc = %d",
                 (uint16_t)rc);
    }
    else
    {
        //PK_TRACE("1MERRCTL: %08x%08x",
        //         merrctl.words.high_order,
        //         merrctl.words.low_order);
        if(merrctl.fields.snsc_master_enable == SNSC_MASTER_DISABLED)
        {
            // attempted to enable it
            merrctl.fields.snsc_master_enable = SNSC_MASTER_ENABLED;
            rc = membuf_put_scom(i_config,
                                 i_instance,
                                 MMIO_MERRCTL,
                                 merrctl.value);
            if(rc)
            {
                PK_TRACE("OCMB MERRCTL putscom failed. rc = %d",
                         (uint16_t)rc);
            }
            //PK_TRACE("2MERRCTL: %08x%08x",
            //         merrctl.words.high_order,
            //         merrctl.words.low_order);
        }

    }
    return rc;
}

int gpe_ocmb_configuration_create(MemBufConfiguration_t* o_config)
{
    int rc = 0;
    int i = 0;
    int l_rule = 0;
    int l_ext_addr_mask = 0;
    int designated_sync = -1;
    mcfgpr_t mcfgpr;
    pb_mode_t pb_mode;
    uint64_t*   ptr = (uint64_t*)o_config;

    // Prevent unwanted interrupts from scom errors
    // Enable store Gathering, and Icache prefetch for performance.
    const uint32_t orig_msr = mfmsr();
    mtmsr((orig_msr & ~(MSR_SIBRC | MSR_SIBRCA)) | MSR_SEM | MSR_IS1 | MSR_IS2 );
    sync();

    uint32_t l_config = o_config->config; // Save
    //Clear MemBufConfiguration
    for(i = 0; i < sizeof(MemBufConfiguration_t) / 8; ++i)
    {
        *ptr++ = 0ull;
    }

    o_config->configRc = MEMBUF_NOT_CONFIGURED;
    o_config->membuf_type = MEMTYPE_OCMB;
    o_config->config = l_config;

    do
    {
        // Create the PBASLV configuration for collecting sensor data.
        rc = gpe_pba_parms_create(&(o_config->dataParms),
                                  PBA_SLAVE_MEMBUF,
                                  PBA_WRITE_TTYPE_CI_PR_W,
                                  PBA_WRITE_TTYPE_DC,
                                  PBA_READ_TTYPE_CI_PR_RD);

        if (rc)
        {
            rc = MEMBUF_DATA_SETUP_ERROR;
            break;
        }

        // create the PBASLV configuration for doing inband scoms.
        rc = gpe_pba_parms_create(&(o_config->scomParms),
                                  PBA_SLAVE_MEMBUF,
                                  PBA_WRITE_TTYPE_CI_PR_W,
                                  PBA_WRITE_TTYPE_DC,
                                  PBA_READ_TTYPE_CI_PR_RD);
        if (rc)
        {
            rc = MEMBUF_SCOM_SETUP_ERROR;
            break;
        }

        // On Ocmb the mmio bar value has been swizzled
        // Need to un-swizzle
        rc = getscom_abs(P9A_PU_NMMU_MMCQ_PB_MODE_REG, &(pb_mode.value));
        if( rc )
        {
            PK_TRACE("Scom failed on PB_MODE_REG. rc = %d",
                     rc);
            rc = MEMBUF_PBMODE_GETSCOM_FAILURE;
            break;
        }

        l_ext_addr_mask = pb_mode.fields.addr_extension_group_id << 4;
        l_ext_addr_mask |= pb_mode.fields.addr_extension_chip_id;

        // bits [6:13] in mmio bar have been rearranged - look up translation
        // rule.
        for(l_rule = 0; l_rule < NUM_EXT_MASKS; ++l_rule)
        {
            if(EXT_MASK_REORDER[l_rule][0] == l_ext_addr_mask) // found
            {
                break;
            }
        }

        if(l_rule == NUM_EXT_MASKS) // rule not found.
        {
            PK_TRACE("Failed to find match for %x in EXT_MASK_REORDER",
                     l_ext_addr_mask);
            rc = MEMBUF_PMODE_DATA_MISMATCH;
            break;
        }

        // Find all configured MEMBUFs
        for (i = 0; i < OCCHW_N_MC_CHANNEL; ++i)
        {
            uint64_t l_pba_addr = 0;
            uint32_t l_mmio_bar = 0;

            rc = getscom_abs(AXONE_MCFGPR[i], &(mcfgpr.value));

            if( rc )
            {
                rc = 0; // Can't be scommed then ignore this MEMBUF
                continue;
            }

            if(!mcfgpr.fields.mmio_valid)
            {
                continue;  // MEMBUF MMIOBAR not configured, ignore MEMBUF
            }

            l_mmio_bar =
                (uint32_t)(mcfgpr.fields.mmio_group_base_addr) << 1;
            PK_TRACE("Ocmb[%d] MMIO Bar: %08x", i, l_mmio_bar);

            // The mmio bar has been swizzled,  It needs to be unswizzled.
            // Make this easy for PPE 32 bit arch.

            uint32_t l_mask = 0x02000000; //Start bit 6
            uint32_t l_reordered = 0;
            int      l_col;

            for(l_col = 1; l_col < 9; ++l_col)
            {
                if((l_mask & l_mmio_bar) != 0)
                {
                    uint32_t l_setbit = 0x80000000 >>
                        EXT_MASK_REORDER[l_rule][l_col];
                    l_reordered |= l_setbit;
                }

                l_mask >>= 1;
            }
            l_mmio_bar &= 0xfc03ffff;  //mask off bits 6-13
            l_mmio_bar |= (l_reordered << 8);

            l_pba_addr = (uint64_t)(l_mmio_bar) << 32;

            // The 31-bit base-address (inband scom BAR) corresponds
            //  to bits [8:38] of the Power Bus addresss
            l_pba_addr >>= 8;

            PK_TRACE("MMIO Base Address: 0x%08x%08x on ocmb %d",
                     (uint32_t)(l_pba_addr >> 32),
                     (uint32_t)(l_pba_addr),
                     i);

            // If channel is configured then subchannelA must always have an MB.
            // MB on subchannelB is optional - check config passed in from HTMGT
            o_config->baseAddress[2*i] = l_pba_addr;
            if(o_config->config & CHIP_CONFIG_MEMBUF((2*i)+1))
            {
                o_config->baseAddress[(2*i)+1] = l_pba_addr | OCMB_IB_BAR_B_BIT;
            }

            // Add this MC Channel
            o_config->config |= (CHIP_CONFIG_MCS(i));
        }

        if( rc )
        {
            break;
        }

        // Find the designated sync.
        // Find the register that HWPs used to sync the throttle n/m values
        // accross all membufs. Only one register should be setup.
        for (i = 0; i < (OCCHW_N_MC_PORT); ++i)
        {
            uint64_t mcsync;
            rc = getscom_abs(AXONE_MCSYNC[i], &mcsync);
            if (rc)
            {
                PK_TRACE("getscom failed on MCSYNC, rc = %d. The first configured "
                         "MC will be the designated sync",rc);
                rc = 0;
            }
            if (mcsync != 0)
            {
                designated_sync = i;
                // There can only be one sync, so stop searching.
                break;
            }
        }

        if (designated_sync < 0)
        {
            // Leave mcSyncAddr zero.
            PK_TRACE("No designated sync found. Ocmb sync disabled.");
        }
        else
        {
            o_config->mcSyncAddr = AXONE_MCSYNC[designated_sync];
        }

        if(o_config->config != 0)
        {
            rc = configure_pba_bar_for_inband_access(o_config);

            if( rc )
            {
                break;
            }
        }

        if(!rc)
        {
            // Find out which DTS are present
            // Note: The MB sensor cache supports three DTS readings, but
            // what they will represent has not yet been decided 
            OcmbMemData escache;
            MemBufGetMemDataParms_t l_parms;

            l_parms.update = -1; //NONE
            l_parms.data = (uint64_t *)(&escache);

            for(i = 0; i < OCCHW_N_MEMBUF; ++i)
            {
                if( CHIP_CONFIG_MEMBUF(i) & (o_config->config))
                {
                    l_parms.collect = i;
                    rc = get_ocmb_sensorcache(o_config, &l_parms);
                    if( rc )
                    {
                        PK_TRACE("gpe_ocmb_configuration_create failed to"
                                 " get sensorcache for MEMBUF %d, rc = %d.",
                                 i, rc);

                        // Remove this OCMB from the configuration
                        o_config->config &= ~(CHIP_CONFIG_MEMBUF(i));

                        continue; // Thermal sensors not available.
                    }

                    if(escache.status.fields.ubdts0_present)
                    {
                        o_config->dts_config |= CONFIG_UBDTS0(i);
                    }
                    if(escache.status.fields.memdts0_present)
                    {
                        o_config->dts_config |= CONFIG_MEMDTS0(i);
                    }
                    if(escache.status.fields.memdts1_present)
                    {
                        o_config->dts_config |= CONFIG_MEMDTS1(i);
                    }
                    PK_TRACE("Ocmb dts_config: %08x",o_config->dts_config);
                }
            }
        }

    }
    while( 0 );

    o_config->configRc = rc;

    mtmsr(orig_msr);

    return rc;
}

int check_and_reset_mmio_fir(MemBufConfiguration_t* i_config, unsigned int i_membuf)
{
    int rc = 0;
    mmio_mfir_t mfir;

    rc = membuf_get_scom(i_config, i_membuf, MMIO_MFIR, &(mfir.value));
    if(rc)
    {
        PK_TRACE("check_and_reset_mmio_fir mmio scom fail. rc = %d",rc);
    }
    else
    {

        // Check for bits 7,8,9,10
        if(mfir.fields.snsc_both_starts_err ||
           mfir.fields.snsc_mult_seq_perr ||
           mfir.fields.snsc_fsm_perr ||
           mfir.fields.snsc_reg_perr)
        {
            PK_TRACE("ocmb mmio fir: 0x%08x%08x",
                     mfir.words.high_order,
                     mfir.words.low_order);

            rc = MEMBUF_SCACHE_ERROR;

            mfir.value = 0xffffffffffffffffull;
            mfir.fields.snsc_both_starts_err = 0;
            mfir.fields.snsc_mult_seq_perr = 0;
            mfir.fields.snsc_fsm_perr = 0;
            mfir.fields.snsc_reg_perr = 0;
            membuf_put_scom(i_config, i_membuf, MMIO_MFIR_AND, mfir.value);
        }
    }

    return rc;
}

int ocmb_throttle_sync(MemBufConfiguration_t* i_config)
{
    // see
    // https://farm3802.rtp.stglabs.ibm.com/regdb/entire_table.php?db=FIGDB_cb1_25_36_DB&name=MB_SIM.SRQ.MBA_FARB3Q
    // SYNC only needed if OCMB_MBA_FARB3Q bit cfg_nm_change_after_sync is set.
    // HWP programs this.
    // Ocmb may not need to sync as it only has one FARB3Q reg to write.
    uint64_t data;
    int rc = 0;
    do
    {
        // No designated sync addr, therefore Sync not needed.
        if(i_config->mcSyncAddr == 0)
        {
            break;
        }

        rc = getscom_abs(i_config->mcSyncAddr,&data);
        if (rc)
        {
            PK_TRACE("ocmb_throttle_sync: getscom failed. rc = %d",rc);
            break;
        }

        data &= ~MCS_MCSYNC_SYNC_GO;

        rc = putscom_abs(i_config->mcSyncAddr, data);
        if (rc)
        {
            PK_TRACE("ocmb_throttle_sync: reset sync putscom failed. rc = %d",rc);
            break;
        }

        data |= MCS_MCSYNC_SYNC_GO;

        rc = putscom_abs(i_config->mcSyncAddr, data);
        if (rc)
        {
            PK_TRACE("ocmb_throttle_sync: set sync putscom failed. rc = %d",rc);
            break;
        }
    } while (0);

    return rc;
}

void extract_32b(uint32_t oci_addr, uint64_t * i_dest_addr)
{
    typedef union
    {
        uint64_t v;
        struct
        {
            uint32_t h;
            uint32_t l;
        } w;
    } data64_t;

    data64_t data64;
    uint32_t * dest_addr = (uint32_t *)i_dest_addr;
    PkMachineContext        ctx;

    pk_critical_section_enter(&ctx);
    // Invalidate the cache block at oci_addr
    asm volatile ("dcbi 0, %0" : : "r" (oci_addr) : "memory");

    // The first load causes the PBA bridge to fetch the 32 byte sensor cache
    // and place it in the PPE42 data cache. The PPE42 cache must not be
    // invalidated by any other load(s) until all 32 bytes have been read.
    // Note: A store does a store-through and won't invalidate the data cache.

    asm volatile ("lvd %[d],  0(%[a])\n" : [d]"=r"(data64.v) : [a]"b"(oci_addr));
    dest_addr[7] = data64.w.h;
    dest_addr[6] = data64.w.l;

    asm volatile ("lvd %[d],  8(%[a])\n" : [d]"=r"(data64.v) : [a]"b"(oci_addr));
    dest_addr[5] = data64.w.h;
    dest_addr[4] = data64.w.l;

    asm volatile ("lvd %[d], 16(%[a])\n" : [d]"=r"(data64.v) : [a]"b"(oci_addr));
    dest_addr[3] = data64.w.h;
    dest_addr[2] = data64.w.l;

    asm volatile ("lvd %[d], 24(%[a])\n" : [d]"=r"(data64.v) : [a]"b"(oci_addr));
    dest_addr[1] = data64.w.h;
    dest_addr[0] = data64.w.l;
    sync();
    pk_critical_section_exit(&ctx);
}

int get_ocmb_sensorcache(MemBufConfiguration_t* i_config,
                             MemBufGetMemDataParms_t* i_parms)
{
    int rc = 0;
    uint64_t pba_slvctln_save;

    i_parms->error.rc = MEMBUF_GET_MEM_DATA_DIED;

    pbaslvctl_reset(&(i_config->dataParms));
    pba_slvctln_save = pbaslvctl_setup(&(i_config->dataParms));

    // Clear SIB error accumulator bits & mask SIB errors from
    // generating machine checks
    mtmsr((mfmsr() & ~(MSR_SIBRC | MSR_SIBRCA)) | MSR_SEM);

    //collect has the ordinal # of the membuf from which to collect the
    //sensor cache.
    if(i_parms->collect != -1)
    {
        if((i_parms->collect >= OCCHW_N_MEMBUF) ||
           (0 == (CHIP_CONFIG_MEMBUF(i_parms->collect) & (i_config->config))))
        {
            rc = MEMBUF_GET_MEM_DATA_COLLECT_INVALID;
        }
        else
        {
            uint32_t oci_addr = 0;

            int rc1 = check_and_reset_mmio_fir(i_config, i_parms->collect);
            int rc2 = ocmb_check_sensor_cache_enabled(i_config,
                                                          i_parms->collect);

            if(rc2)
            {
                rc = rc2; // mmio scom fail
            }
            else if (rc1)
            {
                rc = rc1; //mmio scom fail or MEMBUF_SCACHE_ERROR;
            }
            else
            {

                // NOTE: inband_scom_setup can be used to map the oci_addr so
                // long as i_config->dataParms and i_config->scomParms are the,
                // same, which for ocmb is currently true.
                rc = inband_scom_setup(i_config,
                                       i_parms->collect,
                                       OCMB_IB_SENSOR_CACHE_ADDR,
                                       &oci_addr);

                if(!rc)
                {
                    uint32_t org_msr = mfmsr();
                    // Mask off SIB errors from gen mck and set data cache enable,
                    // Enable Store Gathering, and Icache prefetch
                    mtmsr(org_msr | MSR_SEM | MSR_IS0 | MSR_IS1 | MSR_IS2);
                    sync();
                    g_inband_access_state = INBAND_ACCESS_IN_PROGRESS;

                    extract_32b(oci_addr,(i_parms->data));           //packet0
                    extract_32b(oci_addr + 32, (i_parms->data) + 4); //packet1

                    // Poll for SIB errors or machine check
                    if((mfmsr() & MSR_SIBRC) ||
                       g_inband_access_state != INBAND_ACCESS_IN_PROGRESS)
                    {
                        // Take membuf out of config list
                        PK_TRACE("Removing Membuf %d from list of configured Membufs",
                                 i_parms->collect);
                        i_config->config &= ~(CHIP_CONFIG_MEMBUF(i_parms->collect));

                        // This rc will cause the 405 to remove this centaur sensor
                        rc = MEMBUF_CHANNEL_CHECKSTOP;
                    }
                    mtmsr(org_msr);
                    g_inband_access_state = INBAND_ACCESS_INACTIVE;

                    // Finish making data big-endian
                    uint32_t * p = (uint32_t *)(i_parms->data);
                    int i;
                    for(i=0; i < 16; ++i)
                    {
                        swap_u32(p);
                        ++p;
                    }
                }
            }
        }
    }
    // if(i_parms->update != -1) {}  -- not used for Ocmb

    pbaslvctl_reset(&(i_config->dataParms));
    PPE_STVD((i_config->dataParms).slvctl_address, pba_slvctln_save);

    i_parms->error.rc = rc;
    return rc;
}


