/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/iota/iota.c $                                         */
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
#include "iota_ppe42.h"
#include "iota_panic_codes.h"

// Force all kernel variables into .sdata
// Order controlled by linker script
union
{
    uint64_t value;
    struct
    {
        uint32_t upper;
        uint32_t lower;
    };
} ppe42_64bit_timebase __attribute__((section(".sdata.ppe42_64bit_timebase"))) = {0};

#if defined(__PPE42A__)

iotaMachineState g_iota_machine_state_stack[(IOTA_MAX_NESTED_INTERRUPTS)] __attribute__((aligned(8))) =
{
    [ 0 ... (IOTA_MAX_NESTED_INTERRUPTS - 1) ] = IOTA_MACHINE_STATE_INIT
};
iotaMachineState*  g_iota_curr_machine_state_ptr
__attribute__((section(".sdata.g_iota_curr_machine_state_ptr")))
    = g_iota_machine_state_stack;

#else

uint32_t g_iota_curr_machine_state_ptr __attribute__((section(".sdata.g_iota_curr_machine_state_ptr"))) = 0;

#endif

uint64_t g_iota_execution_stack[IOTA_STACK_DWORD_SIZE] __attribute__((aligned(8))) =
{
    [ 0 ... (IOTA_STACK_DWORD_SIZE - 1) ] = IOTA_STACK_PATTERN
};
uint64_t g_iota_execution_stack_end __attribute__((section(".data.g_iota_execution_stack_end"))) = 0;

// Plugin default functions
iotaTimerFuncPtr g_iota_dec_handler = IOTA_TIMER_HANDLER(__iota_halt);
iotaTimerFuncPtr g_iota_fit_handler = IOTA_TIMER_HANDLER(__iota_halt);

extern uint32_t G_LOCAL_TIMEBASE_REGISTER __attribute__((section(".sdata")));
uint32_t G_LOCAL_TIMEBASE_REGISTER = LOCAL_TIMEBASE_REGISTER;

#if IOTA_IDLE_TASKS_ENABLE
void _iota_evaluate_idle_tasks()
{
    // Iterate over all idle tasks
    uint32_t idle_task_idx = 0;

    for(; idle_task_idx < g_iota_idle_task_list_size; ++idle_task_idx)
    {
        // Enabled tasks get executed
        if(g_iota_idle_task_list[idle_task_idx].state >= IOTA_IDLE_ENABLED)
        {
            if(g_iota_idle_task_list[idle_task_idx].function == IOTA_NO_TASK)
            {
                // Tried to execute a NULLPTR task
                iota_dead(IOTA_NULLPTR_TASK);
            }

            g_iota_idle_task_list[idle_task_idx].function(idle_task_idx, 0);

#if IOTA_AUTO_DISABLE_IDLE_TASKS
            uint32_t ctx = mfmsr();
            wrteei(0);

            g_iota_idle_task_list[idle_task_idx].state = IOTA_IDLE_DISABLED;

            mtmsr(ctx);
#endif
        }
    }
}
#endif  // IOTA_IDLE_TASKS_ENABLE

extern void main(void);

void _iota_boot()
{
    mtdbcr(PPE42_DBCR_INITIAL);

    // Disable timer interrupts
    mttcr(0);

#if defined(__PPE42A__)
    g_iota_curr_machine_state_ptr = g_iota_machine_state_stack;
#else
    g_iota_curr_machine_state_ptr = 0;
#endif

    mtmsr(IOTA_INITIAL_MSR);

    __hwmacro_setup(); // Configures external interrupt registers

    main();
}

void iota_run()
{
    mtmsr(IOTA_DEFAULT_MSR);
    wrteei(1);

    while(1)
    {
#if IOTA_IDLE_TASKS_ENABLE
        // Run any enabled idle tasks
        _iota_evaluate_idle_tasks();
#endif

        // Put PPE engine into wait mode (idle).
        ppe_idle();
    }
}

#if IOTA_IDLE_TASKS_ENABLE
void iota_set_idle_task_state(uint32_t state, uint32_t idle_task_idx)
{
    if(idle_task_idx < g_iota_idle_task_list_size)
    {
        // Ignore any task entries which have no task assigned...
        if(g_iota_idle_task_list[idle_task_idx].function != IOTA_NO_TASK)
        {
            // Setting/changing an idle task enable needs to be done atomically
            uint32_t ctx = mfmsr();
            wrteei(0);

            g_iota_idle_task_list[idle_task_idx].state = state;

            mtmsr(ctx);
        }
    }
}
#endif

void _iota_schedule(uint32_t schedule_reason)
{
#if defined(__PPE42A__)

    // Increment machine state pointer
    if(g_iota_curr_machine_state_ptr <
       &g_iota_machine_state_stack[IOTA_MAX_NESTED_INTERRUPTS - 1])
    {
        g_iota_curr_machine_state_ptr++;
    }
    else
    {
        iota_dead(IOTA_MACHINE_STATE_STACK_OVERFLOW);
    }

#else

    ++g_iota_curr_machine_state_ptr;
    // stack check
    uint32_t stack_ptr;
    asm volatile ("mr %0,1":"=r"(stack_ptr)::);

    if(stack_ptr <= (uint32_t)g_iota_execution_stack)
    {
        iota_dead(IOTA_MACHINE_STATE_STACK_OVERFLOW);
    }

#endif

    // Call appropriate interrupt handler here
    switch(schedule_reason)
    {
        case _IOTA_SCHEDULE_REASON_EXT:
            {
                uint32_t task_idx = __ext_irq_handler();

                if(g_iota_task_list[task_idx] != IOTA_NO_TASK)
                {
                    mtmsr(IOTA_DEFAULT_MSR);
                    g_iota_task_list[task_idx]();
                    // Interrupts are disabled once the task returns
                    wrteei(0);
                    __ext_irq_resume();
                }
                else
                {
                    iota_dead(IOTA_NULLPTR_TASK);
                }
            }

            break;

#if ENABLE_DEC_TIMER

        case _IOTA_SCHEDULE_REASON_DEC:
            g_iota_dec_handler();
            break;
#endif

#if ENABLE_FIT_TIMER

        case _IOTA_SCHEDULE_REASON_FIT:
            g_iota_fit_handler();
            break;
#endif

    }

#if IOTA_IDLE_TASKS_ENABLE
    // Check for idle tasks here
    // Rationale: if the g_iota_curr_machine_state_ptr ==
    //            g_iota_machine_state_stack,
    //            then all interrupt tasks must be completed since this is the
    //            last context about to be restored, which means enabled idle
    //            tasks can be executed
#if defined(__PPE42A__)

    if(g_iota_curr_machine_state_ptr - 1 == g_iota_machine_state_stack)
#else
    if(g_iota_curr_machine_state_ptr == 1)
#endif
    {
        uint32_t ctx = mfmsr();
        wrteei(1);
        _iota_evaluate_idle_tasks();
        mtmsr(ctx);
    }

#endif

    // Check execution stack integrity
    if(g_iota_execution_stack[0] != IOTA_STACK_PATTERN)
    {
        iota_dead(IOTA_EXECUTION_STACK_OVERFLOW);
    }

#if defined(__PPE42A__)

    // Decrement machine state pointer
    if(g_iota_curr_machine_state_ptr > g_iota_machine_state_stack)
#else
    if(g_iota_curr_machine_state_ptr != 0)
#endif
    {
        g_iota_curr_machine_state_ptr--;
    }
    else
    {
        iota_dead(IOTA_MACHINE_STATE_STACK_UNDERFLOW);
    }

}

// Assumption is that this is called more frequently than
// the local time base hw register rolls over.
uint64_t pk_timebase_get()
{
    uint32_t hw_timebase = in64(G_LOCAL_TIMEBASE_REGISTER) >> 32;

    if(ppe42_64bit_timebase.lower > hw_timebase)
    {
        ++ppe42_64bit_timebase.upper;
    }

    ppe42_64bit_timebase.lower = hw_timebase;
    return ppe42_64bit_timebase.value;
}

