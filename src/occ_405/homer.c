/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/homer.c $                                             */
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

// Description: HOMER specific functions.

#include "ssx.h"
#include <errl.h>
#include <occ_service_codes.h>
#include <occ_common.h>
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

// NOTE: Optimization of O1 is needed for this function due to the l_bootLoaderHeader pointer
// pointing to a 0 address (which is considered NULL by the compiler) and thus with newer
// gcc compilers (4.9.0 and above), a new optimization flag issolate-erroneous-paths-dereference
// the compiler will set a trap in the code that will stop it from running.
// Setting the Optimization to 1 will disable this flag when compiling with gcc 4.9 and above.

homer_rc_t __attribute__((optimize("O1"))) homer_hd_map_read_unmap(const homer_read_var_t  i_id,
                                   void                    * const o_host_data,
                                   int                     * const o_ssx_rc)
{
    Ppc405MmuMap l_mmuMapHomer = 0;
    homer_rc_t l_rc = HOMER_SUCCESS;
    occHostConfigDataArea_t *l_hdcfg_data = 0x00000000;

    // Validate the pointers
    if (!o_host_data || !o_ssx_rc || ((uint32_t)o_host_data % 4))
    {
        l_rc = HOMER_BAD_PARM;
    }
    else
    {
        *o_ssx_rc = SSX_OK;

#if PPC405_MMU_SUPPORT

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
#endif
        if (SSX_OK != *o_ssx_rc)
        {
            l_rc = HOMER_SSX_MAP_ERR;
        }
        else
        {
            // Check version, if ok handle ID requested. We need to support
            // current version as well as older ones
            if ((HOMER_VERSION_MIN > l_hdcfg_data->version)
                ||
                (HOMER_VERSION_MAX < l_hdcfg_data->version))
            {
                l_rc = HOMER_UNSUPPORTED_HD_VERSION;
            }
            else
            {
                // Version guaranteed to be within supported range

                // HOMER Version 1 support
                if (HOMER_VERSION_1 == l_hdcfg_data->version)
                {
                    switch (i_id)
                    {
                        case HOMER_VERSION:
                            *(uint32_t *)o_host_data = l_hdcfg_data->version;
                            break;
                        case HOMER_NEST_FREQ:
                            *(uint32_t *)o_host_data = l_hdcfg_data->nestFrequency;
                            break;
                        default:
                            l_rc = HOMER_UNKNOWN_ID;
                            break;
                    }
                }
                else if (HOMER_VERSION_2 == l_hdcfg_data->version)
                {
                    switch (i_id)
                    {
                        case HOMER_VERSION:
                            *(uint32_t *)o_host_data = l_hdcfg_data->version;
                            break;
                        case HOMER_NEST_FREQ:
                            *(uint32_t *)o_host_data = l_hdcfg_data->nestFrequency;
                            break;
                        case HOMER_INT_TYPE:
                            *(uint32_t *)o_host_data = l_hdcfg_data->occInterruptType;
                            break;
                        default:
                            l_rc = HOMER_UNKNOWN_ID;
                            break;
                    }
                }
                else if (HOMER_VERSION_3 == l_hdcfg_data->version)
                {
                    switch (i_id)
                    {
                        case HOMER_VERSION:
                            *(uint32_t *)o_host_data = l_hdcfg_data->version;
                            break;
                        case HOMER_NEST_FREQ:
                            *(uint32_t *)o_host_data = l_hdcfg_data->nestFrequency;
                            break;
                        case HOMER_INT_TYPE:
                            *(uint32_t *)o_host_data = l_hdcfg_data->occInterruptType;
                            break;
                        case HOMER_FIR_MASTER:
                            *(uint32_t *)o_host_data = l_hdcfg_data->firMaster;
                            break;
                        case HOMER_FIR_PARMS:
                            memcpy(o_host_data, &(l_hdcfg_data->firParms[0]), HOMER_FIR_PARM_SIZE);
                            break;
                        default:
                            l_rc = HOMER_UNKNOWN_ID;
                            break;
                    }
                }
            }
#if PPC405_MMU_SUPPORT
            // Unmap the HOMER before returning to caller
            *o_ssx_rc = ppc405_mmu_unmap(&l_mmuMapHomer);
            if ((SSX_OK != *o_ssx_rc) && (HOMER_SUCCESS == l_rc))
            {
                l_rc = HOMER_SSX_UNMAP_ERR;
            }
#endif
        }
    }

    return l_rc;
}
// End of homer_hd_map_read_unmap


/*
 * Function Specification
 *
 * Name: homer_log_access_error
 *
 * Description: Utility function to log an error that occurred while accessing
 *              the HOMER.
 *
 * End Function Specification
 */
void homer_log_access_error(const homer_rc_t i_homer_rc,
                            const int i_ssx_rc,
                            const uint32_t i_usr_data2)
{
    // Catch and log the homer error
    if (HOMER_SUCCESS != i_homer_rc)
    {
        // We could potentially have both an internal error dealing with the
        // homer and an SSX error, for example we could find an unsupported
        // version number in the homer and then have an ssx error trying to
        // unmap the homer address space.  This check catches all those cases.
        if (SSX_OK != i_ssx_rc)
        {
            /* @
             * @errortype
             * @moduleid    MAIN_MID
             * @reasoncode  SSX_GENERIC_FAILURE
             * @userdata1   HOMER and SSX return codes
             * @userdata2   Host interrupt type used
             * @userdata4   ERC_HOMER_MAIN_SSX_ERROR
             * @devdesc     An SSX error occurred mapping the HOMER host data
             *              into the OCC address space. User word 1 contains
             *              both the internal and SSX return codes returned
             *              by the method used to access the HOMER data.
             */
            errlHndl_t l_err = createErrl(MAIN_MID,                 //modId
                                          SSX_GENERIC_FAILURE,      //reasoncode
                                          ERC_HOMER_MAIN_SSX_ERROR, //Extended reason code
                                          ERRL_SEV_PREDICTIVE,      //Severity
                                          NULL,                     //Trace Buf
                                          DEFAULT_TRACE_SIZE,       //Trace Size
                                          (i_homer_rc << 16) | (0xFFFF & (uint32_t)i_ssx_rc), //userdata1
                                          i_usr_data2);             //userdata2
            commitErrl(&l_err);
        }
        else
        {
            /* @
             * @errortype
             * @moduleid    MAIN_MID
             * @reasoncode  INTERNAL_FAILURE
             * @userdata1   HOMER return code
             * @userdata2   Default host interrupt type used.
             * @userdata4   ERC_HOMER_MAIN_ACCESS_ERROR
             * @devdesc     Error accessing initialization data
             */
            errlHndl_t l_err = createErrl(MAIN_MID,                 //modId
                                          INTERNAL_FAILURE,         //reasoncode
                                          ERC_HOMER_MAIN_ACCESS_ERROR,//Extended reason code
                                          ERRL_SEV_INFORMATIONAL,   //Severity
                                          NULL,                     //Trace Buf
                                          DEFAULT_TRACE_SIZE,       //Trace Size
                                          i_homer_rc,               //userdata1
                                          i_usr_data2);             //userdata2
            commitErrl(&l_err);
        }
    }
}
// End of homer_log_access_error

// End of homer.c
