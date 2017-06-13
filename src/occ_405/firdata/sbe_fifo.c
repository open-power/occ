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
#include <fsi.h>
#include <native.h>

/** @brief  Waits for FIFO to be ready to be written to
 *  @param  i_target The SCOM target.
 *  @return Non-SUCCESS if the SCOM fails. SUCCESS otherwise.
 */
uint32_t waitUpFifoReady(SCOM_Trgt_t* i_target)
{
    uint32_t l_rc = SUCCESS;

    uint64_t l_elapsed_time_ns = 0;
    uint32_t l_addr = SBE_FIFO_UPFIFO_STATUS;
    uint32_t l_data = 0;

    do
    {
        //Read upstream status to see if there is room for more data
        l_rc = getfsi(*i_target, l_addr, &l_data);
        if(l_rc != SUCCESS)
        {
            TRAC_ERR("waitUpFifoReady: failed to getfsi from addr 0x%08x",
                     l_addr);
            break;
        }

        if(!(l_data & UPFIFO_STATUS_FIFO_FULL))
        {
            break;
        }

        //Check for timeout
        if(l_elapsed_time_ns >= MAX_UP_FIFO_TIMEOUT_NS)
        {
            l_rc = FAIL;
            TRAC_ERR("waitUpFifoReady: timeout occurred while waiting for"
                     " FIFO to clear");
            break;
        }

        sleep(10000); //sleep for 10,000 ns
        l_elapsed_time_ns += 10000;
    }while(TRUE);

    return l_rc;
}

/** @brief  Writes a request to FIFO
 *  @param  i_target The SCOM target.
 *  @param  i_fifoRequest the request to execute.
 *  @return Non-SUCCESS if the SCOM fails. SUCCESS otherwise.
 */
uint32_t writeRequest(SCOM_Trgt_t* i_target, uint32_t* i_fifoRequest)
{
    uint32_t l_rc = SUCCESS;

    TRAC_INFO("Enter writeRequest");

    // Ensure Downstream Max Transfer Counter is 0 non-0 can cause
    // protocol issues)
    uint32_t l_addr = SBE_FIFO_DNFIFO_MAX_TSFR;
    uint32_t l_data = 0;
    l_rc = putfsi(*i_target, l_addr, l_data);
    if(l_rc != SUCCESS)
    {
        TRAC_ERR("writeRequest: failed to putfsi to addr 0x%08x",
                 l_addr);
        return l_rc;
    }

    //The first uint32_t has the number of uint32_t words in the request
    l_addr            = SBE_FIFO_UPFIFO_DATA_IN;
    uint32_t* l_sent  = i_fifoRequest; //This pointer will advance as words are sent
    uint32_t  l_count = *l_sent;
    uint32_t  i;

    for(i = 0; i < l_count; ++i)
    {
        //Wait for room to write into fifo
        l_rc = waitUpFifoReady(i_target);
        if(l_rc != SUCCESS)
        {
            return l_rc;
        }

        //Send data into fifo
        l_rc = putfsi(*i_target, l_addr, *l_sent);
        if(l_rc != SUCCESS)
        {
            TRAC_ERR("writeRequest: failed to putfsi to addr 0x%08x",
                     l_addr);
            return l_rc;
        }

        l_sent++;
    }

    //Notify SBE that last word has been sent
    l_rc = waitUpFifoReady(i_target);
    if(l_rc != SUCCESS)
    {
        return l_rc;
    }

    l_addr = SBE_FIFO_UPFIFO_SIG_EOT;
    l_data = FSB_UPFIFO_SIG_EOT;
    l_rc = putfsi(*i_target, l_addr, l_data);
    if(l_rc != SUCCESS)
    {
        TRAC_ERR("writeRequest: failed to putfsi to addr 0x%08x", l_addr);
    }

    TRAC_INFO("Exit writeRequest");

    return l_rc;
}

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

    l_rc = writeRequest(i_target, i_fifoRequest);
    if(l_rc != SUCCESS)
    {
        return l_rc;
    }

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
