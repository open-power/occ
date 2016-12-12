/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/rtls/test/rtls_tables.c $                         */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
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

#include "rtls.h"

// Default task data buffers
uint32_t task_0_data = 0x00000000;
uint32_t task_1_data = 0x00000000;
uint32_t task_2_data = 0x00000000;
uint32_t task_3_data = 0x00000000;

// Alternate task data buffers
uint32_t task_0_alt_data = 0x00000000;
uint32_t task_1_alt_data = 0x00000000;
uint32_t task_2_alt_data = 0x00000000;
uint32_t task_3_alt_data = 0x00000000;

#define TASK_ID_0 0
#define TASK_ID_1 1
#define TASK_ID_2 2
#define TASK_ID_3 3
#define TASK_ID_END 4

// Test tick sequences
// For testing, run every task on every tick, but vary the order.
uint8_t G_tick0_seq[TASK_END + 1] = { TASK_ID_0, TASK_ID_1, TASK_ID_2, TASK_ID_3, TASK_END };
uint8_t G_tick1_seq[TASK_END + 1] = { TASK_ID_1, TASK_ID_2, TASK_ID_3, TASK_ID_0, TASK_END };
uint8_t G_tick2_seq[TASK_END + 1] = { TASK_ID_2, TASK_ID_3, TASK_ID_0, TASK_ID_1, TASK_END };
uint8_t G_tick3_seq[TASK_END + 1] = { TASK_ID_3, TASK_ID_0, TASK_ID_1, TASK_ID_2, TASK_END };
uint8_t G_tick4_seq[TASK_END + 1] = { TASK_ID_3, TASK_ID_2, TASK_ID_1, TASK_ID_0, TASK_END };
uint8_t G_tick5_seq[TASK_END + 1] = { TASK_ID_2, TASK_ID_1, TASK_ID_0, TASK_ID_3, TASK_END };
uint8_t G_tick6_seq[TASK_END + 1] = { TASK_ID_1, TASK_ID_0, TASK_ID_3, TASK_ID_2, TASK_END };
uint8_t G_tick7_seq[TASK_END + 1] = { TASK_ID_0, TASK_ID_3, TASK_ID_2, TASK_ID_1, TASK_END };

/* Tick Table */
uint8_t *G_tick_table[MAX_NUM_TICKS] = {
    G_tick0_seq,
    G_tick1_seq,
    G_tick2_seq,
    G_tick3_seq,
    G_tick4_seq,
    G_tick5_seq,
    G_tick6_seq,
    G_tick7_seq
};

// Example Task Table
// Use task_id_t values to index into this table and find a specific task.
task_t G_task_table[TASK_END] = {
    // flags,       func_ptr,    data_ptr,        task_id_t
    { 0x00000000, task_0_func, (void *)&task_0_data }, // TASK_ID_0
    { 0x00000000, task_1_func, (void *)&task_1_data }, // TASK_ID_1
    { 0x00000000, task_2_func, (void *)&task_2_data }, // TASK_ID_2
    { 0x00000000, task_3_func, (void *)&task_3_data }  // TASK_ID_3
};

void task_0_func(struct task *);
void task_1_func(struct task *);
void task_2_func(struct task *);
void task_3_func(struct task *);

// Function Specification
//
// Name: task_0_func
//
// Description: Example task 0
//
// End Function Specification
void task_0_func(struct task *i_self) {
    uint8_t curr_tick = (uint8_t)((MAX_NUM_TICKS - 1) & CURRENT_TICK);

    // If our task pointer & it's data pointer are valid, set a bit
    // in the data area indicating which tick this function ran during.

    if ( i_self && i_self -> data_ptr ) {
        *((uint32_t *)(i_self -> data_ptr)) |= (0x00000001 << curr_tick);
    }
    return;
}

// Function Specification
//
// Name: task_1_func
//
// Description: Example task 1
//
// End Function Specification
void task_1_func(struct task *i_self) {
    uint8_t curr_tick = (uint8_t)((MAX_NUM_TICKS - 1) & CURRENT_TICK);

    // If our task pointer & it's data pointer are valid, set a bit
    // in the data area indicating which tick this function ran during.

    if ( i_self && i_self -> data_ptr ) {
        *((uint32_t *)(i_self -> data_ptr)) |= (0x00000001 << curr_tick);
    }
    return;
}

// Function Specification
//
// Name: task_2_func
//
// Description: Example task 2
//
// End Function Specification
void task_2_func(struct task *i_self) {
    uint8_t curr_tick = (uint8_t)((MAX_NUM_TICKS - 1) & CURRENT_TICK);

    // If our task pointer & it's data pointer are valid, set a bit
    // in the data area indicating which tick this function ran during.

    if ( i_self && i_self -> data_ptr ) {
        *((uint32_t *)(i_self -> data_ptr)) |= (0x00000001 << curr_tick);
    }
    return;
}

// Function Specification
//
// Name: task_3_func
//
// Description: Example task 3
//
// End Function Specification
void task_3_func(struct task *i_self) {
    uint8_t curr_tick = (uint8_t)((MAX_NUM_TICKS - 1) & CURRENT_TICK);

    // If our task pointer & it's data pointer are valid, set a bit
    // in the data area indicating which tick this function ran during.

    if ( i_self && i_self -> data_ptr ) {
        *((uint32_t *)(i_self -> data_ptr)) |= (0x00000001 << curr_tick);
    }
    return;
}

