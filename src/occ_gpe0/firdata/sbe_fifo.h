/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/firdata/sbe_fifo.h $                             */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017,2019                        */
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
#ifndef __SBE_FIFO_H
#define __SBE_FIFO_H

#include <scom_trgt.h>
#include <stdint.h>

#define PACKED __attribute__((__packed__))

#define PUT_SCOM_REQUEST_WORD_CNT 0x06
#define GET_SCOM_REQUEST_WORD_CNT 0x04

#define SBE_FIFO_CMD_GET_SCOM 0x01
#define SBE_FIFO_CMD_PUT_SCOM 0x02
#define SBE_FIFO_CLASS_SCOM_ACCESS 0xA2
#define SBE_FIFO_DNFIFO_MAX_TSFR 0x00002458
#define SBE_FIFO_UPFIFO_DATA_IN 0x00002400
#define SBE_FIFO_UPFIFO_SIG_EOT 0x00002408
#define FSB_UPFIFO_SIG_EOT 0x80000000
#define SBE_FIFO_UPFIFO_STATUS 0x00002404
#define SBE_FIFO_DNFIFO_STATUS 0x00002444
#define SBE_FIFO_UPFIFO_REQ_RESET 0x0000240c
#define UPFIFO_STATUS_FIFO_FULL 0x00200000
#define UPFIFO_STATUS_FIFO_NOTREADY 0x02000000
#define DNFIFO_STATUS_FIFO_EMPTY 0x00100000
#define DNFIFO_STATUS_DEQUEUED_EOT_FLAG 0x00800000
#define SBE_FIFO_DNFIFO_ACK_EOT 0x00002454
#define SBE_FIFO_DNFIFO_DATA_OUT 0x00002440
#define NS_PER_MSEC (1000000ull)
#define MAX_UP_FIFO_TIMEOUT_NS 10*NS_PER_MSEC
// READ_BUFFER_SIZE is number of words when reading scom via SBE
// (minimum 6: 2 words of scom data + 3 words of header + 1 EOT)
#define READ_BUFFER_SIZE 8
#define FIFO_STATUS_MAGIC 0xC0DE
#define SBE_PRI_OPERATION_SUCCESSFUL 0x00
#define SBE_SEC_OPERATION_SUCCESSFUL 0x00

#define TimeoutForStartOfTrx 0x1000

typedef union
{
    struct
    {
        uint8_t class;
        uint8_t type;
    } PACKED s;

    uint16_t u;

} FifoCmd_t;

typedef struct
{
    uint16_t magic; //set to 0xC0DE
    FifoCmd_t command;
    uint16_t primaryStatus;
    uint16_t secondaryStatus;

} PACKED FifoRespStatus_t;

struct fifoPutScomRequest
{
    uint32_t wordCnt;
    uint16_t reserved;
    FifoCmd_t command;
    uint64_t address;
    uint64_t data;
} PACKED;

struct fifoGetScomRequest
{
    uint32_t wordCnt;
    uint16_t reserved;
    FifoCmd_t command;
    uint64_t address;
} PACKED;

/** @brief  Performs a write SCOM operation using SBE FIFO
 *  @param  i_target The SCOM target.
 *  @param  i_addr 64-bit SCOM address.
 *  @param  i_data  64-bit data to write.
 *  @return Non-SUCCESS if the SCOM fails. SUCCESS otherwise.
 */
int32_t putFifoScom(SCOM_Trgt_t* i_target, uint64_t i_addr, uint64_t i_data);

/** @brief  Performs a read SCOM operation using SBE FIFO
 *  @param  i_target The SCOM target.
 *  @param  i_addr 64-bit SCOM address.
 *  @param  o_data  64-bit returned value.
 *  @return Non-SUCCESS if the SCOM fails. SUCCESS otherwise.
 */
int32_t getFifoScom(SCOM_Trgt_t* i_target, uint64_t i_addr, uint64_t* o_data);

#endif

