/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/firdata/ipmidd.C $                                */
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
/**
 *  @file ipmidd.c
 *
 *  @brief Implementation of the IPMI Device Driver
 */

/*****************************************************************************/
// I n c l u d e s
/*****************************************************************************/
//#include <devicefw/driverif.H>
//#include <trace/interface.H>


extern "C" {

#include "native.h"
#include "lpc.h"
#include "gpe_util.h"

}

#include "ipmidd.H"
//#include "ipmirp.H"
//#include <ipmi/ipmiif.H>
//#include <initservice/initserviceif.H>
//#include <util/align.H>

//#include <config.h>

//#include <sys/msg.h>
//#include <errno.h>

//#include <sys/time.h>
//#include <sys/task.h>

/*****************************************************************************/
// D e f i n e s
/*****************************************************************************/

#if defined(IPMI_TRAC)
#undef IPMI_TRAC
#endif

#define IPMI_TRAC(printf_string,args...) \
    PK_TRACE(printf_string,##args)

#define ERR_MRK "E>"
#define INFO_MRK "I>"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Read an address from LPC space
 */
int IpmiDD::readLPC(const uint32_t i_addr, uint8_t& o_data)
{
    int rc = 0;
    rc = lpc_read(LPC_TRANS_IO, i_addr,
                  static_cast<uint8_t*>(&o_data),
                  sizeof(uint8_t));
    return rc;
}

/**
 * @brief Write an address from LPC space
 */
int IpmiDD::writeLPC(const uint32_t i_addr,
                            uint8_t i_data)
{
    int rc = 0;
    rc = lpc_write(LPC_TRANS_IO,
                   i_addr,
                   static_cast<uint8_t*>(&i_data),
                   sizeof(uint8_t));
    return rc;
}


/**
 * @brief Poll the control register
 */
int IpmiDD::pollCtrl(void)
{
    IPMI_TRAC(">>pollCtrl" );
    int rc = 0;

    uint8_t ctrl = 0;

    rc = readLPC(REG_CONTROL, ctrl);

    if (rc)
    {
        // Not sure there's much we can do here but trace.
        // Likely a scom fail.
        IPMI_TRAC(ERR_MRK "polling loop encountered an error."
                  " rc = %d, exiting",rc);
    }
    else
    {
        if ((ctrl & IDLE_STATE) == 0)
        {
            rc = RC_IPMIDD_IDLE;
        }
        // If we see the B2H_ATN, there's a response waiting
        else if (ctrl & CTRL_B2H_ATN)
        {
            rc = receive();
        }

        // If we see the SMS_ATN, there's an event waiting
        else if (ctrl & CTRL_SMS_ATN)
        {
            IPMI_TRAC(INFO_MRK "sms/event");
            receive();
            //Not sure what to do about events or if we even need to handle them.
            // Reject them all?

            // Clear the SMS bit.
            rc = writeLPC(REG_CONTROL, CTRL_SMS_ATN);

            if (rc)
            {
                IPMI_TRAC(ERR_MRK "pollCtrl: IPMI SMS_ATN on, "
                          "Clear failed rc = %d",rc);
            }

            // event was already traced in receive()
            // caller can decide what to do with this
            rc = RC_IPMI_EVENT;
        }
        else
        {
            rc = RC_IPMI_BUSY;
        }
    }
    return rc;
}

/**
 * @brief Performs a reset of the BT hardware
 */
inline int IpmiDD::reset(void)
{
    IPMI_TRAC("resetting the IPMI BT interface");
    return writeLPC(REG_INTMASK, INT_BMC_HWRST);
}

/**
 * @brief Performs an IPMI Message Write Operation
 */
int IpmiDD::send(void)
{
    int err = 0;
    uint8_t    ctrl = 0;
    int         l_len = iv_data_len + 3; // Add header size

    do
    {
        err = readLPC(REG_CONTROL, ctrl);
        if (err) { break; }

        // If the interface isn't idle, tell the caller to come back
        if ((ctrl & IDLE_STATE) != 0)
        {
            return RC_IPMIDD_NOT_IDLE;
        }

        // Tell the interface we're writing. Per p. 135 of the
        // spec we *do not* set H_BUSY.
        err = writeLPC(REG_CONTROL, CTRL_CLR_WR_PTR);
        if (err)
        {
            break;
        }

        err = writeLPC(REG_HOSTBMC, l_len);
        if (err)
        {
            break;
        }

        err = writeLPC(REG_HOSTBMC, iv_netfun);
        if (err)
        {
            break;
        }

        err = writeLPC(REG_HOSTBMC, iv_seq);
        if (err)
        {
            break;
        }

        err = writeLPC(REG_HOSTBMC, iv_cmd);
        if (err)
        {
            break;
        }


        for( int i = 0; (i < iv_data_len) && (err == 0); ++i)
        {
            err = writeLPC(REG_HOSTBMC, iv_data[i]);
            if (err)
            {
                break;
            }
        }
        if (err)
        {
            break;
        }

        IPMI_TRAC("I> write %x:%x seq %x len %x",
                  iv_netfun, iv_cmd, iv_seq,
                  iv_data_len);

        // If all is well, alert the host we sent bits.
        err = writeLPC(REG_CONTROL, CTRL_H2B_ATN);
        if (err)
        {
            break;
        }

    } while(false);

    // If we have an error, try to reset the interface.
    if (err)
    {
        reset();
    }

    return err;
}

/**
 * @brief Read a response to an issued command, or an sms
 */
//int IpmiDD::receive(IPMI::BTMessage* o_msg)
int IpmiDD::receive(void)
{
    int         err = 0;
    int         l_len = 0;
    uint8_t     ctrl = 0;
    bool        marked_busy = false;
    uint8_t     byte = 0;

    do
    {
        err = readLPC(REG_CONTROL, ctrl);
        if (err)
        {
            break;
        }

        // Tell the interface we're busy.
        err = writeLPC(REG_CONTROL, CTRL_H_BUSY);
        if (err)
        {
            break;
        }

        marked_busy = true;

        // Clear the pending state from the control register.
        // Note the spec distinctly says "after setting H_BUSY,
        // the host should clear this bit" - not at the same time.
        // This is the hand-shake; H_BUSY gates the BMC which allows
        // us to clear the ATN bits. Don't get fancy.
        err = writeLPC(REG_CONTROL, CTRL_B2H_ATN);
        if (err)
        {
            break;
        }

        // Tell the interface we're reading
        err = writeLPC(REG_CONTROL, CTRL_CLR_RD_PTR);
        if (err)
        {
            break;
        }

        // The first byte is the length, grab it so we can allocate a buffer.
        err = readLPC(REG_HOSTBMC, byte);
        if (err)
        {
            break;
        }

        l_len = byte;
        l_len -= 4;
        if(l_len > MAX_PACKET_DATA_SIZE)
        {
            IPMI_TRAC(ERR_MRK "IpmiDD::receive() Data larger than expected,"
                      " tuncating! size = %d",
                      l_len);

            l_len = MAX_PACKET_DATA_SIZE;
        }

        iv_data_len = static_cast<uint8_t>(l_len);

        err = readLPC(REG_HOSTBMC, iv_netfun);
        if (err)
        {
            break;
        }

        err = readLPC(REG_HOSTBMC, iv_seq);
        if (err)
        {
            break;
        }

        err = readLPC(REG_HOSTBMC, iv_cmd);
        if (err)
        {
            break;
        }

        err = readLPC(REG_HOSTBMC, iv_cc);
        if (err)
        {
            break;
        }

        for( int i = 0; (i < l_len) && (err == 0); ++i)
        {
            err = readLPC(REG_HOSTBMC, iv_data[i]);
            if(err)
            {
                break;
            }
        }
        if (err)
        {
            break;
        }

    } while(0);

    if (marked_busy)
    {
        // Clear the busy state (write 1 to toggle). Note if we get
        // an error from the writeLPC, we toss it and return the first
        // error as it likely has better information in it.
        writeLPC(REG_CONTROL, CTRL_H_BUSY);
    }


    IPMI_TRAC("I> read b2h %x:%x seq %x cc %x",
              iv_netfun,
              iv_cmd,
              iv_seq,
              iv_cc);

    return err;
}

/**
 * @brief  Constructor
 */
IpmiDD::IpmiDD(uint8_t i_netfun,
               uint8_t i_cmd,
               uint8_t i_seq,
               uint8_t i_cc,
               uint8_t * i_data,
               uint8_t i_data_len)
    :
        iv_netfun(i_netfun),
        iv_seq(i_seq),
        iv_cmd(i_cmd),
        iv_cc(i_cc),
        iv_data_len(i_data_len)
{
    if(iv_data_len > MAX_PACKET_DATA_SIZE)
    {
        iv_data_len = MAX_PACKET_DATA_SIZE;
    }
    for(int i = 0; i < iv_data_len; ++i)
    {
        iv_data[i] = i_data[i];
    }
}
