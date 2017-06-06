/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/firdata/pnor_util.c $                             */
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

/* Interfaces to write into PNOR */

#include <native.h>
#include <pnor_mboxdd.h>
#include <ecc.h>
#include <pnor_util.h>
#include <norflash.h>

/*================================================================= */
/* The offset of the next byte to write */
uint32_t g_next_byte = 0xFFFFFFFF;
/* Size of the FIRDATA section of PNOR */
uint32_t g_pnor_size = 0;
/* Global PNOR Mbox object */
pnorMbox_t* g_pnorMbox;

/* Cache to queue up PNOR writes */
uint8_t g_write_cache[PAGE_PROGRAM_BYTES];
/* Current position of data inside write cache */
size_t g_write_cache_index = 0;

/**
 * @brief Write 8 bytes of data into PNOR starting
 */
int32_t pnor_write_8B( uint64_t i_data )
{
    int32_t rc = SUCCESS;

    if ( (g_next_byte == 0xFFFFFFFF) || /* initialized data */
         ((g_next_byte + g_pnor_size) < (g_next_byte + 9)) ) /* make sure there is room */
    {
        TRACFCOMP("pnor_write_8B> g_next_byte=%.8X, g_pnor_size=%.8X",g_next_byte,g_pnor_size);
        /* must have been some error in the prep */
        return FAIL;
    }

    /* Create 9-byte ECC-ified version */
    uint8_t data9[9];
    injectECC( (uint8_t*)(&i_data), 8, data9 );

    /* Copy data into the write cache until we queue up
       a big chunk of data to write.  This is more efficient
       and avoids handling the write boundary of the PP
       command internally. */
    size_t cpsz = 9;
    if( (g_write_cache_index + cpsz) > PAGE_PROGRAM_BYTES )
    {
        cpsz = PAGE_PROGRAM_BYTES - g_write_cache_index;
    }
    memcpy( &(g_write_cache[g_write_cache_index]), data9, cpsz );
    g_write_cache_index += cpsz;

    /* Write a complete chunk into the flash */
    if( g_write_cache_index == PAGE_PROGRAM_BYTES )
    {
        errorHndl_t tmp = writeFlash( &g_pnorMbox,
                                      g_next_byte,
                                      PAGE_PROGRAM_BYTES,
                                      g_write_cache );
        if ( NO_ERROR != tmp )
        {
            TRACFCOMP("pnor_write_8B> writeFlash failed");
            /* hit an error, stop any more writes from happening */
            g_next_byte = 0xFFFFFFFF;
            g_pnor_size = 0;
            return FAIL;
        }
        g_next_byte += PAGE_PROGRAM_BYTES;
        memset( g_write_cache, 0xFF, PAGE_PROGRAM_BYTES );
        g_write_cache_index = 0;

        /* Handle the overflow */
        if( (9 - cpsz) > 0 )
        {
            memcpy( &(g_write_cache[0]), &(data9[cpsz]), 9 - cpsz );
            g_write_cache_index = 9 - cpsz;
        }
    }

    return rc;
}


/**
 * @brief Perform any necessary operations to prepare
 *        the PNOR hw/code for writing
 */
errorHndl_t pnor_prep( HOMER_PnorInfo_t* i_pnorInfo )
{
    errorHndl_t l_err = NO_ERROR;

    /* Figure out where to start */
    TRACFCOMP("FIRDATA is at %.8X..%.8X", i_pnorInfo->pnorOffset, i_pnorInfo->pnorOffset+i_pnorInfo->pnorSize );
    g_next_byte = i_pnorInfo->pnorOffset;
    g_pnor_size = i_pnorInfo->pnorSize;
    memset( g_write_cache, 0xFF, PAGE_PROGRAM_BYTES );

    /* Can we rely on skiboot leaving things in a good state? */
    l_err = hwInit(&g_pnorMbox);
    if( l_err )
    {
        TRACFCOMP("hwInit failed");
        /* hit an error, stop any writes from happening */
        g_next_byte = 0xFFFFFFFF;
        g_pnor_size = 0;
    }

    return l_err;
}

/*------------------------------------------------------------------------------ */

int32_t PNOR_writeFirData( HOMER_PnorInfo_t i_pnorInfo,
                           uint8_t * i_buf, uint32_t i_bufSize )
{
    int32_t rc = SUCCESS;
    TRACFCOMP(">>PNOR_writeFirData");

    do
    {
        /* Initialize the PNOR data. */
        errorHndl_t l_err = pnor_prep( &i_pnorInfo );
        if( l_err )
        {
            TRACFCOMP("pnor_prep failed");
            rc = FAIL;
            break; /*nothing more to do here*/
        }

        uint32_t idx = 0;
        uint64_t dataChunk  = 0;
        size_t sz_dataChunk = sizeof(uint64_t);

        /* Add PNOR data 8 bytes at a time. */
        for ( idx = 0; idx < i_bufSize; idx += sz_dataChunk )
        {
            memcpy( &dataChunk, &i_buf[idx], sz_dataChunk );

            rc = pnor_write_8B( dataChunk );
            if ( SUCCESS != rc )
            {
                TRACFCOMP( "pnor_write_8B() failed during FIR write" );
                break;
            }
        }
        if ( SUCCESS != rc ) break;

        /* Add any extra bytes if they exist at the end of the buffer. */
        if ( idx != i_bufSize )
        {
            uint32_t extraBytes = idx - i_bufSize;

            dataChunk = 0;
            memcpy( &dataChunk, &i_buf[idx], extraBytes );

            rc = pnor_write_8B( dataChunk );
            if ( SUCCESS != rc )
            {
                TRACFCOMP( "pnor_write_8B() failed during blank fill" );
                break;
            }
        }

        /* Fill the rest of the page with good ECC */
        for ( idx = i_bufSize;
              idx < i_pnorInfo.pnorSize;
              idx += sz_dataChunk )
        {
            dataChunk = 0xFFFFFFFFFFFFFFFFull;
            rc = pnor_write_8B( dataChunk );
            if ( SUCCESS != rc )
            {
                TRACFCOMP( "pnor_write_8B() failed during ECC fill" );
                break;
            }
        }

    } while (0);

    TRACFCOMP("<<PNOR_writeFirData");
    return rc;
}

