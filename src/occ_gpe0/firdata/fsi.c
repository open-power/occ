/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/firdata/fsi.c $                                  */
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

#include <fsi.h>
#include <scom_util.h>
#include <native.h>
#include <gpe_util.h>

#define OPB_REG_CMD   0x00020010
#define OPB_REG_STAT  0x00020011
#define OPB_REG_RES   0x00020014
#define OPB_STAT_BUSY  0x0001000000000000 /**< 15 is the Busy bit */
#define NS_PER_MSEC (1000000ull)


void fsi_recovery()
{
    TRAC_IMP(">>fsi_recovery");
    int32_t rc = SUCCESS;

    /* Clear out OPB error */
    uint64_t scom_data = 0;
    scom_data = 0x8000000000000000; /*0=Unit Reset*/
    rc |= xscom_write( OPB_REG_RES,  scom_data );
    rc |= xscom_write( OPB_REG_STAT, scom_data );

    /* Check if we have any errors left */
    rc |= xscom_read( OPB_REG_STAT, &scom_data );

    TRAC_IMP( "<<fsi_recovery: PIB2OPB Status after cleanup = %08X%08X (rc=%d)",
              (uint32_t)(scom_data >> 32), (uint32_t)scom_data, rc );
}

/**
 * @brief  Poll for completion of a FSI operation, return data on read
 */
int32_t poll_for_complete( uint32_t * o_val )
{
    int32_t rc = SUCCESS;

    enum { MAX_OPB_TIMEOUT_NS = 10*NS_PER_MSEC }; /*=10ms */

    *o_val = 0;

    uint64_t read_data = 0;
    uint64_t elapsed_time_ns = 0;
    do
    {
        rc = xscom_read( OPB_REG_STAT, &read_data );
        if ( SUCCESS != rc )
        {
            fsi_recovery(); /* Try to recover the engine. */
            return rc;
        }

        /* Check for completion. Note: not checking for FSI errors. */
        if ( (read_data & OPB_STAT_BUSY) == 0 ) break; /* Not busy */

        busy_wait( 10 ); /* sleep for 10,000 ns */
        elapsed_time_ns += 10000;

    } while ( elapsed_time_ns <= MAX_OPB_TIMEOUT_NS );

    if ( MAX_OPB_TIMEOUT_NS < elapsed_time_ns )
    {
        TRAC_ERR( "[poll_for_complete] FSI request timed out." );
        return FAIL;
    }

    *o_val = (uint32_t)read_data; /* Data in the bottom half. */

    return rc;
}

/**
 * @brief Read a FSI register
 */
int32_t getfsi( SCOM_Trgt_t* i_trgt, uint32_t i_addr, uint32_t * o_val )
{
    int32_t rc = SUCCESS;

    uint32_t fsi_addr = i_trgt->fsiBaseAddr | i_addr;

    /* setup the OPB command register */
    /*  only supporting 4-byte access */
    uint64_t fsi_cmd = fsi_addr | 0x60000000; /* 011=Read Full Word */
    fsi_cmd <<= 32; /* Command is in the upper word of the scom */

    /* Write the OPB command register to trigger the read */
    rc = xscom_write( OPB_REG_CMD, fsi_cmd );
    if ( SUCCESS != rc )
    {
        fsi_recovery(); /* Try to recover the engine. */
        return rc;
    }

    /* Poll for complete and get the data back. */
    rc = poll_for_complete( o_val );

    return rc;
}

/**
 * @brief Write a FSI register
 */
int32_t putfsi( SCOM_Trgt_t* i_trgt, uint32_t i_addr, uint32_t i_val )
{
    int32_t rc = SUCCESS;

    uint32_t fsi_addr = i_trgt->fsiBaseAddr | i_addr;

    /* setup the OPB command register */
    /*  only supporting 4-byte access */
    uint64_t fsi_cmd = fsi_addr | 0xE0000000; /* 111=Write Full Word */
    fsi_cmd <<= 32;   /* Command is in the upper word of the scom */
    fsi_cmd |= i_val; /* Data is in the bottom 32-bits */

    /* Write the OPB command register to trigger the read */
    rc = xscom_write( OPB_REG_CMD, fsi_cmd );
    if ( SUCCESS != rc )
    {
        fsi_recovery(); /* Try to recover the engine. */
        return rc;
    }

    /* Poll for complete */
    uint32_t junk = 0; // Not used.
    rc = poll_for_complete( &junk );

    return rc;
}

