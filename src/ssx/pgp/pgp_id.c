// $Id: pgp_id.c,v 1.2 2014/02/03 01:30:35 daviddu Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/pgp_id.c,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pgp_id.h
/// \brief PgP chip and EC-level identification + chip configuration

#include "ssx.h"
#include "pgp_config.h"


// Note: These cached variables are all declared as 64 bits, noncacheable so
// that that they are also available as-is to PORE programs.

uint64_t G_node_id SECTION_ATTRIBUTE(".noncacheable") = 0;
uint64_t G_chip_id SECTION_ATTRIBUTE(".noncacheable") = 0;
uint64_t G_cfam_id SECTION_ATTRIBUTE(".noncacheable") = 0;
uint64_t G_cfam_chip_type SECTION_ATTRIBUTE(".noncacheable") = 0;
uint64_t G_cfam_ec_level SECTION_ATTRIBUTE(".noncacheable") = 0;

void
_pgp_get_ids(void)
{
    tpc_gp0_t gp0;
    tpc_device_id_t deviceId;
    cfam_id_t cfamId;

    getscom(TPC_GP0, &(gp0.value));
    G_node_id = gp0.fields.tc_node_id_dc;
    G_chip_id = gp0.fields.tc_chip_id_dc;

    getscom(TPC_DEVICE_ID, &(deviceId.value));
    G_cfam_id = cfamId.value = deviceId.fields.cfam_id;
    G_cfam_chip_type = cfamId.chipType;
    G_cfam_ec_level = (cfamId.majorEc << 4) | cfamId.minorEc;
}


uint8_t
node_id(void)
{
    return G_node_id;
}

uint8_t
chip_id(void)
{
    return G_chip_id;
}

uint32_t
cfam_id(void)
{
    return G_cfam_id;
}

uint8_t 
cfam_chip_type(void)
{
    return G_cfam_chip_type;
}

uint8_t
cfam_ec_level(void)
{
    return G_cfam_ec_level;
}


// The chiplet configuration is computed by doing a "select-mode" multicast to
// the all-functional-chiplets-core group.  Correctness here depends on the
// convention that the "select" bit number will always be 0.  We check just to
// be sure. Since this is called from initialization code there is no recourse
// here except to panic in the event of error.

// Note: Ex-chiplets start at chiplet 16 and are left-justified in the
// ChipConfig.
    

ChipConfig G_chip_configuration SECTION_ATTRIBUTE(".noncacheable") = 0;
uint64_t G_core_configuration SECTION_ATTRIBUTE(".noncacheable") = 0;

/// \bug This API currently only computes the core configuration.  It needs to
/// be extended to also compute the MC and Centaur configuration.
///
/// \bug in Simics we're doing this based on the PMC_CORE_DECONFIGURATION_REG
/// pending Simics support for the base pervasive functionality

void
_pgp_get_chip_configuration(void)
{
    if (SIMICS_ENVIRONMENT) {

        pmc_core_deconfiguration_reg_t pcdr;

        pcdr.value = in32(PMC_CORE_DECONFIGURATION_REG);
        G_chip_configuration = 
            ~((uint64_t)(pcdr.fields.core_chiplet_deconf_vector) << 48); 

    } else {

        uint64_t select, configuration;
        int rc;
    
        rc = getscom(0x000f0008, &select); /* TP CHIPLET SELECT */
        if (rc) SSX_PANIC(PGP_ID_SCOM_ERROR_SELECT);
        if (select != 0) SSX_PANIC(PGP_ID_SELECT_ERROR);
    
        rc = getscom(MC_ADDRESS(0x000f0012, 
                                MC_GROUP_EX_CORE, 
                                PCB_MULTICAST_SELECT), 
                     &configuration);
        if (rc) SSX_PANIC(PGP_ID_SCOM_ERROR_CONFIG);
    
        G_chip_configuration = (configuration << 16) & 0xffff000000000000ull;
    }

    G_core_configuration = G_chip_configuration & 0xffff000000000000ull;
}


uint32_t core_configuration(void)
{
    return G_core_configuration >> 32;
}

    

    

    
