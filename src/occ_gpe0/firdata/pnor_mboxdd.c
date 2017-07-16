/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/firdata/pnor_mboxdd.c $                           */
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

/**
 *  @file ast_mboxdd.c
 *
 *  @brief Implementation of the PNOR access code on top of AST MBOX protocol
 */

/*****************************************************************************/
// I n c l u d e s
/*****************************************************************************/
#include <native.h>
#include <norflash.h>
#include <pnor_mboxdd.h>
#include <lpc.h>

errorHndl_t hwInit(pnorMbox_t* i_pnorMbox)
{
    errorHndl_t l_err = NO_ERROR;
    uint8_t* l_data;
    int i;
    do
    {
        //Current window starts closed
        i_pnorMbox->iv_curWindowOpen = false;
        i_pnorMbox->iv_mbox.iv_mboxMsgSeq = 1;
        l_err = initializeMbox();
        if (l_err)
        {
            TRAC_ERR("initializeMbox failed rc:0x%x", l_err);
            break;
        }

        //Send message to BMC Mbox to get MBOX info
        // This message gets the MBOX protocol version
        mboxMessage_t l_getInfoMsg;
        l_data = (uint8_t*)(&l_getInfoMsg);
        for (i = 0; i < BMC_MBOX_DATA_REGS; i++)
        {
            l_data[i] = 0;
        }

        l_getInfoMsg.iv_cmd = MBOX_C_GET_MBOX_INFO;
        put8(&l_getInfoMsg, 0, 2);
        l_err = doMessage(&i_pnorMbox->iv_mbox, &l_getInfoMsg);
        if (l_err)
        {
            TRAC_ERR("doMessage to ping BMC failed with rc=0x%x", l_err);
            break;
        }

        i_pnorMbox->iv_protocolVersion = get8(&l_getInfoMsg, 0);
        if (i_pnorMbox->iv_protocolVersion == 1)
        {
            i_pnorMbox->iv_blockShift = 12;
            i_pnorMbox->iv_readWindowSize = get16(&l_getInfoMsg, 1)
                                                << i_pnorMbox->iv_blockShift;
            i_pnorMbox->iv_writeWindowSize = get16(&l_getInfoMsg, 3)
                                                << i_pnorMbox->iv_blockShift;
        }
        else
        {
            i_pnorMbox->iv_blockShift = get8(&l_getInfoMsg, 5);
        }

        //Now get the size of the flash
        mboxMessage_t l_getSizeMsg;
        l_getSizeMsg.iv_cmd = MBOX_C_GET_FLASH_INFO;
        l_err = doMessage(&i_pnorMbox->iv_mbox, &l_getSizeMsg);
        if (l_err)
        {
            TRAC_ERR("doMessage failed to get flash size rc=0x%x", l_err);
            break;
        }

        if (i_pnorMbox->iv_protocolVersion == 1)
        {
            i_pnorMbox->iv_flashSize = get32(&l_getSizeMsg, 0);
            i_pnorMbox->iv_flashEraseSize = get32(&l_getSizeMsg, 4);
        }
        else
        {
            i_pnorMbox->iv_flashSize = get16(&l_getSizeMsg, 0)
                                            << i_pnorMbox->iv_blockShift;
            i_pnorMbox->iv_flashEraseSize = get16(&l_getSizeMsg, 2)
                                            << i_pnorMbox->iv_blockShift;
        }
    } while (0);
    return l_err;
}


errorHndl_t readFlash(pnorMbox_t* i_pnorMbox,
                      uint32_t i_addr,
                      uint32_t i_size,
                      void* o_data)
{
    errorHndl_t l_err = NO_ERROR;

    do
    {
        // Ensure we are operating on a 4-byte boundary
        if ((i_size % 4 != 0) && (i_addr % 4 != 0))
        {
            TRAC_ERR("readFlash: not on 4-byte boundary");
            return FAIL;
        }

        while (i_size)
        {
            uint32_t l_lpcAddr;
            uint32_t l_chunkLen;

            l_err = adjustMboxWindow(i_pnorMbox,
                                     false,
                                     i_addr,
                                     i_size,
                                     &l_lpcAddr,
                                     &l_chunkLen);

            if (l_err)
            {
                break;
            }

            //Directly access LPC to do read/write as BMC is setup now
            l_err = lpc_read(LPC_TRANS_FW, l_lpcAddr, o_data, l_chunkLen);

            if (l_err)
            {
                break;
            }

            i_addr += l_chunkLen;
            i_size -= l_chunkLen;
            o_data = (char*)o_data + l_chunkLen;
        }

        if(l_err)
        {
            break;
        }

    }
    while(0);

    return l_err;
}

errorHndl_t writeFlash(pnorMbox_t* i_pnorMbox,
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
            TRAC_ERR("writeFlash: not on 4-byte boundary");
            return FAIL;
        }


        errorHndl_t l_flushErr = NO_ERROR;

        while (i_size)
        {
            uint32_t l_lpcAddr;
            uint32_t l_chunkLen;

            l_err = adjustMboxWindow(i_pnorMbox,
                                     true,
                                     i_addr,
                                     i_size,
                                     &l_lpcAddr,
                                     &l_chunkLen);

            if (l_err)
            {
                break;
            }

            //Directly do LPC access to space pointed to by BMC
            l_err = lpc_write(LPC_TRANS_FW, l_lpcAddr, i_data, l_chunkLen);

            if (l_err)
            {
                break;
            }

            //Tell BMC to push data from LPC space into PNOR
            l_err = writeDirty(i_pnorMbox, i_addr, l_chunkLen);

            if (l_err)
            {
                break;
            }

            i_addr += l_chunkLen;
            i_size -= l_chunkLen;
            i_data = (char*)i_data + l_chunkLen;
        }

        /* We flush whether we had an error or not.
         *
         * NOTE: It would help the daemon a lot if we moved that out of here
         * and instead had a single flush call over a series of writes.
         */
        l_flushErr = writeFlush(i_pnorMbox);

        if ( l_err == NO_ERROR && l_flushErr )
        {
            l_err = l_flushErr;
        }

        if( l_err )
        {
            i_size = 0;
        }

        if(l_err)
        {
            break;
        }

    }
    while(0);

    return l_err;
}

errorHndl_t adjustMboxWindow(pnorMbox_t* i_pnorMbox,
                                    bool i_isWrite, uint32_t i_reqAddr,
                                    uint32_t i_reqSize, uint32_t *o_lpcAddr,
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
        uint32_t l_wEnd = i_pnorMbox->iv_curWindowOffset +
                              i_pnorMbox->iv_curWindowSize;

        /* A read request can be serviced by a write window */
        if (i_pnorMbox->iv_curWindowOpen &&
            (i_pnorMbox->iv_curWindowWrite || !i_isWrite) &&
            i_reqAddr >= i_pnorMbox->iv_curWindowOffset && i_reqAddr < l_wEnd)
        {
            uint32_t l_gap = (l_wEnd - i_reqAddr);

            *o_lpcAddr = i_pnorMbox->iv_curWindowLpcOffset +
                          (i_reqAddr - i_pnorMbox->iv_curWindowOffset);
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
        i_pnorMbox->iv_curWindowOpen = false;

        /*
         * Then open the new one at the right position. The required
         * alignment differs between protocol versions
         */
        if (i_pnorMbox->iv_protocolVersion == 1)
        {
            l_wSize = i_isWrite ? i_pnorMbox->iv_writeWindowSize
                                : i_pnorMbox->iv_readWindowSize;
            l_pos = i_reqAddr & ~(l_wSize - 1);
            l_reqSize = 0;
        }
        else
        {
            uint32_t l_blockMask = (1u << i_pnorMbox->iv_blockShift) - 1;
            l_wSize = 0;
            l_pos = i_reqAddr & ~l_blockMask;
            l_reqSize = (((i_reqAddr + i_reqSize) + l_blockMask) & ~l_blockMask)
                          - l_pos;
        }

        mboxMessage_t winMsg;
        if (i_isWrite)
        {
            winMsg.iv_cmd = MBOX_C_CREATE_WRITE_WINDOW;
        }
        else
        {
            winMsg.iv_cmd = MBOX_C_CREATE_READ_WINDOW;
        }

        put16(&winMsg, 0, l_pos >> i_pnorMbox->iv_blockShift);
        put16(&winMsg, 2, l_reqSize >> i_pnorMbox->iv_blockShift);
        l_err = doMessage(&i_pnorMbox->iv_mbox, &winMsg);

        if (l_err)
        {
            break;
        }

        i_pnorMbox->iv_curWindowLpcOffset =
                        (get16(&winMsg,0)) << i_pnorMbox->iv_blockShift;

        if (i_pnorMbox->iv_protocolVersion == 1)
        {
            i_pnorMbox->iv_curWindowOffset = l_pos;
            i_pnorMbox->iv_curWindowLpcOffset =
                        (get16(&winMsg,0)) << i_pnorMbox->iv_blockShift;
            i_pnorMbox->iv_curWindowSize = l_wSize;
        }
        else
        {
            i_pnorMbox->iv_curWindowLpcOffset = (get16(&winMsg,0))
                             << i_pnorMbox->iv_blockShift;
            i_pnorMbox->iv_curWindowSize = (get16(&winMsg,2))
                             << i_pnorMbox->iv_blockShift;
            i_pnorMbox->iv_curWindowOffset = (get16(&winMsg,4))
                             << i_pnorMbox->iv_blockShift;
        }

        i_pnorMbox->iv_curWindowOpen = true;
        i_pnorMbox->iv_curWindowWrite = i_isWrite;

    }
    while (true);

    return l_err;
}

errorHndl_t writeDirty(pnorMbox_t* i_pnorMbox, uint32_t i_addr, uint32_t i_size)
{
    /* To pass a correct "size" for both protocol versions, we
     * calculate the block-aligned start and end.
     */
    uint32_t l_blockMask = (1u << i_pnorMbox->iv_blockShift) - 1;
    uint32_t l_start     = i_addr & ~l_blockMask;
    uint32_t l_end       = ((i_addr + i_size) + l_blockMask) & ~l_blockMask;

    mboxMessage_t dirtyMsg;
    dirtyMsg.iv_cmd = MBOX_C_MARK_WRITE_DIRTY;

    if (i_pnorMbox->iv_protocolVersion == 1)
    {
        put16(&dirtyMsg, 0, i_addr >> i_pnorMbox->iv_blockShift);
        put32(&dirtyMsg, 2, l_end - l_start);
    }
    else
    {
        put16(&dirtyMsg, 0, (i_addr - i_pnorMbox->iv_curWindowOffset)
                                >> i_pnorMbox->iv_blockShift);
        put16(&dirtyMsg, 2, (l_end - l_start) >> i_pnorMbox->iv_blockShift);
    }

    return doMessage(&i_pnorMbox->iv_mbox, &dirtyMsg);
}

errorHndl_t writeFlush(pnorMbox_t* i_pnorMbox)
{
    mboxMessage_t flushMsg;
    flushMsg.iv_cmd = MBOX_C_WRITE_FLUSH;

    put16(&flushMsg, 0, 0);
    put32(&flushMsg, 2, 0);

    return doMessage(&i_pnorMbox->iv_mbox, &flushMsg);
}
