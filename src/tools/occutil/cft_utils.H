/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/tools/occutil/cft_utils.H $                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2020,2022                        */
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
extern unsigned int G_sbe_timeout;

#define UINT16_GET(__uint8_ptr)   ((uint16_t)((*((const uint8_t *)(__uint8_ptr)) << 8) | *((const uint8_t *)(__uint8_ptr) + 1)))
#define UINT32_GET(__uint8_ptr)   ((uint32_t)((uint32_t)(UINT16_GET(__uint8_ptr) << 16) | UINT16_GET((((const uint8_t *)(__uint8_ptr)) + 2))))

#define cmtOutputError printf

#define   CMT_SBE_FIFO_OFF    "off"
#define   CMT_SBE_FIFO_ON     "on"

enum cmt_status_e
{
    CMT_SUCCESS = 0,
    CMT_INIT_FAILURE,
    CMT_INVALID_TARGET,
    CMT_SEND_RECEIVE_FAILURE,
    CMT_INVALID_DATA,
    CMT_NOT_SUPPORTED,
    CMT_TIMEOUT,
    CMT_INVALID_PARAMETER,
    CMT_DATA_TRUNCATED,
    CMT_REQUEST_FAILED
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


const char *getRcString(uint32_t rc);
void dumpHex(const uint8_t *data, const unsigned int i_len);

void initHeader(cmtHeader & i_header, uint32_t i_lengthOfMsg);
void ecmdTargetInit(ecmdChipTarget &o_target);
bool update_sbe_fifo(ecmdChipTarget &i_target, const char *new_value = NULL);
uint32_t sbeWriteToHostboot(ecmdChipTarget &i_target,
                            ecmdDataBuffer &i_data,
                            uint8_t       *&o_responseData,
                            uint32_t       &o_responseSize,
                            const uint32_t  i_timeout = G_sbe_timeout);

bool isFsp();

uint32_t send_fsp_command(const char *i_cmd);

uint32_t send_fsp_command_rspdata(const char * i_cmd,
                                  uint8_t *  & o_rspdata,
                                  uint32_t   & o_rsplen);

uint32_t send_bmc_command(const char *i_cmd);
uint32_t send_bmc_command(const char *i_cmd, std::string& o_results);

// Convert BMC byte array string to uint8_t hex buffer and return length
// hexBuffer will be allocated by function and must be freed by caller.
uint16_t bmc_to_array(const std::string& strBuffer, uint8_t*& hexBuffer);

// Return timestamp string HH:MM:SS
char *get_timestamp();

#endif
