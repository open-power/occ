#ifndef __OCB_FIRMWARE_REGISTERS_H__
#define __OCB_FIRMWARE_REGISTERS_H__

// $Id: ocb_firmware_registers.h,v 1.2 2014/03/14 15:33:03 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/registers/ocb_firmware_registers.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2014
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file ocb_firmware_registers.h
/// \brief C register structs for the OCB unit

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




typedef union ocb_oitr0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oitr0_t;



typedef union ocb_oiepr0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oiepr0_t;



typedef union ocb_ocir0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocir0_t;



typedef union ocb_onisr0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_onisr0_t;



typedef union ocb_ouder0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ouder0_t;



typedef union ocb_ocisr0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocisr0_t;



typedef union ocb_odher0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t dbg_halt_en : 32;
#else
    uint32_t dbg_halt_en : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_odher0_t;



typedef union ocb_oisr0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t debugger : 1;
    uint32_t trace_trigger : 1;
    uint32_t reserved_2 : 1;
    uint32_t pba_error : 1;
    uint32_t srt_error : 1;
    uint32_t pore_sw_error : 1;
    uint32_t pore_gpe0_fatal_error : 1;
    uint32_t pore_gpe1_fatal_error : 1;
    uint32_t pore_sbe_fatal_error : 1;
    uint32_t pmc_error : 1;
    uint32_t ocb_error : 1;
    uint32_t spipss_error : 1;
    uint32_t check_stop : 1;
    uint32_t pmc_malf_alert : 1;
    uint32_t adu_malf_alert : 1;
    uint32_t external_trap : 1;
    uint32_t occ_timer0 : 1;
    uint32_t occ_timer1 : 1;
    uint32_t pore_gpe0_error : 1;
    uint32_t pore_gpe1_error : 1;
    uint32_t pore_sbe_error : 1;
    uint32_t pmc_interchip_msg_recv : 1;
    uint32_t reserved_22 : 1;
    uint32_t pore_gpe0_complete : 1;
    uint32_t pore_gpe1_complete : 1;
    uint32_t adcfsm_ongoing : 1;
    uint32_t reserved_26 : 1;
    uint32_t pba_occ_push0 : 1;
    uint32_t pba_occ_push1 : 1;
    uint32_t pba_bcde_attn : 1;
    uint32_t pba_bcue_attn : 1;
    uint32_t reserved_31 : 1;
#else
    uint32_t reserved_31 : 1;
    uint32_t pba_bcue_attn : 1;
    uint32_t pba_bcde_attn : 1;
    uint32_t pba_occ_push1 : 1;
    uint32_t pba_occ_push0 : 1;
    uint32_t reserved_26 : 1;
    uint32_t adcfsm_ongoing : 1;
    uint32_t pore_gpe1_complete : 1;
    uint32_t pore_gpe0_complete : 1;
    uint32_t reserved_22 : 1;
    uint32_t pmc_interchip_msg_recv : 1;
    uint32_t pore_sbe_error : 1;
    uint32_t pore_gpe1_error : 1;
    uint32_t pore_gpe0_error : 1;
    uint32_t occ_timer1 : 1;
    uint32_t occ_timer0 : 1;
    uint32_t external_trap : 1;
    uint32_t adu_malf_alert : 1;
    uint32_t pmc_malf_alert : 1;
    uint32_t check_stop : 1;
    uint32_t spipss_error : 1;
    uint32_t ocb_error : 1;
    uint32_t pmc_error : 1;
    uint32_t pore_sbe_fatal_error : 1;
    uint32_t pore_gpe1_fatal_error : 1;
    uint32_t pore_gpe0_fatal_error : 1;
    uint32_t pore_sw_error : 1;
    uint32_t srt_error : 1;
    uint32_t pba_error : 1;
    uint32_t reserved_2 : 1;
    uint32_t trace_trigger : 1;
    uint32_t debugger : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oisr0_t;



typedef union ocb_oisr0_and {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t debugger : 1;
    uint32_t trace_trigger : 1;
    uint32_t reserved_2 : 1;
    uint32_t pba_error : 1;
    uint32_t srt_error : 1;
    uint32_t pore_sw_error : 1;
    uint32_t pore_gpe0_fatal_error : 1;
    uint32_t pore_gpe1_fatal_error : 1;
    uint32_t pore_sbe_fatal_error : 1;
    uint32_t pmc_error : 1;
    uint32_t ocb_error : 1;
    uint32_t spipss_error : 1;
    uint32_t check_stop : 1;
    uint32_t pmc_malf_alert : 1;
    uint32_t adu_malf_alert : 1;
    uint32_t external_trap : 1;
    uint32_t occ_timer0 : 1;
    uint32_t occ_timer1 : 1;
    uint32_t pore_gpe0_error : 1;
    uint32_t pore_gpe1_error : 1;
    uint32_t pore_sbe_error : 1;
    uint32_t pmc_interchip_msg_recv : 1;
    uint32_t reserved_22 : 1;
    uint32_t pore_gpe0_complete : 1;
    uint32_t pore_gpe1_complete : 1;
    uint32_t adcfsm_ongoing : 1;
    uint32_t reserved_26 : 1;
    uint32_t pba_occ_push0 : 1;
    uint32_t pba_occ_push1 : 1;
    uint32_t pba_bcde_attn : 1;
    uint32_t pba_bcue_attn : 1;
    uint32_t reserved_31 : 1;
#else
    uint32_t reserved_31 : 1;
    uint32_t pba_bcue_attn : 1;
    uint32_t pba_bcde_attn : 1;
    uint32_t pba_occ_push1 : 1;
    uint32_t pba_occ_push0 : 1;
    uint32_t reserved_26 : 1;
    uint32_t adcfsm_ongoing : 1;
    uint32_t pore_gpe1_complete : 1;
    uint32_t pore_gpe0_complete : 1;
    uint32_t reserved_22 : 1;
    uint32_t pmc_interchip_msg_recv : 1;
    uint32_t pore_sbe_error : 1;
    uint32_t pore_gpe1_error : 1;
    uint32_t pore_gpe0_error : 1;
    uint32_t occ_timer1 : 1;
    uint32_t occ_timer0 : 1;
    uint32_t external_trap : 1;
    uint32_t adu_malf_alert : 1;
    uint32_t pmc_malf_alert : 1;
    uint32_t check_stop : 1;
    uint32_t spipss_error : 1;
    uint32_t ocb_error : 1;
    uint32_t pmc_error : 1;
    uint32_t pore_sbe_fatal_error : 1;
    uint32_t pore_gpe1_fatal_error : 1;
    uint32_t pore_gpe0_fatal_error : 1;
    uint32_t pore_sw_error : 1;
    uint32_t srt_error : 1;
    uint32_t pba_error : 1;
    uint32_t reserved_2 : 1;
    uint32_t trace_trigger : 1;
    uint32_t debugger : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oisr0_and_t;



typedef union ocb_oisr0_or {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t debugger : 1;
    uint32_t trace_trigger : 1;
    uint32_t reserved_2 : 1;
    uint32_t pba_error : 1;
    uint32_t srt_error : 1;
    uint32_t pore_sw_error : 1;
    uint32_t pore_gpe0_fatal_error : 1;
    uint32_t pore_gpe1_fatal_error : 1;
    uint32_t pore_sbe_fatal_error : 1;
    uint32_t pmc_error : 1;
    uint32_t ocb_error : 1;
    uint32_t spipss_error : 1;
    uint32_t check_stop : 1;
    uint32_t pmc_malf_alert : 1;
    uint32_t adu_malf_alert : 1;
    uint32_t external_trap : 1;
    uint32_t occ_timer0 : 1;
    uint32_t occ_timer1 : 1;
    uint32_t pore_gpe0_error : 1;
    uint32_t pore_gpe1_error : 1;
    uint32_t pore_sbe_error : 1;
    uint32_t pmc_interchip_msg_recv : 1;
    uint32_t reserved_22 : 1;
    uint32_t pore_gpe0_complete : 1;
    uint32_t pore_gpe1_complete : 1;
    uint32_t adcfsm_ongoing : 1;
    uint32_t reserved_26 : 1;
    uint32_t pba_occ_push0 : 1;
    uint32_t pba_occ_push1 : 1;
    uint32_t pba_bcde_attn : 1;
    uint32_t pba_bcue_attn : 1;
    uint32_t reserved_31 : 1;
#else
    uint32_t reserved_31 : 1;
    uint32_t pba_bcue_attn : 1;
    uint32_t pba_bcde_attn : 1;
    uint32_t pba_occ_push1 : 1;
    uint32_t pba_occ_push0 : 1;
    uint32_t reserved_26 : 1;
    uint32_t adcfsm_ongoing : 1;
    uint32_t pore_gpe1_complete : 1;
    uint32_t pore_gpe0_complete : 1;
    uint32_t reserved_22 : 1;
    uint32_t pmc_interchip_msg_recv : 1;
    uint32_t pore_sbe_error : 1;
    uint32_t pore_gpe1_error : 1;
    uint32_t pore_gpe0_error : 1;
    uint32_t occ_timer1 : 1;
    uint32_t occ_timer0 : 1;
    uint32_t external_trap : 1;
    uint32_t adu_malf_alert : 1;
    uint32_t pmc_malf_alert : 1;
    uint32_t check_stop : 1;
    uint32_t spipss_error : 1;
    uint32_t ocb_error : 1;
    uint32_t pmc_error : 1;
    uint32_t pore_sbe_fatal_error : 1;
    uint32_t pore_gpe1_fatal_error : 1;
    uint32_t pore_gpe0_fatal_error : 1;
    uint32_t pore_sw_error : 1;
    uint32_t srt_error : 1;
    uint32_t pba_error : 1;
    uint32_t reserved_2 : 1;
    uint32_t trace_trigger : 1;
    uint32_t debugger : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oisr0_or_t;



typedef union ocb_oimr0 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t debugger : 1;
    uint32_t trace_trigger : 1;
    uint32_t reserved_2 : 1;
    uint32_t pba_error : 1;
    uint32_t srt_error : 1;
    uint32_t pore_sw_error : 1;
    uint32_t pore_gpe0_fatal_error : 1;
    uint32_t pore_gpe1_fatal_error : 1;
    uint32_t pore_sbe_fatal_error : 1;
    uint32_t pmc_error : 1;
    uint32_t ocb_error : 1;
    uint32_t spipss_error : 1;
    uint32_t check_stop : 1;
    uint32_t pmc_malf_alert : 1;
    uint32_t adu_malf_alert : 1;
    uint32_t external_trap : 1;
    uint32_t occ_timer0 : 1;
    uint32_t occ_timer1 : 1;
    uint32_t pore_gpe0_error : 1;
    uint32_t pore_gpe1_error : 1;
    uint32_t pore_sbe_error : 1;
    uint32_t pmc_interchip_msg_recv : 1;
    uint32_t reserved_22 : 1;
    uint32_t pore_gpe0_complete : 1;
    uint32_t pore_gpe1_complete : 1;
    uint32_t adcfsm_ongoing : 1;
    uint32_t reserved_26 : 1;
    uint32_t pba_occ_push0 : 1;
    uint32_t pba_occ_push1 : 1;
    uint32_t pba_bcde_attn : 1;
    uint32_t pba_bcue_attn : 1;
    uint32_t reserved_31 : 1;
#else
    uint32_t reserved_31 : 1;
    uint32_t pba_bcue_attn : 1;
    uint32_t pba_bcde_attn : 1;
    uint32_t pba_occ_push1 : 1;
    uint32_t pba_occ_push0 : 1;
    uint32_t reserved_26 : 1;
    uint32_t adcfsm_ongoing : 1;
    uint32_t pore_gpe1_complete : 1;
    uint32_t pore_gpe0_complete : 1;
    uint32_t reserved_22 : 1;
    uint32_t pmc_interchip_msg_recv : 1;
    uint32_t pore_sbe_error : 1;
    uint32_t pore_gpe1_error : 1;
    uint32_t pore_gpe0_error : 1;
    uint32_t occ_timer1 : 1;
    uint32_t occ_timer0 : 1;
    uint32_t external_trap : 1;
    uint32_t adu_malf_alert : 1;
    uint32_t pmc_malf_alert : 1;
    uint32_t check_stop : 1;
    uint32_t spipss_error : 1;
    uint32_t ocb_error : 1;
    uint32_t pmc_error : 1;
    uint32_t pore_sbe_fatal_error : 1;
    uint32_t pore_gpe1_fatal_error : 1;
    uint32_t pore_gpe0_fatal_error : 1;
    uint32_t pore_sw_error : 1;
    uint32_t srt_error : 1;
    uint32_t pba_error : 1;
    uint32_t reserved_2 : 1;
    uint32_t trace_trigger : 1;
    uint32_t debugger : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oimr0_t;



typedef union ocb_oimr0_and {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t debugger : 1;
    uint32_t trace_trigger : 1;
    uint32_t reserved_2 : 1;
    uint32_t pba_error : 1;
    uint32_t srt_error : 1;
    uint32_t pore_sw_error : 1;
    uint32_t pore_gpe0_fatal_error : 1;
    uint32_t pore_gpe1_fatal_error : 1;
    uint32_t pore_sbe_fatal_error : 1;
    uint32_t pmc_error : 1;
    uint32_t ocb_error : 1;
    uint32_t spipss_error : 1;
    uint32_t check_stop : 1;
    uint32_t pmc_malf_alert : 1;
    uint32_t adu_malf_alert : 1;
    uint32_t external_trap : 1;
    uint32_t occ_timer0 : 1;
    uint32_t occ_timer1 : 1;
    uint32_t pore_gpe0_error : 1;
    uint32_t pore_gpe1_error : 1;
    uint32_t pore_sbe_error : 1;
    uint32_t pmc_interchip_msg_recv : 1;
    uint32_t reserved_22 : 1;
    uint32_t pore_gpe0_complete : 1;
    uint32_t pore_gpe1_complete : 1;
    uint32_t adcfsm_ongoing : 1;
    uint32_t reserved_26 : 1;
    uint32_t pba_occ_push0 : 1;
    uint32_t pba_occ_push1 : 1;
    uint32_t pba_bcde_attn : 1;
    uint32_t pba_bcue_attn : 1;
    uint32_t reserved_31 : 1;
#else
    uint32_t reserved_31 : 1;
    uint32_t pba_bcue_attn : 1;
    uint32_t pba_bcde_attn : 1;
    uint32_t pba_occ_push1 : 1;
    uint32_t pba_occ_push0 : 1;
    uint32_t reserved_26 : 1;
    uint32_t adcfsm_ongoing : 1;
    uint32_t pore_gpe1_complete : 1;
    uint32_t pore_gpe0_complete : 1;
    uint32_t reserved_22 : 1;
    uint32_t pmc_interchip_msg_recv : 1;
    uint32_t pore_sbe_error : 1;
    uint32_t pore_gpe1_error : 1;
    uint32_t pore_gpe0_error : 1;
    uint32_t occ_timer1 : 1;
    uint32_t occ_timer0 : 1;
    uint32_t external_trap : 1;
    uint32_t adu_malf_alert : 1;
    uint32_t pmc_malf_alert : 1;
    uint32_t check_stop : 1;
    uint32_t spipss_error : 1;
    uint32_t ocb_error : 1;
    uint32_t pmc_error : 1;
    uint32_t pore_sbe_fatal_error : 1;
    uint32_t pore_gpe1_fatal_error : 1;
    uint32_t pore_gpe0_fatal_error : 1;
    uint32_t pore_sw_error : 1;
    uint32_t srt_error : 1;
    uint32_t pba_error : 1;
    uint32_t reserved_2 : 1;
    uint32_t trace_trigger : 1;
    uint32_t debugger : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oimr0_and_t;



typedef union ocb_oimr0_or {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t debugger : 1;
    uint32_t trace_trigger : 1;
    uint32_t reserved_2 : 1;
    uint32_t pba_error : 1;
    uint32_t srt_error : 1;
    uint32_t pore_sw_error : 1;
    uint32_t pore_gpe0_fatal_error : 1;
    uint32_t pore_gpe1_fatal_error : 1;
    uint32_t pore_sbe_fatal_error : 1;
    uint32_t pmc_error : 1;
    uint32_t ocb_error : 1;
    uint32_t spipss_error : 1;
    uint32_t check_stop : 1;
    uint32_t pmc_malf_alert : 1;
    uint32_t adu_malf_alert : 1;
    uint32_t external_trap : 1;
    uint32_t occ_timer0 : 1;
    uint32_t occ_timer1 : 1;
    uint32_t pore_gpe0_error : 1;
    uint32_t pore_gpe1_error : 1;
    uint32_t pore_sbe_error : 1;
    uint32_t pmc_interchip_msg_recv : 1;
    uint32_t reserved_22 : 1;
    uint32_t pore_gpe0_complete : 1;
    uint32_t pore_gpe1_complete : 1;
    uint32_t adcfsm_ongoing : 1;
    uint32_t reserved_26 : 1;
    uint32_t pba_occ_push0 : 1;
    uint32_t pba_occ_push1 : 1;
    uint32_t pba_bcde_attn : 1;
    uint32_t pba_bcue_attn : 1;
    uint32_t reserved_31 : 1;
#else
    uint32_t reserved_31 : 1;
    uint32_t pba_bcue_attn : 1;
    uint32_t pba_bcde_attn : 1;
    uint32_t pba_occ_push1 : 1;
    uint32_t pba_occ_push0 : 1;
    uint32_t reserved_26 : 1;
    uint32_t adcfsm_ongoing : 1;
    uint32_t pore_gpe1_complete : 1;
    uint32_t pore_gpe0_complete : 1;
    uint32_t reserved_22 : 1;
    uint32_t pmc_interchip_msg_recv : 1;
    uint32_t pore_sbe_error : 1;
    uint32_t pore_gpe1_error : 1;
    uint32_t pore_gpe0_error : 1;
    uint32_t occ_timer1 : 1;
    uint32_t occ_timer0 : 1;
    uint32_t external_trap : 1;
    uint32_t adu_malf_alert : 1;
    uint32_t pmc_malf_alert : 1;
    uint32_t check_stop : 1;
    uint32_t spipss_error : 1;
    uint32_t ocb_error : 1;
    uint32_t pmc_error : 1;
    uint32_t pore_sbe_fatal_error : 1;
    uint32_t pore_gpe1_fatal_error : 1;
    uint32_t pore_gpe0_fatal_error : 1;
    uint32_t pore_sw_error : 1;
    uint32_t srt_error : 1;
    uint32_t pba_error : 1;
    uint32_t reserved_2 : 1;
    uint32_t trace_trigger : 1;
    uint32_t debugger : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oimr0_or_t;



typedef union ocb_oitr1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oitr1_t;



typedef union ocb_oiepr1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oiepr1_t;



typedef union ocb_ocir1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocir1_t;



typedef union ocb_onisr1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_onisr1_t;



typedef union ocb_ouder1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ouder1_t;



typedef union ocb_ocisr1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t value : 32;
#else
    uint32_t value : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocisr1_t;



typedef union ocb_odher1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t dbg_halt_en : 32;
#else
    uint32_t dbg_halt_en : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_odher1_t;



typedef union ocb_oisr1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t reserved_32 : 1;
    uint32_t reserved_33 : 1;
    uint32_t occ_strm0_pull : 1;
    uint32_t occ_strm0_push : 1;
    uint32_t occ_strm1_pull : 1;
    uint32_t occ_strm1_push : 1;
    uint32_t occ_strm2_pull : 1;
    uint32_t occ_strm2_push : 1;
    uint32_t occ_strm3_pull : 1;
    uint32_t occ_strm3_push : 1;
    uint32_t reserved_42 : 1;
    uint32_t reserved_43 : 1;
    uint32_t pmc_voltage_change_ongoing : 1;
    uint32_t pmc_protocol_ongoing : 1;
    uint32_t pmc_sync : 1;
    uint32_t pmc_pstate_change : 1;
    uint32_t reserved_48 : 1;
    uint32_t reserved_49 : 1;
    uint32_t pmc_idle_exit : 1;
    uint32_t pore_sw_complete : 1;
    uint32_t pmc_idle_enter : 1;
    uint32_t reserved_53 : 1;
    uint32_t pmc_interchip_msg_send_ongoing : 1;
    uint32_t oci2spivid_ongoing : 1;
    uint32_t pmc_ocb_o2p_ongoing : 1;
    uint32_t pssbridge_ongoing : 1;
    uint32_t pore_sbe_complete : 1;
    uint32_t ipi0 : 1;
    uint32_t ipi1 : 1;
    uint32_t ipi2 : 1;
    uint32_t ipi3 : 1;
    uint32_t reserved_63 : 1;
#else
    uint32_t reserved_63 : 1;
    uint32_t ipi3 : 1;
    uint32_t ipi2 : 1;
    uint32_t ipi1 : 1;
    uint32_t ipi0 : 1;
    uint32_t pore_sbe_complete : 1;
    uint32_t pssbridge_ongoing : 1;
    uint32_t pmc_ocb_o2p_ongoing : 1;
    uint32_t oci2spivid_ongoing : 1;
    uint32_t pmc_interchip_msg_send_ongoing : 1;
    uint32_t reserved_53 : 1;
    uint32_t pmc_idle_enter : 1;
    uint32_t pore_sw_complete : 1;
    uint32_t pmc_idle_exit : 1;
    uint32_t reserved_49 : 1;
    uint32_t reserved_48 : 1;
    uint32_t pmc_pstate_change : 1;
    uint32_t pmc_sync : 1;
    uint32_t pmc_protocol_ongoing : 1;
    uint32_t pmc_voltage_change_ongoing : 1;
    uint32_t reserved_43 : 1;
    uint32_t reserved_42 : 1;
    uint32_t occ_strm3_push : 1;
    uint32_t occ_strm3_pull : 1;
    uint32_t occ_strm2_push : 1;
    uint32_t occ_strm2_pull : 1;
    uint32_t occ_strm1_push : 1;
    uint32_t occ_strm1_pull : 1;
    uint32_t occ_strm0_push : 1;
    uint32_t occ_strm0_pull : 1;
    uint32_t reserved_33 : 1;
    uint32_t reserved_32 : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oisr1_t;



typedef union ocb_oisr1_and {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t reserved_32 : 1;
    uint32_t reserved_33 : 1;
    uint32_t occ_strm0_pull : 1;
    uint32_t occ_strm0_push : 1;
    uint32_t occ_strm1_pull : 1;
    uint32_t occ_strm1_push : 1;
    uint32_t occ_strm2_pull : 1;
    uint32_t occ_strm2_push : 1;
    uint32_t occ_strm3_pull : 1;
    uint32_t occ_strm3_push : 1;
    uint32_t reserved_42 : 1;
    uint32_t reserved_43 : 1;
    uint32_t pmc_voltage_change_ongoing : 1;
    uint32_t pmc_protocol_ongoing : 1;
    uint32_t pmc_sync : 1;
    uint32_t pmc_pstate_change : 1;
    uint32_t reserved_48 : 1;
    uint32_t reserved_49 : 1;
    uint32_t pmc_idle_exit : 1;
    uint32_t pore_sw_complete : 1;
    uint32_t pmc_idle_enter : 1;
    uint32_t reserved_53 : 1;
    uint32_t pmc_interchip_msg_send_ongoing : 1;
    uint32_t oci2spivid_ongoing : 1;
    uint32_t pmc_ocb_o2p_ongoing : 1;
    uint32_t pssbridge_ongoing : 1;
    uint32_t pore_sbe_complete : 1;
    uint32_t ipi0 : 1;
    uint32_t ipi1 : 1;
    uint32_t ipi2 : 1;
    uint32_t ipi3 : 1;
    uint32_t reserved_63 : 1;
#else
    uint32_t reserved_63 : 1;
    uint32_t ipi3 : 1;
    uint32_t ipi2 : 1;
    uint32_t ipi1 : 1;
    uint32_t ipi0 : 1;
    uint32_t pore_sbe_complete : 1;
    uint32_t pssbridge_ongoing : 1;
    uint32_t pmc_ocb_o2p_ongoing : 1;
    uint32_t oci2spivid_ongoing : 1;
    uint32_t pmc_interchip_msg_send_ongoing : 1;
    uint32_t reserved_53 : 1;
    uint32_t pmc_idle_enter : 1;
    uint32_t pore_sw_complete : 1;
    uint32_t pmc_idle_exit : 1;
    uint32_t reserved_49 : 1;
    uint32_t reserved_48 : 1;
    uint32_t pmc_pstate_change : 1;
    uint32_t pmc_sync : 1;
    uint32_t pmc_protocol_ongoing : 1;
    uint32_t pmc_voltage_change_ongoing : 1;
    uint32_t reserved_43 : 1;
    uint32_t reserved_42 : 1;
    uint32_t occ_strm3_push : 1;
    uint32_t occ_strm3_pull : 1;
    uint32_t occ_strm2_push : 1;
    uint32_t occ_strm2_pull : 1;
    uint32_t occ_strm1_push : 1;
    uint32_t occ_strm1_pull : 1;
    uint32_t occ_strm0_push : 1;
    uint32_t occ_strm0_pull : 1;
    uint32_t reserved_33 : 1;
    uint32_t reserved_32 : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oisr1_and_t;



typedef union ocb_oisr1_or {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t reserved_32 : 1;
    uint32_t reserved_33 : 1;
    uint32_t occ_strm0_pull : 1;
    uint32_t occ_strm0_push : 1;
    uint32_t occ_strm1_pull : 1;
    uint32_t occ_strm1_push : 1;
    uint32_t occ_strm2_pull : 1;
    uint32_t occ_strm2_push : 1;
    uint32_t occ_strm3_pull : 1;
    uint32_t occ_strm3_push : 1;
    uint32_t reserved_42 : 1;
    uint32_t reserved_43 : 1;
    uint32_t pmc_voltage_change_ongoing : 1;
    uint32_t pmc_protocol_ongoing : 1;
    uint32_t pmc_sync : 1;
    uint32_t pmc_pstate_change : 1;
    uint32_t reserved_48 : 1;
    uint32_t reserved_49 : 1;
    uint32_t pmc_idle_exit : 1;
    uint32_t pore_sw_complete : 1;
    uint32_t pmc_idle_enter : 1;
    uint32_t reserved_53 : 1;
    uint32_t pmc_interchip_msg_send_ongoing : 1;
    uint32_t oci2spivid_ongoing : 1;
    uint32_t pmc_ocb_o2p_ongoing : 1;
    uint32_t pssbridge_ongoing : 1;
    uint32_t pore_sbe_complete : 1;
    uint32_t ipi0 : 1;
    uint32_t ipi1 : 1;
    uint32_t ipi2 : 1;
    uint32_t ipi3 : 1;
    uint32_t reserved_63 : 1;
#else
    uint32_t reserved_63 : 1;
    uint32_t ipi3 : 1;
    uint32_t ipi2 : 1;
    uint32_t ipi1 : 1;
    uint32_t ipi0 : 1;
    uint32_t pore_sbe_complete : 1;
    uint32_t pssbridge_ongoing : 1;
    uint32_t pmc_ocb_o2p_ongoing : 1;
    uint32_t oci2spivid_ongoing : 1;
    uint32_t pmc_interchip_msg_send_ongoing : 1;
    uint32_t reserved_53 : 1;
    uint32_t pmc_idle_enter : 1;
    uint32_t pore_sw_complete : 1;
    uint32_t pmc_idle_exit : 1;
    uint32_t reserved_49 : 1;
    uint32_t reserved_48 : 1;
    uint32_t pmc_pstate_change : 1;
    uint32_t pmc_sync : 1;
    uint32_t pmc_protocol_ongoing : 1;
    uint32_t pmc_voltage_change_ongoing : 1;
    uint32_t reserved_43 : 1;
    uint32_t reserved_42 : 1;
    uint32_t occ_strm3_push : 1;
    uint32_t occ_strm3_pull : 1;
    uint32_t occ_strm2_push : 1;
    uint32_t occ_strm2_pull : 1;
    uint32_t occ_strm1_push : 1;
    uint32_t occ_strm1_pull : 1;
    uint32_t occ_strm0_push : 1;
    uint32_t occ_strm0_pull : 1;
    uint32_t reserved_33 : 1;
    uint32_t reserved_32 : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oisr1_or_t;



typedef union ocb_oimr1 {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t reserved_32 : 1;
    uint32_t reserved_33 : 1;
    uint32_t occ_strm0_pull : 1;
    uint32_t occ_strm0_push : 1;
    uint32_t occ_strm1_pull : 1;
    uint32_t occ_strm1_push : 1;
    uint32_t occ_strm2_pull : 1;
    uint32_t occ_strm2_push : 1;
    uint32_t occ_strm3_pull : 1;
    uint32_t occ_strm3_push : 1;
    uint32_t reserved_42 : 1;
    uint32_t reserved_43 : 1;
    uint32_t pmc_voltage_change_ongoing : 1;
    uint32_t pmc_protocol_ongoing : 1;
    uint32_t pmc_sync : 1;
    uint32_t pmc_pstate_change : 1;
    uint32_t reserved_48 : 1;
    uint32_t reserved_49 : 1;
    uint32_t pmc_idle_exit : 1;
    uint32_t pore_sw_complete : 1;
    uint32_t pmc_idle_enter : 1;
    uint32_t reserved_53 : 1;
    uint32_t pmc_interchip_msg_send_ongoing : 1;
    uint32_t oci2spivid_ongoing : 1;
    uint32_t pmc_ocb_o2p_ongoing : 1;
    uint32_t pssbridge_ongoing : 1;
    uint32_t pore_sbe_complete : 1;
    uint32_t ipi0 : 1;
    uint32_t ipi1 : 1;
    uint32_t ipi2 : 1;
    uint32_t ipi3 : 1;
    uint32_t reserved_63 : 1;
#else
    uint32_t reserved_63 : 1;
    uint32_t ipi3 : 1;
    uint32_t ipi2 : 1;
    uint32_t ipi1 : 1;
    uint32_t ipi0 : 1;
    uint32_t pore_sbe_complete : 1;
    uint32_t pssbridge_ongoing : 1;
    uint32_t pmc_ocb_o2p_ongoing : 1;
    uint32_t oci2spivid_ongoing : 1;
    uint32_t pmc_interchip_msg_send_ongoing : 1;
    uint32_t reserved_53 : 1;
    uint32_t pmc_idle_enter : 1;
    uint32_t pore_sw_complete : 1;
    uint32_t pmc_idle_exit : 1;
    uint32_t reserved_49 : 1;
    uint32_t reserved_48 : 1;
    uint32_t pmc_pstate_change : 1;
    uint32_t pmc_sync : 1;
    uint32_t pmc_protocol_ongoing : 1;
    uint32_t pmc_voltage_change_ongoing : 1;
    uint32_t reserved_43 : 1;
    uint32_t reserved_42 : 1;
    uint32_t occ_strm3_push : 1;
    uint32_t occ_strm3_pull : 1;
    uint32_t occ_strm2_push : 1;
    uint32_t occ_strm2_pull : 1;
    uint32_t occ_strm1_push : 1;
    uint32_t occ_strm1_pull : 1;
    uint32_t occ_strm0_push : 1;
    uint32_t occ_strm0_pull : 1;
    uint32_t reserved_33 : 1;
    uint32_t reserved_32 : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oimr1_t;



typedef union ocb_oimr1_and {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t reserved_32 : 1;
    uint32_t reserved_33 : 1;
    uint32_t occ_strm0_pull : 1;
    uint32_t occ_strm0_push : 1;
    uint32_t occ_strm1_pull : 1;
    uint32_t occ_strm1_push : 1;
    uint32_t occ_strm2_pull : 1;
    uint32_t occ_strm2_push : 1;
    uint32_t occ_strm3_pull : 1;
    uint32_t occ_strm3_push : 1;
    uint32_t reserved_42 : 1;
    uint32_t reserved_43 : 1;
    uint32_t pmc_voltage_change_ongoing : 1;
    uint32_t pmc_protocol_ongoing : 1;
    uint32_t pmc_sync : 1;
    uint32_t pmc_pstate_change : 1;
    uint32_t reserved_48 : 1;
    uint32_t reserved_49 : 1;
    uint32_t pmc_idle_exit : 1;
    uint32_t pore_sw_complete : 1;
    uint32_t pmc_idle_enter : 1;
    uint32_t reserved_53 : 1;
    uint32_t pmc_interchip_msg_send_ongoing : 1;
    uint32_t oci2spivid_ongoing : 1;
    uint32_t pmc_ocb_o2p_ongoing : 1;
    uint32_t pssbridge_ongoing : 1;
    uint32_t pore_sbe_complete : 1;
    uint32_t ipi0 : 1;
    uint32_t ipi1 : 1;
    uint32_t ipi2 : 1;
    uint32_t ipi3 : 1;
    uint32_t reserved_63 : 1;
#else
    uint32_t reserved_63 : 1;
    uint32_t ipi3 : 1;
    uint32_t ipi2 : 1;
    uint32_t ipi1 : 1;
    uint32_t ipi0 : 1;
    uint32_t pore_sbe_complete : 1;
    uint32_t pssbridge_ongoing : 1;
    uint32_t pmc_ocb_o2p_ongoing : 1;
    uint32_t oci2spivid_ongoing : 1;
    uint32_t pmc_interchip_msg_send_ongoing : 1;
    uint32_t reserved_53 : 1;
    uint32_t pmc_idle_enter : 1;
    uint32_t pore_sw_complete : 1;
    uint32_t pmc_idle_exit : 1;
    uint32_t reserved_49 : 1;
    uint32_t reserved_48 : 1;
    uint32_t pmc_pstate_change : 1;
    uint32_t pmc_sync : 1;
    uint32_t pmc_protocol_ongoing : 1;
    uint32_t pmc_voltage_change_ongoing : 1;
    uint32_t reserved_43 : 1;
    uint32_t reserved_42 : 1;
    uint32_t occ_strm3_push : 1;
    uint32_t occ_strm3_pull : 1;
    uint32_t occ_strm2_push : 1;
    uint32_t occ_strm2_pull : 1;
    uint32_t occ_strm1_push : 1;
    uint32_t occ_strm1_pull : 1;
    uint32_t occ_strm0_push : 1;
    uint32_t occ_strm0_pull : 1;
    uint32_t reserved_33 : 1;
    uint32_t reserved_32 : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oimr1_and_t;



typedef union ocb_oimr1_or {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t reserved_32 : 1;
    uint32_t reserved_33 : 1;
    uint32_t occ_strm0_pull : 1;
    uint32_t occ_strm0_push : 1;
    uint32_t occ_strm1_pull : 1;
    uint32_t occ_strm1_push : 1;
    uint32_t occ_strm2_pull : 1;
    uint32_t occ_strm2_push : 1;
    uint32_t occ_strm3_pull : 1;
    uint32_t occ_strm3_push : 1;
    uint32_t reserved_42 : 1;
    uint32_t reserved_43 : 1;
    uint32_t pmc_voltage_change_ongoing : 1;
    uint32_t pmc_protocol_ongoing : 1;
    uint32_t pmc_sync : 1;
    uint32_t pmc_pstate_change : 1;
    uint32_t reserved_48 : 1;
    uint32_t reserved_49 : 1;
    uint32_t pmc_idle_exit : 1;
    uint32_t pore_sw_complete : 1;
    uint32_t pmc_idle_enter : 1;
    uint32_t reserved_53 : 1;
    uint32_t pmc_interchip_msg_send_ongoing : 1;
    uint32_t oci2spivid_ongoing : 1;
    uint32_t pmc_ocb_o2p_ongoing : 1;
    uint32_t pssbridge_ongoing : 1;
    uint32_t pore_sbe_complete : 1;
    uint32_t ipi0 : 1;
    uint32_t ipi1 : 1;
    uint32_t ipi2 : 1;
    uint32_t ipi3 : 1;
    uint32_t reserved_63 : 1;
#else
    uint32_t reserved_63 : 1;
    uint32_t ipi3 : 1;
    uint32_t ipi2 : 1;
    uint32_t ipi1 : 1;
    uint32_t ipi0 : 1;
    uint32_t pore_sbe_complete : 1;
    uint32_t pssbridge_ongoing : 1;
    uint32_t pmc_ocb_o2p_ongoing : 1;
    uint32_t oci2spivid_ongoing : 1;
    uint32_t pmc_interchip_msg_send_ongoing : 1;
    uint32_t reserved_53 : 1;
    uint32_t pmc_idle_enter : 1;
    uint32_t pore_sw_complete : 1;
    uint32_t pmc_idle_exit : 1;
    uint32_t reserved_49 : 1;
    uint32_t reserved_48 : 1;
    uint32_t pmc_pstate_change : 1;
    uint32_t pmc_sync : 1;
    uint32_t pmc_protocol_ongoing : 1;
    uint32_t pmc_voltage_change_ongoing : 1;
    uint32_t reserved_43 : 1;
    uint32_t reserved_42 : 1;
    uint32_t occ_strm3_push : 1;
    uint32_t occ_strm3_pull : 1;
    uint32_t occ_strm2_push : 1;
    uint32_t occ_strm2_pull : 1;
    uint32_t occ_strm1_push : 1;
    uint32_t occ_strm1_pull : 1;
    uint32_t occ_strm0_push : 1;
    uint32_t occ_strm0_pull : 1;
    uint32_t reserved_33 : 1;
    uint32_t reserved_32 : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oimr1_or_t;



typedef union ocb_occmisc {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t core_ext_intr : 1;
    uint32_t reason_intr : 1;
    uint32_t _reserved0 : 30;
#else
    uint32_t _reserved0 : 30;
    uint32_t reason_intr : 1;
    uint32_t core_ext_intr : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_occmisc_t;



typedef union ocb_occmisc_and {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t core_ext_intr : 1;
    uint32_t _reserved0 : 31;
#else
    uint32_t _reserved0 : 31;
    uint32_t core_ext_intr : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_occmisc_and_t;



typedef union ocb_occmisc_or {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t core_ext_intr : 1;
    uint32_t _reserved0 : 31;
#else
    uint32_t _reserved0 : 31;
    uint32_t core_ext_intr : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_occmisc_or_t;



typedef union ocb_otrn {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t timeout : 1;
    uint32_t control : 1;
    uint32_t auto_reload : 1;
    uint32_t reserved : 13;
    uint32_t timer : 16;
#else
    uint32_t timer : 16;
    uint32_t reserved : 13;
    uint32_t auto_reload : 1;
    uint32_t control : 1;
    uint32_t timeout : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_otrn_t;

#endif // __ASSEMBLER__
#define OCB_OTRN_TIMEOUT 0x80000000
#define OCB_OTRN_CONTROL 0x40000000
#define OCB_OTRN_AUTO_RELOAD 0x20000000
#define OCB_OTRN_TIMER_MASK 0x0000ffff
#ifndef __ASSEMBLER__


typedef union ocb_ohtmcr {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t htm_src_sel : 2;
    uint32_t htm_stop : 1;
    uint32_t htm_marker_slave_adrs : 3;
    uint32_t event2halt_mode : 2;
    uint32_t event2halt_en : 11;
    uint32_t reserved : 1;
    uint32_t event2halt_halt : 1;
    uint32_t _reserved0 : 11;
#else
    uint32_t _reserved0 : 11;
    uint32_t event2halt_halt : 1;
    uint32_t reserved : 1;
    uint32_t event2halt_en : 11;
    uint32_t event2halt_mode : 2;
    uint32_t htm_marker_slave_adrs : 3;
    uint32_t htm_stop : 1;
    uint32_t htm_src_sel : 2;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ohtmcr_t;



typedef union ocb_oehdr {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t event2halt_delay : 20;
    uint32_t _reserved0 : 12;
#else
    uint32_t _reserved0 : 12;
    uint32_t event2halt_delay : 20;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oehdr_t;



typedef union ocb_ocbslbrn {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t pull_oci_region : 2;
    uint32_t pull_start : 27;
    uint32_t _reserved0 : 3;
#else
    uint32_t _reserved0 : 3;
    uint32_t pull_start : 27;
    uint32_t pull_oci_region : 2;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocbslbrn_t;



typedef union ocb_ocbshbrn {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t push_oci_region : 2;
    uint32_t push_start : 27;
    uint32_t _reserved0 : 3;
#else
    uint32_t _reserved0 : 3;
    uint32_t push_start : 27;
    uint32_t push_oci_region : 2;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocbshbrn_t;



typedef union ocb_ocbslcsn {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t pull_full : 1;
    uint32_t pull_empty : 1;
    uint32_t reserved0 : 2;
    uint32_t pull_intr_action : 2;
    uint32_t pull_length : 5;
    uint32_t reserved1 : 2;
    uint32_t pull_write_ptr : 5;
    uint32_t reserved2 : 3;
    uint32_t pull_read_ptr : 5;
    uint32_t reserved3 : 5;
    uint32_t pull_enable : 1;
#else
    uint32_t pull_enable : 1;
    uint32_t reserved3 : 5;
    uint32_t pull_read_ptr : 5;
    uint32_t reserved2 : 3;
    uint32_t pull_write_ptr : 5;
    uint32_t reserved1 : 2;
    uint32_t pull_length : 5;
    uint32_t pull_intr_action : 2;
    uint32_t reserved0 : 2;
    uint32_t pull_empty : 1;
    uint32_t pull_full : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocbslcsn_t;

#endif // __ASSEMBLER__
#define OCB_OCBSLCSN_PULL_FULL 0x80000000
#define OCB_OCBSLCSN_PULL_EMPTY 0x40000000
#define OCB_OCBSLCSN_PULL_INTR_ACTION_MASK 0x0c000000
#define OCB_OCBSLCSN_PULL_LENGTH_MASK 0x03e00000
#define OCB_OCBSLCSN_PULL_WRITE_PTR_MASK 0x0007c000
#define OCB_OCBSLCSN_PULL_READ_PTR_MASK 0x000007c0
#define OCB_OCBSLCSN_PULL_ENABLE 0x00000001
#ifndef __ASSEMBLER__


typedef union ocb_ocbshcsn {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t push_full : 1;
    uint32_t push_empty : 1;
    uint32_t reserved0 : 2;
    uint32_t push_intr_action : 2;
    uint32_t push_length : 5;
    uint32_t reserved1 : 2;
    uint32_t push_write_ptr : 5;
    uint32_t reserved2 : 3;
    uint32_t push_read_ptr : 5;
    uint32_t reserved3 : 5;
    uint32_t push_enable : 1;
#else
    uint32_t push_enable : 1;
    uint32_t reserved3 : 5;
    uint32_t push_read_ptr : 5;
    uint32_t reserved2 : 3;
    uint32_t push_write_ptr : 5;
    uint32_t reserved1 : 2;
    uint32_t push_length : 5;
    uint32_t push_intr_action : 2;
    uint32_t reserved0 : 2;
    uint32_t push_empty : 1;
    uint32_t push_full : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocbshcsn_t;

#endif // __ASSEMBLER__
#define OCB_OCBSHCSN_PUSH_FULL 0x80000000
#define OCB_OCBSHCSN_PUSH_EMPTY 0x40000000
#define OCB_OCBSHCSN_PUSH_INTR_ACTION_MASK 0x0c000000
#define OCB_OCBSHCSN_PUSH_LENGTH_MASK 0x03e00000
#define OCB_OCBSHCSN_PUSH_WRITE_PTR_MASK 0x0007c000
#define OCB_OCBSHCSN_PUSH_READ_PTR_MASK 0x000007c0
#define OCB_OCBSHCSN_PUSH_ENABLE 0x00000001
#ifndef __ASSEMBLER__


typedef union ocb_ocbslin {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t reserved : 32;
#else
    uint32_t reserved : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocbslin_t;



typedef union ocb_ocbshin {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t reserved : 32;
#else
    uint32_t reserved : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocbshin_t;



typedef union ocb_ocbsesn {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t push_read_underflow : 1;
    uint32_t pull_write_overflow : 1;
    uint32_t _reserved0 : 30;
#else
    uint32_t _reserved0 : 30;
    uint32_t pull_write_overflow : 1;
    uint32_t push_read_underflow : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocbsesn_t;



typedef union ocb_ocbicrn {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t allow_unsecure_pib_masters : 1;
    uint32_t _reserved0 : 31;
#else
    uint32_t _reserved0 : 31;
    uint32_t allow_unsecure_pib_masters : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocbicrn_t;



typedef union ocb_ocblwcrn {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t linear_window_enable : 1;
    uint32_t spare_0 : 3;
    uint32_t linear_window_bar : 16;
    uint32_t linear_window_mask : 12;
#else
    uint32_t linear_window_mask : 12;
    uint32_t linear_window_bar : 16;
    uint32_t spare_0 : 3;
    uint32_t linear_window_enable : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocblwcrn_t;



typedef union ocb_ocblwsrn {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t linear_window_scresp : 3;
    uint32_t spare_0 : 5;
    uint32_t _reserved0 : 24;
#else
    uint32_t _reserved0 : 24;
    uint32_t spare_0 : 5;
    uint32_t linear_window_scresp : 3;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocblwsrn_t;



typedef union ocb_ocblwsbrn {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t linear_window_region : 2;
    uint32_t linear_window_base : 8;
    uint32_t _reserved0 : 22;
#else
    uint32_t _reserved0 : 22;
    uint32_t linear_window_base : 8;
    uint32_t linear_window_region : 2;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocblwsbrn_t;



typedef union ocb_ocichsw {

    uint32_t value;
    struct {
#ifdef _BIG_ENDIAN
    uint32_t m0_priority : 2;
    uint32_t m1_priority : 2;
    uint32_t m2_priority : 2;
    uint32_t m3_priority : 2;
    uint32_t m4_priority : 2;
    uint32_t m5_priority : 2;
    uint32_t m6_priority : 2;
    uint32_t m7_priority : 2;
    uint32_t dcu_priority_sel : 1;
    uint32_t icu_priority_sel : 1;
    uint32_t plbarb_lockerr : 1;
    uint32_t _reserved0 : 13;
#else
    uint32_t _reserved0 : 13;
    uint32_t plbarb_lockerr : 1;
    uint32_t icu_priority_sel : 1;
    uint32_t dcu_priority_sel : 1;
    uint32_t m7_priority : 2;
    uint32_t m6_priority : 2;
    uint32_t m5_priority : 2;
    uint32_t m4_priority : 2;
    uint32_t m3_priority : 2;
    uint32_t m2_priority : 2;
    uint32_t m1_priority : 2;
    uint32_t m0_priority : 2;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocichsw_t;



typedef union ocb_ocr {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t core_reset : 1;
    uint64_t chip_reset : 1;
    uint64_t system_reset : 1;
    uint64_t oci_arb_reset : 1;
    uint64_t trace_disable : 1;
    uint64_t trace_event : 1;
    uint64_t dbg_unconditional_event : 1;
    uint64_t ext_interrupt : 1;
    uint64_t critical_interrupt : 1;
    uint64_t spare : 7;
    uint64_t _reserved0 : 48;
#else
    uint64_t _reserved0 : 48;
    uint64_t spare : 7;
    uint64_t critical_interrupt : 1;
    uint64_t ext_interrupt : 1;
    uint64_t dbg_unconditional_event : 1;
    uint64_t trace_event : 1;
    uint64_t trace_disable : 1;
    uint64_t oci_arb_reset : 1;
    uint64_t system_reset : 1;
    uint64_t chip_reset : 1;
    uint64_t core_reset : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocr_t;



typedef union ocb_ocr_and {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t core_reset : 1;
    uint64_t chip_reset : 1;
    uint64_t system_reset : 1;
    uint64_t oci_arb_reset : 1;
    uint64_t trace_disable : 1;
    uint64_t trace_event : 1;
    uint64_t dbg_unconditional_event : 1;
    uint64_t ext_interrupt : 1;
    uint64_t critical_interrupt : 1;
    uint64_t spare : 7;
    uint64_t _reserved0 : 48;
#else
    uint64_t _reserved0 : 48;
    uint64_t spare : 7;
    uint64_t critical_interrupt : 1;
    uint64_t ext_interrupt : 1;
    uint64_t dbg_unconditional_event : 1;
    uint64_t trace_event : 1;
    uint64_t trace_disable : 1;
    uint64_t oci_arb_reset : 1;
    uint64_t system_reset : 1;
    uint64_t chip_reset : 1;
    uint64_t core_reset : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocr_and_t;



typedef union ocb_ocr_or {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t core_reset : 1;
    uint64_t chip_reset : 1;
    uint64_t system_reset : 1;
    uint64_t oci_arb_reset : 1;
    uint64_t trace_disable : 1;
    uint64_t trace_event : 1;
    uint64_t dbg_unconditional_event : 1;
    uint64_t ext_interrupt : 1;
    uint64_t critical_interrupt : 1;
    uint64_t spare : 7;
    uint64_t _reserved0 : 48;
#else
    uint64_t _reserved0 : 48;
    uint64_t spare : 7;
    uint64_t critical_interrupt : 1;
    uint64_t ext_interrupt : 1;
    uint64_t dbg_unconditional_event : 1;
    uint64_t trace_event : 1;
    uint64_t trace_disable : 1;
    uint64_t oci_arb_reset : 1;
    uint64_t system_reset : 1;
    uint64_t chip_reset : 1;
    uint64_t core_reset : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocr_or_t;



typedef union ocb_ocdbg {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t value : 12;
    uint64_t _reserved0 : 52;
#else
    uint64_t _reserved0 : 52;
    uint64_t value : 12;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocdbg_t;



typedef union ocb_ocbarn {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t oci_region : 2;
    uint64_t ocb_address : 27;
    uint64_t reserved : 3;
    uint64_t _reserved0 : 32;
#else
    uint64_t _reserved0 : 32;
    uint64_t reserved : 3;
    uint64_t ocb_address : 27;
    uint64_t oci_region : 2;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocbarn_t;

#endif // __ASSEMBLER__
#define OCB_OCBARN_OCI_REGION_MASK SIXTYFOUR_BIT_CONSTANT(0xc000000000000000)
#define OCB_OCBARN_OCB_ADDRESS_MASK SIXTYFOUR_BIT_CONSTANT(0x3ffffff800000000)
#ifndef __ASSEMBLER__


typedef union ocb_ocbcsrn {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t pull_read_underflow : 1;
    uint64_t push_write_overflow : 1;
    uint64_t pull_read_underflow_en : 1;
    uint64_t push_write_overflow_en : 1;
    uint64_t ocb_stream_mode : 1;
    uint64_t ocb_stream_type : 1;
    uint64_t reserved1 : 2;
    uint64_t ocb_oci_timeout : 1;
    uint64_t ocb_oci_read_data_parity : 1;
    uint64_t ocb_oci_slave_error : 1;
    uint64_t ocb_pib_addr_parity_err : 1;
    uint64_t ocb_pib_data_parity_err : 1;
    uint64_t reserved2 : 1;
    uint64_t ocb_fsm_err : 1;
    uint64_t _reserved0 : 49;
#else
    uint64_t _reserved0 : 49;
    uint64_t ocb_fsm_err : 1;
    uint64_t reserved2 : 1;
    uint64_t ocb_pib_data_parity_err : 1;
    uint64_t ocb_pib_addr_parity_err : 1;
    uint64_t ocb_oci_slave_error : 1;
    uint64_t ocb_oci_read_data_parity : 1;
    uint64_t ocb_oci_timeout : 1;
    uint64_t reserved1 : 2;
    uint64_t ocb_stream_type : 1;
    uint64_t ocb_stream_mode : 1;
    uint64_t push_write_overflow_en : 1;
    uint64_t pull_read_underflow_en : 1;
    uint64_t push_write_overflow : 1;
    uint64_t pull_read_underflow : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocbcsrn_t;

#endif // __ASSEMBLER__
#define OCB_OCBCSRN_PULL_READ_UNDERFLOW SIXTYFOUR_BIT_CONSTANT(0x8000000000000000)
#define OCB_OCBCSRN_PUSH_WRITE_OVERFLOW SIXTYFOUR_BIT_CONSTANT(0x4000000000000000)
#define OCB_OCBCSRN_PULL_READ_UNDERFLOW_EN SIXTYFOUR_BIT_CONSTANT(0x2000000000000000)
#define OCB_OCBCSRN_PUSH_WRITE_OVERFLOW_EN SIXTYFOUR_BIT_CONSTANT(0x1000000000000000)
#define OCB_OCBCSRN_OCB_STREAM_MODE SIXTYFOUR_BIT_CONSTANT(0x0800000000000000)
#define OCB_OCBCSRN_OCB_STREAM_TYPE SIXTYFOUR_BIT_CONSTANT(0x0400000000000000)
#define OCB_OCBCSRN_OCB_OCI_TIMEOUT SIXTYFOUR_BIT_CONSTANT(0x0080000000000000)
#define OCB_OCBCSRN_OCB_OCI_READ_DATA_PARITY SIXTYFOUR_BIT_CONSTANT(0x0040000000000000)
#define OCB_OCBCSRN_OCB_OCI_SLAVE_ERROR SIXTYFOUR_BIT_CONSTANT(0x0020000000000000)
#define OCB_OCBCSRN_OCB_PIB_ADDR_PARITY_ERR SIXTYFOUR_BIT_CONSTANT(0x0010000000000000)
#define OCB_OCBCSRN_OCB_PIB_DATA_PARITY_ERR SIXTYFOUR_BIT_CONSTANT(0x0008000000000000)
#define OCB_OCBCSRN_OCB_FSM_ERR SIXTYFOUR_BIT_CONSTANT(0x0002000000000000)
#ifndef __ASSEMBLER__


typedef union ocb_ocbcsrn_and {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t pull_read_underflow : 1;
    uint64_t push_write_overflow : 1;
    uint64_t pull_read_underflow_en : 1;
    uint64_t push_write_overflow_en : 1;
    uint64_t ocb_stream_mode : 1;
    uint64_t ocb_stream_type : 1;
    uint64_t reserved1 : 2;
    uint64_t ocb_oci_timeout : 1;
    uint64_t ocb_oci_read_data_parity : 1;
    uint64_t ocb_oci_slave_error : 1;
    uint64_t ocb_pib_addr_parity_err : 1;
    uint64_t ocb_pib_data_parity_err : 1;
    uint64_t reserved2 : 1;
    uint64_t ocb_fsm_err : 1;
    uint64_t _reserved0 : 49;
#else
    uint64_t _reserved0 : 49;
    uint64_t ocb_fsm_err : 1;
    uint64_t reserved2 : 1;
    uint64_t ocb_pib_data_parity_err : 1;
    uint64_t ocb_pib_addr_parity_err : 1;
    uint64_t ocb_oci_slave_error : 1;
    uint64_t ocb_oci_read_data_parity : 1;
    uint64_t ocb_oci_timeout : 1;
    uint64_t reserved1 : 2;
    uint64_t ocb_stream_type : 1;
    uint64_t ocb_stream_mode : 1;
    uint64_t push_write_overflow_en : 1;
    uint64_t pull_read_underflow_en : 1;
    uint64_t push_write_overflow : 1;
    uint64_t pull_read_underflow : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocbcsrn_and_t;



typedef union ocb_ocbcsrn_or {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t pull_read_underflow : 1;
    uint64_t push_write_overflow : 1;
    uint64_t pull_read_underflow_en : 1;
    uint64_t push_write_overflow_en : 1;
    uint64_t ocb_stream_mode : 1;
    uint64_t ocb_stream_type : 1;
    uint64_t reserved1 : 2;
    uint64_t ocb_oci_timeout : 1;
    uint64_t ocb_oci_read_data_parity : 1;
    uint64_t ocb_oci_slave_error : 1;
    uint64_t ocb_pib_addr_parity_err : 1;
    uint64_t ocb_pib_data_parity_err : 1;
    uint64_t reserved2 : 1;
    uint64_t ocb_fsm_err : 1;
    uint64_t _reserved0 : 49;
#else
    uint64_t _reserved0 : 49;
    uint64_t ocb_fsm_err : 1;
    uint64_t reserved2 : 1;
    uint64_t ocb_pib_data_parity_err : 1;
    uint64_t ocb_pib_addr_parity_err : 1;
    uint64_t ocb_oci_slave_error : 1;
    uint64_t ocb_oci_read_data_parity : 1;
    uint64_t ocb_oci_timeout : 1;
    uint64_t reserved1 : 2;
    uint64_t ocb_stream_type : 1;
    uint64_t ocb_stream_mode : 1;
    uint64_t push_write_overflow_en : 1;
    uint64_t pull_read_underflow_en : 1;
    uint64_t push_write_overflow : 1;
    uint64_t pull_read_underflow : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocbcsrn_or_t;



typedef union ocb_ocbesrn {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t ocb_error_addr : 32;
    uint64_t _reserved0 : 32;
#else
    uint64_t _reserved0 : 32;
    uint64_t ocb_error_addr : 32;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocbesrn_t;



typedef union ocb_ocbdrn {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t ocb_data : 64;
#else
    uint64_t ocb_data : 64;
#endif // _BIG_ENDIAN
    } fields;
} ocb_ocbdrn_t;



typedef union ocb_osbcr {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t occ_block_unsecure_masters : 1;
    uint64_t _reserved0 : 63;
#else
    uint64_t _reserved0 : 63;
    uint64_t occ_block_unsecure_masters : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_osbcr_t;



typedef union ocb_otdcr {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t trace_bus_en : 1;
    uint64_t ocb_trace_mux_sel : 1;
    uint64_t occ_trace_mux_sel : 2;
    uint64_t oci_trace_mux_sel : 4;
    uint64_t _reserved0 : 56;
#else
    uint64_t _reserved0 : 56;
    uint64_t oci_trace_mux_sel : 4;
    uint64_t occ_trace_mux_sel : 2;
    uint64_t ocb_trace_mux_sel : 1;
    uint64_t trace_bus_en : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_otdcr_t;



typedef union ocb_oppcinj {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t oci_err_inj_dcu : 1;
    uint64_t oci_err_inj_icu : 1;
    uint64_t oci_err_inj_ce_ue : 1;
    uint64_t oci_err_inj_singl_cont : 1;
    uint64_t _reserved0 : 60;
#else
    uint64_t _reserved0 : 60;
    uint64_t oci_err_inj_singl_cont : 1;
    uint64_t oci_err_inj_ce_ue : 1;
    uint64_t oci_err_inj_icu : 1;
    uint64_t oci_err_inj_dcu : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_oppcinj_t;



typedef union ocb_occlfir {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t occ_fw0 : 1;
    uint64_t occ_fw1 : 1;
    uint64_t occ_fw2 : 1;
    uint64_t occ_fw3 : 1;
    uint64_t pmc_pore_sw_malf : 1;
    uint64_t pmc_occ_hb_malf : 1;
    uint64_t pore_gpe0_fatal_err : 1;
    uint64_t pore_gpe1_fatal_err : 1;
    uint64_t ocb_error : 1;
    uint64_t pmc_error : 1;
    uint64_t srt_ue : 1;
    uint64_t srt_ce : 1;
    uint64_t srt_read_error : 1;
    uint64_t srt_write_error : 1;
    uint64_t srt_oci_write_data_parity : 1;
    uint64_t srt_oci_be_parity_err : 1;
    uint64_t srt_oci_addr_parity_err : 1;
    uint64_t pore_sw_error_err : 1;
    uint64_t pore_gpe0_error_err : 1;
    uint64_t pore_gpe1_error_err : 1;
    uint64_t external_trap : 1;
    uint64_t ppc405_core_reset : 1;
    uint64_t ppc405_chip_reset : 1;
    uint64_t ppc405_system_reset : 1;
    uint64_t ppc405_dbgmsrwe : 1;
    uint64_t ppc405_dbgstopack : 1;
    uint64_t ocb_db_oci_timeout : 1;
    uint64_t ocb_db_oci_read_data_parity : 1;
    uint64_t ocb_db_oci_slave_error : 1;
    uint64_t ocb_pib_addr_parity_err : 1;
    uint64_t ocb_db_pib_data_parity_err : 1;
    uint64_t ocb_idc0_error : 1;
    uint64_t ocb_idc1_error : 1;
    uint64_t ocb_idc2_error : 1;
    uint64_t ocb_idc3_error : 1;
    uint64_t srt_fsm_err : 1;
    uint64_t jtagacc_err : 1;
    uint64_t ocb_dw_err : 1;
    uint64_t c405_ecc_ue : 1;
    uint64_t c405_ecc_ce : 1;
    uint64_t c405_oci_machinecheck : 1;
    uint64_t sram_spare_direct_error0 : 1;
    uint64_t sram_spare_direct_error1 : 1;
    uint64_t sram_spare_direct_error2 : 1;
    uint64_t sram_spare_direct_error3 : 1;
    uint64_t slw_ocislv_err : 1;
    uint64_t gpe_ocislv_err : 1;
    uint64_t ocb_ocislv_err : 1;
    uint64_t c405icu_m_timeout : 1;
    uint64_t c405dcu_m_timeout : 1;
    uint64_t spare_fir : 12;
    uint64_t fir_parity_err_dup : 1;
    uint64_t fir_parity_err : 1;
#else
    uint64_t fir_parity_err : 1;
    uint64_t fir_parity_err_dup : 1;
    uint64_t spare_fir : 12;
    uint64_t c405dcu_m_timeout : 1;
    uint64_t c405icu_m_timeout : 1;
    uint64_t ocb_ocislv_err : 1;
    uint64_t gpe_ocislv_err : 1;
    uint64_t slw_ocislv_err : 1;
    uint64_t sram_spare_direct_error3 : 1;
    uint64_t sram_spare_direct_error2 : 1;
    uint64_t sram_spare_direct_error1 : 1;
    uint64_t sram_spare_direct_error0 : 1;
    uint64_t c405_oci_machinecheck : 1;
    uint64_t c405_ecc_ce : 1;
    uint64_t c405_ecc_ue : 1;
    uint64_t ocb_dw_err : 1;
    uint64_t jtagacc_err : 1;
    uint64_t srt_fsm_err : 1;
    uint64_t ocb_idc3_error : 1;
    uint64_t ocb_idc2_error : 1;
    uint64_t ocb_idc1_error : 1;
    uint64_t ocb_idc0_error : 1;
    uint64_t ocb_db_pib_data_parity_err : 1;
    uint64_t ocb_pib_addr_parity_err : 1;
    uint64_t ocb_db_oci_slave_error : 1;
    uint64_t ocb_db_oci_read_data_parity : 1;
    uint64_t ocb_db_oci_timeout : 1;
    uint64_t ppc405_dbgstopack : 1;
    uint64_t ppc405_dbgmsrwe : 1;
    uint64_t ppc405_system_reset : 1;
    uint64_t ppc405_chip_reset : 1;
    uint64_t ppc405_core_reset : 1;
    uint64_t external_trap : 1;
    uint64_t pore_gpe1_error_err : 1;
    uint64_t pore_gpe0_error_err : 1;
    uint64_t pore_sw_error_err : 1;
    uint64_t srt_oci_addr_parity_err : 1;
    uint64_t srt_oci_be_parity_err : 1;
    uint64_t srt_oci_write_data_parity : 1;
    uint64_t srt_write_error : 1;
    uint64_t srt_read_error : 1;
    uint64_t srt_ce : 1;
    uint64_t srt_ue : 1;
    uint64_t pmc_error : 1;
    uint64_t ocb_error : 1;
    uint64_t pore_gpe1_fatal_err : 1;
    uint64_t pore_gpe0_fatal_err : 1;
    uint64_t pmc_occ_hb_malf : 1;
    uint64_t pmc_pore_sw_malf : 1;
    uint64_t occ_fw3 : 1;
    uint64_t occ_fw2 : 1;
    uint64_t occ_fw1 : 1;
    uint64_t occ_fw0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_occlfir_t;

#endif // __ASSEMBLER__
#define OCB_OCCLFIR_OCC_FW0 SIXTYFOUR_BIT_CONSTANT(0x8000000000000000)
#define OCB_OCCLFIR_OCC_FW1 SIXTYFOUR_BIT_CONSTANT(0x4000000000000000)
#define OCB_OCCLFIR_OCC_FW2 SIXTYFOUR_BIT_CONSTANT(0x2000000000000000)
#define OCB_OCCLFIR_OCC_FW3 SIXTYFOUR_BIT_CONSTANT(0x1000000000000000)
#define OCB_OCCLFIR_PMC_PORE_SW_MALF SIXTYFOUR_BIT_CONSTANT(0x0800000000000000)
#define OCB_OCCLFIR_PMC_OCC_HB_MALF SIXTYFOUR_BIT_CONSTANT(0x0400000000000000)
#define OCB_OCCLFIR_PORE_GPE0_FATAL_ERR SIXTYFOUR_BIT_CONSTANT(0x0200000000000000)
#define OCB_OCCLFIR_PORE_GPE1_FATAL_ERR SIXTYFOUR_BIT_CONSTANT(0x0100000000000000)
#define OCB_OCCLFIR_OCB_ERROR SIXTYFOUR_BIT_CONSTANT(0x0080000000000000)
#define OCB_OCCLFIR_PMC_ERROR SIXTYFOUR_BIT_CONSTANT(0x0040000000000000)
#define OCB_OCCLFIR_SRT_UE SIXTYFOUR_BIT_CONSTANT(0x0020000000000000)
#define OCB_OCCLFIR_SRT_CE SIXTYFOUR_BIT_CONSTANT(0x0010000000000000)
#define OCB_OCCLFIR_SRT_READ_ERROR SIXTYFOUR_BIT_CONSTANT(0x0008000000000000)
#define OCB_OCCLFIR_SRT_WRITE_ERROR SIXTYFOUR_BIT_CONSTANT(0x0004000000000000)
#define OCB_OCCLFIR_SRT_OCI_WRITE_DATA_PARITY SIXTYFOUR_BIT_CONSTANT(0x0002000000000000)
#define OCB_OCCLFIR_SRT_OCI_BE_PARITY_ERR SIXTYFOUR_BIT_CONSTANT(0x0001000000000000)
#define OCB_OCCLFIR_SRT_OCI_ADDR_PARITY_ERR SIXTYFOUR_BIT_CONSTANT(0x0000800000000000)
#define OCB_OCCLFIR_PORE_SW_ERROR_ERR SIXTYFOUR_BIT_CONSTANT(0x0000400000000000)
#define OCB_OCCLFIR_PORE_GPE0_ERROR_ERR SIXTYFOUR_BIT_CONSTANT(0x0000200000000000)
#define OCB_OCCLFIR_PORE_GPE1_ERROR_ERR SIXTYFOUR_BIT_CONSTANT(0x0000100000000000)
#define OCB_OCCLFIR_EXTERNAL_TRAP SIXTYFOUR_BIT_CONSTANT(0x0000080000000000)
#define OCB_OCCLFIR_PPC405_CORE_RESET SIXTYFOUR_BIT_CONSTANT(0x0000040000000000)
#define OCB_OCCLFIR_PPC405_CHIP_RESET SIXTYFOUR_BIT_CONSTANT(0x0000020000000000)
#define OCB_OCCLFIR_PPC405_SYSTEM_RESET SIXTYFOUR_BIT_CONSTANT(0x0000010000000000)
#define OCB_OCCLFIR_PPC405_DBGMSRWE SIXTYFOUR_BIT_CONSTANT(0x0000008000000000)
#define OCB_OCCLFIR_PPC405_DBGSTOPACK SIXTYFOUR_BIT_CONSTANT(0x0000004000000000)
#define OCB_OCCLFIR_OCB_DB_OCI_TIMEOUT SIXTYFOUR_BIT_CONSTANT(0x0000002000000000)
#define OCB_OCCLFIR_OCB_DB_OCI_READ_DATA_PARITY SIXTYFOUR_BIT_CONSTANT(0x0000001000000000)
#define OCB_OCCLFIR_OCB_DB_OCI_SLAVE_ERROR SIXTYFOUR_BIT_CONSTANT(0x0000000800000000)
#define OCB_OCCLFIR_OCB_PIB_ADDR_PARITY_ERR SIXTYFOUR_BIT_CONSTANT(0x0000000400000000)
#define OCB_OCCLFIR_OCB_DB_PIB_DATA_PARITY_ERR SIXTYFOUR_BIT_CONSTANT(0x0000000200000000)
#define OCB_OCCLFIR_OCB_IDC0_ERROR SIXTYFOUR_BIT_CONSTANT(0x0000000100000000)
#define OCB_OCCLFIR_OCB_IDC1_ERROR SIXTYFOUR_BIT_CONSTANT(0x0000000080000000)
#define OCB_OCCLFIR_OCB_IDC2_ERROR SIXTYFOUR_BIT_CONSTANT(0x0000000040000000)
#define OCB_OCCLFIR_OCB_IDC3_ERROR SIXTYFOUR_BIT_CONSTANT(0x0000000020000000)
#define OCB_OCCLFIR_SRT_FSM_ERR SIXTYFOUR_BIT_CONSTANT(0x0000000010000000)
#define OCB_OCCLFIR_JTAGACC_ERR SIXTYFOUR_BIT_CONSTANT(0x0000000008000000)
#define OCB_OCCLFIR_OCB_DW_ERR SIXTYFOUR_BIT_CONSTANT(0x0000000004000000)
#define OCB_OCCLFIR_C405_ECC_UE SIXTYFOUR_BIT_CONSTANT(0x0000000002000000)
#define OCB_OCCLFIR_C405_ECC_CE SIXTYFOUR_BIT_CONSTANT(0x0000000001000000)
#define OCB_OCCLFIR_C405_OCI_MACHINECHECK SIXTYFOUR_BIT_CONSTANT(0x0000000000800000)
#define OCB_OCCLFIR_SRAM_SPARE_DIRECT_ERROR0 SIXTYFOUR_BIT_CONSTANT(0x0000000000400000)
#define OCB_OCCLFIR_SRAM_SPARE_DIRECT_ERROR1 SIXTYFOUR_BIT_CONSTANT(0x0000000000200000)
#define OCB_OCCLFIR_SRAM_SPARE_DIRECT_ERROR2 SIXTYFOUR_BIT_CONSTANT(0x0000000000100000)
#define OCB_OCCLFIR_SRAM_SPARE_DIRECT_ERROR3 SIXTYFOUR_BIT_CONSTANT(0x0000000000080000)
#define OCB_OCCLFIR_SLW_OCISLV_ERR SIXTYFOUR_BIT_CONSTANT(0x0000000000040000)
#define OCB_OCCLFIR_GPE_OCISLV_ERR SIXTYFOUR_BIT_CONSTANT(0x0000000000020000)
#define OCB_OCCLFIR_OCB_OCISLV_ERR SIXTYFOUR_BIT_CONSTANT(0x0000000000010000)
#define OCB_OCCLFIR_C405ICU_M_TIMEOUT SIXTYFOUR_BIT_CONSTANT(0x0000000000008000)
#define OCB_OCCLFIR_C405DCU_M_TIMEOUT SIXTYFOUR_BIT_CONSTANT(0x0000000000004000)
#define OCB_OCCLFIR_SPARE_FIR_MASK SIXTYFOUR_BIT_CONSTANT(0x0000000000003ffc)
#define OCB_OCCLFIR_FIR_PARITY_ERR_DUP SIXTYFOUR_BIT_CONSTANT(0x0000000000000002)
#define OCB_OCCLFIR_FIR_PARITY_ERR SIXTYFOUR_BIT_CONSTANT(0x0000000000000001)
#ifndef __ASSEMBLER__


typedef union ocb_occlfir_and {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t occ_fw0 : 1;
    uint64_t occ_fw1 : 1;
    uint64_t occ_fw2 : 1;
    uint64_t occ_fw3 : 1;
    uint64_t pmc_pore_sw_malf : 1;
    uint64_t pmc_occ_hb_malf : 1;
    uint64_t pore_gpe0_fatal_err : 1;
    uint64_t pore_gpe1_fatal_err : 1;
    uint64_t ocb_error : 1;
    uint64_t pmc_error : 1;
    uint64_t srt_ue : 1;
    uint64_t srt_ce : 1;
    uint64_t srt_read_error : 1;
    uint64_t srt_write_error : 1;
    uint64_t srt_oci_write_data_parity : 1;
    uint64_t srt_oci_be_parity_err : 1;
    uint64_t srt_oci_addr_parity_err : 1;
    uint64_t pore_sw_error_err : 1;
    uint64_t pore_gpe0_error_err : 1;
    uint64_t pore_gpe1_error_err : 1;
    uint64_t external_trap : 1;
    uint64_t ppc405_core_reset : 1;
    uint64_t ppc405_chip_reset : 1;
    uint64_t ppc405_system_reset : 1;
    uint64_t ppc405_dbgmsrwe : 1;
    uint64_t ppc405_dbgstopack : 1;
    uint64_t ocb_db_oci_timeout : 1;
    uint64_t ocb_db_oci_read_data_parity : 1;
    uint64_t ocb_db_oci_slave_error : 1;
    uint64_t ocb_pib_addr_parity_err : 1;
    uint64_t ocb_db_pib_data_parity_err : 1;
    uint64_t ocb_idc0_error : 1;
    uint64_t ocb_idc1_error : 1;
    uint64_t ocb_idc2_error : 1;
    uint64_t ocb_idc3_error : 1;
    uint64_t srt_fsm_err : 1;
    uint64_t jtagacc_err : 1;
    uint64_t ocb_dw_err : 1;
    uint64_t c405_ecc_ue : 1;
    uint64_t c405_ecc_ce : 1;
    uint64_t c405_oci_machinecheck : 1;
    uint64_t sram_spare_direct_error0 : 1;
    uint64_t sram_spare_direct_error1 : 1;
    uint64_t sram_spare_direct_error2 : 1;
    uint64_t sram_spare_direct_error3 : 1;
    uint64_t slw_ocislv_err : 1;
    uint64_t gpe_ocislv_err : 1;
    uint64_t ocb_ocislv_err : 1;
    uint64_t c405icu_m_timeout : 1;
    uint64_t c405dcu_m_timeout : 1;
    uint64_t spare_fir : 12;
    uint64_t fir_parity_err_dup : 1;
    uint64_t fir_parity_err : 1;
#else
    uint64_t fir_parity_err : 1;
    uint64_t fir_parity_err_dup : 1;
    uint64_t spare_fir : 12;
    uint64_t c405dcu_m_timeout : 1;
    uint64_t c405icu_m_timeout : 1;
    uint64_t ocb_ocislv_err : 1;
    uint64_t gpe_ocislv_err : 1;
    uint64_t slw_ocislv_err : 1;
    uint64_t sram_spare_direct_error3 : 1;
    uint64_t sram_spare_direct_error2 : 1;
    uint64_t sram_spare_direct_error1 : 1;
    uint64_t sram_spare_direct_error0 : 1;
    uint64_t c405_oci_machinecheck : 1;
    uint64_t c405_ecc_ce : 1;
    uint64_t c405_ecc_ue : 1;
    uint64_t ocb_dw_err : 1;
    uint64_t jtagacc_err : 1;
    uint64_t srt_fsm_err : 1;
    uint64_t ocb_idc3_error : 1;
    uint64_t ocb_idc2_error : 1;
    uint64_t ocb_idc1_error : 1;
    uint64_t ocb_idc0_error : 1;
    uint64_t ocb_db_pib_data_parity_err : 1;
    uint64_t ocb_pib_addr_parity_err : 1;
    uint64_t ocb_db_oci_slave_error : 1;
    uint64_t ocb_db_oci_read_data_parity : 1;
    uint64_t ocb_db_oci_timeout : 1;
    uint64_t ppc405_dbgstopack : 1;
    uint64_t ppc405_dbgmsrwe : 1;
    uint64_t ppc405_system_reset : 1;
    uint64_t ppc405_chip_reset : 1;
    uint64_t ppc405_core_reset : 1;
    uint64_t external_trap : 1;
    uint64_t pore_gpe1_error_err : 1;
    uint64_t pore_gpe0_error_err : 1;
    uint64_t pore_sw_error_err : 1;
    uint64_t srt_oci_addr_parity_err : 1;
    uint64_t srt_oci_be_parity_err : 1;
    uint64_t srt_oci_write_data_parity : 1;
    uint64_t srt_write_error : 1;
    uint64_t srt_read_error : 1;
    uint64_t srt_ce : 1;
    uint64_t srt_ue : 1;
    uint64_t pmc_error : 1;
    uint64_t ocb_error : 1;
    uint64_t pore_gpe1_fatal_err : 1;
    uint64_t pore_gpe0_fatal_err : 1;
    uint64_t pmc_occ_hb_malf : 1;
    uint64_t pmc_pore_sw_malf : 1;
    uint64_t occ_fw3 : 1;
    uint64_t occ_fw2 : 1;
    uint64_t occ_fw1 : 1;
    uint64_t occ_fw0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_occlfir_and_t;



typedef union ocb_occlfir_or {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t occ_fw0 : 1;
    uint64_t occ_fw1 : 1;
    uint64_t occ_fw2 : 1;
    uint64_t occ_fw3 : 1;
    uint64_t pmc_pore_sw_malf : 1;
    uint64_t pmc_occ_hb_malf : 1;
    uint64_t pore_gpe0_fatal_err : 1;
    uint64_t pore_gpe1_fatal_err : 1;
    uint64_t ocb_error : 1;
    uint64_t pmc_error : 1;
    uint64_t srt_ue : 1;
    uint64_t srt_ce : 1;
    uint64_t srt_read_error : 1;
    uint64_t srt_write_error : 1;
    uint64_t srt_oci_write_data_parity : 1;
    uint64_t srt_oci_be_parity_err : 1;
    uint64_t srt_oci_addr_parity_err : 1;
    uint64_t pore_sw_error_err : 1;
    uint64_t pore_gpe0_error_err : 1;
    uint64_t pore_gpe1_error_err : 1;
    uint64_t external_trap : 1;
    uint64_t ppc405_core_reset : 1;
    uint64_t ppc405_chip_reset : 1;
    uint64_t ppc405_system_reset : 1;
    uint64_t ppc405_dbgmsrwe : 1;
    uint64_t ppc405_dbgstopack : 1;
    uint64_t ocb_db_oci_timeout : 1;
    uint64_t ocb_db_oci_read_data_parity : 1;
    uint64_t ocb_db_oci_slave_error : 1;
    uint64_t ocb_pib_addr_parity_err : 1;
    uint64_t ocb_db_pib_data_parity_err : 1;
    uint64_t ocb_idc0_error : 1;
    uint64_t ocb_idc1_error : 1;
    uint64_t ocb_idc2_error : 1;
    uint64_t ocb_idc3_error : 1;
    uint64_t srt_fsm_err : 1;
    uint64_t jtagacc_err : 1;
    uint64_t ocb_dw_err : 1;
    uint64_t c405_ecc_ue : 1;
    uint64_t c405_ecc_ce : 1;
    uint64_t c405_oci_machinecheck : 1;
    uint64_t sram_spare_direct_error0 : 1;
    uint64_t sram_spare_direct_error1 : 1;
    uint64_t sram_spare_direct_error2 : 1;
    uint64_t sram_spare_direct_error3 : 1;
    uint64_t slw_ocislv_err : 1;
    uint64_t gpe_ocislv_err : 1;
    uint64_t ocb_ocislv_err : 1;
    uint64_t c405icu_m_timeout : 1;
    uint64_t c405dcu_m_timeout : 1;
    uint64_t spare_fir : 12;
    uint64_t fir_parity_err_dup : 1;
    uint64_t fir_parity_err : 1;
#else
    uint64_t fir_parity_err : 1;
    uint64_t fir_parity_err_dup : 1;
    uint64_t spare_fir : 12;
    uint64_t c405dcu_m_timeout : 1;
    uint64_t c405icu_m_timeout : 1;
    uint64_t ocb_ocislv_err : 1;
    uint64_t gpe_ocislv_err : 1;
    uint64_t slw_ocislv_err : 1;
    uint64_t sram_spare_direct_error3 : 1;
    uint64_t sram_spare_direct_error2 : 1;
    uint64_t sram_spare_direct_error1 : 1;
    uint64_t sram_spare_direct_error0 : 1;
    uint64_t c405_oci_machinecheck : 1;
    uint64_t c405_ecc_ce : 1;
    uint64_t c405_ecc_ue : 1;
    uint64_t ocb_dw_err : 1;
    uint64_t jtagacc_err : 1;
    uint64_t srt_fsm_err : 1;
    uint64_t ocb_idc3_error : 1;
    uint64_t ocb_idc2_error : 1;
    uint64_t ocb_idc1_error : 1;
    uint64_t ocb_idc0_error : 1;
    uint64_t ocb_db_pib_data_parity_err : 1;
    uint64_t ocb_pib_addr_parity_err : 1;
    uint64_t ocb_db_oci_slave_error : 1;
    uint64_t ocb_db_oci_read_data_parity : 1;
    uint64_t ocb_db_oci_timeout : 1;
    uint64_t ppc405_dbgstopack : 1;
    uint64_t ppc405_dbgmsrwe : 1;
    uint64_t ppc405_system_reset : 1;
    uint64_t ppc405_chip_reset : 1;
    uint64_t ppc405_core_reset : 1;
    uint64_t external_trap : 1;
    uint64_t pore_gpe1_error_err : 1;
    uint64_t pore_gpe0_error_err : 1;
    uint64_t pore_sw_error_err : 1;
    uint64_t srt_oci_addr_parity_err : 1;
    uint64_t srt_oci_be_parity_err : 1;
    uint64_t srt_oci_write_data_parity : 1;
    uint64_t srt_write_error : 1;
    uint64_t srt_read_error : 1;
    uint64_t srt_ce : 1;
    uint64_t srt_ue : 1;
    uint64_t pmc_error : 1;
    uint64_t ocb_error : 1;
    uint64_t pore_gpe1_fatal_err : 1;
    uint64_t pore_gpe0_fatal_err : 1;
    uint64_t pmc_occ_hb_malf : 1;
    uint64_t pmc_pore_sw_malf : 1;
    uint64_t occ_fw3 : 1;
    uint64_t occ_fw2 : 1;
    uint64_t occ_fw1 : 1;
    uint64_t occ_fw0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} ocb_occlfir_or_t;



typedef union ocb_occlfirmask {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t value : 64;
#else
    uint64_t value : 64;
#endif // _BIG_ENDIAN
    } fields;
} ocb_occlfirmask_t;



typedef union ocb_occlfirmask_and {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t value : 64;
#else
    uint64_t value : 64;
#endif // _BIG_ENDIAN
    } fields;
} ocb_occlfirmask_and_t;



typedef union ocb_occlfirmask_or {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t value : 64;
#else
    uint64_t value : 64;
#endif // _BIG_ENDIAN
    } fields;
} ocb_occlfirmask_or_t;



typedef union ocb_occlfiract0 {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t value : 64;
#else
    uint64_t value : 64;
#endif // _BIG_ENDIAN
    } fields;
} ocb_occlfiract0_t;



typedef union ocb_occlfiract1 {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t value : 64;
#else
    uint64_t value : 64;
#endif // _BIG_ENDIAN
    } fields;
} ocb_occlfiract1_t;



typedef union ocb_occerrrpt {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t sram_cerrrpt : 10;
    uint64_t jtagacc_cerrrpt : 6;
    uint64_t c405_dcu_ecc_ue_cerrrpt : 1;
    uint64_t c405_dcu_ecc_ce_cerrrpt : 1;
    uint64_t c405_icu_ecc_ue_cerrrpt : 1;
    uint64_t c405_icu_ecc_ce_cerrrpt : 1;
    uint64_t slw_ocislv_err : 7;
    uint64_t gpe_ocislv_err : 7;
    uint64_t ocb_ocislv_err : 6;
    uint64_t _reserved0 : 24;
#else
    uint64_t _reserved0 : 24;
    uint64_t ocb_ocislv_err : 6;
    uint64_t gpe_ocislv_err : 7;
    uint64_t slw_ocislv_err : 7;
    uint64_t c405_icu_ecc_ce_cerrrpt : 1;
    uint64_t c405_icu_ecc_ue_cerrrpt : 1;
    uint64_t c405_dcu_ecc_ce_cerrrpt : 1;
    uint64_t c405_dcu_ecc_ue_cerrrpt : 1;
    uint64_t jtagacc_cerrrpt : 6;
    uint64_t sram_cerrrpt : 10;
#endif // _BIG_ENDIAN
    } fields;
} ocb_occerrrpt_t;



typedef union ocb_scan_dummy_1 {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t _reserved0 : 48;
    uint64_t value : 16;
#else
    uint64_t value : 16;
    uint64_t _reserved0 : 48;
#endif // _BIG_ENDIAN
    } fields;
} ocb_scan_dummy_1_t;



typedef union ocb_scan_dummy_2 {

    uint64_t value;
    struct {
#ifdef _BIG_ENDIAN
        uint32_t high_order;
        uint32_t low_order;
#else
        uint32_t low_order;
        uint32_t high_order;
#endif // _BIG_ENDIAN
    } words;
    struct {
#ifdef _BIG_ENDIAN
    uint64_t _reserved0 : 63;
    uint64_t value : 1;
#else
    uint64_t value : 1;
    uint64_t _reserved0 : 63;
#endif // _BIG_ENDIAN
    } fields;
} ocb_scan_dummy_2_t;


#endif // __ASSEMBLER__
#endif // __OCB_FIRMWARE_REGISTERS_H__

