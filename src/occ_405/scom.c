/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/scom.c $                                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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
#include <scom_util.h>

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
    int                         l_rc;
    errlHndl_t                  l_err = NULL;

    //P9 SCOM logic requires a target. However, if we're here, then it doesn't
    //matter which target we pass in, so long as isMaster is true. This will
    //allow to take the branch of code that schedules GPE scom job. See
    //src/occ_405/firdata/scom_util.c for more info.
    SCOM_Trgt_t l_tempTarget;
    l_tempTarget.type = TRGT_PROC;
    l_tempTarget.isMaster = TRUE;
    l_tempTarget.procUnitPos = 0;

    l_rc = SCOM_getScom(l_tempTarget, i_addr, o_data);

    if(l_rc)
    {
        //grab additional ffdc
        TRAC_ERR("getscom_ffdc: scom failed.  addr[%08x] rc[%08x]",
                 i_addr,
                 -l_rc);

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
    int                         l_rc;
    errlHndl_t                  l_err = NULL;

    //P9 SCOM logic requires a target. However, if we're here, then it doesn't
    //matter which target we pass in, so long as isMaster is true. This will
    //allow to take the branch of code that schedules GPE scom job. See
    //src/occ_405/firdata/scom_util.c for more info.
    SCOM_Trgt_t l_tempTarget;
    l_tempTarget.type = TRGT_PROC;
    l_tempTarget.isMaster = TRUE;
    l_tempTarget.procUnitPos = 0;

    l_rc = SCOM_putScom(l_tempTarget, i_addr, i_data);

    if(l_rc)
    {
        TRAC_ERR("putscom_ffdc: scom failed.  addr[%08x] rc[%08x]",
                 i_addr,
                 -l_rc);

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

