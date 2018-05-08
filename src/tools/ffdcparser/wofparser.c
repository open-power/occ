/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/tools/ffdcparser/wofparser.c $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2018                        */
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

// NOTE: This tool is to be used when FFDC is dumped by the OCC, and currently
//       only accepts input files in binary format.

void dump_wof_data(wof_data_t * data)
{
    uint32_t i = 0;
    printf("Expected Data size %d\n", sizeof(wof_data_t));
    printf("wof_disabled: 0x%08x\n", data->wof_disabled);
    printf("version: %d\n", data->version);
    printf("vfrt_block_size: %d\n", data->vfrt_block_size);
    printf("vfrt_blck_hdr_sz: %d\n", data->vfrt_blck_hdr_sz);
    printf("vfrt_data_size: %d\n", data->vfrt_data_size);
    printf("active_quads_size: %d\n", data->active_quads_size);
    printf("core_count: %d\n", data->core_count);
    printf("vdn_start: %d\n", data->vdn_start);
    printf("vdn_step: %d\n", data->vdn_step);
    printf("vdn_size: %d\n", data->vdn_size);
    printf("vdd_start: %d\n", data->vdd_start);
    printf("vdd_step: %d\n", data->vdd_step);
    printf("vdd_size: %d\n", data->vdd_size);
    printf("vratio_start: %d\n", data->vratio_start);
    printf("vratio_step: %d\n", data->vratio_step);
    printf("vratio_size: %d\n", data->vratio_size);
    printf("fratio_start: %d\n", data->fratio_start);
    printf("fratio_step: %d\n", data->fratio_step);
    printf("fratio_size: %d\n", data->fratio_size);
    for( i = 0; i < 8; i++)
        printf("vdn_percent[%d]: %d\n", i, data->vdn_percent[i]);
    printf("socket_power_w: %d\n", data->socket_power_w);
    printf("nest_freq_mhz: %d\n", data->nest_freq_mhz);
    printf("nom_freq_mhz: %d\n", data->nom_freq_mhz);
    printf("rdp_capacity: %d\n", data->rdp_capacity);
    printf("wof_tbls_src_tag: 0x%X\n", data->wof_tbls_src_tag);
    printf("package_name_hi: 0x%08x\n", data->package_name_hi);
    printf("package_name_lo: 0x%08x\n", data->package_name_lo);
    printf("vdd_step_from_start: %d\n", data->vdd_step_from_start);
    printf("vdn_step_from_start: %d\n", data->vdn_step_from_start);
    printf("quad_step_from_start: %d\n", data->quad_step_from_start);
    for(i = 0; i < MAXIMUM_QUADS; i++)
        printf("v_core_100uV[%d]: %d\n", i, data->v_core_100uV[i]);
    printf("core_pwr_on: 0x%08X\n", data->core_pwr_on);
    for(i = 0; i < MAXIMUM_QUADS; i++)
        printf("cores_on_per_quad[%d]: %d\n", i, data->cores_on_per_quad[i]);
    printf("voltvddsense_sensor: %d\n", data->voltvddsense_sensor);
    for(i = 0; i < MAX_NUM_CORES; i++)
        printf("tempprocthrmc[%d]: %d\n", i, data->tempprocthrmc[i]);
    printf("tempnest_sensor: %d\n", data->tempnest_sensor);
    for(i = 0; i < MAXIMUM_QUADS; i++)
        printf("tempq[%d]: %d\n", i, data->tempq[i]);
    printf("curvdd_sensor: %d\n", data->curvdd_sensor);
    printf("curvdn_sensor: %d\n", data->curvdn_sensor);
    printf("voltvdn_sensor: %d\n", data->voltvdn_sensor);
    for(i = 0; i < MAXIMUM_QUADS; i++)
        printf("quad_x_pstates[%d]: %d\n", i, data->quad_x_pstates[i]);
    for(i = 0; i < MAXIMUM_QUADS; i++)
        printf("quad_v_idx[%d]: %d\n", i, data->quad_v_idx[i]);
    printf("quad_ivrm_states: %d\n", data->quad_ivrm_states);
    printf("idc_vdd: %d\n", data->idc_vdd);
    printf("idc_vdn: %d\n", data->idc_vdn);
    printf("idc_quad %d\n", data->idc_quad);
    printf("iac_vdd: %d\n", data->iac_vdd);
    printf("iac_vdn: %d\n", data->iac_vdn);
    printf("iac_tdp_vdd %d\n", data->iac_tdp_vdd);
    printf("v_ratio: %d\n", data->v_ratio);
    printf("f_ratio: %d\n", data->f_ratio);
    printf("v_clip: %d\n", data->v_clip);
    printf("f_clip_ps: 0x%x\n", data->f_clip_ps);
    printf("f_clip_freq: %d\n", data->f_clip_freq);
    printf("ceff_tdp_vdd: %d\n", data->ceff_tdp_vdd);
    printf("ceff_vdd: %d\n", data->ceff_vdd);
    printf("ceff_ratio_vdd: %d\n", data->ceff_ratio_vdd);
    printf("ceff_tdp_vdn: %d\n", data->ceff_tdp_vdn);
    printf("ceff_vdn: %d\n", data->ceff_vdn);
    printf("ceff_ratio_vdn: %d\n", data->ceff_ratio_vdn);
    printf("chip_volt_idx: %d\n", data->chip_volt_idx);
    printf("all_cores_off_iso: %d\n", data->all_cores_off_iso);
    printf("all_good_caches_on_iso: %d\n", data->all_good_caches_on_iso);
    printf("all_caches_off_iso: %d\n", data->all_caches_off_iso);
    for( i = 0; i < MAXIMUM_QUADS; i++)
        printf("quad_good_cores_only[%d]: %d\n", i, data->quad_good_cores_only[i]);
    for( i = 0; i < MAXIMUM_QUADS; i++)
        printf("quad_on_cores[%d]: %d\n", i, data->quad_on_cores[i]);
    for( i = 0; i < MAXIMUM_QUADS; i++)
        printf("quad_bad_off_cores[%d]: %d\n", i, data->quad_bad_off_cores[i]);
    printf("req_active_quad_update: 0x%x\n", data->req_active_quad_update);
    printf("prev_req_active_quads: 0x%x\n", data->prev_req_active_quads);
    printf("num_active_quads: %d\n", data->num_active_quads);
    printf("curr_ping_pong_buf: 0x%08X\n", data->curr_ping_pong_buf);
    printf("next_ping_pong_buf: 0x%08X\n", data->next_ping_pong_buf);
    printf("curr_vfrt_main_mem_addr: 0x%08X\n", data->curr_vfrt_main_mem_addr);
    printf("next_vfrt_main_mem_addr: 0x%08X\n", data->next_vfrt_main_mem_addr);
    printf("vfrt_tbls_main_mem_addr: 0x%08X\n", data->vfrt_tbls_main_mem_addr);
    printf("vfrt_tbls_len: 0x%08X\n", data->vfrt_tbls_len);
    printf("wof_init_state: %d\n", data->wof_init_state);
    printf("quad_state_0_addr: 0x%08x\n", data->quad_state_0_addr);
    printf("quad_state_1_addr: 0x%08x\n", data->quad_state_1_addr);
    printf("pgpe_wof_state_addr: 0x%08x\n", data->pgpe_wof_state_addr);
    printf("req_active_quads_addr: 0x%08x\n", data->req_active_quads_addr);
    printf("core_leakage_percent: %d\n", data->core_leakage_percent);
    printf("pstate_tbl_sram_addr: 0x%08x\n", data->pstate_tbl_sram_addr);
    printf("gpe_req_rc: 0x%x\n", data->gpe_req_rc);
    printf("control_ipc_rc: 0x%x\n", data->control_ipc_rc);
    printf("vfrt_callback_error: 0x%x\n", data->vfrt_callback_error);
    printf("pgpe_wof_off: %d\n", data->pgpe_wof_off);
    printf("pgpe_wof_disabled: %d\n", data->pgpe_wof_disabled);
    printf("vfrt_mm_offset: %d\n", data->vfrt_mm_offset);
    printf("wof_vfrt_req_rc: 0x%x\n", data->wof_vfrt_req_rc);
    printf("c_ratio_vdd_volt: %d\n", data->c_ratio_vdd_volt);
    printf("c_ratio_vdd_freq: %d\n", data->c_ratio_vdd_freq);
    printf("c_ratio_vdn_volt: %d\n", data->c_ratio_vdn_volt);
    printf("c_ratio_vdn_freq: %d\n", data->c_ratio_vdn_freq);
    printf("vfrt_state: %d\n", data->vfrt_state);
    printf("all_cores_off_before: %d\n", data->all_cores_off_before);
    printf("good_quads_per_sort: %d\n", data->good_quads_per_sort);
    printf("good_normal_cores_per_sort: %d\n", data->good_normal_cores_per_sort);
    printf("good_caches_per_sort: %d\n", data->good_caches_per_sort);
    for( i = 0; i < MAXIMUM_QUADS; i++)
        printf("good_normal_cores[%d]: %d\n", i, data->good_normal_cores[i]);
    for( i = 0; i < MAXIMUM_QUADS; i++)
        printf("good_caches[%d]: %d\n", i, data->good_caches[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++ )
        printf("allGoodCoresCachesOn[%d]: %d\n",i, data->allGoodCoresCachesOn[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++ )
        printf("allCoresCachesOff[%d]: %d\n",i, data->allCoresCachesOff[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++ )
        printf("coresOffCachesOn[%d]: %d\n",i, data->coresOffCachesOn[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++ )
        printf("quad1CoresCachesOn[%d]: %d\n", i, data->quad1CoresCachesOn[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++ )
        printf("quad2CoresCachesOn[%d]: %d\n", i, data->quad2CoresCachesOn[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++ )
        printf("quad3CoresCachesOn[%d]: %d\n", i, data->quad3CoresCachesOn[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++ )
        printf("quad4CoresCachesOn[%d]: %d\n", i, data->quad4CoresCachesOn[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++ )
        printf("quad5CoresCachesOn[%d]: %d\n", i, data->quad5CoresCachesOn[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++ )
        printf("quad6CoresCachesOn[%d]: %d\n", i, data->quad6CoresCachesOn[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++ )
        printf("ivdn[%d]: %d\n", i, data->ivdn[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        printf("allCoresCachesOnT[%d]: %d\n", i, data->allCoresCachesOnT[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        printf("allCoresCachesOffT[%d]: %d\n", i, data->allCoresCachesOffT[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        printf("coresOffCachesOnT[%d]: %d\n", i, data->coresOffCachesOnT[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        printf("quad1CoresCachesOnT[%d]: %d\n", i , data->quad1CoresCachesOnT[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        printf("quad2CoresCachesOnT[%d]: %d\n", i , data->quad2CoresCachesOnT[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        printf("quad3CoresCachesOnT[%d]: %d\n", i , data->quad3CoresCachesOnT[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        printf("quad4CoresCachesOnT[%d]: %d\n", i , data->quad4CoresCachesOnT[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        printf("quad5CoresCachesOnT[%d]: %d\n", i , data->quad5CoresCachesOnT[i]);
    for( i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        printf("quad6CoresCachesOnT[%d]: %d\n", i , data->quad6CoresCachesOnT[i]);
    for(  i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        printf("avgtemp_vdn[%d]: %d\n",i, data->avgtemp_vdn[i]);
}

int main(int argc, char** argv)
{
    FILE*       wof_file = NULL;
    wof_data_t  data = {0};
    uint32_t    i = 0;

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
    if(file_size < sizeof(wof_data_t))
    {
        fprintf(stderr, "WARNING: WOF data file size (%d) is less than what was "
                "expected(%d)\n", file_size, sizeof(wof_data_t));
    }

    // Populate struct from file data
    data.wof_disabled   = get_uint32(wof_file);
    data.version        = fgetc(wof_file);
    data.vfrt_block_size= get_uint16(wof_file);
    data.vfrt_blck_hdr_sz = get_uint16(wof_file);
    data.vfrt_data_size = get_uint16(wof_file);
    data.active_quads_size = fgetc(wof_file);
    data.core_count = fgetc(wof_file);
    data.vdn_start = get_uint16(wof_file);
    data.vdn_step = get_uint16(wof_file);
    data.vdn_size = get_uint16(wof_file);
    data.vdd_start = get_uint16(wof_file);
    data.vdd_step = get_uint16(wof_file);
    data.vdd_size = get_uint16(wof_file);
    data.vratio_start = get_uint16(wof_file);
    data.vratio_step = get_uint16(wof_file);
    data.vratio_size = get_uint16(wof_file);
    data.fratio_start = get_uint16(wof_file);
    data.fratio_step = get_uint16(wof_file);
    data.fratio_size = get_uint16(wof_file);
    for( i = 0; i < 8; i++ )
        data.vdn_percent[i] = get_uint16(wof_file);
    data.socket_power_w = get_uint16(wof_file);
    data.nest_freq_mhz = get_uint16(wof_file);
    data.nom_freq_mhz = get_uint16(wof_file);
    data.rdp_capacity = get_uint16(wof_file);
    data.wof_tbls_src_tag = get_uint64(wof_file);
    data.package_name_hi = get_uint64(wof_file);
    data.package_name_lo = get_uint64(wof_file);
    data.vdd_step_from_start = get_uint16(wof_file);
    data.vdn_step_from_start = get_uint16(wof_file);
    data.quad_step_from_start = fgetc(wof_file);
    for(i = 0; i < MAXIMUM_QUADS; i++)
        data.v_core_100uV[i] = get_uint32(wof_file);
    data.core_pwr_on = get_uint32(wof_file);
    for(i = 0; i < MAXIMUM_QUADS; i++)
        data.cores_on_per_quad[i] = fgetc(wof_file);
    data.voltvddsense_sensor = get_uint32(wof_file);
    for(i = 0; i < MAX_NUM_CORES; i++)
        data.tempprocthrmc[i] = get_uint16(wof_file);
    data.tempnest_sensor = get_uint16(wof_file);
    for(i = 0; i < MAXIMUM_QUADS; i++)
        data.tempq[i] = get_uint16(wof_file);
    data.curvdd_sensor = get_uint16(wof_file);
    data.curvdn_sensor = get_uint16(wof_file);
    data.voltvdn_sensor = get_uint16(wof_file);
    for(i = 0; i < MAXIMUM_QUADS; i++)
        data.quad_x_pstates[i] = fgetc(wof_file);
    for(i = 0; i < MAXIMUM_QUADS; i++)
        data.quad_v_idx[i] = fgetc(wof_file);
    data.quad_ivrm_states = fgetc(wof_file);
    data.idc_vdd = get_uint32(wof_file);
    data.idc_vdn = get_uint32(wof_file);
    data.idc_quad = get_uint32(wof_file);
    data.iac_vdd = get_uint32(wof_file);
    data.iac_vdn = get_uint32(wof_file);
    data.iac_tdp_vdd = get_uint32(wof_file);
    data.v_ratio = get_uint16(wof_file);
    data.f_ratio = get_uint16(wof_file);
    data.v_clip = get_uint16(wof_file);
    data.f_clip_ps = fgetc(wof_file);
    data.f_clip_freq = get_uint32(wof_file);
    data.ceff_tdp_vdd = get_uint32(wof_file);
    data.ceff_vdd = get_uint32(wof_file);
    data.ceff_ratio_vdd = get_uint32(wof_file);
    data.ceff_tdp_vdn = get_uint32(wof_file);
    data.ceff_vdn = get_uint32(wof_file);
    data.ceff_ratio_vdn = get_uint32(wof_file);
    data.chip_volt_idx = fgetc(wof_file);
    data.all_cores_off_iso = get_uint32(wof_file);
    data.all_good_caches_on_iso = get_uint32(wof_file);
    data.all_caches_off_iso = get_uint32(wof_file);
    for(i = 0; i < MAXIMUM_QUADS; i++)
        data.quad_good_cores_only[i] = get_uint32(wof_file);
    for(i = 0; i < MAXIMUM_QUADS; i++)
        data.quad_on_cores[i] = get_uint16(wof_file);
    for(i = 0; i < MAXIMUM_QUADS; i++)
        data.quad_bad_off_cores[i] = get_uint16(wof_file);
    data.req_active_quad_update = fgetc(wof_file);
    data.prev_req_active_quads = fgetc(wof_file);
    data.num_active_quads = fgetc(wof_file);
    data.curr_ping_pong_buf = get_uint32(wof_file);
    data.next_ping_pong_buf = get_uint32(wof_file);
    data.curr_vfrt_main_mem_addr = get_uint32(wof_file);
    data.next_vfrt_main_mem_addr = get_uint32(wof_file);
    data.vfrt_tbls_main_mem_addr = get_uint32(wof_file);
    data.vfrt_tbls_len = get_uint32(wof_file);
    data.wof_init_state = fgetc(wof_file);
    data.quad_state_0_addr = get_uint32(wof_file);
    data.quad_state_1_addr = get_uint32(wof_file);
    data.pgpe_wof_state_addr = get_uint32(wof_file);
    data.req_active_quads_addr = get_uint32(wof_file);
    data.core_leakage_percent = get_uint16(wof_file);
    data.pstate_tbl_sram_addr = get_uint32(wof_file);
    data.gpe_req_rc = get_uint32(wof_file);
    data.control_ipc_rc = get_uint32(wof_file);
    data.vfrt_callback_error = fgetc(wof_file);
    data.pgpe_wof_off = fgetc(wof_file);
    data.pgpe_wof_disabled = fgetc(wof_file);
    data.vfrt_mm_offset = get_uint32(wof_file);
    data.wof_vfrt_req_rc = fgetc(wof_file);
    data.c_ratio_vdd_volt = get_uint32(wof_file);
    data.c_ratio_vdd_freq = get_uint32(wof_file);
    data.c_ratio_vdn_volt = get_uint32(wof_file);
    data.c_ratio_vdn_freq = get_uint32(wof_file);
    data.vfrt_state = fgetc(wof_file);
    data.all_cores_off_before = get_uint32(wof_file);
    data.good_quads_per_sort = fgetc(wof_file);
    data.good_normal_cores_per_sort = fgetc(wof_file);
    data.good_caches_per_sort = fgetc(wof_file);
    for(i = 0; i < MAXIMUM_QUADS; i++)
        data.good_normal_cores[i] = fgetc(wof_file);
    for(i = 0; i < MAXIMUM_QUADS; i++)
        data.good_caches[i] = fgetc(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.allGoodCoresCachesOn[i] = get_uint16(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.allCoresCachesOff[i] = get_uint16(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.quad1CoresCachesOn[i] = get_uint16(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.quad2CoresCachesOn[i] = get_uint16(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.quad3CoresCachesOn[i] = get_uint16(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.quad4CoresCachesOn[i] = get_uint16(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.quad5CoresCachesOn[i] = get_uint16(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.quad6CoresCachesOn[i] = get_uint16(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.ivdn[i]= get_uint16(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.allCoresCachesOnT[i] = fgetc(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.allCoresCachesOffT[i] = fgetc(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.coresOffCachesOnT[i] = fgetc(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.quad1CoresCachesOnT[i] = fgetc(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.quad2CoresCachesOnT[i] = fgetc(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.quad3CoresCachesOnT[i] = fgetc(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.quad4CoresCachesOnT[i] = fgetc(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.quad5CoresCachesOnT[i] = fgetc(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.quad6CoresCachesOnT[i] = fgetc(wof_file);
    for(i = 0; i < CORE_IDDQ_MEASUREMENTS; i++)
        data.avgtemp_vdn[i] = fgetc(wof_file);

    // Print the data to stdout
    dump_wof_data(&data);

    // Close the file
    if(wof_file != NULL)
        fclose(wof_file);

    return 0;
}
