/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/firdata/sbe_fifo.c $                              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017                             */
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
#include "sbe_fifo.h"
#include <trac.h>

/** @brief  Performs a FIFO operation (read or write)
 *  @param  i_target The SCOM target.
 *  @param  i_fifoRequest the FIFO request data structure
 *  @param  i_fifoResponse the response from SBE
 *  @param  i_responseSize the size of the response
 *  @return Non-SUCCESS if the operation fails. SUCCESS otherwise.
 */
uint32_t performFifoChipOp(SCOM_Trgt_t* i_target,
                           uint32_t* i_fifoRequest,
                           uint32_t* i_fifoResponse,
                           uint32_t  i_responseSize)
{
    uint32_t l_rc = SUCCESS;

    TRAC_INFO("Enter performFifoChipOp");


    TRAC_INFO("Exit performFifoChioOp");

    return l_rc;
}

/** @brief  Performs a write SCOM operation using SBE FIFO
 *  @param  i_target The SCOM target.
 *  @param  i_addr 64-bit SCOM address.
 *  @param  i_data  64-bit data to write.
 *  @return Non-SUCCESS if the SCOM fails. SUCCESS otherwise.
 */
int32_t putFifoScom(SCOM_Trgt_t* i_target, uint64_t i_addr, uint64_t i_data)
{
    uint32_t l_rc = SUCCESS;

    TRAC_INFO("Enter putFifoScom");

    struct fifoPutScomRequest  l_fifoRequest;
    struct fifoPutScomResponse l_fifoResponse;

    l_fifoRequest.wordCnt = PUT_SCOM_REQUEST_WORD_CNT;
    l_fifoRequest.reserved = 0;
    l_fifoRequest.commandClass = SBE_FIFO_CLASS_SCOM_ACCESS;
    l_fifoRequest.command = SBE_FIFO_CMD_PUT_SCOM;
    l_fifoRequest.address = i_addr;
    l_fifoRequest.data = i_data;

    l_rc = performFifoChipOp(i_target,
                             (uint32_t*)&l_fifoRequest,
                             (uint32_t*)&l_fifoResponse,
                             sizeof(struct fifoPutScomResponse));

    TRAC_INFO("Exit putFifoScom");

    return l_rc;
}

/** @brief  Performs a read SCOM operation using SBE FIFO
 *  @param  i_target The SCOM target.
 *  @param  i_addr 64-bit SCOM address.
 *  @param  o_data  64-bit returned value.
 *  @return Non-SUCCESS if the SCOM fails. SUCCESS otherwise.
 */
int32_t getFifoScom(SCOM_Trgt_t* i_target, uint64_t i_addr, uint64_t* o_data)
{
    uint32_t l_rc = SUCCESS;

    TRAC_INFO("Enter getFifoScom");

    struct fifoGetScomRequest  l_fifoRequest;
    struct fifoGetScomResponse l_fifoResponse;

    l_fifoRequest.wordCnt = GET_SCOM_REQUEST_WORD_CNT;
    l_fifoRequest.reserved = 0;
    l_fifoRequest.commandClass = SBE_FIFO_CLASS_SCOM_ACCESS;
    l_fifoRequest.command = SBE_FIFO_CMD_GET_SCOM;
    l_fifoRequest.address = i_addr;

    l_rc = performFifoChipOp(i_target,
                             (uint32_t*)&l_fifoRequest,
                             (uint32_t*)&l_fifoResponse,
                             sizeof(struct fifoGetScomResponse));

    //Always return data even if there is an error
    *o_data = l_fifoResponse.data;

    TRAC_INFO("Exit getFifoScom");

    return l_rc;
}
