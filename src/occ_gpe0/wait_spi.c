/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/wait_spi.c $                                      */
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


// This file contains a single function that deals with polling the
// xxxxx_ONGOING status bit in two spi scom registers to confirm completion
// of SPI transactions.
//
// In the near future, this function may be combined with additional spi
// utilities and in that case the filename will definitely change.



#include "pk.h"
#include "ppe42_scom.h"
#include "pss_constants.h"

//--------------------------------------------------------------------
// Name: wait_spi
//
// Description:  Read the specified register (SPIPSS_P2S_STATUS_REG
//               or SPIPSS_ADC_STATUS_REG), and and check if it p2s_ongoing
//               bit is 0 (operations done).  If not, wait
//               up to the timeout usec (~1usec per retry).
//               If still not clear, continue looping,
//               If error/reserved bits are set, a return code will be sent back
//
// Inputs:       timeout - # usec to wait for ongoing bit to clear
//               Register: SPIPSS_P2S_STATUS_REG or SPIPSS_ADC_STATUS_REG
//
// return:       0 -> Success: spi transaction completed within timeout limit
//               not 0 -> timeout: spi transaction did not complete within timeout
//               bits 0:7 are masked, and returned back for potential analysis
//                   of the reason that the transaction timed out
//--------------------------------------------------------------------

int wait_spi_completion(int reg, int timeout)
{
    int i;
    int rc;
    uint64_t status;

    if(reg != SPIPSS_P2S_STATUS_REG &&     // Only these SPI status registers tested
       reg != SPIPSS_ADC_STATUS_REG)       // for xxxx_ONGOING bits for now.
    {
        PK_TRACE("gpe0:wait_spi_completion failed: Invalid Register 0x%08x", reg);
        pk_halt();
    }
    
    for (i = 0; i< timeout; i++)           // Keep polling the xxxx_ONGOING bits for timeout
    {
        rc = getscom_abs(reg, &status);
        if(rc)
        {
            PK_TRACE("gpe0:wait_spi_completion failed with rc = 0x%08x", rc);
            pk_halt();
        }


        // bit zero is the P2s_ONGOING / HWCTRL_ONGOING
        // set to 1:   means operation is in progress (ONGOING)
        // reset to 0: means operation is complete
        if(status & 0x8000000000000000)
            break;
        pk_sleep(PK_MICROSECONDS(1));   // sleep for 1 microsecond before retry
    }
    
    if (i < timeout) // success: transaction completed before timeout limit
    {
        // REVIEW: It is true that the operation completed, but should we
        // check whether a reserved or error bit was set? ...
        // If this is possible, then we should set the ffdc to indicate that
        return 0;
        }
    else  //timeout:
          // P2s_ONGOING / HWCTRL_ONGOING bit is on, other status info too
          // return 32 MSB only, (only 8 are used now,
          // while the 56 LSB are reserved in P9
    {
        return (status >> 32);
    }
}
