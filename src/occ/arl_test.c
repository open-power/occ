/******************************************************************************
// @file arl_test.c
// @brief OCC main arl_test
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section arl_test.c ARL_TEST.C
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      mware     10/21/2011  Created initial version.
 *   @mw010             mware     10/23/2011  Added code to inject per thread 
 *                                            SCOM data to test per thread sensors on P8.
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *
 *  @endverbatim
 *
 *///*************************************************************************/

//*************************************************************************
// Includes
//*************************************************************************
#include "ssx.h"
#include "ssx_io.h"
#include "simics_stdio.h"
#include "arl_test_data.h"

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

//*************************************************************************
// Strustures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************
#ifdef AMEC_ARL_TEST_COMPILE
sOCCData_t gOCC = {0};
sRTLData_t gRTL = {0};

sOCCData_t * gpOCC = &gOCC;
sRTLData_t * gpRTL = &gRTL;
#endif

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

// Function Specification
//
// Name:  arl_test
//
// Description:  Perform any needed mode changes
//
// Flow:              FN=None
//
// End Function Specification
void arl_test(void)
{
#ifdef AMEC_ARL_TEST_COMPILE
  UINT8    i = 0;
  UINT32   temp32  = 0;     
  UINT32   temp32a = 0;
  UINT16   temp16 = 0;
  UINT16   temp16a = 0;


  //gpRTL->cycles250usincrement = CYCLES_INC;      // hard coded to correspond to 4GHz and the # of cycles every 250usec
  gpRTL->cycles250us = gpRTL->cycles250us + gpRTL->cycles250usincrement;
  
  // Inject raw cycle counter (currently based on 4GHz core frequency)
  gpOCC->SCOM_ARRAY[INDEX_PC_RAW]=gpRTL->cycles250us;
  
  // Compute differential in raw PC cycles
  temp32 = gpOCC->SCOM_ARRAY[INDEX_PC_RAW];
  temp32a = gpRTL->scom_prev_PC_RAW_250us; 
  temp32 = temp32 - temp32a;   

  // Convert to frequency for this core
  temp32 = temp32 << 8;                         // Shift left 8 bits (raw value is < 2^24)
  temp32a = 64000;                              // Divide by 64000 to convert to frequency sensor in 1MHz increments
  temp32 = temp32 / temp32a;
  gpRTL->FREQ250USP0C0=(UINT16)temp32;          // Write to frequency sensor
  

  // $mw010  Begin: Debug code to inject data into SCOMs for per thread information

  for (i=0; i<MAX_THREADS; i++)
     {       
        temp32 = gpRTL->utilcounter[i]+gpRTL->utilincrement[i];        
     	gpRTL->utilcounter[i]=temp32;
        gpOCC->SCOM_ARRAY[INDEX_PC_RUN_T0+i]=temp32;
     }

  

  // $mw010  End: Debug code to inject data into SCOMs for per thread information

  /////////////////////////////////////////////////////////////////////////
  // Begin: 2ms state machine
  ///////////////////////////////////////////////////////////////////////// 

    switch (gpOCC->AME_sm_cnt)
    {

    //////////////////////////////////////////////////////////////////////////
    // Begin:  Perform State 0 of EMPATH 2msec State Machine 
    //////////////////////////////////////////////////////////////////////////
    case 0x00:

     // This state processes core 0's per thread utilization SCOMs

     // $mw010  Begin: Code to read SCOM_ARRAY and convert into actual per thread utilization sensors

     // First step is to calculate how many raw cycles went by in the core during the last 2msec 

     temp32 = gpOCC->SCOM_ARRAY[INDEX_PC_RAW];
     temp32a = gpRTL->scom_prev_PC_RAW_2ms; 
     gpRTL->cycles2ms = temp32 - temp32a;                    // Compute 32 bit differential between SCOM reads now and previous 2msec

     for (i=0; i<MAX_THREADS; i++)
        {  
           temp32 = gpOCC->SCOM_ARRAY[INDEX_PC_RUN_T0+i];    // Read 32 bit SCOM value for this thread (free running) 
           temp32a = gpRTL->scom_prev_thread[i];             // Read previous 32 bit SCOM value from 32msec ago
	       temp32 = temp32 - temp32a;           // Compute 32 bit differential between SCOM reads now and previous 2msec
		   temp32 = temp32 >> 8;                // Limit to 16 bits (highest count is < 2^24)
		   temp16 = (UINT16)temp32;
		   temp16a = 10000;                     // 10000 = 100.00%
		   temp32 = ((UINT32)temp16a)*((UINT32)temp16);   // scale by 10000
		   temp32a = gpRTL->cycles2ms;
		   temp32a = temp32a >> 8;              // Limit to 16 bits 
		   temp32 = temp32 / temp32a;
		   gpRTL->util2ms_thread[i]=(UINT16)temp32;
        } 


    // Last step is to remember current SCOMs in scom_prev_thread array for next time
    for (i=0; i<MAX_THREADS; i++)
       {       
          gpRTL->scom_prev_thread[i] = gpOCC->SCOM_ARRAY[INDEX_PC_RUN_T0+i];
       }

    gpRTL->scom_prev_PC_RAW_2ms = gpOCC->SCOM_ARRAY[INDEX_PC_RAW];   // Capture raw cycle count for usage in the next 2msec

     // $mw010  End: Code to read SCOM_ARRAY and convert into actual per thread utilization sensors


        break;

    //////////////////////////////////////////////////////////////////////////
    // Begin:  Perform State 1 of EMPATH 2msec State Machine 
    //////////////////////////////////////////////////////////////////////////
    case 0x01:

        break;

    //////////////////////////////////////////////////////////////////////////
    // Begin:  Perform State 2 of EMPATH 2msec State Machine 
    //////////////////////////////////////////////////////////////////////////
    case 0x02:

        break;

    //////////////////////////////////////////////////////////////////////////
    // Begin:  Perform State 3 of EMPATH 2msec State Machine 
    //////////////////////////////////////////////////////////////////////////
    case 0x03:

        break;

    //////////////////////////////////////////////////////////////////////////
    // Begin:  Perform State 4 of EMPATH 2msec State Machine 
    //////////////////////////////////////////////////////////////////////////
    case 0x04:

        break;

    //////////////////////////////////////////////////////////////////////////
    // Begin:  Perform State 5 of EMPATH 2msec State Machine 
    //////////////////////////////////////////////////////////////////////////
    case 0x05:

        break;

    //////////////////////////////////////////////////////////////////////////
    // Begin:  Perform State 6 of EMPATH 2msec State Machine 
    //////////////////////////////////////////////////////////////////////////
    case 0x06:

        break;

    //////////////////////////////////////////////////////////////////////////
    // Begin:  Perform State 7 of EMPATH 2msec State Machine 
    //////////////////////////////////////////////////////////////////////////
    case 0x07:

        break;

    }
	
	 
  // Capture raw cycle count to use in next 250us	
  gpRTL->scom_prev_PC_RAW_250us = gpOCC->SCOM_ARRAY[INDEX_PC_RAW];   // Capture raw cycle count for usage in the next 250us

  // Increment AME state machine counter using modulo AME_SM_TICK
  gpOCC->AME_sm_cnt = (UINT8) ((gpOCC->AME_sm_cnt + 1) & (AME_SM_TICKS - 1));

 
  // Always increment AME time on every call to ISR
  gpOCC->r_cnt++;

  // Insert ARL Test Code Here
  //printf("ARL Test Code\n");

#endif
}


