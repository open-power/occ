/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/errl/test/errltest.c $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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

#include "ssx.h"
#include "ssx_io.h"
#include "simics_stdio.h"
#include <thread.h>
#include <threadSch.h>
#include <errl.h>
#include <rand.h>

uint8_t noncritical_stack[NONCRITICAL_STACK_SIZE];
uint8_t critical_stack[CRITICAL_STACK_SIZE];

/// Period in which to run #timer_routine
#define TIMER_INTERVAL (SsxInterval) SSX_MICROSECONDS(100)

#define SevAsciiValue( _v_ ) \
    ( ( _v_ == ERRL_SEV_INFORMATIONAL ) ? "Informational" : \
      ( _v_ == ERRL_SEV_PREDICTIVE ) ? "Predictive" : \
      ( _v_ == ERRL_SEV_UNRECOVERABLE ) ? "Unrecovrable" : \
      ( _v_ == ERRL_SEV_CALLHOME_DATA ) ? "Call Home" : \
      "Unknown" )

/*----------------------------------------------------------------------------*/
/* SsxTimer and SsxThread Declarations and Priorities                         */
/*----------------------------------------------------------------------------*/

/// Our timer based on TIMER_INTERVAL that kicks off most of the work.  See #timer_routine
SsxTimer timer;

/// Our idle thread.  See #main_thread_routine
SsxThread main_thread;

SsxThread prcd_thread;

/*----------------------------------------------------------------------------*/
/* SsxSemaphore Declarations                                                  */
/*----------------------------------------------------------------------------*/
SsxSemaphore prcd_sem;

int g_j = 0;
int g_k = 0;

SimicsStdio simics_stdout;
SimicsStdio simics_stderr;

extern void timer_routine(void *private);
extern void rtloop_ocb_init(void);

// Function Specification
//
// Name: pgp_validation_ssx_main_hook
//
// Description:
//
// End Function Specification
void pgp_validation_ssx_main_hook(void)
{

}

// Function Specification
//
// Name: Cmd_Hndl_thread_routine
//
// Description:
//
// End Function Specification
void Cmd_Hndl_thread_routine(void *arg)
{
}

// Function Specification
//
// Name: App_thread_routine
//
// Description:
//
// End Function Specification
void App_thread_routine(void *arg)
{
}

// Function Specification
//
// Name: Thermal_Monitor_thread_routine
//
// Description:
//
// End Function Specification
void Thermal_Monitor_thread_routine(void *arg)
{
}

// Function Specification
//
// Name: Hlth_Monitor_thread_routine
//
// Description:
//
// End Function Specification
void Hlth_Monitor_thread_routine(void *arg)
{
}

// Function Specification
//
// Name: FFDC_thread_routine
//
// Description:
//
// End Function Specification
void FFDC_thread_routine(void *arg)
{
}


// Function Specification
//
// Name: prcd_thread_routine
//
// Description: This thread loops as the highest priority thread, where it currently just
//
// End Function Specification
void prcd_thread_routine(void *private)
{
  while(1)
  {
    // Just sit here until this semaphore is posted, which will never happen.
    ssx_semaphore_pend(&prcd_sem, SSX_WAIT_FOREVER);

    // Only trace the first XX times that this function loops
    if(g_j < 20)
    {
      g_k = 0;
      g_j++;

    }
  }
}

// Function Specification
//
// Name: dumpLog
//
// Description:
//
// End Function Specification
void dumpLog( errlHndl_t i_log, uint32_t i_len )
{
    uint32_t    l_written = 0;
    uint32_t    l_counter = 0;
    uint8_t *   l_data = (uint8_t*) i_log;

   printf("----------%p---------- \n", i_log );

    if ( i_log == NULL )
        return;

    while ( l_counter < i_len)
    {
        printf("|   %08X     ", (uint32_t) l_data + l_counter);

        // Display 16 bytes in Hex with 2 spaces in between
        l_written = 0;
        uint8_t i = 0;
        for ( i = 0; i < 16 && l_counter < i_len; i++ )
        {
            l_written += printf("%02X",l_data[l_counter++]);

            if ( ! ( l_counter % 4 ) )
            {
                l_written += printf("  ");
            }
        }

        // Pad with spaces
        uint8_t l_space[64] = {0};
        memset( l_space, 0x00, sizeof( l_space ));
        memset( l_space, ' ', 43-l_written);
        printf("%s", l_space );

        // Display ASCII
        l_written = 0;
        uint8_t l_char;
        for ( ; i > 0 ; i-- )
        {
            l_char = l_data[ l_counter-i ];

            if ( isprint( l_char ) &&
                 ( l_char != '&' ) &&
                 ( l_char != '<' ) &&
                 ( l_char != '>' )
               )
            {
                l_written += printf("%c",l_char );
            }
            else
            {
                l_written += printf("." );
            }
        }

        // Pad with spaces
        uint8_t l_space2[64] = {0};
        memset( l_space2, 0x00, sizeof( l_space2 ));
        memset( l_space2, ' ', 19-l_written);
        printf("%s\n", l_space2 );
    }
   printf("----------%p---------- \n", i_log );

}

// Function Specification
//
// Name: ppdumpslot
//
// Description:
//
// End Function Specification
void ppdumpslot(void)
{
    errlHndl_t l_array[ERRL_MAX_SLOTS] = {
                            (errlHndl_t)G_errslot1,
                            (errlHndl_t)G_errslot2,
                            (errlHndl_t)G_errslot3,
                            (errlHndl_t)G_errslot4,
                            (errlHndl_t)G_errslot5,
                            (errlHndl_t)G_errslot6,
                            (errlHndl_t)G_errslot7,
                            (errlHndl_t)G_infoslot,
                            (errlHndl_t)G_callslot,
                            };


    printf("-------- \n");

    uint8_t l_index = 0;
    for(l_index =0; l_index < ERRL_MAX_SLOTS; l_index++)
    {
        if(l_array[l_index]->iv_version !=0)
        {
            printf("slot[%01d] sz[%04d] id[%03d] @[%p] \n",l_index,l_array[l_index]->iv_userDetails.iv_entrySize, l_array[l_index]->iv_entryId, l_array[l_index]);
        }
        else
        {
            printf("slot[%01d] [0] \n",l_index);
        }
    }
    printf("-------- \n");
}


// Function Specification
//
// Name: testsizelimit
//
// Description:
//
// End Function Specification
void testsizelimit(const tracDesc_t i_trace)
{
    printf("%s: START \n", __FUNCTION__ );
    errlHndl_t l_handle = NULL;
    errlHndl_t l_handle2 = NULL;
    errlHndl_t l_handle3 = NULL;

    l_handle = createErrl( 0x1616, 0x08, ERRL_SEV_PREDICTIVE, i_trace, 512, 0x1, 0x2);
    l_handle2 = createErrl( 0x1616, 0x08, ERRL_SEV_CALLHOME_DATA, i_trace, 512, 0x1, 0x2);
    l_handle3 = createErrl( 0x1616, 0x08, ERRL_SEV_INFORMATIONAL, i_trace, 512, 0x1, 0x2);
    errlHndl_t l_handleX = l_handle;
    errlHndl_t l_handle2X = l_handle2;
    errlHndl_t l_handle3X = l_handle3;
    printf("%s: Slots after Create - 3 slots should be used (one of each)\n", __FUNCTION__ );
    ppdumpslot();

    uint8_t l_data[ MAX_ERRL_CALL_HOME_SZ * 2 ];

    memset( l_data, 0xCC, sizeof( l_data ) );

    addUsrDtlsToErrl( l_handle, l_data, sizeof( l_data ), ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA );

    memset( l_data, 0xEE, sizeof( l_data ) );

    addUsrDtlsToErrl( l_handle2, l_data, sizeof( l_data ), ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_CALLHOME_DATA );

    memset( l_data, 0xDD, sizeof( l_data ) );

    addUsrDtlsToErrl( l_handle3, l_data, 76, ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA );

    dumpLog( l_handle, l_handle->iv_userDetails.iv_entrySize );
    dumpLog( l_handle2, l_handle2->iv_userDetails.iv_entrySize );
    dumpLog( l_handle3, l_handle3->iv_userDetails.iv_entrySize );

    commitErrl( &l_handle );
    commitErrl( &l_handle2 );
    commitErrl( &l_handle3 );
    printf("%s: Slots after Commit -  3 slots should be used/committed \n", __FUNCTION__ );
    ppdumpslot();

    deleteErrl(&l_handleX);
    deleteErrl(&l_handle2X);
    deleteErrl(&l_handle3X);
    printf("%s: Slots after delete Log - All slots should be empty\n", __FUNCTION__ );
    ppdumpslot();
    printf("%s: END \n", __FUNCTION__ );
}

// Function Specification
//
// Name: testdtlsizelimit
//
// Description:
//
// End Function Specification
void testdtlsizelimit(const tracDesc_t i_trace)
{
    printf("%s: START \n", __FUNCTION__ );
    errlHndl_t l_handle = NULL;

    l_handle = createErrl( 0x1616, 0x08, ERRL_SEV_PREDICTIVE, i_trace, 512, 0x1, 0x2);
    errlHndl_t l_handleX = l_handle;
    ppdumpslot();

    uint8_t l_data[ MAX_ERRL_CALL_HOME_SZ * 2 ];

    memset( l_data, 0xAA, sizeof( l_data ) );
    addUsrDtlsToErrl( l_handle, l_data, 256, ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA );
    //dumpLog( l_handle, l_handle->iv_userDetails.iv_entrySize );
    printf("%s: Slots after create + 256 bytes \n", __FUNCTION__ );
    ppdumpslot();

    memset( l_data, 0xBB, sizeof( l_data ) );
    addUsrDtlsToErrl( l_handle, l_data, 512, ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA );
    //dumpLog( l_handle, l_handle->iv_userDetails.iv_entrySize );
    printf("%s: Slots after create + 256 + 512 bytes \n", __FUNCTION__ );
    ppdumpslot();

    memset( l_data, 0xCC, sizeof( l_data ) );
    addUsrDtlsToErrl( l_handle, l_data, 1024, ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA );
    //dumpLog( l_handle, l_handle->iv_userDetails.iv_entrySize );
    printf("%s: Slots after create + 256 + 512 +1024 bytes \n", __FUNCTION__ );
    ppdumpslot();

    memset( l_data, 0xDD, sizeof( l_data ) );
    addUsrDtlsToErrl( l_handle, l_data, 2048, ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA );
    printf("%s: Slots after create 2k should be full with DD having only 72 bytes \n", __FUNCTION__ );
    dumpLog( l_handle, l_handle->iv_userDetails.iv_entrySize );
    ppdumpslot();


    commitErrl( &l_handle );
    deleteErrl(&l_handleX);
    printf("%s: Slots should now be empty \n", __FUNCTION__ );
    ppdumpslot();
    printf("%s: END \n", __FUNCTION__ );
}

// Function Specification
//
// Name: timetest
//
// Description:
//
// End Function Specification
void timetest(const tracDesc_t i_trace)
{
    printf("%s: START \n", __FUNCTION__ );
    errlHndl_t l_handle = NULL;
    uint64_t l_start = 0;
    uint64_t l_end = 0;

    l_start = ssx_timebase_get();
    l_handle = createErrl( 0x1716, 0x08, ERRL_SEV_CALLHOME_DATA, i_trace, 128, 0x1, 0x2);
    errlHndl_t l_handle2 = l_handle;
    commitErrl( &l_handle );
    l_end = ssx_timebase_get();
    printf("%s: Time to create/delete secs[%d] \n",__FUNCTION__, (int)SSX_MICROSECONDS(l_end-l_start));

    deleteErrl(&l_handle2);
    printf("%s: END \n", __FUNCTION__ );
}

// Function Specification
//
// Name: createcommitdeletelog
//
// Description:
//
// End Function Specification
void createcommitdeletelog(const tracDesc_t i_trace)
{
    printf("%s: START \n", __FUNCTION__ );
    errlHndl_t l_handle = NULL;
    l_handle = createErrl( 0x1616, 0x08, ERRL_SEV_CALLHOME_DATA, i_trace, 512, 0x1, 0x2);
    printf("%s: Slots after Creating call home log \n", __FUNCTION__ );
    ppdumpslot();


    errlHndl_t l_handle2 = l_handle;
    commitErrl( &l_handle );
    printf("%s: Slots after Commiting call home log \n ", __FUNCTION__ );
    dumpLog( l_handle2, l_handle2->iv_userDetails.iv_entrySize );
    ppdumpslot();

    deleteErrl(&l_handle2);
    printf("%s: Slots after delete Log \n", __FUNCTION__ );
    ppdumpslot();

    printf("%s: END \n", __FUNCTION__ );
}


// Function Specification
//
// Name: create2infologtest
//
// Description:
//
// End Function Specification
void create2infologtest(const tracDesc_t i_trace)
{
    printf("%s: START \n", __FUNCTION__ );
    printf("--------------------------------\n");
    errlHndl_t l_handle = NULL;
    errlHndl_t l_handle2= NULL;
    l_handle = createErrl( 0x1616, 0x08, ERRL_SEV_INFORMATIONAL,i_trace, 5, 0x1, 0x2);

    l_handle2 = createErrl( 0x2727, 0x19, ERRL_SEV_INFORMATIONAL, i_trace, 6, 0x2, 0x3);

    if( l_handle2 == INVALID_ERR_HNDL )
    {
        printf("%s: Creating 2 info logs PASSED, only 1 was created @ %p \n", __FUNCTION__, l_handle );
        ppdumpslot();
    }
    else
    {
        printf("%s: Creating 2 info logs FAILED, only 1 info log should be created! %p \n", __FUNCTION__, l_handle2 );
        dumpLog( l_handle, l_handle->iv_userDetails.iv_entrySize );
        dumpLog( l_handle2, l_handle2->iv_userDetails.iv_entrySize );
        ppdumpslot();
    }

    deleteErrl(&l_handle);

    printf("%s: END \n", __FUNCTION__ );
}

// Function Specification
//
// Name: createMaxLogs
//
// Description:
//
// End Function Specification
void createMaxLogs(const tracDesc_t i_trace)
{
    ERRL_SEVERITY l_sevs[4] =
    {
        ERRL_SEV_UNRECOVERABLE,
        ERRL_SEV_PREDICTIVE,
        ERRL_SEV_CALLHOME_DATA,
        ERRL_SEV_INFORMATIONAL
    };

    uint8_t l_chars[6] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };


    printf("%s: START \n", __FUNCTION__ );
    errlHndl_t l_handle = NULL;
    printf("--------------------------------\n");


    uint32_t l_index = 0;
    for(l_index =0; l_index < ERRL_MAX_SLOTS; l_index++)
    {
        uint32_t l_sev = (uint32_t)rand32(4);
        uint32_t l_trace = (uint32_t)rand32( 512 );
        l_handle = createErrl( 0x1616, 0x08, l_sevs[ l_sev ], i_trace, l_trace, 0x1, 0x2);

        if( ( l_handle != INVALID_ERR_HNDL) &&
            ( l_handle != NULL ) )
        {
            printf("Log Created @ %p with Sev[%s]\n",l_handle, SevAsciiValue( l_sevs[ l_sev ]) );

            uint8_t  l_ud = (uint8_t) rand32(2);
            if ( l_ud )
            {
                uint8_t l_data[ (uint32_t) rand32( 512 ) ];

                memset( l_data, l_chars[(uint8_t) rand32(6)], sizeof( l_data ) );
                addUsrDtlsToErrl( l_handle, l_data, sizeof(l_data), ERRL_USR_DTL_STRUCT_VERSION_1, ERRL_USR_DTL_TRACE_DATA );
            }

            commitErrl( &l_handle );
        }
        else
        {
            printf("Could not create error log with Sev[%s]\n",SevAsciiValue( l_sevs[ l_sev ]) );

            uint8_t  l_purge = (uint8_t) rand32(2);

            if ( l_purge )
            {
                uint32_t l_slots = (uint32_t)rand32(ERRL_MAX_SLOTS);

                if ((G_occErrSlots[l_slots] != INVALID_ERR_HNDL) &&
                    (G_occErrSlots[l_slots] != NULL ))
                {
                    printf("Deleting log with id[%d] @ %p (slot %d)\n", G_occErrSlots[l_slots]->iv_entryId, G_occErrSlots[l_slots], l_slots );
                    errlHndl_t l_handle = G_occErrSlots[l_slots];
                    deleteErrl( &l_handle );
                }
            }

        }

        uint8_t  l_dumpfile = (uint8_t) rand32(2);

        if ( l_dumpfile )
        {
            uint32_t l_slots = (uint32_t)rand32(ERRL_MAX_SLOTS);

            if ((G_occErrSlots[l_slots] != INVALID_ERR_HNDL) &&
                (G_occErrSlots[l_slots] != NULL ))
            {
                errlHndl_t l_handle = G_occErrSlots[l_slots];
                dumpLog( l_handle, l_handle->iv_userDetails.iv_entrySize );
            }
        }

        ppdumpslot();
    }

    printf("%s: END \n", __FUNCTION__ );
}


// Function Specification
//
// Name: testcallouts
//
// Description:
//
// End Function Specification
void testcallouts(const tracDesc_t i_trace)
{
    printf("%s: START \n", __FUNCTION__ );
    errlHndl_t l_handle = NULL;
    printf("--------------------------------\n");

    l_handle = createErrl( 0x1616, 0x08,ERRL_SEV_PREDICTIVE,i_trace, 5, 0x1, 0x2);
    errlHndl_t l_log = l_handle;

    if(l_handle != INVALID_ERR_HNDL)
    {
//             printf("Commiting log @p %p\n",l_handle);
        ERRL_CALLOUT_PRIORITY l_array[8] = {
                                             ERRL_CALLOUT_PRIORITY_HIGH,
                                             ERRL_CALLOUT_PRIORITY_MED,
                                             ERRL_CALLOUT_PRIORITY_LOW,
                                             ERRL_CALLOUT_PRIORITY_HIGH,
                                             ERRL_CALLOUT_PRIORITY_MED,
                                             ERRL_CALLOUT_PRIORITY_MED,
                                             ERRL_CALLOUT_PRIORITY_LOW,
                                             ERRL_CALLOUT_PRIORITY_LOW,
                                            };
        uint8_t l_index = 0;
        for(l_index =0; l_index < 8; l_index++)
        {
            printf("%s: current callouts %d attempting to add callout # %d with priority %d\n", __FUNCTION__, l_handle->iv_numCallouts, l_index, l_array[l_index] );
            addCalloutToErrl(l_handle,l_index,l_array[l_index]);
        }

        dumpLog( l_handle, l_handle->iv_userDetails.iv_entrySize );

        commitErrl( &l_handle );
    }
    else
    {
        printf("could not create error log with [%s]\n",SevAsciiValue(ERRL_SEV_CALLHOME_DATA));
    }

    ppdumpslot();

    deleteErrl( &l_log );

    printf("%s: END \n", __FUNCTION__ );
}

// Function Specification
//
// Name: main_thread_routine
//
// Description: This thread currently just loops as the lowest priority thread, handling
//              the lowest priority tasks.
//
// End Function Specification
void main_thread_routine(void *private)
{
  // Start the critical 250uS timer
  ssx_timer_schedule(&timer, 1, TIMER_INTERVAL);

  while(1)
  {
    // Only trace the first XX times that this function loops
    if(g_k < 3)
    {
      g_k++;

    }

    // Sleep for 1000 before we run the loop again
    ssx_sleep_absolute(1000);
  }
}


// Function Specification
//
// Name: main
//
// Description: main() currently initalizes our trace buffer along with creating threads
//              and timers for execution.  Note that once main runs ssx_start_threads, we
//              never return as the SSX kernel takes over.
//
// End Function Specification
int main(int argc, char **argv)
{

    // Initialize stdout so we can do printf from within simics env
    simics_stdout_create(&simics_stdout);
    simics_stderr_create(&simics_stderr);
    stdout = (FILE *)(&simics_stdout);
    stderr = (FILE *)(&simics_stderr);

    // Initialize SSX Stacks (note that this also reinitializes the time base to 0)
    ssx_initialize((SsxAddress)noncritical_stack, NONCRITICAL_STACK_SIZE,
                   (SsxAddress)critical_stack, CRITICAL_STACK_SIZE,
                   0);

    // TRACE: Trace buffers initialized
    tracDesc_t g_trac_main = NULL;

    timetest(g_trac_main);

    //createcommitdeletelog(g_trac_main);

    //testsizelimit( g_trac_main );

    //testdtlsizelimit( g_trac_main );

    //create2infologtest(g_trac_main);

    //createMaxLogs(g_trac_main);

    //testcallouts(g_trac_main);


    // Create Timers
    //ssx_timer_create(&timer, timer_routine, 0);

    // Create Global Semaphores
    ssx_semaphore_create(&prcd_sem, 0, 13);

    // Create Threads
    ssx_thread_create(&main_thread,
                      main_thread_routine,
                      (void *)0,
                      (SsxAddress)main_thread_stack,
                      THREAD_STACK_SIZE,
                      1);

    // Create Threads
    ssx_thread_create(&prcd_thread,
                      prcd_thread_routine,
                      (void *)0,
                      (SsxAddress)prcd_thread_stack,
                      THREAD_STACK_SIZE,
                      0);

    // Make Threads runnable
    ssx_thread_resume(&main_thread);
    ssx_thread_resume(&prcd_thread);

    // Initialize Realtime Loop Timer Interrupt
    //rtloop_ocb_init();

    // Enter SSX Kernel
    ssx_start_threads();

    return 0;
}

