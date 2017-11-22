/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/tools/ffdcparser/ffdcparser.c $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2017                        */
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
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>

// NOTE: This tool is to be used when FFDC is dumped by the OCC, and currently
//       only accepts input files in binary format.

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
    uint32_t stack_trace[8];
} thread_dump_t;

// FFDC struct
typedef struct __attribute__((packed,aligned(4))) ffdc
{
    uint8_t     seq;        // Sequence Number (0x00 for FFDC)
    uint8_t     cmd;        // Command (0x00 for FFDC)
    uint8_t     excp;       // Exception Code
    uint16_t    len;        // FFDC data length
    uint8_t     reserved;   // (0x00 for FFDC)
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
            ret |= (byte << (i*8));
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

void get_thread_data(FILE* i_fhndl, thread_dump_t * i_thrd)
{
    uint32_t i = 0;
    i_thrd->len = fgetc(i_fhndl);
    i_thrd->pri = fgetc(i_fhndl);
    i_thrd->state = fgetc(i_fhndl);
    i_thrd->flags = fgetc(i_fhndl);
    i_thrd->timer = get_uint32(i_fhndl);
    i_thrd->sem = get_uint32(i_fhndl);
    i_thrd->srr0 = get_uint32(i_fhndl);
    i_thrd->srr1 = get_uint32(i_fhndl);
    i_thrd->srr2 = get_uint32(i_fhndl);
    i_thrd->srr3 = get_uint32(i_fhndl);
    i_thrd->lr = get_uint32(i_fhndl);
    for(i = 0; i < 8; i++)
        i_thrd->stack_trace[i] = get_uint32(i_fhndl);
}

void print_thread_data(thread_dump_t * i_thrd, char* i_name)
{
    uint32_t i = 0;

    printf("%s Thread Dump\n", i_name);
    printf("\tPriority: 0x%02X\n", i_thrd->pri);
    printf("\tState: 0x%02X\n", i_thrd->state);
    printf("\tFlags: 0x%02X\n", i_thrd->flags);
    printf("\tTimer: 0x%08X\n", i_thrd->timer);
    printf("\tSemaphore: 0x%08X\n", i_thrd->sem);
    printf("\tSRR0: 0x%08X\n", i_thrd->srr0);
    printf("\tSRR1: 0x%08X\n", i_thrd->srr1);
    printf("\tSRR2: 0x%08X\n", i_thrd->srr2);
    printf("\tSRR3: 0x%08X\n", i_thrd->srr3);
    printf("\tStack Trace\n");
    for(i = 0; i < 8; i++)
        printf("\t\t%d:    0x%08X\n", i+1, i_thrd->stack_trace[i]);
}

void dump_ffdc(ffdc_t * data)
{
    uint32_t i = 0;

    printf("Exception Code: 0x%02X\n", data->excp);
    printf("Checkpoint: 0x%04X\n", data->ckpt);
    printf("SSX Panic Code: 0x%08X\n", data->ssx_panic);
    printf("Panic Address: 0x%08X\n", data->panic_addr);
    printf("LR: 0x%08X\n", data->lr);
    printf("MSR: 0x%08X\n", data->msr);
    printf("CR: 0x%08X\n", data->cr);
    printf("CTR: 0x%08X\n", data->ctr);
    for(i = 0; i < 32; i++)
        printf("GPR%02d: 0x%08X\n", i, data->gpr[i]);
    printf("EVPR: 0x%08X\n", data->evpr);
    printf("XER: 0x%08X\n", data->xer);
    printf("ESR: 0x%08X\n", data->esr);
    printf("DEAR: 0x%08X\n", data->dear);
    printf("SRR0: 0x%08X\n", data->srr0);
    printf("SRR1: 0x%08X\n", data->srr1);
    printf("SRR2: 0x%08X\n", data->srr2);
    printf("SRR3: 0x%08X\n", data->srr3);
    printf("MCSR: 0x%08X\n", data->mcsr);
    printf("PID:: 0x%08X\n", data->pid);
    printf("ZPR: 0x%08X\n", data->zpr);
    printf("USPRG0: 0x%08X\n", data->usprg0);
    for(i = 0; i < 8; i++)
        printf("SPRG%d: 0x%08X\n", i, data->sprg[i]);
    printf("TCR: 0x%08X\n", data->tcr);
    printf("TSR: 0x%08X\n", data->tsr);
    printf("DBCR0: 0x%08X\n", data->dbcr0);
    printf("DBCR1: 0x%08X\n", data->dbcr1);
    printf("DBSR: 0x%08X\n", data->dbsr);
    printf("OCB_OISR0: 0x%08X\n", data->ocb_oisr0);
    printf("OCB_OISR1: 0x%08X\n", data->ocb_oisr1);
    printf("OCB_OCCMISC: 0x%08X\n", data->ocb_occmisc);
    printf("OCB_OHTMCR: 0x%08X\n", data->ocb_ohtmcr);
    printf("OCB_OIMR0: 0x%08X\n", data->ocb_oimr0);
    printf("OCB_OIMR1: 0x%08X\n", data->ocb_oimr1);
    printf("OCB_OITR0: 0x%08X\n", data->ocb_oitr0);
    printf("OCB_OITR1: 0x%08X\n", data->ocb_oitr1);
    printf("OCB_OIEPR0: 0x%08X\n", data->ocb_oiepr0);
    printf("OCB_OIEPR1: 0x%08X\n", data->ocb_oiepr1);
    printf("OCB_OEHDR: 0x%08X\n", data->ocb_oehdr);
    printf("OCB_OCICFG: 0x%08X\n", data->ocb_ocicfg);
    printf("OCB_ONISR0: 0x%08X\n", data->ocb_onisr0);
    printf("OCB_ONISR1: 0x%08X\n", data->ocb_onisr1);
    printf("OCB_OCISR0: 0x%08X\n", data->ocb_ocisr0);
    printf("OCB_OCISR1: 0x%08X\n", data->ocb_ocisr1);
    printf("OCB_OCCFLG: 0x%08X\n", data->ocb_occflg);
    printf("OCB_OCCHBR: 0x%08X\n", data->ocb_occhbr);
    printf("SSX Timebase: 0x%08X\n", data->ssx_timebase);
    printf("OCC Buildname: %s\n", data->buildname);
    printf("OCC LFIR: 0x%016X\n", data->occlfir);
    printf("PBA FIR: 0x%016X\n", data->pbafir);
    printf("Cores Deconfigured: 0x%08X\n", data->cores_deconf);
    print_thread_data(&data->main, "MAIN");
    print_thread_data(&data->cmdh, "CMDH");
    print_thread_data(&data->dcom, "DCOM");
    printf("Stack Trace:\n");
    for(i = 0; i < 8; i++)
        printf("\t%d:   0x%08X\n", i+1, data->stack_trace[i]);
}

int main(int argc, char** argv)
{
    FILE*       ffdc_file = NULL;
    ffdc_t      data = {0};
    uint32_t    i = 0;

    // Verify a file was passed as an argument
    if(argc < 2)
    {
        fprintf(stderr, "ERROR: Requires a file with the binary FFDC data\n");
        return -1;
    }
    else
    {
        ffdc_file = fopen(argv[1], "rb");
        if(ffdc_file == NULL)
        {
            fprintf(stderr, "ERROR: %s cannot be opened or does not exist\n", argv[1]);
            return -1;
        }
    }

    // Get file size
    fseek(ffdc_file, 0, SEEK_END);
    const unsigned int file_size = ftell(ffdc_file);
    fseek(ffdc_file, 0, SEEK_SET);

    // Binary file is open, parse it
    data.seq = fgetc(ffdc_file);
    data.cmd = fgetc(ffdc_file);
    data.excp = fgetc(ffdc_file);
    data.len = get_uint16(ffdc_file);

    if (file_size < data.len)
    {
        fprintf(stderr, "WARNING: FFDC file size (%d) is less than what was expected (%d)\n",
                file_size, data.len);
        // fgetc will continue to return 0xFF once the end of file is reached
    }

    if(fseek(ffdc_file, 5, SEEK_SET))
    {
        fprintf(stderr, "ERROR: Something happened when changing offsets in ffdc file\n");
        return -1;
    }

    data.reserved = fgetc(ffdc_file);
    data.ckpt = get_uint16(ffdc_file);
    data.ssx_panic = get_uint32(ffdc_file);
    data.panic_addr = get_uint32(ffdc_file);
    data.lr = get_uint32(ffdc_file);
    data.msr = get_uint32(ffdc_file);
    data.cr = get_uint32(ffdc_file);
    data.ctr = get_uint32(ffdc_file);
    for(i = 0; i < 32; i++)
        data.gpr[i] = get_uint32(ffdc_file);
    data.evpr = get_uint32(ffdc_file);
    data.xer = get_uint32(ffdc_file);
    data.esr = get_uint32(ffdc_file);
    data.dear = get_uint32(ffdc_file);
    data.srr0 = get_uint32(ffdc_file);
    data.srr1 = get_uint32(ffdc_file);
    data.srr2 = get_uint32(ffdc_file);
    data.srr3 = get_uint32(ffdc_file);
    data.mcsr = get_uint32(ffdc_file);
    data.pid = get_uint32(ffdc_file);
    data.zpr = get_uint32(ffdc_file);
    data.usprg0 = get_uint32(ffdc_file);
    for(i = 0; i < 8; i++)
        data.sprg[i] = get_uint32(ffdc_file);
    data.tcr = get_uint32(ffdc_file);
    data.tsr = get_uint32(ffdc_file);
    data.dbcr0 = get_uint32(ffdc_file);
    data.dbcr1 = get_uint32(ffdc_file);
    data.dbsr = get_uint32(ffdc_file);
    data.ocb_oisr0 = get_uint32(ffdc_file);
    data.ocb_oisr1 = get_uint32(ffdc_file);
    data.ocb_occmisc = get_uint32(ffdc_file);
    data.ocb_ohtmcr = get_uint32(ffdc_file);
    data.ocb_oimr0 = get_uint32(ffdc_file);
    data.ocb_oimr1 = get_uint32(ffdc_file);
    data.ocb_oitr0 = get_uint32(ffdc_file);
    data.ocb_oitr1 = get_uint32(ffdc_file);
    data.ocb_oiepr0 = get_uint32(ffdc_file);
    data.ocb_oiepr1 = get_uint32(ffdc_file);
    data.ocb_oehdr = get_uint32(ffdc_file);
    data.ocb_ocicfg = get_uint32(ffdc_file);
    data.ocb_onisr0 = get_uint32(ffdc_file);
    data.ocb_onisr1 = get_uint32(ffdc_file);
    data.ocb_ocisr0 = get_uint32(ffdc_file);
    data.ocb_ocisr1 = get_uint32(ffdc_file);
    data.ocb_occflg = get_uint32(ffdc_file);
    data.ocb_occhbr = get_uint32(ffdc_file);
    data.ssx_timebase = get_uint32(ffdc_file);
    fgets(data.buildname, 16, ffdc_file);
    fgetc(ffdc_file);
    data.occlfir = get_uint64(ffdc_file);
    data.pbafir = get_uint64(ffdc_file);
    data.cores_deconf = get_uint32(ffdc_file);
    get_thread_data(ffdc_file, &data.main);
    get_thread_data(ffdc_file, &data.cmdh);
    get_thread_data(ffdc_file, &data.dcom);
    for(i=0; i<8; i++)
        data.stack_trace[i] = get_uint32(ffdc_file);
    data.eye_catcher = get_uint32(ffdc_file);
    dump_ffdc(&data);
    if(data.eye_catcher != 0xFFDCFFDC)
        printf("WARNING: Eye catcher(0x%08X) was not 0xFFDCFFDC\n", data.eye_catcher);

    if(ffdc_file != NULL)
        fclose(ffdc_file);

    return 0;
}
