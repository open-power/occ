/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/common/ocmb_mem_data.h $                                  */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2018                        */
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

/* This header file is used by both occ_405 and occ_gpe1.                 */
/* Contains common structures and globals.                                */
#if !defined(__OCMB_MEM_DATA_H__)
#define __OCMB_MEM_DATA_H__

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t ubdts0_err      : 1;
        uint16_t ubdts0_valid    : 1;
        uint16_t ubdts0_present  : 1;
        uint16_t memdts0_err     : 1;
        uint16_t memdts0_valid   : 1;
        uint16_t memdts0_present : 1;
        uint16_t memdts1_err     : 1;
        uint16_t memdts1_valid   : 1;
        uint16_t memdts1_present : 1;
        uint16_t event           : 1;
        uint16_t initial_packet0 : 1;
        uint16_t reserved        : 5;

    } fields;
} ocmb_status_t;

typedef union
{
    uint32_t value;
    struct
    {
        uint32_t ubdts0_err      : 1;
        uint32_t ubdts0_valid    : 1;
        uint32_t ubdts0_present  : 1;
        uint32_t memdts0_err     : 1;
        uint32_t memdts0_valid   : 1;
        uint32_t memdts0_present : 1;
        uint32_t memdts1_err     : 1;
        uint32_t memdts1_valid   : 1;
        uint32_t memdts1_present : 1;
        uint32_t memdts2_err     : 1;
        uint32_t memdts2_valid   : 1;
        uint32_t memdts2_present : 1;
        uint32_t memdts3_err     : 1;
        uint32_t memdts3_valid   : 1;
        uint32_t memdts3_present : 1;
        uint32_t event           : 1;
        uint32_t initial_packet0 : 1;
        uint32_t reserved        : 15;
    } fields;
} ocmb_status_ddr5_t;

typedef union
{
    uint32_t value;
    struct
    {
        uint32_t initial_packet1 : 1;
        uint32_t reserved2       : 7;
        uint32_t reserved1       : 24;
    } fields;
} ocmb_pkt1_status_t;

typedef union
{
    uint32_t value;
    struct
    {
        uint32_t reserve            : 24;
        uint32_t self_timed_refresh : 8;
    } fields;
} ocmb_self_timed_refresh_t;

typedef struct
{
    ocmb_status_t status;        // 16 bit status
    uint16_t ubdts0;             // Membuf thermal sensor
    uint16_t memdts[2];          // dimm0/1 thermal sensor
    uint32_t mba_rd;             // MBA reads
    uint32_t mba_wr;             // MBA writes

    uint32_t mba_act;            // MBA activations
    uint32_t mba_powerups;
    uint8_t  self_timed_refresh;  // lp2_exists
    uint8_t  reserved1;
    uint16_t reserved2;
    uint32_t frame_count;

    uint32_t mba_arr_cnt_base;
    uint32_t mba_arr_cnt_low;
    uint32_t mba_arr_cnt_med;
    uint32_t mba_arr_cnt_high;

    ocmb_pkt1_status_t status1;
    uint32_t reserved4;
    uint64_t reserved5;
} OcmbMemData;

typedef struct
{
    ocmb_status_ddr5_t status;   // 32 bit status
    uint16_t ubdts0;             // Membuf thermal sensor
    uint16_t memdts[4];          // dts0/1/2/3 thermal sensors
    uint16_t reserved1;

    uint32_t side0_rd;           // reads
    uint32_t side0_wr;           // writes
    uint32_t side0_act;          // activations
    uint32_t side0_powerups;

    ocmb_pkt1_status_t status1;  // 32 bit status
    ocmb_self_timed_refresh_t  side0_self_timed_refresh;
    ocmb_self_timed_refresh_t  side1_self_timed_refresh;
    uint32_t reserved2;

    uint32_t side1_rd;             // reads
    uint32_t side1_wr;             // writes
    uint32_t side1_act;            // activations
    uint32_t side1_powerups;
} OcmbMemDataDDR5;

#endif
