/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/tools/ffdcparser/wof_static_data_parser.c $               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2023                        */
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
#include <string.h>
#include "parser_common.h"

#define WOF_STATIC_DATA_SIZE 1792
#define WOF_TABLE_HEADER_SIZE 128
// NOTE: This tool is to be used when WOF Static data is dumped by the OCC, and currently
//       only accepts input files in binary format.


int main(int argc, char** argv)
{
    FILE*       wof_file = NULL;
    uint32_t    i = 0;
    uint32_t    j = 0;
    uint32_t    l_util = 0;
    uint32_t    l_power = 0;
    uint8_t     l_extra_bytes = 0;
    uint8_t     l_num_bytes = 0;

    // Verify a file was passed as an argument
    if(argc < 2)
    {
        fprintf(stderr, "ERROR: Requires a file with the binary WOF data\n");
        return -1;
    }
    else
    {
        wof_file = fopen(argv[1], "rb");
        if(wof_file == NULL)
        {
            fprintf(stderr, "ERROR: %s cannot be opened or does not exist\n", argv[1]);
            return -1;
        }
    }

    // Get file size
    fseek(wof_file, 0, SEEK_END);
    const unsigned int file_size = ftell(wof_file);
    fseek(wof_file, 0, SEEK_SET);

    // Sanity check
    if(file_size < WOF_STATIC_DATA_SIZE)
    {
        fprintf(stderr, "WARNING: WOF static data file size (%d) is less than %d expected\n", file_size, WOF_STATIC_DATA_SIZE);
    }
    else if(file_size > WOF_STATIC_DATA_SIZE)
    {
        fprintf(stderr, "WARNING: WOF static data file size (%d) is more than %d expected\n", file_size, WOF_STATIC_DATA_SIZE);
        l_extra_bytes = file_size - WOF_STATIC_DATA_SIZE;
        fprintf(stderr, "WARNING: Assuming cmd header skipping first %d bytes\n", l_extra_bytes);
    }
    else
    {
        printf("Received Expected Data size %d\n", WOF_STATIC_DATA_SIZE);
    }

    // Print the data to stdout
    // Format should match amec_static_wof_t in /occ/src/occ_405/wof/wof.h
    while(l_extra_bytes)
    {
         printf("EXTRA BYTE: %d\n", fgetc(wof_file));
         l_extra_bytes--;
    }
    // WOF header WOF_TABLE_HEADER_SIZE bytes
    printf("WOF Tables Header:\n");
    printf("     Magic Number: ");
    for(i = 0; i < 4; i++)
        printf("%c", fgetc(wof_file));
    printf("\n");
    l_num_bytes += 4;
    printf("     Major DD Level: 0x%02X\n", fgetc(wof_file));
    l_num_bytes += 1;
    printf("     Minor DD Level: 0x%02X\n", fgetc(wof_file));
    l_num_bytes += 1;
    printf("     WOV Credit Knob: 0x%02X\n", fgetc(wof_file));
    l_num_bytes += 1;
    printf("     Version: 0x%02X\n", fgetc(wof_file));
    l_num_bytes += 1;
    printf("     VRT Block Size: %d\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     VRT Block Header Size: %d\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     VRT Data Size: %d\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Flags: 0x%02X\n", fgetc(wof_file));
    l_num_bytes += 1;
    printf("     Core Count: %d\n", fgetc(wof_file));
    l_num_bytes += 1;
    printf("     VCS Start: %d(0.01%%)\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     VCS Step: %d(0.01%%)\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     VCS Size (number indicies): %d\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     VDD Start: %d(0.01%%)\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     VDD Step: %d(0.01%%)\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     VDD Size (number indicies): %d\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Vratio Start: %d(0.01%%)\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Vratio Step: %d(0.01%%)\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Vratio Size (number indicies): %d\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     IO Start: %d(0.01%%)\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     IO Step: %d(0.01%%)\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     IO Size (number indicies): %d\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Ambient Start: %d\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Ambient Step: %d\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Ambient Size (number indicies): %d\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Sort Throttle Freq: %dMHz\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Socket Power: %dW\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Sort Power aka WOF base Freq: %dMHz\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Regulator Design Point Current: %dA\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Boost Current Capacity: %dA\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     VCS Ceff TDP Index: %d\n", fgetc(wof_file));
    l_num_bytes += 1;
    printf("     VDD Ceff TDP Index: %d\n", fgetc(wof_file));
    l_num_bytes += 1;
    printf("     IO Power TDP Index: %d\n", fgetc(wof_file));
    l_num_bytes += 1;
    printf("     Ambient TDP Index: %d\n", fgetc(wof_file));
    l_num_bytes += 1;
    printf("     IO TDP Wattage: %dW\n", fgetc(wof_file));
    l_num_bytes += 1;
    printf("     IO Min Wattage: %dW\n", fgetc(wof_file));
    l_num_bytes += 1;
    printf("     Sort UT Freq: %dMHz\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Table Timestamp: 0x%08X\n", get_uint32(wof_file));
    l_num_bytes += 4;
    printf("     Override Freq: %dMHz\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Override Power: %dW\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Table Version: ");
    for(i = 0; i < 16; i++)
        printf("%c", fgetc(wof_file));
    printf("\n");
    l_num_bytes += 16;
    printf("     Package Name: ");
    for(i = 0; i < 16; i++)
        printf("%c", fgetc(wof_file));
    printf("\n");
    l_num_bytes += 16;
    printf("     Sort Power Save Freq: %dMHz\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Sort Fixed Freq: %dMHz\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Bal Perf Ceff Adj: %d%%\n", fgetc(wof_file));
    l_num_bytes += 1;
    printf("     Favor Perf Ceff Adj: %d%%\n", fgetc(wof_file));
    l_num_bytes += 1;
    printf("     Favor Power Ceff Adj: %d%%\n", fgetc(wof_file));
    l_num_bytes += 1;
    printf("     Non Det Ceff Adj: %d%%\n", fgetc(wof_file));
    l_num_bytes += 1;
    printf("     Bal Perf Freq Limit: %dMHz\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Favor Perf Freq Limit: %dMHz\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Favor Power Save Freq Limit: %dMHz\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Non Det Freq Limit: %dMHz\n", get_uint16(wof_file));
    l_num_bytes += 2;
    printf("     Max Power at Min Freq: %dW\n", get_uint16(wof_file));
    l_num_bytes += 2;

    l_extra_bytes = 0;
    if(l_num_bytes < WOF_TABLE_HEADER_SIZE)
    {
        l_extra_bytes = WOF_TABLE_HEADER_SIZE - l_num_bytes;
        printf("     WOF Table Header Padding: 0x");
    }
    while(l_extra_bytes)
    {
         printf("%02X", fgetc(wof_file));
         l_extra_bytes--;
    }
    printf("\n");

    printf("VRT Main Memory Address: 0x%08X\n", get_uint32(wof_file));
    printf("VRT Length in main memory: %d\n", get_uint32(wof_file));
    printf("OCC WOF Values SRAM Address: 0x%08X\n", get_uint32(wof_file));
    printf("PGPE WOF Values SRAM Address: 0x%08X\n", get_uint32(wof_file));
    printf("XGPE WOF Values SRAM Address: 0x%08X\n", get_uint32(wof_file));
    printf("XGPE IDDQ Activity SRAM Address: 0x%08X\n", get_uint32(wof_file));
    printf("Pstate Table SRAM Address: 0x%08X\n", get_uint32(wof_file));
    printf("Vdd Vret p1mv (from OPPB): %d\n", get_uint32(wof_file));
    printf("Voltage Index for Vdd Vret: %d\n", fgetc(wof_file));
    printf("IDDQ Activity Sample Depth: %d\n", fgetc(wof_file));
    printf("IDDQ Activity divide bit shift: %d\n", fgetc(wof_file));
    printf("Altitude Temperature Adjustment in (C/km)*1000: %d\n", get_uint32(wof_file));
    printf("Altitude reference base: %dm\n", get_uint32(wof_file));
    printf("Last ambient condition in WOF tables: %d\n", get_uint32(wof_file));
    for(i = 0; i < MAX_NUM_OCMBS; i++)
    {
        printf("OCMB %d Utilization to Power Interpolation points:\n", i);
        for(j = 0; j < MAX_NUM_MEM_INT_PTS; j++)
	{
            l_util = get_uint32(wof_file);
            l_power = get_uint32(wof_file);
            printf("     Utilization %dc%% Power %dcW\n", l_util, l_power);
	}
    }

    // Close the file
    if(wof_file != NULL)
        fclose(wof_file);

    return 0;
}
