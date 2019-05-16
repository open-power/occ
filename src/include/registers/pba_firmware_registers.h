/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/include/registers/pba_firmware_registers.h $              */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2019                        */
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
#ifndef __PBA_FIRMWARE_REGISTERS_H__
#define __PBA_FIRMWARE_REGISTERS_H__

/// \file pba_firmware_registers.h
/// \brief C register structs for the PBA unit

// *** WARNING *** - This file is generated automatically, do not edit.

#ifndef SIXTYFOUR_BIT_CONSTANT
    #ifdef __ASSEMBLER__
        #define SIXTYFOUR_BIT_CONSTANT(x) x
    #else
        #define SIXTYFOUR_BIT_CONSTANT(x) x##ull
    #endif
#endif

#ifndef __ASSEMBLER__

#include <stdint.h>




typedef union pba_mode
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t reserved1 : 4;
    uint64_t dis_rearb : 1;
    uint64_t dis_mstid_match_pref_inv : 1;
    uint64_t dis_slave_rdpipe : 1;
    uint64_t dis_slave_wrpipe : 1;
    uint64_t en_marker_ack : 1;
    uint64_t reserved2 : 1;
    uint64_t en_second_wrbuf : 1;
    uint64_t dis_rerequest_to : 1;
    uint64_t inject_type : 2;
    uint64_t inject_mode : 2;
    uint64_t pba_region : 2;
    uint64_t oci_marker_space : 3;
    uint64_t bcde_ocitrans : 2;
    uint64_t bcue_ocitrans : 2;
    uint64_t dis_master_rd_pipe : 1;
    uint64_t dis_master_wr_pipe : 1;
    uint64_t en_slv_fairness : 1;
    uint64_t en_event_count : 1;
        uint64_t reserved3 : 1;
    uint64_t slv_event_mux : 2;
    uint64_t enable_debug_bus : 1;
    uint64_t debug_pb_not_oci : 1;
    uint64_t debug_oci_mode : 5;
        uint64_t reserved4 : 1;
    uint64_t ocislv_fairness_mask : 5;
    uint64_t ocislv_rereq_hang_div : 5;
        uint64_t reserved5 : 14;
#else
        uint64_t reserved5 : 14;
    uint64_t ocislv_rereq_hang_div : 5;
    uint64_t ocislv_fairness_mask : 5;
        uint64_t reserved4 : 1;
    uint64_t debug_oci_mode : 5;
    uint64_t debug_pb_not_oci : 1;
    uint64_t enable_debug_bus : 1;
    uint64_t slv_event_mux : 2;
        uint64_t reserved3 : 1;
    uint64_t en_event_count : 1;
    uint64_t en_slv_fairness : 1;
    uint64_t dis_master_wr_pipe : 1;
    uint64_t dis_master_rd_pipe : 1;
    uint64_t bcue_ocitrans : 2;
    uint64_t bcde_ocitrans : 2;
    uint64_t oci_marker_space : 3;
    uint64_t pba_region : 2;
    uint64_t inject_mode : 2;
    uint64_t inject_type : 2;
    uint64_t dis_rerequest_to : 1;
    uint64_t en_second_wrbuf : 1;
    uint64_t reserved2 : 1;
    uint64_t en_marker_ack : 1;
    uint64_t dis_slave_wrpipe : 1;
    uint64_t dis_slave_rdpipe : 1;
    uint64_t dis_mstid_match_pref_inv : 1;
    uint64_t dis_rearb : 1;
    uint64_t reserved1 : 4;
#endif // _BIG_ENDIAN
    } fields;
} pba_mode_t;



typedef union pba_slvrst
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t set : 3;
    uint64_t reserved1 : 1;
    uint64_t in_prog : 4;
    uint64_t busy_status : 4;
    uint64_t scope_attn_bar : 2;
    uint64_t reserved2 : 50;
#else
    uint64_t reserved2 : 50;
    uint64_t scope_attn_bar : 2;
    uint64_t busy_status : 4;
    uint64_t in_prog : 4;
    uint64_t reserved1 : 1;
    uint64_t set : 3;
#endif // _BIG_ENDIAN
    } fields;
} pba_slvrst_t;



typedef union pba_slvctln
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t enable : 1;
    uint64_t mid_match_value : 3;
    uint64_t reserved1 : 1;
    uint64_t mid_care_mask : 3;
    uint64_t write_ttype : 3;
    uint64_t reserved2 : 4;
    uint64_t read_ttype : 1;
    uint64_t read_prefetch_ctl : 2;
    uint64_t buf_invalidate_ctl : 1;
    uint64_t buf_alloc_w : 1;
    uint64_t buf_alloc_a : 1;
    uint64_t buf_alloc_b : 1;
    uint64_t buf_alloc_c : 1;
    uint64_t reserved3 : 1;
    uint64_t dis_write_gather : 1;
    uint64_t wr_gather_timeout : 3;
    uint64_t write_tsize : 8;
    uint64_t extaddr : 14;
    uint64_t reserved4 : 1;
    uint64_t reserved5 : 13;
#else
    uint64_t reserved5 : 13;
    uint64_t reserved4 : 1;
    uint64_t extaddr : 14;
    uint64_t write_tsize : 8;
    uint64_t wr_gather_timeout : 3;
    uint64_t dis_write_gather : 1;
    uint64_t reserved3 : 1;
    uint64_t buf_alloc_c : 1;
    uint64_t buf_alloc_b : 1;
    uint64_t buf_alloc_a : 1;
    uint64_t buf_alloc_w : 1;
    uint64_t buf_invalidate_ctl : 1;
    uint64_t read_prefetch_ctl : 2;
    uint64_t read_ttype : 1;
    uint64_t reserved2 : 4;
    uint64_t write_ttype : 3;
    uint64_t mid_care_mask : 3;
    uint64_t reserved1 : 1;
    uint64_t mid_match_value : 3;
    uint64_t enable : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_slvctln_t;



typedef union pba_bcde_ctl
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t stop : 1;
    uint64_t start : 1;
    uint64_t reserved1 : 62;
#else
    uint64_t reserved1 : 62;
    uint64_t start : 1;
    uint64_t stop : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_bcde_ctl_t;



typedef union pba_bcde_set
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t reserved1 : 2;
    uint64_t copy_length : 6;
    uint64_t reserved2 : 56;
#else
    uint64_t reserved2 : 56;
    uint64_t copy_length : 6;
    uint64_t reserved1 : 2;
#endif // _BIG_ENDIAN
    } fields;
} pba_bcde_set_t;



typedef union pba_bcde_stat
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t running : 1;
    uint64_t waiting : 1;
    uint64_t wrcmp : 6;
    uint64_t reserved1 : 6;
    uint64_t rdcmp : 6;
    uint64_t debug : 9;
    uint64_t stopped : 1;
    uint64_t error : 1;
    uint64_t done : 1;
    uint64_t reserved2 : 32;
#else
    uint64_t reserved2 : 32;
    uint64_t done : 1;
    uint64_t error : 1;
    uint64_t stopped : 1;
    uint64_t debug : 9;
    uint64_t rdcmp : 6;
    uint64_t reserved1 : 6;
    uint64_t wrcmp : 6;
    uint64_t waiting : 1;
    uint64_t running : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_bcde_stat_t;



typedef union pba_bcde_dr
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t reserved1 : 2;
    uint64_t pb_offset : 23;
    uint64_t reserved2 : 2;
    uint64_t extaddr : 14;
    uint64_t reserved3 : 2;
    uint64_t reserved4 : 21;
#else
    uint64_t reserved4 : 21;
    uint64_t reserved3 : 2;
    uint64_t extaddr : 14;
    uint64_t reserved2 : 2;
    uint64_t pb_offset : 23;
    uint64_t reserved1 : 2;
#endif // _BIG_ENDIAN
    } fields;
} pba_bcde_dr_t;



typedef union pba_bcde_ocibar
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t addr : 25;
    uint64_t reserved1 : 39;
#else
    uint64_t reserved1 : 39;
    uint64_t addr : 25;
#endif // _BIG_ENDIAN
    } fields;
} pba_bcde_ocibar_t;



typedef union pba_bcue_ctl
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t stop : 1;
    uint64_t start : 1;
    uint64_t reserved1 : 62;
#else
    uint64_t reserved1 : 62;
    uint64_t start : 1;
    uint64_t stop : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_bcue_ctl_t;



typedef union pba_bcue_set
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t reserved1 : 2;
    uint64_t copy_length : 6;
    uint64_t reserved2 : 56;
#else
    uint64_t reserved2 : 56;
    uint64_t copy_length : 6;
    uint64_t reserved1 : 2;
#endif // _BIG_ENDIAN
    } fields;
} pba_bcue_set_t;



typedef union pba_bcue_stat
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t running : 1;
    uint64_t waiting : 1;
    uint64_t wrcmp : 6;
    uint64_t reserved1 : 6;
    uint64_t rdcmp : 6;
    uint64_t debug : 9;
    uint64_t stopped : 1;
    uint64_t error : 1;
    uint64_t done : 1;
    uint64_t reserved2 : 32;
#else
    uint64_t reserved2 : 32;
    uint64_t done : 1;
    uint64_t error : 1;
    uint64_t stopped : 1;
    uint64_t debug : 9;
    uint64_t rdcmp : 6;
    uint64_t reserved1 : 6;
    uint64_t wrcmp : 6;
    uint64_t waiting : 1;
    uint64_t running : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_bcue_stat_t;



typedef union pba_bcue_dr
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t reserved1 : 2;
    uint64_t pb_offset : 23;
    uint64_t reserved2 : 2;
    uint64_t extaddr : 14;
    uint64_t reserved3 : 2;
    uint64_t reserved4 : 21;
#else
    uint64_t reserved4 : 21;
    uint64_t reserved3 : 2;
    uint64_t extaddr : 14;
    uint64_t reserved2 : 2;
    uint64_t pb_offset : 23;
    uint64_t reserved1 : 2;
#endif // _BIG_ENDIAN
    } fields;
} pba_bcue_dr_t;



typedef union pba_bcue_ocibar
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t addr : 25;
    uint64_t reserved1 : 39;
#else
    uint64_t reserved1 : 39;
    uint64_t addr : 25;
#endif // _BIG_ENDIAN
    } fields;
} pba_bcue_ocibar_t;



typedef union pba_ocrn
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t reserved1 : 44;
        uint64_t count : 20;
#else
        uint64_t count : 20;
        uint64_t reserved1 : 44;
#endif // _BIG_ENDIAN
    } fields;
} pba_ocrn_t;



typedef union pba_xsndtx
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t snd_scope : 3;
    uint64_t snd_qid : 1;
    uint64_t snd_type : 1;
    uint64_t snd_reservation : 1;
    uint64_t reserved1 : 2;
    uint64_t snd_groupid : 4;
    uint64_t snd_chipid : 3;
    uint64_t reserved2 : 1;
    uint64_t vg_targe : 16;
    uint64_t reserved3 : 27;
    uint64_t snd_stop : 1;
    uint64_t snd_cnt : 4;
#else
    uint64_t snd_cnt : 4;
    uint64_t snd_stop : 1;
    uint64_t reserved3 : 27;
    uint64_t vg_targe : 16;
    uint64_t reserved2 : 1;
    uint64_t snd_chipid : 3;
    uint64_t snd_groupid : 4;
    uint64_t reserved1 : 2;
    uint64_t snd_reservation : 1;
    uint64_t snd_type : 1;
    uint64_t snd_qid : 1;
    uint64_t snd_scope : 3;
#endif // _BIG_ENDIAN
    } fields;
} pba_xsndtx_t;



typedef union pba_xcfg
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t pbax_en : 1;
    uint64_t reservation_en : 1;
    uint64_t snd_reset : 1;
    uint64_t rcv_reset : 1;
    uint64_t rcv_groupid : 4;
    uint64_t rcv_chipid : 3;
    uint64_t reserved1 : 1;
    uint64_t rcv_brdcst_group : 8;
    uint64_t rcv_datato_div : 5;
    uint64_t reserved2 : 2;
    uint64_t snd_retry_count_overcom : 1;
    uint64_t snd_retry_thresh : 8;
    uint64_t snd_rsvto_div : 5;
    uint64_t reserved3 : 23;
#else
    uint64_t reserved3 : 23;
    uint64_t snd_rsvto_div : 5;
    uint64_t snd_retry_thresh : 8;
    uint64_t snd_retry_count_overcom : 1;
    uint64_t reserved2 : 2;
    uint64_t rcv_datato_div : 5;
    uint64_t rcv_brdcst_group : 8;
    uint64_t reserved1 : 1;
    uint64_t rcv_chipid : 3;
    uint64_t rcv_groupid : 4;
    uint64_t rcv_reset : 1;
    uint64_t snd_reset : 1;
    uint64_t reservation_en : 1;
    uint64_t pbax_en : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_xcfg_t;



typedef union pba_xsndstat
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t snd_in_progress : 1;
    uint64_t snd_error : 1;
    uint64_t snd_phase_status : 2;
    uint64_t snd_cnt_status : 4;
    uint64_t snd_retry_count : 8;
    uint64_t reserved1 : 48;
#else
    uint64_t reserved1 : 48;
    uint64_t snd_retry_count : 8;
    uint64_t snd_cnt_status : 4;
    uint64_t snd_phase_status : 2;
    uint64_t snd_error : 1;
    uint64_t snd_in_progress : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_xsndstat_t;



typedef union pba_xsnddat
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t pbax_datahi : 32;
    uint64_t pbax_datalo : 32;
#else
    uint64_t pbax_datalo : 32;
    uint64_t pbax_datahi : 32;
#endif // _BIG_ENDIAN
    } fields;
} pba_xsnddat_t;



typedef union pba_xrcvstat
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t rcv_in_progress : 1;
    uint64_t rcv_error : 1;
    uint64_t rcv_write_in_progress : 1;
    uint64_t rcv_reservation_set : 1;
    uint64_t rcv_capture : 16;
    uint64_t reserved1 : 44;
#else
    uint64_t reserved1 : 44;
    uint64_t rcv_capture : 16;
    uint64_t rcv_reservation_set : 1;
    uint64_t rcv_write_in_progress : 1;
    uint64_t rcv_error : 1;
    uint64_t rcv_in_progress : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_xrcvstat_t;



typedef union pba_xshbrn
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t push_start : 29;
    uint64_t reserved1 : 35;
#else
    uint64_t reserved1 : 35;
    uint64_t push_start : 29;
#endif // _BIG_ENDIAN
    } fields;
} pba_xshbrn_t;



typedef union pba_xshcsn
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t push_full : 1;
    uint64_t push_empty : 1;
    uint64_t reserved1 : 2;
        uint64_t push_intr_action01 : 2;
    uint64_t push_length : 5;
    uint64_t reserved2 : 2;
    uint64_t push_write_ptr : 5;
    uint64_t reserved3 : 3;
    uint64_t push_read_ptr : 5;
    uint64_t reserved4 : 5;
    uint64_t push_enable : 1;
    uint64_t reserved5 : 32;
#else
    uint64_t reserved5 : 32;
    uint64_t push_enable : 1;
    uint64_t reserved4 : 5;
    uint64_t push_read_ptr : 5;
    uint64_t reserved3 : 3;
    uint64_t push_write_ptr : 5;
    uint64_t reserved2 : 2;
    uint64_t push_length : 5;
        uint64_t push_intr_action01 : 2;
    uint64_t reserved1 : 2;
    uint64_t push_empty : 1;
    uint64_t push_full : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_xshcsn_t;



typedef union pba_xshincn
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t reserved1 : 64;
#else
    uint64_t reserved1 : 64;
#endif // _BIG_ENDIAN
    } fields;
} pba_xshincn_t;



typedef union pba_xisndtx
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t snd_scope : 3;
        uint64_t snd_qid : 1;
        uint64_t snd_type : 1;
        uint64_t snd_reservation : 1;
        uint64_t reserved1 : 2;
        uint64_t snd_groupid : 4;
        uint64_t snd_chipid : 3;
        uint64_t reserved2 : 1;
        uint64_t vg_targe : 16;
        uint64_t reserved3 : 27;
        uint64_t snd_stop : 1;
        uint64_t snd_cnt : 4;
#else
        uint64_t snd_cnt : 4;
        uint64_t snd_stop : 1;
        uint64_t reserved3 : 27;
        uint64_t vg_targe : 16;
        uint64_t reserved2 : 1;
        uint64_t snd_chipid : 3;
        uint64_t snd_groupid : 4;
        uint64_t reserved1 : 2;
        uint64_t snd_reservation : 1;
        uint64_t snd_type : 1;
        uint64_t snd_qid : 1;
        uint64_t snd_scope : 3;
#endif // _BIG_ENDIAN
    } fields;
} pba_xisndtx_t;



typedef union pba_xicfg
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t pbax_en : 1;
        uint64_t reservation_en : 1;
        uint64_t snd_reset : 1;
        uint64_t rcv_reset : 1;
        uint64_t rcv_groupid : 4;
        uint64_t rcv_chipid : 3;
        uint64_t reserved1 : 1;
        uint64_t rcv_brdcst_group : 8;
        uint64_t rcv_datato_div : 5;
        uint64_t reserved2 : 2;
        uint64_t snd_retry_count_overcom : 1;
        uint64_t snd_retry_thresh : 8;
        uint64_t snd_rsvto_div : 5;
        uint64_t reserved3 : 23;
#else
        uint64_t reserved3 : 23;
        uint64_t snd_rsvto_div : 5;
        uint64_t snd_retry_thresh : 8;
        uint64_t snd_retry_count_overcom : 1;
        uint64_t reserved2 : 2;
        uint64_t rcv_datato_div : 5;
        uint64_t rcv_brdcst_group : 8;
        uint64_t reserved1 : 1;
        uint64_t rcv_chipid : 3;
        uint64_t rcv_groupid : 4;
        uint64_t rcv_reset : 1;
        uint64_t snd_reset : 1;
        uint64_t reservation_en : 1;
        uint64_t pbax_en : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_xicfg_t;



typedef union pba_xisndstat
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t snd_in_progress : 1;
        uint64_t snd_error : 1;
        uint64_t snd_phase_status : 2;
        uint64_t snd_cnt_status : 4;
        uint64_t snd_retry_count : 8;
        uint64_t reserved1 : 48;
#else
        uint64_t reserved1 : 48;
        uint64_t snd_retry_count : 8;
        uint64_t snd_cnt_status : 4;
        uint64_t snd_phase_status : 2;
        uint64_t snd_error : 1;
        uint64_t snd_in_progress : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_xisndstat_t;



typedef union pba_xisnddat
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t pbax_datahi : 32;
        uint64_t pbax_datalo : 32;
#else
        uint64_t pbax_datalo : 32;
        uint64_t pbax_datahi : 32;
#endif // _BIG_ENDIAN
    } fields;
} pba_xisnddat_t;



typedef union pba_xircvstat
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t rcv_in_progress : 1;
        uint64_t rcv_error : 1;
        uint64_t rcv_write_in_progress : 1;
        uint64_t rcv_reservation_set : 1;
        uint64_t rcv_capture : 16;
        uint64_t reserved1 : 44;
#else
        uint64_t reserved1 : 44;
        uint64_t rcv_capture : 16;
        uint64_t rcv_reservation_set : 1;
        uint64_t rcv_write_in_progress : 1;
        uint64_t rcv_error : 1;
        uint64_t rcv_in_progress : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_xircvstat_t;



typedef union pba_xishbrn
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t push_start : 29;
        uint64_t reserved1 : 35;
#else
        uint64_t reserved1 : 35;
        uint64_t push_start : 29;
#endif // _BIG_ENDIAN
    } fields;
} pba_xishbrn_t;



typedef union pba_xishcsn
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t push_full : 1;
        uint64_t push_empty : 1;
        uint64_t reserved1 : 2;
        uint64_t push_intr_action01 : 2;
        uint64_t push_length : 5;
        uint64_t reserved2 : 2;
        uint64_t push_write_ptr : 5;
        uint64_t reserved3 : 3;
        uint64_t push_read_ptr : 5;
        uint64_t reserved4 : 5;
        uint64_t push_enable : 1;
        uint64_t reserved5 : 32;
#else
        uint64_t reserved5 : 32;
        uint64_t push_enable : 1;
        uint64_t reserved4 : 5;
        uint64_t push_read_ptr : 5;
        uint64_t reserved3 : 3;
        uint64_t push_write_ptr : 5;
        uint64_t reserved2 : 2;
        uint64_t push_length : 5;
        uint64_t push_intr_action01 : 2;
        uint64_t reserved1 : 2;
        uint64_t push_empty : 1;
        uint64_t push_full : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_xishcsn_t;



typedef union pba_xishincn
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t reserved1 : 64;
#else
        uint64_t reserved1 : 64;
#endif // _BIG_ENDIAN
    } fields;
} pba_xishincn_t;



typedef union pba_fir
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t oci_apar_err : 1;
    uint64_t oci_slave_init : 1;
    uint64_t oci_wrpar_err : 1;
    uint64_t reserved1 : 1;
    uint64_t bcue_setup_err : 1;
    uint64_t bcue_oci_daterr : 1;
    uint64_t bcde_setup_err : 1;
    uint64_t bcde_oci_daterr : 1;
    uint64_t internal_err : 1;
    uint64_t oci_bad_reg_addr : 1;
    uint64_t axpush_wrerr : 1;
        uint64_t axipush_wrerr : 1;
    uint64_t axflow_err : 1;
        uint64_t axiflow_err : 1;
    uint64_t axsnd_rsverr : 1;
        uint64_t axisnd_rsverr : 1;
    uint64_t reserved2 : 1;
    uint64_t reserved3 : 1;
    uint64_t reserved4 : 1;
        uint64_t reserved5 : 1;
        uint64_t reserved6 : 44;
#else
        uint64_t reserved6 : 44;
        uint64_t reserved5 : 1;
    uint64_t reserved4 : 1;
    uint64_t reserved3 : 1;
    uint64_t reserved2 : 1;
        uint64_t axisnd_rsverr : 1;
    uint64_t axsnd_rsverr : 1;
        uint64_t axiflow_err : 1;
    uint64_t axflow_err : 1;
        uint64_t axipush_wrerr : 1;
    uint64_t axpush_wrerr : 1;
    uint64_t oci_bad_reg_addr : 1;
    uint64_t internal_err : 1;
    uint64_t bcde_oci_daterr : 1;
    uint64_t bcde_setup_err : 1;
    uint64_t bcue_oci_daterr : 1;
    uint64_t bcue_setup_err : 1;
    uint64_t reserved1 : 1;
    uint64_t oci_wrpar_err : 1;
    uint64_t oci_slave_init : 1;
    uint64_t oci_apar_err : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_fir_t;



typedef union pba_fir_and
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t oci_apar_err : 1;
    uint64_t oci_slave_init : 1;
    uint64_t oci_wrpar_err : 1;
    uint64_t reserved1 : 1;
    uint64_t bcue_setup_err : 1;
    uint64_t bcue_oci_daterr : 1;
    uint64_t bcde_setup_err : 1;
    uint64_t bcde_oci_daterr : 1;
    uint64_t internal_err : 1;
    uint64_t oci_bad_reg_addr : 1;
    uint64_t axpush_wrerr : 1;
        uint64_t axipush_wrerr : 1;
    uint64_t axflow_err : 1;
        uint64_t axiflow_err : 1;
    uint64_t axsnd_rsverr : 1;
        uint64_t axisnd_rsverr : 1;
    uint64_t reserved2 : 1;
    uint64_t reserved3 : 1;
    uint64_t reserved4 : 1;
        uint64_t reserved5 : 1;
        uint64_t reserved6 : 44;
#else
        uint64_t reserved6 : 44;
        uint64_t reserved5 : 1;
    uint64_t reserved4 : 1;
    uint64_t reserved3 : 1;
    uint64_t reserved2 : 1;
        uint64_t axisnd_rsverr : 1;
    uint64_t axsnd_rsverr : 1;
        uint64_t axiflow_err : 1;
    uint64_t axflow_err : 1;
        uint64_t axipush_wrerr : 1;
    uint64_t axpush_wrerr : 1;
    uint64_t oci_bad_reg_addr : 1;
    uint64_t internal_err : 1;
    uint64_t bcde_oci_daterr : 1;
    uint64_t bcde_setup_err : 1;
    uint64_t bcue_oci_daterr : 1;
    uint64_t bcue_setup_err : 1;
    uint64_t reserved1 : 1;
    uint64_t oci_wrpar_err : 1;
    uint64_t oci_slave_init : 1;
    uint64_t oci_apar_err : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_fir_and_t;



typedef union pba_fir_or
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t oci_apar_err : 1;
    uint64_t oci_slave_init : 1;
    uint64_t oci_wrpar_err : 1;
    uint64_t reserved1 : 1;
    uint64_t bcue_setup_err : 1;
    uint64_t bcue_oci_daterr : 1;
    uint64_t bcde_setup_err : 1;
    uint64_t bcde_oci_daterr : 1;
    uint64_t internal_err : 1;
    uint64_t oci_bad_reg_addr : 1;
    uint64_t axpush_wrerr : 1;
        uint64_t axipush_wrerr : 1;
    uint64_t axflow_err : 1;
        uint64_t axiflow_err : 1;
    uint64_t axsnd_rsverr : 1;
        uint64_t axisnd_rsverr : 1;
    uint64_t reserved2 : 1;
    uint64_t reserved3 : 1;
    uint64_t reserved4 : 1;
        uint64_t reserved5 : 1;
        uint64_t reserved6 : 44;
#else
        uint64_t reserved6 : 44;
        uint64_t reserved5 : 1;
    uint64_t reserved4 : 1;
    uint64_t reserved3 : 1;
    uint64_t reserved2 : 1;
        uint64_t axisnd_rsverr : 1;
    uint64_t axsnd_rsverr : 1;
        uint64_t axiflow_err : 1;
    uint64_t axflow_err : 1;
        uint64_t axipush_wrerr : 1;
    uint64_t axpush_wrerr : 1;
    uint64_t oci_bad_reg_addr : 1;
    uint64_t internal_err : 1;
    uint64_t bcde_oci_daterr : 1;
    uint64_t bcde_setup_err : 1;
    uint64_t bcue_oci_daterr : 1;
    uint64_t bcue_setup_err : 1;
    uint64_t reserved1 : 1;
    uint64_t oci_wrpar_err : 1;
    uint64_t oci_slave_init : 1;
    uint64_t oci_apar_err : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_fir_or_t;



typedef union pba_firmask
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t oci_apar_err_mask : 1;
    uint64_t oci_slave_init_mask : 1;
    uint64_t oci_wrpar_err_mask : 1;
    uint64_t reserved1 : 1;
    uint64_t bcue_setup_err_mask : 1;
    uint64_t bcue_oci_daterr_mask : 1;
    uint64_t bcde_setup_err_mask : 1;
    uint64_t bcde_oci_daterr_mask : 1;
        uint64_t internals_err_mask : 1;
    uint64_t oci_bad_reg_addr_mask : 1;
    uint64_t axpush_wrerr_mask : 1;
        uint64_t axipush_wrerr_mask : 1;
    uint64_t axflow_err_mask : 1;
        uint64_t axiflow_err_mask : 1;
    uint64_t axsnd_rsverr_mask : 1;
        uint64_t axisnd_rsverr_mask : 1;
        uint64_t reserved_mask : 4;
        uint64_t reserved2 : 44;
#else
        uint64_t reserved2 : 44;
        uint64_t reserved_mask : 4;
        uint64_t axisnd_rsverr_mask : 1;
    uint64_t axsnd_rsverr_mask : 1;
        uint64_t axiflow_err_mask : 1;
    uint64_t axflow_err_mask : 1;
        uint64_t axipush_wrerr_mask : 1;
    uint64_t axpush_wrerr_mask : 1;
    uint64_t oci_bad_reg_addr_mask : 1;
        uint64_t internals_err_mask : 1;
    uint64_t bcde_oci_daterr_mask : 1;
    uint64_t bcde_setup_err_mask : 1;
    uint64_t bcue_oci_daterr_mask : 1;
    uint64_t bcue_setup_err_mask : 1;
    uint64_t reserved1 : 1;
    uint64_t oci_wrpar_err_mask : 1;
    uint64_t oci_slave_init_mask : 1;
    uint64_t oci_apar_err_mask : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_firmask_t;



typedef union pba_firmask_and
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t oci_apar_err_mask : 1;
    uint64_t oci_slave_init_mask : 1;
    uint64_t oci_wrpar_err_mask : 1;
    uint64_t reserved1 : 1;
    uint64_t bcue_setup_err_mask : 1;
    uint64_t bcue_oci_daterr_mask : 1;
    uint64_t bcde_setup_err_mask : 1;
    uint64_t bcde_oci_daterr_mask : 1;
        uint64_t internals_err_mask : 1;
    uint64_t oci_bad_reg_addr_mask : 1;
    uint64_t axpush_wrerr_mask : 1;
        uint64_t axipush_wrerr_mask : 1;
    uint64_t axflow_err_mask : 1;
        uint64_t axiflow_err_mask : 1;
    uint64_t axsnd_rsverr_mask : 1;
        uint64_t axisnd_rsverr_mask : 1;
        uint64_t reserved_mask : 4;
        uint64_t reserved2 : 44;
#else
        uint64_t reserved2 : 44;
        uint64_t reserved_mask : 4;
        uint64_t axisnd_rsverr_mask : 1;
    uint64_t axsnd_rsverr_mask : 1;
        uint64_t axiflow_err_mask : 1;
    uint64_t axflow_err_mask : 1;
        uint64_t axipush_wrerr_mask : 1;
    uint64_t axpush_wrerr_mask : 1;
    uint64_t oci_bad_reg_addr_mask : 1;
        uint64_t internals_err_mask : 1;
    uint64_t bcde_oci_daterr_mask : 1;
    uint64_t bcde_setup_err_mask : 1;
    uint64_t bcue_oci_daterr_mask : 1;
    uint64_t bcue_setup_err_mask : 1;
    uint64_t reserved1 : 1;
    uint64_t oci_wrpar_err_mask : 1;
    uint64_t oci_slave_init_mask : 1;
    uint64_t oci_apar_err_mask : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_firmask_and_t;



typedef union pba_firmask_or
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t oci_apar_err_mask : 1;
    uint64_t oci_slave_init_mask : 1;
    uint64_t oci_wrpar_err_mask : 1;
    uint64_t reserved1 : 1;
    uint64_t bcue_setup_err_mask : 1;
    uint64_t bcue_oci_daterr_mask : 1;
    uint64_t bcde_setup_err_mask : 1;
    uint64_t bcde_oci_daterr_mask : 1;
        uint64_t internals_err_mask : 1;
    uint64_t oci_bad_reg_addr_mask : 1;
    uint64_t axpush_wrerr_mask : 1;
        uint64_t axipush_wrerr_mask : 1;
    uint64_t axflow_err_mask : 1;
        uint64_t axiflow_err_mask : 1;
    uint64_t axsnd_rsverr_mask : 1;
        uint64_t axisnd_rsverr_mask : 1;
        uint64_t reserved_mask : 4;
        uint64_t reserved2 : 44;
#else
        uint64_t reserved2 : 44;
        uint64_t reserved_mask : 4;
        uint64_t axisnd_rsverr_mask : 1;
    uint64_t axsnd_rsverr_mask : 1;
        uint64_t axiflow_err_mask : 1;
    uint64_t axflow_err_mask : 1;
        uint64_t axipush_wrerr_mask : 1;
    uint64_t axpush_wrerr_mask : 1;
    uint64_t oci_bad_reg_addr_mask : 1;
        uint64_t internals_err_mask : 1;
    uint64_t bcde_oci_daterr_mask : 1;
    uint64_t bcde_setup_err_mask : 1;
    uint64_t bcue_oci_daterr_mask : 1;
    uint64_t bcue_setup_err_mask : 1;
    uint64_t reserved1 : 1;
    uint64_t oci_wrpar_err_mask : 1;
    uint64_t oci_slave_init_mask : 1;
    uint64_t oci_apar_err_mask : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_firmask_or_t;



typedef union pba_firact0
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t fir_action0 : 20;
        uint64_t reserved1 : 44;
#else
        uint64_t reserved1 : 44;
        uint64_t fir_action0 : 20;
#endif // _BIG_ENDIAN
    } fields;
} pba_firact0_t;



typedef union pba_firact1
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t fir_action1 : 20;
        uint64_t reserved1 : 44;
#else
        uint64_t reserved1 : 44;
        uint64_t fir_action1 : 20;
#endif // _BIG_ENDIAN
    } fields;
} pba_firact1_t;



typedef union pba_occact
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t occ_action_set : 20;
        uint64_t reserved1 : 44;
#else
        uint64_t reserved1 : 44;
        uint64_t occ_action_set : 20;
#endif // _BIG_ENDIAN
    } fields;
} pba_occact_t;



typedef union pba_ocfg
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t chsw_hang_on_adrerror : 1;
    uint64_t chsw_dis_ociabuspar_check : 1;
    uint64_t chsw_dis_ocibepar_check : 1;
    uint64_t chsw_hang_on_derror : 1;
    uint64_t chsw_dis_write_match_rearb : 1;
    uint64_t chsw_dis_ocidatapar_gen : 1;
    uint64_t chsw_dis_ocidatapar_check : 1;
        uint64_t chsw_use_topology_id_scope : 1;
        uint64_t reserved1 : 8;
        uint64_t reserved2 : 48;
#else
        uint64_t reserved2 : 48;
        uint64_t reserved1 : 8;
        uint64_t chsw_use_topology_id_scope : 1;
    uint64_t chsw_dis_ocidatapar_check : 1;
    uint64_t chsw_dis_ocidatapar_gen : 1;
    uint64_t chsw_dis_write_match_rearb : 1;
    uint64_t chsw_hang_on_derror : 1;
    uint64_t chsw_dis_ocibepar_check : 1;
    uint64_t chsw_dis_ociabuspar_check : 1;
    uint64_t chsw_hang_on_adrerror : 1;
#endif // _BIG_ENDIAN
    } fields;
} pba_ocfg_t;



typedef union pba_errrpt0
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t reserved1 : 64;
#else
        uint64_t reserved1 : 64;
#endif // _BIG_ENDIAN
    } fields;
} pba_errrpt0_t;



typedef union pba_errrpt1
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t reserved1 : 6;
    uint64_t cerr_bcde_setup_err : 2;
    uint64_t cerr_bcue_setup_err : 2;
    uint64_t cerr_bcue_oci_dataerr : 2;
        uint64_t reserved2 : 52;
#else
        uint64_t reserved2 : 52;
    uint64_t cerr_bcue_oci_dataerr : 2;
    uint64_t cerr_bcue_setup_err : 2;
    uint64_t cerr_bcde_setup_err : 2;
    uint64_t reserved1 : 6;
#endif // _BIG_ENDIAN
    } fields;
} pba_errrpt1_t;



typedef union pba_errrpt2
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t cerr_slv_internal_err : 8;
    uint64_t cerr_bcde_internal_err : 4;
    uint64_t cerr_bcue_internal_err : 4;
        uint64_t cerr_axflow_err : 3;
    uint64_t cerr_axpush_wrerr : 2;
        uint64_t reserved1 : 43;
#else
        uint64_t reserved1 : 43;
    uint64_t cerr_axpush_wrerr : 2;
        uint64_t cerr_axflow_err : 3;
    uint64_t cerr_bcue_internal_err : 4;
    uint64_t cerr_bcde_internal_err : 4;
    uint64_t cerr_slv_internal_err : 8;
#endif // _BIG_ENDIAN
    } fields;
} pba_errrpt2_t;



typedef union pba_rbufvaln
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t rd_slvnum : 2;
    uint64_t cur_rd_addr : 23;
    uint64_t reserved1 : 3;
    uint64_t prefetch : 1;
    uint64_t reserved2 : 2;
    uint64_t abort : 1;
    uint64_t reserved3 : 1;
    uint64_t buffer_status : 7;
    uint64_t reserved4 : 1;
    uint64_t masterid : 3;
    uint64_t reserved5 : 20;
#else
    uint64_t reserved5 : 20;
    uint64_t masterid : 3;
    uint64_t reserved4 : 1;
    uint64_t buffer_status : 7;
    uint64_t reserved3 : 1;
    uint64_t abort : 1;
    uint64_t reserved2 : 2;
    uint64_t prefetch : 1;
    uint64_t reserved1 : 3;
    uint64_t cur_rd_addr : 23;
    uint64_t rd_slvnum : 2;
#endif // _BIG_ENDIAN
    } fields;
} pba_rbufvaln_t;



typedef union pba_wbufvaln
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t wr_slvnum : 2;
    uint64_t start_wr_addr : 30;
    uint64_t reserved1 : 3;
    uint64_t wr_buffer_status : 5;
    uint64_t reserved2 : 1;
    uint64_t wr_byte_count : 7;
    uint64_t reserved3 : 16;
#else
    uint64_t reserved3 : 16;
    uint64_t wr_byte_count : 7;
    uint64_t reserved2 : 1;
    uint64_t wr_buffer_status : 5;
    uint64_t reserved1 : 3;
    uint64_t start_wr_addr : 30;
    uint64_t wr_slvnum : 2;
#endif // _BIG_ENDIAN
    } fields;
} pba_wbufvaln_t;



typedef union pba_barn
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t cmd_scope : 3;
    uint64_t reserved1 : 1;
    uint64_t reserved2 : 4;
    uint64_t addr : 36;
    uint64_t reserved3 : 4;
    uint64_t vtarget : 16;
#else
    uint64_t vtarget : 16;
    uint64_t reserved3 : 4;
    uint64_t addr : 36;
    uint64_t reserved2 : 4;
    uint64_t reserved1 : 1;
    uint64_t cmd_scope : 3;
#endif // _BIG_ENDIAN
    } fields;
} pba_barn_t;



typedef union pba_barmskn
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
    uint64_t reserved1 : 23;
    uint64_t mask : 21;
    uint64_t reserved2 : 20;
#else
    uint64_t reserved2 : 20;
    uint64_t mask : 21;
    uint64_t reserved1 : 23;
#endif // _BIG_ENDIAN
    } fields;
} pba_barmskn_t;



typedef union pba_pbtxtn
{

    uint64_t value;
    struct
    {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct
    {
#ifdef _BIG_ENDIAN
        uint64_t entry_valid : 8;
        uint64_t entry0 : 4;
        uint64_t entry1 : 4;
        uint64_t entry2 : 4;
        uint64_t entry3 : 4;
        uint64_t entry4 : 4;
        uint64_t entry5 : 4;
        uint64_t entry6 : 4;
        uint64_t entry7 : 4;
        uint64_t reserved1 : 24;
#else
        uint64_t reserved1 : 24;
        uint64_t entry7 : 4;
        uint64_t entry6 : 4;
        uint64_t entry5 : 4;
        uint64_t entry4 : 4;
        uint64_t entry3 : 4;
        uint64_t entry2 : 4;
        uint64_t entry1 : 4;
        uint64_t entry0 : 4;
        uint64_t entry_valid : 8;
#endif // _BIG_ENDIAN
    } fields;
} pba_pbtxtn_t;


#endif // __ASSEMBLER__
#endif // __PBA_FIRMWARE_REGISTERS_H__

