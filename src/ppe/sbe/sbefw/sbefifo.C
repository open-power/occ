/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/sbefw/sbefifo.C $                                 */
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
/*
 * @file: ppe/sbe/sbefw/sbefifo.C
 *
 * @brief This file contains the SBE FIFO Commands
 *
 */

#include "sbeexeintf.H"
#include "sbefifo.H"
#include "sbetrace.H"
#include "sbe_sp_intf.H"

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
uint32_t sbeUpFifoDeq_mult (uint8_t     &io_len,
                            uint32_t    *o_pData,
                            const bool  i_flush)
{
    #define SBE_FUNC " sbeUpFifoDeq_mult "
    uint32_t l_rc = SBE_FIFO_RC_UNKNOWN;
    uint8_t  l_len = 0;

    // @TODO via RTC : 130575
    //       Refactor this utility to
    //       optimize RC handling, stack usage
    //       and FIFO operation infrastructure.
    //

    do
    {
        sbe_upfifo_entry_t l_data = {0};
        uint64_t l_upfifo_data = 0;

        // Read Double word from the Upstream FIFO;
        // The DW data represents the first 32 bits of data word entry
        // followed by the status bits.

        // Bit 0-31    : Data
        // Bit 32      : Data valid flag
        // Bit 33      : EOT flag
        // Bit 34-63   : Status (2-31)
        // Valid : EOT
        //    1  : 0   -> data=message
        //    0  : 1   -> data=dummy_data of EOT operation
        //    0  : 0   -> data=dummy_data
        //    1  : 1   -> Not used

        l_rc = sbeUpFifoDeq ( &l_upfifo_data );

        if (l_rc)
        {
            // Error while dequeueing from upstream FIFO
            SBE_ERROR(SBE_FUNC"sbeUpFifoDeq failed,"
                         "l_rc=[0x%08X]", l_rc);
            l_rc = SBE_SEC_FIFO_ACCESS_FAILURE;
            break;
        }

        l_data.upfifo_data   =   (uint32_t)(l_upfifo_data>>32);
        l_data.upfifo_status.upfifo_status_uint32 = (uint32_t)
                                               (l_upfifo_data);

        SBE_DEBUG(SBE_FUNC"sbeUpFifoDeq, "
                    "l_data.upfifo_data=[0x%08X],"
                    "l_data.upfifo_status=[0x%08X]",
                     l_data.upfifo_data,
                     l_data.upfifo_status.upfifo_status_uint32);

        // If FIFO reset is requested
        if(l_data.upfifo_status.upfifo_status_bitset.req_upfifo_reset)
        {
            // @TODO via RTC : 126147
            //       Review reset loop flow in here.
            //       Received a FIFO reset request
            l_rc = SBE_FIFO_RC_RESET;
            break;
        }

        // if EOT flag is set
        //    clear EOT
        if (l_data.upfifo_status.upfifo_status_bitset.eot_flag)
        {
            l_rc = sbeUpFifoAckEot();
            if (l_rc)
            {
                // Error while ack'ing EOT in upstream FIFO
                SBE_ERROR(SBE_FUNC"sbeUpFifoAckEot failed,"
                          "l_rc=[0x%08X]", l_rc);
                // Collect FFDC
                l_rc = SBE_FIFO_RC_EOT_ACK_FAILED;
            }
            else
            {
                l_rc = SBE_FIFO_RC_EOT_ACKED;
            }
            break;
        }

        // if Upstream FIFO is empty,
        if (l_data.upfifo_status.upfifo_status_bitset.fifo_empty)
        {
            l_rc = SBE_FIFO_RC_EMPTY;
            continue;
        }

        if (i_flush)
        {
            l_len  = 0; // to force the upFIFO flush until EOT arrives
            continue;
        }

        o_pData[l_len] = l_data.upfifo_data;
        ++l_len;
        l_rc = SBE_SEC_OPERATION_SUCCESSFUL;

    } while(l_len<io_len);

    // Return the length of entries dequeued.
    // When user sets i_flush as true, this
    // would return io_len as 0;
    io_len = l_len;
    return l_rc;

    #undef SBE_FUNC
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
uint32_t sbeDownFifoEnq_mult (uint8_t        &io_len,
                              const uint32_t *i_pData)
{
    #define SBE_FUNC " sbeDownFifoEnq_mult "
    uint8_t  l_rc   = SBE_FIFO_RC_UNKNOWN;
    uint8_t  l_len = 0;

    // @TODO via RTC : 130575
    //       Refactor this utility to
    //       optimize RC handling, stack usage
    //       and FIFO operation infrastructure.

    do
    {
        sbe_downfifo_status_t l_downFifoStatus ;
        typedef union
        {
            uint64_t status;
            uint64_t data;
        } sbeDownFiFoEntry_t;
        sbeDownFiFoEntry_t l_sbeDownFiFoEntry ;

        // Read the down stream FIFO status
        l_rc = sbeDownFifoGetStatus (&l_sbeDownFiFoEntry.status);
        if (l_rc)
        {
            // Error while reading downstream FIFO status
            SBE_ERROR(SBE_FUNC"sbeDownFifoGetStatus failed, "
                      "l_rc=[0x%08X]", l_rc);
            l_rc = SBE_SEC_FIFO_ACCESS_FAILURE;
            break;
        }

        l_downFifoStatus.downfifo_status_uint32 = (uint32_t)
                                      (l_sbeDownFiFoEntry.status>>32);

        SBE_DEBUG(SBE_FUNC"downstream fifo status[0x%08X]",
                   l_downFifoStatus.downfifo_status_uint32);

        // Check if there was a FIFO reset request from SP
        if (l_downFifoStatus.downfifo_status_bitset.req_upfifo_reset)
        {
            // @TODO via RTC : 126147
            //       Review reset loop flow in here.
            //       Received an upstream FIFO reset request
            SBE_ERROR(SBE_FUNC"Received reset request");
            l_rc = SBE_FIFO_RC_RESET;
            break;
        }

        // Check if downstream FIFO is full
        if (l_downFifoStatus.downfifo_status_bitset.fifo_full)
        {
            // Downstream FIFO is full
            SBE_INFO(SBE_FUNC"Downstream FIFO is full");
            l_rc = SBE_FIFO_RC_FULL; // in case we ever add timeout
            continue;
        }

        // PIB write data format:
        // Bit 0 - 31  : Data
        // Bit 32 - 63 : Unused

        l_sbeDownFiFoEntry.data   = (uint64_t)(*(i_pData+l_len));
        l_sbeDownFiFoEntry.data   = l_sbeDownFiFoEntry.data<<32;

        SBE_DEBUG(SBE_FUNC"Downstream fifo data entry[0x%08X]",
                                    (l_sbeDownFiFoEntry.data>>32));

        // Write the data into the downstream FIFO
        l_rc = sbeDownFifoEnq (l_sbeDownFiFoEntry.data);
        if (l_rc)
        {
            SBE_ERROR(SBE_FUNC"sbeDownFifoEnq failed, "
                              "l_rc[0x%08X]", l_rc);
            l_rc = SBE_SEC_FIFO_ACCESS_FAILURE;
            break;
        }

        l_rc = SBE_SEC_OPERATION_SUCCESSFUL;
        ++l_len;

    } while(l_len<io_len);

    io_len = l_len;
    return l_rc;
    #undef SBE_FUNC
}

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
void sbeBuildMinRespHdr ( uint32_t *io_pBuf,
                          uint8_t  &io_curIndex,
                    const uint16_t  i_primStatus,
                    const uint16_t  i_secStatus,
                    const uint32_t  i_pcbpibStatus,
                    const uint8_t   i_startIndex )
{
    do
    {
        if (!io_pBuf)
        {
            break;
        }

        io_pBuf[io_curIndex] = sbeBuildRespHeaderMagicCodeCmdClass();
        io_pBuf[++io_curIndex] = sbeBuildRespHeaderStatusWordLocal(
                                          i_primStatus, i_secStatus);

        // @TODO via RTC: 128916
        //       pcb-pib error is optional,
        //       not needed for success case
        io_pBuf[++io_curIndex]    = i_pcbpibStatus;

        // Somehow this compiler isn't allowing the
        // index pre-increment for the last array entry
        // directly embedded into the assignment
        ++io_curIndex;
        io_pBuf[io_curIndex]   = io_curIndex - i_startIndex + 1;

    } while(false);
}
