/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe_centaur.c $                                  */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017,2019                        */
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
/**
 * @briefcentaur_thermal_access
 */

#include "gpe_membuf.h"
#include "ppe42_scom.h"
#include "pk.h"
#include "p9_misc_scom_addresses.h"
#include "mcs_firmware_registers.h"
#include "pba_firmware_constants.h"
#include "pba_register_addresses.h"
#include "centaur_register_addresses.h"
#include "ppe42_msr.h"
#include "occhw_pba_common.h"

// Power Bus Address bit that configures centaur for HOST/OCC P9=bit(38)
#define PBA_HOST_OCC_CFG 0x0000000002000000ull;


const uint32_t MCFGPR[OCCHW_N_MEMBUF] =
{
    MCS_0_MCRSVDE,
    MCS_0_MCRSVDF,
    MCS_1_MCRSVDE,
    MCS_1_MCRSVDF,
    MCS_2_MCRSVDE,
    MCS_2_MCRSVDF,
    MCS_3_MCRSVDE,
    MCS_3_MCRSVDF
};

const uint32_t MCSYNC[OCCHW_N_MEMBUF/2] =
{
    MCS_0_MCSYNC,
    MCS_1_MCSYNC,
    MCS_2_MCSYNC,
    MCS_3_MCSYNC
};

const uint32_t MCCHIFIR[OCCHW_N_MEMBUF] =
{
    MCP_CHAN0_CHI_FIR,
    MCP_CHAN1_CHI_FIR,
    MCP_CHAN2_CHI_FIR,
    MCP_CHAN3_CHI_FIR,
    MCP_CHAN4_CHI_FIR,
    MCP_CHAN5_CHI_FIR,
    MCP_CHAN6_CHI_FIR,
    MCP_CHAN7_CHI_FIR
};

const uint32_t MCMCICFG1Q[OCCHW_N_MEMBUF] =
{
    MCP_CHAN0_CHI_MCICFG1Q,
    MCP_CHAN1_CHI_MCICFG1Q,
    MCP_CHAN2_CHI_MCICFG1Q,
    MCP_CHAN3_CHI_MCICFG1Q,
    MCP_CHAN4_CHI_MCICFG1Q,
    MCP_CHAN5_CHI_MCICFG1Q,
    MCP_CHAN6_CHI_MCICFG1Q,
    MCP_CHAN7_CHI_MCICFG1Q
};

///////////////////////////////////////////////////////////////
// These are PPE specific PBA routines.
//////////////////////////////////////////////////////////////
int
gpe_pba_parms_create(GpePbaParms* parms,
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
}


////////////////////////////////////////////////
// Centaur specific routines
////////////////////////////////////////////////
int gpe_centaur_configuration_create(MemBufConfiguration_t* o_config)
{
    int rc = 0;
    unsigned int i  = 0;
    mcfgpr_t   mcfgpr;
    uint64_t*   ptr = (uint64_t*)o_config;
    int designated_sync = -1;

    // Prevent unwanted interrupts from scom errors
    const uint32_t orig_msr = mfmsr();
    mtmsr((orig_msr & ~(MSR_SIBRC | MSR_SIBRCA)) | MSR_SEM);

    for(i = 0; i < sizeof(MemBufConfiguration_t) / 8; ++i)
    {
        *ptr++ = 0ull;
    }

    o_config->configRc = MEMBUF_NOT_CONFIGURED;
    o_config->membuf_type = MEMTYPE_CENTAUR;

    do
    {
        // Create the PBASLV configurations for the GPE procedures.
        // The 'dataParms' define the PBASLV setup needed to access the
        // Centaur sensor cache.  The 'scomParms' define the PBASLV setup
        // needed to access the Centaur SCOMs.

        rc = gpe_pba_parms_create(&(o_config->dataParms),
                                  PBA_SLAVE_MEMBUF,
                                  PBA_WRITE_TTYPE_CI_PR_W,
                                  PBA_WRITE_TTYPE_DC,
                                  PBA_READ_TTYPE_CL_RD_NC);

        if (rc)
        {
            rc = MEMBUF_DATA_SETUP_ERROR;
            break;
        }

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

        // Iterate through each MCS on the chip and check configuration.

        // Note that the code uniformly treats SCOM failures of the MCFGPR
        // registers as an unconfigured Centaur. This works both for real
        // hardware,  as well as for our VBU models where some of the "valid"
        // MCS are not in the simulation models.

        for (i = 0; i < OCCHW_N_MEMBUF; ++i)
        {
            // check for channel checkstop
            rc = check_centaur_channel_chkstp(i);
            if (rc)
            {
                // If scom failed OR there is a channel checkstop then
                // Centaur is not usable.
                rc = 0;
                continue;
            }

            // Verify that inband scom has been setup. If not then
            // assume the centaur is either non-existant or not configured.
            // Setup is provided by HWP p9c_set_inband_addr.C
            rc = getscom_abs(MCFGPR[i], &(mcfgpr.value));

            if (rc)
            {
                // ignore if can't be scomed.
                rc = 0;
                continue;
            }

            // If inband scom is not configured then assume the centaur does not exist
            if (!mcfgpr.fields.mcfgprq_valid)
            {
                continue;
            }


            // The 31-bit base-address (inband scom BAR) corresponds to bits [8:38] in the
            // 64-bit PowerBus address.
            // Set the HOST/OCC bit in the address.
            o_config->baseAddress[i] =
                ((uint64_t)(mcfgpr.fields.mcfgprq_base_address) << 25) | PBA_HOST_OCC_CFG;

            PK_TRACE_DBG("Centar[%d] Base Address: %016llx",i,o_config->baseAddress[i]);

            // Add the Centaur to the configuration
            o_config->config |= (CHIP_CONFIG_MCS(i) | CHIP_CONFIG_MEMBUF(i));
        }

        if (rc)
        {
            break;
        }

        // Find the designated sync
        for (i = 0; i < (OCCHW_N_MEMBUF/2); ++i)
        {
            uint64_t mcsync;
            rc = getscom_abs(MCSYNC[i], &mcsync);
            if (rc)
            {
                PK_TRACE("getscom failed on MCSYNC, rc = %d. The first configured MC will be"
                         " the designated sync",rc);
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
            designated_sync = cntlz32(o_config->config << CHIP_CONFIG_MCS_BASE);
            PK_TRACE("No designated sync found, using MCS(%d)",designated_sync);
        }

        o_config->mcSyncAddr = MCSYNC[designated_sync];


        rc = configure_pba_bar_for_inband_access(o_config);
        if( rc )
        {
            break;
        }
        // At this point the structure is initialized well-enough that it can
        // be used by gpe_inband_scom().


        o_config->configRc = 0;

        if (o_config->config == 0)
        {
            break;
        }


        // Get Device ID from each centaur
        membuf_get_scom_vector(o_config,
                               CENTAUR_DEVICE_ID,
                               (uint64_t*)(&(o_config->deviceId[0])));

    }
    while(0);

    o_config->configRc = rc;

    mtmsr(orig_msr);

    return rc;
}

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

        // Do an independent check that every Centaur base address
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

                        PK_TRACE("BAR/Mask (%d) error for MCS/Centaur %d",
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
}



int check_centaur_channel_chkstp(unsigned int i_centaur)
{
    int rc = 0;
    mcchifir_t chifir;
    mcmcicfg_t chicfg;

    do
    {
        rc = getscom_abs(MCCHIFIR[i_centaur], &(chifir.value));
        if (rc)
        {
            PK_TRACE("MCCHIFIR scom failed. rc = %d",rc);
            break;
        }

        if(chifir.fields.fir_dsrc_no_forward_progress ||
           chifir.fields.fir_dmi_channel_fail  ||
           chifir.fields.fir_channel_init_timeout ||
           chifir.fields.fir_channel_interlock_err ||
           chifir.fields.fir_replay_buffer_ue ||
           chifir.fields.fir_replay_buffer_overrun ||
           chifir.fields.fir_df_sm_perr ||
           chifir.fields.fir_cen_checkstop ||
           chifir.fields.fir_dsff_tag_overrun ||
           chifir.fields.fir_dsff_mca_async_cmd_error ||
           chifir.fields.fir_dsff_seq_error ||
           chifir.fields.fir_dsff_timeout)
        {
            PK_TRACE("MCCHIFIR: %08x%08x for channel %d",
                     chifir.words.high_order,
                     chifir.words.low_order,
                     i_centaur);

            rc = getscom_abs(MCMCICFG1Q[i_centaur], &(chicfg.value));
            if (rc)
            {
                PK_TRACE("MCMCICFG scom failed. rc = %d",rc);
                break;
            }

            PK_TRACE("MCMCICFG1Q %08x%08x",
                chicfg.words.high_order,
                chicfg.words.low_order);

            rc = MEMBUF_CHANNEL_CHECKSTOP;
        }
    } while(0);

    return rc;
}

int centaur_throttle_sync(MemBufConfiguration_t* i_config)
{
    uint64_t data;
    int rc = 0;
    do
    {
        rc = getscom_abs(i_config->mcSyncAddr,&data);
        if (rc)
        {
            PK_TRACE("centaur_throttle_sync: getscom failed. rc = %d",rc);
            break;
        }

        data &= ~MCS_MCSYNC_SYNC_GO;

        rc = putscom_abs(i_config->mcSyncAddr, data);
        if (rc)
        {
            PK_TRACE("centaur_throttle_sync: reset sync putscom failed. rc = %d",rc);
            break;
        }

        data |= MCS_MCSYNC_SYNC_GO;

        rc = putscom_abs(i_config->mcSyncAddr, data);
        if (rc)
        {
            PK_TRACE("centaur_throttle_sync: set sync putscom failed. rc = %d",rc);
            break;
        }
    } while (0);

    return rc;
}

int centaur_sensorcache_setup(MemBufConfiguration_t* i_config,
                              uint32_t i_centaur_instance,
                              uint32_t * o_oci_addr)
{
    int rc = 0;
#if defined(__USE_PBASLV__)
    pba_slvctln_t slvctln;
#endif
    uint64_t pb_addr = i_config->baseAddress[i_centaur_instance];

    // bit 38 set OCI master, bits 39,40 Centaur thermal sensors '10'b
    pb_addr |= 0x0000000003000000ull;

#if defined(__USE_PBASLV__)
    PPE_LVD((i_config->dataParms).slvctl_address, slvctln.value);
    slvctln.fields.extaddr = pb_addr >> 27;
    PPE_STVD((i_config->dataParms).slvctl_address, slvctln.value);
#else
    {
        // HW bug workaround - don't use extaddr - use pbabar.
        uint64_t barMsk = 0;

        // Mask SIB from generating mck
        mtmsr(mfmsr() | MSR_SEM);

        // put the PBA in the BAR
        rc = putscom_abs(PBA_BARN(PBA_BAR_MEMBUF), pb_addr);
        if (rc)
        {
            PK_TRACE("centaur_sensorcache_setup: putscom fail on PBABAR,"
                     " rc = %d",rc);
        }
        else
        {
            rc = putscom_abs(PBA_BARMSKN(PBA_BAR_MEMBUF), barMsk);
            if (rc)
            {
                PK_TRACE("centaur_sensrocache_setup: putscom fail on"
                         " PBABARMSK, rc = %d",rc);
            }
        }
    }
#endif
    // make oci address
    *o_oci_addr = (uint32_t)(pb_addr & 0x07ffffffull);

    // PBA space bits[0:1] = '10'  bar select bits[3:4]
    *o_oci_addr |= ((PBA_BAR_MEMBUF | 0x8) << 28);

    return rc;
}


// read centaur data sensor cache
int get_centaur_sensorcache(MemBufConfiguration_t* i_config,
                            MemBufGetMemDataParms_t* i_parms)
{
    int rc = 0;
    uint32_t oci_addr = 0;
    uint64_t pba_slvctln_save;
    uint64_t data64 = 0;

    i_parms->error.rc = MEMBUF_GET_MEM_DATA_DIED;

    pbaslvctl_reset(&(i_config->dataParms));
    pba_slvctln_save = pbaslvctl_setup(&(i_config->dataParms));

    // Clear SIB error accumulator bits & mask SIB errors from
    // generating machine checks
    mtmsr((mfmsr() & ~(MSR_SIBRC | MSR_SIBRCA)) | MSR_SEM);

    if(i_parms->collect != -1)
    {
        if((i_parms->collect >= OCCHW_N_MEMBUF) ||
           (0 == (CHIP_CONFIG_MEMBUF(i_parms->collect) & (i_config->config))))
        {
            rc = MEMBUF_GET_MEM_DATA_COLLECT_INVALID;
        }
        else
        {
            rc = centaur_sensorcache_setup(i_config, i_parms->collect,&oci_addr);

            if(!rc)
            {
                uint32_t org_msr = mfmsr();
                mtmsr(org_msr | MSR_SEM); // Mask off SIB errors from gen mck
                g_inband_access_state = INBAND_ACCESS_IN_PROGRESS;
                // Read 128 bytes from centaur cache
                int i;
                for(i = 0; i < 128; i += 8)
                {
                    PPE_LVDX(oci_addr, i, data64);
                    PPE_STVDX((i_parms->data), i, data64);
                }

                // Poll for SIB errors or machine check
                if((mfmsr() & MSR_SIBRC) ||
                   g_inband_access_state != INBAND_ACCESS_IN_PROGRESS)
                {
                    // Take centaur out of config list
                    PK_TRACE("Removing Membuf %d from list of configured Membufs",
                             i_parms->collect);
                    i_config->config &= ~(CHIP_CONFIG_MEMBUF(i_parms->collect));

                    // This rc will cause the 405 to remove this centaur sensor
                    rc = MEMBUF_CHANNEL_CHECKSTOP;
                }
                mtmsr(org_msr);
                g_inband_access_state = INBAND_ACCESS_INACTIVE;
            }
        }
    }

    if(i_parms->update != -1)
    {
        int update_rc = 0;
        if((i_parms->update >= OCCHW_N_MEMBUF) ||
           (0 == (CHIP_CONFIG_MEMBUF(i_parms->update) & (i_config->config))))
        {
            update_rc = MEMBUF_GET_MEM_DATA_UPDATE_INVALID;
        }
        else
        {
            update_rc = centaur_sensorcache_setup(i_config, i_parms->update,&oci_addr);

            if(!update_rc)
            {
                // Writing a zero to this address tells the centaur to update
                // the sensor cache for the next centaur.
                data64 = 0;
                update_rc = inband_access(i_config,
                                           i_parms->update,
                                           oci_addr,
                                           &data64,
                                           INBAND_ACCESS_WRITE);
            }
        }
        if(!rc && update_rc)
        {
            rc = update_rc;
        }
    }

    pbaslvctl_reset(&(i_config->dataParms));
    PPE_STVD((i_config->dataParms).slvctl_address, pba_slvctln_save);

    if(!rc)
    {
        int instance =  i_parms->collect;
        if(instance == -1)
        {
            instance = i_parms->update;
        }
        if (instance != -1)
        {
            rc = check_centaur_channel_chkstp(instance);
        }
    }

    i_parms->error.rc = rc;
    return rc;
}



