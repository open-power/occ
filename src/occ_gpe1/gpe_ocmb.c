/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe_ocmb.c $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2019                        */
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
#include "mcs_firmware_registers.h"
#include "pba_firmware_registers.h"
#include "pba_firmware_constants.h"
#include "ocmb_register_addresses.h"
#include "ocmb_firmware_registers.h"
#include "ocmb_mem_data.h"

const uint32_t MI_MCFGPR[OCCHW_N_MC_CHANNEL] =
{
    MI_0_MCFGPR0,
    MI_0_MCFGPR1,
    MI_1_MCFGPR0,
    MI_1_MCFGPR1,
    MI_2_MCFGPR0,
    MI_2_MCFGPR1,
    MI_3_MCFGPR0,
    MI_3_MCFGPR1
};

const uint32_t MI_MCSYNC[OCCHW_N_MC_PORT] =
{
    MI_0_MCSYNC,
    MI_1_MCSYNC,
    MI_2_MCSYNC,
    MI_3_MCSYNC
};

const uint32_t MI_DSTLFIR[OCCHW_N_MC_CHANNEL] =
{
    MI_0_DSTLFIR0,
    MI_0_DSTLFIR1,
    MI_1_DSTLFIR0,
    MI_1_DSTLFIR1,
    MI_2_DSTLFIR0,
    MI_2_DSTLFIR1,
    MI_3_DSTLFIR0,
    MI_3_DSTLFIR1
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

int membuf_put_scom_all(MemBufConfiguration_t* i_config,
                         uint32_t i_scom_address,
                         uint64_t i_data);

int check_and_reset_mmio_fir(MemBufConfiguration_t * i_config,unsigned int i_membuf);

int check_channel_fail(int i_membuf);

void swap_u32(uint32_t * data32)
{
    uint32_t val = *data32;
    val = ((val << 8) & 0xff00ff00) | ((val >> 8) & 0x00ff00ff);
    *data32 = (val << 16) | (val >> 16);
}


/**
 * Create PBA slave configuration parameters.
 * @param[in] ptr tor param data area to be filled out.
 * @param[in] PBA slave to use.
 * @param[in] write ttype (@see occhw_pba_common.h)
 * @param[in] write tsize (@see occhw_pba_common.h)
 * @param[in] read_ttype  (@see occhw_pba_common.h)
 * @return [SUCCESS | return code]
 */
int gpe_pba_parms_create(GpePbaParms* parms,
                         int slave,
                         int write_ttype,
                         int write_tsize,
                         int read_ttype)
{
    pba_slvctln_t* slvctl, *mask;
    pba_slvrst_t* slvrst;
    pba_slvrst_t* slvrst_in_progress;
    uint64_t all1 = 0xffffffffffffffffull;

    parms->slave_id = slave;

    slvctl = &(parms->slvctl);
    mask = &(parms->mask);
    slvrst = &(parms->slvrst);
    slvrst_in_progress = &(parms->slvrst_in_progress);

    parms->slvctl_address = PBA_SLVCTLN(slave);

    slvrst->value = 0;
    slvrst->fields.set = PBA_SLVRST_SET(slave);

    slvrst_in_progress->value = 0;
    slvrst_in_progress->fields.in_prog = PBA_SLVRST_IN_PROG(slave);

    slvctl->value = 0;
    mask->value = 0;

    slvctl->fields.enable = 1;
    mask->fields.enable = all1;

    slvctl->fields.mid_match_value = OCI_MASTER_ID_GPE1;
    mask->fields.mid_match_value = all1;

    slvctl->fields.mid_care_mask = all1;
    mask->fields.mid_care_mask = all1;

    slvctl->fields.write_ttype = write_ttype;
    mask->fields.write_ttype = all1;

    slvctl->fields.write_tsize = write_tsize;
    mask->fields.write_tsize = all1;

    slvctl->fields.read_ttype = read_ttype;
    mask->fields.read_ttype = all1;

    slvctl->fields.buf_alloc_a = 1;
    slvctl->fields.buf_alloc_b = 1;
    slvctl->fields.buf_alloc_c = 1;
    slvctl->fields.buf_alloc_w = 1;
    mask->fields.buf_alloc_a = 1;
    mask->fields.buf_alloc_b = 1;
    mask->fields.buf_alloc_c = 1;
    mask->fields.buf_alloc_w = 1;

    if (read_ttype == PBA_READ_TTYPE_CI_PR_RD)
    {

        slvctl->fields.buf_invalidate_ctl = 1;
        mask->fields.buf_invalidate_ctl = all1;

        slvctl->fields.read_prefetch_ctl = PBA_READ_PREFETCH_NONE;
        mask->fields.read_prefetch_ctl = all1;

    }
    else
    {

        slvctl->fields.buf_invalidate_ctl = 0;
        mask->fields.buf_invalidate_ctl = all1;
    }

    mask->value = ~(mask->value);

    return 0;

} // end gpe_pba_parms_create()


/**
 * Configure the PBABAR for inband access
 * @param[in] Configuration information
 */
int configure_pba_bar_for_inband_access(MemBufConfiguration_t * i_config)
{
    uint64_t bar = 0;
    uint64_t barMsk = PBA_BARMSKN_MASK_MASK;
    uint64_t mask = 0;
    int i = 0;
    int rc = 0;

    do
    {
        // Configure the PBA BAR and PBA BARMSK.
        // Set the BARMSK bits such that:
        // -PBA[8:22] are provided by the PBABAR.
        // -PBA[23:36] are provided by the PBASLVCTL ExtrAddr field
        // -PBA[37:43] are provided by the OCI addr[5:11]
        // PBA[44:63] will always come from the OCI addr[12:31]
        // Note: This code should no longer be needed when the BAR/BARMSK is set
        // by PHYP.
        if (i_config->config != 0)
        {

            for (i = 0; i < OCCHW_N_MEMBUF; ++i)
            {
                bar |= i_config->baseAddress[i];
            }

            bar &= ~barMsk;

            PK_TRACE_DBG("PBABAR(%d): %016llx", PBA_BAR_MEMBUF, bar);
            PK_TRACE_DBG("PBABARMSK: %016llx", barMsk);

            rc = putscom_abs(PBA_BARMSKN(PBA_BAR_MEMBUF), barMsk);

            if (rc)
            {
                PK_TRACE_DBG("Unexpected rc = 0x%08x SCOMing PBA_BARMSKN(%d)\n",
                             (uint32_t)rc, PBA_BAR_MEMBUF);
                rc = MEMBUF_BARMSKN_PUTSCOM_FAILURE;
                break;
            }

            rc = putscom_abs(PBA_BARN(PBA_BAR_MEMBUF), bar);
            if (rc)
            {
                PK_TRACE_DBG("Unexpected rc = 0x%08x SCOMing PBA_BARN(%d)\n",
                             (uint32_t)rc, PBA_BAR_MEMBUF);
                rc = MEMBUF_BARN_PUTSCOM_FAILURE;
                break;
            }
        }

        // Do an independent check that every membuf base address
        // can be generated by the combination of the current BAR and
        // BAR Mask, along with the initial requirement that the mask must
        // include at least bits 38:43.

        if (i_config->config != 0)
        {
            rc = getscom_abs(PBA_BARN(PBA_BAR_MEMBUF), &bar);

            if (rc)
            {
                PK_TRACE_DBG("Unexpected rc = 0x%08x SCOMing PBA_BARN(%d)\n",
                             (uint32_t)rc, PBA_BAR_MEMBUF);
                rc = MEMBUF_BARN_GETSCOM_FAILURE;
                break;
            }

            rc = getscom_abs(PBA_BARMSKN(PBA_BAR_MEMBUF), &mask);

            if (rc)
            {
                PK_TRACE_DBG("Unexpected rc = 0x%08x SCOMing PBA_BARMSKN(%d)\n",
                             (uint32_t)rc, PBA_BAR_MEMBUF);
                rc = MEMBUF_BARMSKN_GETSCOM_FAILURE;
                break;
            }

            bar = bar & PBA_BARN_ADDR_MASK;
            mask = mask & PBA_BARMSKN_MASK_MASK;

            if ((mask & 0x0000000003f00000ull) != 0x0000000003f00000ull)
            {

                PK_TRACE("PBA BAR mask (%d) does not cover bits 38:43\n", PBA_BAR_MEMBUF);
                rc = MEMBUF_MASK_ERROR;
                break;
            }

            for (i = 0; i < OCCHW_N_MEMBUF; ++i)
            {
                if (i_config->baseAddress[i] != 0)
                {
                    if ((i_config->baseAddress[i] & ~mask) !=
                        (bar & ~mask))
                    {

                        PK_TRACE("BAR/Mask (%d) error for MCS/membuf %d",
                                 PBA_BAR_MEMBUF, i);

                        PK_TRACE("    base = 0x%08x%08x",
                                 (uint32_t)(i_config->baseAddress[i]>>32),
                                 (uint32_t)(i_config->baseAddress[i]));

                        PK_TRACE("    bar  = 0x%08x%08x"
                                 "    mask = 0x%08x%08x",
                                 (uint32_t)(bar >> 32),
                                 (uint32_t)(bar),
                                 (uint32_t)(mask >> 32),
                                 (uint32_t)(mask));

                        rc = MEMBUF_BAR_MASK_ERROR;
                        break;
                    }
                }
            }
        }
    } while(0);
    return rc;

} // end configure_pba_bar_for_inband_access()


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
    int designated_sync = -1;
    mcfgpr_t mcfgpr;
    uint64_t*   ptr64 = (uint64_t*)o_config;
    ocmb_therm_t sensor;
    int fail_count = 0;

    // Prevent unwanted interrupts from scom errors
    // Enable store Gathering, and Icache prefetch for performance.
    const uint32_t orig_msr = mfmsr();
    mtmsr((orig_msr & ~(MSR_SIBRC | MSR_SIBRCA)) | MSR_SEM | MSR_IS1 | MSR_IS2 );
    sync();

    uint32_t l_config = o_config->config;
    for(i = 0; i < sizeof(MemBufConfiguration_t) / 8; ++i)
    {
        *ptr64++ = 0ull;
    }
    barrier();   //Needed to prevent compiler optimizing out restore of l_config

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

        pbaslvctl_reset(&(o_config->scomParms));
        pbaslvctl_setup(&(o_config->scomParms));
        // Find all configured MEMBUFs
        for (i = 0; i < OCCHW_N_MC_CHANNEL; ++i)
        {
            uint64_t l_pba_addr = 0;
            uint32_t l_mmio_bar = 0;

            rc = getscom_abs(MI_MCFGPR[i], &(mcfgpr.value));

            if( rc )
            {
                rc = 0; // Can't be scommed then ignore this MEMBUF
                continue;
            }

            if(!mcfgpr.fields.mmio_valid)
            {
                continue;  // MEMBUF MMIOBAR not configured, ignore MEMBUFs
            }

            if(check_channel_fail(i*OCCWH_MEMBUF_PER_CHANNEL) != 0)
            {
                continue; // Channel is in fail state - ignore
            }

            l_mmio_bar =
                (uint32_t)(mcfgpr.fields.mmio_group_base_addr) << 1;
            PK_TRACE("Ocmb[%d] MMIO Bar: %08x", i, l_mmio_bar);

            l_pba_addr = (uint64_t)(l_mmio_bar) << 32;

            // The 31-bit base-address (inband scom BAR) corresponds
            //  to bits [8:38] of the Power Bus addresss
            l_pba_addr >>= 8;

            o_config->baseAddress[2*i] = l_pba_addr;
            o_config->baseAddress[(2*i)+1] = l_pba_addr | OCMB_IB_BAR_B_BIT;

            // Add this MC channel to the configuration
            o_config->config |= CHIP_CONFIG_MCS(i);
        }

        // Find the designated sync.
        // Find the register that HWPs used to sync the throttle n/m values
        // accross all membufs. Only one register should be setup.
        for (i = 0; i < (OCCHW_N_MC_PORT); ++i)
        {
            uint64_t mcsync;
            rc = getscom_abs(MI_MCSYNC[i], &mcsync);
            if (rc)
            {
                // MCPORT is not enabled
                rc = 0;
                continue;
            }

            // MCS_MCSYNC_EN_SYNC_IN should be set on all non-designated ports
            if ((mcsync & MCS_MCSYNC_EN_SYNC_IN) == 0)
            {
                designated_sync = i;
                // There can only be one designated sync, so stop searching.
                break;
            }
        }

        if (designated_sync < 0)
        {
            // Leave mcSyncAddr zero.
            PK_TRACE("gpe_ocmb_configuration_create: No designated sync found. Ocmb mem throttling disabled.");
        }
        else
        {
            PK_TRACE("gpe_ocmb_configuration_create: designate_sync MI_MCSYNC[%d]",
                     designated_sync);
            o_config->mcSyncAddr = MI_MCSYNC[designated_sync];
        }

        if(o_config->config != 0)
        {
            rc = configure_pba_bar_for_inband_access(o_config);

            if( rc )
            {
                break;
            }
        }

        // Find out which DTS are present
        for(i = 0; i < OCCHW_N_MEMBUF; ++i)
        {
            if( o_config->baseAddress[i] != 0 )
            {
                fail_count = 0;
                rc = membuf_get_scom(o_config, i, MMIO_OCTHERM, &(sensor.value));
                if(rc)
                {
                    PK_TRACE("gpe_ocmb_configuration_create failed to read"
                             " MMIO_OCTHERM for OCMB %d. rc = %d",
                             i, rc);
                    ++fail_count;
                }
                else
                {
                    if(sensor.fields.present)
                    {
                        o_config->dts_config |= CONFIG_UBDTS0(i);
                    }
                }

                rc = membuf_get_scom(o_config, i, MMIO_D0THERM, &(sensor.value));
                if(rc)
                {
                    PK_TRACE("gpe_ocmb_configuration_create failed to read"
                             " MMIO_D0THERM for OCMB %d. rc = %d",
                             i, rc);
                    ++fail_count;
                }
                else
                {
                    if(sensor.fields.present)
                    {
                        o_config->dts_config |= CONFIG_MEMDTS0(i);
                    }
                }

                rc = membuf_get_scom(o_config, i, MMIO_D1THERM, &(sensor.value));
                if(rc)
                {
                    PK_TRACE("gpe_ocmb_configuration_create failed to read"
                             " MMIO_D1THERM for OCMB %d. rc = %d",
                             i, rc);
                    ++fail_count;
                }
                else
                {
                    if(sensor.fields.present)
                    {
                        o_config->dts_config |= CONFIG_MEMDTS1(i);
                    }
                }

                if(fail_count == 3)
                {
                    // This OCMB is not configured correctly. Remove it.
                    o_config->config &= ~(CHIP_CONFIG_MEMBUF(i));
                    o_config->baseAddress[i] = 0;
                }
                rc = 0; // error not terminal. The 405 will notice any missing sensors.
            }
        }

        PK_TRACE("OCMB dts_config: %08x%08x",
                 (uint32_t)(o_config->dts_config >> 32),
                 (uint32_t)(o_config->dts_config));
        PK_TRACE("OCMB_config: %08x",o_config->config);
    }
    while( 0 );

    o_config->configRc = rc;

    mtmsr(orig_msr);

    return rc;
}

/**
 * Check for OCMBr sensor errors
 * @param[in] Configuration information
 * @param[in] The ordinal membuf number
 * @return [0 | return code]
 */
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

int ocmb_throttle_sync(MemBufConfiguration_t* i_config, uint32_t i_sync_type)
{
    // see
    // https://farm3802.rtp.stglabs.ibm.com/regdb/entire_table.php?db=FIGDB_cb1_25_36_DB&name=MB_SIM.SRQ.MBA_FARB3Q
    // SYNC only needed if OCMB_MBA_FARB3Q bit cfg_nm_change_after_sync is set.
    // HWP programs this.
    uint64_t data;
    int rc = 0;
    int i;

    do
    {
        // No designated sync addr, therefore Sync not enabled.
        if(i_config->mcSyncAddr == 0)
        {
            break;
        }

        // Setup non-designated sync regs
        for(i = 0; i < (OCCHW_N_MC_PORT); ++i)
        {
            // If ether MCS channel is configured then port is configured
            uint32_t port_config = CHIP_CONFIG_MCS(i*2) | CHIP_CONFIG_MCS((i*2)+1);
            if((i_config->config & port_config) &&
               (MI_MCSYNC[i] != i_config->mcSyncAddr))
            {
                rc = getscom_abs(MI_MCSYNC[i],&data);
                if (rc)
                {
                    PK_TRACE("ocmb_throttle_sync: getscom failed. rc = %d. scom[0x%08x]",
                             rc,
                             MI_MCSYNC[i]);
                    break;
                }

                data &= ~(MCS_MCSYNC_SYNC_TYPE_FIELD);
                data |= ((uint64_t)i_sync_type) << 32;

                rc = putscom_abs(MI_MCSYNC[i], data);
                if (rc)
                {
                    PK_TRACE("ocmb_throttle_sync: putscom failed. rc = %d. scom[0x%08x]",
                             rc,
                             MI_MCSYNC[i]);
                    break;
                }
            }
        }
        if (rc) break;

        // setup designated sync and toggle go
        rc = getscom_abs(i_config->mcSyncAddr,&data);
        if (rc)
        {
            PK_TRACE("ocmb_throttle_sync: getscom failed. rc = %d. scom[0x%08x]",
                     rc,
                     i_config->mcSyncAddr);
            break;
        }
        data &= ~(MCS_MCSYNC_SYNC_GO|MCS_MCSYNC_SYNC_TYPE_FIELD);
        data |= ((uint64_t)i_sync_type) << 32;

        rc = putscom_abs(i_config->mcSyncAddr, data);
        if (rc)
        {
            PK_TRACE("ocmb_throttle_sync: reset sync putscom failed. rc = %d. scom[0x%08x]",
                     rc,
                     i_config->mcSyncAddr);
            break;
        }

        data |= MCS_MCSYNC_SYNC_GO;

        rc = putscom_abs(i_config->mcSyncAddr, data);
        if (rc)
        {
            PK_TRACE("ocmb_throttle_sync: putscom failed. rc = %d. scom[0x%08x]",
                     rc,
                     i_config->mcSyncAddr);
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
           (0 == i_config->baseAddress[i_parms->collect]))
        {
            rc = MEMBUF_GET_MEM_DATA_COLLECT_INVALID;
        }
        else if(check_channel_fail(i_parms->collect) != 0)
        {
            // Remove the membuf from the config
            i_config->config &= ~(CHIP_CONFIG_MEMBUF(i_parms->collect));
            rc =  MEMBUF_CHANNEL_CHECKSTOP;
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

                        // This rc will cause the 405 to remove this membuf sensor
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
    pbaslvctl_reset(&(i_config->dataParms));
    PPE_STVD((i_config->dataParms).slvctl_address, pba_slvctln_save);

    if(i_parms->touch != 0)
    {
        // Reset OCMB deadman timer.
        ocmb_throttle_sync(i_config, MCS_MCSYNC_SYNC_TYPE_OCC_TOUCH);
    }


    i_parms->error.rc = rc;
    return rc;
}


int gpe_ocmb_init(MemBufConfiguration_t * i_config)
{
    uint64_t data64 = 0;
    int instance = 0;
    // Issue occ touch sync (resets deadman timer count)
    // Any errors will already be traced
    PK_TRACE("gpe_ocmb_init: Issue OCCO_TOUCH");
    ocmb_throttle_sync(i_config, MCS_MCSYNC_SYNC_TYPE_OCC_TOUCH);

    // Clear emergency trottle
    PK_TRACE("gpe_ocmb_init: Clear emergency throttle");
    membuf_put_scom_all(i_config, OCMB_MBA_FARB7Q, 0);

    // Clear safe refresh mode
    PK_TRACE("gpe_ocmb_init: Clear safe refresh mode");
    membuf_put_scom_all(i_config, OCMB_MBA_FARB8Q, 0);

    // verify clear and retry as needed.
    for(instance = 0; instance < OCCHW_N_MEMBUF; ++instance)
    {
        if(0 != ( CHIP_CONFIG_MEMBUF(instance) & (i_config->config)))
        {
            membuf_get_scom(i_config, instance, OCMB_MBA_FARB7Q, &data64);
            if(0 != (data64 & 0x8000000000000000ull))
            {
                PK_TRACE("gpe_ocmb_init: Retry clear emergency throttle");
                data64 = 0;
                membuf_put_scom(i_config, instance, OCMB_MBA_FARB7Q, data64);
            }

            membuf_get_scom(i_config, instance, OCMB_MBA_FARB8Q, &data64);
            if(0 != (data64 & 0x8000000000000000ull))
            {
                PK_TRACE("gpe_ocmb_init: Retry clear safe refresh");
                data64 = 0;
                membuf_put_scom(i_config, instance, OCMB_MBA_FARB8Q, data64);
            }
        }
    }

    // Any errors sould have been traced
    return 0;
}

int check_channel_fail(int i_membuf)
{
    int rc = 0;
    int i_channel = i_membuf/(OCCHW_N_MEMBUF/OCCHW_N_MC_CHANNEL);
    dstlfir_t fir;

    rc = getscom_abs(MI_DSTLFIR[i_channel],&fir.value);
    if ( rc )
    {
        PK_TRACE("check_channel_fail: Scom read failed addr[%08x], rc[%d]",
                 MI_DSTLFIR[i_channel],
                 rc);
        // If the scom failed the count it as a channel checkstop though the
        // entire MI has probably failed or is offline
        rc = MEMBUF_CHANNEL_CHECKSTOP;
    }
    else
    {
        // Each membuf on the channel is connected to it's own sub-channel
        if((i_membuf & 0x1) == 0)// even
        {
            if(fir.fields.sub_channel_a_fail != 0)
            {
                rc = MEMBUF_CHANNEL_CHECKSTOP;
            }
        }
        else // odd
        {
            if(fir.fields.sub_channel_b_fail != 0)
            {
                rc = MEMBUF_CHANNEL_CHECKSTOP;
            }
        }
    }

    return rc;
}

