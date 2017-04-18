/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/cmdh/cmdh_snapshot.c $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
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

#include "cmdh_snapshot.h"
#include "cmdh_service_codes.h"
#include "cmdh_fsp_cmds.h"
#include "dcom.h"
#include "threadSch.h"

// Array of snapshot buffers to return to TMGT
cmdh_snapshot_buffer_t g_cmdh_snapshot_array[CMDH_SNAPSHOT_MAX];

// Current snapshot id.
uint8_t g_cmdh_snapshot_cur_id = 0;

// Current index into array for newest entry
uint8_t g_cmdh_snapshot_cur_index = CMDH_SNAPSHOT_DEFAULT_CUR_INDEX;

// This global is set to TRUE by default to get a clean start, and is also
// set to TRUE in case of a resync request.
bool g_cmdh_snapshot_reset = TRUE;

SsxTimer G_snapshotTimer;

// Max, min power, and accumulated frequency average over 30seconds
pwr250us_over30sec_t  g_pwr250us_over30sec;

// Function Specification
//
// Name: cmdh_snapshot_find_oldest_newest
//
// Description: Returns the index of the oldest and newest snapshot in the
//              g_cmdh_snapshot_array.
//
// End Function Specification

VOID cmdh_snapshot_find_oldest_newest(uint8_t  *o_oldest,
                                      uint8_t  *o_newest)
{
    uint8_t                 l_index = 0;

    do
    {
        // We wont be called if no snapshots are available so we know at least
        // one is available

        // If our current index is 0, it's id is 0, and the last entry has
        // an id of 0 then we know we haven't wrapped yet since 2 entries
        // can not have the same current_id and 0 was what's written in
        // as an initializer
        if((g_cmdh_snapshot_cur_index == 0) &&
           (g_cmdh_snapshot_array[0].current_id == 0) &&
           (g_cmdh_snapshot_array[CMDH_SNAPSHOT_MAX_INDEX].current_id == 0))
        {
            *o_oldest = 0;
            *o_newest = 0;
            break;
        }

        // Newest will be g_cmdh_snapshot_cur_index
        *o_newest = g_cmdh_snapshot_cur_index;

        // n = newest, o=oldest, x=used, _=empty
        //________________________________________
        //|x|x|x|x|x|x|x|x|x|x|x|n|o|x|x|x|x|x|x|x|
        // Start by assuming that the oldest snapshot is right after the newest.
        l_index = g_cmdh_snapshot_cur_index+1;

        //________________________________________
        //|o|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|n|
        //Check on whether the newest was at the end of the array.  That would
        //mean that the oldest entry is at index 0.
        if(l_index > CMDH_SNAPSHOT_MAX_INDEX)
        {
            l_index = 0;
        }
        //________________________________________
        //|o|x|x|x|x|x|x|x|x|x|x|n|_|_|_|_|_|_|_|_|
        //Check on whether the last entry in the array is empty and so is the one
        //right after the newest entry. If so, that means oldest is at index 0 since
        //we haven't wrapped yet.
        else if((g_cmdh_snapshot_array[l_index].current_id == 0) &&
                (g_cmdh_snapshot_array[CMDH_SNAPSHOT_MAX_INDEX].current_id == 0))
        {
            l_index = 0;
        }
        // else oldest is just equal to l_index
        *o_oldest = l_index;

    }while(FALSE);


    return;
}


// Function Specification
//
// Name: cmdh_snapshot_buffer_nonite
//
// Description: Returns the requested snapshot buffer to tmgt.
//
// End Function Specification
ERRL_RC cmdh_snapshot_buffer_nonite(const cmdh_fsp_cmd_t *i_cmd_ptr,
                                          cmdh_fsp_rsp_t *o_rsp_ptr)
{
    cmdh_get_snapshot_query_t   *l_cmd_ptr      = (cmdh_get_snapshot_query_t *) i_cmd_ptr;
    ERRL_RC                     l_rc            = ERRL_RC_SUCCESS;
    cmdh_get_snapshot_resp_v0_t *l_rsp_ptr      = (cmdh_get_snapshot_resp_v0_t*) o_rsp_ptr;
    uint8_t                     l_newest        = 0;
    uint8_t                     l_oldest        = 0;
    uint8_t                     l_req_idx       = 0;
    uint8_t                     i               = 0;

    do
    {

        // Check case where there are no snapshot buffers available.
        if (g_cmdh_snapshot_cur_index == CMDH_SNAPSHOT_DEFAULT_CUR_INDEX)
        {
            break;
        }

        // Determine newest and oldest buffer.
        cmdh_snapshot_find_oldest_newest(&l_oldest, &l_newest);

        l_rsp_ptr->oldest_id = g_cmdh_snapshot_array[l_oldest].current_id;
        l_rsp_ptr->newest_id = g_cmdh_snapshot_array[l_newest].current_id;

        // Determine which snapshot buffer is requested by TMGT
        if (l_cmd_ptr->getnewest)
        {
            l_req_idx = l_newest;
        }
        else
        {
            // Find requested snapshot, or if not available grab latest.
            for (i =0; i< CMDH_SNAPSHOT_MAX; i++)
            {
                if (g_cmdh_snapshot_array[i].current_id == l_cmd_ptr->requested_id)
                {
                    l_req_idx = i;
                    break;
                }
            }

            if (i == CMDH_SNAPSHOT_MAX)
            {
                l_req_idx = l_newest;
            }
        }

        // Copy the requested snapshot buffer into the response buffer.
        memcpy(&(l_rsp_ptr->snapshot),
               &(g_cmdh_snapshot_array[l_req_idx]),
               sizeof(cmdh_snapshot_buffer_t));

        // Calculate returned data size.
        uint16_t l_size = 2 + sizeof(cmdh_snapshot_buffer_t); // 2 bytes for newest and oldest + size of snapshots.
        o_rsp_ptr->data_length[0] = ((uint8_t *)&l_size)[0];
        o_rsp_ptr->data_length[1] = ((uint8_t *)&l_size)[1];


    } while (0);

    return(l_rc);
}

// Function Specification
//
// Name: cmdh_get_snapshot_buffer
//
// Description: Returns requested snapshot buffer to tmgt when requested.
//
// End Function Specification
errlHndl_t cmdh_get_snapshot_buffer(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                          cmdh_fsp_rsp_t * o_rsp_ptr)
{
    cmdh_get_snapshot_query_t   *l_cmd_ptr        = (cmdh_get_snapshot_query_t *) i_cmd_ptr;
    ERRL_RC                     l_rc              = ERRL_RC_SUCCESS;
    uint16_t                    l_query_sz        = 0;
    errlHndl_t                  l_err             = NULL;

    do
    {


        // Command is only supported on Master OCC
        if (G_occ_role == OCC_SLAVE)
        {
            TRAC_ERR("cmdh_get_snapshot_buffer: Get snapshot buffer command not supported on Slave OCCs!");
            l_rc = ERRL_RC_INVALID_CMD;
            break;
        }

        // Function Inputs Sanity Check
        if( (NULL == i_cmd_ptr) || (NULL == o_rsp_ptr) )
        {
            TRAC_ERR("cmdh_get_snapshot_buffer: Received invalid inputs.");
            l_rc = ERRL_RC_INTERNAL_FAIL;
            break;
        }

        l_query_sz = CMDH_DATALEN_FIELD_UINT16(i_cmd_ptr);
        // Command Length Check.  Should have 4 bytes total
        if(l_query_sz != CMDH_GET_SNAPSHOT_QUERY_DATALEN)
        {
            TRAC_ERR("cmdh_get_snapshot_buffer: Received an invalid packet size. Expecting 4 bytes, received:%i",
                     l_query_sz);
            l_rc = ERRL_RC_INVALID_CMD_LEN;
            break;
        }

        // Call appropriate function based on version.
        switch (l_cmd_ptr->version)
        {
            case CMDH_GET_SNAPSHOT_NONITE_VERSION:
                l_rc = cmdh_snapshot_buffer_nonite(i_cmd_ptr, o_rsp_ptr);
                break;

            case CMDH_GET_SNAPSHOT_ITE_VERSION:
            default:
                TRAC_ERR("cmdh_get_snapshot_buffer: Version %i cmd is not supported.", l_cmd_ptr->version);
                l_rc = ERRL_RC_INVALID_DATA;
                break;
        }

    } while(0);

    if (l_rc)
    {
        // Build Error Response packet
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, l_rc, &l_err);
    }

    return l_err;
}


#define CMDH_SNAPSHOT_SYNC_DATA_SIZE    1
// Function Specification
//
// Name: cmdh_snapshot_sync
//
// Description: Resets the snapshot buffer array and starts a new snapshot buffer from time 0.
//
// End Function Specification
errlHndl_t cmdh_snapshot_sync(const cmdh_fsp_cmd_t * i_cmd_ptr,
                                    cmdh_fsp_rsp_t * o_rsp_ptr)
{
    cmdh_snapshot_sync_query_t      *l_cmd_ptr = (cmdh_snapshot_sync_query_t *) i_cmd_ptr;
    cmdh_snapshot_sync_resp_t       *l_resp_ptr = (cmdh_snapshot_sync_resp_t *) o_rsp_ptr;
    errlHndl_t                      l_err = NULL;
    uint8_t                         l_query_sz = 0;
    ERRL_RC                         l_rc = 0;

    do
    {
        l_query_sz = CMDH_DATALEN_FIELD_UINT16(i_cmd_ptr);
        // Verify query data size
        if(l_query_sz != CMDH_SNAPSHOT_SYNC_DATA_SIZE)
        {
            TRAC_ERR("cmdh_snapshot_sync: Received an invalid packet size. Expecting 1 byte, received:%i",
                     l_query_sz);
            l_rc = ERRL_RC_INVALID_CMD_LEN;
            break;
        }

        l_cmd_ptr = (cmdh_snapshot_sync_query_t *)i_cmd_ptr;

        // Check received packet version
        if (CMDH_SNAPSHOT_SYNC_VERSION != l_cmd_ptr->version)
        {
            TRAC_ERR("cmdh_snapshot_sync: Version %i cmd is not supported.", l_cmd_ptr->version);
            l_rc = ERRL_RC_INVALID_DATA;
            break;
        }

        // Set the global reset flag, that will cause all saved data to be cleared in the
        // next callback that is done every 30 seconds via a timer.
        g_cmdh_snapshot_reset = TRUE;

        // Reset current index to stop any possible calls from tmgt to get snapshot buffers.
        g_cmdh_snapshot_cur_index = CMDH_SNAPSHOT_DEFAULT_CUR_INDEX;

        // Reset timer and start counting from now. This will cause a call to the snapshot_callback
        // function below which will reset the other globals based on the fact that g_cmdh_snapshot_reset
        // is set to true.
        l_rc = ssx_timer_schedule(&G_snapshotTimer, 0, SSX_SECONDS(30));
        if (l_rc != SSX_OK)
        {
            TRAC_ERR("cmdh_snapshot_sync: reseting the snapshot timer failed.");
            break;
        }

        l_resp_ptr->data_length[0] = 0;
        l_resp_ptr->data_length[1] = 0;
        l_resp_ptr->rc = 0;
    }while(FALSE);

    if (l_rc)
    {
        // Build Error Response packet
        cmdh_build_errl_rsp(i_cmd_ptr, o_rsp_ptr, l_rc, &l_err);
    }
    return(l_err);
}

// Function Specification
//
// Name: cmdh_
//
// Description: Called from timer every 30seconds to store cimp data into
//              an array of 20 cimp snapshots. Then it triggers the generation
//              of a new snapshot in the g_cmdh_sna
//
// End Function Specification
void cmdh_snapshot_callback(void * arg)
{
    static cmdh_snapshot_cimp_entry_t L_cim_buf[CMDH_CIMP_MAX]; // Holds the earlier cim data snapshots.
    static uint8_t  L_cim_seq_number = 0;       // Holds the next cim seq number to be used.
    static uint32_t L_prev_pwr_accum = 0;       // Holds the previous power sensor accumulator
    static uint32_t L_prev_pwr_update_tag = 0;  // Holds the previous power sensor update tag.

    uint32_t        l_pwr_accum         =
        (uint32_t)AMECSENSOR_PTR(PWR250US)->accumulator;
    uint32_t        l_pwr_update_tag    = AMECSENSOR_PTR(PWR250US)->update_tag;
    uint32_t        l_avg_pwr           = 0;
    uint32_t        l_min_pwr           = g_pwr250us_over30sec.min;
    uint32_t        l_max_pwr           = g_pwr250us_over30sec.max;
    uint32_t        l_freq_accum        = g_pwr250us_over30sec.freqaAccum;
    uint32_t        l_freq_cnt          = g_pwr250us_over30sec.count;
    uint16_t        l_avg_freq          = 0;

    cmdh_snapshot_cimp_entry_t  l_cim_buf_temp[CMDH_CIMP_MAX - 1];
    cmdh_snapshot_buffer_t      l_30s_snapshot; // Used to temporarily hold new snapshot data.


    // Clear data that is calculated every 250us to prep for next callback in 30seconds.
    // Reset max and min over 30 seconds.
    memset(&g_pwr250us_over30sec,0,sizeof(g_pwr250us_over30sec));

    if (g_cmdh_snapshot_reset)
    {
        memset(g_cmdh_snapshot_array, 0, sizeof(g_cmdh_snapshot_array));
        g_cmdh_snapshot_cur_id  = 0;
        memset(L_cim_buf,0,sizeof(cmdh_snapshot_buffer_t));
        L_cim_seq_number        = 0;
        L_prev_pwr_accum        = l_pwr_accum;
        L_prev_pwr_update_tag   = l_pwr_update_tag;
        // Clear reset flag.  - This should be the only place that clears it.
        g_cmdh_snapshot_reset   = FALSE;
        g_cmdh_snapshot_cur_index = CMDH_SNAPSHOT_DEFAULT_CUR_INDEX;
    }
    else
    {

        uint32_t l_pwr_accum_diff =  0;
        uint32_t l_pwr_uptag_diff =  0;

        // Calculate the accumulator difference.
        if(l_pwr_accum >= L_prev_pwr_accum)
        {
            l_pwr_accum_diff =  l_pwr_accum - L_prev_pwr_accum;
        }
        else
        {
            l_pwr_accum_diff = l_pwr_accum + (~L_prev_pwr_accum);
        }

        // Calculate the update tag difference.
        if(l_pwr_update_tag >= L_prev_pwr_update_tag)
        {
            l_pwr_uptag_diff =  l_pwr_update_tag - L_prev_pwr_update_tag;
        }
        else if(l_pwr_update_tag < L_prev_pwr_update_tag)   // accum must have wrapped.
        {
            l_pwr_uptag_diff = l_pwr_update_tag + (~L_prev_pwr_update_tag);
        }

        // Make sure we don't divide by 0
        if(l_pwr_uptag_diff == 0)
        {
            l_avg_pwr = 0;
        }
        else
        {
            // Calculate average power since previous callback.
            l_avg_pwr = l_pwr_accum_diff / l_pwr_uptag_diff;
        }

        if(l_freq_cnt == 0)
        {
            l_avg_freq = 0;
        }
        else
        {
            // Calculate average frequency of all Cores for the whole Node (all OCCs)
            l_avg_freq = (uint16_t)(l_freq_accum / l_freq_cnt);
        }

        // Save power sensor accumulator and update tag values for next callback.
        L_prev_pwr_accum = l_pwr_accum;
        L_prev_pwr_update_tag = l_pwr_update_tag;

        // Append the new cim buffer entry to the front of the local static array of cim buffers.
        memcpy(l_cim_buf_temp, L_cim_buf, sizeof(cmdh_snapshot_cimp_entry_t) * (CMDH_CIMP_MAX - 1));

        L_cim_buf[0].seq_number = L_cim_seq_number;
        memcpy(L_cim_buf[0].avg_dc, &l_avg_pwr,sizeof(L_cim_buf[0].avg_dc));
        memcpy(L_cim_buf[0].min_dc, &l_min_pwr,sizeof(L_cim_buf[0].min_dc));
        memcpy(L_cim_buf[0].max_dc, &l_max_pwr,sizeof(L_cim_buf[0].max_dc));
        memcpy(L_cim_buf[0].avg_cpu_freq, &l_avg_freq,sizeof(L_cim_buf[0].avg_cpu_freq));

        memcpy(&(L_cim_buf[1]), l_cim_buf_temp, sizeof(cmdh_snapshot_cimp_entry_t) * (CMDH_CIMP_MAX - 1));

        // Populate the local 30s snapshot buffer before writing it out to the global array.  Reason is
        // we don't want the buffer data to be read by tmgt cmd while we are filling it.

        // Copy cim data to snapshot buffer.
        memcpy(l_30s_snapshot.cim, L_cim_buf, sizeof(cmdh_snapshot_cimp_entry_t) * CMDH_CIMP_MAX);
        l_30s_snapshot.current_id = g_cmdh_snapshot_cur_id;

        // Increment current index to signify that we are going to add a new snapshot.
        g_cmdh_snapshot_cur_index++;
        if (g_cmdh_snapshot_cur_index > CMDH_SNAPSHOT_MAX_INDEX)
        {
            // Wrap back to start
            g_cmdh_snapshot_cur_index = 0;
        }

        // Write buffer to the current index in the global array g_cmdh_snapshot_array.
        memcpy(&(g_cmdh_snapshot_array[g_cmdh_snapshot_cur_index]),&l_30s_snapshot, sizeof(cmdh_snapshot_buffer_t));

        // Increment snapshot id.
        g_cmdh_snapshot_cur_id++;

        // Increment sequence count.
        L_cim_seq_number ++;
    }
}
