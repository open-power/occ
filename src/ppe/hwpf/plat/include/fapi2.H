/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwpf/plat/include/fapi2.H $                           */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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
///
/// @file fapi2.H
/// @brief top level header for fapi2
///

#ifndef __FAPI2_TOP_LEVEL__
#define __FAPI2_TOP_LEVEL__

// Define which platforms will not have FAPI Return Codes
#undef __noRC__
#if defined (__CME__) || defined (__SGPE__) || defined (__PGPE__)
#define __noRC__
#endif

// Determine if running on a PPE platform
#ifndef __PPE__
#if defined (__SBE__) || defined (__CME__) || defined (__SGPE__) || defined (__PGPE__)
#define __PPE__
#endif
#endif

#include <plat_trace.H>
#include <target.H>
#include <return_code.H>
#include <buffer.H>
#include <hw_access.H>
#include <utils.H>


// In turn includes the needed generated headers (hwp_ffd_classes, etc.)
#include <error_scope.H>
//#include <set_sbe_error.H>      // Generated file  mb empty file addCopyright.pl does not like empty files
#include <fapi2AttributeService.H>
#include <fapi2AttributeIds.H>      // Generated file


// #include <hwp_executor.H> // mb empty file addCopyright.pl does not like empty files

// Block of headers not currently in fapi2
#ifdef FAPI2_MISSING_HEADERS
  #include <mvpdAccess.H>
  #include <mbvpdAccess.H>
#endif


#endif // __FAPI2_TOP_LEVEL__






