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

/** @brief  Waits for information to show up in FIFO
 *  @param  i_target The SCOM target.
 *  @param  o_status the status of the FIFO
 *  @return Non-SUCCESS if the SCOM fails. SUCCESS otherwise.
 */
uint32_t waitDnFifoReady(SCOM_Trgt_t* i_target, uint32_t* o_status)
{
    uint32_t l_rc = SUCCESS;

    uint64_t l_elapsed_time_ns = 0;
    uint32_t l_addr = SBE_FIFO_DNFIFO_STATUS;

    do
    {
        // read dnstream status to see if data ready to be read
        // or if has hit the EOT
        l_rc = getfsi(*i_target, l_addr, o_status);
        if(l_rc != SUCCESS)
        {
            return l_rc;
        }

        if(!(*o_status & DNFIFO_STATUS_FIFO_EMPTY) ||
            (*o_status & DNFIFO_STATUS_DEQUEUED_EOT_FLAG))
        {
            break;
        }
        else
        {
            TRAC_IMP("SBE status reg returned fifo empty or dequeued eot flag 0x%.8X",
                      *o_status);
        }

        // Check for timeout
        if(l_elapsed_time_ns >= MAX_UP_FIFO_TIMEOUT_NS)
        {
            TRAC_ERR("waitDnFifoReady: timeout waiting for downstream FIFO"
                      " to be empty.");
            l_rc = FAIL;
            break;
        }

        sleep(10000); // wait for 10,000 ns
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

    TRAC_IMP("Enter writeRequest");

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

    TRAC_IMP("Exit writeRequest");

    return l_rc;
}

/** @brief  Reads and processes the FIFO response
 *  @param  i_target The SCOM target.
 *  @param  i_fifoRequest the original FIFO request.
 *  @param  o_fifoResponse the FIFO response.
 *  @param  i_responseSize the expected size of the response.
 *  @return Non-SUCCESS if the SCOM fails. SUCCESS otherwise.
 */
uint32_t readResponse(SCOM_Trgt_t* i_target,
                      uint32_t* i_fifoRequest,
                      uint32_t* o_fifoResponse,
                      uint32_t  i_responseSize)
{
    uint32_t l_rc = SUCCESS;
    uint32_t l_readBuffer[READ_BUFFER_SIZE];

    TRAC_IMP("Enter readResponse");

    // EOT is expected before the response buffer is full. Room for
    // the PCBPIB status or FFDC is included, but is only returned
    // if there is an error. The last received word has the distance
    // to the status, which is placed at the end of the returned data
    // in order to reflect errors during transfer.

    uint32_t* l_received = o_fifoResponse; // advance as words are received
    uint32_t  l_maxWords = i_responseSize / sizeof(uint32_t);
    uint32_t  l_wordsReceived = 0; // Used to validata the "distance" to status
    bool      l_eotReceived = FALSE;
    uint32_t  l_lastWord = 0; // Last word received. Final read is the "distance"
                             // in words to the status header.
    bool      l_overRun = FALSE;

    do
    {
        // Wait for data to be ready to receive (download) or if the EOT
        // has been sent. If not EOT, then data ready to receive.
        uint32_t l_status = 0;
        l_rc = waitDnFifoReady(i_target, &l_status);
        if(l_rc != SUCCESS)
        {
            return l_rc;
        }

        if(l_status & DNFIFO_STATUS_DEQUEUED_EOT_FLAG)
        {
            l_eotReceived = TRUE;
            // Ignore EOT dummy word
            if(l_wordsReceived >= (sizeof(struct statusHeader) / sizeof(uint32_t)))
            {
                if(l_overRun == FALSE)
                {
                    l_received--;
                    l_wordsReceived--;
                    l_lastWord = o_fifoResponse[l_wordsReceived-1];
                }
                else
                {
                    l_lastWord = l_readBuffer[l_wordsReceived-2];
                }
            }
            break;
        }

        // When error occurs, SBE will write more than l_maxWords
        // we have to keep reading 1 word at a time until we get EOT
        // or more than READ_BUFFER_SIZE. Save what we read in the buffer
        if(l_wordsReceived >= l_maxWords)
        {
            l_overRun = TRUE;
        }

        // Read next word
        l_rc = getfsi(*i_target, SBE_FIFO_DNFIFO_DATA_OUT, &l_lastWord);
        if(l_rc != SUCCESS)
        {
            return l_rc;
        }

        l_readBuffer[l_wordsReceived] = l_lastWord;

        if(l_overRun == FALSE)
        {
            *l_received = l_lastWord; // Copy to returned output buffer
            l_received++; // Advance to the next position
        }
        l_wordsReceived++;

        if(l_wordsReceived > READ_BUFFER_SIZE)
        {
            TRAC_ERR("readResponse: data overflow without EOT");
            l_rc = FAIL;
            return l_rc;
        }

    }while(TRUE);

    // At this point, l_wordsReceived of words received.
    // l_received points to 1 word past last word received.
    // l_lastWord has last word received, which is "distance" to status
    // EOT is expected before running out of response buffer
    if(!l_eotReceived)
    {
        l_rc = FAIL;
        TRAC_ERR("readResponse: no EOT cmd = 0x%08x size = %d",
                 i_fifoRequest[1], i_responseSize);
        return l_rc;
    }

    // Notify SBE that EOT has been received
    uint32_t l_eotSig = FSB_UPFIFO_SIG_EOT;
    l_rc = putfsi(*i_target, SBE_FIFO_DNFIFO_ACK_EOT, l_eotSig);
    if(l_rc != SUCCESS)
    {
        return l_rc;
    }

    // Determine if transmission is successful.
    // Last word received has the distance to status in words including itself.
    // l_wordsReceived has number of words received.
    // Need to have received at least status header and distance word.
    if((l_lastWord      < (sizeof(struct statusHeader)/sizeof(uint32_t) + 1)) ||
       (l_wordsReceived < (sizeof(struct statusHeader)/sizeof(uint32_t) + 1)) ||
       (l_lastWord      > l_wordsReceived))
    {
        TRAC_ERR("readResponse: invalid status distance. Cmd = 0x%08x distance"
                 " = %d allocated response size = %d received word size = %d",
                 i_fifoRequest[1], l_lastWord, i_responseSize, l_wordsReceived);
        l_rc = FAIL;
        return l_rc;
    }

    // Check status for success.
    // l_received points one word past last word received.
    // l_lastWord has number of words to status header including self.
    uint32_t* l_statusTmp = (l_overRun == FALSE) ? (l_received - l_lastWord) :
                                             &l_readBuffer[l_wordsReceived - 1];
    struct statusHeader* l_statusHeader = (struct statusHeader*)l_statusTmp;
    if((FIFO_STATUS_MAGIC != l_statusHeader->magic) ||
       (SBE_PRI_OPERATION_SUCCESSFUL != l_statusHeader->primaryStatus) ||
       (SBE_SEC_OPERATION_SUCCESSFUL != l_statusHeader->secondaryStatus))
    {
        TRAC_ERR("readResponse: failing downstream status cmd = 0x%08x magic = "
                 "0x%08x primary status = 0x%08x secondary status = 0x%08x",
                 i_fifoRequest[1],
                 l_statusHeader->magic,
                 l_statusHeader->primaryStatus,
                 l_statusHeader->secondaryStatus);

        l_rc = FAIL;
    }

    TRAC_IMP("Exit readResponse");
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

    TRAC_IMP("Enter performFifoChipOp");

    l_rc = writeRequest(i_target, i_fifoRequest);
    if(l_rc != SUCCESS)
    {
        return l_rc;
    }

    l_rc = readResponse(i_target,
                        i_fifoRequest,
                        i_fifoResponse,
                        i_responseSize);

    TRAC_IMP("Exit performFifoChioOp");

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

    TRAC_IMP("Enter putFifoScom");

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

    TRAC_IMP("Exit putFifoScom");

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

    TRAC_IMP("Enter getFifoScom");

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

    TRAC_IMP("Exit getFifoScom");

    return l_rc;
}
