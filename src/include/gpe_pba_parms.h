/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p9/common/pmlib/include/gpe_pba_parms.h $               */
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
#if !defined(__GPE_PBA_PARMS_H__)
#define __GPE_PBA_PARMS_H__

#include <stdint.h>
#include "pba_register_addresses.h"
#include "pba_firmware_registers.h"

typedef struct
{

    /// The 32-bit OCI address of the PBA_SLVCTLn register to set up
    uint32_t slvctl_address;

    /// The slave id (0 - 3)
    uint32_t slave_id;
    /// An image of the relevant parts of the PBA_SLVCTLn register in effect
    /// for this procedure
    //pba_slvctln_t slvctl;
    pba_slvctln_t slvctl;

    /// The mask in effect for this update of the PBA_SLVCTL
    //pba_slvctln_t mask;
    pba_slvctln_t mask;

    /// The value to write to the PBA_SLVRST register to reset the slave
    //pba_slvrst_t slvrst;
    pba_slvrst_t slvrst;

    /// The bit to AND-poll to check for slave reset in progress
    //pba_slvrst_t slvrst_in_progress;
    pba_slvrst_t slvrst_in_progress;

} GpePbaParms;

#endif
