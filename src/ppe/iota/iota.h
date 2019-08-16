/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/iota/iota.h $                                         */
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
#ifndef __IOTA_H__
#define __IOTA_H__

#ifndef __IOTA__
    #define __IOTA__ 1
#endif

#ifdef USE_APP_CFG_H
    #include "iota_app_cfg.h"
#endif

#ifndef APPCFG_TRACE
    #define APPCFG_TRACE PK_TRACE
#endif

#ifndef APPCFG_PANIC
    #define APPCFG_PANIC IOTA_PANIC
#endif

#ifndef __ASSEMBLER__

#include <stdint.h>
#include <stddef.h>
#include "iota_ppe42.h"


// Default for IOTA_IDLE_TASKS_ENABLE is no idle task support
#if !defined(IOTA_IDLE_TASKS_ENABLE)
    #define IOTA_IDLE_TASKS_ENABLE 0
#endif

/**
 * Run the main IOTA kernel loop
 * @note does not return
 */
void iota_run()  __attribute__((noreturn));

#define iota_halt() __iota_halt()
#define iota_dead(code) IOTA_PANIC((code))

#if IOTA_IDLE_TASKS_ENABLE
    /**
    * Set the state of an idle task
    * @param[in] The state
    * @param[in[ The task index in the idle task table
    */
    void iota_set_idle_task_state(uint32_t state, uint32_t idle_task_idx);
#endif

/// IOTA --- CONSTANTS
#if !defined(IOTA_STACK_PATTERN)
    #define IOTA_STACK_PATTERN  0xfeedfeedfeedfeedull
#endif

#if !defined(IOTA_64U_ARR_INIT)
    #define IOTA_64U_ARR_INIT   0x6666666666666666ull
#endif

#if !defined(IOTA_32U_ARR_INIT)
    #define IOTA_32U_ARR_INIT   0x66666666ul
#endif

#if !defined(IOTA_08U_ARR_INIT)
    #define IOTA_08U_ARR_INIT   0x66u
#endif

#define IOTA_NO_TASK        ((iotaTaskFuncPtr)(IOTA_32U_ARR_INIT))
#define IOTA_IDLE_DISABLED  0x00000000
#define IOTA_IDLE_ENABLED   0x00000001

#if !defined(IOTA_INITIAL_MSR)
    #define IOTA_INITIAL_MSR (MSR_ME | MSR_IS0 | MSR_IS1 | MSR_IS2 | MSR_IPE)
#endif

#if !defined(IOTA_DEFAULT_MSR)
#define IOTA_DEFAULT_MSR    (MSR_UIE | MSR_ME | MSR_IS0 | \
                             MSR_IS1 | MSR_IS2 | MSR_IPE)
#endif

///

/// IOTA -- CONVENIENCE MACROS
// This below macro-fu is an adaptation of a similar macro in the Linux kernel
// (BUILD_BUG_ON). Allows compile time checking of conditions using sizeof()
#define IOTA_TOKEN_PASTE0(a,b) a ## b
#define IOTA_TOKEN_PASTE1(a,b) IOTA_TOKEN_PASTE0(a,b)
#define IOTA_COMPILE_TIME_UNIQUE \
    IOTA_TOKEN_PASTE1(_iota_compile_unique_,__COUNTER__)
#define IOTA_COMPILE_TIME_CHECK(condition) \
    extern int IOTA_COMPILE_TIME_UNIQUE [!!(condition)-1];

#define compile_assert(name,e) \
    enum { compile_assert__##name = 1/(e) };

#define SECTION_SBSS __attribute__((section(".sbss")))
#define SECTION_SDATA __attribute__((section(".sdata")))
#define SECTION(a) __attribute__((section(a)))

// IOTA Plugin functions
#define IOTA_TASK(function) ((iotaTaskFuncPtr)function)
#define IOTA_TIMER_HANDLER(function) ((iotaTimerFuncPtr)function)


#define IOTA_DEC_HANDLER(function) g_iota_dec_handler \
        = IOTA_TIMER_HANDLER(function);

#define IOTA_FIT_HANDLER(function) g_iota_fit_handler \
        = IOTA_TIMER_HANDLER(function);

#if IOTA_IDLE_TASKS_ENABLE

#define IOTA_BEGIN_IDLE_TASK_TABLE \
    iotaIdleTask g_iota_idle_task_list[] \
    SECTION(".sdata.g_iota_idle_task_list") = {
#define IOTA_END_IDLE_TASK_TABLE \
    }; \
    uint32_t const g_iota_idle_task_list_size \
        = (uint32_t)(sizeof(g_iota_idle_task_list)/sizeof(iotaIdleTask));
#endif

#define IOTA_BEGIN_TASK_TABLE \
    iotaTaskFuncPtr g_iota_task_list[] SECTION(".sdata.g_iota_task_list") = {
#define IOTA_END_TASK_TABLE \
    }; \
    uint32_t const g_iota_task_list_size \
        = (uint32_t)(sizeof(g_iota_task_list)/sizeof(iotaTaskFuncPtr)); \
    IOTA_COMPILE_TIME_CHECK((sizeof(g_iota_task_list)/sizeof(iotaTaskFuncPtr)) \
                            == (IOTA_NUM_EXT_IRQ_PRIORITIES));

#define IOTA_MACHINE_STATE_INIT { \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT, \
        IOTA_32U_ARR_INIT \
    }
///

typedef struct
{
#if defined(__PPE42A__)
    uint32_t GPR0;
    uint32_t GPR1;
    uint32_t GPR4;
    uint32_t GPR5;
    uint32_t GPR6;
    uint32_t GPR7;
    uint32_t GPR8;
    uint32_t GPR9;
    uint32_t GPR28;
    uint32_t GPR29;
    uint32_t GPR30;
    uint32_t GPR31;
    uint32_t SRR0;
    uint32_t SRR1;
    uint32_t LR;
    uint32_t CR;
    uint32_t GPR3;
    uint32_t GPR10;
    uint32_t CTR;
    uint32_t XER; // needs to be 8B aligned
#else
    uint32_t back_chain;
    uint32_t next_lr;
    uint32_t CR;
    uint32_t SPRG0;
    uint32_t GPR0;
    uint32_t GPR1;
    uint32_t GPR3;
    uint32_t GPR4;
    uint32_t GPR5;
    uint32_t GPR6;
    uint32_t GPR7;
    uint32_t GPR8;
    uint32_t GPR9;
    uint32_t GPR10;
    uint32_t XER;
    uint32_t CTR;
    uint32_t SRR0;
    uint32_t SRR1;
    uint32_t GPR28;
    uint32_t GPR29;
    uint32_t GPR30;
    uint32_t GPR31;
#endif
} iotaMachineState;

typedef void (*iotaTaskFuncPtr )( );
typedef void (*iotaTimerFuncPtr)(void);

typedef struct
{
    uint32_t        state;
    iotaTaskFuncPtr function;
} iotaIdleTask;

#if defined(__PPE42A__)

    #define IOTA_STACK_DWORD_SIZE ((IOTA_EXECUTION_STACK_SIZE/8)+1)
    extern iotaMachineState   g_iota_machine_state_stack[];
    extern iotaMachineState* g_iota_curr_machine_state_ptr;

#else

    #define MACHINE_STATE_CONTEXT_SIZE  sizeof(iotaMachineState)
    #define MACHINE_STATE_STACK_SIZE (IOTA_MAX_NESTED_INTERRUPTS * MACHINE_STATE_CONTEXT_SIZE)
    #define IOTA_STACK_DWORD_SIZE ((IOTA_EXECUTION_STACK_SIZE + MACHINE_STATE_STACK_SIZE)/8 + 1)

    extern uint32_t g_iota_curr_machine_state_ptr;

#endif

// IF IOTA_IDLE_TASKS_ENABLE,
// This variable just becomes an interrupt level count to
// determine when idle tasks can run.
extern iotaTaskFuncPtr    g_iota_task_list[];
extern uint32_t const     g_iota_task_list_size;
extern uint64_t           g_iota_execution_stack[];
extern uint64_t           g_iota_execution_stack_end;

#if IOTA_IDLE_TASKS_ENABLE
    extern iotaIdleTask       g_iota_idle_task_list[];
    extern uint32_t const     g_iota_idle_task_list_size;
    #if !defined(__PPE42A__)
        // This variable just becomes an interrupt level count to determine when
        // idle tasks can run.
        extern uint32_t g_iota_curr_machine_state_ptr;
    #endif
#endif

void _iota_boot();
void _iota_schedule(uint32_t schedule_reason);
void _iota_evaluate_idle_tasks();

extern void __iota_halt() __attribute__((noreturn));

extern iotaTimerFuncPtr g_iota_dec_handler;
extern iotaTimerFuncPtr g_iota_fit_handler;
extern void __hwmacro_setup(void);

extern uint32_t __ext_irq_handler(void);
extern void     __ext_irq_resume(void);

// To satisfy pk_trace
typedef uint64_t PkTimebase;
PkTimebase pk_timebase_get(void);

#endif // !__ASSEMBLER__

#endif //__IOTA_H__
