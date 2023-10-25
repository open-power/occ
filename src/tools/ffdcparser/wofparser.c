/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/tools/ffdcparser/wofparser.c $                            */
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

#define WOF_DATA_SIZE 954
// NOTE: This tool is to be used when WOF Dynamic data is dumped by the OCC, and currently
//       only accepts input files in binary format.


int main(int argc, char** argv)
{
    FILE*       wof_file = NULL;
    uint32_t    i = 0;
    uint8_t     l_extra_bytes = 0;
    uint8_t     l_vrt_header_byte0 = 0;
    uint8_t     l_vrt_header_byte1 = 0;
    uint8_t     l_vrt_header_byte2 = 0;
    uint8_t     l_vrt_header_byte3 = 0;
    uint8_t     l_temp_data = 0;
    int16_t     l_signed16 = 0;
    int8_t      l_signed8 = 0;

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
    if(file_size < WOF_DATA_SIZE)
    {
        fprintf(stderr, "WARNING: WOF data file size (%d) is less than %d expected\n", file_size, WOF_DATA_SIZE);
    }
    else if(file_size > WOF_DATA_SIZE)
    {
        fprintf(stderr, "WARNING: WOF data file size (%d) is more than %d expected\n", file_size, WOF_DATA_SIZE);
        l_extra_bytes = file_size - WOF_DATA_SIZE;
        fprintf(stderr, "WARNING: Assuming cmd header skipping first %d bytes\n", l_extra_bytes);
    }
    else
    {
        printf("Received Expected Data size %d\n", WOF_DATA_SIZE);
    }

    // Print the data to stdout
    // Format should match amec_wof_t in /occ/src/occ_405/wof/wof.h
    while(l_extra_bytes)
    {
         printf("EXTRA BYTE: %d\n", fgetc(wof_file));
         l_extra_bytes--;
    }

    printf("wof_disabled: 0x%08X\n", get_uint32(wof_file));
    printf("vdd_step_from_start: %d\n", get_uint16(wof_file));
    printf("vdd_override_index: %d\n", fgetc(wof_file));
    printf("vcs_step_from_start: %d\n", get_uint16(wof_file));
    printf("vcs_override_index: %d\n", fgetc(wof_file));
    printf("io_pwr_step_from_start: %d\n", get_uint16(wof_file));
    printf("io_pwr_override_index: %d\n", fgetc(wof_file));
    printf("ambient_step_from_start: %d\n", get_uint16(wof_file));
    printf("ambient_override_index: %d\n", fgetc(wof_file));
    printf("v_ratio_override_index: %d\n", fgetc(wof_file));
    printf("curvdd_sensor: %d\n", get_uint16(wof_file));
    printf("curvcs_sensor: %d\n", get_uint16(wof_file));
    printf("Vdd_chip_p1mv: 0x%08X\n", get_uint32(wof_file));
    printf("Vcs_chip_p1mv: 0x%08X\n", get_uint32(wof_file));
    for(i = 0; i < MAX_NUM_CORES; i++)
        printf("tempprocthrmc[%d]: %d\n", i, get_uint16(wof_file));
    printf("T_racetrack: %d\n", get_uint16(wof_file));
    for(i = 0; i < MAX_NUM_CORES; i++)
        printf("p1pct_on[%d]: %d\n", i, get_uint16(wof_file));
    for(i = 0; i < MAX_NUM_CORES; i++)
        printf("p1pct_off[%d]: %d\n", i, get_uint16(wof_file));
    for(i = 0; i < MAX_NUM_CORES; i++)
        printf("p1pct_vmin[%d]: %d\n", i, get_uint16(wof_file));
    for(i = 0; i < MAX_NUM_CORES; i++)
        printf("p1pct_mma_off[%d]: %d\n", i, get_uint16(wof_file));
    printf("iddq_100ua: 0x%08X\n", get_uint32(wof_file));
    printf("icsq_100ua: 0x%08X\n", get_uint32(wof_file));
    printf("iac_vdd_100ua: 0x%08X\n", get_uint32(wof_file));
    printf("iac_vcs_100ua: 0x%08X\n", get_uint32(wof_file));
    printf("iac_tdp_vdd_100ua: 0x%08X\n", get_uint32(wof_file));
    printf("v_ratio_vdd: 0x%04X\n", get_uint16(wof_file));
    printf("f_clip_ps: %d\n", fgetc(wof_file));
    printf("v_ratio_vcs: 0x%04X\n", get_uint16(wof_file));
    printf("ceff_ratio_vdd_numerator: 0x%08X", get_uint32(wof_file));
    printf("%08X\n", get_uint32(wof_file));
    printf("ceff_ratio_vdd_denominator: 0x%08X", get_uint32(wof_file));
    printf("%08X\n", get_uint32(wof_file));
    printf("ceff_ratio_vdd in 0.01 percent (max of adjusted): %d\n", get_uint32(wof_file));
    printf("ceff_ratio_vcs_numerator: 0x%08X", get_uint32(wof_file));
    printf("%08X\n", get_uint32(wof_file));
    printf("ceff_ratio_vcs_denominator: 0x%08X", get_uint32(wof_file));
    printf("%08X\n", get_uint32(wof_file));
    printf("ceff_ratio_vcs in 0.01 percent: %d\n", get_uint32(wof_file));
    printf("Vdd_chip_index: %d\n", fgetc(wof_file));
    l_signed8 = (int8_t)fgetc(wof_file);
    printf("ambient_adj_for_altitude: %d\n", l_signed8);
    printf("altitude: %d\n", get_uint16(wof_file));
    printf("Vcs_chip_index %d\n", fgetc(wof_file));
    printf("scaled_all_off_off_vdd_chip_ua_nc: 0x%08X\n", get_uint32(wof_file));
    printf("scaled_all_off_off_vcs_chip_ua_nc: 0x%08X\n", get_uint32(wof_file));
    printf("racetrack_only_vcs_chip_ua 0x%08X\n", get_uint32(wof_file));
    printf("scaled_all_off_off_vdd_chip_ua_c: 0x%08X\n", get_uint32(wof_file));
    printf("scaled_all_off_off_vdd_vmin_ua_c: 0x%08X\n", get_uint32(wof_file));
    printf("racetrack_only_vdd_chip_ua: 0x%08X\n", get_uint32(wof_file));
    printf("scaled_all_off_on_vdd_chip_ua_c: 0x%08X\n", get_uint32(wof_file));
    printf("scaled_all_off_on_vdd_vmin_ua_c: 0x%08X\n", get_uint32(wof_file));
    printf("single_core_off_vdd_chip_ua_c: 0x%08X\n", get_uint32(wof_file));
    printf("single_cache_off_vdd_chip_ua_c: 0x%08X\n", get_uint32(wof_file));
    printf("single_core_off_vdd_vmin_ua_c: 0x%08X\n", get_uint32(wof_file));
    printf("single_cache_on_vdd_chip_ua_c: 0x%08X\n", get_uint32(wof_file));
    printf("scaled_all_off_off_vdd_vmin_ua_nc: 0x%08X\n", get_uint32(wof_file));
    printf("scaled_all_off_on_vdd_chip_ua_nc: 0x%08X\n", get_uint32(wof_file));
    printf("scaled_all_off_on_vdd_vmin_ua_nc: 0x%08X\n", get_uint32(wof_file));
    printf("single_core_off_vdd_chip_ua_nc: 0x%08X\n", get_uint32(wof_file));
    printf("single_cache_off_vdd_chip_ua_nc: 0x%08X\n", get_uint32(wof_file));
    printf("single_core_off_vdd_vmin_ua_nc: 0x%08X\n", get_uint32(wof_file));
    printf("single_cache_on_vdd_chip_ua_nc: 0x%08X\n", get_uint32(wof_file));
    printf("scaled_all_off_off_vcs_chip_ua_c: 0x%08X\n", get_uint32(wof_file));
    printf("scaled_all_off_on_vcs_chip_ua_c: 0x%08X\n", get_uint32(wof_file));
    printf("single_core_off_vcs_chip_ua_c: 0x%08X\n", get_uint32(wof_file));
    printf("single_cache_off_vcs_chip_ua_c: 0x%08X\n", get_uint32(wof_file));
    printf("single_cache_on_vcs_chip_ua_c: 0x%08X\n", get_uint32(wof_file));
    printf("scaled_all_off_on_vcs_chip_ua_nc: 0x%08X\n", get_uint32(wof_file));
    printf("single_core_off_vcs_chip_ua_nc: 0x%08X\n", get_uint32(wof_file));
    printf("single_cache_off_vcs_chip_ua_nc: 0x%08X\n", get_uint32(wof_file));
    printf("single_cache_on_vcs_chip_ua_nc: 0x%08X\n", get_uint32(wof_file));
    printf("scaled_good_eqs_on_on_vdd_chip_ua: 0x%08X\n", get_uint32(wof_file));
    printf("single_core_on_vdd_chip_eqs_ua: 0x%08X\n", get_uint32(wof_file));
    printf("scaled_good_eqs_on_on_vdd_vmin_ua: 0x%08X\n", get_uint32(wof_file));
    printf("single_core_on_vdd_vmin_eqs_ua: 0x%08X\n", get_uint32(wof_file));
    printf("scaled_good_eqs_on_on_vcs_chip_ua: 0x%08X\n", get_uint32(wof_file));
    printf("single_core_on_vcs_chip_eqs_ua: 0x%08X\n", get_uint32(wof_file));
    printf("curr_ping_pong_buf: 0x%08X\n", get_uint32(wof_file));
    printf("next_ping_pong_buf: 0x%08X\n", get_uint32(wof_file));
    printf("vrt_main_mem_addr: 0x%08X\n", get_uint32(wof_file));
    printf("vrt_bce_table_offset: 0x%08X\n", get_uint32(wof_file));
    printf("wof_init_state: %d\n", fgetc(wof_file));
    printf("gpe_req_rc: 0x%08X\n", get_uint32(wof_file));
    printf("control_ipc_rc: 0x%08X\n", get_uint32(wof_file));
    printf("vrt_callback_error: 0x%02X\n", fgetc(wof_file));
    printf("pgpe_wof_off: %d\n", fgetc(wof_file));
    printf("pgpe_wof_disabled: %d\n", fgetc(wof_file));
    printf("vrt_mm_offset: 0x%08X\n", get_uint32(wof_file));
    printf("wof_vrt_req_rc: %d\n", fgetc(wof_file));
    printf("c_ratio_iac_tdp_vddp1ma: 0x%08X\n", get_uint32(wof_file));
    printf("avg_freq_mhz: 0x%08X\n", get_uint32(wof_file));
    printf("c_ratio_iac_tdp_vcsp1ma: 0x%08X\n", get_uint32(wof_file));
    printf("iac_tdp_vcs_100ua: 0x%08X\n", get_uint32(wof_file));
    printf("vrt_state: %d\n", fgetc(wof_file));
    printf("pgpe_wof_values_dw0:\n");
    printf("     Average Pstate (includes throttle space): %d\n", fgetc(wof_file));
    printf("     Average Frequency Pstate given to DPLL: %d\n", fgetc(wof_file));
    printf("     WOF Clip Pstate (last value read from VRT): %d\n", fgetc(wof_file));
    printf("     Average throttle index (value written to WCOR register): %d\n", fgetc(wof_file));
    printf("     Vratio VCS average: 0x%04X\n", get_uint16(wof_file));
    printf("     Vratio VDD average: 0x%04X\n", get_uint16(wof_file));
    printf("pgpe_wof_values_dw1:\n");
    printf("     VDD current 10mA: %d\n", get_uint16(wof_file));
    printf("     VCS Current 10mA: %d\n", get_uint16(wof_file));
    printf("     VDN Current 10mA: %d\n", get_uint16(wof_file));
    printf("     RDP Limit 10ma for OCS: %d\n", get_uint16(wof_file));
    printf("pgpe_wof_values_dw2:\n");
    printf("     VDD Voltage mV: %d\n", get_uint16(wof_file));
    printf("     VCS Voltage mV: %d\n", get_uint16(wof_file));
    printf("     VDN Voltage mV: %d\n", get_uint16(wof_file));
    printf("     VIO Voltage mV: %d\n", get_uint16(wof_file));
    printf("pgpe_wof_values_dw3:\n");
    printf("     ocs_avg_0p01pct: %d\n", get_uint16(wof_file));
    printf("     vratio_vcs_roundup_avg: 0x%04X\n", get_uint16(wof_file));
    printf("     vratio_vdd_roundup_avg: 0x%04X\n", get_uint16(wof_file));
    printf("     uv_avg_0p1pct: %d\n", fgetc(wof_file));
    printf("     ov_avg_0p1pct: %d\n", fgetc(wof_file));
    printf("ocs_dirty: 0x02%X\n", fgetc(wof_file));
    printf("xgpe_wof_values_dw0:\n");
    printf("     IO Proxy Power 0.01W: %d\n", get_uint16(wof_file));
    printf("     Reserved: %d\n", fgetc(wof_file));
    printf("     I/O index for WOF table I/O dimension: %d\n", fgetc(wof_file));
    printf("     Reserved: 0x%08X\n", get_uint32(wof_file));
    printf("ocs_increase_ceff: %d\n", get_uint16(wof_file));
    printf("ocs_decrease_ceff: %d\n", get_uint16(wof_file));
    printf("Vdd Ceff add for OC only: %d\n", get_uint16(wof_file));
    printf("Adjusted Vdd Ceff for OC only: %d\n", get_uint16(wof_file));
    printf("ocs_not_dirty_count: 0x%08X\n", get_uint32(wof_file));
    printf("ocs_not_dirty_type1_count: 0x%08X\n", get_uint32(wof_file));
    printf("ocs_dirty_type0_count: 0x%08X\n", get_uint32(wof_file));
    printf("ocs_dirty_type1_count: 0x%08X\n", get_uint32(wof_file));
    printf("ambient_condition: 0x%08X\n", get_uint32(wof_file));
    printf("vpd_index1: %d\n", fgetc(wof_file));
    printf("vpd_index2: %d\n", fgetc(wof_file));
    printf("vdd_avg_tdp_100uv: 0x%08X\n", get_uint32(wof_file));
    printf("tdp_idd_rt_ac_100ua: 0x%08X\n", get_uint32(wof_file));
    printf("vcs_avg_tdp_100uv: 0x%08X\n", get_uint32(wof_file));
    printf("vddp1mv_exp1p3: 0x%08X\n", get_uint32(wof_file));
    printf("vcsp1mv_exp1p3: 0x%08X\n", get_uint32(wof_file));
    printf("tdpvcsp1mv_exp1p3: 0x%08X\n", get_uint32(wof_file));
    printf("tdpvddp1mv_exp1p3: 0x%08X\n", get_uint32(wof_file));
    for(i = 0; i < MAX_NUM_CORES; i++)
    {
        printf("core[%d] Activity Counts:\n", i);
        printf("     ACT_CNT_IDX_CORECLK_OFF: %d\n", fgetc(wof_file));
        printf("     ACT_CNT_IDX_CORE_VMIN: %d\n", fgetc(wof_file));
        printf("     ACT_CNT_IDX_MMA_OFF: %d\n", fgetc(wof_file));
        printf("     ACT_CNT_IDX_CORECACHE_OFF: %d\n", fgetc(wof_file));
    }
    printf("interpolate_ambient_vrt? %d\n", fgetc(wof_file));

    // Parse out VRT
    l_vrt_header_byte0 = fgetc(wof_file);
    l_vrt_header_byte1 = fgetc(wof_file);
    l_vrt_header_byte2 = fgetc(wof_file);
    l_vrt_header_byte3 = fgetc(wof_file);
    printf("Current VRT:\n");
    printf("             Header:  0x%02X%02X%02X%02X -->\n", l_vrt_header_byte0,
                                                             l_vrt_header_byte1,
                                                             l_vrt_header_byte2,
                                                             l_vrt_header_byte3);
    if(l_vrt_header_byte0 == 0x56)
        printf("                                    0x%02X (V Marker)\n", l_vrt_header_byte0);
    else
        printf("ERROR: Expected marker V (0x56) read 0x%02X\n", l_vrt_header_byte0);

    if(l_vrt_header_byte1 & 0x80)
        printf("                                    Type 1 (HOMER)\n");
    else
        printf("                                    Type 0 (System)\n");

    if(l_vrt_header_byte1 & 0x40)
        printf("                                    Content Type 1 (Reserved -- CeffRatio Target)\n");
    else
        printf("                                    Content Type 0 (CeffRatio Delta -- Overage)\n");

    l_temp_data = (l_vrt_header_byte1 & 0x30) >> 4;
        printf("                                    Version = %d\n", l_temp_data);

    l_temp_data = (l_vrt_header_byte1 & 0x0F) << 1;
    l_temp_data |= ((l_vrt_header_byte2 & 0x80) >> 7);
        printf("                                    IO Index = %d\n", l_temp_data);

    l_temp_data = (l_vrt_header_byte2 & 0x7C) >> 2;
        printf("                                    Ambient Condition Index = %d\n", l_temp_data);

    l_temp_data = (l_vrt_header_byte2 & 0x03) << 3;
    l_temp_data |= ((l_vrt_header_byte3 & 0xE0) >> 5);
        printf("                                    Vcs Index = %d\n", l_temp_data);

    l_temp_data = l_vrt_header_byte3 & 0x1F;
        printf("                                    Vdd Index = %d\n", l_temp_data);

    printf("             Pstates: 0x%08X\n", get_uint32(wof_file));
    printf("                      0x%08X\n", get_uint32(wof_file));
    printf("                      0x%08X\n", get_uint32(wof_file));

    printf("Vdd Ceff add for throttling: %d\n", get_uint32(wof_file));
    printf("Throttling adjusted Vdd Ceff in 0.01 percent: %d\n", get_uint32(wof_file));
    printf("Throttling index percent in 0.000001: %d\n", get_uint32(wof_file));
    printf("Throttling index offset from OCC Scratch 3: %d\n", get_uint32(wof_file));
    l_signed16 = (int16_t)get_uint16(wof_file);
    printf("Eco mode ceff addr 0.01 percent: %d\n", l_signed16);
    printf("Eco mode max frequency degrade -%dMHz\n", get_uint16(wof_file));
    printf("Memory thermal credit constant: 0x%04X\n", get_uint16(wof_file));
    printf("Worst case DIMM pwr per OCMB: 0x%08XcW\n", get_uint32(wof_file));
    printf("OCMBs pwr data present: 0x%08X\n", get_uint32(wof_file));
    printf("Total worst case DIMM pwr: 0x%08XcW\n", get_uint32(wof_file));
    printf("Total present DIMM preheat pwr: 0x%08XcW\n", get_uint32(wof_file));
    l_signed8 = (int8_t)fgetc(wof_file);
    printf("ambient_adj_for_dimm: %d\n", l_signed8);
    for(i = 0; i < MAX_NUM_OCMBS; i++)
        printf("Sensor MEMUTILM%d: %d (0.01%%)\n", i, get_uint16(wof_file));
    for(i = 0; i < MAX_NUM_OCMBS; i++)
        printf("Sensor MEMUTILP1M%d: %d (0.01%%)\n", i, get_uint16(wof_file));

    // Close the file
    if(wof_file != NULL)
        fclose(wof_file);

    return 0;
}
