/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/sample/pk_scom.c $                                */
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
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file   pk_scom.c
/// \brief  Lowest level SCOM definitions. 
///
/// A fapi-level SCOM should call these functions.
///
/// Todo:
/// - Poll PCB master for SCOM completion.
/// - Return error code of SCOM fails.
/// - Return error code if SCOM input parms violate rules.

#include "pk.h"
#include "pk_scom.h"

uint32_t putscom( uint32_t i_chiplet_id, uint32_t i_address, uint64_t i_data)
{
    uint32_t l_cid=0;

    // CMO-Declaring variables tied to specific registers enables us to protect 
    // the SCOM data and address variables used in the new stvd and lvd 64-bit
    // data instructions. This protection is needed since the new instructions
    // are not yet properly considered by the compiler.
    // l_dataH is used to represent the "beginning" of the 64-bit data in d8
    // (i.e., r8+r9). 
    uint32_t register l_dataH asm("r8")=0;
    uint32_t register l_dataL asm("r9")=0;
    uint32_t register l_addr_eff asm("r10")=0;
    uint32_t register l_scratch asm("r31")=0;

    if (i_chiplet_id)
    {
        // Accommodate two different ways of supplying the chiplet ID:
        //   0xNN000000: Only bits in high-order two nibbles : Valid
        //   0x000000NN: Only bits in low-order two nibbles :  Valid
        //
        if ((i_chiplet_id & 0xFF000000) == i_chiplet_id)
        {
            // Valid: Chiplet ID in high-order two nibbles.
            l_cid = i_chiplet_id;
        }
        else if ((i_chiplet_id & 0x000000FF) == i_chiplet_id)
        {
            // Valid: Chiplet ID in low-order two nibbles. Convert to high-order.
            l_cid = i_chiplet_id << 24;
        }
        else
        {
            // Invalid: Invalid type of chiplet ID
            PK_TRACE("putscom() : Invalid value of i_chiplet_id (=0x%08X)",i_chiplet_id);
            return 1; //CMO-improve Return sensible rc here.
        }

        l_addr_eff = (i_address & 0x00FFFFFF) | l_cid;
    }
    else
    {
        // Chiplet ID is zero. Accept address as is.
        // This is useful for PIB addresses and non-EX chiplets, and even for
        //   EX chiplets if the fully qualified EX chiplet addr is already known.
        l_addr_eff = i_address;

    }

    l_dataH = (uint32_t)(i_data>>32);
    l_dataL = (uint32_t)(i_data);    

    // CMO-The following sequence forces usage of l_dataH/L and l_addr_eff
    // and thus the population of them as well. 
    // Further note that unless l_dataH/L are placed right before the following
    // sequence, more specifically, if they're placed at the top of putscom(),
    // r8, or l_dataH, might be overwritten in the if(chiplet_id) section.
    // Secondly, we test l_addr_eff for non-zero through the CR0 register 
    // (which was populated in the "mr." instruction.) This is to convince the 
    // compiler that we actually used l_addr_eff for something. 
    // At present the test result causes no action except to execute the stvd 
    // instruction in either case.
    asm volatile ( \
        "mr. %0, %1 \n" \
        : "=r"(l_scratch) \
        : "r"(l_dataH) );
    asm volatile ( \
        "mr. %0, %1 \n" \
        : "=r"(l_scratch) \
        : "r"(l_dataL) );
    asm volatile ( \
        "mr. %0, %1 \n" \
        : "=r"(l_scratch) \
        : "r"(l_addr_eff) );
    asm volatile ( \
        "beq 0x4 \n" );


/*    asm volatile ( \
        "stw %[data], 0(%[effective_address]) \n" \
        : [data]"=r"(l_dataH) \
        : [effective_address]"r"(l_addr_eff) );
*/        
    // CMO-This instruction is not fully supported by the compiler (as of
    // 20150108):
    // - Correct: It is correctly translated into the proper OP code
    //            format.
    // - Incorrect: The compiler does not seem to recognize the usage
    //              of the two l_xyz variables in that it doesn't
    //              know prior to this command that the registers that
    //              contain the values of l_xyz need to be protected
    //              up to this point. Thus, we are forced to use those
    //              two l_xyz variables in some dummy instructions just
    //              before this point in order to protect them.    
    asm volatile ( \
        "stvd %[data], 0(%[effective_address]) \n" \
        : [data]"=r"(l_dataH) \
        : [effective_address]"r"(l_addr_eff) );

    // CMO-TBD
    // Check PIB response code in 0x00001007(17:19)
    // Translate PIB rc to PK rc
    // Does this rc get reset to zero on success?
    // Do we need to check this rc prior to issuing the SCOM?

    return 0;
}


uint32_t getscom( uint32_t i_chiplet_id, uint32_t i_address, uint64_t *o_data)
{
    uint32_t  l_cid=0;

    // CMO-Declaring variables tied to specific registers enables us to protect 
    // the SCOM data and address variables used in the new stvd and lvd 64-bit
    // data instructions. This protection is needed since the new instructions
    // are not yet properly considered by the compiler.
    // l_dataH is used to represent the "beginning" of the 64-bit data in d8
    // (i.e., r8+r9). 
    uint32_t register l_dataH asm("r8")=0;
    uint32_t register l_dataL asm("r9")=0;
    uint32_t register l_addr_eff asm("r10")=0;
    uint32_t register l_scratch asm("r31")=0;

    if (i_chiplet_id)
    {
        // Accommodate two different ways of supplying the chiplet ID:
        //   0xNN000000: Only bits in high-order two nibbles : Valid
        //   0x000000NN: Only bits in low-order two nibbles :  Valid
        //
        if ((i_chiplet_id & 0xFF000000) == i_chiplet_id)
        {
            // Valid: Chiplet ID in high-order two nibbles.
            l_cid = i_chiplet_id;
        }
        else if ((i_chiplet_id & 0x000000FF) == i_chiplet_id)
        {
            // Valid: Chiplet ID in low-order two nibbles. Convert to high-order.
            l_cid = i_chiplet_id << 24;
        }
        else
        {
            // Invalid: Invalid type of chiplet ID
            PK_TRACE("getscom() : Invalid value of i_chiplet_id (=0x%08X)",i_chiplet_id);
            return 1; //CMO-improve Return sensible rc here.
        }

        l_addr_eff = (i_address & 0x00FFFFFF) | l_cid;
    }
    else
    {
        // Chiplet ID is zero. Accept address as is.
        // This is useful for PIB addresses and non-EX chiplets, and even for
        //   EX chiplets if the fully qualified EX chiplet addr is already known.
        l_addr_eff = i_address;
    }

    // CMO-The following sequence forces usage of l_addr_eff and thus the
    // population of it as well. 
    // Secondly, we test l_addr_eff for non-zero through the CR0 register 
    // (which was populated in the "mr." instruction.) This is to convince the 
    // compiler that we actually used l_addr_eff for something. 
    // At present the test result causes no action except to execute the lvd 
    // instruction in either case.
    asm volatile ( \
        "mr. %0, %1 \n" \
        : "=r"(l_scratch) \
        : "r"(l_addr_eff) );
    asm volatile ( \
        "beq 0x4 \n" );

    asm volatile ( \
        "lvd %[data], 0(%[effective_address]) \n" \
        : [data]"=r"(l_dataH) \
        : [effective_address]"r"(l_addr_eff) );

    // CMO-The following sequence moves the read data, in l_dataH/L, into the
    // 64-bit o_data location.
    asm volatile ( \
        "stw %0, 0(%1) \n" \
        : "=r"(l_dataH) \
        : "r"(o_data) );
    asm volatile ( \
        "stw %0, 4(%1) \n" \
        : "=r"(l_dataL) \
        : "r"(o_data) );
    
    // CMO-TBD
    // Check PIB response code in 0x00001007(17:19)
    // Translate PIB rc to PK rc

    return 0;
}
