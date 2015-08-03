/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/scom.c $                                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

#include <occ_common.h>
#include "ssx.h"
#include "scom.h"
#include "trac.h"
#include "occ_service_codes.h"
#include "occ_sys_config.h"
#include "polling.h"

#define MAX_SCOM_FFDC_RETRIES 1

// Function Specification
//
// Name: getscom_ffdc
//
// Description: peforms a getscom that does not panic on error and will add
//              trace data and create a predictive error log with callouts.
//              If caller passes in NULL for o_errp, the error will be
//              committed internally.  Otherwise, o_errp will point to the
//              uncomitted error log.
//
// End Function Specification
int getscom_ffdc(uint32_t i_addr, uint64_t* o_data, errlHndl_t* o_errp)
{
    pmc_o2p_addr_reg_t          l_addr;
    pmc_o2p_ctrl_status_reg_t   l_status;
    int                         l_rc;
    int                         l_retries = 0;
    errlHndl_t                  l_err = NULL;

    while(l_retries <= MAX_SCOM_FFDC_RETRIES)
    {
        l_rc = _getscom(i_addr, o_data, SCOM_TIMEOUT);
        if(!l_rc)
        {
            break;
        }

        //_getscom returns immediately if the o2p interface was busy
        //wait to see if busy condition clears up
        if(l_rc == -SCOM_PROTOCOL_ERROR_GETSCOM_BUSY)
        {
            busy_wait(SCOM_TIMEOUT);
        }

        l_retries++;
    }

    if(l_rc)
    {
        //grab additional ffdc
        l_status.value = in32(PMC_O2P_CTRL_STATUS_REG);
        l_addr.value = in32(PMC_O2P_ADDR_REG);
        TRAC_ERR("getscom_ffdc: scom failed.  addr[%08x] rc[%08x] o2p_stat[%08x] o2p_addr[%08x]",
                 i_addr,
                 -l_rc,
                 l_status.value,
                 l_addr.value);

        /* @
         * @errortype
         * @moduleid    GETSCOM_FFDC_MID
         * @reasoncode  PROC_SCOM_ERROR
         * @userdata1   scom address
         * @userdata2   failure code
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Processor register read failure
         */
         l_err = createErrl(GETSCOM_FFDC_MID,                 //modId
                            PROC_SCOM_ERROR,                  //reasoncode
                            OCC_NO_EXTENDED_RC,               //Extended reason code
                            ERRL_SEV_PREDICTIVE,              //Severity
                            NULL,                             //Trace Buf
                            DEFAULT_TRACE_SIZE,               //Trace Size
                            i_addr,                           //userdata1
                            -l_rc);                           //userdata2

         //adding a processor callout should also cause data to be collected by PRDF component on FSP
         addCalloutToErrl(l_err,
                          ERRL_CALLOUT_TYPE_HUID,
                          G_sysConfigData.proc_huid,
                          ERRL_CALLOUT_PRIORITY_MED);
         addCalloutToErrl(l_err,
                          ERRL_CALLOUT_TYPE_COMPONENT_ID,
                          ERRL_COMPONENT_ID_FIRMWARE,
                          ERRL_CALLOUT_PRIORITY_HIGH);

         if(o_errp)
         {
             //caller will commit the error
             *o_errp = l_err;
         }
         else
         {
             //error committed internally
             commitErrl(&l_err);
         }
    }

    return l_rc;
}

// Function Specification
//
// Name: putscom_ffdc
//
// Description: peforms a putscom that does not panic on error and will add
//              trace data and create a predictive error log with callouts.
//              If caller passes in NULL for o_errp, the error will be
//              committed internally.  Otherwise, o_errp will point to the
//              uncomitted error log.
//
// End Function Specification
int putscom_ffdc(uint32_t i_addr, uint64_t i_data, errlHndl_t* o_errp)
{
    pmc_o2p_addr_reg_t          l_addr;
    pmc_o2p_ctrl_status_reg_t   l_status;
    int                         l_rc;
    int                         l_retries = 0;
    errlHndl_t                  l_err = NULL;

    while(l_retries <= MAX_SCOM_FFDC_RETRIES)
    {
        l_rc = _putscom(i_addr, i_data, SCOM_TIMEOUT);
        if(!l_rc)
        {
            break;
        }

        //_putscom returns immediately if the o2p interface was busy.  Instead, see if
        //it cleared after SCOM_TIMEOUT period.
        if(l_rc == -SCOM_PROTOCOL_ERROR_PUTSCOM_BUSY)
        {
            busy_wait(SCOM_TIMEOUT);
        }

        l_retries++;
    }

    if(l_rc)
    {
        //grab addtional ffdc
        l_status.value = in32(PMC_O2P_CTRL_STATUS_REG);
        l_addr.value = in32(PMC_O2P_ADDR_REG);
        TRAC_ERR("putscom_ffdc: scom failed.  addr[%08x] rc[%08x] o2p_stat[%08x] o2p_addr[%08x]",
                 i_addr,
                 -l_rc,
                 l_status.value,
                 l_addr.value);

        /* @
         * @errortype
         * @moduleid    PUTSCOM_FFDC_MID
         * @reasoncode  PROC_SCOM_ERROR
         * @userdata1   scom address
         * @userdata2   failure code
         * @userdata4   OCC_NO_EXTENDED_RC
         * @devdesc     Processor register write failure
         */
         l_err = createErrl(PUTSCOM_FFDC_MID,                 //modId
                            PROC_SCOM_ERROR,                  //reasoncode
                            OCC_NO_EXTENDED_RC,               //Extended reason code
                            ERRL_SEV_PREDICTIVE,              //Severity
                            NULL,                             //Trace Buf
                            DEFAULT_TRACE_SIZE,               //Trace Size
                            i_addr,                           //userdata1
                            -l_rc);                           //userdata2

         //adding a processor callout should also cause data to be collected by PRDF component on FSP
         addCalloutToErrl(l_err,
                          ERRL_CALLOUT_TYPE_HUID,
                          G_sysConfigData.proc_huid,
                          ERRL_CALLOUT_PRIORITY_MED);
         addCalloutToErrl(l_err,
                          ERRL_CALLOUT_TYPE_COMPONENT_ID,
                          ERRL_COMPONENT_ID_FIRMWARE,
                          ERRL_CALLOUT_PRIORITY_HIGH);
         if(o_errp)
         {
             //caller will commit the error
             *o_errp = l_err;
         }
         else
         {
             //commit error internally
             commitErrl(&l_err);
         }
    }

    return l_rc;
}

