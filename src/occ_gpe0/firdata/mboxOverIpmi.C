/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/firdata/mboxOverIpmi.C $                          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2018                             */
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

extern "C" {

#include "native.h"
#include "gpe_util.h"
#include "ast_mboxdd.h"

int ipmi_sendCommand(mboxMessage_t *io_msg, int i_arg_size);

}

#include "ipmidd.H"

enum
{
    IPMI_MAX_TRIES = 10000,
};


/**
 * Wrap the astMbox message in an IPMI message and send/recv over IPMI
 * interface.
 */
int ipmi_sendCommand(mboxMessage_t *io_msg, int i_arg_size)
{
    int rc = 0;
    int i = 0;
    uint8_t * l_data = reinterpret_cast<uint8_t*>(io_msg);
    int l_data_size = i_arg_size + 2; //#args + iv_cmd + iv_seq

    IpmiDD l_ipmidd(IPMI::NETFUN_IBM,
                    IPMI::PNOR_CMD,
                    io_msg->iv_seq, // seems unique enough to use this
                    IPMI::CC_UNKBAD,
                    l_data,
                    l_data_size // #args + iv_cmd + iv_seq
                   );

    do
    {
        for(i = 0; i < IPMI_MAX_TRIES; ++i)
        {
            rc = l_ipmidd.send();
            if(rc != RC_IPMIDD_NOT_IDLE)
            {
                break;
            }
            busy_wait(100); // 100 us
        }

        // Note: OCC_GPE can't generate error logs.
        // For whatever reason errors are not returned to the OCC_405 as it
        // probably can't report them either in the current state.
        // If the write to PNOR fails then the fir data is simply not captured
        // and only the PK trace has the reason.
        // It's not clear if the PK trace can be extracted w/o Cronus.
        if(i == IPMI_MAX_TRIES)
        {
            PK_TRACE("ipmi_sendCommand: cmd 0x%x TIMEOUT waiting to send",
                     io_msg->iv_cmd);
            rc = RC_IPMIDD_TIMEOUT;
        }

        if( rc )
        {
            break;
        }

        // Wait for response.
        for(i = 0; i < IPMI_MAX_TRIES; ++i)
        {
            busy_wait(100);  // 100 us
            rc = l_ipmidd.pollCtrl();

            // keep waiting if IDLE or BUSY and ignore IPMI events
            if ((rc != RC_IPMIDD_IDLE) &&
                (rc != RC_IPMI_EVENT) &&
                (rc != RC_IPMI_BUSY))
            {
                break;
            }
        }

        if(i == IPMI_MAX_TRIES)
        {
            PK_TRACE("ipmi_sendCommand: cmd 0x%x TIMEOUT "
                     "waiting for a response!",
                     io_msg->iv_cmd);

            rc = RC_IPMIDD_TIMEOUT;
            break;
        }

        if( rc )
        {
            PK_TRACE("ipmi_sendCommand: cmd 0x%x Failed rc = %d",
                     io_msg->iv_cmd,
                     rc);
            break;
        }

        // Return reponse code if not CC_OK;
        if(l_ipmidd.iv_cc != IPMI::CC_OK)
        {
            PK_TRACE("ipmi_sendCommand: cmd 0x%x. IPMI completion code = 0x%x",
                     io_msg->iv_cmd,
                     l_ipmidd.iv_cc);

            rc = (int)l_ipmidd.iv_cc;
            break;
        }

        l_data_size = l_ipmidd.iv_data_len;

        if(l_data_size > (int)sizeof(mboxMessage_t))
        {
            PK_TRACE("ipmi_sendCommand: recvd data packet too big (%d)",
                     l_data_size);
            rc = RC_IPMIDD_INVALID_RESP_SIZE;
            break;
        }


        if( io_msg->iv_seq != l_ipmidd.iv_seq )
        {
            PK_TRACE("ipmi_sendCommand: cmd 0x%x bad sequence number "
                     "in mboxMessage.  sent %d recvd %d",
                     io_msg->iv_cmd,
                     io_msg->iv_seq,
                     l_ipmidd.iv_seq);

            rc = RC_BAD_SEQUENCE;
            break;
        }

        uint8_t org_cmd = io_msg->iv_cmd;

        // This overwrites io_msg object with recvd ipmi msg payload
        for(int i = 0; i < l_data_size; ++i)
        {
            l_data[i] = l_ipmidd.iv_data[i];
        }

        // The GET_INFO command does not have the same response format over
        // IPMI as it does over ast-mbox. This work-around fixes that.
        if(org_cmd == MBOX_C_GET_MBOX_INFO)
        {
           // Version 1 is the same between ast and IPMI
           if(get8(io_msg,0) > 1)
            {
                io_msg->iv_args[5] = io_msg->iv_args[1];
                io_msg->iv_args[6] = io_msg->iv_args[2];
                io_msg->iv_args[7] = io_msg->iv_args[3];
                io_msg->iv_args[8] = io_msg->iv_args[4];
            }
        }

    } while(0);

    return rc;
}

