/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/rtls/test/main.c $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2014                        */
/* [+] Google Inc.                                                        */
/* [+] International Business Machines Corp.                              */
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

#include "ssx.h"
#include "ssx_io.h"
#include "simics_stdio.h"
#include <thread.h>
#include <errl.h>
#include <rand.h>
#include <rtls.h>
#include <appletId.h>  // For applet ID

extern void __ssx_boot;

IMAGE_HEADER (G_mainAppImageHdr,__ssx_boot,MAIN_APP_ID,OCC_APLT_TEST);

// Period in which to run #timer_routine
#define TIMER_INTERVAL (SsxInterval) SSX_MICROSECONDS(5000)

int g_j = 0;
int g_k = 0;

extern uint32_t G_run_mask;

// Globals from local copy of rtls_tables.c:
extern task_t G_task_table[TASK_END];  // Global task table

extern uint32_t task_0_data; // Default task data buffers
extern uint32_t task_1_data;
extern uint32_t task_2_data;
extern uint32_t task_3_data;

extern uint32_t task_0_alt_data; // Alternate task data buffers
extern uint32_t task_1_alt_data;
extern uint32_t task_2_alt_data;
extern uint32_t task_3_alt_data;

SimicsStdio simics_stdout;
SimicsStdio simics_stderr;


/*----------------------------------------------------------------------------*/
/* Critical/Non-Critical stack                                                */
/*----------------------------------------------------------------------------*/
uint8_t noncritical_stack[NONCRITICAL_STACK_SIZE];
uint8_t critical_stack[CRITICAL_STACK_SIZE];
uint8_t test_thread_stack[THREAD_STACK_SIZE];

SsxThread test_thread;
SsxTimer G_test_timer;

// Inits the IRQ handler.
extern void rtl_ocb_init( void );

// Run the IRQ handler for at least MAX_NUM_TICKS, then disable it again.
int rtl_run_max_ticks(void);

// Utility clears the data buffers in all tasks in the global task table.
void clear_task_data_bufs(void);

// Utility clears the flags in all tasks in the global task table.
void clear_task_flags(void);

// Utility writes the flags in the specified task to the specified value.
void write_task_flags(task_id_t i_task_id, uint32_t new_flags);

// Utility to dump flags & data for all tasks in G_task_table, along with
// the current values of G_run_mask and CURRENT_TICK.
void dump_global_state(void);

// Tests the API that reads & writes the global & task run flags.
void test_flags_api(void);

// Tests the function ('rtl_stop_task') that indicates a task is NOT ready to run.
void test_task_stop_api(void);

// Tests the function ('rtl_set_task_data') that assigns a new data buffer to a
// task in the global task list.
void test_set_data_api(void);

// Thread routine that runs all our tests.
void test_thread_routine(void *private);

// The main routine.
int main(int argc, char **argv);

// Function Specification
//
// Name: rtl_run_max_ticks
//
// Description:
// Run the RTLS IRQ handler for at least MAX_NUM_TICKS, then stop.
// Note: This function does not guarantee that the IRQ handler will run for
// exactly MAX_NUM_TICKS ticks.  It does guarantee that the IRQ handler will
// run for _at_least_ MAX_NUM_TICKS ticks (but could run for more).
//
// End Function Specification

int rtl_run_max_ticks(void)
{
// If we can loop this many cycles without seeing our IRQ stop condition,
// then something is broken.
#define MAX_WAIT_CYCLES 10000

    int  i = 0;  // Counter
    int rc = 0;  // Return Code

    // Start the RTLS IRQ timer.
    ssx_irq_enable( PGP_IRQ_OCC_TIMER0 );

    // Wait for the timer to hit MAX_NUM_TICKS ticks.
    // Note: When we enter this loop, CURRENT_TICK may still equal 0xFFFFFFFF,
    // so handle that condition appropriately.
    for( i = 0; i < MAX_WAIT_CYCLES; i++) {

        // Warning: ssx_sleep only works if called from a thread OTHER than the one that
        // calls ocb_timer_setup (function called by rtl_ocb_init).  If this rule is not
        // followed, the OCB IRQ handler will run a trap instruction and hang.
        ssx_sleep(1);

        if( (CURRENT_TICK >= MAX_NUM_TICKS) && (CURRENT_TICK < 0xFFFFFFFF) ) {
            ssx_irq_disable(PGP_IRQ_OCC_TIMER0);
            CURRENT_TICK = 0xFFFFFFFF;
            break;
        }
    }

    if( i >= MAX_WAIT_CYCLES ) {
        // We timed-out waiting for CURRENT_TICK to equal MAX_NUM_TICKS.  Make noise & exit.
        printf("  ERROR: %s: %d: Timed-out waiting for CURRENT_TICK to reach %d.\n\n", __FILE__, __LINE__, MAX_NUM_TICKS);
        rc = -1;
    }
    return rc;
}


// Function Specification
//
// Name: clear_task_data_bufs
//
// Description:
// Utility clears the data buffers in all tasks in the global task table.
//
// End Function Specification

void clear_task_data_bufs(void)
{
    int i;

    for( i = 0; i < TASK_END; i++ ) {
        *((uint32_t *)(G_task_table[i].data_ptr)) = 0x00000000;
    }
}


// Function Specification
//
// Name: clear_task_flags
//
// Description:
// Utility clears the flags in all tasks in the global task table.
// Note: This sets all task flags to 0.
//
// End Function Specification

void clear_task_flags(void)
{
    int i;

    for( i = 0; i < TASK_END; i++ ) {
        G_task_table[i].flags = 0x00000000;
    }
}

// Function Specification
//
// Name: write_task_flags
//
// Description:
// Utility writes the flags in the specified task to the specified value.
// Note: This overwrites the prior flags value for the given task.
//
// End Function Specification

void write_task_flags(task_id_t i_task_id, uint32_t new_flags)
{
    if( i_task_id >= TASK_END ) {
        printf("Warning: %s: write_task_flags: Invalid task ID: %02x.  Not overwriting flags.\n", __FILE__, i_task_id);
    }
    else {
        G_task_table[i_task_id].flags = new_flags;
    }
}


// Function Specification
//
// Name: dump_global_state
//
// Description:
// Utility to dump flags & data for all tasks in G_task_table, along with
// the current values of G_run_mask and CURRENT_TICK.
//
// End Function Specification

void dump_global_state(void)
{
    int i;
    printf("\n  G_run_mask = [0x%04x] ; CURRENT_TICK = [0x%04x]\n", G_run_mask, CURRENT_TICK);

    for( i = 0; i < TASK_END; i++ ) {
        printf("  TASK_ID_%02d : flags [0x%08x] ; data_p [0x%p] ; data [0x%08x]\n", \
                i, G_task_table[i].flags, G_task_table[i].data_ptr, *((uint32_t *)(G_task_table[i].data_ptr)) );
    }
    printf("\n");
}


// Function Specification
//
// Name: test_1
//
// Description:
// This test is to stop ocb timer after complete 1 loop
//
// End Function Specification

void test_1()
{
    // Announce which test is running.
    printf("\ntest_1:\n");

    // re-enable interrupt so RTL will continue again (CURRENT_TICK = 0xFFFFFFFF)
    printf("  Test_1_1: starting ocb timer\n");

    rtl_run_max_ticks();

    printf("  Test_1_1: exiting subroutine.\n");
    return;
}


// Function Specification
//
// Name: test_2
//
// Description:
// This test to set and clear the global run mask interfaces
//
// End Function Specification
void test_2()
{
    // Announce which test is running.
    printf("\ntest_2:\n");

    //test rtl_set_run_mask() interface
    G_run_mask = 0x00000000;
    printf("  Test_2_1: G_run_mask = 0x%08x before setting\n", G_run_mask);

    rtl_set_run_mask( RTL_FLAG_ACTIVE | RTL_FLAG_OBS );
    printf("  Test_2_2: G_run_mask = 0x%08x after setting\n", G_run_mask);

    //test rtl_clr_run_mask() interface
    G_run_mask = RTL_FLAG_MSTR | RTL_FLAG_NOTMSTR | RTL_FLAG_ACTIVE |
                 RTL_FLAG_OBS | RTL_FLAG_RST_REQ |RTL_FLAG_NO_APSS ;
    printf("  Test_2_3: G_run_mask = 0x%08x before clearing\n", G_run_mask);

    rtl_clr_run_mask( RTL_FLAG_ACTIVE | RTL_FLAG_OBS );
    printf("  Test_2_4: G_run_mask = 0x%08x after clearing\n", G_run_mask);

    return;
}

// Function Specification
//
// Name: test_flags_api
//
// Description: Tests the global & task run flags, including the API that
// reads & writes these flags.
//
// End Function Specification
void test_flags_api(void)
{
    int i = 0;  // Counter
    BOOLEAN l_result = FALSE;

    // Local copy of new run mask (odd-numbered bits set).
    uint32_t l_odd_run_mask = RTL_FLAG_MSTR | RTL_FLAG_OBS | RTL_FLAG_RST_REQ;

    // Partial inverse of l_odd_run_mask (even-numbered bits set, except for the 'run' flag: bit 0).
    uint32_t l_even_run_mask = RTL_FLAG_NOTMSTR | RTL_FLAG_ACTIVE | RTL_FLAG_NO_APSS;

    // Announce which test is running.
    printf("\ntest_flags_api:\n");

    // Clear all task data & flags.
    clear_task_data_bufs();
    clear_task_flags();

    // Use API to set 'run' flag + odd-numbered bits in global flags mask.
    rtl_clr_run_mask(0xFFFFFFFF);
    rtl_set_run_mask(RTL_FLAG_RUN | l_odd_run_mask);

    // Set 1/2 the tasks to run w/same mask as global, and the other 1/2 of the
    // tasks to run w/opposite mask from the global.
    for (i = 0; i < TASK_END; i++)
    {
        // Use i as i_task_id
        if (i % 2)
        {
            // Task ID is odd
            write_task_flags((task_id_t) i, l_odd_run_mask);
        }
        else
        {
            // Task ID is even
            write_task_flags((task_id_t) i, l_even_run_mask);
        }
    }

    // Use API to set all tasks as runnable.
    for (i = 0; i < TASK_END; i++)
    {
        rtl_start_task((task_id_t) i);
    }

    // Use API to check which tasks are runnable.
    // Expect all tasks to report as runnable.
    printf("  Before IRQ enable.  Using \'odd\' global mask.\n");
    printf("  Expect: All tasks report as runnable.\n");
    printf("  Actual: Tasks reporting as runnable: ");

    for (i = 0; i < TASK_END; i++)
    {
        l_result = rtl_task_is_runnable((task_id_t) i);

        if (l_result)
        {
            printf("%d ", i);
        }
    }
    printf("\n");
    dump_global_state();

    // Run the rtls for MAX_NUM_TICKS, then stop.
    rtl_run_max_ticks();

    // Dump task data, task flags, global flags and CURRENT_TICK.
    printf("  After IRQ ran.\n");
    printf("  Expect: Only tasks using the \'odd\' flags mask actually ran.\n");
    printf("  Actual: Task state:\n");
    dump_global_state();

    // Clear all task data, but not the task flags.
    clear_task_data_bufs();

    // Use the API to reverse the sense of the global flags only.
    rtl_clr_run_mask(0xFFFFFFFF);
    rtl_set_run_mask(RTL_FLAG_RUN | l_even_run_mask);

    // Use API to check which tasks are runnable.
    // Expect all tasks to report as runnable.
    printf("  Before IRQ enable.  Using \'even\' global mask.\n");
    printf("  Expect: All tasks report as runnable.\n");
    printf("  Actual: Tasks reporting as runnable: ");

    for (i = 0; i < TASK_END; i++) {

        l_result = rtl_task_is_runnable((task_id_t) i);
        printf("%d ", i);
    }
    printf("\n");
    dump_global_state();

    // Run the rtls for MAX_NUM_TICKS, then stop.
    rtl_run_max_ticks();

    // Dump task data, task flags, global flags and CURRENT_TICK.
    printf("  After IRQ ran.\n");
    printf("  Expect: Only tasks using the \'even\' flags mask actually ran.\n");
    printf("  Actual: Task state:\n");
    dump_global_state();

    return;
}


// Function Specification
//
// Name: test_task_stop_api
//
// Description: Tests the API function that indicates a task is NOT ready
// to run: 'rtl_stop_task'.
//
// End Function Specification

void test_task_stop_api(void)
{
    int i = 0;  // Counter
    BOOLEAN l_result = FALSE;

    // Local copy of new run mask (odd-numbered bits set).
    uint32_t l_odd_run_mask = RTL_FLAG_MSTR | RTL_FLAG_OBS | RTL_FLAG_RST_REQ;

    // Announce which test is running.
    printf("\ntest_task_stop_api:\n");

    // Clear all task data & flags.
    clear_task_data_bufs();
    clear_task_flags();

    // Use API to set 'run' flag + odd-numbered bits in global flags mask.
    rtl_clr_run_mask(0xFFFFFFFF);
    rtl_set_run_mask(RTL_FLAG_RUN | l_odd_run_mask);

    // Set other task flags so all task flags == global flag mask.
    // Use the API to set the RTL_FLAG_RUN flags in all tasks.
    for (i = 0; i < TASK_END; i++) {
        // Use i as i_task_id
        write_task_flags((task_id_t) i, l_odd_run_mask);
        rtl_start_task((task_id_t) i);
    }

    // Now use the API to "stop" ~1/2 the tasks (clear RTL_FLAG_RUN).
    for (i = 0; i < TASK_END; i++) {
        // Use i as i_task_id

        if ( (i % 2) == 0 ) {
            // Task ID is even
            rtl_stop_task((task_id_t) i);
        }
    }

    // Use API to check which tasks are runnable.  Expect 1/2 the tasks are runnable.
    printf("  Before IRQ runs.\n");
    printf("  Expect: Half of the tasks report as runnable.\n");
    printf("  Actual: Tasks reporting as runnable: ");

    for (i = 0; i < TASK_END; i++) {

        l_result = rtl_task_is_runnable((task_id_t) i);

        if (l_result) { printf("%d ", i); }
    }
    printf("\n");
    dump_global_state();

    // Run the rtls for MAX_NUM_TICKS, then stop.
    rtl_run_max_ticks();

    // Dump task data, task flags, global flags and CURRENT_TICK.
    printf("  After IRQ ran.\n");
    printf("  Expect: \'Runnable\' tasks ran; \'non-runnable\' tasks did not run.\n");
    printf("  Actual: Task state:\n");
    dump_global_state();
    return;
}


// Function Specification
//
// Name: test_set_data_apii
//
// Description: Tests the API function that assigns a new data buffer to a
// task in the global task list: 'rtl_set_task_data'.
//
// End Function Specification

void test_set_data_api(void)
{
    int i = 0;  // Counter
    BOOLEAN l_result = FALSE;

    uint32_t *l_task_1_old_data_p = NULL;  // Copy of original task 1 data ptr
    uint32_t *l_task_3_old_data_p = NULL;  // Copy of original task 3 data ptr

    // Local copy of new run mask (odd-numbered bits set).
    uint32_t l_odd_run_mask = RTL_FLAG_MSTR | RTL_FLAG_OBS | RTL_FLAG_RST_REQ;

    // Announce which test is running.
    printf("\ntest_set_data_api:\n");

    // Clear all task data & flags.
    clear_task_data_bufs();
    clear_task_flags();

    // Use API to set global flags.
    rtl_clr_run_mask(0xFFFFFFFF);
    rtl_set_run_mask(RTL_FLAG_RUN | l_odd_run_mask);

    printf("  Before task data pointers re-assigned.\n");
    dump_global_state();

    // Save-off old data ptrs for tasks 1 & 3.
    // Use API to re-assign data ptrs for ~1/2 of the tasks.
    // Remember: The task data pointer won't get changed if
    // RTL_FLAG_RUN is set in the task's flags mask.
    l_task_1_old_data_p = G_task_table[1].data_ptr;
    l_task_3_old_data_p = G_task_table[3].data_ptr;

    rtl_set_task_data(TASK_ID_1, &task_1_alt_data);
    rtl_set_task_data(TASK_ID_3, &task_3_alt_data);

    // To be sure we start clean, clear the task data buffers again.
    clear_task_data_bufs();

    // Use API to set task flags so all tasks will run.
    for (i = 0; i < TASK_END; i++) {
        // Use i as i_task_id
        write_task_flags((task_id_t) i, l_odd_run_mask);
        rtl_start_task((task_id_t) i);
    }

    // Use API to check which tasks are runnable.  Expect all tasks are runnable.
    printf("  After task data pointers re-assigned; before IRQ runs.\n");
    printf("  Expect: All tasks will report as runnable.\n");
    printf("  Actual: Tasks reporting as runnable: ");

    for (i = 0; i < TASK_END; i++) {

        l_result = rtl_task_is_runnable((task_id_t) i);

        if (l_result) { printf("%d ", i); }
    }
    printf("\n");
    dump_global_state();

    // Run the rtls for MAX_NUM_TICKS, then stop.
    rtl_run_max_ticks();

    // Dump task data, task flags, global flags and CURRENT_TICK.
    // Dump the old task data (the ones that were de-assigned).
    // Expect: New task data buffers indicate the tasks ran.  Old task data buffers are still cleared.
    printf("  After IRQ ran.\n");
    printf("  Expect: All tasks will have run.  Old task data buffers will still be clear.\n");
    printf("  Actual: Task state:\n");
    dump_global_state();
    printf("  TASK_ID_1 : old data_p: [0x%p] ; old data [0x%08x]\n", l_task_1_old_data_p, *l_task_1_old_data_p);
    printf("  TASK_ID_3 : old data_p: [0x%p] ; old data [0x%08x]\n", l_task_3_old_data_p, *l_task_3_old_data_p);
    printf("\n");

    // Unless you want to report a "task busy" error when you un-do the data ptr assignments,
    // clear your task flags here (mainly just need to clear the run flag).
    clear_task_flags();

    // Now un-do data ptr re-assignments.
    rtl_set_task_data(TASK_ID_1, &task_1_data);
    rtl_set_task_data(TASK_ID_3, &task_3_data);

    return;
}


// Function Specification
//
// Name: test_thread_routine
//
// Description:
// We run our tests in a thread so that, when a test re-enables the OCB timer for the RTLS,
// the timer will actually interrupt the test, and run until it hits MAX_NUM_TICKS and
// is re-disabled.  At that point, the test resumes its activity and dumps the results
// of the RTLS IRQ's operation.
//
// End Function Specification

void test_thread_routine(void *private)
{
    test_1();
    test_2();
    test_flags_api();
    test_task_stop_api();
    test_set_data_api();

    // Uncomment if you want to re-enable the OCB timer for further testing
    // after this test suite exits.
    // ssx_irq_enable( PGP_IRQ_OCC_TIMER0 );
}

// Function Specification
//
// Name: main
//
// Description: Entry point for running our tests.
// Sets up our OCB timer and our test thread, then kicks them both off.
//
// End Function Specification

int main(int argc, char **argv)
{
    // Initialize Trace Buffers immediately, so they can be used
    // from this point on.

    // Initialize stdout so we can do printf from within simics env
    simics_stdout_create(&simics_stdout);
    simics_stderr_create(&simics_stderr);
    stdout = (FILE *)(&simics_stdout);
    stderr = (FILE *)(&simics_stderr);

    // Initialize SSX Stacks (note that this also reinitializes the time base to 0)
    ssx_initialize((SsxAddress)noncritical_stack, NONCRITICAL_STACK_SIZE,
           (SsxAddress)critical_stack, CRITICAL_STACK_SIZE,
           0);

    // Create our test thread
    ssx_thread_create(&test_thread,                 // Thread control block (struct)
                      test_thread_routine,          // Thread function
                      (void *)0,                    // Thread function arg
                      (SsxAddress)test_thread_stack,// Stack to use for this thread
                      THREAD_STACK_SIZE,            // Size of this thread's stack
                      THREAD_PRIORITY_0);           // This thread's priority (0 is highest)

    // Make the test thread runnable
    ssx_thread_resume(&test_thread);

    // Start rtl code
    rtl_ocb_init();

    // Stop the interrupt handler and initialize CURRENT_TICK (just in case).
    // Also set the 'testing' flag.
    ssx_irq_disable(PGP_IRQ_OCC_TIMER0);
    CURRENT_TICK = 0xFFFFFFFF;
    printf("\n\nmain: ocb timer stopped; waiting for first test\n");

    // Enter SSX Kernel
    ssx_start_threads();

    return 0;
}

