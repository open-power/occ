/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/wof/wof.h $                                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2016,2017                        */
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
#ifndef _WOF_H
#define _WOF_H



//******************************************************************************
// Define
//******************************************************************************
#define MIN_BCE_REQ_SIZE 256
#define ACTIVE_QUAD_SZ_MIN 1
#define ACTIVE_QUAD_SZ_MAX 6
#define WOF_HEADER_SIZE 32

#define WOF_RC_MODE_NO_SUPPORT_MASK                     0x0008

// Structure to hold relevant data from the WOF header in Mainstore
typedef struct __attribute__ ((packed))
{
    uint64_t magic_number;
    uint8_t size_of_vfrt;
    uint8_t vfrt_data_size;
    uint8_t active_quads_start;
    uint8_t active_quads_size;
    uint8_t vdn_start;
    uint8_t vdn_step;
    uint8_t vdn_size;
    uint8_t vdd_start;
    uint8_t vdd_step;
    uint8_t vdd_size;
} wof_header_data_t;



// Structure used in g_amec
typedef struct
{
    // Bit vector where each bit signifies a different failure case
    uint16_t wof_disabled;
} amec_wof_t;

typedef struct
{
    // There is no guarantee that we can fit everything into the min BceRequest
    // size of 128 given that there may be a need to padding in the event the
    // Main Memory address is not 128-byte aligned. The data here is 256 to
    // ensure we have enough room for any and all padding that may be needed.
    uint8_t data[MIN_BCE_REQ_SIZE];
} temp_bce_request_buffer_t __attribute ((aligned(128)));


// Parameter structure used to pass information to the copy_vfrt_to_sram
// call back function.
typedef struct
{
    temp_bce_request_buffer_t * vfrt_table;
    uint8_t pad;
} copy_vfrt_to_sram_parms_t;


//******************************************************************************
// Function Prototypes
//******************************************************************************

void wof_main( void );

uint16_t calculate_step_from_start( uint16_t i_ceff_vdx,
                                    uint8_t i_step_size,
                                    uint8_t i_min_step,
                                    uint8_t i_max_step );

uint8_t calc_quad_step_from_start( uint8_t i_num_active_quads );



uint32_t calc_vfrt_mainstore_addr( uint16_t i_vdd_step_from_start,
                            uint16_t i_vdn_step_from_start,
                            uint8_t i_quad_step_from_start );

void copy_vfrt_to_sram( copy_vfrt_to_sram_parms_t * i_parms );

void send_vfrt_to_pgpe( uint32_t i_vfrt_address );

void read_shared_sram( void );

#endif
