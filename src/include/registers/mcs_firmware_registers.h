/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: include/registers/mcs_firmware_registers.h $                  */
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
#if !defined(__MCS_FIRWARE_REGISTERS_H__)
#define __MCS_FIRWARE_REGISTERS_H__

typedef union
{
    uint64_t value;
    struct
    {
        uint64_t cfg_valid              :  1;
        uint64_t mmio_valid             :  1;
        uint64_t cfg_group_base_addr    : 31;
        uint64_t mmio_group_base_addr   : 31;
    } fields;
} mcfgpr_t;


typedef union mcmcicfg
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
        uint64_t dontCare0 : 47;
        uint64_t disable_channel_fail : 1;
        uint64_t dontCare1 : 16;
#else
        uint64_t dontcare1 : 16;
        uint64_t disable_channel_fail ; 1;
        uint64_t dontCare0 : 47;
#endif
    } fields;
} mcmcicfg_t;



typedef union mcchifir
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
        uint64_t fir_scom_wr_perr : 1;
        uint64_t fir_scom_cfg_perr : 1;
        uint64_t fir_dsrc_no_forward_progress : 1;
        uint64_t fir_dsrc_perf_degrad : 1;
        uint64_t fir_dmi_channel_fail : 1;
        uint64_t fir_channel_init_timeout : 1;
        uint64_t fir_channel_interlock_err : 1;
        uint64_t dontCare0 : 5;
        uint64_t fir_replay_buffer_ue : 1;
        uint64_t dontCare1 : 1;
        uint64_t fir_replay_buffer_overrun : 1;
        uint64_t fir_df_sm_perr : 1;
        uint64_t fir_cen_checkstop : 1;
        uint64_t dontCare2 : 15;
        uint64_t fir_dsff_tag_overrun : 1;
        uint64_t dontCare3 : 7;
        uint64_t fir_dsff_mca_async_cmd_error : 2;
        uint64_t fir_dsff_seq_error : 1;
        uint64_t dontCare4 : 18;
        uint64_t fir_dsff_timeout : 1;
        uint64_t dontCare5 : 2;
#else
        uint64_t dontCare  : 64;
#endif // _BIG_ENDIAN
    } fields;
} mcchifir_t;

#endif
