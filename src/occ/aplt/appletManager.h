/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/aplt/appletManager.h $                                */
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

#ifndef _appletManager_H
#define _appletManager_H

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include "errl.h"
#include <appletId.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************
// These are the status codes that the applet manager code may return.
typedef enum
{
    OCC_APLT_SUCCESS             = 0x00,
    OCC_APLT_PRE_START_FAILURE   = 0x01,
    OCC_APLT_POST_START_FAILURE  = 0x02,
    OCC_APLT_EXECUTE_FAILURE     = 0x03,

} OCC_APLT_STATUS_CODES;

// Applet types
typedef enum
{
    APLT_TYPE_PRODUCT = 0x00,
    APLT_TYPE_TEST    = 0x01,
    APLT_TYPE_INVALID = 0xFF
}OCC_APLT_TYPE;

//*************************************************************************
// Structures
//*************************************************************************
// Applet address/size structure
typedef struct
{
    uint32_t iv_aplt_address;
    uint32_t iv_size;

} ApltAddress_t;

//*************************************************************************
// Globals
//*************************************************************************
// Currently NOT externalized, only used in appletManager.c

// Used as a way to pass in parameters to each applet
// Applet Manager will copy arguments passed in into this
// global and pass them to applet
//extern void *         G_ApltParms;

extern ApltAddress_t  G_ApltAddressTable[ OCC_APLT_LAST ];

//*************************************************************************
// Function Prototypes
//*************************************************************************
/* run an applet */
void runApplet(
    const OCC_APLT  i_applet,
    void *          i_parms,
    const bool      i_block,
    SsxSemaphore   *io_appletComplete,
    errlHndl_t     *o_errHndl,
    OCC_APLT_STATUS_CODES * o_status);

/* initialize the applet manager */
void initAppletManager( void ) INIT_SECTION;

//*************************************************************************
// Functions
//*************************************************************************

#endif //_appletManager_H
