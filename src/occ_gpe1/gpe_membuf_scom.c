/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe_membuf_scom.c $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2018,2019                        */
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
#include <stdint.h>
#include "gpe_membuf.h"
#include "gpe_pba_cntl.h"
#include "ppe42_scom.h"
#include "ppe42.h"
#include "pba_register_addresses.h"
#include "ppe42_msr.h"

/**
 * @file gpe_membuf_scom
 * @brief scom access from gpe to a membuf
 */

uint32_t g_inband_access_state = INBAND_ACCESS_INACTIVE;

void swap_u64(uint64_t * data64)
{
    uint64_t val = *data64;
    val = ((val << 8) & 0xff00ff00ff00ff00ULL) | ((val >> 8) & 0x00ff00ff00ff00ffULL);
    val = ((val <<16) & 0xffff0000ffff0000ULL) | ((val >>16) & 0x0000ffff0000ffffULL);
    *data64 = (val << 32) | (val >> 32);
}

int inband_access(MemBufConfiguration_t* i_config,
                   uint32_t i_instance,
                   uint32_t i_oci_addr,
                   uint64_t * io_data,
                   int      i_read_write)
{
    int rc = 0;
    uint32_t org_msr = mfmsr();
    uint32_t msr = org_msr | MSR_SEM; // Mask off SIB from generating mck.

    g_inband_access_state = INBAND_ACCESS_IN_PROGRESS;

    if(i_read_write == INBAND_ACCESS_READ)
    {
        mtmsr(msr);
        sync();
        PPE_LVD(i_oci_addr, *io_data);

        PK_TRACE_DBG("inband read %08x%08x from %08x",
                     (uint32_t)((*io_data)>>32),
                     (uint32_t)((*io_data)),
                     i_oci_addr);
        if(i_config->membuf_type != MEMTYPE_CENTAUR)
        {
            swap_u64(io_data);
        }
    }
    else
    {
        if(i_config->membuf_type != MEMTYPE_CENTAUR)
        {
            swap_u64(io_data);
        }
        // Set PPE to precise mode for stores so that in the case of a machine
        // check, there is a predictable instruction address to resume on.

        msr &= ~MSR_IPE;
        mtmsr(msr);
        sync();

        uint64_t data64 = *io_data; //This makes PPE_STVD generate better code
        PPE_STVD(i_oci_addr, data64);

        PK_TRACE_DBG("inband write %08x%08x to %08x",
                     (uint32_t)((*io_data)>>32),
                     (uint32_t)((*io_data)),
                     i_oci_addr);
    }

    // Poll SIB error or machine check
    if((mfmsr() & MSR_SIBRC) ||
       g_inband_access_state != INBAND_ACCESS_IN_PROGRESS)
    {
        // Take membuf out of config
        PK_TRACE("Removing MemBuf %d from list of configured MemBufs",
                 i_instance);

        i_config->config &= ~(CHIP_CONFIG_MEMBUF(i_instance));

        // This will cause the 405 to remove the membuf sensor(s).
        rc = MEMBUF_CHANNEL_CHECKSTOP;
    }
    g_inband_access_state = INBAND_ACCESS_INACTIVE;
    mtmsr(org_msr);
    return rc;
}

/**
 * Setup the PBASLVCTLN extended address and calculate the OCI scom address
 * @param[in] PBA base address
 * @param[in] The inband scom address
 * @returns the mapped OCI address to scom the membuf
 * @Post The extended address field in the PBASLVCNT is set
 */
int inband_scom_setup(MemBufConfiguration_t* i_config,
                            uint32_t i_membuf_instance,
                            uint32_t i_scom_address,
                            uint32_t *o_oci_addr)
{
    int rc = 0;
    uint32_t scom_address = i_scom_address;
    // TODO use PBASLV in P10
#if defined(__USE_PBASLV__)
   pba_slvctln_t slvctln;
#endif
    uint64_t pb_addr = i_config->baseAddress[i_membuf_instance];

    if(i_config->membuf_type == MEMTYPE_CENTAUR)
    {
        // Break address into componets
        uint32_t local = i_scom_address & 0x00001fff;
        uint32_t port  = i_scom_address & 0x000f0000;
        uint32_t slave = i_scom_address & 0x03000000;
        uint32_t multi = i_scom_address & 0xc0000000;

        // compress to 21 bits for P9
        scom_address =
            local +
            (port >> 3) +
            (slave >> 7) +
            (multi >> 11);
        // P9: Turn on bit 38 to indicate OCC
        pb_addr  |= 0x0000000002000000ull;
    }
    pb_addr  |= ((uint64_t)scom_address << 3);

#if defined(__USE_PBASLV__)
    // put bits 23:36 of address into slvctln extended addr
    PPE_LVD((i_config->scomParms).slvctl_address, slvctln.value);
    slvctln.fields.extaddr = pb_addr >> 27;
    PPE_STVD((i_config->scomParms).slvctl_address, slvctln.value);
#else
    // TODO P9 HW bug work-around - Use PBASLV in P10
    {
        // workaround - don't use extraddr - use pbabar.
        uint64_t barMsk = 0;

        // Mask SIB from generating mck
        mtmsr(mfmsr() | MSR_SEM);

        // put the PBA in the BAR
        rc = putscom_abs(PBA_BARN(PBA_BAR_MEMBUF), pb_addr & 0x00fffffffff00000ull);
        if(rc)
        {
            PK_TRACE("inband_scom_setup. putscom fail on PBABAR."
                     " rc = %d",rc);
        }
        else
        {
            rc = putscom_abs(PBA_BARMSKN(PBA_BAR_MEMBUF), barMsk);
            if(rc)
            {
                PK_TRACE("inband_scom_setup. putscom fail on PBABARMSK"
                         " rc = %d",rc);
            }
        }
    }
#endif
    // make oci address
    *o_oci_addr = (uint32_t)(pb_addr & 0x07ffffffull);

    // upper nibble is PBA region and BAR_SELECT
    *o_oci_addr  |= ((PBA_BAR_MEMBUF | 0x8) << 28);
    PK_TRACE_DBG("OCI mapped scom addr: %08x",*o_oci_addr);
    return rc;
}

void pbaslvctl_reset(GpePbaParms* i_pba_parms)
{
    uint64_t val = 0;

    do
    {
        PPE_STVD(PBA_SLVRST, i_pba_parms->slvrst.value);
        PPE_LVD(PBA_SLVRST, val);
        val &= i_pba_parms->slvrst_in_progress.value;
    }
    while(val != 0);
}

uint64_t pbaslvctl_setup(GpePbaParms* i_pba_parms)
{
    uint64_t slvctl_val;
    uint64_t slvctl_val_org;
    PPE_LVD(i_pba_parms->slvctl_address, slvctl_val_org);
    slvctl_val = slvctl_val_org;
    slvctl_val &= i_pba_parms->mask.value;
    slvctl_val |= i_pba_parms->slvctl.value;
    PPE_STVD(i_pba_parms->slvctl_address, slvctl_val);
    return slvctl_val_org;
}

// Get data from each existing membuf.
int membuf_get_scom_vector(MemBufConfiguration_t* i_config,
                            uint32_t i_scom_address,
                            uint64_t* o_data)
{
    int rc = 0;
    int access_rc = 0;
    int pba_rc = 0;
    int instance = 0;
    int max_mb = OCCHW_N_MEMBUF;
    uint64_t pba_slvctln_save;

    pbaslvctl_reset(&(i_config->scomParms));
    pba_slvctln_save = pbaslvctl_setup(&(i_config->scomParms));

    // clear SIB errors in MSR
    mtmsr((mfmsr() & ~(MSR_SIBRC | MSR_SIBRCA)));

    if(i_config->membuf_type == MEMTYPE_CENTAUR)
    {
        max_mb = OCCHW_N_CENT;
    }

    for(instance = 0; instance < max_mb; ++instance)
    {
        if( CHIP_CONFIG_MEMBUF(instance) & (i_config->config))
        {
            uint32_t oci_addr;
            pba_rc = inband_scom_setup(i_config,
                                        instance,
                                        i_scom_address,
                                        &oci_addr);

            if(pba_rc)
            {
                rc = pba_rc;
                // Already traced.
                // Trumps any access error
                *o_data = 0;
            }
            else
            {
                // inband scom read
                access_rc = inband_access(i_config,
                                          instance,
                                          oci_addr,
                                          o_data,
                                          INBAND_ACCESS_READ);
                // only set access_rc if rc not already set.
                if(!rc && access_rc)
                {
                    // not critical, but don't access this membuf again.
                    rc = access_rc;
                    *o_data = 0;
                    // continue with next instance
                }
            }
        }
        else
        {
            *o_data = 0;
        }

        ++o_data;
    }

    // gpe_pba_cntl function?
    pbaslvctl_reset(&(i_config->scomParms));
    PPE_STVD((i_config->scomParms).slvctl_address, pba_slvctln_save);

    return rc;
}

int membuf_get_scom(MemBufConfiguration_t* i_config,
                     int i_membuf_instance,
                     uint32_t i_scom_address,
                     uint64_t* o_data)
{
    int rc = 0;
    uint32_t oci_addr;
    uint64_t pba_slvctln_save;

    pbaslvctl_reset(&(i_config->scomParms));
    pba_slvctln_save = pbaslvctl_setup(&(i_config->scomParms));

    rc = inband_scom_setup(i_config,
                            i_membuf_instance,
                            i_scom_address,
                            &oci_addr);

    if( !rc && (CHIP_CONFIG_MEMBUF(i_membuf_instance) & (i_config->config)))
    {
        // read membuf scom
        rc = inband_access(i_config,
                            i_membuf_instance,
                            oci_addr,
                            o_data,
                            INBAND_ACCESS_READ);
    }
    else
    {
        *o_data = 0;
    }

    // gpe_pba_cntl function?
    pbaslvctl_reset(&(i_config->scomParms));
    PPE_STVD((i_config->scomParms).slvctl_address, pba_slvctln_save);

    return rc;
}


// Write all configured membuf with the same data
int membuf_put_scom_all(MemBufConfiguration_t* i_config,
                         uint32_t i_scom_address,
                         uint64_t i_data)
{
    int rc = 0;
    int pba_rc = 0;
    int access_rc = 0;
    int instance = 0;
    int max_mb = OCCHW_N_MEMBUF;
    uint64_t pba_slvctln_save;

    pbaslvctl_reset(&(i_config->scomParms));
    pba_slvctln_save = pbaslvctl_setup(&(i_config->scomParms));

    if(i_config->membuf_type == MEMTYPE_CENTAUR)
    {
        max_mb = OCCHW_N_CENT;
    }

    for(instance = 0; instance < max_mb; ++instance)
    {
        if( CHIP_CONFIG_MEMBUF(instance) & (i_config->config))
        {
            uint32_t oci_addr;
            pba_rc = inband_scom_setup(i_config,
                                    instance,
                                    i_scom_address,
                                    &oci_addr);

            if(pba_rc)
            {
                // Already traced in inband_scom_setup
                // Trumps access_rc
                rc = pba_rc;
            }
            else
            {
                // membuf scom
                access_rc = inband_access(i_config,
                                          instance,
                                          oci_addr,
                                          &i_data,
                                          INBAND_ACCESS_WRITE);
                // Only set access_rc if rc not already set
                if(!rc && access_rc)
                {
                    // This MemBuf won't be touched again.
                    rc = access_rc;
                    // continue with next instance
                }
            }
        }
    }

    // reset pba slave
    pbaslvctl_reset(&(i_config->scomParms));
    PPE_STVD((i_config->scomParms).slvctl_address, pba_slvctln_save);

    return rc;
}

int membuf_put_scom(MemBufConfiguration_t* i_config,
                     int i_membuf_instance,
                     uint32_t i_scom_address,
                     uint64_t i_data)
{
    int rc = 0;
    uint32_t oci_addr;
    uint64_t pba_slvctln_save;

    pbaslvctl_reset(&(i_config->scomParms));
    pba_slvctln_save = pbaslvctl_setup(&(i_config->scomParms));

    rc = inband_scom_setup(i_config,
                            i_membuf_instance,
                            i_scom_address,
                            &oci_addr);

    if(!rc)
    {
        if(CHIP_CONFIG_MEMBUF(i_membuf_instance) & (i_config->config))
        {
            // write membuf scom
            rc = inband_access(i_config,
                                i_membuf_instance,
                                oci_addr,
                                &i_data,
                                INBAND_ACCESS_WRITE);
        }
    }

    // reset pba slave
    pbaslvctl_reset(&(i_config->scomParms));
    PPE_STVD((i_config->scomParms).slvctl_address, pba_slvctln_save);

    return rc;
}

// write x
int membuf_scom_rmw(MemBufConfiguration_t* i_config,
                     int i_membuf_instance,
                     uint32_t i_scom_address,
                     uint64_t i_mask,
                     uint64_t* i_data)
{
    int rc = 0;
    uint32_t oci_addr;
    uint64_t pba_slvctln_save;
    uint64_t data64;

    pbaslvctl_reset(&(i_config->scomParms));
    pba_slvctln_save = pbaslvctl_setup(&(i_config->scomParms));

    rc = inband_scom_setup(i_config,
                            i_membuf_instance,
                            i_scom_address,
                            &oci_addr);
    if(!rc)
    {

        rc = inband_access(i_config,
                            i_membuf_instance,
                            oci_addr,
                            &data64,
                            INBAND_ACCESS_READ);

        if(!rc)
        {
            data64 &= (i_mask ^ 0xffffffffffffffffull);
            data64 |= *i_data;

            rc = inband_access(i_config,
                                i_membuf_instance,
                                oci_addr,
                                &data64,
                                INBAND_ACCESS_WRITE);
        }
    }

    pbaslvctl_reset(&(i_config->scomParms));
    PPE_STVD((i_config->scomParms).slvctl_address, pba_slvctln_save);

    return rc;
}


int membuf_scom_rmw_all(MemBufConfiguration_t* i_config,
                         uint32_t i_scom_address,
                         uint64_t i_mask,
                         uint64_t i_data)
{
    int rc = 0;
    int pba_rc = 0;
    int access_rc = 0;
    int instance = 0;
    int max_mb = OCCHW_N_MEMBUF;
    uint64_t pba_slvctln_save;

    pbaslvctl_reset(&(i_config->scomParms));
    pba_slvctln_save = pbaslvctl_setup(&(i_config->scomParms));

    if(i_config->membuf_type == MEMTYPE_CENTAUR)
    {
        max_mb = OCCHW_N_CENT;
    }

    for(instance = 0; (instance < max_mb); ++instance)
    {
        if( CHIP_CONFIG_MEMBUF(instance) & (i_config->config))
        {
            uint64_t data64;
            uint32_t oci_addr;
            pba_rc = inband_scom_setup(i_config,
                                        instance,
                                        i_scom_address,
                                        &oci_addr);
            if(pba_rc)
            {
                rc = pba_rc;
                // Already traced in inband_scom_setup
                // Trumps any access_rc
            }
            if(!pba_rc)
            {

                access_rc = inband_access(i_config,
                                           instance,
                                           oci_addr,
                                           &data64,
                                           INBAND_ACCESS_READ);

                if(!access_rc)
                {
                    data64 &= (i_mask ^ 0xffffffffffffffffull);
                    data64 |= i_data;

                    access_rc = inband_access(i_config,
                                               instance,
                                               oci_addr,
                                               &data64,
                                               INBAND_ACCESS_WRITE);
                }
            }
            if(!rc && access_rc)
            {
                rc = access_rc;
            }

            pbaslvctl_reset(&(i_config->scomParms));
        }
    }

    PPE_STVD((i_config->scomParms).slvctl_address, pba_slvctln_save);

    return rc;
}



// MemBufConfiguration needs to be setup before this is called
void gpe_inband_scom(MemBufConfiguration_t* i_config,
                      MemBufScomParms_t* i_parms)
{
    int i;
    int rc = 0;
    mtmsr((mfmsr() & ~(MSR_SIBRC | MSR_SIBRCA)) | MSR_SEM);

    for(i = 0; i < i_parms->entries; ++i)
    {
        switch(i_parms->scomList[i].commandType)
        {
            case MEMBUF_SCOM_NOP:
                break;

            case MEMBUF_SCOM_READ:
                rc =membuf_get_scom(i_config,
                                     i_parms->scomList[i].instanceNumber,
                                     i_parms->scomList[i].scom,
                                     &(i_parms->scomList[i].data));
                break;

            case MEMBUF_SCOM_WRITE:
                rc = membuf_put_scom(i_config,
                                      i_parms->scomList[i].instanceNumber,
                                      i_parms->scomList[i].scom,
                                      i_parms->scomList[i].data);
                break;

            case MEMBUF_SCOM_RMW:
                rc = membuf_scom_rmw(i_config,
                                      i_parms->scomList[i].instanceNumber,
                                      i_parms->scomList[i].scom,
                                      i_parms->scomList[i].mask,
                                      &(i_parms->scomList[i].data));
                break;

            case MEMBUF_SCOM_READ_VECTOR:
                rc = membuf_get_scom_vector(i_config,
                                             i_parms->scomList[i].scom,
                                             i_parms->scomList[i].pData
                                            );
                break;

            case MEMBUF_SCOM_WRITE_ALL:
                rc = membuf_put_scom_all(i_config,
                                          i_parms->scomList[i].scom,
                                          i_parms->scomList[i].data);
                break;

            case MEMBUF_SCOM_RMW_ALL:
                rc = membuf_scom_rmw_all(i_config,
                                          i_parms->scomList[i].scom,
                                          i_parms->scomList[i].mask,
                                          i_parms->scomList[i].data);
                break;

            case MEMBUF_SCOM_MEMBUF_SYNC:
                if( i_config->membuf_type == MEMTYPE_CENTAUR)
                {
                    rc = centaur_throttle_sync(i_config);
                }
                else
                {
                    rc = ocmb_throttle_sync(i_config);
                }
                break;

            default:
                break;
        };
        if (rc)
        {
            break;
        }
    }
    i_parms->error.rc = rc;
}
