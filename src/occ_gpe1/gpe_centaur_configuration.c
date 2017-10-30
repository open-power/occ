/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p9/procedures/lib/pm/centaur_thermal_access.c $         */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* EKB Project                                                            */
/*                                                                        */
/* COPYRIGHT 2017                                                         */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* The source code for this program is not published or otherwise         */
/* divested of its trade secrets, irrespective of what has been           */
/* deposited with the U.S. Copyright Office.                              */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
/**
 * @briefcentaur_thermal_access
 */

#include "gpe_centaur.h"
#include "ppe42_scom.h"
#include "pk.h"
#include "p9_misc_scom_addresses.h"
#include "mcs_firmware_registers.h"
#include "pba_firmware_constants.h"
#include "pba_register_addresses.h"
#include "centaur_register_addresses.h"
#include "ppe42_msr.h"
#include "occhw_pba_common.h"

// Which GPE controls the PBASLAVE
#define OCI_MASTER_ID_GPE1 1

// Power Bus Address bit that configures centaur for HOST/OCC P9=bit(38)
#define PBA_HOST_OCC_CFG 0x0000000002000000ull;


const uint32_t MCFGPR[OCCHW_NCENTAUR] =
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

//const uint32_t MCSMODE0[OCCHW_NCENTAUR / 2] __attribute__((section(".sdata2"))) =
//{
//    MCS_0_MCMODE0,
//    MCS_1_MCMODE0,
//    MCS_2_MCMODE0,
//    MCS_3_MCMODE0
//};

//const uint32_t MCFIR[OCCHW_NCENTAUR / 2] __attribute((section(".sdata2"))) =
//{
//    MCS_0_MCFIR,
//    MCS_1_MCFIR,
//    MCS_2_MCFIR,
//    MCS_3_MCFIR
//};

const uint32_t MCFGP[OCCHW_NCENTAUR/2] =
{
    MCS_0_MCFGP,
    MCS_1_MCFGP,
    MCS_2_MCFGP,
    MCS_3_MCFGP
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
int gpe_centaur_configuration_create(CentaurConfiguration_t* o_config)
{
    int rc = 0;
    unsigned int i  = 0;
    mcfgpr_t   mcfgpr;
    // mcifir_t   mcifir;
    // mcsmode0_t mcsmode0;
    pba_slvctln_t slvctl;
    int         designatedSync = -1;
    uint64_t    bar = 0;
    uint64_t    mask = 0;
    uint64_t    base = 0;
    uint64_t*   ptr = (uint64_t*)o_config;

    // Prevent unwanted interrupts from scom errors
    const uint32_t orig_msr = mfmsr() & MSR_SEM;
    mtmsr((orig_msr & ~(MSR_SIBRC | MSR_SIBRCA)) | MSR_SEM);

    for(i = 0; i < sizeof(CentaurConfiguration_t) / 8; ++i)
    {
        *ptr++ = 0ull;
    }

    o_config->configRc = CENTAUR_NOT_CONFIGURED;

    do
    {
        // Create the setups for the GPE procedures. The 'dataParms' are the
        // setup for accessing the Centaur sensor cache.  The 'scomParms' are
        // the setup for accessing Centaur SCOMs.

        rc = gpe_pba_parms_create(&(o_config->dataParms),
                                  PBA_SLAVE_CENTAUR,
                                  PBA_WRITE_TTYPE_CI_PR_W,
                                  PBA_WRITE_TTYPE_DC,
                                  PBA_READ_TTYPE_CL_RD_NC);

        if (rc)
        {
            rc = CENTAUR_DATA_SETUP_ERROR;
            break;
        }

        rc = gpe_pba_parms_create(&(o_config->scomParms),
                                  PBA_SLAVE_CENTAUR,
                                  PBA_WRITE_TTYPE_CI_PR_W,
                                  PBA_WRITE_TTYPE_DC,
                                  PBA_READ_TTYPE_CI_PR_RD);

        if (rc)
        {
            rc = CENTAUR_SCOM_SETUP_ERROR;
            break;
        }

        // Go into each MCS on the chip, and for all enabled MCS get a couple
        // of SCOMs and check configuration items for correctness. If any of
        // the Centaur are configured, exactly one of the MCS must be
        // designated to receive the SYNC commands.

        // Note that the code uniformly treats SCOM failures of the MCFGPR
        // registers as an unconfigured Centaur. This works both for Murano,
        // which only defines the final 4 MCS, as well as for our VBU models
        // where some of the "valid" MCS are not in the simulation models.

        for (i = 0; i < OCCHW_NCENTAUR; ++i)
        {
            uint64_t val64;

            // If can't scom then assume not configured
            //rc = getscom_abs(MCFIR[i / 2], &(mcifir.value));

            //if (rc)
            //{
            //    rc = 0;
            //    continue;
            //}

            // See Cumulus MC "Datapath Fault Isolation Register"  Is it right???
            //FIR bits have changed from p8 TODO do we need to look at the fir
            //for P9 ??
            //if (mcifir.fields.channel_fail_signal_active)
            //{
            //    continue;
            //}

            // Routine p9c_set_inband_addr.C uses MCRSVDE and MCRSVDF to set
            //   inband address (MCFGPR)
            //
            rc = getscom_abs(MCFGPR[i], &(mcfgpr.value));

            if (rc)
            {
                rc = 0;
                continue;
            }

            // TODO ENGD work-around until gets pushed up.
            // Turn on bit 25 in mode0 regs. They should all have the same value
            rc = getscom_abs(0x05010811, &val64);
            val64 |= 0x0000004000000000ull;
            rc = putscom_abs(0x05010811, val64);
            rc = putscom_abs(0x05010891, val64);
            rc = putscom_abs(0x03010811, val64);
            rc = putscom_abs(0x03010891, val64);

            // If inband scom is not configured then assume the centaur does not exist
            if (!mcfgpr.fields.mcfgprq_valid)
            {
                continue;
            }


            // The 31-bit base-address is moved to begin at bit 8 in the
            // 64-bit PowerBus address.
            // Set the HOST/OCC bit in the address.
            o_config->baseAddress[i] =
                ((uint64_t)(mcfgpr.fields.mcfgprq_base_address) << 25) | PBA_HOST_OCC_CFG;

            PK_TRACE_DBG("Centar[%d] Base Address: %016llx",i,o_config->baseAddress[i]);


            // TODO this bit no longer exists
            // If this MCS is configured to be the designated SYNC unit, it
            // must be the only one.

            //if (mcsmode0.fields.enable_centaur_sync)
            //{
            //    if (designatedSync > 0)
            //    {
            //        PK_TRACE_DBG("Both MCS %d and %d are designated "
            //                     "for Centaur Sync",
            //                     designatedSync, i);
            //        rc = CENTAUR_MULTIPLE_DESIGNATED_SYNC;
            //        break;

            //    }
            //    else
            //    {
            //        designatedSync = i;
            //    }
            //}


            // Add the Centaur to the configuration
            o_config->config |= (CHIP_CONFIG_MCS(i) | CHIP_CONFIG_CENTAUR(i));
        }

        if (rc)
        {
            break;
        }


        // In p9 the enable_centaur_sync is no longer available as a mode bit.
        // TODO  Anything to do?
        // P8:
        // If Centaur are configured, make sure at least one of the MCS will
        // handle the SYNC. If so, convert its base address into an address
        // for issuing SYNC commands by setting bits 27 (OCC) 28 and 29
        // (Sync), then insert this address into the extended address field of
        // a PBA slave control register image. gsc_scom_centaur() then merges
        // this extended address into the PBA slave control register (which
        // has been set up for Centaur SCOM) to do the SYNC.

        // In the override mode (i_setup > 1) we tag the first valid MCS
        // to recieve the sync if the firmware has not set it up correctly.

        if (o_config->config)
        {
#if defined(__P8_DESIGNATED_SYNC__)

            if (designatedSync < 0)
            {
                if (i_setup <= 1)
                {
                    PK_TRACE_DBG("No MCS is designated for Centaur SYNC");
                    rc = CENTAUR_NO_DESIGNATED_SYNC;
                    break;

                }
                else
                {

                    designatedSync =
                        cntlz32(o_config->config << CHIP_CONFIG_MCS_BASE);

                    rc = _getscom(designatedSync, MCSMODE0, &(mcsmode0.value));

                    if (rc)
                    {
                        PK_TRACE_DBG("Unexpected rc = 0x%08x "
                                     "SCOMing MCSMODE0(%d)",
                                     (uint32_t)rc,
                                     designatedSync);

                        rc = CENTAUR_MCSMODE0_SCOM_FAILURE;
                        break;
                    }

                    mcsmode0.fields.enable_centaur_sync = 1;

                    rc = _putscom(designatedSync, MCSMODE0, mcsmode0.value);

                    if (rc)
                    {
                        PK_TRACE_DBG("Unexpected rc = 0x%08x "
                                     "SCOMing MCSMODE0(%d)",
                                     (uint32_t)rc,
                                     designatedSync);

                        rc = CENTAUR_MCSMODE0_SCOM_FAILURE;
                        break;
                    }
                }
            }

#else
            // first centaur found
            designatedSync = cntlz32(o_config->config << CHIP_CONFIG_MCS_BASE);
#endif
            // Set the OCC/HOST bit in the PBA
            base = o_config->baseAddress[designatedSync] | PBA_HOST_OCC_CFG;

            //Pick out the PBA address sub field that will be set by the slvctl extaddr
            //bits [23:36]
            slvctl.value = 0;
            slvctl.fields.extaddr = base >> 27;

            o_config->syncSlaveControl.value = slvctl.value;
        }


        // Configure the PBA BAR and PBA BARMSK.
        // Set the BARMSK bits such that:
        // -PBA[8:22] are provided by the PBABAR.
        // -PBA[23:36] are provided by the PBASLVCTL ExtrAddr field
        // -PBA[37:43] are provided by the OCI addr[5:11]
        // PBA[44:63] will always come from the OCI addr[12:31]
        // Note: This code should no longer be needed when the BAR/BARMSK is set
        // by PHYP.
        if (o_config->config != 0)
        {
            uint64_t bar = 0;
            uint64_t barMsk = PBA_BARMSKN_MASK_MASK;

            for (i = 0; i < OCCHW_NCENTAUR; ++i)
            {
                bar |= o_config->baseAddress[i];
            }

            bar &= ~barMsk;

            PK_TRACE_DBG("PBABAR(%d): %016llx", PBA_BAR_CENTAUR, bar);
            PK_TRACE_DBG("PBABARMSK: %016llx", barMsk);

            rc = putscom_abs(PBA_BARMSKN(PBA_BAR_CENTAUR), barMsk);

            if (rc)
            {
                PK_TRACE_DBG("Unexpected rc = 0x%08x SCOMing PBA_BARMSKN(%d)\n",
                             (uint32_t)rc, PBA_BAR_CENTAUR);
                rc = CENTAUR_BARMSKN_PUTSCOM_FAILURE;
                break;
            }

            rc = putscom_abs(PBA_BARN(PBA_BAR_CENTAUR), bar);
            if (rc)
            {
                PK_TRACE_DBG("Unexpected rc = 0x%08x SCOMing PBA_BARN(%d)\n",
                             (uint32_t)rc, PBA_BAR_CENTAUR);
                rc = CENTAUR_BARN_PUTSCOM_FAILURE;
                break;
            }
        }

        // Do an independent check that every Centaur base address
        // can be generated by the combination of the current BAR and
        // BAR Mask, along with the initial requirement that the mask must
        // include at least bits 38:43.

        if (o_config->config != 0)
        {
            rc = getscom_abs(PBA_BARN(PBA_BAR_CENTAUR), &bar);

            if (rc)
            {
                PK_TRACE_DBG("Unexpected rc = 0x%08x SCOMing PBA_BARN(%d)\n",
                             (uint32_t)rc, PBA_BAR_CENTAUR);
                rc = CENTAUR_BARN_GETSCOM_FAILURE;
                break;
            }

            rc = getscom_abs(PBA_BARMSKN(PBA_BAR_CENTAUR), &mask);

            if (rc)
            {
                PK_TRACE_DBG("Unexpected rc = 0x%08x SCOMing PBA_BARMSKN(%d)\n",
                             (uint32_t)rc, PBA_BAR_CENTAUR);
                rc = CENTAUR_BARMSKN_GETSCOM_FAILURE;
                break;
            }

            bar = bar & PBA_BARN_ADDR_MASK;
            mask = mask & PBA_BARMSKN_MASK_MASK;

            if ((mask & 0x0000000003f00000ull) != 0x0000000003f00000ull)
            {

                PK_TRACE("PBA BAR mask (%d) does not cover bits 38:43\n", PBA_BAR_CENTAUR);
                rc = CENTAUR_MASK_ERROR;
                break;
            }

            for (i = 0; i < OCCHW_NCENTAUR; ++i)
            {
                if (o_config->baseAddress[i] != 0)
                {
                    if ((o_config->baseAddress[i] & ~mask) !=
                        (bar & ~mask))
                    {

                        PK_TRACE("BAR/Mask (%d) error for MCS/Centaur %d",
                                 PBA_BAR_CENTAUR, i);

                        PK_TRACE("    base = 0x%08x%08x",
                                 (uint32_t)(o_config->baseAddress[i]>>32),
                                 (uint32_t)(o_config->baseAddress[i]));

                        PK_TRACE("    bar  = 0x%08x%08x"
                                 "    mask = 0x%08x%08x",
                                 (uint32_t)(bar >> 32),
                                 (uint32_t)(bar),
                                 (uint32_t)(mask >> 32),
                                 (uint32_t)(mask));

                        rc = CENTAUR_BAR_MASK_ERROR;
                        break;
                    }
                }
            }

            if (rc)
            {
                break;
            }
        }


        // At this point the structure is initialized well-enough that it can
        // be used by gpe_scom_centaur().


        o_config->configRc = 0;

        if (o_config->config == 0)
        {
            break;
        }


        // Get Device ID from each centaur
        centaur_get_scom_vector(o_config,
                                CENTAUR_DEVICE_ID,
                                (uint64_t*)(&(o_config->deviceId[0])));

    }
    while(0);

    o_config->configRc = rc;

    mtmsr(orig_msr);

    return rc;
}

