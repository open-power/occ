/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/firdata/native.c $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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

#include <native.h>
#include <scom_trgt.h>
#include <scom_util.h>

void sleep( PkInterval i_nanoseconds )
{
    pk_sleep(PK_NANOSECONDS(i_nanoseconds));
}

int TRACE_XSCOM=0;

int32_t xscom_read( uint32_t i_address, uint64_t * o_data )
{
    int32_t rc = SUCCESS;
    *o_data = 0;

    //P9 SCOM logic requires a target. However, if we're here, then it doesn't
    //matter which target we pass in, so long as isMaster is true. This will
    //allow to take the branch of code that schedules GPE scom job. See
    //src/occ_gpe0/firdata/scom_util.c for more info.
    SCOM_Trgt_t l_tempTarget;
    l_tempTarget.type = TRGT_PROC;
    l_tempTarget.isMaster = TRUE;
    l_tempTarget.procUnitPos = 0;

    rc = SCOM_getScom(l_tempTarget, i_address, o_data);
    if ( SUCCESS != rc )
    {
        TRAC_ERR( "SCOM error in xscom_read wrapper, rc=%d", rc );
    }

    return rc;
}

int32_t xscom_write( uint32_t i_address, uint64_t i_data )
{
    int32_t rc = SUCCESS;

    //P9 SCOM logic requires a target. However, if we're here, then it doesn't
    //matter which target we pass in, so long as isMaster is true. This will
    //allow to take the branch of code that schedules GPE scom job. See
    //src/occ_gpe0/firdata/scom_util.c for more info.
    SCOM_Trgt_t l_tempTarget;
    l_tempTarget.type = TRGT_PROC;
    l_tempTarget.isMaster = TRUE;
    l_tempTarget.procUnitPos = 0;

    rc = SCOM_putScom(l_tempTarget, i_address, i_data);
    if ( SUCCESS != rc )
    {
        TRAC_ERR( "SCOM error in xscom_write wrapper, rc=%d", rc );
    }

    return rc;
}

