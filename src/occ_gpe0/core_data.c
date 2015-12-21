/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p9/procedures/lib/pm/core_data.c $                      */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* EKB Project                                                            */
/*                                                                        */
/* COPYRIGHT 2015                                                         */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* The source code for this program is not published or otherwise         */
/* divested of its trade secrets, irrespective of what has been           */
/* deposited with the U.S. Copyright Office.                              */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
/// \file core_data.c
/// \brief The GPE program that collect raw data for DTS and EMPATH
///
//  *HWP HWP Owner: Doug Gilbert <dgilbert@us.ibm.com>
//  *HWP FW Owner: Martha Broyles <mbroyles@us.ibm.com>
//  *HWP Team: PM
//  *HWP Level: 1
//  *HWP Consumed by: OCC
#include "core_data.h"
#include "ppe42_scom.h"
#include "p9_config.h"

uint32_t get_core_data(uint32_t i_core,
                       CoreData* o_data)
{
    uint32_t rc = 0;
    uint32_t size = sizeof(CoreData) / 8;
    uint64_t* ptr = (uint64_t*)o_data;
    uint32_t coreSelect = CHIPLET_CORE_ID(i_core);
    uint32_t quadSelect = CHIPLET_CACHE_ID((i_core / 4));

    uint32_t i;

    for(i = 0; i < size; ++i)
    {
        ptr[i] = 0;
    }

    dts_sensor_result_reg_t scom_data;

    rc = getscom(quadSelect, THERM_DTS_RESULT, &(scom_data.value));

    if(!rc)
    {
        // Pick the sensor reading closest to core
        // first two cores - use cache dts0
        // last two cores -  use cache dts1
        if(i_core & 0x00000002)
        {
            o_data->dts.cache.result = scom_data.half_words.reading[1];
        }
        else
        {
            o_data->dts.cache.result = scom_data.half_words.reading[0];
        }

        rc = getscom(coreSelect, THERM_DTS_RESULT, &(scom_data.value));

        if(!rc)
        {
            o_data->dts.core[0].result = scom_data.half_words.reading[0];
            o_data->dts.core[1].result = scom_data.half_words.reading[1];
        }
    }

    return rc;
}
