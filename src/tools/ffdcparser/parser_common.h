/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/tools/ffdcparser/parser_common.h $                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2018,2023                        */
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

// Constants
#define MAX_NUM_CORES 32
#define MAX_NUM_OCMBS 16
#define MAX_NUM_MEM_INT_PTS 12



// Thread struct
typedef struct __attribute__((packed,aligned(4))) thread_dump
{
    uint8_t     len;
    uint8_t     pri;
    uint8_t     state;
    uint8_t     flags;
    uint32_t    timer;
    uint32_t    sem;
    uint32_t    srr0;
    uint32_t    srr1;
    uint32_t    srr2;
    uint32_t    srr3;
    uint32_t    lr;
    uint32_t    stack_trace[8];
} thread_dump_t;

// FFDC struct
typedef struct __attribute__((packed,aligned(4))) ffdc
{
    uint8_t     seq;        // Sequence Number (0x00 for FFDC)
    uint8_t     cmd;        // Command (0x00 for FFDC)
    uint8_t     excp;       // Exception Code
    uint16_t    len;        // FFDC data length
    uint8_t     version;    // (0x00 for FFDC, 0x01 FFDC + BCE registers)
    uint16_t    ckpt;       // Checkpoint (usually 0x0F00 for FFDC)
    uint32_t    ssx_panic;  // SSX Panic Code
    uint32_t    panic_addr; // Address of panic instruction
    uint32_t    lr;         // Link Register
    uint32_t    msr;        // Machine Status Register
    uint32_t    cr;         // Condition Register
    uint32_t    ctr;        // Count Register
    uint32_t    gpr[32];    // GPR0 - GPR31
    uint32_t    evpr;       // Exception Vector Prefix Register
    uint32_t    xer;        // Fixedpoint Exception Register
    uint32_t    esr;        // Exception Syndrome Register
    uint32_t    dear;       // Bad Address
    uint32_t    srr0;       // Return Address for Non-Crit Interrupts
    uint32_t    srr1;       // MSR at time of non-crit interrupt
    uint32_t    srr2;       // Return Address for Crit Interrupts
    uint32_t    srr3;       // MSR at time of crit interrupt
    uint32_t    mcsr;       // Machine Check Syndrome Register
    uint32_t    pid;        // Process ID Register
    uint32_t    zpr;        // Zone Protection Register
    uint32_t    usprg0;     // User SPR General Purpose Register
    uint32_t    sprg[8];    // SPRG0 - SPRG7
    uint32_t    tcr;        // Timer Control Register
    uint32_t    tsr;        // Timer Status Register
    uint32_t    dbcr0;      // Debug Control Register0
    uint32_t    dbcr1;      // Debug Control Register1
    uint32_t    dbsr;       // Debug Status Register
    uint32_t    ocb_oisr0;
    uint32_t    ocb_oisr1;
    uint32_t    ocb_occmisc;
    uint32_t    ocb_ohtmcr;
    uint32_t    ocb_oimr0;
    uint32_t    ocb_oimr1;
    uint32_t    ocb_oitr0;
    uint32_t    ocb_oitr1;
    uint32_t    ocb_oiepr0;
    uint32_t    ocb_oiepr1;
    uint32_t    ocb_oehdr;
    uint32_t    ocb_ocicfg;
    uint32_t    ocb_onisr0;
    uint32_t    ocb_onisr1;
    uint32_t    ocb_ocisr0;
    uint32_t    ocb_ocisr1;
    uint32_t    ocb_occflg;
    uint32_t    ocb_occhbr;
    uint32_t    pba_base;
    uint32_t    pba_mode;
    uint32_t    pba_slvctl0;
    uint32_t    pba_slvctl1;
    uint32_t    pba_slvctl2;
    uint32_t    pba_slvctl3;
    uint32_t    pba_bcde_ctl;
    uint32_t    pba_bcde_set;
    uint32_t    pba_bcde_stat;
    uint32_t    pba_bcde_dr;
    uint32_t    pba_bcde_ocibar;
    uint32_t    pba_bcue_ctl;
    uint32_t    pba_bcue_set;
    uint32_t    pba_bcue_stat;
    uint32_t    pba_bcue_dr;
    uint32_t    pba_bcue_ocibar;
    uint32_t    pba_occ_action;
    uint32_t    pba_bar0;
    uint32_t    pba_bar1;
    uint32_t    pba_bar2;
    uint32_t    pba_bar3;
    uint32_t    pba_barmsk0;
    uint32_t    pba_barmsk1;
    uint32_t    pba_barmsk2;
    uint32_t    pba_barmsk3;
    uint32_t    pba_errrpt0;
    uint32_t    pba_errrpt1;
    uint32_t    pba_errrpt2;
    uint32_t    pba_rbufval0;
    uint32_t    pba_rbufval1;
    uint32_t    pba_rbufval2;
    uint32_t    pba_rbufval3;
    uint32_t    pba_rbufval4;
    uint32_t    pba_rbufval5;
    uint32_t    pba_wbufval0;
    uint32_t    pba_wbufval1;
    uint32_t    pba_slvrst;
    uint32_t    ssx_timebase;
    char        buildname[16];
    uint64_t    occlfir;
    uint64_t    pbafir;
    uint32_t    cores_deconf;
    thread_dump_t   main;
    thread_dump_t   cmdh;
    thread_dump_t   dcom;
    uint32_t        stack_trace[8];
    uint32_t        eye_catcher;
} ffdc_t;

uint64_t get_uint64(FILE* i_fhndl)
{
    int      i = 0;
    uint64_t ret = 0;
    uint8_t  byte = 0;

    for(i = 7; i >= 0; i--)
    {
        byte = fgetc(i_fhndl);
        if(EOF != byte)
        {
            ret |= ((uint64_t)byte << (i*8));
        }
    }

    return ret;
}

uint32_t get_uint32(FILE* i_fhndl)
{
    int      i = 0;
    uint32_t ret = 0;
    uint8_t  byte = 0;

    for(i = 3; i >= 0; i--)
    {
        byte = fgetc(i_fhndl);
        if(EOF != byte)
        {
            ret |= (byte << (i*8));
        }
    }

    return ret;
}

uint16_t get_uint16(FILE* i_fhndl)
{
    int      i = 0;
    uint16_t ret = 0;
    uint8_t  byte = 0;

    for(i = 1; i >= 0; i--)
    {
        byte = fgetc(i_fhndl);
        if(EOF != byte)
        {
            ret |= (byte << (i*8));
        }
    }

    return ret;
}
