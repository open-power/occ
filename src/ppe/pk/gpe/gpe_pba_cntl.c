/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/pk/gpe/gpe_pba_cntl.c $                               */
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

// PBA slave control for PK
// PBA slave 0,1,2,3 (disable 3 for now)
//  Write TTypes:
//      PBA_WRITE_TTYPE_DMA_PR_WR  (DMA Partial Write) (default if don't care)
//      PBA_WRITE_TTYPE_LCO_M       (L3 LCO for IPL, Tsize denotes chiplet)
//      PBA_WRITE_TTYPE_ATOMIC_RMW  (Atomic)
//      PBA_WRITE_TTYPE_CACHE_INJECT(Cache inject after IPL)
//      PBA_WRITE_TTYPE_CI_PR_W     (Cache-inhibited partial write)
//
//  Read TTypes
//      PBA_READ_TTYPE_CL_RD_NC  (Cache line read) (default if don't care)
//      PBA_READ_TTYPE_CI_PR_R   (Cache-inhibitited parital read)
//
// @see ssx/occhw/occhw_pba.h
//
// See 6.10.3.4 of the P9_Power_Managemente_Spec.
// 1. Stop write requests on Master(s),
//     (Read requests can continue if not changing read ttype)
// 2. Write PBASLVRST(SLV Reset) on the desired slave
// 3. Poll for PBASLVRST(Busy SLV Status) = 0
// 4. Change PBASLVCTL
// 5. Resume requests from Master(s)
//

#include "pba_register_addresses.h"
#include "pba_firmware_registers.h"
#include "gpe_pba_cntl.h"

#ifndef __PPE42__
#define PPE_LVD(_m_address, _m_data) \
    asm volatile \
    ( \
      "lvd %[data], 0(%[address]) \n" \
      :  [data]"=r"(_m_data) \
      :  [address]"b"(_m_address) \
    );


// PPE Store Virtual Double operation
#define PPE_STVD(_m_address, _m_data) \
    asm volatile \
    ( \
      "stvd %[data], 0(%[address]) \n" \
      : [data]"=&r"(_m_data) \
      : "[data]"(_m_data), \
      [address]"b"(_m_address) \
      : "memory" \
    );
#endif

// See gpe_pba_cntl.h for contract
void gpe_pba_reset()
{
    uint32_t slave = PBASLVCTLN;
#ifdef __PPE42__
    volatile uint64_t* pbaSlvrst = (uint64_t*)(PBA_SLVRST);
#else
    uint64_t value64;
#endif

    uint64_t val = 0;
    pba_slvrst_t slvrst;
    slvrst.value = 0;
    slvrst.fields.set = PBA_SLVRST_SET(slave);
    pba_slvrst_t rst_in_progress;
    rst_in_progress.value = 0;
    rst_in_progress.fields.in_prog = PBA_SLVRST_IN_PROG(slave);

    do
    {
#ifdef __PPE42__
        *pbaSlvrst = slvrst.value;
#else
        value64 = slvrst.value;
        PPE_STVD(PBA_SLVRST, value64);
#endif


        val = rst_in_progress.value;
#ifdef __PPE42__
        val &= *pbaSlvrst;
#else
        PPE_LVD(PBA_SLVRST, value64);
        val &= value64;
#endif
    }
    while(val != 0);
}


// See gpe_pba_cntl.h for contract.
void gpe_pba_slave_setup(uint32_t i_gpeInstanceId,
                         uint32_t i_write_ttype,
                         uint32_t i_write_tsize,
                         uint32_t i_read_ttype,
                         uint32_t i_buf_alloc)
{
    uint32_t slave = PBASLVCTLN;
    // Only SLVCTL if it need to change
#ifdef __PPE42__
    volatile uint64_t* slvctl_reg =
        (uint64_t*)((uint32_t)(PBA_SLVCTLN(slave)));
#endif

    pba_slvctln_t slvctln;

    slvctln.value = 0;
    slvctln.fields.enable = 1;
    slvctln.fields.mid_match_value = i_gpeInstanceId;
    slvctln.fields.mid_care_mask = 0x7;
    slvctln.fields.write_ttype = i_write_ttype;
    slvctln.fields.read_ttype = i_read_ttype;
    // read  prefetch - default 0, does it need param?
    slvctln.fields.read_prefetch_ctl = PBA_READ_PREFETCH_NONE;
    // slvctln.fields.buf_invalidate_ctl - leave 0
    slvctln.fields.buf_alloc_w = i_buf_alloc & PBA_BUF_W;
    slvctln.fields.buf_alloc_a = i_buf_alloc & PBA_BUF_A;
    slvctln.fields.buf_alloc_b = i_buf_alloc & PBA_BUF_B;
    slvctln.fields.buf_alloc_c = i_buf_alloc & PBA_BUF_C;
    slvctln.fields.dis_write_gather = 1;
    slvctln.fields.wr_gather_timeout = PBA_WRITE_GATHER_TIMEOUT_2_PULSES;
    slvctln.fields.write_tsize = i_write_tsize;
    // slvctln.fields.extaddr = 0; PowerBus address bits (23:36)

#ifdef __PPE42__
    uint64_t current = *slvctl_reg;
#else
    uint64_t current;
    PPE_LVD(PBA_SLVCTLN(slave), current);
#endif

    if(slvctln.value != current)
    {
        gpe_pba_reset();
#ifdef __PPE42__
        *slvctl_reg = slvctln.value;
#else
        PPE_STVD(PBA_SLVCTLN(slave), slvctln.value);
#endif
    }
}

