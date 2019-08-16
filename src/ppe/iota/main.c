/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/iota/main.c $                                         */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2019                             */
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
#include "iota.h"
#include "iota_trace.h"

/**
 * @file main.c
 * @brief example main file.  Should be overridden
 */
void func0(uint32_t idle_task_idx)
{
    static uint32_t val = 1;

    uint32_t i;

    for(i = 0; i < 10; ++i)
    {
        asm volatile("mtspr 0x110, %0" : : "r" (val++));

        iota_set_idle_task_state(IOTA_IDLE_DISABLED, idle_task_idx);
    }
}

__attribute__((weak)) void dec_handler()
{
    asm volatile("mtspr 0x110, %0" : : "r" (0xddddddddul));
}

__attribute__((weak)) void fit_handler()
{
    asm volatile("mtspr 0x110, %0" : : "r" (0xfffffffful));
}

__attribute__((weak)) void ext_handler(uint32_t task_idx)
{
    asm volatile("mtspr 0x110, %0" : : "r" (0xeeeeeeeeul));

    iota_set_idle_task_state(IOTA_IDLE_ENABLED, 1);
    out64(CME_LCL_EISR_CLR, 0x8000000000000000ull);

    asm("trap");

    iota_uih_irq_vec_restore();
}

IOTA_BEGIN_IDLE_TASK_TABLE
{ IOTA_IDLE_DISABLED, IOTA_NO_TASK },
{ IOTA_IDLE_DISABLED, IOTA_TASK(func0) }
IOTA_END_IDLE_TASK_TABLE

IOTA_BEGIN_TASK_TABLE
IOTA_TASK(ext_handler),
          IOTA_NO_TASK,
          IOTA_NO_TASK,
          IOTA_NO_TASK,
          IOTA_NO_TASK,
          IOTA_NO_TASK,
          IOTA_NO_TASK,
          IOTA_NO_TASK,
          IOTA_NO_TASK,
          IOTA_NO_TASK,
          IOTA_NO_TASK
          IOTA_END_TASK_TABLE;

__attribute__((weak)) int main()
{
    IOTA_DEC_HANDLER(dec_handler);
    IOTA_FIT_HANDLER(func0);

    PK_TRACE("Test Trace");
    PK_TRACE("Tiny Trace %d", (uint16_t)1234);
    PK_TRACE("Big trace %d", 0x12345678);

    out64(CME_LCL_EITR_OR, 0x8000000000000000ull);
    out64(CME_LCL_EIMR   , 0x7fffffffffffffffull);

    iota_run();

    return 0;
}
