/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/cmdh/cmdh_tunable_parms.c $                           */
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

#include "cmdh_tunable_parms.h"
#include "cmdh_service_codes.h"
#include "cmdh_fsp_cmds.h"
#include "dcom.h"

// Function Specification
//
// Name:  cmdh_tunable_parms_query
//
// Description: This function returns all supported tuanble parameters from the
// Tunable Parameters List that this system type supports.
//
// End Function Specification
uint8_t cmdh_tunable_parms_query(   const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                     l_rc = ERRL_RC_SUCCESS;
    uint16_t                    l_numParms = 0;
    tunable_parms_query_cmd_t  *l_cmd_ptr = (tunable_parms_query_cmd_t*) i_cmd_ptr;
    tunable_parms_query_rsp_t  *l_rsp_ptr = (tunable_parms_query_rsp_t*) o_rsp_ptr;

    do
    {
        // Check version
        if ( l_cmd_ptr->version !=  TUNABLE_PARMS_QUERY_VERSION )
        {
            TRAC_ERR("cmdh_tunable_parms_query: Tunable Parms invalid version: %x", l_cmd_ptr->version );
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }


        // Start setting up response ::

        // Version
        l_rsp_ptr->version = TUNABLE_PARMS_QUERY_VERSION;

        // Number of parameters
        l_numParms = sizeof(G_mst_tunable_parameter_table) / sizeof (cmdh_tunable_param_table_t);
        l_rsp_ptr->numParms = l_numParms;

        TRAC_INFO("cmdh_tunable_parms_query: Found %d entries", l_numParms );

        // Copy complete global array data into response
        memcpy( l_rsp_ptr->data, G_mst_tunable_parameter_table, sizeof(G_mst_tunable_parameter_table) );

    }while(0);

    // Setup the response data packet info
    uint16_t l_size = 2 + sizeof(G_mst_tunable_parameter_table);
    o_rsp_ptr->rc = l_rc;
    o_rsp_ptr->data_length[0] = ((uint8_t *)&l_size)[0];
    o_rsp_ptr->data_length[1] = ((uint8_t *)&l_size)[1];

    return l_rc;
}

// Function Specification
//
// Name:  cmdh_tunable_parms_write
//
// Description: This function is used to set the values for tunable parameters
//
// End Function Specification
uint8_t cmdh_tunable_parms_write(   const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                     l_rc = ERRL_RC_SUCCESS;
    tunable_parms_write_cmd_t  *l_cmd_ptr = (tunable_parms_write_cmd_t*) i_cmd_ptr;

    do
    {
        // Check version
        if ( l_cmd_ptr->version !=  TUNABLE_PARMS_WRITE_VERSION )
        {
            TRAC_ERR("cmdh_tunable_parms_write: Tunable Parms invalid version: %x", l_cmd_ptr->version );
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }

        // Loop through each parameter entry sent
        uint8_t i = 0;
        for (i=0; i < l_cmd_ptr->numParms; i++)
        {
            // Check if id is valid
            uint8_t l_id = l_cmd_ptr->data[i].id;
            if ( (l_id>0) && (l_id<=CMDH_DEFAULT_TUNABLE_PARAM_NUM) )
            {
                // Save off value
                G_mst_tunable_parameter_table[l_id-1].value = CONVERT_UINT8_ARRAY_UINT16(l_cmd_ptr->data[i].value[0], l_cmd_ptr->data[i].value[1]);
                G_mst_tunable_parameter_table_ext[l_id-1].adj_value = G_mst_tunable_parameter_table[l_id-1].value*G_mst_tunable_parameter_table_ext[l_id-1].multiplier;
            }
            else
            {
                TRAC_INFO("cmdh_tunable_parms_write: Tunable Parms invalid data found id=%x, value=%x",
                        l_id, CONVERT_UINT8_ARRAY_UINT16(l_cmd_ptr->data[i].value[0], l_cmd_ptr->data[i].value[1]) );

                l_rc = ERRL_RC_INVALID_DATA;
            }
        }

    }while(0);

    // Populate the response data packet
    o_rsp_ptr->rc = l_rc;

    // Set global var
    G_mst_tunable_parameter_overwrite = 1;

    return l_rc;
}

// Function Specification
//
// Name:  cmdh_tunable_parms_restore
//
// Description: This is used to tell OCC to use default value for all supported
// tunable parameters defined in the Tunable Parameters list.
//
// End Function Specification
uint8_t cmdh_tunable_parms_restore(const cmdh_fsp_cmd_t * i_cmd_ptr,
                           cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                         l_rc = ERRL_RC_SUCCESS;

    do
    {
        // Loop through global array
        uint8_t i = 0;
        for (i=0; i<CMDH_DEFAULT_TUNABLE_PARAM_NUM; i++)
        {
            // Update value using default value
            G_mst_tunable_parameter_table[i].value = G_mst_tunable_parameter_table_ext[i].def_value;
            G_mst_tunable_parameter_table_ext[i].adj_value = G_mst_tunable_parameter_table_ext[i].def_value*G_mst_tunable_parameter_table_ext[i].multiplier;
        }

    }while(0);

    // Populate the response data header
    o_rsp_ptr->rc = l_rc;

    // Set global var
    G_mst_tunable_parameter_overwrite = 2;

    return l_rc;
}



// Function Specification
//
// Name:  cmdh_tunable_parms
//
// Description:  This function parses the tunable parms commands sent via TMGT.
//
// End Function Specification
errlHndl_t cmdh_tunable_parms (   const cmdh_fsp_cmd_t * i_cmd_ptr,
                            cmdh_fsp_rsp_t * o_rsp_ptr)
{
    uint8_t                     l_rc = 0;
    uint8_t                     l_sub_cmd = 0;
    errlHndl_t                  l_errl = NULL;

    do
    {
        // Command is only supported on Master OCC
        if (G_occ_role == OCC_SLAVE)
        {
            TRAC_ERR("cmdh_tunable_parms: Tunable Parameters command not supported on Slave OCCs!");
            l_rc = ERRL_RC_INVALID_CMD;
            break;
        }

        // Sub-command is always first byte of data
        l_sub_cmd = i_cmd_ptr->data[0];

        TRAC_INFO("cmdh_tunable_parms: Tunable Parms sub-command [%d]", l_sub_cmd );

        switch (l_sub_cmd)
        {
            case TUNABLE_PARMS_QUERY:
                l_rc = cmdh_tunable_parms_query(i_cmd_ptr, o_rsp_ptr);
                break;

            case TUNABLE_PARMS_WRITE:
                l_rc = cmdh_tunable_parms_write(i_cmd_ptr, o_rsp_ptr);
                break;

            case TUNABLE_PARMS_RESTORE:
                l_rc = cmdh_tunable_parms_restore(i_cmd_ptr, o_rsp_ptr);
                break;

            default:
                // Should never get here...
                l_rc = ERRL_RC_INVALID_DATA;
                break;
        }
    } while (0);

    // All errors in TUNABLE PARMS logged internally
    if (l_rc)
    {
        TRAC_ERR("Tunable Parms command 0x%02x failed with rc = %d", l_sub_cmd, l_rc);

        // Build Error Response packet
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, l_rc, &l_errl);
    }

    return l_errl;
}

