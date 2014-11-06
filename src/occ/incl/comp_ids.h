/******************************************************************************
// @file comp_ids.h
// @brief File containing OCC component IDs
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section comp_ids.h COMP_IDS.H
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *                      TEAM      06/16/2010  Port               
 *   @01                dwoodham  08/10/2011  Add ID & Name for rtls and make all
 *                                            comp ID's increments of 0x0100
 *                      pbavari   08/16/2011  Added SNSR(sensor) component id
 *   @cc000             cjcain    08/30/2011  Added APSS component id
 *   @02                tapiar    08/16/2011  Applet Manager component id
 *   @pb007             pbavari   09/26/2011  Added TMER component Id
 *   @03                abagepa   09/16/2010  Communication component id
 *   @04                nguyenp   10/03/2011  Added Proc data component id
 *   @th00a             thallet   02/03/2012  Added AMEC component id
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *   @th00c             thallet   04/23/2012  Added CENT component id
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
 //Note: Be sure to mirror changes in this file to occ/plugins/tmgtTpmdCompIds.H!!!
 // If you don't, the ERRL plugin will eventually break, and you might break the
 // fips build for TMGT. - Mike

#ifndef _COMP_IDS_H
#define _COMP_IDS_H

//*************************************************************************
// Includes
//*************************************************************************

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

#define COMP_NAME_SIZE       4

// >@01c
#define MAIN_COMP_ID         0x0100
#define MAIN_COMP_NAME       "MAIN"


#define ERRL_COMP_ID         0x0200
#define ERRL_COMP_NAME       "ERRL"

#define TRAC_COMP_ID         0x0300
#define TRAC_COMP_NAME       "TRAC"
// <@01c

// >@01a
#define RTLS_COMP_ID         0x0400
#define RTLS_COMP_NAME       "RTLS"
// <@01a

#define THRD_COMP_ID         0x0500
#define THRD_COMP_NAME       "THRD"

#define SNSR_COMP_ID         0x0600
#define SNSR_COMP_NAME       "SNSR"

// Applet Manager @02a
#define APLT_COMP_ID         0x0700 
#define APLT_COMP_NAME       "APLT"

// @cc000
#define PSS_COMP_ID          0x0800 		
#define PSS_COMP_NAME        "PSS"

//@pb007a
#define TMER_COMP_ID         0x0900
#define TMER_COMP_NAME       "TMER"

// @03a
#define DCOM_COMP_ID         0x0A00 		
#define DCOM_COMP_NAME       "DCOM"

// Proc data @04a
#define PROC_COMP_ID         0x0B00
#define PROC_COMP_NAME       "PROC"

// Amec data @th00a
#define AMEC_COMP_ID         0x0C00
#define AMEC_COMP_NAME       "AMEC"

// Centaur data @th00c
#define CENT_COMP_ID         0x0D00
#define CENT_COMP_NAME       "CENT"

// Command Handler
#define CMDH_COMP_ID         0x0E00
#define CMDH_COMP_NAME       "CMDH"
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

#endif

