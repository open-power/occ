/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/homer.h $                                             */
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

// Description: homer header file

#ifndef _homer_h
#define _homer_h


// Offset into the HOMER of the host data section and the size
#define HOMER_HD_OFFSET       0x00100000
#define HOMER_HD_SZ           (128 * 1024)
#define HOMER_FIR_PARM_SIZE   (3 * 1024)

// Version(s) of HOMER host data currently supported
typedef enum homer_version
{
    HOMER_VERSION_MIN   = 1,
    HOMER_VERSION_1     = 1,
    HOMER_VERSION_2     = 2,
    HOMER_VERSION_3     = 3,
    HOMER_VERSION_MAX   = 3,
} homer_version_t;

// ID of host data variables
typedef enum homer_read_var
{
    HOMER_VERSION,
    HOMER_NEST_FREQ,
    HOMER_INT_TYPE,
    HOMER_FIR_MASTER,
    HOMER_FIR_PARMS,
    HOMER_LAST_VAR
} homer_read_var_t;

// HOMER methods return codes
typedef enum homer_rc
{
    HOMER_SUCCESS,
    HOMER_UNSUPPORTED_HD_VERSION,
    HOMER_BAD_PARM,
    HOMER_UNKNOWN_ID,
    HOMER_SSX_MAP_ERR,
    HOMER_SSX_UNMAP_ERR,
    HOMER_LAST_RC
} homer_rc_t;

// Current version of the layout for the Host Config Data section of the HOMER
struct occHostConfigDataArea
{
    uint32_t version;
    uint32_t nestFrequency;
    uint32_t occInterruptType;
    uint32_t firMaster;
    uint8_t  firParms[HOMER_FIR_PARM_SIZE];
    uint8_t  __reserved[HOMER_HD_SZ - (4 * sizeof(uint32_t)) - HOMER_FIR_PARM_SIZE];
}__attribute__ ((__packed__));
typedef struct occHostConfigDataArea occHostConfigDataArea_t;

homer_rc_t homer_hd_map_read_unmap(const homer_read_var_t, void * const, int * const);
void homer_log_access_error(const homer_rc_t, const int, const uint32_t);

#endif // _homer_h
