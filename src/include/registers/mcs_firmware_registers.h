/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: chips/p9/common/pmlib/include/registers/mcs_firmware_registers.h $ */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* EKB Project                                                            */
/*                                                                        */
/* COPYRIGHT 2017                                                         */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* The source code for this program is not published or otherwise         */
/* divested of its trade secrets, irrespective of what has been           */
/* deposited with the U.S. Copyright Office.                              */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
#if !defined(__MCS_FIRWARE_REGISTERS_H__)
#define __MCS_FIRWARE_REGISTERS_H__

typedef union mcfgpr
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
        uint64_t mcfgprq_valid : 1;
        uint64_t reserved0 : 2;
        uint64_t disable_extended_bar : 1 ; // low = P9 mode
        uint64_t mcfgprq_base_address : 31;
        uint64_t _reserved0 : 29;
#else
        uint64_t _reserved0 : 29;
        uint64_t mcfgprq_base_address : 31;
        uint64_t disable_extended_bar : 1 ; // low = p9 mode
        uint64_t reserved0 : 2;
        uint64_t mcfgprq_valid : 1;
#endif // _BIG_ENDIAN
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
