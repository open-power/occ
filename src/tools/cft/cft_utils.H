/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/tools/cft/cftocctool.C $                                  */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2020                             */
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
/**
  @file cftocctool.C
  @brief Main Program entry point for cmtDllClient Application
*/

#ifndef __CFT_UTILS_H
#define __CFT_UTILS_H


#include <stdio.h>
#include <string.h>
#include <arpa/inet.h> // htons
#include <stddef.h> // offsetof
#include <ecmdClientCapi.H>


extern unsigned int G_verbose;

#define UINT16_GET(__uint8_ptr)   ((uint16_t)((*((const uint8_t *)(__uint8_ptr)) << 8) | *((const uint8_t *)(__uint8_ptr) + 1)))
#define UINT32_GET(__uint8_ptr)   ((uint32_t)((uint32_t)(UINT16_GET(__uint8_ptr) << 16) | UINT16_GET((((const uint8_t *)(__uint8_ptr)) + 2))))

#define cmtOutputError printf


enum cmt_status_e
{
    CMT_SUCCESS = 0,
    CMT_INIT_FAILURE,
    CMT_INVALID_TARGET,
    CMT_SEND_RECEIVE_FAILURE
};

struct cmtHeader
{
    uint32_t sbeHeaderVersion;
    uint32_t lengthOfMsg;
    uint32_t sequenceId;
    uint32_t cmdHeaderVersion;
    uint32_t status;
    uint32_t dataOffset;
    uint32_t dataSize;
    uint16_t cmdClass;
    uint16_t cmdCode;
} __attribute__((packed));


void dumpHex(const uint8_t *data, const unsigned int i_len);

void initHeader(cmtHeader & i_header, uint32_t i_lengthOfMsg);
void ecmdTargetInit(ecmdChipTarget &o_target);
uint32_t cmtOCCSendReceive(ecmdChipTarget &i_target, ecmdDataBuffer &i_data, uint8_t *&o_responseData,  uint32_t  & o_responseSize);

bool isFsp();

#endif
