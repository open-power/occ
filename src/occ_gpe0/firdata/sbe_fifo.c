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
#include <fsi.h>
#include <native.h>
#include <string.h>

extern void busy_wait(uint32_t t_microseconds);

enum
{
    RC_RESP_DATA_OVERFLOW        = 1001,
    RC_RESP_MIN_SIZE_INVALID     = 1002,
    RC_RESP_DISTANCE_INVALID     = 1003,
    RC_RESP_MAGIC_WORD_INVALID   = 1004,
    RC_RESP_UNEXPECTED_CMD       = 1005,
    RC_RESP_UNEXPECTED_DATA_SIZE = 1006,

    RC_RESP_SCOM_ERROR = 1010,

    RC_FIFO_TIMEOUT_UP = 1021,
    RC_FIFO_TIMEOUT_DN = 1022,
};

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
        l_rc = getfsi(i_target, l_addr, &l_data);
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
            l_rc = RC_FIFO_TIMEOUT_UP;
            TRAC_ERR("waitUpFifoReady: timeout occurred while waiting for"
                     " FIFO to clear");
            break;
        }

        busy_wait(10); //sleep for 10,000 ns
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
        l_rc = getfsi(i_target, l_addr, o_status);
        if(l_rc != SUCCESS)
        {
            return l_rc;
        }

        if(!(*o_status & DNFIFO_STATUS_FIFO_EMPTY) ||
            (*o_status & DNFIFO_STATUS_DEQUEUED_EOT_FLAG))
        {
            break;
        }

        // Check for timeout
        if(l_elapsed_time_ns >= MAX_UP_FIFO_TIMEOUT_NS)
        {
            TRAC_ERR("waitDnFifoReady: timeout waiting for downstream FIFO"
                      " to be empty.");
            l_rc = RC_FIFO_TIMEOUT_DN;
            break;
        }

        busy_wait(10); // wait for 10,000 ns
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

    // Ensure Downstream Max Transfer Counter is 0 non-0 can cause
    // protocol issues)
    uint32_t l_addr = SBE_FIFO_DNFIFO_MAX_TSFR;
    uint32_t l_data = 0;
    l_rc = putfsi(i_target, l_addr, l_data);
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
        l_rc = putfsi(i_target, l_addr, *l_sent);
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
    l_rc = putfsi(i_target, l_addr, l_data);
    if(l_rc != SUCCESS)
    {
        TRAC_ERR("writeRequest: failed to putfsi to addr 0x%08x", l_addr);
    }

    return l_rc;
}

// For error path debug.
void printBuffer( uint32_t* i_readBuffer, uint32_t i_wordsReceived )
{
    // OCC traces only support max 4 arguments and trace buffers very limited.
    // So try to print as many entries as possible on one line as possible.
    uint32_t i = 0;
    for ( i = 0; i < i_wordsReceived; i+=4 )
    {
        uint32_t j = i_wordsReceived - i;
        if ( 4 <= j )
        {
            TRAC_ERR( "    %08x %08x %08x %08x", i_readBuffer[i],
                      i_readBuffer[i+1], i_readBuffer[i+2], i_readBuffer[i+3] );
        }
        else if ( 3 == j )
        {
            TRAC_ERR( "    %08x %08x %08x", i_readBuffer[i], i_readBuffer[i+1],
                      i_readBuffer[i+2] );
        }
        else if ( 2 == j )
        {
            TRAC_ERR( "    %08x %08x", i_readBuffer[i], i_readBuffer[i+1] );
        }
        else if ( 1 == j )
        {
            TRAC_ERR( "    %08x", i_readBuffer[i] );
        }
    }
}

/** @brief  Reads and processes the FIFO response
 *  @param  i_target       The SCOM target.
 *  @param  i_fifoReqCmd   The FIFO request command.
 *  @param  o_responseData The returned response data (getSCOMs only).
 *  @return Non-SUCCESS on error, otherwise SUCCESS.
 */
uint32_t readResponse( SCOM_Trgt_t* i_target, FifoCmd_t* i_fifoReqCmd,
                       uint64_t* o_responseData )
{
    uint32_t l_rc = SUCCESS;

    memset( o_responseData, 0, sizeof(uint64_t) ); // Just in case.

    uint32_t readBuffer[READ_BUFFER_SIZE];
    uint32_t wordsReceived = 0;

    do
    {
        // Wait to read data or EOT from the FIFO.
        uint32_t l_status = 0;
        l_rc = waitDnFifoReady( i_target, &l_status );
        if ( SUCCESS != l_rc )
        {
            return l_rc;
        }

        // Check for EOT.
        if ( l_status & DNFIFO_STATUS_DEQUEUED_EOT_FLAG )
        {
            // There should be a word at the end of the buffer containing a EOT
            // dummy word, which can be ignored.
            if ( 0 < wordsReceived ) wordsReceived--;

            break; // Nothing more to read.
        }

        // Ensure there is enough room in the buffer to read the next word.
        if ( READ_BUFFER_SIZE <= wordsReceived )
        {
            TRAC_ERR( "readResponse: data overflow without EOT. "
                      "wordsReceived=%u", wordsReceived );
            printBuffer( readBuffer, wordsReceived );
            return RC_RESP_DATA_OVERFLOW;
        }

        // Read the next word.
        l_rc = getfsi( i_target, SBE_FIFO_DNFIFO_DATA_OUT,
                       &readBuffer[wordsReceived] );
        if ( SUCCESS != l_rc )
        {
            return l_rc;
        }

        // Increment the number of words received.
        wordsReceived++;

    } while ( TRUE );

    // The only path that allows us to get here is if we successfully received
    // the EOT. Notify the SBE that it has been received.
    l_rc = putfsi( i_target, SBE_FIFO_DNFIFO_ACK_EOT, FSB_UPFIFO_SIG_EOT );
    if ( SUCCESS != l_rc )
    {
        return l_rc;
    }

    // At a minimum, the response should have returned enough data to contain
    // the status header and a word that contains the distance from the end of
    // the response to the beginning of the status header.
    uint32_t l_minWords = (sizeof(FifoRespStatus_t) / sizeof(uint32_t)) + 1;
    if ( wordsReceived < l_minWords )
    {
        TRAC_ERR( "readResponse: minimum response size is invalid. "
                  "wordsReceived=%u", wordsReceived );
        printBuffer( readBuffer, wordsReceived );
        return RC_RESP_MIN_SIZE_INVALID;
    }

    // The distance between the end of the response to the beginning of the
    // status header is stored in the last word in the buffer.
    uint32_t distance = readBuffer[wordsReceived-1];
    if ( distance < l_minWords || wordsReceived < distance )
    {
        TRAC_ERR( "readResponse: invalid response distance. wordsReceived=%u "
                  "distance=%u", wordsReceived, distance );
        printBuffer( readBuffer, wordsReceived );
        return RC_RESP_DISTANCE_INVALID;
    }

    // Check for a successful response.
    uint32_t numDataWords = wordsReceived - distance;
    FifoRespStatus_t* status = (FifoRespStatus_t*)(&readBuffer[numDataWords]);

    if ( FIFO_STATUS_MAGIC != status->magic )
    {
        TRAC_ERR( "readResponse: invalid magic word. magic=0x%04x",
                  status->magic );
        printBuffer( readBuffer, wordsReceived );
        return RC_RESP_MAGIC_WORD_INVALID;
    }

    // Verify that this response was for the command that was sent.
    if ( i_fifoReqCmd->u != status->command.u )
    {
        TRAC_ERR( "readResponse: unexpected response command. cmd=0x%08x",
                  status->command.u );
        printBuffer( readBuffer, wordsReceived );
        return RC_RESP_UNEXPECTED_CMD;
    }

    // For getSCOMs only, get the response data (if it exists) regardless if
    // there was a SCOM error.
    uint32_t expDataWords = 0;
    if ( SBE_FIFO_CMD_GET_SCOM == status->command.s.type )
    {
        expDataWords = sizeof(uint64_t) / sizeof(uint32_t);
        if ( expDataWords == numDataWords )
        {
            memcpy( o_responseData, &readBuffer[0], sizeof(uint64_t) );
        }
    }

    // Check for SCOM errors.
    if ( (SBE_PRI_OPERATION_SUCCESSFUL != status->primaryStatus  ) ||
         (SBE_SEC_OPERATION_SUCCESSFUL != status->secondaryStatus) )
    {
        // NOTE: If there was some sort of SCOM error, there should be an FFDC
        //       section after the status. At this time, we have no use for the
        //       data so it will be ignored for now.

        TRAC_ERR( "readResponse: unexpected response status. cmd=0x%08x "
                  "primaryStatus=0x%08x secondaryStatus=0x%08x",
                  i_fifoReqCmd->u, status->primaryStatus,
                  status->secondaryStatus );
        printBuffer( readBuffer, wordsReceived );
        return RC_RESP_SCOM_ERROR;
    }

    // The command was successful. Ensure the response data was at least a
    // valid size.
    if ( expDataWords != numDataWords )
    {
        TRAC_ERR( "readResponse: unexpected response data size. cmd=0x%08x "
                  "wordsReceived=%u distance=%u", i_fifoReqCmd->u,
                  wordsReceived, distance );
        printBuffer( readBuffer, wordsReceived );
        return RC_RESP_UNEXPECTED_DATA_SIZE;
    }

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

    struct fifoPutScomRequest  l_fifoRequest;
    l_fifoRequest.wordCnt = PUT_SCOM_REQUEST_WORD_CNT;
    l_fifoRequest.reserved = 0;
    l_fifoRequest.command.s.class = SBE_FIFO_CLASS_SCOM_ACCESS;
    l_fifoRequest.command.s.type  = SBE_FIFO_CMD_PUT_SCOM;
    l_fifoRequest.address = i_addr;
    l_fifoRequest.data = i_data;

    l_rc = writeRequest( i_target, (uint32_t*)&l_fifoRequest );
    if ( SUCCESS == l_rc )
    {
        uint64_t unused = 0;
        l_rc = readResponse( i_target, &l_fifoRequest.command, &unused );
    }

    if ( l_rc != SUCCESS )
    {
        // Reset the FIFO for subsequent SCOMs
        uint32_t l_data = 0xDEAD;
        putfsi( i_target, 0x2450, l_data );
    }

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

    struct fifoGetScomRequest  l_fifoRequest;
    l_fifoRequest.wordCnt = GET_SCOM_REQUEST_WORD_CNT;
    l_fifoRequest.reserved = 0;
    l_fifoRequest.command.s.class = SBE_FIFO_CLASS_SCOM_ACCESS;
    l_fifoRequest.command.s.type  = SBE_FIFO_CMD_GET_SCOM;
    l_fifoRequest.address = i_addr;

    l_rc = writeRequest( i_target, (uint32_t*)&l_fifoRequest );
    if ( SUCCESS == l_rc )
    {
        l_rc = readResponse( i_target, &l_fifoRequest.command, o_data );
    }

    if ( l_rc != SUCCESS )
    {
        // Reset the FIFO for subsequent SCOMs
        uint32_t l_data = 0xDEAD;
        putfsi( i_target, 0x2450, l_data );
    }

    return l_rc;
}
