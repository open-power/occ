/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/pgp_vrm.h $                                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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
#ifndef __PGP_VRM_H__
#define __PGP_VRM_H__

/// \file pgp_vrm.h
/// \brief Header for PgP VRM device drivers.  Some constants are also held in
/// pgp_common.h.

#include "pgp_common.h"


#ifndef __ASSEMBLER__

/// VRM Command Header
///
/// This structure defines the values written on the SPI interface for
/// 'read' commands.

//typedef union {
//    uint64_t value;
//    uint32_t word[2];
//    struct {
//      uint64_t address : 4;
//      uint64_t command : 4;
//    } fields;
//} vrm_command_t;


/// VRM Write Transaction Command 
///
/// The 8-bit \a phase_enable is an 8-bit VRM-11 VID code 
///
/// The 8-bit \a vcs_offset is an 8-bit signed offset

typedef union {
    uint64_t value;
    uint32_t word[2];
    struct {
        uint64_t command      : 4;
        uint64_t phase_enable : 4;
        uint64_t vdd_vid      : 8;
        uint64_t vcs_offset   : 8;
        uint64_t master_crc   : 8;
    } fields;
} vrm_write_transaction_t;
        

/// VRM Write Transaction Response
/// writes status is duplicated 3x for the minority detect feature.

typedef union {
    uint64_t value;
    uint32_t word[2];
    struct {
        uint64_t write_status0 : 8;
        uint64_t write_status1 : 8;
        uint64_t write_status2 : 8;
        uint64_t optional_crc  : 8;
    } fields;
} vrm_write_resp_t;



/// VRM Read State Command
///  reserved field should be sent as 0s

typedef union {
    uint64_t value;
    uint32_t word[2];
    struct {
        uint64_t command    : 4;
        uint64_t reserved   : 20;
        uint64_t master_crc : 8;
    } fields;
} vrm_read_state_t;
        

/// VRM Read State Response
/// Results are duplicated 3x for the minority detecte feature.

typedef union {
    uint64_t value;
    uint32_t word[2];
    struct {
        uint64_t read_ready0    : 1;
        uint64_t minus_nplus1_0 : 1;
        uint64_t minus_n0       : 1;
        uint64_t reserved1_0    : 1;
        uint64_t vrm_fan0       : 1;
        uint64_t vrm_overtemp0  : 1;
        uint64_t reserved2_0    : 2;
        uint64_t read_ready1    : 1;
        uint64_t minus_nplus1_1 : 1;
        uint64_t minus_n1       : 1;
        uint64_t reserved1_1    : 1;
        uint64_t vrm_fan1       : 1;
        uint64_t vrm_overtemp1  : 1;
        uint64_t reserved2_1    : 2;
        uint64_t read_ready2    : 1;
        uint64_t minus_nplus1_2 : 1;
        uint64_t minus_n2       : 1;
        uint64_t reserved1_2    : 1;
        uint64_t vrm_fan2       : 1;
        uint64_t vrm_overtemp2  : 1;
        uint64_t reserved2_2    : 2;
        uint64_t slave_crc      : 8;
    } fields;
} vrm_read_state_resp_t;

/// VRM Read Voltage Command
///  reserved field should be sent as 0s

typedef union {
    uint64_t value;
    uint32_t word[2];
    struct {
        uint64_t command    : 4;
        uint64_t rail       : 4;
        uint64_t reserved   : 1;
        uint64_t master_crc : 8;
    } fields;
} vrm_read_voltage_t;
        

/// VRM Read Voltage Response
/// Results are duplicated 3x for the minority detect feature.

typedef union {
    uint64_t value;
    uint32_t word[2];
    struct {
        uint64_t vid0 : 8;
        uint64_t vid1 : 8;
        uint64_t vid2 : 8;
        uint64_t slave_crc : 8;
    } fields;
} vrm_read_voltage_resp_t;


/// VRM Read Current 1 Response
///
/// The 16-bit current readings come from 12-bit DACS; the DAC output is
/// right-padded with 0b0000.  The current units are 0.025 Ampere.

//typedef union {
//    uint64_t value;
//    uint32_t word[2];
//    struct {
//      uint64_t header : 8;
//      uint64_t read_not_ready : 1;
//      uint64_t minus_nplus1 : 1;
//      uint64_t minus_n : 1;
//      uint64_t reserved0 : 1;
//      uint64_t vrm_fan : 1;
//      uint64_t vrm_overtemp : 1;
//      uint64_t reserved1 : 2;
//      uint64_t vdd_current : 16;
//      uint64_t vcs_current : 16;
//      uint64_t vio_current : 16;
//   } fields;
//} vrm_read_current_1_t;


/// VRM Read Current 2 Response
///
/// The 16-bit current readings come from 12-bit DACS; the DAC output is
/// right-padded with 0b0000.  The current units are 0.025 Ampere.

//typedef union {
//    uint64_t value;
//    uint32_t word[2];
//    struct {
//      uint64_t header : 8;
//      uint64_t read_not_ready : 1;
//      uint64_t minus_nplus1 : 1;
//      uint64_t minus_n : 1;
//      uint64_t reserved0 : 1;
//      uint64_t vrm_fan : 1;
//      uint64_t vrm_overtemp : 1;
//      uint64_t reserved1 : 2;
//      uint64_t spare1_current : 16;
//      uint64_t spare2_current : 16;
//      uint64_t spare3_current : 16;
//  } fields;
//} vrm_read_current_2_t;

#endif  /* __ASSEMBLER__ */

// These are the default values for the SPIVRM/O2S interface

#define SPIVRM_BITS 71          /* Actual # of bits minus 1 */
#define SPIVRM_CPOL 0           /* Clock polarity */
#define SPIVRM_CPHA 0           /* Clock phase */
#define SPIVRM_FREQUENCY_HZ 16000000 /* 16 MHz */
#define SPIVRM_ENABLE_ECC 1
#define SPIVRM_NPORTS 3         /* Maximum # of ports supported by HW */
#define SPIVRM_NRAILS 2         /* Maximum # of rails supported by read voltage cmd*/
#define SPIVRM_ENABLED_PORTS 0x4 /* 3 bit mask, left justified */
#define SPIVRM_PHASES 15        /* System dependent */

/// Convert an integer index into a VRM designator (mask)
#define SPIVRM_PORT(i) (1 << (SPIVRM_NPORTS - (i) - 1))

// SPIVRM specific setup defaults

#define SPIVRM_READ_STATUS_DELAY 48 /* Cycles, system dependent */
#define SPIVRM_ADDRESS 0        /* First 4 bits of SPIVRM packet */

// Default values for the O2S bridge

#define O2S_BRIDGE_ENABLE 1
#define O2S_READ_DELAY 48       /* Cycles, system dependent */
#define O2S_ADDRESS 0           /* First 4 bits of O2S packet */


#endif  /* __PGP_VRM_H__ */
