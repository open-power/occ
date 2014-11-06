/******************************************************************************
// @file apsstest.c
// @brief APSS test applet
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section apsstest.c APSSTEST.c
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      cjcain    08/30/2011  created
 *   @02                abagepa   10/03/2011  updated task signature
 *   @dw000             dwoodham  12/12/2011  Update call to IMAGE_HEADER macro
 *   @rc001             rickylie  01/10/2012  Change DEBUG_PRINTF to APSS_DBG
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
//*************************************************************************
// Includes
//*************************************************************************
#include <common_types.h>   // imageHdr_t declaration and image header macro
#include <errl.h>           // For error handle
#include "ssx_io.h"         // For printfs
#include <apss.h>           // APSS Interfaces
#include <appletId.h>       // For applet ID @dw000a
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
// Flow:              FN=None
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
// @dw000 - call macro with Applet ID arg
IMAGE_HEADER (G_apssTestMain,apssTestMain,APSSTESTMAIN_ID,OCC_APLT_TEST);

