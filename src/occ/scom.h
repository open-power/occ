/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/scom.h $                                              */
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

#ifndef _scom_h
#define _scom_h

#include <occ_common.h>
#include <common_types.h>
#include "errl.h"

//uses non-panic version of getscom/putscom and creates predictive error log
//with trace and processor callout on failure.  If o_err is NULL, error is
//committed internally.  Otherwise, it is assumed the caller will handle
//committing the error log.  These errors should not be discarded!
int putscom_ffdc(uint32_t i_addr, uint64_t i_data, errlHndl_t* o_err);
int getscom_ffdc(uint32_t i_addr, uint64_t* o_data, errlHndl_t* o_err);

#endif

