/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/tools/ffdcparser/ffdcparser.c $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2019                        */
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
#include "parser_common.h"
#include <string.h> // strcpy
#include <stdlib.h> // getenv
// NOTE: This tool is to be used when FFDC is dumped by the OCC, and currently
//       only accepts input files in binary format.

unsigned int G_stack_parser = 0;
char repo_directory[255] = "";

void lookup_address(uint32_t address)
{
    int rc = -1;
    if (G_stack_parser)
    {
        char command[512];
        sprintf(command, "occ_stack.pl 0x%08X", address);
        //printf("==> %s", command);
        fflush(stdout);
        rc = system(command);
    }
    if (rc != 0)
    {
        printf("0x%08X\n", address);
    }
}

void parse_stack(char * header, uint32_t *stack, unsigned int num)
{
    unsigned int i;

    for(i = 0; i < num; i++)
    {
        if ((G_stack_parser) && (stack[i] != 0))
        {
            char command[512];
            int rc;
            printf("%s%d:   ", header, i+1);
            fflush(stdout);
            sprintf(command, "occ_stack.pl 0x%08X", stack[i]);
            //printf("==> %s", command);
            rc = system(command);
            fflush(stdout);
            //printf("...returned %d\n", rc);
            if (rc != 0)
            {
                printf("0x%08X\n", stack[i]);
            }
        }
        else
        {
            printf("%s%d:   0x%08X\n", header, i+1, stack[i]);
        }
    }
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
    parse_stack("\t\t", i_thrd->stack_trace, 8);
}

void dump_ffdc(ffdc_t * data)
{
    uint32_t i = 0;

    printf("Exception Code: 0x%02X\n", data->excp);
    printf("Checkpoint: 0x%04X\n", data->ckpt);
    printf("SSX Panic Code: 0x%08X\n", data->ssx_panic);
    printf("Panic Address: 0x%08X\n", data->panic_addr);
    printf("LR: ");
    lookup_address(data->lr);
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
    parse_stack("\t", data->stack_trace, 8);
}

int main(int argc, char** argv)
{
    FILE*       ffdc_file = NULL;
    ffdc_t      data = {0};
    uint32_t    i = 0;
    unsigned int rc;

    rc = system("which occ_stack.pl > /dev/null 2>&1");
    if (rc == 0)
    {
        printf("NOTE: Found occ_stack.pl in $PATH\n");
        G_stack_parser = 1;
    }

    // Verify a file was passed as an argument
    char bin_filename[255] = "";

    for (i = 1; i < argc; i++)
    {
        //printf("arg[%d]: %s\n", i, argv[i]);
        if (argv[i][0] == '-')
        {
            if (strcmp(argv[i], "-g") == 0) // OCC Git Repo
            {
                if (i+1 < argc)
                {
                    i += 1;
                    strcpy(repo_directory, argv[i]);
                }
                else
                {
                    printf("ERROR: -g parameter requires OCC repo directory location\n");
                    return -2;
                }
            }
            else
            {
                printf("WARNING: Ignoring unknown option \"%s\"\n", argv[i]);
            }
        }
        else
        {
            if (bin_filename[0] == '\0')
            {
                strcpy(bin_filename, argv[i]);
            }
            else
            {
                printf("WARNING: Ignoring unknown parameter \"%s\"\n", argv[i]);
            }
        }
    }

    if (repo_directory[0] == '\0')
    {
        char *occ_repo = getenv("OCCREPO");
        if (occ_repo != NULL)
        {
            strcpy(repo_directory, occ_repo);
        }
    }

    if (bin_filename[0] == '\0')
    {
        fprintf(stderr, "ERROR: Requires a file with the binary FFDC data\n");
        return -1;
    }

    ffdc_file = fopen(bin_filename, "rb");
    if(ffdc_file == NULL)
    {
        fprintf(stderr, "ERROR: %s cannot be opened or does not exist\n", bin_filename);
        return -1;
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
