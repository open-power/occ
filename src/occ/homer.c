/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/homer.c $                                             */
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

// Description: HOMER specific functions.

#include "ssx.h"
#include <homer.h>

/*
 * Function Specification
 *
 * Name: homer_hd_map_read_unmap
 *
 * Description: Wrapper function for reading the host data area of the homer.
 *              The caller passes in an ID indication what they want to read,
 *              this function checks the version of the host data area and
 *              returns the data to the location pointed to by the pointer
 *              argument, it is the callers responsibility to guarantee that
 *              there is enough space to receive the returned data.
 *              Note that this function will map the HOMER host data section to
 *              the normally reserved address 0 and unmap the data before
 *              returning to the caller.
 *              If an ssx error is returned from the map or unmap request the
 *              ssx return code will be returned in the ssx rc output word.
 *
 * End Function Specification
 */

homer_rc_t homer_hd_map_read_unmap(const homer_read_var_t  i_id,
                                   uint32_t                *o_host_data,
                                   int                     *o_ssx_rc)
{
    // Locals
    Ppc405MmuMap l_mmuMapHomer = 0;
    homer_rc_t l_rc = HOMER_SUCCESS;
    occHostConfigDataArea_t *l_hdcfg_data = 0x00000000;
    *o_ssx_rc = SSX_OK;

    // Validate the parms and id
    if (!o_host_data)
    {
        l_rc = HOMER_BAD_PARM;
    }
    else if (HOMER_LAST_VAR <= i_id)
    {
        l_rc = HOMER_UNKNOWN_ID;
    }
    else
    {
        /*
         * Map to mainstore at HOMER host data offset. The first parameter is
         * the effective address where the data can be accessed once mapped, the
         * second parameter is the real address in main memory (PBA adjusted)
         * where the data is located.
         */
        *o_ssx_rc = ppc405_mmu_map((SsxAddress)l_hdcfg_data,
                                   HOMER_HD_OFFSET,
                                   sizeof(occHostConfigDataArea_t),
                                   0,
                                   0,
                                   &l_mmuMapHomer);

        if (SSX_OK != *o_ssx_rc)
        {
            l_rc = HOMER_SSX_MAP_ERR;
        }
        else
        {
            // Check version, if ok return data requested. We need to support
            // current version as well as older ones
            if (HOMER_HD_VERSION_SUPPORT < l_hdcfg_data->version)
            {
                l_rc = HOMER_UNSUPPORTED_HD_VERSION;
            }
            else
            {
                switch (i_id)
                {
                    case HOMER_VERSION:
                        *o_host_data = l_hdcfg_data->version;
                        break;
                    case HOMER_NEST_FREQ:
                        *o_host_data = l_hdcfg_data->nestFrequency;
                        break;
                    case HOMER_INT_TYPE:
                        *o_host_data = l_hdcfg_data->occInterruptType;
                        break;
                    default:
                        // Nothing to do, range checked above
                        break;
                }
            }

            *o_ssx_rc = ppc405_mmu_unmap(&l_mmuMapHomer);
            if ((SSX_OK != *o_ssx_rc) && (HOMER_SUCCESS == l_rc))
            {
                l_rc = HOMER_SSX_UNMAP_ERR;
            }
        }
    }

    return l_rc;
}
// End of homer_hd_map_read_unmap

// End of homer.c
