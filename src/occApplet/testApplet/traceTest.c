/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occApplet/testApplet/traceTest.c $                        */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2011,2014              */
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

//*************************************************************************
// Includes
//*************************************************************************
#include <common_types.h>   // imageHdr_t declaration and image header macro
#include <errl.h>           // For error handle
#include "ssx_io.h"         // For printfs
#include <trac.h>           // For traces
#include <appletId.h>       // For applet ID
#include <trac_service_codes.h>

//*************************************************************************
// Externs
//*************************************************************************
extern SsxSemaphore    g_trac_mutex;

//*************************************************************************
// Macros
//*************************************************************************
#define TRAC_INTF_MUTEX_TIMEOUT         SSX_SECONDS(5)
//*************************************************************************
// Defines/Enums
//*************************************************************************
#define TRACETEST_ID  "traceTest\0"
#define para_int_0  "0 para"
#define para_int_1  "1 para (%d)"
//#define para_int_2  "2 para (%d) (%d)"
//#define para_int_3  "3 para (%d) (%d) (%d)"
//#define para_int_4  "4 para (%d) (%d) (%d) (%d)"
#define para_int_5  "5 para (%d) (%d) (%d) (%d) (%d)"
#define para_int_6  "6 para (%d) (%d) (%d) (%d) (%d) (%d)"
#define para_hex_0  "0 para"
#define para_hex_1  "1 para (%X)"
//#define para_hex_2  "2 para (%X) (%X)"
//#define para_hex_3  "3 para (%X) (%X) (%X)"
//#define para_hex_4  "4 para (%X) (%X) (%X) (%X)"
#define para_hex_5  "5 para (%X) (%X) (%X) (%X) (%X)"
#define para_hex_6  "6 para (%X) (%X) (%X) (%X) (%X) (%X)"
#define para_chr_0  "0 para"
#define para_chr_1  "1 para (%s)"
//#define para_chr_2  "2 para (%s) (%s)"
//#define para_chr_3  "3 para (%s) (%s) (%s)"
//#define para_chr_4  "4 para (%s) (%s) (%s) (%s)"
#define para_chr_5  "5 para (%s) (%s) (%s) (%s) (%s)"
#define para_chr_6  "6 para (%s) (%s) (%s) (%s) (%s) (%s)"
#define MIN_TRACE_ENTRY_SIZE    32

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
char G_trac_buffer[TRACE_BUFFER_SIZE + 100];

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************
int traceSemTest(void)
{
    UINT l_rc = 0;

    // lock trace semaphore first
    l_rc = ssx_semaphore_pend(&g_trac_mutex, TRAC_INTF_MUTEX_TIMEOUT);
    if(SSX_OK == l_rc)
    {
        // create one trace
        TRAC_INFO("traceTest Applet: test trace semaphore");

        // Unlock trace semaphore
        ssx_semaphore_post(&g_trac_mutex);

    }
    else
    {
        l_rc = 1;
    }

    return l_rc;
}

int traceFuncTest()
{
    UINT l_rc = 0;
    UINT l_max_trace_entries = TRACE_BUFFER_SIZE / MIN_TRACE_ENTRY_SIZE;
    UINT l_entry_count = 0;
    UINT l_buffer_size = 0;
    tracDesc_t l_head = NULL;

    do
    {
        // Test target - trac_write_XXX(), TRAC_get_buffer() and TRAC_get_td()
        // This testcase would create l_max_trace_entries +1 trace entries
        // to fill trace buffer, times_wrap should be larger than zero
        do{
            l_entry_count++;
            TRAC_INFO("traceTest applet INFO record: count %d", (int)l_entry_count);
            TRAC_ERR("traceTest applet ERR record: count %d", (int)l_entry_count);
            TRAC_IMP("traceTest applet IMP record: count %d", (int)l_entry_count);
        }while(l_max_trace_entries >= l_entry_count);

        // Check times_wrap in TRAC_INFO.
        // Because structures are all the same, skip TRAC_ERR and TRAC_IMP
        l_rc = TRAC_get_buffer(TRAC_get_td("INF"), G_trac_buffer);
        l_head = (tracDesc_t)&G_trac_buffer;
        if((l_rc != 0 ) || (l_head->times_wrap == 0))
        {
            printf("Fail: times_wrap error in trace buffer: %d, %d\n", l_rc, l_head->times_wrap);
            break;
        }

        // Test target - TRAC_get_buffer() and TRAC_get_td()
        // case: invalid parameters
        l_rc = TRAC_get_buffer(TRAC_get_td("INF"), NULL);
        l_head = (tracDesc_t)&G_trac_buffer;
        if(l_rc == 0)
        {
            printf("TRAC_get_buffer(), reason code: %d\n", l_rc);
            printf("Fail: test TRAC_get_buffer() invalid 1th parameter\n");
            break;
        }

        l_rc = TRAC_get_buffer(NULL, G_trac_buffer);
        l_head = (tracDesc_t)&G_trac_buffer;
        if(l_rc == 0)
        {
            printf("TRAC_get_buffer(), reason code: %d\n", l_rc);
            printf("Fail: test TRAC_get_buffer() invalid 2nd parameter\n");
            break;
        }

        // Test target - TRAC_get_buffer_partial() and TRAC_get_td()
        // case: invalid parameters
        l_buffer_size = TRACE_BUFFER_SIZE;
        l_rc = TRAC_get_buffer_partial(NULL, G_trac_buffer, &l_buffer_size);
        if((l_rc != TRAC_INVALID_PARM) && (l_buffer_size !=0))
        {
            printf("TRAC_get_buffer_partial(), reason code: %d\n", l_rc);
            printf("Fail: test TRAC_get_buffer_partial() invalid 1st parameter\n");
            break;
        }

        l_rc = TRAC_get_buffer_partial(TRAC_get_td("UNKNOWN"), NULL, &l_buffer_size);
        if((l_rc != TRAC_INVALID_PARM) && (l_buffer_size !=0))
        {
            printf("TRAC_get_buffer_partial(), reason code: %d\n", l_rc);
            printf("Fail: test TRAC_get_buffer_partial() invalid 1st parameter\n");
            break;
        }

        l_rc = TRAC_get_buffer_partial(TRAC_get_td("INF"), NULL, &l_buffer_size);
        if((l_rc != TRAC_INVALID_PARM) && (l_buffer_size !=0))
        {
            printf("TRAC_get_buffer_partial(), reason code: %d\n", l_rc);
            printf("Fail: test TRAC_get_buffer_partial() invalid 2nd parameter\n");
            break;
        }

        l_rc = TRAC_get_buffer_partial(TRAC_get_td("ERR"), G_trac_buffer, NULL);
        if(l_rc != TRAC_INVALID_PARM)
        {
            printf("TRAC_get_buffer_partial(), reason code: %d\n", l_rc);
            printf("Fail: test TRAC_get_buffer_partial() invalid 3rd parameter\n");
            break;
        }

        // Test target - TRAC_get_buffer_partial()
        // case: input buffer less then the size of trace buffer header
        l_buffer_size = sizeof(trace_buf_head_t) - 1;
        l_rc = TRAC_get_buffer_partial(TRAC_get_td("IMP"), G_trac_buffer, &l_buffer_size);
        if(l_rc != TRAC_DATA_SIZE_LESS_THAN_HEADER_SIZE)
        {
            printf("TRAC_get_buffer_partial(), reason code: %d\n", l_rc);
            printf("Fail: test TRAC_get_buffer_partial() with illegal small input buffer\n");
            break;
        }

        // Test target - TRAC_get_buffer_partial()
        // case: input buffer is small then then trace buffer
        l_buffer_size = sizeof(trace_buf_head_t) + (TRACE_BUFFER_SIZE/4);
        l_rc = TRAC_get_buffer_partial(TRAC_get_td("INF"), G_trac_buffer, &l_buffer_size);
        if(l_rc)
        {
            printf("TRAC_get_buffer_partial(), reason code: %d\n", l_rc);
            printf("Fail: test TRAC_get_buffer_partial() with small input buffer\n");
            break;
        }

        // Test target - TRAC_get_buffer_partial()
        // case: input buffer is larger then trace buffer
        l_buffer_size = sizeof(G_trac_buffer);
        l_rc = TRAC_get_buffer_partial(TRAC_get_td("INF"), G_trac_buffer, &l_buffer_size);
        if(l_rc || (l_buffer_size != TRACE_BUFFER_SIZE))
        {
            printf("TRAC_get_buffer_partial(), reason code: %d size %d/%d\n", l_rc, l_buffer_size, TRACE_BUFFER_SIZE);
            printf("Fail: test TRAC_get_buffer_partial() with too large input buffer\n");
            break;
        }

        // Test target - TRAC_reset_buf() and TRAC_get_buffer_partial()
        // case: clear trace buffer and check with buffer larger than trace buffer
        TRAC_reset_buf();
        l_buffer_size = sizeof(G_trac_buffer);
        l_rc = TRAC_get_buffer_partial(TRAC_get_td("ERR"), G_trac_buffer, &l_buffer_size);
        if(l_rc)
        {
            printf("TRAC_get_buffer_partial(), reason code: %d\n", l_rc);
            printf("Fail: test TRAC_reset_buf()/TRAC_get_buffer_partial() with empty trace\n");
            break;
        }

        // Test target - TRAC_reset_buf() and TRAC_get_buffer_partial()
        // case: clear trace buffer and check it with buffer smaller than trace buffer
        TRAC_reset_buf();
        l_buffer_size = TRACE_BUFFER_SIZE/2;
        l_rc = TRAC_get_buffer_partial(TRAC_get_td("ERR"), G_trac_buffer, &l_buffer_size);
        if(l_rc)
        {
            printf("TRAC_get_buffer_partial(), reason code: %d\n", l_rc);
            printf("Fail: test TRAC_reset_buf()/TRAC_get_buffer_partial() with empty trace\n");
            break;
        }

        // Test target - TRAC_get_buffer_partial()
        // case: create some traces and test with large input buffer
        l_entry_count = 0;
        do{
            l_entry_count++;
            TRAC_INFO("traceTest applet INFO record: count %d", (int)l_entry_count);
            TRAC_ERR("traceTest applet ERR record: count %d", (int)l_entry_count);
            TRAC_IMP("traceTest applet IMP record: count %d", (int)l_entry_count);
        }while((l_max_trace_entries/4) >= l_entry_count);
        l_buffer_size = TRACE_BUFFER_SIZE;
        l_rc = TRAC_get_buffer_partial(TRAC_get_td("IMP"), G_trac_buffer, &l_buffer_size);
        l_head = (tracDesc_t)&G_trac_buffer;
        if(l_rc || (l_head->times_wrap != 0))
        {
            printf("TRAC_get_buffer_partial(), reason code: %d\n", l_rc);
            printf("Fail: test TRAC_get_buffer_partial() with large input buffer\n");
            break;
        }

        // Test target - TRAC_get_buffer_partial()
        // case: create some traces and test with small input buffer
        l_buffer_size = sizeof(trace_buf_head_t) + (TRACE_BUFFER_SIZE/4);
        l_rc = TRAC_get_buffer_partial(TRAC_get_td("INF"), G_trac_buffer, &l_buffer_size);
        if(l_rc)
        {
            printf("TRAC_get_buffer_partial(), reason code: %d\n", l_rc);
            printf("Fail: test TRAC_get_buffer_partial() with small input buffer\n");
            break;
        }

    }while(0);

    return l_rc;
}

//*************************************************************************
// Entry point function
//*************************************************************************
errlHndl_t traceTest(void * i_arg)
{
    errlHndl_t l_err = NULL;
    UINT l_rc = 0;

    do
    {
        // function unit test
        l_rc = traceFuncTest();
        if(l_rc)
        {
            printf("traceTest Applet: Function test failed\n");
            break;
        }

        // Macro test: test basic trace macros with/without parameters

        // int: supported
        TRAC_INFO(para_int_0);
        TRAC_INFO(para_int_1, 1);
        TRAC_INFO(para_int_5, 1, 2, 3, 4, 5);
        TRAC_INFO(para_int_6, 1, 2, 3, 4, 5, 6);

        // hex: supported
        TRAC_ERR(para_hex_0);
        TRAC_ERR(para_hex_1, 0xA);
        TRAC_ERR(para_hex_5, 0xA, 0xB, 0xC, 0xD, 0xE);
        TRAC_ERR(para_hex_6, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF);

        // char: not supported
        TRAC_IMP(para_int_0);
        TRAC_IMP(para_chr_1, "1");
        TRAC_IMP(para_chr_5, "1", "2", "3", "4", "5");
        TRAC_IMP(para_chr_6, "1", "2", "3", "4", "5", "6");

#ifdef TEST_SEMAPHORE
        // semaphore test
        l_rc = traceSemTest();
        if(l_rc)
        {
            printf("traceTest Applet: Semaphore test failed\n");
            break;
        }
#endif
    }while(0);

    printf("traceTest Applet: test finished\n");
    return l_err;
}


//*************************************************************************
// Image Header
//*************************************************************************
IMAGE_HEADER (G_traceTest,traceTest,TRACETEST_ID,OCC_APLT_TEST);
