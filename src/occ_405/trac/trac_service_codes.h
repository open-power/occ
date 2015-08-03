/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/trac/trac_service_codes.h $                       */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

#ifndef _TRAC_SERVICE_CODES_H_
#define _TRAC_SERVICE_CODES_H_

//*************************************************************************
// Includes
//*************************************************************************
#include <comp_ids.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
enum tracReasonCodes
{
    TRAC_ERROR_BLOCK_ALLOCATE                       =  TRAC_COMP_ID | 0x00,
    TRAC_ERROR_COMP_NOT_FOUND                       =  TRAC_COMP_ID | 0x01,
    TRAC_MEM_ALLOC_FAIL                             =  TRAC_COMP_ID | 0x02,
    TRAC_MEM_BUFF_TOO_SMALL                         =  TRAC_COMP_ID | 0x03,
    TRAC_INVALID_PARM                               =  TRAC_COMP_ID | 0x04,
    TRAC_DATA_SIZE_TOO_LARGE                        =  TRAC_COMP_ID | 0x05,
    TRAC_DATA_SIZE_LESS_THAN_HEADER_SIZE            =  TRAC_COMP_ID | 0x06,
    TRAC_CIRCULAR_BUFF_FULL                         =  TRAC_COMP_ID | 0x07,
};

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

#endif /* #ifndef _TRAC_SERVICE_CODES_H_ */
