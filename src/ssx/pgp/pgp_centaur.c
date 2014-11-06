// $Id: pgp_centaur.c,v 1.2 2013/12/13 23:01:15 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/pgp_centaur.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pgp_centaur.c
/// \brief Support for Centaur access and configuration from OCC.
///
/// Generic PBA generated PowerBus Address in pictures and words:
/// 
/// \code
///
///           1         2         3         4         5         6
/// 0123456789012345678901234567890123456789012345678901234567890123
///    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
///                                                                 
/// ..............B-------BX------------XO-----OA------------------A
/// 
/// .: Unused
/// B: Direct from PBA BAR, bits 14:22
/// X: If PBA BAR MASK 23:36 == 0, then PBA BAR 23:36
///    Else Extended Address 0:13
/// O: If PBA BAR MASK 37:43 == 0, then PBA BAR 37:43
///    Else OCI Address 5:11
/// A: OCI Address 12:31
///
/// \endcode
///
/// The OCI address always selects the low-order 20 bits of the PowerBus
/// address, i.e., the window size is always a multiple of 1MB. The PBA BAR
/// mask allows up to a 128MB window into main memory without using the
/// extended address. The extended address allows OCC to address up to 2^41
/// bytes by manipulating the extended address, assuming PHYP sets up the mask
/// correctly.
/// 
/// 
/// Centaur in-band SCOM, sensor cache and SYNC addressing in pictures and words:
///
/// \code
/// 
///           1         2         3         4         5         6
/// 0123456789012345678901234567890123456789012345678901234567890123
///    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
///                                                                 
/// ..............B-------BX------------XO-----OA------------------A - See Above
/// ..............M-----------M10S------------------------------S000 - SCOM
/// ..............M-----------M1100000000000000000000000000000000000 - Sensor cache
/// ..............M-----------M1110000000000000000000000000000000000 - Sync
/// 
/// .: Unused
/// M: The base address of the Centaur, taken from the MCS MCFGPR.
/// O: 1 Signifies that this access comes from the OCC.
/// S: The 32-bit SCOM address
/// 0: Zero
/// 
/// \endcode
///
/// In order to access the Centaur for in-band SCOM, the PBA BAR MASK must
/// extend at least from bit 29 down to bit 43, in order to allow the OCC to
/// generate these addresses. In practice the mask must allow all configured
/// Centaur to be accessed. This means that all Centaur in-band address bits
/// not controllable by OCC through the mask must be equal.
/// 
/// Note that the SCOM address must be split between the extended address and
/// the OCI address.
/// 
/// We assume (and verify) that MCMODE0(36) will always be set which means
/// that bit 27 is a flag indicating whether an access comes from FSP or
/// OCC. All OCC (GPE) accesses set this flag to 1.

#include "ssx.h"
#include "gpe_scom.h"

#if defined(VERIFICATION) || defined(LAB_VALIDATION)
#define PRINTD(...) printk(__VA_ARGS__)
#else
#define PRINTD(...)
#endif

CentaurConfiguration G_centaurConfiguration
SECTION_ATTRIBUTE(".noncacheable_ro") = {.configRc = CENTAUR_NOT_CONFIGURED};

const uint16_t _pgp_mcs_offset[PGP_NMCS] = {
    0x0800, 0x0880, 0x0900, 0x0980, 0x0c00, 0x0c80, 0x0d00, 0x0d80
};


// All GpeScomParms structures are required to be noncacheable, so we have to
// allocate a static instance rather than using the stack. For simplicity the
// single-entry scomList_t required to collect the Centaur device IDs is
// allocated statically as well.

static GpeScomParms S_parms SECTION_ATTRIBUTE(".noncacheable") = {0};
static scomList_t S_scomList SECTION_ATTRIBUTE(".noncacheable") = {{{0}}};

int
_centaur_configuration_create(int i_bar, int i_slave, int i_setup)
{
    CentaurConfiguration config;
    int i, designatedSync, diffInit;
    int64_t rc;                 /* Must be copied to global struct. */
    mcfgpr_t mcfgpr;
    mcsmode0_t mcsmode0;
    pba_slvctln_t slvctl;
    uint64_t diffMask, addrAccum, bar, mask, base;
    PoreFlex request;

    // Start by clearing the local structure and setting the error flag.
    memset(&config, 0, sizeof(config));
    config.configRc = CENTAUR_NOT_CONFIGURED;

    designatedSync = -1;

    do {
        // Basic consistency checks

        if ((i_bar < 0) || (i_bar >= PBA_BARS) ||
            (i_slave < 0) || (i_slave >= PBA_SLAVES)) {
            
            rc = CENTAUR_INVALID_ARGUMENT;
            break;
        }

        
        // Create the setups for the GPE procedures. The 'dataParms' are the
        // setup for accessing the Centaur sensor cache.  The 'scomParms' are
        // the setup for accessing Centaur SCOMs. 

        rc = gpe_pba_parms_create(&(config.dataParms),
                                  PBA_SLAVE_PORE_GPE,
                                  PBA_WRITE_TTYPE_CI_PR_W,
                                  PBA_WRITE_TTYPE_DC,
                                  PBA_READ_TTYPE_CL_RD_NC);
        if (rc) {
            rc = CENTAUR_DATA_SETUP_ERROR;
            break;
        }

        rc = gpe_pba_parms_create(&(config.scomParms),
                                  PBA_SLAVE_PORE_GPE,
                                  PBA_WRITE_TTYPE_CI_PR_W,
                                  PBA_WRITE_TTYPE_DC,
                                  PBA_READ_TTYPE_CI_PR_RD);
        if (rc) {
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

        for (i = 0; i < PGP_NCENTAUR; i++) {

            rc = _getscom(MCS_ADDRESS(MCFGPR, i), &(mcfgpr.value),
                          SCOM_TIMEOUT);
            if (rc) {
                rc = 0;
                config.baseAddress[i] = 0;
                continue;
            }

            if (!mcfgpr.fields.mcfgprq_valid) continue;

            rc = _getscom(MCS_ADDRESS(MCSMODE0, i), &(mcsmode0.value),
                          SCOM_TIMEOUT);
            if (rc) {
                PRINTD("Unexpected rc = 0x%08x SCOMing MCSMODE0(%d)\n",
                       (uint32_t)rc, i);
                rc = CENTAUR_MCSMODE0_SCOM_FAILURE;
                break;
            }


            // We require that the MCFGRP_19_IS_HO_BIT be set in the mode
            // register.  We do not support the option of this bit not being
            // set, and all of our procedures will set bit 19 of the PowerBus
            // address to indicate that OCC is making the access.

            if (!mcsmode0.fields.mcfgrp_19_is_ho_bit) {

                PRINTD("MCSMODE0(%d).mcfgrp_19_is_ho_bit == 0\n", i);
                rc = CENTAUR_MCSMODE0_19_FAILURE;
                break;
            }
                

            // The 14-bit base-address is moved to begin at bit 14 in the
            // 64-bit PowerBus address. The low-order bit of this address (bit
            // 19 mentioned above which is bit 27 as an address bit) must be 0
            // - otherwise there is confusion over who's controlling this
            // bit.

            config.baseAddress[i] = 
                ((uint64_t)(mcfgpr.fields.mcfgprq_base_address)) << 
                (64 - 14 - 14);

            if (config.baseAddress[i] & 0x0000001000000000ull) {

                PRINTD("Centaur base address %d has bit 27 set\n", i);
                rc = CENTAUR_ADDRESS_27_FAILURE;
                break;
            }


            // If this MCS is configured to be the designated SYNC unit, it
            // must be the only one. 

            if (mcsmode0.fields.enable_centaur_sync) {

                if (designatedSync > 0) {

                    PRINTD("Both MCS %d and %d are designated "
                           "for Centaur Sync\n",
                           designatedSync, i);
                    rc = CENTAUR_MULTIPLE_DESIGNATED_SYNC;
                    break;

                } else {

                    designatedSync = i;
                }
            }


            // Add the Centaur to the configuration

            config.config |= (CHIP_CONFIG_MCS(i) | CHIP_CONFIG_CENTAUR(i));
        }

        if (rc) break;


        // If Centaur are configured, make sure at least one of the MCS will
        // handle the SYNC. If so, convert its base address into an address
        // for issuing SYNC commands by setting bits 27 (OCC) 28 and 29
        // (Sync), then insert this address into the extended address field of
        // a PBA slave control register image. gsc_scom_centaur() then merges
        // this extended address into the PBA slave control register (which
        // has been set up for Centaur SCOM) to do the SYNC.

        // In the override mode (i_setup > 1) we tag the first valid MCS
        // to recieve the sync if the firmware has not set it up correctly.

        if (config.config) {

            if (designatedSync < 0) {

                if (i_setup <= 1) {

                    PRINTD("No MCS is designated for Centaur SYNC\n");
                    rc = CENTAUR_NO_DESIGNATED_SYNC;
                    break;

                } else {

                    designatedSync = 
                        cntlz32(left_justify_mcs_config(config.config));

                    rc = _getscom(MCS_ADDRESS(MCSMODE0, designatedSync), 
                                  &(mcsmode0.value),
                                  SCOM_TIMEOUT);
                    if (rc) {
                        PRINTD("Unexpected rc = 0x%08x SCOMing MCSMODE0(%d)\n",
                               (uint32_t)rc, designatedSync);
                        rc = CENTAUR_MCSMODE0_SCOM_FAILURE;
                        break;
                    }

                    mcsmode0.fields.enable_centaur_sync = 1;

                    rc = _putscom(MCS_ADDRESS(MCSMODE0, designatedSync), 
                                  mcsmode0.value,
                                  SCOM_TIMEOUT);
                    if (rc) {
                        PRINTD("Unexpected rc = 0x%08x SCOMing MCSMODE0(%d)\n",
                               (uint32_t)rc, designatedSync);
                        rc = CENTAUR_MCSMODE0_SCOM_FAILURE;
                        break;
                    }
                }
            }

            base = config.baseAddress[designatedSync] | 0x0000001c00000000ull;

            slvctl.value = 0;
            slvctl.fields.extaddr = (base & 0x000001fff8000000ull) >> 27;

            config.syncSlaveControl = slvctl.value;
        }

        
        // At this point we have one or more enabled MCS and they pass the
        // initial configuration sniff test. We can now implement the option
        // to configure the PBA BAR and BAR MASK correctly to allow access to
        // these Centaur. We do this by computing the minimum BAR mask that
        // covers all of the Centaur base addresses. This is done by
        // accumulating a difference mask of the base addresses and finding
        // the first set bit in the mask.
        //
        // Note that we do the configuration here on demand, but always do the
        // correctness checking as the next step.

        if (i_setup && (config.config != 0)) {

            diffInit = 0;
            diffMask = 0;       /* GCC happiness */
            addrAccum = 0;      /* GCC happiness */
            
            for (i = 0; i < PGP_NCENTAUR; i++) {

                if (config.baseAddress[i] != 0) {

                    if (!diffInit) {

                        diffInit = 1;
                        diffMask = 0;
                        addrAccum = config.baseAddress[i];

                    } else {

                        diffMask |= 
                            (config.baseAddress[i] ^ addrAccum);
                        addrAccum |= config.baseAddress[i];
                    }

                    if (0) {

                        // Debug

                        printk("i:%d baseAddress: 0x%016llx "
                               "diffMask: 0x%016llx, addrAccum: 0x%016llx\n",
                               i, config.baseAddress[i], diffMask, addrAccum);
                    }
                }
            }

            // The mask must cover all differences - and must also have at
            // least bit 27 set. The mask register contains only the mask. The
            // BAR is set to the accumulated address outside of the mask. The
            // BAR also contains a scope field which defaults to 0 (Nodal
            // Scope) for Centaur inband access.

            diffMask |= 0x0000001000000000ull;            
            mask = 
                ((1ull << (64 - cntlz64(diffMask))) - 1) &
                PBA_BARMSKN_MASK_MASK;

            rc = _putscom(PBA_BARMSKN(i_bar), mask, SCOM_TIMEOUT);
            if (rc) {
                PRINTD("Unexpected rc = 0x%08x SCOMing PBA_BARMSKN(%d)\n",
                       (uint32_t)rc, i_bar);
                rc = CENTAUR_BARMSKN_PUTSCOM_FAILURE;
                break;
            }

            rc = _putscom(PBA_BARN(i_bar), addrAccum & ~mask, SCOM_TIMEOUT);
            if (rc) {
                PRINTD("Unexpected rc = 0x%08x SCOMing PBA_BARN(%d)\n",
                       (uint32_t)rc, i_bar);
                rc = CENTAUR_BARN_PUTSCOM_FAILURE;
                break;
            }
        }


        // Do an independent check that every Centaur base address
        // can be generated by the combination of the current BAR and
        // BAR Mask, along with the initial requirement that the mask must
        // include at least bits 27:43.

        if (config.config != 0) {

            rc = _getscom(PBA_BARN(i_bar), &bar, SCOM_TIMEOUT);
            if (rc) {
                PRINTD("Unexpected rc = 0x%08x SCOMing PBA_BARN(%d)\n",
                       (uint32_t)rc, i_bar);
                rc = CENTAUR_BARN_GETSCOM_FAILURE;
                break;
            }

            rc = _getscom(PBA_BARMSKN(i_bar), &mask, SCOM_TIMEOUT);

            if (rc) {
                PRINTD("Unexpected rc = 0x%08x SCOMing PBA_BARMSKN(%d)\n",
                       (uint32_t)rc, i_bar);
                rc = CENTAUR_BARMSKN_GETSCOM_FAILURE;
                break;
            }

            bar = bar & PBA_BARN_ADDR_MASK;
            mask = mask & PBA_BARMSKN_MASK_MASK;

            if ((mask & 0x0000001ffff00000ull) != 0x0000001ffff00000ull) {

                PRINTD("PBA BAR mask (%d) does not cover bits 27:43\n", i_bar);
                rc = CENTAUR_MASK_ERROR;
                break;
            }

            for (i = 0; i < PGP_NCENTAUR; i++) {

                if (config.baseAddress[i] != 0) {

                    if ((config.baseAddress[i] & ~mask) != 
                        (bar & ~mask)) {

                        PRINTD("BAR/Mask (%d) error for MCS/Centaur %d\n"
                               "    base = 0x%016llx\n"
                               "    bar  = 0x%016llx\n"
                               "    mask = 0x%016llx\n",

                               i_bar, i, config.baseAddress[i], bar, mask);
                        rc = CENTAUR_BAR_MASK_ERROR;
                        break;
                    }
                }
            }

            if (rc) break;
        }


        // At this point the structure is initialized well-enough that it can
        // be used by gpe_scom_centaur(). We run gpe_scom_centaur() to collect
        // the CFAM ids of the chips.  Prior to this we copy our local copy
        // into the global read-only data structure. (Note that GPE can DMA
        // under the OCC TLB memory protection.) In order for
        // gpe_scom_centaur() to run the global configuration must be valid
        // (configRc == 0) - so we provisionally mark it valid (and will
        // invalidate it later if errors occur here).

        // Note however that if no Centaur are present then we're already
        // done.

        // It's assumed that this procedure is being run before threads have
        // started, therefore we must poll for completion of the GPE program.
        // Assuming no contention for GPE1 this procedure should take a few
        // microseconds at most to complete.

        if (0) {

            // Debug for Simics - only enable MCS 5

            config.baseAddress[0] =
                config.baseAddress[1] =
                config.baseAddress[2] =
                config.baseAddress[3] =
                config.baseAddress[4] =
                config.baseAddress[6] =
                config.baseAddress[7] = 0;
        }


        config.configRc = 0;
        memcpy_real(&G_centaurConfiguration, &config, sizeof(config));

        if (config.config == 0) break;

        S_scomList.scom = CENTAUR_DEVICE_ID;
        S_scomList.commandType = GPE_SCOM_READ_VECTOR;
        S_scomList.pData = G_centaurConfiguration.deviceId;

        S_parms.scomList = CAST_POINTER(uint64_t, &S_scomList);
        S_parms.entries = 1;
        S_parms.options = 0;
        
        pore_flex_create(&request,
                         &G_pore_gpe1_queue,
                         gpe_scom_centaur,
                         (uint32_t)(&S_parms),
                         SSX_MILLISECONDS(10), /* Timeout */
                         0, 0, 0);

        rc = pore_flex_schedule(&request);

        if (rc) break;

        while (!async_request_is_idle((AsyncRequest*)(&request)));

        if (!async_request_completed((AsyncRequest*)(&request)) ||
            (S_parms.rc != 0)) {

            PRINTD("gpe_scom_centaur() for CENTAUR_DEVICE_ID failed:\n"
                   "    Async state                   = 0x%02x\n"
                   "    gpe_scom_centaur() rc         = %u\n"
                   "    gpe_scom_centaur() errorIndex = %d\n",
                   ((AsyncRequest*)(&request))->state,
                   S_parms.rc, S_parms.errorIndex);
                   
            rc = CENTAUR_READ_TPC_ID_FAILURE;
        }

        if (0) {

            // Debug

            slvctl.value = G_gsc_lastSlaveControl;
            
            PRINTD("centaur_configuration_create:Debug\n"
                   "    Last SCOM (PowerBus) address  = 0x%016llx\n"
                   "    Last Slave Control            = 0x%016llx\n"
                   "    Extended Address (positioned) = 0x%016llx\n"
                   "    Last OCI Address              = 0x%016llx\n",
                   G_gsc_lastScomAddress,
                   G_gsc_lastSlaveControl,
                   (unsigned long long)(slvctl.fields.extaddr) <<
                   (64 - 23 - 14),
                   G_gsc_lastOciAddress);
        }

    } while (0);

    // Copy the final RC into the global structure and done.

    memcpy_real(&(G_centaurConfiguration.configRc), &rc, sizeof(rc));

    return rc;
}


// For now we have to handle configuring the PBA BAR and mask, and designating
// a SYNC if the firmware forgot to.

int
centaur_configuration_create(void)
{
    return _centaur_configuration_create(PBA_BAR_CENTAUR, 
                                         PBA_SLAVE_PORE_GPE,
                                         2);
}


uint32_t mb_id(int i_mb)
{
    uint32_t rv;
    centaur_device_id_t id;

    if ((i_mb < 0) || (i_mb >= PGP_NCENTAUR) ||
        (G_centaurConfiguration.configRc != 0)) {

        rv = (uint32_t)-1;

    } else {

        id.value = G_centaurConfiguration.deviceId[i_mb];
        rv = id.fields.cfam_id;
    }

    return rv;
}


uint8_t mb_chip_type(int i_mb)
{
    uint8_t rv;
    cfam_id_t id;

    if ((id.value = mb_id(i_mb)) == -1) {

        rv = (uint8_t)-1;

    } else {

        rv = id.chipType;
    }

    return rv;
}


uint8_t mb_ec_level(int i_mb)
{
    uint8_t rv;
    cfam_id_t id;

    if ((id.value = mb_id(i_mb)) == -1) {

        rv = (uint8_t)-1;

    } else {

        rv = (id.majorEc << 4) | id.minorEc;
    }

    return rv;
}
