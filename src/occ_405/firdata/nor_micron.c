/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/firdata/nor_micron.C $                                */
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

#include "norflash.h"
#include "sfc_ast2400.h"
#include <native.h>




/**
 * @brief Check flag status bit on Micron NOR chips
 *        Some versions of Micron parts require the Flag
 *        Status register be read after a write or erase operation,
 *        otherwise all future operations won't work..
 */
errorHndl_t micronFlagStatus( Sfc_t* i_sfc )
{
    errorHndl_t l_err = NO_ERROR;
    /*TRACDCOMP( g_trac_pnor, "micronFlagStatus>" );  */

    do {
        /*Read Micron 'flag status' register */
        uint8_t flagstat = 0;
        l_err = sendSpiCmd( i_sfc,
                            SPI_MICRON_FLAG_STAT,
                            NO_ADDRESS,
                            0, NULL,
                            sizeof(flagstat), &flagstat );
        if(l_err) { break; }

        /*TRACDCOMP(g_trac_pnor,  */
        /*          "micronFlagStatus> (0x%.2X)", */
        /*          flagstat); */

        /* check for ready and no errors */
        /* bit 0 = ready, bit 2=erase fail, bit 3=Program (Write) failure */
        if( (flagstat & 0xB0) != 0x80)
        {
            /*TRACFCOMP(g_trac_pnor, "micronFlagStatus> Error or timeout from Micron Flag Status Register (0x%.2X)", flagstat);  */
            l_err = -1;
            break;
        }


    }while(0);

    return l_err;

}

