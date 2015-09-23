/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/sbefw/sbecmdscomaccess.C $                        */
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
 * @file: ppe/sbe/sbefw/sbecmdscomaccess.C
 *
 * @brief This file contains the SBE SCOM Access chipOps
 *
 */

#include "sbecmdscomaccess.H"
#include "sbefifo.H"
#include "sbe_sp_intf.H"
#include "sbetrace.H"

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
uint32_t sbeGetScom (uint8_t *i_pArg)
{
    #define SBE_FUNC " sbeGetScom "
    SBE_ENTER(SBE_FUNC);

    uint32_t l_rc = SBE_SEC_OPERATION_SUCCESSFUL;

    do
    {
        uint16_t l_primStatus = g_sbeCmdRespHdr.prim_status;
        uint16_t l_secStatus  = g_sbeCmdRespHdr.sec_status ;

        // Will attempt to dequeue two entries for
        // the scom addresses plus the expected
        // EOT entry at the end

        // @TODO via RTC : 130575
        //       Optimize both the RC handling and
        //       FIFO operation infrastructure.
        uint8_t  l_len2dequeue  = 3;
        uint32_t l_scomAddr[3] = {0};
        l_rc = sbeUpFifoDeq_mult (l_len2dequeue, &l_scomAddr[0]);

        // If FIFO access failure
        if (l_rc == SBE_SEC_FIFO_ACCESS_FAILURE)
        {
            // Let command processor routine to handle the RC.
            break;
        }

        // If we didn't receive EOT yet
        if ( (l_rc != SBE_FIFO_RC_EOT_ACKED)  &&
             (l_rc != SBE_FIFO_RC_EOT_ACK_FAILED) )
        {
            // We must have received unexpected data
            // on the upstream FIFO.

            // Flush upstream FIFO until EOT;
            l_len2dequeue = 1;
            l_rc = sbeUpFifoDeq_mult ( l_len2dequeue, NULL, true );

            // We will break out here to force
            // command processor routine to handle the RC.
            // If the RC indicates the receipt of EOT,
            // It would send the appropriate response
            // back into the down stream FIFO.
            // For all other failures, it would force
            // timeout the chipOp operation
            break;
        }

        // If EOT arrived prematurely
        if ( ((l_rc == SBE_FIFO_RC_EOT_ACKED)  ||
              (l_rc == SBE_FIFO_RC_EOT_ACK_FAILED))
              && (l_len2dequeue < 2) )
        {
            // We will break out here to force
            // command processor routine to respond
            // into the downstream FIFO with
            // primary response code as SBE_PRI_INVALID_DATA
            break;
        }

        uint32_t l_sbeDownFifoRespBuf[6] = {0};
        uint32_t l_pcbpibStatus = SBE_PCB_PIB_ERROR_NONE;
        uint8_t  l_len2enqueue  = 0;
        uint8_t  l_index = 0;
        // successfully dequeued two entries for
        // scom address followed by the EOT entry
        if ( ((l_rc == SBE_FIFO_RC_EOT_ACKED)  ||
              (l_rc == SBE_FIFO_RC_EOT_ACK_FAILED))
              && (l_len2dequeue == 2) )
        {
            // @TODO via RTC : 126140
            //       Support Indirect SCOM
            // Data entry 1 : Scom Register Address (0..31)
            // Data entry 2 : Register Address (32..63)
            // For Direct SCOM, will ignore entry 1

            uint64_t l_scomData = 0;
            SBE_TRACE(SBE_FUNC"scomAddr1[0x%08X]", l_scomAddr[1]);
            l_rc = getscom (0, l_scomAddr[1], &l_scomData);

            if (l_rc) // scom failed
            {
                SBE_ERROR(SBE_FUNC"getscom failed, l_rc[0x%08X]", l_rc);
                l_primStatus = SBE_PRI_GENERIC_EXECUTION_FAILURE;
                l_secStatus  = SBE_SEC_GENERIC_FAILURE_IN_EXECUTION;
                l_pcbpibStatus = l_rc;
            }

            if (!l_rc) // successful scom
            {
                SBE_TRACE(SBE_FUNC"getscom succeeds, l_scomData[0x%X]",
                                   l_scomData);

                l_sbeDownFifoRespBuf[0] = (uint32_t)(l_scomData>>32);
                l_sbeDownFifoRespBuf[1] = (uint32_t)(l_scomData);

                // Push the data into downstream FIFO
                l_len2enqueue = 2;
                l_rc = sbeDownFifoEnq_mult (l_len2enqueue,
                                      &l_sbeDownFifoRespBuf[0]);
                if (l_rc)
                {
                    // will let command processor routine
                    // handle the failure
                    break;
                }
                l_index = 2;
            } // end successful scom
        } // end successful dequeue

        // Build the response header packet

        uint8_t l_curIndex = l_index ;
        sbeBuildMinRespHdr(&l_sbeDownFifoRespBuf[0],
                            l_curIndex,
                            l_primStatus,
                            l_secStatus,
                            l_pcbpibStatus,
                            l_index);

        // Now enqueue into the downstream FIFO
        l_len2enqueue = ++l_curIndex - l_index;
        l_rc = sbeDownFifoEnq_mult (l_len2enqueue,
                               &l_sbeDownFifoRespBuf[l_index]);
        if (l_rc)
        {
           // will let command processor routine
           // handle the failure
           break;
        }

    } while(false);

    return l_rc;
    #undef SBE_FUNC
}

/////////////////////////////////////////////////////
//////////////////////////////////////////////////////
uint32_t sbePutScom (uint8_t *i_pArg)
{
    #define SBE_FUNC " sbePutScom "
    SBE_ENTER(SBE_FUNC);

    uint32_t l_rc = SBE_SEC_OPERATION_SUCCESSFUL;

    do
    {
        uint16_t l_primStatus = g_sbeCmdRespHdr.prim_status;
        uint16_t l_secStatus  = g_sbeCmdRespHdr.sec_status ;

        // Will attempt to dequeue four entries for
        // the scom address (two entries) and the
        // corresponding data (two entries) plus
        // the expected EOT entry at the end

        // @TODO via RTC : 130575
        //       Optimize both the RC handling and
        //       FIFO operation infrastructure.
        uint8_t  l_len2dequeue  = 5;
        uint32_t l_scomAddr_Data[5] = {0};
        l_rc = sbeUpFifoDeq_mult (l_len2dequeue, &l_scomAddr_Data[0]);

        // If FIFO access failure
        if (l_rc == SBE_SEC_FIFO_ACCESS_FAILURE)
        {
            // Let command processor routine to handle the RC.
            break;
        }

        // If we didn't receive EOT yet
        if ( (l_rc != SBE_FIFO_RC_EOT_ACKED)  &&
             (l_rc != SBE_FIFO_RC_EOT_ACK_FAILED) )
        {
            // We must have received unexpected data
            // on the upstream FIFO.

            // Flush upstream FIFO until EOT;
            l_len2dequeue = 1;
            l_rc = sbeUpFifoDeq_mult ( l_len2dequeue, NULL, true );

            // We will break out here to force
            // command processor routine to handle the RC.
            // If the RC indicates the receipt of EOT,
            // It would send the appropriate response
            // back into the down stream FIFO.
            // For all other failures, it would force
            // timeout the chipOp operation
            break;
        }

        // If EOT arrived prematurely
        if ( ((l_rc == SBE_FIFO_RC_EOT_ACKED)  ||
              (l_rc == SBE_FIFO_RC_EOT_ACK_FAILED))
              && (l_len2dequeue < 4) )
        {
            // We will break out here to force
            // command processor routine to respond
            // into the downstream FIFO with
            // primary response code as SBE_PRI_INVALID_DATA
            break;
        }

        uint64_t l_scomData = 0;
        uint32_t l_sbeDownFifoRespBuf[4] = {0};
        uint32_t l_pcbpibStatus = SBE_PCB_PIB_ERROR_NONE;
        uint8_t  l_len2enqueue  = 0;
        // successfully dequeued two entries for
        // scom address followed by the EOT entry
        if ( ((l_rc == SBE_FIFO_RC_EOT_ACKED)  ||
              (l_rc == SBE_FIFO_RC_EOT_ACK_FAILED))
              && (l_len2dequeue == 4) )
        {
            // @TODO via RTC : 126140
            //       Support Indirect SCOM
            // Data entry 1 : Scom Register Address (0..31)
            // Data entry 2 : Scom Register Address (32..63)
            // Data entry 3 : Scom Register Data (0..31)
            // Data entry 4 : Scom Register Data (32..63)
            // For Direct SCOM, will ignore entry 1
            l_scomData = ((uint64_t)(l_scomAddr_Data[2])<<32)
                           | (l_scomAddr_Data[3]);

            SBE_DEBUG(SBE_FUNC"scomAddr0[0x%X]"
                              "scomAddr1[0x%X]"
                              "scomData0[0x%X]"
                              "scomData1[0x%X]",
                  l_scomAddr_Data[0], l_scomAddr_Data[1],
                  l_scomAddr_Data[2], l_scomAddr_Data[3]);

            l_rc = putscom (0, l_scomAddr_Data[1], l_scomData);

            if (l_rc) // scom failed
            {
                SBE_ERROR(SBE_FUNC"putscom failed, l_rc[0x%08X]", l_rc);
                l_primStatus = SBE_PRI_GENERIC_EXECUTION_FAILURE;
                l_secStatus  = SBE_SEC_GENERIC_FAILURE_IN_EXECUTION;
                l_pcbpibStatus = l_rc;
            }
        } // end successful dequeue

        // Build the response header packet

        uint8_t  l_curIndex = 0;
        sbeBuildMinRespHdr(&l_sbeDownFifoRespBuf[0],
                            l_curIndex,
                            l_primStatus,
                            l_secStatus,
                            l_pcbpibStatus);

        // Now enqueue into the downstream FIFO
        l_len2enqueue = ++l_curIndex;
        l_rc = sbeDownFifoEnq_mult (l_len2enqueue, &l_sbeDownFifoRespBuf[0]);
        if (l_rc)
        {
           // will let command processor routine
           // handle the failure
           break;
        }

    } while(false);

    return l_rc;
    #undef SBE_FUNC
}
