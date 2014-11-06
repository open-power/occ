/******************************************************************************
// @file gpe_export.h
// @brief This file contains structures, etc, that are needed
//        by applications external to the GPE
*/
/******************************************************************************
 *
 *       @page ChangeLogs Change Logs
 *       @section gpe_export.h GPE_EXPORT.H
 *       @verbatim
 *
 *   Flag    Def/Fea    Userid    Date        Description
 *   ------- ---------- --------  ----------  ----------------------------------
 *   @cc000             cjcain    08/04/2011  Created      
 *   @rc003             rickylie  02/03/2012  Verify & Clean Up OCC Headers & Comments
 *
 *  @endverbatim
 *
 *///*************************************************************************/
 
#ifndef _GPE_EXPORT_H
#define _GPE_EXPORT_H

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

//*************************************************************************
// Structures
//*************************************************************************
typedef struct {
     union
     {
       struct {
         uint32_t rc;
         uint32_t addr;
       };
       uint64_t error;
     };
     uint64_t ffdc;
} PoreGpeErrorStruct;  // Same for every GPE program

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

#endif //_GPE_EXPORT_H
