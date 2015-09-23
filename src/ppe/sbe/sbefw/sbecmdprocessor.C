/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/sbefw/sbecmdprocessor.C $                         */
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
 * @file: ppe/sbe/sbefw/sbecmdprocessor.C
 *
 * @brief This file contains the SBE Command processing Thread Routines
 *
 */


#include "sbeexeintf.H"
#include "sbefifo.H"
#include "sbecmdparser.H"
#include "sbeirq.H"
#include "sbetrace.H"
#include "sbe_sp_intf.H"

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void sbeSyncCommandProcessor_routine(void *i_pArg)
{
    #define SBE_FUNC " sbeSyncCommandProcessor_routine "
    SBE_ENTER(SBE_FUNC);

    do
    {
        uint32_t l_rc = SBE_SEC_OPERATION_SUCCESSFUL;
        uint8_t  l_dist2StatusHdr        = 0;
        uint32_t l_sbeDownFifoRespBuf[4] = {0};

        // Wait for new command processing
        int l_rcPk = pk_semaphore_pend (
                    &g_sbeSemCmdProcess, PK_WAIT_FOREVER);

        do
        {
            uint16_t l_primStatus = g_sbeCmdRespHdr.prim_status;
            uint16_t l_secStatus  = g_sbeCmdRespHdr.sec_status ;
            SBE_DEBUG (SBE_FUNC"l_primStatus=[0x%04X], l_secStatus=[0x%04X]",
                            l_primStatus, l_secStatus);

            // PK API failure
            if (l_rcPk != PK_OK)
            {
                SBE_ERROR(SBE_FUNC"pk_semaphore_pend failed, "
                          "l_rcPk=%d, g_sbeSemCmdRecv.count=%d",
                           l_rcPk, g_sbeSemCmdRecv.count);

                // if the command receiver thread already updated
                // the response status codes, don't override them.
                if (l_primStatus != SBE_PRI_OPERATION_SUCCESSFUL)
                {
                    l_primStatus = SBE_PRI_INTERNAL_ERROR;
                    l_secStatus  = SBE_SEC_OS_FAILURE;
                }
            }

            SBE_DEBUG(SBE_FUNC"unblocked");

            // if there was a PK API failure or the
            // command receiver thread indicated of
            // a failure due to
            //    Command Validation or
            //    FIFO Reset request
            if (l_primStatus)
            {
                uint8_t  l_len2dequeue           = 0;
                switch (l_primStatus)
                {
                    case SBE_FIFO_RESET_RECEIVED:
                        SBE_ERROR(SBE_FUNC"FIFO reset received");
                        l_rc = SBE_FIFO_RC_RESET;
                        break;

                    case SBE_PRI_INVALID_COMMAND:
                        // Command or SBE state validation failed
                        // just follow through

                    case SBE_PRI_INTERNAL_ERROR:
                        // Flush out the upstream FIFO till EOT arrives
                        l_len2dequeue = 1;
                        l_rc = sbeUpFifoDeq_mult (l_len2dequeue, NULL, true);
                        if (l_rc == SBE_FIFO_RC_RESET)
                        {
                           break;
                        }

                        // Not handling any other RC from sbeUpFifoDeq_mult
                        // while flushing out to keep this code simple.

                        // Don't break here to force the flow through
                        // the next case to enqueue the response into
                        // the downstream FIFO

                    case SBE_PRI_INVALID_DATA:
                        // SBE caller already wrongly sent EOT
                        // before sending two mandatory header entries
                        //
                        // enqueue the response payload now into
                        // the downstream FIFO

                        // @TODO via RTC : 130575
                        //       Optimize RC handling infrastructure code

                        // Build the response packet first
                        sbeBuildMinRespHdr(&l_sbeDownFifoRespBuf[0],
                                            l_dist2StatusHdr,
                                            l_primStatus,
                                            l_secStatus,
                                            0);

                        // Now enqueue into the downstream FIFO
                        l_rc = sbeDownFifoEnq_mult (++l_dist2StatusHdr,
                                                &l_sbeDownFifoRespBuf[0]);
                        if (l_rc)
                        {
                            SBE_ERROR(SBE_FUNC"sbeDownFifoEnq_mult failure,"
                                      " l_rc[0x%X]", l_rc);
                        }
                        break;

                        // Signal EOT in Downstream FIFO
                        l_rc = sbeDownFifoSignalEot();
                        if (l_rc)
                        {
                            SBE_ERROR(SBE_FUNC"sbeDownFifoSignalEot failure,"
                                   " l_rc[0x0%08X]", l_rc);
                            break;
                        }

                    default:
                        break;
                } // End switch

                break;
            }

            SBE_DEBUG(SBE_FUNC"New cmd arrived, g_sbeSemCmdProcess.count=%d",
                         g_sbeSemCmdProcess.count);

            uint8_t  l_cmdClass  = 0;
            uint8_t  l_cmdOpCode = 0;
            uint32_t (*l_pFuncP) (uint8_t *) ;

            // @TODO via RTC: 128658
            //       Review if Mutex protection is required
            //       for all the globals used between threads
            l_cmdClass  = g_sbeCmdHdr.cmdClass;
            l_cmdOpCode = g_sbeCmdHdr.command;

            // Get the command function
            l_pFuncP = sbeFindCmdFunc (l_cmdClass, l_cmdOpCode) ;

            if (!l_pFuncP)
            {
                // No Supported function found
                SBE_ERROR(SBE_FUNC"No supported function");
                l_rc = SBE_FUNC_NOT_SUPPORTED;

                // @TODO via RTC : 129166
                //       force assert
                break;
            }

            // Call the ChipOp function
            l_rc = l_pFuncP ((uint8_t *)i_pArg);

        } while(false); // Inner do..while loop ends here

        SBE_DEBUG(SBE_FUNC"l_rc=[0x%08X]", l_rc);

        // Handle FIFO reset case
        if (l_rc == SBE_FIFO_RC_RESET)
        {
            // @TODO via RTC : 126147
            //       Handle FIFO reset flow
            pk_irq_enable(SBE_IRQ_SBEFIFO_DATA);
            continue;
        }

        switch (l_rc)
        {
            // EOT arrived prematurely in upstream FIFO
            // or there were unexpected data in upstream
            // FIFO
            case SBE_FIFO_RC_EOT_ACKED:
            case SBE_FIFO_RC_EOT_ACK_FAILED:
                SBE_ERROR(SBE_FUNC"Received unexpected EOT, l_rc[0x%08X]",
                                 l_rc);
                sbeBuildMinRespHdr(&l_sbeDownFifoRespBuf[0],
                                    l_dist2StatusHdr,
                                    SBE_PRI_INVALID_DATA,
                                    SBE_SEC_GENERIC_FAILURE_IN_EXECUTION,
                                    0);
                l_rc = sbeDownFifoEnq_mult (++l_dist2StatusHdr,
                                        &l_sbeDownFifoRespBuf[0]);
                if (l_rc)
                {
                    SBE_ERROR(SBE_FUNC"sbeDownFifoEnq_mult failure,"
                        " l_rc[0x0%08X]", l_rc);
                    // not attempting to signal EOT
                    break;
                }
                // Follow through to signal EOT in downstream

            case SBE_SEC_OPERATION_SUCCESSFUL: // Successful execution
                // Signal EOT in Downstream FIFO
                l_rc = sbeDownFifoSignalEot();
                if (l_rc)
                {
                    SBE_ERROR(SBE_FUNC"sbeDownFifoSignalEot failure,"
                        " l_rc[0x0%08X]", l_rc);
                }
                SBE_INFO(SBE_FUNC"ChipOp Done");
                break;

             default:
                break;
        }

        // @TODO via RTC : 126147
        //       Review all the scenarios
        // Enable the new data available interrupt
        pk_irq_enable(SBE_IRQ_SBEFIFO_DATA);

    } while(true); // Thread always exists
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void sbeAsyncCommandProcessor_routine(void *arg)
{
    SBE_TRACE("sbeAsyncCommandProcessor Thread started");

    do
    {
        //  @TODO RTC via : 130392
        //        Add infrastructure for host interface

    } while(0);
}
