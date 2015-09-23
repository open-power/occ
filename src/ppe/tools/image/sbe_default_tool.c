/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/tools/image/sbe_default_tool.c $                      */
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
/// \file sbe_default_tool.c
/// \brief SBE-XIP image setter tool for attributes in fixed section
///
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <endian.h>


#define __PPE__
#include "fapi2.H"
#include "proc_sbe_fixed.H"

const char* g_usage =
"Usage: sbe_default_tool <image> <attribute> <value> <target type> <index>\n"
"The 'image' is the binary image with fixed section.\n"
"\n"
"The 'attribute' is the attribute to be set.\n"
"\n"
"The 'value' is the value of the attribute to be set.\n"
"\n"
"The 'target type' is the type of the target. The following targets are defined:\n"
"TARGET_TYPE_PROC_CHIP: chip target\n"
"TARGET_TYPE_PERV: pervasive target\n"
"TARGET_TYPE_CORE: core target\n"
"TARGET_TYPE_EQ: eq target\n"
"TARGET_TYPE_EX: ex target\n"
"\n"
"The 'index' is the index of the value. Checking is performed.\n"
"example:\n"
"./sbe_default_tool ./sbe_main.bin ATTR_PLL_RING 0x33CAFE34 TARGET_TYPE_PERV 0\n"
"./sbe_default_tool ./sbe_main.bin ATTR_SCRATCH_UINT8_1 12 TARGET_TYPE_PROC_CHIP 0\n"
    ;


void assertTarget(const char* str, unsigned int index)
{

    if(strcmp(str, "TARGET_TYPE_PROC_CHIP") == 0) {
        if (index > 0) {
            fprintf(stderr, "sbe_default_tool: index is larger than 0\n");
            exit(1);
        } 
        return;
    } else if(strcmp(str, "TARGET_TYPE_EX") == 0)  {
        if (index >= EX_TARGET_COUNT) {
            fprintf(stderr, "sbe_default_tool: index is larger than EX_TARGET_COUNT\n");
            exit(1);
        } 
        return;
    } else if(strcmp(str, "TARGET_TYPE_EQ") == 0)  {
        if (index >= EQ_TARGET_COUNT) {
            fprintf(stderr, "sbe_default_tool: index is larger than EQ_TARGET_COUNT\n");
            exit(1);
        } 
        return;
    } else if(strcmp(str, "TARGET_TYPE_CORE") == 0)  {
        if (index >= CORE_TARGET_COUNT) {
            fprintf(stderr, "sbe_default_tool: index is larger than EQ_TARGET_COUNT\n");
            exit(1);
        } 
        return;
    } else if(strcmp(str, "TARGET_TYPE_PERV") == 0)  {
        if (index >= PERV_TARGET_COUNT) {
            fprintf(stderr, "sbe_default_tool: index is larger than PERV_TARGET_COUNT\n");
            exit(1);
        } 
        return;
    } else {

        if (index >= PERV_TARGET_COUNT) {
            fprintf(stderr, "sbe_default_tool: target not supported:");
            fprintf(stderr, " %s\n", str);
            exit(1);
        }    
    }
}

void setAttribute(void* image, const char* attribute, unsigned int index, uint64_t val) {


    SbeXipItem item;
    void *thePointer;
    int rc;

    rc = sbe_xip_find(image, attribute, &item);
    if (rc) {
        fprintf(stderr, "sbe_default_tool: attribute not existing:");
        fprintf(stderr, " %s", attribute);
        exit(1);
    }

    // debug purpose
    //printf("offset in string section: 0x%x \n",     be32toh(item.iv_toc->iv_id));
    //printf("address: 0x%x \n", item.iv_address);

    sbe_xip_image2host(image, item.iv_address, &thePointer);

    // debug purpose
    //printf("pointer1: 0x%x \n", thePointer);
    //printf("val: 0x%llx \n", val);

    if(item.iv_toc->iv_type == SBE_XIP_UINT8) {

        *((uint8_t*)thePointer + index) = (uint8_t)val;

    } else if(item.iv_toc->iv_type == SBE_XIP_INT8) {

        *((int8_t*)thePointer + index) = (int8_t)val;

    } else if(item.iv_toc->iv_type == SBE_XIP_UINT16) {

        *((uint16_t*)thePointer + index) = (uint16_t)val;

    } else if(item.iv_toc->iv_type == SBE_XIP_INT16) {

        *((int16_t*)thePointer + index) = (int16_t)val;

    } else if(item.iv_toc->iv_type == SBE_XIP_UINT32) {

        *((uint32_t*)thePointer + index) = (uint32_t)val;

    } else if(item.iv_toc->iv_type == SBE_XIP_INT32) {

        *((int32_t*)thePointer + index) = (int32_t)val;

    } else if(item.iv_toc->iv_type == SBE_XIP_UINT64) {

        *((uint64_t*)thePointer + index) = (uint64_t)val;

    } else if(item.iv_toc->iv_type == SBE_XIP_INT64) {

        *((int64_t*)thePointer + index) = (int64_t)val;

    } else {
        fprintf(stderr, "sbe_default_tool: type not available");
        exit(1);
    }




    SBE_XIP_SECTION_NAMES(section_name);
    SBE_XIP_TYPE_STRINGS(type_name);

    // debug purpose
    //printf("pointer2: 0x%x \n", thePointer);
    //printf("section id: %s \n",  section_name[item.iv_toc->iv_section]);
    //printf("location in section: 0x%x \n",  be32toh(item.iv_toc->iv_data));
    //printf("type name: %s \n",  type_name[item.iv_toc->iv_type]);

    return;
}


uint64_t getAttribute(void* image, const char* attribute, unsigned int index) {

    uint64_t val = 0;

    SbeXipItem item;
    void *thePointer;
    int rc;

    rc = sbe_xip_find(image, attribute, &item);
    if (rc) {
        fprintf(stderr, "sbe_default_tool: attribute not existing:");
        fprintf(stderr, " %s", attribute);
        exit(1);
    }
    
    sbe_xip_image2host(image, item.iv_address, &thePointer);

    if(item.iv_toc->iv_type == SBE_XIP_UINT8) {

        val = *((uint8_t*)thePointer + index);
        
    } else if(item.iv_toc->iv_type == SBE_XIP_INT8) {

        val = *((int8_t*)thePointer + index);
        val &= 0xFF; 

    } else if(item.iv_toc->iv_type == SBE_XIP_UINT16) {

        val = *((uint16_t*)thePointer + index);

    } else if(item.iv_toc->iv_type == SBE_XIP_INT16) {

        val = *((int16_t*)thePointer + index);
        val &= 0xFFFF; 

    } else if(item.iv_toc->iv_type == SBE_XIP_UINT32) {

        val = *((uint32_t*)thePointer + index);

    } else if(item.iv_toc->iv_type == SBE_XIP_INT32) {

        val = *((int32_t*)thePointer + index);
        val &= 0xFFFFFFFF; 

    } else if(item.iv_toc->iv_type == SBE_XIP_UINT64) {

        val = *((uint64_t*)thePointer + index);

    } else if(item.iv_toc->iv_type == SBE_XIP_INT64) {

        val = *((int64_t*)thePointer + index);

    } else {
        fprintf(stderr, "sbe_default_tool: type not available");
        exit(1);
    }



    return val;
}

int  main(int argc, const char** argv)
{

    int fileFd, rc;
    void* image;
    struct stat buf;

    if(argc != 6) {
        fprintf(stderr, "sbe_default_tool: argument missing\n");
        fprintf(stderr, g_usage);
        exit(1);
    } 

    printf("sbe_default_tool %s %s %s %s %s\n", argv[1], argv[2], argv[3], argv[4], argv[5]);

    fileFd = open(argv[1], O_RDWR);
    if (fileFd < 0) {
        fprintf(stderr, "sbe_default_tool: open() of the file to be appended failed");
        exit(1);
    }

    rc = fstat(fileFd, &buf);
    if (rc) {
        fprintf(stderr, "sbe_default_tool: fstat() of the file to be appended failed");
        exit(1);
    }

    image = mmap(0, buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fileFd, 0);
    if (image == MAP_FAILED) {
        fprintf(stderr, "sbe_default_tool: mmap() of the file to be appended failed");
        exit(1);
    }


    uint64_t val=strtoull(argv[3], 0, 0);

    unsigned int index = strtoul(argv[5], 0, 10);

    assertTarget(argv[4], index);

    setAttribute(image, argv[2], index, val);

    uint64_t check = getAttribute(image, argv[2], index);

    if((check & val) != check) {
            
        fprintf(stderr, "sbe_default_tool: set and get values not equal");
        fprintf(stderr, "%lx != %lx\n", check, val);
        exit(1);

    }

    rc = close(fileFd);
    if (rc) {
        fprintf(stderr, "sbe_default_tool: close() of modified image failed");
        exit(1);
    }

			
    return 0;
}
