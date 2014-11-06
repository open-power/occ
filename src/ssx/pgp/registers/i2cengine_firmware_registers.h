#ifndef __I2CENGINE_FIRMWARE_REGISTERS_H__
#define __I2CENGINE_FIRMWARE_REGISTERS_H__

#ifndef SIXTYFOUR_BIT_CONSTANT
#ifdef __ASSEMBLER__
#define SIXTYFOUR_BIT_CONSTANT(x) x
#else
#define SIXTYFOUR_BIT_CONSTANT(x) x##ull
#endif
#endif

#ifndef __ASSEMBLER__

// $Id: i2cengine_firmware_registers.h,v 1.1.1.1 2013/12/11 21:03:25 bcbrock Exp $

/// \file i2cengine_firmware_registers.h
/// \brief C register structs for the I2CENGINE unit

// *** WARNING *** - This file is generated automatically, do not edit.

#include <stdint.h>



typedef union i2cengine_fast_control {

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
	uint64_t with_start : 1;
	uint64_t with_address : 1;
	uint64_t read_continue : 1;
	uint64_t with_stop : 1;
	uint64_t data_length : 4;
	uint64_t device_address : 7;
	uint64_t read_not_write : 1;
	uint64_t speed : 2;
	uint64_t port_number : 5;
	uint64_t address_range : 3;
	uint64_t _reserved0 : 6;
	uint64_t data0 : 8;
	uint64_t data1 : 8;
	uint64_t data2 : 8;
	uint64_t data3 : 8;
#else
	uint64_t data3 : 8;
	uint64_t data2 : 8;
	uint64_t data1 : 8;
	uint64_t data0 : 8;
	uint64_t _reserved0 : 6;
	uint64_t address_range : 3;
	uint64_t port_number : 5;
	uint64_t speed : 2;
	uint64_t read_not_write : 1;
	uint64_t device_address : 7;
	uint64_t data_length : 4;
	uint64_t with_stop : 1;
	uint64_t read_continue : 1;
	uint64_t with_address : 1;
	uint64_t with_start : 1;
#endif // _BIG_ENDIAN
    } fields;
} i2cengine_fast_control_t;



typedef union i2cengine_fast_reset {

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
} i2cengine_fast_reset_t;



typedef union i2cengine_fast_status {

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
	uint64_t pib_address_invalid : 1;
	uint64_t pib_write_invalid : 1;
	uint64_t pib_read_invalid : 1;
	uint64_t pib_address_parity_error : 1;
	uint64_t pib_parity_error : 1;
	uint64_t lb_parity_error : 1;
	uint64_t read_data : 32;
	uint64_t _reserved0 : 6;
	uint64_t i2c_macro_busy : 1;
	uint64_t i2c_invalid_command : 1;
	uint64_t i2c_parity_error : 1;
	uint64_t i2c_back_end_overrun_error : 1;
	uint64_t i2c_back_end_access_error : 1;
	uint64_t i2c_arbitration_lost : 1;
	uint64_t i2c_nack_received : 1;
	uint64_t i2c_data_request : 1;
	uint64_t i2c_command_complete : 1;
	uint64_t i2c_stop_error : 1;
	uint64_t i2c_port_busy : 1;
	uint64_t i2c_interface_busy : 1;
	uint64_t i2c_fifo_entry_count : 8;
#else
	uint64_t i2c_fifo_entry_count : 8;
	uint64_t i2c_interface_busy : 1;
	uint64_t i2c_port_busy : 1;
	uint64_t i2c_stop_error : 1;
	uint64_t i2c_command_complete : 1;
	uint64_t i2c_data_request : 1;
	uint64_t i2c_nack_received : 1;
	uint64_t i2c_arbitration_lost : 1;
	uint64_t i2c_back_end_access_error : 1;
	uint64_t i2c_back_end_overrun_error : 1;
	uint64_t i2c_parity_error : 1;
	uint64_t i2c_invalid_command : 1;
	uint64_t i2c_macro_busy : 1;
	uint64_t _reserved0 : 6;
	uint64_t read_data : 32;
	uint64_t lb_parity_error : 1;
	uint64_t pib_parity_error : 1;
	uint64_t pib_address_parity_error : 1;
	uint64_t pib_read_invalid : 1;
	uint64_t pib_write_invalid : 1;
	uint64_t pib_address_invalid : 1;
#endif // _BIG_ENDIAN
    } fields;
} i2cengine_fast_status_t;



typedef union i2cengine_fast_data {

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
} i2cengine_fast_data_t;



typedef union i2cengine_fifo_byte {

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
	uint64_t data : 32;
	uint64_t _reserved0 : 32;
#else
	uint64_t _reserved0 : 32;
	uint64_t data : 32;
#endif // _BIG_ENDIAN
    } fields;
} i2cengine_fifo_byte_t;



typedef union i2cengine_command {

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
	uint64_t with_start : 1;
	uint64_t with_address : 1;
	uint64_t read_continue : 1;
	uint64_t not_used : 1;
	uint64_t reserved : 4;
	uint64_t device_address : 7;
	uint64_t read_not_write : 1;
	uint64_t length_bytes : 16;
	uint64_t _reserved0 : 32;
#else
	uint64_t _reserved0 : 32;
	uint64_t length_bytes : 16;
	uint64_t read_not_write : 1;
	uint64_t device_address : 7;
	uint64_t reserved : 4;
	uint64_t not_used : 1;
	uint64_t read_continue : 1;
	uint64_t with_address : 1;
	uint64_t with_start : 1;
#endif // _BIG_ENDIAN
    } fields;
} i2cengine_command_t;



typedef union i2cengine_mode {

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
	uint64_t bit_rate_divisor : 15;
	uint64_t _reserved0 : 1;
	uint64_t port_number : 6;
	uint64_t reserved : 6;
	uint64_t enhanced_mode : 1;
	uint64_t diagnostic_mode : 1;
	uint64_t pacing_allow_mode : 1;
	uint64_t wrap_mode : 1;
	uint64_t _reserved1 : 32;
#else
	uint64_t _reserved1 : 32;
	uint64_t wrap_mode : 1;
	uint64_t pacing_allow_mode : 1;
	uint64_t diagnostic_mode : 1;
	uint64_t enhanced_mode : 1;
	uint64_t reserved : 6;
	uint64_t port_number : 6;
	uint64_t _reserved0 : 1;
	uint64_t bit_rate_divisor : 15;
#endif // _BIG_ENDIAN
    } fields;
} i2cengine_mode_t;



typedef union i2cengine_watermark {

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
	uint64_t reserved : 15;
	uint64_t _reserved0 : 1;
	uint64_t high_water_mark : 4;
	uint64_t reserved1 : 4;
	uint64_t low_water_mark : 4;
	uint64_t reserved2 : 4;
	uint64_t _reserved1 : 32;
#else
	uint64_t _reserved1 : 32;
	uint64_t reserved2 : 4;
	uint64_t low_water_mark : 4;
	uint64_t reserved1 : 4;
	uint64_t high_water_mark : 4;
	uint64_t _reserved0 : 1;
	uint64_t reserved : 15;
#endif // _BIG_ENDIAN
    } fields;
} i2cengine_watermark_t;



typedef union i2cengine_interrupt_mask {

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
	uint64_t invalid_command : 1;
	uint64_t lbus_parity_error : 1;
	uint64_t back_end_overrun_error : 1;
	uint64_t back_end_access_error : 1;
	uint64_t arbitration_lost_error : 1;
	uint64_t nack_received_error : 1;
	uint64_t data_request : 1;
	uint64_t command_complete : 1;
	uint64_t stop_error : 1;
	uint64_t i2c_busy : 1;
	uint64_t not_i2c_busy : 1;
	uint64_t reserved1 : 1;
	uint64_t scl_eq_1 : 1;
	uint64_t scl_eq_0 : 1;
	uint64_t sda_eq_1 : 1;
	uint64_t sda_eq_0 : 1;
	uint64_t _reserved0 : 32;
#else
	uint64_t _reserved0 : 32;
	uint64_t sda_eq_0 : 1;
	uint64_t sda_eq_1 : 1;
	uint64_t scl_eq_0 : 1;
	uint64_t scl_eq_1 : 1;
	uint64_t reserved1 : 1;
	uint64_t not_i2c_busy : 1;
	uint64_t i2c_busy : 1;
	uint64_t stop_error : 1;
	uint64_t command_complete : 1;
	uint64_t data_request : 1;
	uint64_t nack_received_error : 1;
	uint64_t arbitration_lost_error : 1;
	uint64_t back_end_access_error : 1;
	uint64_t back_end_overrun_error : 1;
	uint64_t lbus_parity_error : 1;
	uint64_t invalid_command : 1;
	uint64_t reserved : 16;
#endif // _BIG_ENDIAN
    } fields;
} i2cengine_interrupt_mask_t;



typedef union i2cengine_interrupt_condition {

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
	uint64_t invalid_command : 1;
	uint64_t lbus_parity_error : 1;
	uint64_t back_end_overrun_error : 1;
	uint64_t back_end_access_error : 1;
	uint64_t arbitration_lost_error : 1;
	uint64_t nack_received_error : 1;
	uint64_t data_request : 1;
	uint64_t command_complete : 1;
	uint64_t stop_error : 1;
	uint64_t i2c_busy : 1;
	uint64_t not_i2c_busy : 1;
	uint64_t reserved1 : 1;
	uint64_t scl_eq_1 : 1;
	uint64_t scl_eq_0 : 1;
	uint64_t sda_eq_1 : 1;
	uint64_t sda_eq_0 : 1;
	uint64_t _reserved0 : 32;
#else
	uint64_t _reserved0 : 32;
	uint64_t sda_eq_0 : 1;
	uint64_t sda_eq_1 : 1;
	uint64_t scl_eq_0 : 1;
	uint64_t scl_eq_1 : 1;
	uint64_t reserved1 : 1;
	uint64_t not_i2c_busy : 1;
	uint64_t i2c_busy : 1;
	uint64_t stop_error : 1;
	uint64_t command_complete : 1;
	uint64_t data_request : 1;
	uint64_t nack_received_error : 1;
	uint64_t arbitration_lost_error : 1;
	uint64_t back_end_access_error : 1;
	uint64_t back_end_overrun_error : 1;
	uint64_t lbus_parity_error : 1;
	uint64_t invalid_command : 1;
	uint64_t reserved : 16;
#endif // _BIG_ENDIAN
    } fields;
} i2cengine_interrupt_condition_t;



typedef union i2cengine_interrupts {

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
	uint64_t invalid_command : 1;
	uint64_t lbus_parity_error : 1;
	uint64_t back_end_overrun_error : 1;
	uint64_t back_end_access_error : 1;
	uint64_t arbitration_lost_error : 1;
	uint64_t nack_received_error : 1;
	uint64_t data_request : 1;
	uint64_t command_complete : 1;
	uint64_t stop_error : 1;
	uint64_t i2c_busy : 1;
	uint64_t not_i2c_busy : 1;
	uint64_t reserved1 : 1;
	uint64_t scl_eq_1 : 1;
	uint64_t scl_eq_0 : 1;
	uint64_t sda_eq_1 : 1;
	uint64_t sda_eq_0 : 1;
	uint64_t _reserved0 : 32;
#else
	uint64_t _reserved0 : 32;
	uint64_t sda_eq_0 : 1;
	uint64_t sda_eq_1 : 1;
	uint64_t scl_eq_0 : 1;
	uint64_t scl_eq_1 : 1;
	uint64_t reserved1 : 1;
	uint64_t not_i2c_busy : 1;
	uint64_t i2c_busy : 1;
	uint64_t stop_error : 1;
	uint64_t command_complete : 1;
	uint64_t data_request : 1;
	uint64_t nack_received_error : 1;
	uint64_t arbitration_lost_error : 1;
	uint64_t back_end_access_error : 1;
	uint64_t back_end_overrun_error : 1;
	uint64_t lbus_parity_error : 1;
	uint64_t invalid_command : 1;
	uint64_t reserved : 16;
#endif // _BIG_ENDIAN
    } fields;
} i2cengine_interrupts_t;



typedef union i2cengine_status {

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
	uint64_t invalid_command : 1;
	uint64_t lbus_parity_error : 1;
	uint64_t back_end_overrun_error : 1;
	uint64_t back_end_access_error : 1;
	uint64_t arbitration_lost_error : 1;
	uint64_t nack_received_error : 1;
	uint64_t data_request : 1;
	uint64_t command_complete : 1;
	uint64_t stop_error : 1;
	uint64_t upper_threshold : 6;
	uint64_t _reserved0 : 1;
	uint64_t any_i2c_interrupt : 1;
	uint64_t reserved1 : 3;
	uint64_t scl_input_lvl : 1;
	uint64_t sda_input_lvl : 1;
	uint64_t i2c_port_busy : 1;
	uint64_t i2c_interface_busy : 1;
	uint64_t fifo_entry_count : 8;
	uint64_t _reserved1 : 32;
#else
	uint64_t _reserved1 : 32;
	uint64_t fifo_entry_count : 8;
	uint64_t i2c_interface_busy : 1;
	uint64_t i2c_port_busy : 1;
	uint64_t sda_input_lvl : 1;
	uint64_t scl_input_lvl : 1;
	uint64_t reserved1 : 3;
	uint64_t any_i2c_interrupt : 1;
	uint64_t _reserved0 : 1;
	uint64_t upper_threshold : 6;
	uint64_t stop_error : 1;
	uint64_t command_complete : 1;
	uint64_t data_request : 1;
	uint64_t nack_received_error : 1;
	uint64_t arbitration_lost_error : 1;
	uint64_t back_end_access_error : 1;
	uint64_t back_end_overrun_error : 1;
	uint64_t lbus_parity_error : 1;
	uint64_t invalid_command : 1;
#endif // _BIG_ENDIAN
    } fields;
} i2cengine_status_t;



typedef union i2cengine_extended_status {

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
	uint64_t table_base_addr : 32;
	uint64_t _reserved0 : 32;
#else
	uint64_t _reserved0 : 32;
	uint64_t table_base_addr : 32;
#endif // _BIG_ENDIAN
    } fields;
} i2cengine_extended_status_t;



typedef union i2cengine_residual_front_end_back_end_length {

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
	uint64_t residual_front_end_length : 16;
	uint64_t residual_back_end_length : 16;
	uint64_t _reserved0 : 32;
#else
	uint64_t _reserved0 : 32;
	uint64_t residual_back_end_length : 16;
	uint64_t residual_front_end_length : 16;
#endif // _BIG_ENDIAN
    } fields;
} i2cengine_residual_front_end_back_end_length_t;



typedef union i2cengine_immediate_reset_s_scl {

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
	uint64_t residual_front_end_length : 16;
	uint64_t residual_back_end_length : 16;
	uint64_t _reserved0 : 32;
#else
	uint64_t _reserved0 : 32;
	uint64_t residual_back_end_length : 16;
	uint64_t residual_front_end_length : 16;
#endif // _BIG_ENDIAN
    } fields;
} i2cengine_immediate_reset_s_scl_t;



typedef union i2cengine_immediate_set_s_sda {

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
	uint64_t residual_front_end_length : 16;
	uint64_t residual_back_end_length : 16;
	uint64_t _reserved0 : 32;
#else
	uint64_t _reserved0 : 32;
	uint64_t residual_back_end_length : 16;
	uint64_t residual_front_end_length : 16;
#endif // _BIG_ENDIAN
    } fields;
} i2cengine_immediate_set_s_sda_t;



typedef union i2cengine_immediate_reset_s_sda {

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
	uint64_t field : 1;
	uint64_t _reserved0 : 63;
#else
	uint64_t _reserved0 : 63;
	uint64_t field : 1;
#endif // _BIG_ENDIAN
    } fields;
} i2cengine_immediate_reset_s_sda_t;



typedef union i2cengine_fifo_word {

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
	uint64_t data : 32;
	uint64_t _reserved0 : 32;
#else
	uint64_t _reserved0 : 32;
	uint64_t data : 32;
#endif // _BIG_ENDIAN
    } fields;
} i2cengine_fifo_word_t;


#endif // __ASSEMBLER__
#endif // __I2CENGINE_FIRMWARE_REGISTERS_H__

