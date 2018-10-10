#include <pnor_ipmi.h>
#include <norflash.h>
#include <lpc.h>

// Local functions
void init_ipmi_msg(ipmi_msg_t * i_msg)
{
    static uint8_t l_seq = 0;
    i_msg->iv_netfun = NETFUN_IBM;
    i_msg->iv_seq = ++l_seq;
    i_msg->iv_cmd = PNOR_CMD;
    i_msg->iv_cc = CC_UNKBAD;
    i_msg->iv_hio_msg.iv_seq = l_seq;
    i_msg->iv_hio_msg_len = 0;
}

// Internal
int readLPC(uint32_t i_addr,
            uint8_t * o_data)
{
    int rc = 0;
    rc = lpc_read(LPC_TRANS_IO,
                  i_addr,
                  o_data,
                  sizeof(uint8_t));
    return rc;
}

// Internal
int writeLPC(uint32_t i_addr,
             uint8_t  i_data)
{
    int rc = 0;
    rc = lpc_write(LPC_TRANS_IO,
                   i_addr,
                   &i_data,
                   sizeof(uint8_t));
    return rc;
}
/**
 * @brief Read a response to an issued command, or an sms
 */

int receive(ipmi_msg_t * o_msg)
{
    int         err = 0;
    int         l_len = 0;
    uint8_t     ctrl = 0;
    bool        marked_busy = false;
    uint8_t     byte = 0;

    do
    {
        err = readLPC(REG_CONTROL, &ctrl);
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
        err = readLPC(REG_HOSTBMC, &byte);
        if (err)
        {
            break;
        }

        l_len = byte;
        l_len -= 4;
        if(l_len > MAX_PACKET_DATA_SIZE)
        {
            TRACFCOMP(ERR_MRK "IpmiDD::receive() Data larger than expected,"
                      " tuncating! size = %d",
                      l_len);

            l_len = MAX_PACKET_DATA_SIZE;
        }

        o_msg->iv_hio_msg_len = (uint8_t)l_len;

        err = readLPC(REG_HOSTBMC, &o_msg->iv_netfun);
        if (err)
        {
            break;
        }

        err = readLPC(REG_HOSTBMC, &o_msg->iv_seq);
        if (err)
        {
            break;
        }

        err = readLPC(REG_HOSTBMC, &o_msg->iv_cmd);
        if (err)
        {
            break;
        }

        err = readLPC(REG_HOSTBMC, &o_msg->iv_cc);
        if (err)
        {
            break;
        }

        uint8_t * l_data = (uint8_t *)(&o_msg->iv_hio_msg);
        int i;
        for( i = 0; (i < l_len) && (err == 0); ++i)
        {
            err = readLPC(REG_HOSTBMC, &(l_data[i]));
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


    TRACFCOMP("I> read b2h %x:%x seq %x cc %x",
              o_msg->iv_netfun,
              o_msg->iv_cmd,
              o_msg->iv_seq,
              o_msg->iv_cc);

    return err;
}

/**
 * @brief Poll the control register
 */
int pollCtrl(ipmi_msg_t * o_msg)
{
    TRACFCOMP(">>pollCtrl" );
    int rc = 0;

    uint8_t ctrl = 0;

    rc = readLPC(REG_CONTROL, &ctrl);

    if (rc)
    {
        // Not sure there's much we can do here but trace.
        // Likely a scom fail.
        TRACFCOMP(ERR_MRK "polling loop encountered an error."
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
            rc = receive(o_msg);
        }

        // If we see the SMS_ATN, there's an event waiting
        else if (ctrl & CTRL_SMS_ATN)
        {
            receive(o_msg);

            // There should not be any events during FIR Capture.
            // Reject them.
            TRACFCOMP(ERR_MRK "ipmi pollCtrl: Unexpected event received! %08x",
                      *((uint32_t *)(o_msg)));

            // Clear the SMS bit.
            rc = writeLPC(REG_CONTROL, CTRL_SMS_ATN);

            if (rc)
            {
                TRACFCOMP(ERR_MRK "pollCtrl: IPMI SMS_ATN on, "
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
inline int ipmi_reset(void)
{
    TRACFCOMP("ipmi_resetting the IPMI BT interface");
    return writeLPC(REG_INTMASK, INT_BMC_HWRST);
}

//Internal  Send IPMI msg to BMC
int ipmi_send(ipmi_msg_t * i_msg)
{
    int err = 0;
    uint8_t    ctrl = 0;
    int         l_len = i_msg->iv_hio_msg_len + 3; // Add header size

    do
    {
        err = readLPC(REG_CONTROL, &ctrl);
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

        err = writeLPC(REG_HOSTBMC, i_msg->iv_netfun);
        if (err)
        {
            break;
        }

        err = writeLPC(REG_HOSTBMC, i_msg->iv_seq);
        if (err)
        {
            break;
        }

        err = writeLPC(REG_HOSTBMC, i_msg->iv_cmd);
        if (err)
        {
            break;
        }


        uint8_t * l_data = (uint8_t*)(&(i_msg->iv_hio_msg));
        int i;
        for( i = 0; (i < i_msg->iv_hio_msg_len) && (err == 0); ++i)
        {
            err = writeLPC(REG_HOSTBMC, l_data[i]);
            if (err)
            {
                break;
            }
        }
        if (err)
        {
            break;
        }

        TRACFCOMP("I> write %x:%x seq %x len %x",
                  i_msg->iv_netfun,
                  i_msg->iv_cmd,
                  i_msg->iv_seq,
                  i_msg->iv_hio_msg_len);

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
        ipmi_reset();
    }

    return err;
}

// Internal
errorHndl_t send_ipmi_msg(ipmi_msg_t * i_msg)
{
    int rc = NO_ERROR;
    int i;
    ipmi_msg_t * ipmi_msg = i_msg;

    do
    {
        for(i = 0; i < IPMI_MAX_TRIES; ++i)
        {
            rc = ipmi_send(ipmi_msg);
            if(rc != RC_IPMIDD_NOT_IDLE)
            {
                break;
            }
            sleep(100000); // 100 us
        }

        if(i == IPMI_MAX_TRIES)
        {
            TRACFCOMP(ERR_MRK"send_ipmi_msg: cmd 0x%x TIMEOUT waiting to send",
                      ipmi_msg->iv_hio_msg.iv_cmd);
            rc = RC_IPMIDD_TIMEOUT;
        }

        if(rc)
        {
            break;
        }

        // Wait for response.
        for(i = 0; i < IPMI_MAX_TRIES; ++i)
        {
            sleep(100000);  // 100 us
            rc = pollCtrl(ipmi_msg);

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
            TRACFCOMP(ERR_MRK"send_ipmi_msg: cmd 0x%x TIMEOUT "
                     "waiting for a response!",
                     ipmi_msg->iv_hio_msg.iv_cmd);

            rc = RC_IPMIDD_TIMEOUT;
            break;
        }

        if( rc )
        {
            TRACFCOMP(ERR_MRK"send_ipmi_msg: cmd 0x%x Failed rc = %d",
                     ipmi_msg->iv_hio_msg.iv_cmd,
                     rc);
            break;
        }

        // Return reponse code if not CC_OK;
        if(ipmi_msg->iv_cc != CC_OK)
        {
            TRACFCOMP(ERR_MRK"ipmi_sendCommand: cmd 0x%x. IPMI completion code = 0x%x",
                     ipmi_msg->iv_hio_msg.iv_cmd,
                     ipmi_msg->iv_cc);

            rc = (int)ipmi_msg->iv_cc;
            break;
        }

    } while(0);

    return rc;
}

// External
errorHndl_t getInfo(hioPnorInfo_t * o_pnorInfo)
{
    errorHndl_t l_err = NO_ERROR;
    ipmi_msg_t ipmi_msg;

    o_pnorInfo->iv_curWindowOpen = false;
    o_pnorInfo->iv_useIPMI = false;
    do
    {
        init_ipmi_msg(&ipmi_msg);
        ipmi_msg.iv_hio_msg.iv_cmd = HIOMAP_C_GET_INFO;
        ipmi_msg.iv_hio_msg.iv_args[0] = 2; //version 2
        ipmi_msg.iv_hio_msg_len = 3; //  1 args + len(iv_seq and iv_cmd)

        l_err = send_ipmi_msg(&ipmi_msg);
        if(l_err != NO_ERROR)
        {
            TRACFCOMP(ERR_MRK"getInfo: ping BMC faild with rc = 0x%x",l_err);
            break;
        }

        hio_msg_t * hio_msg = &(ipmi_msg.iv_hio_msg);
        uint32_t proto_version = get8(hio_msg, 0);
        o_pnorInfo->iv_protocolVersion = proto_version;

        if(proto_version == 1)
        {
            o_pnorInfo->iv_blockShift = 12;
            o_pnorInfo->iv_readWindowSize =
                get16(hio_msg, 1) << o_pnorInfo->iv_blockShift;
            o_pnorInfo->iv_writeWindowSize =
                get16(hio_msg, 3) << o_pnorInfo->iv_blockShift;
        }
        else
        {
            o_pnorInfo->iv_blockShift = get8(hio_msg, 1);
        }

        // Get the flash size
        init_ipmi_msg(&ipmi_msg);
        ipmi_msg.iv_hio_msg.iv_cmd = HIOMAP_C_GET_FLASH_INFO;
        ipmi_msg.iv_hio_msg_len = 2; // 0 args

        l_err = send_ipmi_msg(&ipmi_msg);

        if(l_err)
        {
            TRACFCOMP(ERR_MRK"getInfo: failed get flash size with rc = 0x%x",l_err);
            break;
        }

        if(proto_version == 1)
        {
            o_pnorInfo->iv_flashSize = get32(hio_msg, 0);
            o_pnorInfo->iv_flashEraseSize = get32(hio_msg, 4);
        }
        else
        {
            o_pnorInfo->iv_flashSize =
                get16(hio_msg, 0) << o_pnorInfo->iv_blockShift;
            o_pnorInfo->iv_flashEraseSize =
                get16(hio_msg,2) << o_pnorInfo->iv_blockShift;
        }
        o_pnorInfo->iv_useIPMI = true;

    } while(0);

    return l_err;
}

// Internal
errorHndl_t adjustWindow(hioPnorInfo_t* io_hioInfo,
                         bool i_isWrite,
                         uint32_t i_reqAddr,
                         uint32_t i_reqSize,
                         uint32_t *o_lpcAddr,
                         uint32_t *o_chunkLen)
{
    errorHndl_t l_err = NO_ERROR;
    uint32_t l_pos, l_wSize, l_reqSize;

    do
    {
        /*
         * Handle the case where the window is already opened, is of
         * the right type and contains the requested address.
         */
        uint32_t l_wEnd = io_hioInfo->iv_curWindowOffset +
                              io_hioInfo->iv_curWindowSize;

        /* A read request can be serviced by a write window */
        if (io_hioInfo->iv_curWindowOpen &&
            (io_hioInfo->iv_curWindowWrite || !i_isWrite) &&
            i_reqAddr >= io_hioInfo->iv_curWindowOffset && i_reqAddr < l_wEnd)
        {
            uint32_t l_gap = (l_wEnd - i_reqAddr);

            *o_lpcAddr = io_hioInfo->iv_curWindowLpcOffset +
                          (i_reqAddr - io_hioInfo->iv_curWindowOffset);
            if (i_reqSize <= l_gap)
            {
                *o_chunkLen = i_reqSize;
            }
            else
            {
                *o_chunkLen = l_gap;
            }
            return NO_ERROR;
        }

        /*
         * We need a window change, mark it closed first
         */
        io_hioInfo->iv_curWindowOpen = false;

        /*
         * Then open the new one at the right position. The required
         * alignment differs between protocol versions
         */
        TRACFCOMP("astMboxDD::adjustMboxWindow using protocol version: %d",
                    io_hioInfo->iv_protocolVersion);
        if (io_hioInfo->iv_protocolVersion == 1)
        {
            l_wSize = i_isWrite ? io_hioInfo->iv_writeWindowSize
                                : io_hioInfo->iv_readWindowSize;
            l_pos = i_reqAddr & ~(l_wSize - 1);
            l_reqSize = 0;
        }
        else
        {
            uint32_t l_blockMask = (1u << io_hioInfo->iv_blockShift) - 1;
            l_wSize = 0;
            l_pos = i_reqAddr & ~l_blockMask;
            l_reqSize = (((i_reqAddr + i_reqSize) + l_blockMask) & ~l_blockMask)
                          - l_pos;
        }

        ipmi_msg_t ipmi_msg;
        init_ipmi_msg(&ipmi_msg);
        if (i_isWrite)
        {
            ipmi_msg.iv_hio_msg.iv_cmd = HIOMAP_C_CREATE_WRITE_WINDOW;
        }
        else
        {
            ipmi_msg.iv_hio_msg.iv_cmd = HIOMAP_C_CREATE_READ_WINDOW;
        }

        put16(&(ipmi_msg.iv_hio_msg), 0, l_pos >> io_hioInfo->iv_blockShift);
        put16(&(ipmi_msg.iv_hio_msg), 2, l_reqSize >> io_hioInfo->iv_blockShift);
        ipmi_msg.iv_hio_msg_len = 6;
        l_err = send_ipmi_msg(&ipmi_msg);

        if (l_err)
        {
            TRACFCOMP(ERR_MRK"adjustWindow: Error creating PNOR flash Window");
            break;
        }

        io_hioInfo->iv_curWindowLpcOffset =
                        (get16(&(ipmi_msg.iv_hio_msg),0)) << io_hioInfo->iv_blockShift;

        if (io_hioInfo->iv_protocolVersion == 1)
        {
            io_hioInfo->iv_curWindowOffset = l_pos;
            io_hioInfo->iv_curWindowLpcOffset =
                        (get16(&(ipmi_msg.iv_hio_msg),0)) << io_hioInfo->iv_blockShift;
            io_hioInfo->iv_curWindowSize = l_wSize;
        }
        else
        {
            io_hioInfo->iv_curWindowLpcOffset = (get16(&(ipmi_msg.iv_hio_msg),0))
                             << io_hioInfo->iv_blockShift;
            io_hioInfo->iv_curWindowSize = (get16(&(ipmi_msg.iv_hio_msg),2))
                             << io_hioInfo->iv_blockShift;
            io_hioInfo->iv_curWindowOffset = (get16(&(ipmi_msg.iv_hio_msg),4))
                             << io_hioInfo->iv_blockShift;
        }

        io_hioInfo->iv_curWindowOpen = true;
        io_hioInfo->iv_curWindowWrite = i_isWrite;

    }
    while (true);

    return l_err;
}

// Internal
errorHndl_t writeDirty(hioPnorInfo_t* i_hioInfo,
                       uint32_t i_addr,
                       uint32_t i_size)
{
    /* To pass a correct "size" for both protocol versions, we
     * calculate the block-aligned start and end.
     */
    uint32_t l_blockMask = (1u << i_hioInfo->iv_blockShift) - 1;
    uint32_t l_start     = i_addr & ~l_blockMask;
    uint32_t l_end       = ((i_addr + i_size) + l_blockMask) & ~l_blockMask;

    ipmi_msg_t ipmi_msg;
    hio_msg_t * hio_msg = &(ipmi_msg.iv_hio_msg);
    init_ipmi_msg(&ipmi_msg);
    hio_msg->iv_cmd = HIOMAP_C_MARK_WRITE_DIRTY;

    if (i_hioInfo->iv_protocolVersion == 1)
    {
        put16(hio_msg, 0, i_addr >> i_hioInfo->iv_blockShift);
        put32(hio_msg, 2, l_end - l_start);
    }
    else
    {
        put16(hio_msg, 0, (i_addr - i_hioInfo->iv_curWindowOffset)
                                >> i_hioInfo->iv_blockShift);
        put16(hio_msg, 2, (l_end - l_start) >> i_hioInfo->iv_blockShift);
    }

    ipmi_msg.iv_hio_msg_len = 6;
    return send_ipmi_msg(&ipmi_msg);
}


//Internal
errorHndl_t writeFlush(void)
{
    ipmi_msg_t ipmi_msg;
    hio_msg_t * hio_msg = &(ipmi_msg.iv_hio_msg);
    init_ipmi_msg(&ipmi_msg);
    hio_msg->iv_cmd = HIOMAP_C_WRITE_FLUSH;
    ipmi_msg.iv_hio_msg_len = 2;

    put16(hio_msg, 0, 0);
    put32(hio_msg, 2, 0);

    return send_ipmi_msg(&ipmi_msg);
}

// External
errorHndl_t ipmiWriteFlash(hioPnorInfo_t * io_hioInfo,
                       uint32_t i_addr,
                       uint32_t i_size,
                       void* i_data)
{
    errorHndl_t l_err = NO_ERROR;
    do
    {
        // Ensure we are operating on a 4-byte boundary
        if (i_size % 4 != 0)
        {
            TRACFCOMP(ERR_MRK"writeFlash: not on 4-byte boundary");
            return FAIL;
        }

        errorHndl_t l_flushErr = NO_ERROR;

        while (i_size)
        {
            uint32_t l_lpcAddr;
            uint32_t l_chunkLen;


            if (PAGE_PROGRAM_BYTES > i_size)
            {
                l_err = adjustWindow(io_hioInfo,
                                     true,
                                     i_addr,
                                     PAGE_PROGRAM_BYTES,
                                     &l_lpcAddr,
                                     &l_chunkLen);
            }
            else
            {
                 l_err = adjustWindow(io_hioInfo,
                                     true,
                                     i_addr,
                                     i_size,
                                     &l_lpcAddr,
                                     &l_chunkLen);
            }

            if (l_err)
            {
                TRACFCOMP(ERR_MRK"Error adjusting MBOX Window for addr: "
                         "0x%x size: 0x%x", l_lpcAddr, i_size);
                break;
            }

            //LPC writes are done via LPC scom interface and can only handle 4 
            //bytes at a time. We write 256 bytes from the previous functions,
            // so break up the large write into 4 byte writes
            uint32_t l_size_written = 0;
            uint32_t l_lpc_write_size = 4; //in bytes
            uint8_t *l_lpc_write_data = i_data;

            while ( (i_size) && (l_size_written < i_size) &&
                     (l_size_written < l_chunkLen))
            {
                if (i_size - l_size_written < l_lpc_write_size)
                {
                    l_lpc_write_size = i_size - l_size_written;
                }

                //Directly do LPC access to space pointed to by BMC
                l_err = lpc_write(LPC_TRANS_FW,
                                  l_lpcAddr,
                                  l_lpc_write_data,
                                  l_lpc_write_size);

                if (l_err)
                {
                    break;
                }

                l_lpc_write_data += l_lpc_write_size;
                l_size_written += l_lpc_write_size;
                l_lpcAddr += l_lpc_write_size;
            }

            //Tell BMC to push data from LPC space into PNOR
            l_err = writeDirty(io_hioInfo, i_addr, l_chunkLen);

            if (l_err)
            {
                break;
            }

            i_addr += l_chunkLen;
            i_size -= l_chunkLen;
            i_data = (char*)i_data + l_chunkLen;
        }

        /* We flush whether we had an error or not.
         */
        l_flushErr = writeFlush();

        if ( l_err == NO_ERROR && l_flushErr )
        {
            l_err = l_flushErr;
        }

        if( l_err )
        {
            i_size = 0;
            break;
        }
    } while(0);

    return l_err;
}

