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


typedef union mcsmode0
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
        uint64_t reserved0 : 64;
#else
        uint64_t reserved0 : 64;
#endif
    } fields;
} mcsmode0_t;



typedef union mcifir
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
        uint64_t reserved0 : 64;
#else
        uint64_t reserved0 : 64;
#endif // _BIG_ENDIAN
    } fields;
} mcifir_t;

#endif
