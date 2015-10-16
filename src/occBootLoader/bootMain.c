/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occBootLoader/bootMain.c $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2015                        */
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

//*************************************************************************
// Includes
//*************************************************************************
#include <bootMain.h>           // boot loader defines
#include <pgp_common.h>         // Nest frequency constant
#include <stddef.h>             // offsetof

//*************************************************************************
// Externs
//*************************************************************************
extern void __boot_low_level_init;

//*************************************************************************
// Image header
//*************************************************************************
IMAGE_HEADER(G_bootImageHdr,__boot_low_level_init,BOOT_LOADER_ID,
             ID_NUM_INVALID);

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
//*************************************************************************
// Function Prototypes
//*************************************************************************
//Forward declaration
uint32_t boot_test_sram();
uint32_t boot_load_image(const imageHdr_t *i_hdrAddr);
uint32_t calChecksum(const uint32_t i_startAddr ,const uint32_t i_sz);

//*************************************************************************
// Functions
//*************************************************************************

// Function Specification
//
//  Name: boot_main
//
//  Description: boot main will test SRAM, copy main application image from
//  main memory to SRAM, validate checksum and call ssx_boot.
//
// End Function Specification
void main()
{
    WRITE_TO_SPRG0(BOOT_ENTER_OCC_BOOTLOADER);
    uint32_t l_rc = 0;

    //retrieve the ipl time flag from SRAM.
    imageHdr_t *l_hdrPtr = (imageHdr_t *) SRAM_START_ADDRESS;
    uint16_t l_ipl_time = l_hdrPtr->occ_flags.flag_bits.ipl_time_flag;

    if(!l_ipl_time)
    {
        // set checkpoint to boot test SRAM
        WRITE_TO_SPRG0(BOOT_TEST_SRAM_CHKPOINT);
#ifndef VPO
        // This is ifdef'd out b/c it takes too long to run in VPO
        // Test SRAM
        l_rc = boot_test_sram();
#endif

        // If failed to test SRAM, write failed return code to SPRG1 and halt
        if(0 != l_rc)
        {
            WRITE_TO_SPRG1_AND_HALT(l_rc);
        }

        // set imageHdr_t pointer to point to boot image header to get to boot
        // image size. This way we can get to main application image header.
        l_hdrPtr = (imageHdr_t *)(G_bootImageHdr.start_addr +
                                          G_bootImageHdr.image_size);

        // set checkpoint to boot load main application image to SRAM
        WRITE_TO_SPRG0(BOOT_LOAD_IMAGE_CHKPOINT);

        // Load main application image to SRAM including main application header
        l_rc = boot_load_image(l_hdrPtr);

        // If failed to load image, write failed return code to SPRG1 and halt
        if(0 != l_rc)
        {
            WRITE_TO_SPRG1_AND_HALT(l_rc);
        }

        WRITE_TO_SPRG0(BOOT_CALCULATE_CHKSUM_CHKPOINT);
        // calculate checksum for the SRAM main application image
        uint32_t l_checksum = calChecksum(l_hdrPtr->start_addr,
                                          l_hdrPtr->image_size);

        // If checksum does not match, store bad checksum into SPRG1 and halt
        if(l_checksum != l_hdrPtr->checksum)
        {
          WRITE_TO_SPRG1_AND_HALT(l_checksum);

        }

    }

    // set checkpoint to call to SSX_BOOT
    WRITE_TO_SPRG0(BOOT_SSX_BOOT_CALL_CHKPOINT);

    // create function pointer pointing to main application header entry point
    // address.
    void (*execute_ssx_boot)(void) = (void (*)(void))l_hdrPtr->ep_addr;
    (*execute_ssx_boot)();

    // set checkpoint to return from ssx_boot. This should never happen so
    // halt at this point.
    WRITE_TO_SPRG0(BOOT_SSX_RETURNED_CHKPOINT);

    WRITE_TO_SPRG1_AND_HALT(l_hdrPtr->ep_addr);

}

// Function Specification
//
//  Name: calChecksum
//
//  Description: Calculate checksum starting at given address and given size
//               bytes. Skip checksum field in the imageHdr_t while calculating
//               checksum
//
// End Function Specification
uint32_t calChecksum(const uint32_t i_startAddr, const uint32_t i_sz)
{
    uint32_t l_checksum = 0;
    uint32_t l_counter = 0;
    uint8_t *l_srcPtr = (uint8_t *)(i_startAddr);

    while(l_counter < i_sz)
    {
        l_checksum += (*(l_srcPtr + l_counter));
        l_counter = l_counter + 1;
        if(l_counter == (uint32_t)(offsetof(imageHdr_t,checksum)))
        {
            l_counter = ((uint32_t)(offsetof(imageHdr_t,checksum)) +
                        sizeof(G_bootImageHdr.checksum));
        }
    }

    return l_checksum;
}

// Function Specification
//
//  Name: boot_load_image
//
//  Description: This function copies main application image from main memory
//               to SRAM
//
// End Function Specification
uint32_t boot_load_image(const imageHdr_t *i_hdrAddr)
{
    uint32_t l_rc = 0x0;
    uint32_t l_mainAppDestRang = (i_hdrAddr->start_addr) +
                                 (i_hdrAddr->image_size-1);

    // Make sure main application destination rang address falls within SRAM
    // range.
    if((l_mainAppDestRang < SRAM_START_ADDRESS) ||
       (l_mainAppDestRang > SRAM_END_ADDRESS))
    {
        // Return destination rang address if address is out of range and
        // address is not zero. If address is zero, then return eye-catcher
        // address.
        if(l_mainAppDestRang != 0)
        {
            l_rc = l_mainAppDestRang;
        }
        else
        {
            l_rc = EYE_CATCHER_ADDRESS;
        }
    }
    // Make sure main application start address falls within SRAM range
    else if((i_hdrAddr->start_addr < SRAM_START_ADDRESS) ||
            (i_hdrAddr->start_addr > SRAM_END_ADDRESS))
    {
        // Return start address if address is out of range and
        // address is not zero. If address is zero, then return eye-catcher
        // address.
        if(i_hdrAddr->start_addr != 0)
        {
            l_rc = i_hdrAddr->start_addr;
        }
        else
        {
            l_rc = EYE_CATCHER_ADDRESS;
        }
    }
    else
    {
        // At this point we know that main application destination address
        // is within SRAM range.
        // Now copy main application header specified
        // size of data from main memory to main application header specified
        // start address.
        uint8_t *l_srcPtr = (uint8_t *)(i_hdrAddr);
        uint8_t *l_destPtr = (uint8_t *)(i_hdrAddr->start_addr);
        uint32_t l_numWords = i_hdrAddr->image_size;

        while(l_numWords != 0)
        {
            *l_destPtr = *l_srcPtr;
            l_destPtr++;
            l_srcPtr++;
            l_numWords = l_numWords - 1;
        }
    }

    return l_rc;
}


// Function Specification
//
//  Name: boot_test_sram
//
//  Description: This function tests SRAM by writing some bit pattern and
//               verifying it back through read
//
//
// End Function Specification

uint32_t boot_test_sram()
{
    uint32_t l_rc = 0;

    // Point start to SRAM start address
    uint32_t *l_startPtr = (uint32_t *)SRAM_TEST_START_ADDRESS;

    // Copy bit pattern from start until SRAM end address
    while((uint32_t)l_startPtr < SRAM_TEST_END_ADDRESS)
    {
        *l_startPtr = SRAM_TEST_BIT_PATTERN;
        l_startPtr++;
    }

    // Reset start pointer to point to SRAM start Address
    l_startPtr = (uint32_t *)SRAM_TEST_START_ADDRESS;

    //Read and verify bit pattern that was written. If pattern does not match,
    // return address that failed to match the pattern.
    while((uint32_t)l_startPtr < SRAM_TEST_END_ADDRESS)
    {
        if((*l_startPtr) != SRAM_TEST_BIT_PATTERN)
        {
             l_rc = (uint32_t)l_startPtr;
             break;
        }
        l_startPtr++;
    }

    return l_rc;
}

