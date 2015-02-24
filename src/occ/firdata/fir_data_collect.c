/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/firdata/fir_data_collect.c $                          */
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


#include <fir_data_collect.h>
#include <scom.h>
#include <occ_service_codes.h>
#include <errl.h>
#include "tpc_firmware_registers.h"
#include "tpc_register_addresses.h"
#include <trac.h>
#include <homer.h>

FIR_HEAP_BUFFER(uint8_t G_fir_heap[FIR_HEAP_SECTION_SIZE]);
FIR_PARMS_BUFFER(uint8_t G_fir_data_parms[FIR_PARMS_SECTION_SIZE]);
uint32_t G_fir_master = FIR_OCC_NOT_FIR_MASTER;

/*
 * Function Specification
 *
 * Name: fir_data_collect
 *
 * Description: Collects FIR data on checkstop.
 *
 * End Function Specification
 */
void fir_data_collect(void)
{
    int32_t l_rc = 0;

    // Homer data section and size
    uint8_t *l_hBuf = FIR_PARMS_SECTION_BASE_ADDRESS;
    uint32_t l_hBufSize = HOMER_FIR_PARM_SIZE;
    // PNOR working buffer in SRAM and size
    uint8_t *l_pBuf = FIR_HEAP_SECTION_BASE_ADDRESS;
    uint32_t l_pBufSize = FIR_HEAP_SECTION_SIZE;

    l_rc = FirData_captureCsFirData(l_hBuf,
                                    l_hBufSize,
                                    l_pBuf,
                                    l_pBufSize);

    // Trace the rc only, error logs cannot be collected in this state
    TRAC_IMP("Checkstop FIR data capture completed with rc=%d", l_rc);
}


/*
 * Function Specification
 *
 * Name: pnor_access_ok
 *
 * Description: Determines if it is ok for this OCC to access the PNOR.
 *
 * End Function Specification
 */
bool pnor_access_allowed(void)
{
    /* BMC ownership of the PNOR is indicated by bit 18 in TPC_GP0 */
    int         l_rc = 0;
    tpc_gp0_t   l_tp_gp0_read;
    bool        l_access_allowed = FALSE;

    l_tp_gp0_read.words.high_order = 0x00000000;
    l_tp_gp0_read.words.low_order = 0x00000000;

    l_rc = getscom_ffdc(TPC_GP0, (uint64_t *)&l_tp_gp0_read, NULL);

    if (l_rc == 0)
    {
        if ((l_tp_gp0_read.words.high_order & TPC_GP0_BIT18_PNOR_OWNER_MASK) == 0)
        {
            TRAC_INFO("PNOR access allowed at this time");
            l_access_allowed = TRUE;
        }
        else
        {
            TRAC_INFO("PNOR access NOT allowed at this time, tpc_gp0.hi = 0x%08x",
                     l_tp_gp0_read.words.high_order);

            /* @
             * @errortype
             * @moduleid    FIR_DATA_MID
             * @reasoncode  INTERNAL_FAILURE
             * @userdata1   TPC_GP0 high word
             * @userdata4   ERC_PNOR_OWNERSHIP_NOT_AVAILABLE
             * @devdesc     PNOR access not allowed at this time.
             */
            errlHndl_t l_errl = createErrl(
                        FIR_DATA_MID,                   /*ModId */
                        INTERNAL_FAILURE,               /*Reasoncode */
                        ERC_PNOR_OWNERSHIP_NOT_AVAILABLE,    /*Extended reasoncode */
                        ERRL_SEV_INFORMATIONAL,         /*Severity */
                        NULL,                           /*Trace Buf */
                        DEFAULT_TRACE_SIZE,             /*Trace Size */
                        l_tp_gp0_read.words.high_order, /*Userdata1 */
                        0                               /*Userdata2 */
                        );

            /* Commit log */
            commitErrl(&l_errl);
        }
    }
    else
    {
        /* getscom failure */
        TRAC_ERR("TPC_GP0 getscom failure rc = 0x%08x", -l_rc );

        /* @
         * @errortype
         * @moduleid    FIR_DATA_MID
         * @reasoncode  INTERNAL_HW_FAILURE
         * @userdata1   getscom failure rc
         * @userdata4   ERC_GETSCOM_TPC_GP0_FAILURE
         * @devdesc     Failure determining PNOR ownership.  Cannot read TPC_GP0.
         */
        errlHndl_t l_errl = createErrl(
                    FIR_DATA_MID,                   /*ModId */
                    INTERNAL_HW_FAILURE,            /*Reasoncode */
                    ERC_GETSCOM_TPC_GP0_FAILURE,    /*Extended reasoncode */
                    ERRL_SEV_PREDICTIVE,            /*Severity */
                    NULL,                           /*Trace Buf */
                    DEFAULT_TRACE_SIZE,             /*Trace Size */
                    l_rc,                           /*Userdata1 */
                    0                               /*Userdata2 */
                    );

        /* Callout firmware */
        addCalloutToErrl(l_errl,
                         ERRL_CALLOUT_TYPE_COMPONENT_ID,
                         ERRL_COMPONENT_ID_FIRMWARE,
                         ERRL_CALLOUT_PRIORITY_HIGH);

        /* Commit log */
        commitErrl(&l_errl);
    }

    return l_access_allowed;
}


