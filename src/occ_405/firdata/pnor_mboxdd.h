/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/firdata/pnor_mboxdd.h $                           */
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
#ifndef __PNOR_MBOX_H
#define __PNOR_MBOX_H

//NOTE: Protocol Definition is here:
//   https://github.com/openbmc/mboxbridge/blob/master/Documentation/mbox_protocol.md

#include <ast_mboxdd.h>

/** @file pnor_mbox.h
 *  @brief Provides the interfaces to the PNOR via the
 *         MBOX protocol
 */

typedef struct
{
    astMbox_t iv_mbox;
    uint32_t iv_protocolVersion;
    //Block size is either 4k (V1) or BMC defined (V2)
    // the iv_blockShift parm is a representation of that size
    // as a power of 2. Most command and response args are specified
    // in some multiple of block size
    uint32_t iv_blockShift;
    uint32_t iv_flashSize;
    uint32_t iv_flashEraseSize;
    // Current Window
    bool     iv_curWindowOpen;      // Currently open
    bool     iv_curWindowWrite;     // Write vs Read window
    uint32_t iv_curWindowOffset;    // Offset into flash
    uint32_t iv_curWindowSize;      // Size
    uint32_t iv_curWindowLpcOffset; // Offset into LPC FW space
    // Legacy v1 protocol
    uint32_t iv_readWindowSize;
    uint32_t iv_writeWindowSize;

} pnorMbox_t;

/*
 * @brief Do base initialization of the MBOX functionality
 *
 * @parm[io] io_pnorMbox - Pointer to pnorMbox_t structure
 *
 * @return Error from operation
 */
errorHndl_t hwInit(pnorMbox_t* i_pnorMbox);

/*
 * @brief Read data from the PNOR flash
 *
 * @param[in] Pointer to pnorMbox struct
 * @parm[in] i_addr  PNOR flash Address to read
 * @parm[in] i_size  Amount of data to read, in bytes.
 * @parm[out] o_data  Buffer to read data into
 *
 * @return Error from operation
 */
errorHndl_t readFlash(pnorMbox_t* i_pnorMbox,
                      uint32_t i_addr,
                      size_t i_size,
                      void* o_data);

/**
 * @brief Write data to the PNOR flash
 * @param[in] Pointer to pnorMbox struct
 * @parm i_addr  PNOR flash Address to write
 * @parm i_size  Amount of data to write, in bytes.
 * @parm i_data  Buffer containing data to write
 *
 * @return Error from operation
 */
errorHndl_t writeFlash(pnorMbox_t* i_pnorMbox,
                       uint32_t i_addr,
                       size_t i_size,
                       void* i_data);

/**
 * @brief Open a window on the BMC for PNOR accesses
 *        if necessary and return adjusted LPC address and chunk size
 * @param[in] Pointer to pnorMbbox struct
 * @parm[in]  i_isWrite  Write or read window
 * @parm[in]  i_reqAddr  Requested flash offset
 * @parm[in]  i_reqSize  Requested size
 * @parm[out] o_lpcAddr  LPC offset for the requested offset
 * @parm[out] o_chunkLen i_reqSize adjusted to fit in the window
 *
 * @return Error from operation
 */
errorHndl_t adjustMboxWindow(pnorMbox_t* i_pnorMbox,
                            bool i_isWrite,
                            uint32_t i_reqAddr,
                            size_t i_reqSize,
                            uint32_t *o_lpcAddr,
                            size_t *o_chunkLen);

/**
 * @brief Mark a range dirty in a write window
 * @param[in] Pointer to pnorMbox struct
 * @parm[in] i_addr      Flash offset of the range
 * @parm[in] i_size      Size of the range
 *
 * @return Error from operation
 */
errorHndl_t writeDirty(pnorMbox_t* i_pnorMbox, uint32_t i_addr, size_t i_size);

/**
* @brief Flush all pending dirty data to the flash
* @param[in] Pointer to pnorMbox struct
* @return Error from operation
*/
errorHndl_t writeFlush(pnorMbox_t* i_pnorMbox);

#endif /* __PNOR_MBOX_H */
