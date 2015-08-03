/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/firdata/native.C $                                    */
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

#include <native.h>
#include <scom.h>
#include <trac.h>

void sleep( SsxInterval i_nanoseconds )
{
    ssx_sleep(SSX_NANOSECONDS(i_nanoseconds));
}

int TRACE_XSCOM=0;

int32_t xscom_read( uint32_t i_address, uint64_t * o_data )
{
    int32_t rc = SUCCESS;

    *o_data = 0;

    rc = getscom_ffdc( i_address, o_data, NULL );
    if ( SUCCESS != rc )
    {
        TRAC_ERR( "SCOM error in xscom_read wrapper, rc=%d", rc );
    }

    if ( TRACE_XSCOM )
    {
        TRACFCOMP( "xscom_read(%08X)=%08X%08X", i_address,
                   (uint32_t)(*o_data>>32), (uint32_t)(*o_data) );
    }

    return rc;
}

int32_t xscom_write( uint32_t i_address, uint64_t i_data )
{
    int32_t rc = SUCCESS;

    rc = putscom_ffdc( i_address, i_data, NULL );
    if ( SUCCESS != rc )
    {
        TRAC_ERR( "SCOM error in xscom_write wrapper, rc=%d", rc );
    }

    if ( TRACE_XSCOM )
    {
        TRACFCOMP( "xscom_write(%08X)=%08X%08X", i_address,
                   (uint32_t)(i_data>>32), (uint32_t)i_data);
    }

    return rc;
}

