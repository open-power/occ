/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occApplet/testApplet/apsstest.c $                         */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2011,2014              */
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
#include <apss.h>           // APSS Interfaces
#include <appletId.h>       // For applet ID
#include <trac.h>           // For traces

//*************************************************************************
// Externs
//*************************************************************************
extern PoreEntryPoint GPE_pore_nop; // Sleep for specified amount of time...

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
#define APSSTESTMAIN_ID  "apsstest1\0"

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

// Function Specification
//
// Name: apssTestMain
//
// Description:
//
// End Function Specification
errlHndl_t apssTestMain(void * i_arg)
{
  APSS_DBG("Running apssTestMain\n");
  errlHndl_t l_err = NULL;
  task_apss_start_pwr_meas(NULL);

  // Schedule GPE program to delay to ensure the data is available... (BLOCKING)
  // bad: 48, good: 56
  PoreFlex test_request;
  APSS_DBG("apss_test_pwr_meas: delay...\n");
  pore_flex_create(&test_request,
                   &G_pore_gpe0_queue,
                   (void*)GPE_pore_nop,      // entry_point
                   (uint32_t)56,             // entry_point argument
                   SSX_WAIT_FOREVER,         // no timeout
                   NULL,                     // callback,
                   NULL,                     // callback arg
                   ASYNC_REQUEST_BLOCKING);  // options
  pore_flex_schedule(&test_request);
  APSS_DBG("apss_test_pwr_meas: delay complete\n");

  task_apss_continue_pwr_meas(NULL);

  task_apss_complete_pwr_meas(NULL);

  APSS_DBG("Done apssTestMain\n");

  return l_err;
}

/*****************************************************************************/
// Image Header
/*****************************************************************************/
// call macro with Applet ID arg
IMAGE_HEADER (G_apssTestMain,apssTestMain,APSSTESTMAIN_ID,OCC_APLT_TEST);

