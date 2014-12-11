#ifndef __FASTI2C_FIRMWARE_REGISTERS_H__
#define __FASTI2C_FIRMWARE_REGISTERS_H__

#ifndef SIXTYFOUR_BIT_CONSTANT
#ifdef __ASSEMBLER__
#define SIXTYFOUR_BIT_CONSTANT(x) x
#else
#define SIXTYFOUR_BIT_CONSTANT(x) x##ull
#endif
#endif

#ifndef __ASSEMBLER__

// $Id: fasti2c_firmware_registers.h,v 1.1.1.1 2013/12/11 21:03:23 bcbrock Exp $

/// \file fasti2c_firmware_registers.h
/// \brief C register structs for the FASTI2C unit

// *** WARNING *** - This file is generated automatically, do not edit.

#include <stdint.h>



typedef union fasti2c_control {

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
} fasti2c_control_t;



typedef union fasti2c_reset {

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
} fasti2c_reset_t;



typedef union fasti2c_status {

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
} fasti2c_status_t;



typedef union fasti2c_data {

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
} fasti2c_data_t;



typedef union fasti2c_ecc_start {

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
} fasti2c_ecc_start_t;



typedef union fasti2c_ecc_stop {

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
} fasti2c_ecc_stop_t;


#endif // __ASSEMBLER__
#endif // __FASTI2C_FIRMWARE_REGISTERS_H__

