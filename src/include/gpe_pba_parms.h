/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p9/common/pmlib/include/gpe_pba_parms.h $               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2019                        */
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
