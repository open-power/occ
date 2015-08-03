/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwpf/fapi/src/plat_utils.C $                          */
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
/**
 *  @file plat_utils.C
 *  @brief Implements fapi2 common utilities
 */

#include <stdint.h>
#include <plat_trace.H>
#include <return_code.H>
#include <error_info.H>

namespace fapi2
{
    ///
    /// @brief Log an error.
    ///
    void logError(
        fapi2::ReturnCode & io_rc,
        fapi2::errlSeverity_t i_sev = fapi2::FAPI2_ERRL_SEV_UNRECOVERABLE,
        bool i_unitTestError = false )
    {
        FAPI_DBG("logging 0x%lx.", uint64_t(io_rc));

        // Iterate over the vectors and output what is in them.
        const ErrorInfo* ei = io_rc.getErrorInfo();

        FAPI_DBG("ffdcs: %lu", ei->iv_ffdcs.size());
        for( auto i = ei->iv_ffdcs.begin(); i != ei->iv_ffdcs.end(); ++i )
        {
            uint32_t sz;
            (*i)->getData(sz);
            FAPI_DBG("\tid: 0x%x size %d", (*i)->getFfdcId(), sz);
        }

        FAPI_DBG("hwCallouts: %lu", ei->iv_hwCallouts.size());
        for( auto i = ei->iv_hwCallouts.begin(); i != ei->iv_hwCallouts.end();
             ++i )
        {
            FAPI_DBG("\thw: %d pri %d target: 0x%lx",
                     (*i)->iv_hw, (*i)->iv_calloutPriority,
                     (*i)->iv_refTarget.get());
        }

        FAPI_DBG("procedureCallouts: %lu", ei->iv_procedureCallouts.size());
        for( auto i = ei->iv_procedureCallouts.begin();
             i != ei->iv_procedureCallouts.end(); ++i )
        {
            FAPI_DBG("\tprocedure: %d pri %d",
                     (*i)->iv_procedure, (*i)->iv_calloutPriority);
        }

        FAPI_DBG("busCallouts: %lu", ei->iv_busCallouts.size());
        for( auto i = ei->iv_busCallouts.begin(); i != ei->iv_busCallouts.end();
             ++i )
        {
            FAPI_DBG("\tbus: t1: 0x%lx t2: 0x%lx pri: %d",
                     (*i)->iv_target1.get(), (*i)->iv_target2.get(),
                     (*i)->iv_calloutPriority);
        }


        FAPI_DBG("cdgs: %lu", ei->iv_CDGs.size());
        for( auto i = ei->iv_CDGs.begin(); i != ei->iv_CDGs.end(); ++i )
        {
            FAPI_DBG("\ttarget: 0x%lx co: %d dc: %d gard: %d pri: %d",
                     (*i)->iv_target.get(),
                     (*i)->iv_callout,
                     (*i)->iv_deconfigure,
                     (*i)->iv_gard,
                     (*i)->iv_calloutPriority);

        }

        FAPI_DBG("childrenCDGs: %lu", ei->iv_childrenCDGs.size());
        for( auto i = ei->iv_childrenCDGs.begin();
             i != ei->iv_childrenCDGs.end(); ++i )
        {
            FAPI_DBG("\tchildren: parent 0x%lx co: %d dc: %d gard: %d pri: %d",
                     (*i)->iv_parent.get(),
                     (*i)->iv_callout,
                     (*i)->iv_deconfigure,
                     (*i)->iv_gard,
                     (*i)->iv_calloutPriority);
        }

        FAPI_DBG("traces: %lu", ei->iv_traces.size());
        for( auto i = ei->iv_traces.begin(); i != ei->iv_traces.end(); ++i )
        {
            FAPI_DBG("\ttraces: 0x%x", (*i)->iv_eiTraceId);
        }

        // Release the ffdc information now that we're done with it.
        io_rc.forgetData();

    }
};
