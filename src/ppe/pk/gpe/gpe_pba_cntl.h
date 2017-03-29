/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/gpe/gpe_pba_cntl.h $                               */
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
#ifndef __GPE_PBA_CNTL_H__
#define __GPE_PBA_CNTL_H__

#include "pk.h"
#include "occhw_pba_common.h"

#define PBA_BUF_W 0x000008000
#define PBA_BUF_A 0x000004000
#define PBA_BUF_B 0x000002000
#define PBA_BUF_C 0x000001000

/**
 * Reset PBA slave
 * @note global PBASLVCTLN selects with slave (0-3) default is 0
 *       PBASLVCTLN can be set as a compile-time env var.
 * @return NONE
 */
void gpe_pba_reset();

/**
 * Setup PBA slave
 *
 * @note global PBASLVCTLN selects which slave (0-3) default is 0
 * @param[in] i_gpeInstanceId OCCHW_INST_ID_GPE0 to OCCHW_INST_ID_GPE3
 * @param[in] i_write_ttype One of:
 *                  PBA_WRITE_TTYPE_DMA_PR_WR
 *                  PBA_WRITE_TTYPE_ATOMIC_RMW
 *
 * @param[in] i_write_tsize.
 *          If ttype is PBA_WRITE_TTYPE_DMA_PR_WR then
 *              tsize is chiplet ID of L3 Cache. Set to PBA_WRITE_TSIZE_DC(0)
 *          If ttype is PBA_WRITE_TTYPE_ATOMIC_RMW then
 *              tsize must be one of:
 *                  PBA_WRITE_TSIZE_ARMW_ADD
 *                  PBA_WRITE_TSIZE_ARMW_AND
 *                  PBA_WRITE_TSIZE_ARMW_OR
 *                  PBA_WRITE_TSIZE_ARMW_XOR
 *
 * @param[in] i_read_ttype One of:
 *                  PBA_READ_TTYPE_CL_RD_NC
 *                  PBA_READ_TTYPE_CI_PR_RD
 *
 * @param[in] i_buf_alloc Buffers to assign Any/ALL of
 *                  [PBA_BUF_W | PBA_BUF_A | PBA_BUF_B | PBA_BUF_C]
 *
 * example:
 *  gpe_pba_slave_setup(OCCHW_INST_ID_GPE0,
 *                      PBA_WRITE_TTYPE_DMA_PR_WR,
 *                      PBA_WRITE_TSIZE_DC,
 *                      PBA_READ_TTYPE_CL_RD_NC,
 *                      PBA_BUF_W | PBA_BUF_A | PBA_BUF_B | PBA_BUF_C
 *                      );
 */
void gpe_pba_slave_setup(uint32_t i_gpeInstanceId,
                         uint32_t i_write_ttype,
                         uint32_t i_write_tsize,
                         uint32_t i_read_ttype,
                         uint32_t i_buf_alloc);


#endif
