#ifndef __PORE_FIRMWARE_REGISTERS_H__
#define __PORE_FIRMWARE_REGISTERS_H__

// $Id: pore_firmware_registers.h,v 1.1.1.1 2013/12/11 21:03:25 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/registers/pore_firmware_registers.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file pore_firmware_registers.h
/// \brief C register structs for the PORE unit

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




typedef union pore_status {

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
    uint64_t cur_state : 8;
    uint64_t freeze_action : 1;
    uint64_t interrupt_pending : 1;
    uint64_t spare : 2;
    uint64_t stack_pointer : 4;
    uint64_t pc : 48;
#else
    uint64_t pc : 48;
    uint64_t stack_pointer : 4;
    uint64_t spare : 2;
    uint64_t interrupt_pending : 1;
    uint64_t freeze_action : 1;
    uint64_t cur_state : 8;
#endif // _BIG_ENDIAN
    } fields;
} pore_status_t;

#endif // __ASSEMBLER__
#define PORE_STATUS_CUR_STATE_MASK SIXTYFOUR_BIT_CONSTANT(0xff00000000000000)
#define PORE_STATUS_FREEZE_ACTION SIXTYFOUR_BIT_CONSTANT(0x0080000000000000)
#define PORE_STATUS_INTERRUPT_PENDING SIXTYFOUR_BIT_CONSTANT(0x0040000000000000)
#define PORE_STATUS_SPARE_MASK SIXTYFOUR_BIT_CONSTANT(0x0030000000000000)
#define PORE_STATUS_STACK_POINTER_MASK SIXTYFOUR_BIT_CONSTANT(0x000f000000000000)
#define PORE_STATUS_PC_MASK SIXTYFOUR_BIT_CONSTANT(0xffffffffffffffff)
#ifndef __ASSEMBLER__


typedef union pore_control {

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
    uint64_t start_stop : 1;
    uint64_t continue_step : 1;
    uint64_t skip : 1;
    uint64_t set_pc : 1;
    uint64_t set_tp_scan_clk : 3;
    uint64_t lock_exe_trig : 1;
    uint64_t freeze_mask : 1;
    uint64_t check_parity : 1;
    uint64_t prv_parity : 1;
    uint64_t trap_enable : 1;
    uint64_t narrow_mode_trace : 1;
    uint64_t interruptible : 1;
    uint64_t pore_done_override : 1;
    uint64_t interruptible_en : 1;
    uint64_t pc_brk_pt : 48;
#else
    uint64_t pc_brk_pt : 48;
    uint64_t interruptible_en : 1;
    uint64_t pore_done_override : 1;
    uint64_t interruptible : 1;
    uint64_t narrow_mode_trace : 1;
    uint64_t trap_enable : 1;
    uint64_t prv_parity : 1;
    uint64_t check_parity : 1;
    uint64_t freeze_mask : 1;
    uint64_t lock_exe_trig : 1;
    uint64_t set_tp_scan_clk : 3;
    uint64_t set_pc : 1;
    uint64_t skip : 1;
    uint64_t continue_step : 1;
    uint64_t start_stop : 1;
#endif // _BIG_ENDIAN
    } fields;
} pore_control_t;

#endif // __ASSEMBLER__
#define PORE_CONTROL_START_STOP SIXTYFOUR_BIT_CONSTANT(0x8000000000000000)
#define PORE_CONTROL_CONTINUE_STEP SIXTYFOUR_BIT_CONSTANT(0x4000000000000000)
#define PORE_CONTROL_SKIP SIXTYFOUR_BIT_CONSTANT(0x2000000000000000)
#define PORE_CONTROL_SET_PC SIXTYFOUR_BIT_CONSTANT(0x1000000000000000)
#define PORE_CONTROL_SET_TP_SCAN_CLK_MASK SIXTYFOUR_BIT_CONSTANT(0x0e00000000000000)
#define PORE_CONTROL_LOCK_EXE_TRIG SIXTYFOUR_BIT_CONSTANT(0x0100000000000000)
#define PORE_CONTROL_FREEZE_MASK SIXTYFOUR_BIT_CONSTANT(0x0080000000000000)
#define PORE_CONTROL_CHECK_PARITY SIXTYFOUR_BIT_CONSTANT(0x0040000000000000)
#define PORE_CONTROL_PRV_PARITY SIXTYFOUR_BIT_CONSTANT(0x0020000000000000)
#define PORE_CONTROL_TRAP_ENABLE SIXTYFOUR_BIT_CONSTANT(0x0010000000000000)
#define PORE_CONTROL_NARROW_MODE_TRACE SIXTYFOUR_BIT_CONSTANT(0x0008000000000000)
#define PORE_CONTROL_INTERRUPTIBLE SIXTYFOUR_BIT_CONSTANT(0x0004000000000000)
#define PORE_CONTROL_PORE_DONE_OVERRIDE SIXTYFOUR_BIT_CONSTANT(0x0002000000000000)
#define PORE_CONTROL_INTERRUPTIBLE_EN SIXTYFOUR_BIT_CONSTANT(0x0001000000000000)
#define PORE_CONTROL_PC_BRK_PT_MASK SIXTYFOUR_BIT_CONSTANT(0xffffffffffffffff)
#ifndef __ASSEMBLER__


typedef union pore_reset {

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
    uint64_t fn_reset : 1;
    uint64_t oci_reset : 1;
    uint64_t _reserved0 : 62;
#else
    uint64_t _reserved0 : 62;
    uint64_t oci_reset : 1;
    uint64_t fn_reset : 1;
#endif // _BIG_ENDIAN
    } fields;
} pore_reset_t;



typedef union pore_error_mask {

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
    uint64_t enable_err_handler0 : 1;
    uint64_t enable_err_handler1 : 1;
    uint64_t enable_err_handler2 : 1;
    uint64_t enable_err_handler3 : 1;
    uint64_t enable_err_handler4 : 1;
    uint64_t enable_err_output0 : 1;
    uint64_t enable_err_output1 : 1;
    uint64_t enable_err_output2 : 1;
    uint64_t enable_err_output3 : 1;
    uint64_t enable_err_output4 : 1;
    uint64_t enable_fatal_err_output0 : 1;
    uint64_t enable_fatal_err_output1 : 1;
    uint64_t enable_fatal_err_output2 : 1;
    uint64_t enable_fatal_err_output3 : 1;
    uint64_t enable_fatal_err_output4 : 1;
    uint64_t stop_exe_on_error0 : 1;
    uint64_t stop_exe_on_error1 : 1;
    uint64_t stop_exe_on_error2 : 1;
    uint64_t stop_exe_on_error3 : 1;
    uint64_t stop_exe_on_error4 : 1;
    uint64_t gate_chiplet_offline_err : 1;
    uint64_t i2c_bad_status_0 : 1;
    uint64_t i2c_bad_status_1 : 1;
    uint64_t i2c_bad_status_2 : 1;
    uint64_t i2c_bad_status_3 : 1;
    uint64_t group_parity_error_0 : 1;
    uint64_t group_parity_error_1 : 1;
    uint64_t group_parity_error_2 : 1;
    uint64_t group_parity_error_3 : 1;
    uint64_t group_parity_error_4 : 1;
    uint64_t _reserved0 : 34;
#else
    uint64_t _reserved0 : 34;
    uint64_t group_parity_error_4 : 1;
    uint64_t group_parity_error_3 : 1;
    uint64_t group_parity_error_2 : 1;
    uint64_t group_parity_error_1 : 1;
    uint64_t group_parity_error_0 : 1;
    uint64_t i2c_bad_status_3 : 1;
    uint64_t i2c_bad_status_2 : 1;
    uint64_t i2c_bad_status_1 : 1;
    uint64_t i2c_bad_status_0 : 1;
    uint64_t gate_chiplet_offline_err : 1;
    uint64_t stop_exe_on_error4 : 1;
    uint64_t stop_exe_on_error3 : 1;
    uint64_t stop_exe_on_error2 : 1;
    uint64_t stop_exe_on_error1 : 1;
    uint64_t stop_exe_on_error0 : 1;
    uint64_t enable_fatal_err_output4 : 1;
    uint64_t enable_fatal_err_output3 : 1;
    uint64_t enable_fatal_err_output2 : 1;
    uint64_t enable_fatal_err_output1 : 1;
    uint64_t enable_fatal_err_output0 : 1;
    uint64_t enable_err_output4 : 1;
    uint64_t enable_err_output3 : 1;
    uint64_t enable_err_output2 : 1;
    uint64_t enable_err_output1 : 1;
    uint64_t enable_err_output0 : 1;
    uint64_t enable_err_handler4 : 1;
    uint64_t enable_err_handler3 : 1;
    uint64_t enable_err_handler2 : 1;
    uint64_t enable_err_handler1 : 1;
    uint64_t enable_err_handler0 : 1;
#endif // _BIG_ENDIAN
    } fields;
} pore_error_mask_t;

#endif // __ASSEMBLER__
#define PORE_ERROR_MASK_ENABLE_ERR_HANDLER0 SIXTYFOUR_BIT_CONSTANT(0x8000000000000000)
#define PORE_ERROR_MASK_ENABLE_ERR_HANDLER1 SIXTYFOUR_BIT_CONSTANT(0x4000000000000000)
#define PORE_ERROR_MASK_ENABLE_ERR_HANDLER2 SIXTYFOUR_BIT_CONSTANT(0x2000000000000000)
#define PORE_ERROR_MASK_ENABLE_ERR_HANDLER3 SIXTYFOUR_BIT_CONSTANT(0x1000000000000000)
#define PORE_ERROR_MASK_ENABLE_ERR_HANDLER4 SIXTYFOUR_BIT_CONSTANT(0x0800000000000000)
#define PORE_ERROR_MASK_ENABLE_ERR_OUTPUT0 SIXTYFOUR_BIT_CONSTANT(0x0400000000000000)
#define PORE_ERROR_MASK_ENABLE_ERR_OUTPUT1 SIXTYFOUR_BIT_CONSTANT(0x0200000000000000)
#define PORE_ERROR_MASK_ENABLE_ERR_OUTPUT2 SIXTYFOUR_BIT_CONSTANT(0x0100000000000000)
#define PORE_ERROR_MASK_ENABLE_ERR_OUTPUT3 SIXTYFOUR_BIT_CONSTANT(0x0080000000000000)
#define PORE_ERROR_MASK_ENABLE_ERR_OUTPUT4 SIXTYFOUR_BIT_CONSTANT(0x0040000000000000)
#define PORE_ERROR_MASK_ENABLE_FATAL_ERR_OUTPUT0 SIXTYFOUR_BIT_CONSTANT(0x0020000000000000)
#define PORE_ERROR_MASK_ENABLE_FATAL_ERR_OUTPUT1 SIXTYFOUR_BIT_CONSTANT(0x0010000000000000)
#define PORE_ERROR_MASK_ENABLE_FATAL_ERR_OUTPUT2 SIXTYFOUR_BIT_CONSTANT(0x0008000000000000)
#define PORE_ERROR_MASK_ENABLE_FATAL_ERR_OUTPUT3 SIXTYFOUR_BIT_CONSTANT(0x0004000000000000)
#define PORE_ERROR_MASK_ENABLE_FATAL_ERR_OUTPUT4 SIXTYFOUR_BIT_CONSTANT(0x0002000000000000)
#define PORE_ERROR_MASK_STOP_EXE_ON_ERROR0 SIXTYFOUR_BIT_CONSTANT(0x0001000000000000)
#define PORE_ERROR_MASK_STOP_EXE_ON_ERROR1 SIXTYFOUR_BIT_CONSTANT(0x0000800000000000)
#define PORE_ERROR_MASK_STOP_EXE_ON_ERROR2 SIXTYFOUR_BIT_CONSTANT(0x0000400000000000)
#define PORE_ERROR_MASK_STOP_EXE_ON_ERROR3 SIXTYFOUR_BIT_CONSTANT(0x0000200000000000)
#define PORE_ERROR_MASK_STOP_EXE_ON_ERROR4 SIXTYFOUR_BIT_CONSTANT(0x0000100000000000)
#define PORE_ERROR_MASK_GATE_CHIPLET_OFFLINE_ERR SIXTYFOUR_BIT_CONSTANT(0x0000080000000000)
#define PORE_ERROR_MASK_I2C_BAD_STATUS_0 SIXTYFOUR_BIT_CONSTANT(0x0000040000000000)
#define PORE_ERROR_MASK_I2C_BAD_STATUS_1 SIXTYFOUR_BIT_CONSTANT(0x0000020000000000)
#define PORE_ERROR_MASK_I2C_BAD_STATUS_2 SIXTYFOUR_BIT_CONSTANT(0x0000010000000000)
#define PORE_ERROR_MASK_I2C_BAD_STATUS_3 SIXTYFOUR_BIT_CONSTANT(0x0000008000000000)
#define PORE_ERROR_MASK_GROUP_PARITY_ERROR_0 SIXTYFOUR_BIT_CONSTANT(0x0000004000000000)
#define PORE_ERROR_MASK_GROUP_PARITY_ERROR_1 SIXTYFOUR_BIT_CONSTANT(0x0000002000000000)
#define PORE_ERROR_MASK_GROUP_PARITY_ERROR_2 SIXTYFOUR_BIT_CONSTANT(0x0000001000000000)
#define PORE_ERROR_MASK_GROUP_PARITY_ERROR_3 SIXTYFOUR_BIT_CONSTANT(0x0000000800000000)
#define PORE_ERROR_MASK_GROUP_PARITY_ERROR_4 SIXTYFOUR_BIT_CONSTANT(0x0000000400000000)
#ifndef __ASSEMBLER__


typedef union pore_prv_base_address0 {

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
    uint64_t spare : 25;
    uint64_t mc : 1;
    uint64_t chiplet_id : 6;
    uint64_t _reserved0 : 32;
#else
    uint64_t _reserved0 : 32;
    uint64_t chiplet_id : 6;
    uint64_t mc : 1;
    uint64_t spare : 25;
#endif // _BIG_ENDIAN
    } fields;
} pore_prv_base_address0_t;



typedef union pore_prv_base_address1 {

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
    uint64_t spare : 25;
    uint64_t mc : 1;
    uint64_t chiplet_id : 6;
    uint64_t _reserved0 : 32;
#else
    uint64_t _reserved0 : 32;
    uint64_t chiplet_id : 6;
    uint64_t mc : 1;
    uint64_t spare : 25;
#endif // _BIG_ENDIAN
    } fields;
} pore_prv_base_address1_t;



typedef union pore_oci_base_address0 {

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
    uint64_t spare : 18;
    uint64_t oci_mem_route : 14;
    uint64_t oci_base_address : 32;
#else
    uint64_t oci_base_address : 32;
    uint64_t oci_mem_route : 14;
    uint64_t spare : 18;
#endif // _BIG_ENDIAN
    } fields;
} pore_oci_base_address0_t;



typedef union pore_oci_base_address1 {

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
    uint64_t spare : 18;
    uint64_t oci_mem_route : 14;
    uint64_t oci_base_address : 32;
#else
    uint64_t oci_base_address : 32;
    uint64_t oci_mem_route : 14;
    uint64_t spare : 18;
#endif // _BIG_ENDIAN
    } fields;
} pore_oci_base_address1_t;



typedef union pore_table_base_addr {

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
    uint64_t reserved : 16;
    uint64_t memory_space : 16;
    uint64_t table_base_address : 32;
#else
    uint64_t table_base_address : 32;
    uint64_t memory_space : 16;
    uint64_t reserved : 16;
#endif // _BIG_ENDIAN
    } fields;
} pore_table_base_addr_t;



typedef union pore_exe_trigger {

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
    uint64_t reserved : 8;
    uint64_t start_vector : 4;
    uint64_t zeroes : 8;
    uint64_t unused : 12;
    uint64_t mc_chiplet_select_mask : 32;
#else
    uint64_t mc_chiplet_select_mask : 32;
    uint64_t unused : 12;
    uint64_t zeroes : 8;
    uint64_t start_vector : 4;
    uint64_t reserved : 8;
#endif // _BIG_ENDIAN
    } fields;
} pore_exe_trigger_t;



typedef union pore_scratch0 {

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
    uint64_t zeroes : 8;
    uint64_t scratch0 : 24;
    uint64_t _reserved0 : 32;
#else
    uint64_t _reserved0 : 32;
    uint64_t scratch0 : 24;
    uint64_t zeroes : 8;
#endif // _BIG_ENDIAN
    } fields;
} pore_scratch0_t;



typedef union pore_scratch1 {

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
    uint64_t scratch1 : 64;
#else
    uint64_t scratch1 : 64;
#endif // _BIG_ENDIAN
    } fields;
} pore_scratch1_t;



typedef union pore_scratch2 {

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
    uint64_t scratch2 : 64;
#else
    uint64_t scratch2 : 64;
#endif // _BIG_ENDIAN
    } fields;
} pore_scratch2_t;



typedef union pore_ibuf_01 {

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
    uint64_t ibuf0 : 32;
    uint64_t ibuf1 : 32;
#else
    uint64_t ibuf1 : 32;
    uint64_t ibuf0 : 32;
#endif // _BIG_ENDIAN
    } fields;
} pore_ibuf_01_t;



typedef union pore_ibuf_2 {

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
    uint64_t ibuf2 : 32;
    uint64_t _reserved0 : 32;
#else
    uint64_t _reserved0 : 32;
    uint64_t ibuf2 : 32;
#endif // _BIG_ENDIAN
    } fields;
} pore_ibuf_2_t;



typedef union pore_dbg0 {

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
    uint64_t last_completed_address : 32;
    uint64_t last_pib_parity_fail : 1;
    uint64_t last_ret_code_prv : 3;
    uint64_t i2c_bad_status0 : 1;
    uint64_t i2c_bad_status1 : 1;
    uint64_t i2c_bad_status2 : 1;
    uint64_t i2c_bad_status3 : 1;
    uint64_t group_parity_error0 : 1;
    uint64_t group_parity_error1 : 1;
    uint64_t group_parity_error2 : 1;
    uint64_t group_parity_error3 : 1;
    uint64_t group_parity_error4 : 1;
    uint64_t interrupt_counter : 8;
    uint64_t _reserved0 : 11;
#else
    uint64_t _reserved0 : 11;
    uint64_t interrupt_counter : 8;
    uint64_t group_parity_error4 : 1;
    uint64_t group_parity_error3 : 1;
    uint64_t group_parity_error2 : 1;
    uint64_t group_parity_error1 : 1;
    uint64_t group_parity_error0 : 1;
    uint64_t i2c_bad_status3 : 1;
    uint64_t i2c_bad_status2 : 1;
    uint64_t i2c_bad_status1 : 1;
    uint64_t i2c_bad_status0 : 1;
    uint64_t last_ret_code_prv : 3;
    uint64_t last_pib_parity_fail : 1;
    uint64_t last_completed_address : 32;
#endif // _BIG_ENDIAN
    } fields;
} pore_dbg0_t;



typedef union pore_dbg1 {

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
    uint64_t pc_last_access : 48;
    uint64_t oci_master_rd_parity_err : 1;
    uint64_t last_ret_code_oci : 3;
    uint64_t bad_instr_parity : 1;
    uint64_t invalid_instr_code : 1;
    uint64_t pc_overflow_underrun : 1;
    uint64_t bad_scan_crc : 1;
    uint64_t pc_stack_ovflw_undrn_err : 1;
    uint64_t instruction_fetch_error : 1;
    uint64_t invalid_instruction_operand : 1;
    uint64_t invalid_instruction_path : 1;
    uint64_t invalid_start_vector : 1;
    uint64_t fast_i2c_protocol_hang : 1;
    uint64_t spare : 1;
    uint64_t debug_regs_locked : 1;
#else
    uint64_t debug_regs_locked : 1;
    uint64_t spare : 1;
    uint64_t fast_i2c_protocol_hang : 1;
    uint64_t invalid_start_vector : 1;
    uint64_t invalid_instruction_path : 1;
    uint64_t invalid_instruction_operand : 1;
    uint64_t instruction_fetch_error : 1;
    uint64_t pc_stack_ovflw_undrn_err : 1;
    uint64_t bad_scan_crc : 1;
    uint64_t pc_overflow_underrun : 1;
    uint64_t invalid_instr_code : 1;
    uint64_t bad_instr_parity : 1;
    uint64_t last_ret_code_oci : 3;
    uint64_t oci_master_rd_parity_err : 1;
    uint64_t pc_last_access : 48;
#endif // _BIG_ENDIAN
    } fields;
} pore_dbg1_t;



typedef union pore_pc_stack0 {

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
    uint64_t pc_stack0 : 48;
    uint64_t _reserved0 : 11;
    uint64_t set_new_stack_pointer : 1;
    uint64_t new_stack_pointer : 4;
#else
    uint64_t new_stack_pointer : 4;
    uint64_t set_new_stack_pointer : 1;
    uint64_t _reserved0 : 11;
    uint64_t pc_stack0 : 48;
#endif // _BIG_ENDIAN
    } fields;
} pore_pc_stack0_t;



typedef union pore_pc_stack1 {

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
    uint64_t pc_stack1 : 48;
    uint64_t _reserved0 : 16;
#else
    uint64_t _reserved0 : 16;
    uint64_t pc_stack1 : 48;
#endif // _BIG_ENDIAN
    } fields;
} pore_pc_stack1_t;



typedef union pore_pc_stack2 {

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
    uint64_t pc_stack2 : 48;
    uint64_t _reserved0 : 16;
#else
    uint64_t _reserved0 : 16;
    uint64_t pc_stack2 : 48;
#endif // _BIG_ENDIAN
    } fields;
} pore_pc_stack2_t;



typedef union pore_id_flags {

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
    uint64_t reserved0 : 32;
    uint64_t pib_parity_fail : 1;
    uint64_t pib_status : 3;
    uint64_t oci_parity_fail : 1;
    uint64_t oci_status : 3;
    uint64_t reserved1 : 8;
    uint64_t ugt : 1;
    uint64_t ult : 1;
    uint64_t sgt : 1;
    uint64_t slt : 1;
    uint64_t c : 1;
    uint64_t o : 1;
    uint64_t n : 1;
    uint64_t z : 1;
    uint64_t reserved2 : 4;
    uint64_t ibuf_id : 4;
#else
    uint64_t ibuf_id : 4;
    uint64_t reserved2 : 4;
    uint64_t z : 1;
    uint64_t n : 1;
    uint64_t o : 1;
    uint64_t c : 1;
    uint64_t slt : 1;
    uint64_t sgt : 1;
    uint64_t ult : 1;
    uint64_t ugt : 1;
    uint64_t reserved1 : 8;
    uint64_t oci_status : 3;
    uint64_t oci_parity_fail : 1;
    uint64_t pib_status : 3;
    uint64_t pib_parity_fail : 1;
    uint64_t reserved0 : 32;
#endif // _BIG_ENDIAN
    } fields;
} pore_id_flags_t;



typedef union pore_data0 {

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
    uint64_t data0 : 32;
    uint64_t _reserved0 : 32;
#else
    uint64_t _reserved0 : 32;
    uint64_t data0 : 32;
#endif // _BIG_ENDIAN
    } fields;
} pore_data0_t;



typedef union pore_memory_reloc {

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
    uint64_t _reserved0 : 30;
    uint64_t memory_reloc_region : 2;
    uint64_t memory_reloc_base : 20;
    uint64_t _reserved1 : 12;
#else
    uint64_t _reserved1 : 12;
    uint64_t memory_reloc_base : 20;
    uint64_t memory_reloc_region : 2;
    uint64_t _reserved0 : 30;
#endif // _BIG_ENDIAN
    } fields;
} pore_memory_reloc_t;



typedef union pore_i2c_en_param {

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
    uint64_t i2c_engine_identifier : 4;
    uint64_t reserved0 : 1;
    uint64_t i2c_engine_address_range : 3;
    uint64_t reserved1 : 3;
    uint64_t i2c_engine_port : 5;
    uint64_t reserved2 : 1;
    uint64_t i2c_engine_device_id : 7;
    uint64_t reserved3 : 2;
    uint64_t i2c_engine_speed : 2;
    uint64_t i2c_poll_threshold : 4;
    uint64_t _reserved0 : 32;
#else
    uint64_t _reserved0 : 32;
    uint64_t i2c_poll_threshold : 4;
    uint64_t i2c_engine_speed : 2;
    uint64_t reserved3 : 2;
    uint64_t i2c_engine_device_id : 7;
    uint64_t reserved2 : 1;
    uint64_t i2c_engine_port : 5;
    uint64_t reserved1 : 3;
    uint64_t i2c_engine_address_range : 3;
    uint64_t reserved0 : 1;
    uint64_t i2c_engine_identifier : 4;
#endif // _BIG_ENDIAN
    } fields;
} pore_i2c_en_param_t;


#endif // __ASSEMBLER__
#endif // __PORE_FIRMWARE_REGISTERS_H__

