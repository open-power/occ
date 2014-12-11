/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/common.c $                                            */
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

// Description: Standard Library Calls

#include "ssx.h"
#include "ssx_io.h"
#include <common.h>
#include <trac_service_codes.h>
#include <string.h>

// Function Specification
//
// Name: memcmp
//
// Description: Standard Library Calls
//              Need implemented, since we can't statically link in Open Source libs
//              These implementations aren't optimized, but allow the code to function
//              so we can test out the code port.
//
// End Function Specification
//int memcmp ( const void * ptr1, const void * ptr2, size_t num )
//{
//  return 0;
//}






