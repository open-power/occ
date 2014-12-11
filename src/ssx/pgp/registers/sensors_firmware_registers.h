#ifndef __SENSORS_FIRMWARE_REGISTERS_H__
#define __SENSORS_FIRMWARE_REGISTERS_H__

// $Id: sensors_firmware_registers.h,v 1.1.1.1 2013/12/11 21:03:25 bcbrock Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/p8/working/procedures/ssx/pgp/registers/sensors_firmware_registers.h,v $
//-----------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2013
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//-----------------------------------------------------------------------------

/// \file sensors_firmware_registers.h
/// \brief C register structs for the SENSORS unit

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




typedef union sensors_v0 {

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
    uint64_t dts0 : 12;
    uint64_t thermal_trip0 : 2;
    uint64_t spare0 : 1;
    uint64_t valid0 : 1;
    uint64_t dts1 : 12;
    uint64_t thermal_trip1 : 2;
    uint64_t spare1 : 1;
    uint64_t valid1 : 1;
    uint64_t dts2 : 12;
    uint64_t thermal_trip2 : 2;
    uint64_t spare2 : 1;
    uint64_t valid2 : 1;
    uint64_t dts3 : 12;
    uint64_t thermal_trip3 : 2;
    uint64_t spare3 : 1;
    uint64_t valid3 : 1;
#else
    uint64_t valid3 : 1;
    uint64_t spare3 : 1;
    uint64_t thermal_trip3 : 2;
    uint64_t dts3 : 12;
    uint64_t valid2 : 1;
    uint64_t spare2 : 1;
    uint64_t thermal_trip2 : 2;
    uint64_t dts2 : 12;
    uint64_t valid1 : 1;
    uint64_t spare1 : 1;
    uint64_t thermal_trip1 : 2;
    uint64_t dts1 : 12;
    uint64_t valid0 : 1;
    uint64_t spare0 : 1;
    uint64_t thermal_trip0 : 2;
    uint64_t dts0 : 12;
#endif // _BIG_ENDIAN
    } fields;
} sensors_v0_t;



typedef union sensors_v1 {

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
    uint64_t dts4 : 12;
    uint64_t thermal_trip4 : 2;
    uint64_t spare4 : 1;
    uint64_t valid4 : 1;
    uint64_t dts5 : 12;
    uint64_t thermal_trip5 : 2;
    uint64_t spare5 : 1;
    uint64_t valid5 : 1;
    uint64_t dts6 : 12;
    uint64_t thermal_trip6 : 2;
    uint64_t spare6 : 1;
    uint64_t valid6 : 1;
    uint64_t dts7 : 12;
    uint64_t thermal_trip7 : 2;
    uint64_t spare7 : 1;
    uint64_t valid7 : 1;
#else
    uint64_t valid7 : 1;
    uint64_t spare7 : 1;
    uint64_t thermal_trip7 : 2;
    uint64_t dts7 : 12;
    uint64_t valid6 : 1;
    uint64_t spare6 : 1;
    uint64_t thermal_trip6 : 2;
    uint64_t dts6 : 12;
    uint64_t valid5 : 1;
    uint64_t spare5 : 1;
    uint64_t thermal_trip5 : 2;
    uint64_t dts5 : 12;
    uint64_t valid4 : 1;
    uint64_t spare4 : 1;
    uint64_t thermal_trip4 : 2;
    uint64_t dts4 : 12;
#endif // _BIG_ENDIAN
    } fields;
} sensors_v1_t;



typedef union sensors_v2 {

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
    uint64_t dts8 : 12;
    uint64_t thermal_trip8 : 2;
    uint64_t spare8 : 1;
    uint64_t valid8 : 1;
    uint64_t dts9 : 12;
    uint64_t thermal_trip9 : 2;
    uint64_t spare9 : 1;
    uint64_t valid9 : 1;
    uint64_t dts10 : 12;
    uint64_t thermal_trip10 : 2;
    uint64_t spare10 : 1;
    uint64_t valid10 : 1;
    uint64_t dts12 : 12;
    uint64_t thermal_trip12 : 2;
    uint64_t spare12 : 1;
    uint64_t valid12 : 1;
#else
    uint64_t valid12 : 1;
    uint64_t spare12 : 1;
    uint64_t thermal_trip12 : 2;
    uint64_t dts12 : 12;
    uint64_t valid10 : 1;
    uint64_t spare10 : 1;
    uint64_t thermal_trip10 : 2;
    uint64_t dts10 : 12;
    uint64_t valid9 : 1;
    uint64_t spare9 : 1;
    uint64_t thermal_trip9 : 2;
    uint64_t dts9 : 12;
    uint64_t valid8 : 1;
    uint64_t spare8 : 1;
    uint64_t thermal_trip8 : 2;
    uint64_t dts8 : 12;
#endif // _BIG_ENDIAN
    } fields;
} sensors_v2_t;



typedef union sensors_v3 {

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
    uint64_t unknown : 64;
#else
    uint64_t unknown : 64;
#endif // _BIG_ENDIAN
    } fields;
} sensors_v3_t;



typedef union sensors_v5 {

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
    uint64_t raw_cpm0 : 12;
    uint64_t spare0 : 4;
    uint64_t raw_cpm1 : 12;
    uint64_t spare1 : 4;
    uint64_t raw_cpm2 : 12;
    uint64_t spare2 : 4;
    uint64_t raw_cpm3 : 12;
    uint64_t spare3 : 4;
#else
    uint64_t spare3 : 4;
    uint64_t raw_cpm3 : 12;
    uint64_t spare2 : 4;
    uint64_t raw_cpm2 : 12;
    uint64_t spare1 : 4;
    uint64_t raw_cpm1 : 12;
    uint64_t spare0 : 4;
    uint64_t raw_cpm0 : 12;
#endif // _BIG_ENDIAN
    } fields;
} sensors_v5_t;



typedef union sensors_v6 {

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
    uint64_t raw_cpm4 : 12;
    uint64_t spare4 : 4;
    uint64_t raw_cpm5 : 12;
    uint64_t spare5 : 4;
    uint64_t raw_cpm6 : 12;
    uint64_t spare6 : 4;
    uint64_t raw_cpm7 : 12;
    uint64_t spare7 : 4;
#else
    uint64_t spare7 : 4;
    uint64_t raw_cpm7 : 12;
    uint64_t spare6 : 4;
    uint64_t raw_cpm6 : 12;
    uint64_t spare5 : 4;
    uint64_t raw_cpm5 : 12;
    uint64_t spare4 : 4;
    uint64_t raw_cpm4 : 12;
#endif // _BIG_ENDIAN
    } fields;
} sensors_v6_t;



typedef union sensors_v7 {

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
    uint64_t raw_cpm8 : 12;
    uint64_t spare8 : 4;
    uint64_t raw_cpm9 : 12;
    uint64_t spare9 : 4;
    uint64_t raw_cpm10 : 12;
    uint64_t spare10 : 4;
    uint64_t raw_cpm11 : 12;
    uint64_t spare11 : 4;
#else
    uint64_t spare11 : 4;
    uint64_t raw_cpm11 : 12;
    uint64_t spare10 : 4;
    uint64_t raw_cpm10 : 12;
    uint64_t spare9 : 4;
    uint64_t raw_cpm9 : 12;
    uint64_t spare8 : 4;
    uint64_t raw_cpm8 : 12;
#endif // _BIG_ENDIAN
    } fields;
} sensors_v7_t;



typedef union sensors_v8 {

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
    uint64_t dts0 : 12;
    uint64_t thermal_trip0 : 2;
    uint64_t spare0 : 1;
    uint64_t valid0 : 1;
    uint64_t dts1 : 12;
    uint64_t thermal_trip1 : 2;
    uint64_t spare1 : 1;
    uint64_t valid1 : 1;
    uint64_t dts2 : 12;
    uint64_t thermal_trip2 : 2;
    uint64_t spare2 : 1;
    uint64_t valid2 : 1;
    uint64_t encoded_cpm0 : 4;
    uint64_t encoded_cpm1 : 4;
    uint64_t encoded_cpm2 : 4;
    uint64_t encoded_cpm3 : 4;
#else
    uint64_t encoded_cpm3 : 4;
    uint64_t encoded_cpm2 : 4;
    uint64_t encoded_cpm1 : 4;
    uint64_t encoded_cpm0 : 4;
    uint64_t valid2 : 1;
    uint64_t spare2 : 1;
    uint64_t thermal_trip2 : 2;
    uint64_t dts2 : 12;
    uint64_t valid1 : 1;
    uint64_t spare1 : 1;
    uint64_t thermal_trip1 : 2;
    uint64_t dts1 : 12;
    uint64_t valid0 : 1;
    uint64_t spare0 : 1;
    uint64_t thermal_trip0 : 2;
    uint64_t dts0 : 12;
#endif // _BIG_ENDIAN
    } fields;
} sensors_v8_t;



typedef union sensors_v9 {

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
    uint64_t dts4 : 12;
    uint64_t thermal_trip4 : 2;
    uint64_t spare4 : 1;
    uint64_t valid4 : 1;
    uint64_t dts5 : 12;
    uint64_t thermal_trip5 : 2;
    uint64_t spare5 : 1;
    uint64_t valid5 : 1;
    uint64_t dts6 : 12;
    uint64_t thermal_trip6 : 2;
    uint64_t spare6 : 1;
    uint64_t valid6 : 1;
    uint64_t encoded_cpm4 : 4;
    uint64_t encoded_cpm5 : 4;
    uint64_t encoded_cpm6 : 4;
    uint64_t encoded_cpm7 : 4;
#else
    uint64_t encoded_cpm7 : 4;
    uint64_t encoded_cpm6 : 4;
    uint64_t encoded_cpm5 : 4;
    uint64_t encoded_cpm4 : 4;
    uint64_t valid6 : 1;
    uint64_t spare6 : 1;
    uint64_t thermal_trip6 : 2;
    uint64_t dts6 : 12;
    uint64_t valid5 : 1;
    uint64_t spare5 : 1;
    uint64_t thermal_trip5 : 2;
    uint64_t dts5 : 12;
    uint64_t valid4 : 1;
    uint64_t spare4 : 1;
    uint64_t thermal_trip4 : 2;
    uint64_t dts4 : 12;
#endif // _BIG_ENDIAN
    } fields;
} sensors_v9_t;



typedef union sensors_v10 {

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
    uint64_t dts8 : 12;
    uint64_t thermal_trip8 : 2;
    uint64_t spare8 : 1;
    uint64_t valid8 : 1;
    uint64_t dts9 : 12;
    uint64_t thermal_trip9 : 2;
    uint64_t spare9 : 1;
    uint64_t valid9 : 1;
    uint64_t dts10 : 12;
    uint64_t thermal_trip10 : 2;
    uint64_t spare10 : 1;
    uint64_t valid10 : 1;
    uint64_t encoded_cpm8 : 4;
    uint64_t encoded_cpm9 : 4;
    uint64_t encoded_cpm10 : 4;
    uint64_t encoded_cpm11 : 4;
#else
    uint64_t encoded_cpm11 : 4;
    uint64_t encoded_cpm10 : 4;
    uint64_t encoded_cpm9 : 4;
    uint64_t encoded_cpm8 : 4;
    uint64_t valid10 : 1;
    uint64_t spare10 : 1;
    uint64_t thermal_trip10 : 2;
    uint64_t dts10 : 12;
    uint64_t valid9 : 1;
    uint64_t spare9 : 1;
    uint64_t thermal_trip9 : 2;
    uint64_t dts9 : 12;
    uint64_t valid8 : 1;
    uint64_t spare8 : 1;
    uint64_t thermal_trip8 : 2;
    uint64_t dts8 : 12;
#endif // _BIG_ENDIAN
    } fields;
} sensors_v10_t;



typedef union sensors_v11 {

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
    uint64_t dvs0 : 12;
    uint64_t spare00 : 1;
    uint64_t trip0 : 1;
    uint64_t spare01 : 1;
    uint64_t valid0 : 1;
    uint64_t dvs1 : 12;
    uint64_t spare10 : 1;
    uint64_t trip1 : 1;
    uint64_t spare11 : 1;
    uint64_t valid1 : 1;
    uint64_t dvs2 : 12;
    uint64_t spare20 : 1;
    uint64_t trip2 : 1;
    uint64_t spare21 : 1;
    uint64_t valid2 : 1;
    uint64_t dvs3 : 12;
    uint64_t spare30 : 1;
    uint64_t trip3 : 1;
    uint64_t spare31 : 1;
    uint64_t valid3 : 1;
#else
    uint64_t valid3 : 1;
    uint64_t spare31 : 1;
    uint64_t trip3 : 1;
    uint64_t spare30 : 1;
    uint64_t dvs3 : 12;
    uint64_t valid2 : 1;
    uint64_t spare21 : 1;
    uint64_t trip2 : 1;
    uint64_t spare20 : 1;
    uint64_t dvs2 : 12;
    uint64_t valid1 : 1;
    uint64_t spare11 : 1;
    uint64_t trip1 : 1;
    uint64_t spare10 : 1;
    uint64_t dvs1 : 12;
    uint64_t valid0 : 1;
    uint64_t spare01 : 1;
    uint64_t trip0 : 1;
    uint64_t spare00 : 1;
    uint64_t dvs0 : 12;
#endif // _BIG_ENDIAN
    } fields;
} sensors_v11_t;



typedef union sensors_v12 {

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
    uint64_t dvs4 : 12;
    uint64_t spare40 : 1;
    uint64_t trip4 : 1;
    uint64_t spare41 : 1;
    uint64_t valid4 : 1;
    uint64_t dvs5 : 12;
    uint64_t spare50 : 1;
    uint64_t trip5 : 1;
    uint64_t spare51 : 1;
    uint64_t valid5 : 1;
    uint64_t dvs6 : 12;
    uint64_t spare60 : 1;
    uint64_t trip6 : 1;
    uint64_t spare61 : 1;
    uint64_t valid6 : 1;
    uint64_t dvs7 : 12;
    uint64_t spare70 : 1;
    uint64_t trip7 : 1;
    uint64_t spare71 : 1;
    uint64_t valid7 : 1;
#else
    uint64_t valid7 : 1;
    uint64_t spare71 : 1;
    uint64_t trip7 : 1;
    uint64_t spare70 : 1;
    uint64_t dvs7 : 12;
    uint64_t valid6 : 1;
    uint64_t spare61 : 1;
    uint64_t trip6 : 1;
    uint64_t spare60 : 1;
    uint64_t dvs6 : 12;
    uint64_t valid5 : 1;
    uint64_t spare51 : 1;
    uint64_t trip5 : 1;
    uint64_t spare50 : 1;
    uint64_t dvs5 : 12;
    uint64_t valid4 : 1;
    uint64_t spare41 : 1;
    uint64_t trip4 : 1;
    uint64_t spare40 : 1;
    uint64_t dvs4 : 12;
#endif // _BIG_ENDIAN
    } fields;
} sensors_v12_t;



typedef union sensors_v13 {

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
    uint64_t dvs8 : 12;
    uint64_t spare80 : 1;
    uint64_t trip8 : 1;
    uint64_t spare81 : 1;
    uint64_t valid8 : 1;
    uint64_t dvs9 : 12;
    uint64_t spare90 : 1;
    uint64_t trip9 : 1;
    uint64_t spare91 : 1;
    uint64_t valid9 : 1;
    uint64_t dvs10 : 12;
    uint64_t spare100 : 1;
    uint64_t trip10 : 1;
    uint64_t spare101 : 1;
    uint64_t valid10 : 1;
    uint64_t dvs11 : 12;
    uint64_t spare110 : 1;
    uint64_t trip11 : 1;
    uint64_t spare111 : 1;
    uint64_t valid11 : 1;
#else
    uint64_t valid11 : 1;
    uint64_t spare111 : 1;
    uint64_t trip11 : 1;
    uint64_t spare110 : 1;
    uint64_t dvs11 : 12;
    uint64_t valid10 : 1;
    uint64_t spare101 : 1;
    uint64_t trip10 : 1;
    uint64_t spare100 : 1;
    uint64_t dvs10 : 12;
    uint64_t valid9 : 1;
    uint64_t spare91 : 1;
    uint64_t trip9 : 1;
    uint64_t spare90 : 1;
    uint64_t dvs9 : 12;
    uint64_t valid8 : 1;
    uint64_t spare81 : 1;
    uint64_t trip8 : 1;
    uint64_t spare80 : 1;
    uint64_t dvs8 : 12;
#endif // _BIG_ENDIAN
    } fields;
} sensors_v13_t;


#endif // __ASSEMBLER__
#endif // __SENSORS_FIRMWARE_REGISTERS_H__

