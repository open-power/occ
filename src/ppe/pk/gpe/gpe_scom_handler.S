/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/gpe/gpe_scom_handler.S $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2015
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file gpe_scom_handler.S
/// \brief Interrupt handler code for SCOM requests from the ppc405
///

        .nolist
#include "pk.h"
#include "occhw_shared_data.h"
        .list

        ## This function handles requests from the ppc405 to perform a getscom
        ## or putscom operation.
        ##
        ## The ppc405 must supply a request in the following format:
        ##
        ##  bytes   :   Content
        ##  -------------------
        ##  0-3     :   Status
        ##  4-7     :   cmd/address (bit 0 is r/!w bit)
        ##  8-15    :   data (supplied by ppc405 on writes)
        ##
        ## Register Usage:
        ##
        ## r3: SRAM base address
        ## r4: SCOM address
        ## r5: MSR manipulation
        ## d5: SCOM data
        ##
        ## NOTE: getscom requires 2 IC lines and 1 DC line
        ##       putscom requires 3 IC lines and 1 DC line
        ##       Both require the same number of instructions (16)
        .align 5
        .global gpe_scom_handler
gpe_scom_handler:
        
        ## Load the hardcoded base address of the scom request
        _liwa   %r3, OSD_GPE_SCOM_ADDR

        ## Load the scom address from SRAM
        lwz     %r4, OCCHW_SCOM_ADDR_OFFSET(%r3)

        ## Mask all SIB errors
        mfmsr   %r7
        _oriwa  %r5, %r7, MSR_SEM
        mtmsr   %r5
        
        ## Check bit 0 of the scom address to determine if this
        ## is a getscom or a putscom request
        bb0wi   %r4, 0, _do_putscom         #branch if bit 0 is 0

_do_getscom:
        ## the cmd bit was set, which means we're doing a getscom.
        ## need to clear the bit first.
        clrbwi  %r4, %r4, 0

        ## do the getscom
        lvd     %d5, 0(%r4)

        ## store the data into the request in SRAM
        stvd    %d5, OCCHW_SCOM_DATA_OFFSET(%r3)

_get_scom_status:

        ## status is in the MSR, copy it to the scom request in SRAM
        mfmsr   %r5
        stw     %r5, OCCHW_SCOM_STATUS_OFFSET(%r3)

        ## clear the IPI_SCOM interrupt:
        ## first, load the bit we want to clear into a register
        _liwa   %r3, OCCHW_IRQ_MASK32(OCCHW_IRQ_IPI_SCOM)
        
        ## then, store it to the OISR0_CLR address
        _stwi   %r3, %r4, OCB_OISR0_CLR

        ## restore the MSR as it was before we changed it
        mtmsr   %r7

        ## return
        blr

_do_putscom:
        
        ## load data from the request
        lvd     %d5, OCCHW_SCOM_DATA_OFFSET(%r3)

        ## do the putscom
        stvd    %d5, 0(%r4)

        b       _get_scom_status

    


