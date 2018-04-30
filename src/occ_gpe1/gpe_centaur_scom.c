/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p9/procedures/lib/pm/centaur_scom.c $                   */
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
#include <stdint.h>
#include "gpe_centaur.h"
#include "gpe_pba_cntl.h"
#include "ppe42_scom.h"
#include "ppe42.h"
#include "pba_register_addresses.h"
#include "ppe42_msr.h"

/**
 * @file centaur_scom
 * @brief scom access from gpe to a centaur
 */

/**
 * Setup the PBASLVCTLN extended address and calculate the OCI scom address
 * @param[in] PBA base address
 * @param[in] The Centaur scom address
 * @returns the OCI address to scom the centaur
 * @Post The extended address field in the PBASLVCNT is set
 */
int centaur_scom_setup(CentaurConfiguration_t* i_config,
                            uint32_t i_centaur_instance,
                            uint32_t i_scom_address,
                            uint32_t *o_oci_addr)
{
    int rc = 0;
#if defined(__USE_PBASLV__)
   pba_slvctln_t slvctln;
#endif
    uint64_t pb_addr = i_config->baseAddress[i_centaur_instance];

    // Break address into componets
    uint32_t local = i_scom_address & 0x00001fff;
    uint32_t port  = i_scom_address & 0x000f0000;
    uint32_t slave = i_scom_address & 0x03000000;
    uint32_t multi = i_scom_address & 0xc0000000;

    // compress to 21 bits for P9
    uint32_t scom_address =
        local +
        (port >> 3) +
        (slave >> 7) +
        (multi >> 11);

    // P9: Turn on bit 38 to indicate OCC
    pb_addr  |= 0x0000000002000000ull;
    pb_addr  |= ((uint64_t)scom_address << 3);

#if defined(__USE_PBASLV__)
    // put bits 23:36 of address into slvctln extended addr
    PPE_LVD((i_config->scomParms).slvctl_address, slvctln.value);
    slvctln.fields.extaddr = pb_addr >> 27;
    PPE_STVD((i_config->scomParms).slvctl_address, slvctln.value);
#else
    // HW bug work-around
    {
        // workaround - don't use extraddr - use pbabar.
        uint64_t barMsk = 0;
        // put the PBA in the BAR
        rc = putscom_abs(PBA_BARN(PBA_BAR_CENTAUR), pb_addr);
        if(rc)
        {
            PK_TRACE("centaur_scom_setup. putscom fail on PBABAR."
                     " rc = %d",rc);
        }
        else
        {
            rc = putscom_abs(PBA_BARMSKN(PBA_BAR_CENTAUR), barMsk);
            if(rc)
            {
                PK_TRACE("centaur_scom_setup. putscom fail on PBABARMSK"
                         " rc = %d",rc);
            }
        }
    }
#endif
    // make oci address
    *o_oci_addr = (uint32_t)(pb_addr & 0x07ffffffull);

    // upper nibble is PBA region and BAR_SELECT
    *o_oci_addr  |= ((PBA_BAR_CENTAUR | 0x8) << 28);
    PK_TRACE_DBG("Centaur OCI scom addr: %08x",*o_oci_addr);
    return rc;
}

int centaur_sensorcache_setup(CentaurConfiguration_t* i_config,
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
        // put the PBA in the BAR
        rc = putscom_abs(PBA_BARN(PBA_BAR_CENTAUR), pb_addr);
        if (rc)
        {
            PK_TRACE("centaur_sensorcache_setup: putscom fail on PBABAR,"
                     " rc = %d",rc);
        }
        else
        {
            rc = putscom_abs(PBA_BARMSKN(PBA_BAR_CENTAUR), barMsk);
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
    *o_oci_addr |= ((PBA_BAR_CENTAUR | 0x8) << 28);

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

// Get the most severe rc from the sibrca field in the msr
int rc_from_sibrc()
{
    int rc = 0;
    uint32_t sibrca = (mfmsr() & 0x0000007f);
    if(sibrca)
    {
        uint32_t mask = 1;
        rc = 7;
        for(; mask != 0x00000080; mask <<=1)
        {
            if( mask & sibrca )
            {
                break;
            }
            --rc;
        }
    }
    return rc;
}

// Get data from each existing centaur.
int centaur_get_scom_vector(CentaurConfiguration_t* i_config,
                            uint32_t i_scom_address,
                            uint64_t* o_data)
{
    int rc = 0;
    int instance = 0;
    uint64_t pba_slvctln_save;

    pbaslvctl_reset(&(i_config->scomParms));
    pba_slvctln_save = pbaslvctl_setup(&(i_config->scomParms));

    // clear SIB errors in MSR
    mtmsr((mfmsr() & ~(MSR_SIBRC | MSR_SIBRCA)));

    for(instance = 0; instance < OCCHW_NCENTAUR; ++instance)
    {
        if( CHIP_CONFIG_CENTAUR(instance) & (i_config->config))
        {
            uint32_t oci_addr;
            rc = centaur_scom_setup(i_config,
                                    instance,
                                    i_scom_address,
                                    &oci_addr);

            if(rc)
            {
                // Already traced.
                break;
            }
            // read centaur scom
            PPE_LVD(oci_addr, *o_data);
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

    if(!rc)
    {
        rc = rc_from_sibrc();
    }
    return rc;
}

int centaur_get_scom(CentaurConfiguration_t* i_config,
                     int i_centaur_instance,
                     uint32_t i_scom_address,
                     uint64_t* o_data)
{
    int rc = 0;
    uint32_t oci_addr;
    uint64_t pba_slvctln_save;

    pbaslvctl_reset(&(i_config->scomParms));
    pba_slvctln_save = pbaslvctl_setup(&(i_config->scomParms));

    rc = centaur_scom_setup(i_config,
                            i_centaur_instance,
                            i_scom_address,
                            &oci_addr);

    if( !rc && (CHIP_CONFIG_CENTAUR(i_centaur_instance) & (i_config->config)))
    {
        // read centaur scom
        rc = getscom_abs(oci_addr, o_data);
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


// Write all configured centaur with the same data
int centaur_put_scom_all(CentaurConfiguration_t* i_config,
                         uint32_t i_scom_address,
                         uint64_t i_data)
{
    int rc = 0;
    int instance = 0;
    uint64_t pba_slvctln_save;

    pbaslvctl_reset(&(i_config->scomParms));
    pba_slvctln_save = pbaslvctl_setup(&(i_config->scomParms));

    // clear SIB errors in MSR
    mtmsr((mfmsr() & ~(MSR_SIBRC | MSR_SIBRCA)));

    for(instance = 0; instance < OCCHW_NCENTAUR; ++instance)
    {
        if( CHIP_CONFIG_CENTAUR(instance) & (i_config->config))
        {
            uint32_t oci_addr;
            rc = centaur_scom_setup(i_config,
                                    instance,
                                    i_scom_address,
                                    &oci_addr);

            if(rc)
            {
                // Already traced in centaur_scom_setup
                break;
            }

            // centaur scom
            PPE_STVD(oci_addr, i_data);
        }
    }

    // gpe_pba_cntl function?
    pbaslvctl_reset(&(i_config->scomParms));
    PPE_STVD((i_config->scomParms).slvctl_address, pba_slvctln_save);

    if(!rc)
    {
        rc = rc_from_sibrc();
    }
    return rc;
}

int centaur_put_scom(CentaurConfiguration_t* i_config,
                     int i_centaur_instance,
                     uint32_t i_scom_address,
                     uint64_t i_data)
{
    int rc = 0;
    uint32_t oci_addr;
    uint64_t pba_slvctln_save;

    pbaslvctl_reset(&(i_config->scomParms));
    pba_slvctln_save = pbaslvctl_setup(&(i_config->scomParms));

    rc = centaur_scom_setup(i_config,
                            i_centaur_instance,
                            i_scom_address,
                            &oci_addr);

    if(!rc)
    {
        if(CHIP_CONFIG_CENTAUR(i_centaur_instance) & (i_config->config))
        {
            // write centaur scom
            rc = putscom_abs(oci_addr, i_data);
        }
        else
        {
            rc = CENTAUR_INVALID_SCOM;
        }
    }

    // gpe_pba_cntl function?
    pbaslvctl_reset(&(i_config->scomParms));
    PPE_STVD((i_config->scomParms).slvctl_address, pba_slvctln_save);

    return rc;
}

// write x
int centaur_scom_rmw(CentaurConfiguration_t* i_config,
                     int i_centaur_instance,
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

    rc = centaur_scom_setup(i_config,
                            i_centaur_instance,
                            i_scom_address,
                            &oci_addr);
    if(!rc)
    {

        rc = getscom_abs(oci_addr, &data64);
        if(!rc)
        {
            data64 &= (i_mask ^ 0xffffffffffffffffull);
            data64 |= *i_data;

            rc = putscom_abs(oci_addr, data64);
        }
    }

    pbaslvctl_reset(&(i_config->scomParms));
    PPE_STVD((i_config->scomParms).slvctl_address, pba_slvctln_save);

    return rc;
}


int centaur_scom_rmw_all(CentaurConfiguration_t* i_config,
                         uint32_t i_scom_address,
                         uint64_t i_mask,
                         uint64_t i_data)
{
    int rc = 0;
    int instance = 0;
    uint64_t pba_slvctln_save;

    pbaslvctl_reset(&(i_config->scomParms));
    pba_slvctln_save = pbaslvctl_setup(&(i_config->scomParms));

    // clear SIB errors in MSR
    mtmsr((mfmsr() & ~(MSR_SIBRC | MSR_SIBRCA)));

    for(instance = 0; (instance < OCCHW_NCENTAUR); ++instance)
    {
        if( CHIP_CONFIG_CENTAUR(instance) & (i_config->config))
        {
            uint64_t data64;
            uint32_t oci_addr;
            rc = centaur_scom_setup(i_config,
                                    instance,
                                    i_scom_address,
                                    &oci_addr);
            if(rc)
            {
                // Already traced in centaur_scom_setup
                break;
            }

            PPE_LVD(oci_addr, data64);
            data64 &= (i_mask ^ 0xffffffffffffffffull);
            data64 |= i_data;
            PPE_STVD(oci_addr, data64);

            pbaslvctl_reset(&(i_config->scomParms));
        }
    }

    PPE_STVD((i_config->scomParms).slvctl_address, pba_slvctln_save);

    if(!rc)
    {
        rc = rc_from_sibrc();
    }
    return rc;
}


// read centaur data sensor cache
int centaur_get_mem_data(CentaurConfiguration_t* i_config,
                         CentaurGetMemDataParms_t* i_parms)
{
    int rc = 0;
    uint32_t oci_addr = 0;
    uint64_t pba_slvctln_save;
    uint64_t data64 = 0;

    i_parms->error.rc = CENTAUR_GET_MEM_DATA_DIED;

    pbaslvctl_reset(&(i_config->dataParms));
    pba_slvctln_save = pbaslvctl_setup(&(i_config->dataParms));

    if(i_parms->collect != -1)
    {
        if((i_parms->collect >= OCCHW_NCENTAUR) ||
           (0 == (CHIP_CONFIG_CENTAUR(i_parms->collect) & (i_config->config))))
        {
            rc = CENTAUR_GET_MEM_DATA_COLLECT_INVALID;
        }
        else
        {
            rc = centaur_sensorcache_setup(i_config, i_parms->collect,&oci_addr);

            if(!rc)
            {
                // Read 128 bytes from centaur cache
                int i;
                for(i = 0; i < 128; i += 8)
                {
                    PPE_LVDX(oci_addr, i, data64);
                    PPE_STVDX((i_parms->data), i, data64);
                }
            }
        }
    }

    if(!rc && i_parms->update != -1)
    {
        if((i_parms->update >= OCCHW_NCENTAUR) ||
           (0 == (CHIP_CONFIG_CENTAUR(i_parms->update) & (i_config->config))))
        {
            rc = CENTAUR_GET_MEM_DATA_UPDATE_INVALID;
        }
        else
        {
            rc = centaur_sensorcache_setup(i_config, i_parms->update,&oci_addr);

            if(!rc)
            {
                // Writing a zero to this address tells the centaur to update
                // the sensor cache.
                data64 = 0;
                PPE_STVD(oci_addr, data64);
            }
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
            rc = check_channel_chkstp(instance);
        }
    }


    i_parms->error.rc = rc;
    return rc;
}


int centaur_scom_sync(CentaurConfiguration_t* i_config)
{
    uint64_t data;
    int rc = 0;
    do
    {
        rc = getscom_abs(i_config->mcSyncAddr,&data);
        if (rc)
        {
            PK_TRACE("centaur_scom_sync: getscom failed. rc = %d",rc);
            break;
        }

        data &= ~MCS_MCSYNC_SYNC_GO;

        rc = putscom_abs(i_config->mcSyncAddr, data);
        if (rc)
        {
            PK_TRACE("centaur_scom_sync: reset sync putscom failed. rc = %d",rc);
            break;
        }

        data |= MCS_MCSYNC_SYNC_GO;

        rc = putscom_abs(i_config->mcSyncAddr, data);
        if (rc)
        {
            PK_TRACE("centaur_scom_sync: set sync putscom failed. rc = %d",rc);
            break;
        }
    } while (0);

    return rc;
}


// CentaurConfiguration needs to be setup before this is called
void gpe_scom_centaur(CentaurConfiguration_t* i_config,
                      CentaurScomParms_t* i_parms)
{
    int i;
    int rc = 0;
    mtmsr((mfmsr() & ~(MSR_SIBRC | MSR_SIBRCA)) | MSR_SEM);

    for(i = 0; i < i_parms->entries; ++i)
    {
        switch(i_parms->scomList[i].commandType)
        {
            case CENTAUR_SCOM_NOP:
                break;

            case CENTAUR_SCOM_READ:
                rc =centaur_get_scom(i_config,
                                     i_parms->scomList[i].instanceNumber,
                                     i_parms->scomList[i].scom,
                                     &(i_parms->scomList[i].data));
                break;

            case CENTAUR_SCOM_WRITE:
                rc = centaur_put_scom(i_config,
                                      i_parms->scomList[i].instanceNumber,
                                      i_parms->scomList[i].scom,
                                      i_parms->scomList[i].data);
                break;

            case CENTAUR_SCOM_RMW:
                rc = centaur_scom_rmw(i_config,
                                      i_parms->scomList[i].instanceNumber,
                                      i_parms->scomList[i].scom,
                                      i_parms->scomList[i].mask,
                                      &(i_parms->scomList[i].data));
                break;

            case CENTAUR_SCOM_READ_VECTOR:
                rc = centaur_get_scom_vector(i_config,
                                             i_parms->scomList[i].scom,
                                             i_parms->scomList[i].pData
                                            );
                break;

            case CENTAUR_SCOM_WRITE_ALL:
                rc = centaur_put_scom_all(i_config,
                                          i_parms->scomList[i].scom,
                                          i_parms->scomList[i].data);
                break;

            case CENTAUR_SCOM_RMW_ALL:
                rc = centaur_scom_rmw_all(i_config,
                                          i_parms->scomList[i].scom,
                                          i_parms->scomList[i].mask,
                                          i_parms->scomList[i].data);
                break;

            case CENTAUR_SCOM_CENTAUR_SYNC:
                rc = centaur_scom_sync(i_config);
                break;

            default:
                break;
        };
        i_parms->error.rc = rc;
        if (rc)
        {
            break;
        }
    }
}
