/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/sbefw/sbecmdreceiver.C $                          */
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
 * @file: ppe/sbe/sbefw/sbecmdreceiver.C
 *
 * @brief This file contains the SBE Command Receiver Thread Routine
 *
 */


#include "sbeexeintf.H"
#include "sbefifo.H"
#include "sbecmdparser.H"
#include "sbeirq.H"
#include "sbetrace.H"
#include "sbe_sp_intf.H"

sbeCmdReqBuf_t g_sbeCmdHdr;
sbeCmdRespHdr_t g_sbeCmdRespHdr;

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
void sbeCommandReceiver_routine(void *i_pArg)
{
    #define SBE_FUNC " sbeCommandReceiver_routine "
    SBE_ENTER(SBE_FUNC);

    do
    {
        uint32_t l_rc = SBE_SEC_OPERATION_SUCCESSFUL;

        // @TODO via RTC: 128944
        //       Read Scratchpad

        // Wait for new data in FIFO or FIFO reset interrupt
        int l_rcPk = pk_semaphore_pend (&g_sbeSemCmdRecv, PK_WAIT_FOREVER);

        // @TODO via RTC: 128658
        //       Review if Mutex protection is required
        //       for all the globals used between threads
        g_sbeCmdRespHdr.prim_status = SBE_PRI_OPERATION_SUCCESSFUL;
        g_sbeCmdRespHdr.sec_status  = SBE_SEC_OPERATION_SUCCESSFUL;
        g_sbeCmdHdr.cmdClass = SBE_CMD_CLASS_UNKNOWN;

        // inner loop for command handling
        do
        {
            // pk API failure
            if (l_rcPk != PK_OK)
            {
                break;
            }

            SBE_DEBUG(SBE_FUNC"unblocked");

            // @TODO via RTC: 128943
            //       Host services / OPAL handling

            // @TODO via RTC: 128945
            // Handle protocol violation if needed (a long term goal)

            // The responsibility of this thread is limited to dequeueing
            // only the first two word entries from the protocol header.
            uint8_t len = sizeof( g_sbeCmdHdr)/ sizeof(uint32_t);
            l_rc = sbeUpFifoDeq_mult ( len, (uint32_t *)&g_sbeCmdHdr );

            // If FIFO reset is requested,
            if (l_rc == SBE_FIFO_RC_RESET)
            {
                SBE_ERROR(SBE_FUNC"FIFO reset received");
                g_sbeCmdRespHdr.prim_status =
                    (uint16_t)SBE_FIFO_RESET_RECEIVED;
                g_sbeCmdRespHdr.sec_status  =
                    (uint16_t)SBE_SEC_GENERIC_FAILURE_IN_EXECUTION;
                break;
            }

            // If we received EOT pre-maturely or
            // got an error while Ack'ing EOT
            if ( (l_rc == SBE_FIFO_RC_EOT_ACKED)  ||
                 (l_rc == SBE_FIFO_RC_EOT_ACK_FAILED) )
            {
                SBE_ERROR(SBE_FUNC"sbeUpFifoDeq_mult failure, "
                          " l_rc=[0x%08X]", l_rc);
                g_sbeCmdRespHdr.prim_status =
                            SBE_PRI_INVALID_DATA;
                g_sbeCmdRespHdr.sec_status  =
                            SBE_SEC_GENERIC_FAILURE_IN_EXECUTION;
                l_rc = SBE_SEC_OPERATION_SUCCESSFUL;
                break;
            }

            // Any other FIFO access issue
            if ( l_rc != SBE_SEC_OPERATION_SUCCESSFUL)
            {
                SBE_ERROR(SBE_FUNC"sbeUpFifoDeq_mult failue, "
                          "l_rc=[0x%08X]", l_rc);
                break;
            }

            // validate the command class and sub-class opcodes
            l_rc = sbeValidateCmdClass (
                        g_sbeCmdHdr.cmdClass,
                        g_sbeCmdHdr.command ) ;

            if (l_rc)
            {
                // Command Validation failed;
                SBE_ERROR(SBE_FUNC"Command validation failed");
                g_sbeCmdRespHdr.prim_status = SBE_PRI_INVALID_COMMAND;
                g_sbeCmdRespHdr.sec_status  = l_rc;
                l_rc = SBE_SEC_OPERATION_SUCCESSFUL;
                break;
            }

            // @TODO via RTC: 126146
            //       validate state machine constraints

        } while (false); // Inner do..while ends

        // Unblock the command processor thread
        // if we could dequeue the header successfully
        if ((l_rcPk == PK_OK) && (l_rc == SBE_SEC_OPERATION_SUCCESSFUL))
        {
            l_rcPk = pk_semaphore_post(&g_sbeSemCmdProcess);
        }

        if ((l_rcPk != PK_OK) || (l_rc != SBE_SEC_OPERATION_SUCCESSFUL))
        {
            // It's likely a code bug or PK failure, or
            // FIFO reset request arrived or any other
            // FIFO access failure.

            // @TODO via RTC : 129166
            //       Review if we need to add ASSERT here

            // Add Error trace, collect FFDC and
            // continue wait for the next interrupt
            SBE_ERROR(SBE_FUNC"Unexpected failure, "
                "l_rcPk=[%d], g_sbeSemCmdProcess.count=[%d], l_rc=[%d]",
                l_rcPk, g_sbeSemCmdProcess.count, l_rc);

            pk_irq_enable(SBE_IRQ_SBEFIFO_DATA);

            continue;
        }

        SBE_DEBUG(SBE_FUNC"Posted g_sbeSemCmdProcess, "
               "g_sbeSemCmdProcess.count=[%d]", g_sbeSemCmdProcess.count);

    } while (true); // thread always exists
    #undef SBE_FUNC
}
