/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/sensor/sensor_inband_cmd.c $                      */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2017,2017                        */
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


//******************************************************************************
// Includes
//******************************************************************************
#include <cmdh_fsp_cmds.h>
#include <sensor_inband_cmd.h>
#include <homer.h>
#include <occ_service_codes.h>
#include <sensor_service_codes.h>
#include <trac.h>
#include <occhw_async.h>
#include <common.h>


//******************************************************************************
// Block Copy Engine (BCE) Defines/Globals
//******************************************************************************

// Buffer in SRAM to copy larger commands cmd/rsp buffer from/to main memory using the BCE
DMA_BUFFER(inband_max_cmd_t G_inband_cmd_max_data_bce_buff) = {{0}};

// Buffer in SRAM to copy smaller commands cmd/rsp buffer from/to main memory using the BCE
DMA_BUFFER(inband_min_cmd_t G_inband_cmd_min_data_bce_buff) = {{0}};

// BCE request structure.  Used by BCE functions to schedule copy request.
BceRequest G_inband_cmd_bce_req;

/**
 * Specifies whether the BCE request was scheduled.  If false, the request
 * finished or has never been scheduled/initialized.
 */
bool G_inband_cmd_req_scheduled = false;

// Number of tics passed waiting for BCE callback
uint8_t G_bce_callback_wait = 0;

volatile uint8_t G_inband_occ_cmd_state = INBAND_OCC_CMD_NONE;
volatile uint8_t G_inband_occ_bce_saved_state = INBAND_OCC_CMD_NONE;

//******************************************************************************
// Functions
//******************************************************************************


/**
 * Logs an error caused by the Block Copy Engine.  Does nothing if a BCE error
 * has already been logged.
 *
 * Note that the required error log comment containing tags like 'userdata4' and
 * 'devdesc' must be located by the call to this function.  It is not located
 * inside this function because the value of those tags varies.
 *
 * @param i_modId         Module ID
 * @param i_extReasonCode Extended reason code
 * @param i_userData1     Userdata1 value
 * @param i_userData2     Userdata2 value
 */
void inband_cmd_log_bce_error(uint16_t i_modId, uint16_t i_extReasonCode,
                              uint32_t i_userData1, uint32_t i_userData2)
{
    static bool L_error_logged = false;
    if (!L_error_logged)
    {
        // Create and commit error
        errlHndl_t l_errl = createErrl(i_modId,                // Module ID
                                       INBAND_CMD_ERROR,       // Reason code
                                       i_extReasonCode,        // Extended reason code
                                       ERRL_SEV_INFORMATIONAL, // Severity
                                       NULL,                   // Trace Buffers
                                       DEFAULT_TRACE_SIZE,     // Trace Size
                                       i_userData1,            // Userdata1
                                       i_userData2);           // Userdata2
        commitErrl(&l_errl);
        L_error_logged = true;
    }
}


/**
 * Returns whether the global BCE request struct is idle and ready for re-use.
 * Returns true immediately if the request was not scheduled.  If the request
 * was scheduled, checks to see if it has finished.
 *
 * @param i_caller_mod_id Module ID of calling function in case an error occurs
 * @return True if BCE request is idle, false otherwise
 */
bool inband_cmd_is_bce_req_idle(uint16_t i_caller_mod_id)
{
    // Number of times we've waited for current request to finish
    static uint8_t L_wait_count = 0;

    // If the request was not previously scheduled, then it is idle.  This also
    // handles the case where the request has not been initialized yet.
    if (!G_inband_cmd_req_scheduled)
    {
        return true;
    }

    // Request was scheduled; check if it finished and is now idle
    if (async_request_is_idle(&G_inband_cmd_bce_req.request))
    {
        // Request is now idle and ready for re-use
        G_inband_cmd_req_scheduled = false;

        // If we were waiting for request to finish, trace and clear wait count
        if (L_wait_count > 0)
        {
            TRAC_INFO("inband_cmd_is_bce_req_idle: "
                      "Request finished after waiting %u times: caller=0x%04X",
                      L_wait_count, i_caller_mod_id);
            L_wait_count = 0;
        }
        return true;
    }

    // Request was scheduled but has not finished.  Increment wait count unless
    // we are already at the max (to avoid overflow).
    if (L_wait_count < UINT8_MAX)
    {
        ++L_wait_count;
    }

    // If this is the first time we've waited for this request, trace it
    if (L_wait_count == 1)
    {
        TRAC_INFO("inband_cmd_is_bce_req_idle: "
                  "Waiting for request to finish: caller=0x%04X",
                  i_caller_mod_id);
    }

    // If this is the second time we've waited for this request, log BCE error
    if (L_wait_count == 2)
    {
        /* @
         * @errortype
         * @moduleid    INBAND_CMD_IS_BCE_REQ_IDLE_MOD
         * @reasoncode  INBAND_CMD_ERROR
         * @userdata1   Caller module ID
         * @userdata2   0
         * @userdata4   ERC_GENERIC_TIMEOUT
         * @devdesc     BCE request not finished after waiting twice
         */
        inband_cmd_log_bce_error(INBAND_CMD_IS_BCE_REQ_IDLE_MOD, ERC_GENERIC_TIMEOUT,
                                 i_caller_mod_id, 0);
    }

    // Return false since request is not idle
    return false;
}

/**
 * inband_cmd_bce_callback
 *
 * Description: Callback function for G_inband_cmd_bce_req BCE request
 *              NO TRACING OR CALLING FUNCTIONS THAT TRACE ALLOWED
 */
void inband_cmd_bce_callback( void )
{
    static bool L_processed_at_least_one_cmd = FALSE;
    static uint8_t L_last_seq_num_processed = 0;
    uint8_t seq_num = 0;
    uint8_t cmd_flags = 0;

    // Decide what to do next for processing an in-band command for the BCE that finished
    switch (G_inband_occ_cmd_state)
    {
        case INBAND_OCC_CMD_CHECK_FOR_CMD:
            // check for command uses min bce data buffer
            // If we processed at least one cmd it is not a new command if same seq number
            cmd_flags = G_inband_cmd_min_data_bce_buff.header.flags;
            seq_num = G_inband_cmd_min_data_bce_buff.header.seq;
            if( ( (!L_processed_at_least_one_cmd) ||
                  (L_processed_at_least_one_cmd && (seq_num != L_last_seq_num_processed)) ) &&
                ( (cmd_flags & IN_BAND_CMD_READY_MASK) == IN_BAND_CMD_READY_MASK) )
            {
               // There is a command to process
               G_inband_occ_cmd_state = INBAND_OCC_CMD_START;
               // now that we have a cmd save seq num so we don't keep processing the same cmd
               L_processed_at_least_one_cmd = TRUE;
               L_last_seq_num_processed = seq_num;
            }
            else
            {
               // No command
               G_inband_occ_cmd_state = INBAND_OCC_CMD_NONE;
            }
            break;

        case INBAND_OCC_CMD_RSP_READY:
            // response is ready send interrupt
            G_inband_occ_cmd_state = INBAND_OCC_CMD_RSP_INT;
            break;

        default:
            // Invalid state.  Can't trace here set state to invalid to trace later
            G_inband_occ_bce_saved_state = G_inband_occ_cmd_state;
            G_inband_occ_cmd_state = INBAND_OCC_INVALID_BCE_CALLBACK;
            break;
    }
}

/**
 * Copies the specified number of bytes either to main mem or down from main mem
 * for handling an inband command using the Block Copy Engine (BCE).
 *
 * @param i_main_mem_addr Main memory address for copy.  Must be 128-byte aligned
 * @param i_sram_addr     SRAM address for copy.  Must be 128-byte aligned
 * @param i_byte_count    Number of bytes to copy.  Must be multiple of 128.
 *                        Must be <= INBAND_OCC_CMD_BCE_BUF_SIZE.  0 bytes is not valid
 * @param i_to_main_mem   TRUE indicates copy is sram to main memory
 *                        FALSE indicates copy is main memory to sram
 * @param i_caller_mod_id Module ID of the calling function in case an error occurs
 * @return True if BCE request was successfully scheduled, false otherwise
 */
bool inband_cmd_bce_copy(uint32_t i_main_mem_addr, uint32_t i_sram_addr,
                         size_t i_byte_count, bool i_to_main_mem, uint16_t i_caller_mod_id)
{
    int l_rc = 0;

    // Verify address and byte count are valid
    static bool L_traced_param_error = false;
    if (((i_main_mem_addr % 128) != 0) || ((i_sram_addr % 128) != 0) ||
         ((i_byte_count % 128) != 0) || (i_byte_count > INBAND_CMD_MAX_BCE_BUF_SIZE) ||
         (i_byte_count == 0) )
    {
        if (!L_traced_param_error)
        {
            TRAC_ERR("inband_cmd_bce_copy: Input parameter error: "
                     "address=0x%08X SRAM=0x%08X length=%u caller=0x%04X",
                     i_main_mem_addr, i_sram_addr, i_byte_count, i_caller_mod_id);
            L_traced_param_error = true;
        }
        return false;
    }

    // Check if a copy request was previously scheduled and is not yet finished
    static bool L_traced_sched_error = false;
    if (!inband_cmd_is_bce_req_idle(i_caller_mod_id))
    {
        if (!L_traced_sched_error)
        {
            TRAC_ERR("inband_cmd_bce_copy: Previous request not finished: caller=0x%04X",
                     i_caller_mod_id);
            L_traced_sched_error = true;
        }
        return false;
    }

    // Create BCE request based on if copy is up or down from main memory
    if (i_to_main_mem)
    {
       l_rc = bce_request_create(&G_inband_cmd_bce_req,    // Block copy request
                                 &G_pba_bcue_queue,        // SRAM up to mainstore
                                 i_main_mem_addr,          // Mainstore address
                                 i_sram_addr,              // SRAM start address
                                 i_byte_count,             // Size of copy
                                 SSX_WAIT_FOREVER,         // No timeout
          (AsyncRequestCallback) inband_cmd_bce_callback,
                                 NULL,                     // No call back args
                                 ASYNC_CALLBACK_IMMEDIATE);
    }
    else
    {
       l_rc = bce_request_create(&G_inband_cmd_bce_req,    // Block copy request
                                 &G_pba_bcde_queue,        // mainstore down to SRAM
                                 i_main_mem_addr,          // Mainstore address
                                 i_sram_addr,              // SRAM start address
                                 i_byte_count,             // Size of copy
                                 SSX_WAIT_FOREVER,         // No timeout
          (AsyncRequestCallback) inband_cmd_bce_callback,
                                 NULL,                     // No call back args
                                 ASYNC_CALLBACK_IMMEDIATE);
    }

    if (l_rc != SSX_OK)  // fail to create BCE request?
    {
        TRAC_ERR("inband_cmd_bce_copy: Request create failure: rc=0x%08X caller=0x%04X",
                 -l_rc, i_caller_mod_id);
        /* @
         * @errortype
         * @moduleid    INBAND_CMD_BCE_COPY_MOD
         * @reasoncode  INBAND_CMD_ERROR
         * @userdata1   Return code from bce_request_create()
         * @userdata2   Caller module ID
         * @userdata4   ERC_BCE_REQUEST_CREATE_FAILURE
         * @devdesc     Failed to create BCE request
         */
        inband_cmd_log_bce_error(INBAND_CMD_BCE_COPY_MOD, ERC_BCE_REQUEST_CREATE_FAILURE,
                                 -l_rc, i_caller_mod_id);
        return false;
    }

    // Schedule BCE request
    l_rc = bce_request_schedule(&G_inband_cmd_bce_req);
    if (l_rc != SSX_OK)
    {
        TRAC_ERR("inband_cmd_bce_copy: Request schedule failure: rc=0x%08X caller=0x%04X",
                 -l_rc, i_caller_mod_id);
        /* @
         * @errortype
         * @moduleid    INBAND_CMD_BCE_COPY_MOD
         * @reasoncode  INBAND_CMD_ERROR
         * @userdata1   Return code from bce_request_schedule()
         * @userdata2   Caller module ID
         * @userdata4   ERC_BCE_REQUEST_SCHEDULE_FAILURE
         * @devdesc     Failed to schedule BCE request
         */
        inband_cmd_log_bce_error(INBAND_CMD_BCE_COPY_MOD, ERC_BCE_REQUEST_SCHEDULE_FAILURE,
                                 -l_rc, i_caller_mod_id);
        return false;
    }

    // Successfully scheduled request.  Copy is not blocking, so need to check
    // whether it finished later.  Set flag indicating request is scheduled.
    G_inband_cmd_req_scheduled = true;
    return true;
}


// Function Specification
//
// Name: inband_command_check
//
// Description: Check for command from the inband interface
//
// End Function Specification
void inband_command_check(void)
{
    // Only check for a new command if not currently processing an inband command
    if (G_inband_occ_cmd_state == INBAND_OCC_CMD_NONE)
    {
       // Create and Schedule BCE to read minimum bytes of OCC inband command buffer in HOMER
       memset((void*)&G_inband_cmd_min_data_bce_buff, 0x00, sizeof(inband_min_cmd_t));
       bool i_to_main_mem = FALSE; // this request is main mem down to SRAM
       G_inband_occ_cmd_state = INBAND_OCC_CMD_CHECK_FOR_CMD;
       G_bce_callback_wait = 0;
       if (inband_cmd_bce_copy(INBAND_OCC_CMD_ADDRESS_HOMER, (uint32_t)&G_inband_cmd_min_data_bce_buff,
                               INBAND_CMD_MIN_BCE_BUF_SIZE, i_to_main_mem, INBAND_CMD_CHECK_MOD))
       {
           // Copy succeeded. The BCE callback will handle next
       }
       else
       {
          // copy failed try again next time
          G_inband_occ_cmd_state = INBAND_OCC_CMD_NONE;
       }
    }
}

// Function Specification
//
// Name: inband_command_handler
//
// Description: Command handler for inband commands this should only be called if
//               it is already known that there is an in-band command in process
//               This is checked to be called on every tick
//
// End Function Specification
void inband_command_handler(void)
{
    uint8_t l_reason_code = ERRL_RC_INTERNAL_FAIL;
    uint16_t l_cmd_data_len = 0;
    uint8_t l_seq_num = 0;
    uint8_t l_cmd_type = 0xFF;
    uint16_t l_rsp_data_length = 0;
    uint16_t l_bce_copy_size = 0;
    uint8_t l_bce_padding = 0;
    bool l_to_main_mem = TRUE; // BCE requests from here are going SRAM to main memory
    bool l_bce_scheduled = FALSE;

    // Decide what to do next for processing an in-band command
    if(G_inband_occ_cmd_state == INBAND_OCC_CMD_START)
    {
        // We use the max buffer for response
        memset((void*)&G_inband_cmd_max_data_bce_buff, 0x00, INBAND_CMD_MAX_BCE_BUF_SIZE);

        // When we check for command we used the minimum bce buffer
        l_seq_num = G_inband_cmd_min_data_bce_buff.header.seq;
        l_cmd_type = G_inband_cmd_min_data_bce_buff.header.cmd_type;
        l_cmd_data_len = CONVERT_UINT8_ARRAY_UINT16(G_inband_cmd_min_data_bce_buff.header.data_length[0],
                                                    G_inband_cmd_min_data_bce_buff.header.data_length[1]);

        // if data is more than min we will need to do another BCE to read the rest of the cmd
        if(l_cmd_data_len > INBAND_MIN_DATA_LENGTH)
        {
           // invalid command, currently no cmd supported is larger than min requiring 2nd BCE
           l_reason_code = ERRL_RC_INVALID_CMD_LEN;
        }
        else
        {
           // Have the full command, process it based on command type.
           // cmd data is in min data BCE buffer the rsp data will go in the max data BCE buffer
           uint8_t* l_cmd_data_ptr = (uint8_t*) &G_inband_cmd_min_data_bce_buff.data;
           uint8_t* l_rsp_data_ptr = (uint8_t*) &G_inband_cmd_max_data_bce_buff.data;

           switch(l_cmd_type)
           {
              case CMDH_CLEAR_SENSOR_DATA:
                 l_reason_code = cmdh_clear_sensor_data(l_cmd_data_len,
                                                        l_cmd_data_ptr,
                                                        INBAND_MAX_DATA_LENGTH,
                                                        &l_rsp_data_length,
                                                        l_rsp_data_ptr);
                 break;

              case CMDH_SET_PCAP_INBAND:
                 l_reason_code = cmdh_set_pcap_inband(l_cmd_data_len,
                                                      l_cmd_data_ptr,
                                                      INBAND_MAX_DATA_LENGTH,
                                                      &l_rsp_data_length,
                                                      l_rsp_data_ptr);
                 break;

              case CMDH_WRITE_PSR:
                 l_reason_code = cmdh_write_psr(l_cmd_data_len,
                                                l_cmd_data_ptr,
                                                INBAND_MAX_DATA_LENGTH,
                                                &l_rsp_data_length,
                                                l_rsp_data_ptr);
                 break;

              case CMDH_SELECT_SENSOR_GROUPS:
                 l_reason_code = cmdh_select_sensor_groups(l_cmd_data_len,
                                                           l_cmd_data_ptr,
                                                           INBAND_MAX_DATA_LENGTH,
                                                           &l_rsp_data_length,
                                                           l_rsp_data_ptr);
                 break;

              case CMDH_INBAND_WOF_CONTROL:
                 l_reason_code = cmdh_inband_wof_control(l_cmd_data_len,
                                                         l_cmd_data_ptr,
                                                         INBAND_MAX_DATA_LENGTH,
                                                         &l_rsp_data_length,
                                                         l_rsp_data_ptr);
                 break;

              default:
                   l_reason_code = ERRL_RC_INVALID_CMD;
                   break;
           }  // end switch
        }  // end cmd process

        // fill in response header in G_inband_cmd_max_data_bce_buff
        G_inband_cmd_max_data_bce_buff.header.flags = IN_BAND_RSP_READY_MASK;
        G_inband_cmd_max_data_bce_buff.header.seq = l_seq_num;
        G_inband_cmd_max_data_bce_buff.header.cmd_type = l_cmd_type;
        G_inband_cmd_max_data_bce_buff.header.reserved_rc = l_reason_code;
        G_inband_cmd_max_data_bce_buff.header.data_length[0] = ((uint8_t *)&l_rsp_data_length)[0];
        G_inband_cmd_max_data_bce_buff.header.data_length[1] = ((uint8_t *)&l_rsp_data_length)[1];

        // Copy the response from SRAM to main memory
        // Determine BCE copy size, must be factor of 128
        l_bce_copy_size = sizeof(inband_occ_cmd_header_t) + l_rsp_data_length;
        l_bce_padding = l_bce_copy_size % 128;
        if(l_bce_padding)
        {
           l_bce_copy_size += (128 - l_bce_padding);
        }

        G_bce_callback_wait = 0;
        G_inband_occ_cmd_state = INBAND_OCC_CMD_RSP_READY;
        l_bce_scheduled = inband_cmd_bce_copy(INBAND_OCC_RSP_ADDRESS_HOMER,
                                             (uint32_t)&G_inband_cmd_max_data_bce_buff,
                                             l_bce_copy_size,
                                             l_to_main_mem,
                                             INBAND_CMD_HANDLER_MOD);
        if (!l_bce_scheduled)
        {
           // failed to copy response
           G_inband_occ_cmd_state = INBAND_OCC_CMD_NONE;
        }
        else
        {
           TRAC_INFO("inband_command_handler: Command 0x%02X with "
                     "seq_num 0x%02X finished with RC 0x%02X",
                      l_cmd_type, l_seq_num, l_reason_code);
        }

    }  // INBAND_OCC_CMD_START

    else if(G_inband_occ_cmd_state == INBAND_OCC_CMD_RSP_INT)
    {
       // Send interrupt to indciate response is ready in main mem
       notify_host(INTR_REASON_OPAL_SHARED_MEM_CHANGE);
       G_inband_occ_cmd_state = INBAND_OCC_CMD_NONE;
    }

    else if( (G_inband_occ_cmd_state == INBAND_OCC_CMD_CHECK_FOR_CMD) ||
             (G_inband_occ_cmd_state == INBAND_OCC_CMD_RSP_READY) )
    {
       // Waiting for BCE to finish, these states are handled by BCE callback
       // if the callback is never called (BCE failure) for a max wait log error
       G_bce_callback_wait++;
       if(G_bce_callback_wait == MAX_TICS_INBAND_BCE_CALLBACK_WAIT)
       {
           TRAC_ERR("inband_command_handler: Timeout waiting for BCE callback cmd state 0x%02X",
                     G_inband_occ_cmd_state);
           G_inband_occ_cmd_state = INBAND_OCC_CMD_NONE;
           /* @
            * @errortype
            * @moduleid    INBAND_CMD_HANDLER_MOD
            * @reasoncode  INBAND_CMD_ERROR
            * @userdata1   Inband command state
            * @userdata2   Caller module ID
            * @userdata4   ERC_BCE_REQ_CALLBACK_TIMEOUT
            * @devdesc     Timeout waiting for BCE callback
            */
           inband_cmd_log_bce_error(INBAND_CMD_HANDLER_MOD, ERC_BCE_REQ_CALLBACK_TIMEOUT,
                                     G_inband_occ_cmd_state, INBAND_CMD_HANDLER_MOD);
       }
    }

    else if(G_inband_occ_cmd_state == INBAND_OCC_INVALID_BCE_CALLBACK)
    {
       // BCE callback was called with invalid state, trace now and set state to none
       static bool L_traced_bce_bad_state = FALSE;
       if (!L_traced_bce_bad_state)
       {
           TRAC_ERR("inband_command_handler: inband_cmd_bce_callback detected invalid state %u",
                    G_inband_occ_bce_saved_state);
           L_traced_bce_bad_state = TRUE;
       }
       G_inband_occ_cmd_state = INBAND_OCC_CMD_NONE;
       G_inband_occ_bce_saved_state = INBAND_OCC_CMD_NONE;
    }

    else
    {
       // Invalid state
       static bool L_traced_bad_state = FALSE;
       if (!L_traced_bad_state)
       {
           TRAC_ERR("inband_command_handler: Called with invalid state %u",
                    G_inband_occ_cmd_state);
           L_traced_bad_state = TRUE;
       }
       G_inband_occ_cmd_state = INBAND_OCC_CMD_NONE;
    }
}
