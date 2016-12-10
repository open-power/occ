/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/incl/comp_ids.h $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2017                        */
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

//Note: Be sure to mirror changes in this file to occ/plugins/tmgtTpmdCompIds.H!!!
// If you don't, the ERRL plugin will eventually break, and you might break the
// fips build for TMGT.

#ifndef _COMP_IDS_H
#define _COMP_IDS_H

#define COMP_NAME_SIZE       4

#define MAIN_COMP_ID         0x0100
#define MAIN_COMP_NAME       "MAIN"


#define ERRL_COMP_ID         0x0200
#define ERRL_COMP_NAME       "ERRL"

#define TRAC_COMP_ID         0x0300
#define TRAC_COMP_NAME       "TRAC"

#define RTLS_COMP_ID         0x0400
#define RTLS_COMP_NAME       "RTLS"

#define THRD_COMP_ID         0x0500
#define THRD_COMP_NAME       "THRD"

#define SNSR_COMP_ID         0x0600
#define SNSR_COMP_NAME       "SNSR"

// Applet Manager
#define APLT_COMP_ID         0x0700
#define APLT_COMP_NAME       "APLT"

#define PSS_COMP_ID          0x0800
#define PSS_COMP_NAME        "PSS"

#define TMER_COMP_ID         0x0900
#define TMER_COMP_NAME       "TMER"

#define DCOM_COMP_ID         0x0A00
#define DCOM_COMP_NAME       "DCOM"

// Proc data
#define PROC_COMP_ID         0x0B00
#define PROC_COMP_NAME       "PROC"

// Amec data
#define AMEC_COMP_ID         0x0C00
#define AMEC_COMP_NAME       "AMEC"

// Centaur data
#define CENT_COMP_ID         0x0D00
#define CENT_COMP_NAME       "CENT"

// Command Handler
#define CMDH_COMP_ID         0x0E00
#define CMDH_COMP_NAME       "CMDH"

// DIMM State Manager
#define DIMM_COMP_ID         0x0F00
#define DIMM_COMP_NAME       "DIMM"

// MEMORY Control
#define MEM_COMP_ID          0x1000
#define MEM_COMP_NAME        "MEM"

// Workload Optimize Frequency
#define WOF_COMP_ID          0x1100
#define WOF_COMP_NAME        "WOF"

// PGPE Interface
#define PGPE_COMP_ID         0x1200
#define PGPE_COMP_NAME       "PGPE"

#endif

