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
#define HOMER_BASE_ADDRESS    0x80000000
#define HOMER_SPACE_SIZE      0x00400000       // 4 MB

#define HOMER_HD_OFFSET       0x000C0000
#define HOMER_HD_ADDRESS     (HOMER_BASE_ADDRESS+HOMER_HD_OFFSET)
#define HOMER_HD_SZ           (128 * 1024)
#define HOMER_FIR_PARM_SIZE   (3 * 1024)

// OPAL table address in HOMER
#define OPAL_OFFSET_HOMER   0x000E2000 // Offset address of OPAL relative to HOMER
#define OPAL_DYNAMIC_OFFSET 0x0B80     // Offset address of dynamic space (relative to OPAL start address)

#define OPAL_ADDRESS_HOMER         (HOMER_BASE_ADDRESS+OPAL_OFFSET_HOMER)   // OPAL start address
#define OPAL_STATIC_ADDRESS_HOMER   OPAL_ADDRESS_HOMER                      // OPAL's Static Address
#define OPAL_DYNAMIC_ADDRESS_HOMER (OPAL_ADDRESS_HOMER+OPAL_DYNAMIC_OFFSET) // OPAL's Dynamic Address

// OCC/HTMGT command buffer offset in HOMER
#define OCC_HTMGT_CMD_OFFSET_HOMER  0x000E0000
#define OCC_HTMGT_CMD_ADDRESS_HOMER (HOMER_BASE_ADDRESS+OCC_HTMGT_CMD_OFFSET_HOMER)

// OCC/HTMGT response buffer offset in HOMER
#define OCC_HTMGT_RSP_OFFSET_HOMER  0x000E1000
#define OCC_HTMGT_RSP_ADDRESS_HOMER (HOMER_BASE_ADDRESS+OCC_HTMGT_RSP_OFFSET_HOMER)


// Version(s) of HOMER host data currently supported
typedef enum homer_version
{
    HOMER_VERSION_P9    = 0x90,
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
