/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/image/sbe_loader.c $                              */
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
#include "sbe_xip_image.h"
#include "sbetrace.H"

int32_t sbe_loader() __attribute__ ((section (".loader_text")));


int32_t sbe_loader() {

    int32_t rc = 0;

    SbeXipHeader *hdr = 0;//getXipHdr();
    uint32_t idx;


    for(idx = 0; idx < SBE_XIP_SECTIONS; idx++) {
        
        SBE_TRACE("Section Idx:%u Size:0x%08X", idx, hdr->iv_section[idx].iv_size);
        
    }

    return rc;
}
