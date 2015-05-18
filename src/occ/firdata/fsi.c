/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/firdata/fsi.C $                                       */
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

#include <fsi.h>
#include <scom_util.h>
#include <native.h>

#define OPB_REG_CMD   0x00020010
#define OPB_REG_STAT  0x00020011
#define OPB_REG_RES   0x00020014
#define OPB_STAT_BUSY  0x0001000000000000 /**< 15 is the Busy bit */
#define NS_PER_MSEC (1000000ull)


/**
 * @brief  Poll for completion of a FSI operation, return data on read
 */
uint32_t poll_for_complete( void )
{
    int32_t rc = SUCCESS;

    enum { MAX_OPB_TIMEOUT_NS = 10*NS_PER_MSEC }; /*=10ms */

    /* poll for complete */
    uint64_t read_data = FSIFAIL;
    uint64_t elapsed_time_ns = 0;
    do
    {
        rc = xscom_read( OPB_REG_STAT, &read_data );
        if ( SUCCESS != rc )
        {
            return FSIFAIL;
        }

        /* check for completion or xscom error */
        /*   note: not checking for FSI errors */
        if ( (read_data & OPB_STAT_BUSY) == 0 )  /*not busy */
        {
            break;
        }
        else
        {
            read_data = FSIFAIL;
        }

        sleep( 10000 ); /*sleep for 10,000 ns */
        elapsed_time_ns += 10000;
    } while( elapsed_time_ns <= MAX_OPB_TIMEOUT_NS );

    return (uint32_t)read_data; /*data in the bottom half */
}

/**
 * @brief Read a FSI register
 */
uint32_t getfsi( SCOM_Trgt_t i_target, uint32_t i_address )
{
    int32_t rc = SUCCESS;

    uint32_t fsi_base = i_target.fsiBaseAddr;
    uint32_t fsi_addr = fsi_base | i_address;

    /* setup the OPB command register */
    /*  only supporting 4-byte access */
    uint64_t fsi_cmd = fsi_addr | 0x60000000; /*011=Read Full Word */
    fsi_cmd <<= 32; /* Command is in the upper word of the scom */

    /* Write the OPB command register to trigger the read */
    rc = xscom_write( OPB_REG_CMD, fsi_cmd );
    if ( SUCCESS != rc )
    {
        return FSIFAIL;
    }

    /* poll for complete and get the data back */
    uint32_t out_data = poll_for_complete();

    return out_data;
}


/**
 * @brief Write a FSI register
 */
void putfsi( SCOM_Trgt_t i_target,
             uint32_t i_address,
             uint32_t i_data )
{
    int32_t rc = SUCCESS;

    uint32_t fsi_base = i_target.fsiBaseAddr;
    uint32_t fsi_addr = fsi_base | i_address;

    /* setup the OPB command register */
    /*  only supporting 4-byte access */
    uint64_t fsi_cmd = fsi_addr | 0xE0000000; /* 111=Write Full Word */
    fsi_cmd <<= 32; /* Command is in the upper word of the scom */
    fsi_cmd |= i_data; /* Data is in the bottom 32-bits */

    /* Write the OPB command register to trigger the read */
    rc = xscom_write( OPB_REG_CMD, fsi_cmd );
    if ( SUCCESS != rc )
    {
        return;
    }

    /* poll for complete */
    poll_for_complete();

    return;
}

