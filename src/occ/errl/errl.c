/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/errl/errl.c $                                         */
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

#include "ssx.h"
#include <errl.h>
#include <occ_common.h>
#include <comp_ids.h>
#include <trac.h>
#include <state.h>
#include <dcom.h>

uint32_t    G_occErrSlotBits = 0x000000000;
uint8_t     G_occErrIdCounter= 0x00;

uint8_t     G_errslot1[MAX_ERRL_ENTRY_SZ] = {0};
uint8_t     G_errslot2[MAX_ERRL_ENTRY_SZ] = {0};
uint8_t     G_errslot3[MAX_ERRL_ENTRY_SZ] = {0};
uint8_t     G_errslot4[MAX_ERRL_ENTRY_SZ] = {0};
uint8_t     G_errslot5[MAX_ERRL_ENTRY_SZ] = {0};
uint8_t     G_errslot6[MAX_ERRL_ENTRY_SZ] = {0};
uint8_t     G_errslot7[MAX_ERRL_ENTRY_SZ] = {0};

uint8_t     G_infoslot[MAX_ERRL_ENTRY_SZ] = {0};

uint8_t     G_callslot[MAX_ERRL_CALL_HOME_SZ] = {0};

errlHndl_t  G_occErrSlots[ERRL_MAX_SLOTS] = {
                (errlHndl_t) G_errslot1,
                (errlHndl_t) G_errslot2,
                (errlHndl_t) G_errslot3,
                (errlHndl_t) G_errslot4,
                (errlHndl_t) G_errslot5,
                (errlHndl_t) G_errslot6,
                (errlHndl_t) G_errslot7,
                (errlHndl_t) G_infoslot,
                (errlHndl_t) G_callslot
                };

void hexDumpLog( errlHndl_t i_log );

// Function Specification
//
// Name:  getErrSlotNumAndErrId
//
// Description: Get Error Slot Number and Error Id
//
// End Function Specification
uint8_t getErrSlotNumAndErrId(
            ERRL_SEVERITY i_severity,
            uint8_t *o_errlId,
            uint64_t *o_timeStamp
            )
{
    uint8_t     l_rc = ERRL_INVALID_SLOT;
    uint32_t    l_mask = ERRL_SLOT_MASK_DEFAULT;

    switch ( i_severity )
    {
        case ERRL_SEV_INFORMATIONAL:
            l_mask = ERRL_SLOT_MASK_INFORMATIONAL;
            break;
        case ERRL_SEV_PREDICTIVE:
            l_mask = ERRL_SLOT_MASK_PREDICTIVE;
            break;
        case ERRL_SEV_UNRECOVERABLE:
            l_mask = ERRL_SLOT_MASK_UNRECOVERABLE;
            break;
        case ERRL_SEV_CALLHOME_DATA:
            l_mask = ERRL_SLOT_MASK_CALL_HOME_DATA;
            break;
    };


    // we have a valid mask
    if ( l_mask != ERRL_SLOT_MASK_DEFAULT )
    {
        // 1.  Find an available slot
        uint8_t             l_slot = ERRL_INVALID_SLOT;
        uint32_t            l_slotBitWord = ~(G_occErrSlotBits | l_mask);
        SsxMachineContext   l_ctx;

        // 2. use assembly cntlzw to get slot & (disable/enable interrupts)
        ssx_critical_section_enter(SSX_NONCRITICAL, &l_ctx);
        __asm__ __volatile__ ( "cntlzw %0, %1;" : "=r" (l_slot) : "r" (l_slotBitWord));
        ssx_critical_section_exit(&l_ctx);

        // slot is valid
        if ( l_slot < ERRL_MAX_SLOTS )
        {
            ssx_critical_section_enter(SSX_NONCRITICAL, &l_ctx);
            // 3.  Get time stamp & save off timestamp
            //     Internal caller so assuming valid pointer
            *o_timeStamp = ssx_timebase_get();
            // save of counter and then increment it
            // Note: Internal caller so assuming valid pointer
            *o_errlId = ((++G_occErrIdCounter) == 0) ? ++G_occErrIdCounter : G_occErrIdCounter;

            G_occErrSlotBits |= (ERRL_SLOT_SHIFT >> l_slot);
            ssx_critical_section_exit(&l_ctx);

            l_rc =  l_slot;

        }
    }

    // return slot
    return l_rc;
}

// Function Specification
//
// Name:  getErrSlotNumByErrId
//
// Description: Get Error Slot Num By Error Id
//
// End Function Specification
uint8_t getErrSlotNumByErrId(const uint8_t  i_errlId)
{
    uint8_t l_SlotNum = ERRL_INVALID_SLOT;
    uint8_t i = 0;

    // the errID should starting from 1 to 0xff
    if (i_errlId != 0)
    {
        // search for all slot and try to get slot num
        for (i = 0; i < ERRL_MAX_SLOTS; i ++)
        {
            if (i_errlId == G_occErrSlots[i]->iv_entryId)
            {
                // Found, return the array index as slot num
                l_SlotNum = i;
                break;
            }
        }
    }

    // return ERRL_INVALID_SLOT(0xff) if not found
    return l_SlotNum;
}


// Function Specification
//
// Name:  getErrSlotOCIAddr
//
// Description: Get Error Slot OCI address
//
// End Function Specification
uint32_t getErrSlotOCIAddr(const uint8_t  i_SlotNum)
{
    void *l_addr = 0;

    if (i_SlotNum < ERRL_MAX_SLOTS)
    {
        l_addr = G_occErrSlots[i_SlotNum];
    }

    return (uint32_t)l_addr;
}


// Function Specification
//
// Name:  getErrlOCIAddrByID
//
// Description: Get Error Slot OCI address
//
// End Function Specification
uint32_t getErrlOCIAddrByID(const uint8_t i_id)
{
    // This function verifies the id is valid
    uint8_t l_slot = getErrSlotNumByErrId(i_id);

    // This function verifies that slot is valid
    return getErrSlotOCIAddr(l_slot);
}


// Function Specification
//
// Name:  getErrlLengthByID
//
// Description: Get Error Log Length
//
// End Function Specification
uint16_t getErrlLengthByID(const uint8_t i_id)
{
    uint16_t l_length = 0;
    uint8_t  l_slot   = getErrSlotNumByErrId(i_id);

    /// check if error log is committed before returning length
    /// because after it is committed, length shouldn't change.
    if(l_slot < ERRL_MAX_SLOTS)
    {
        if (G_occErrSlots[l_slot]->iv_userDetails.iv_committed)
        {
            l_length = G_occErrSlots[l_slot]->iv_userDetails.iv_entrySize;
        }
    }

    return l_length;
}


// Function Specification
//
// Name:  getLastGetErrID
//
// Description: Get the oldest Error ID
//
// End Function Specification
uint8_t getOldestErrlID()
{
    uint8_t l_entryId = 0;
    uint8_t i = 0;
    uint64_t l_timestamp = 0;

    for (i = 0; i < ERRL_MAX_SLOTS; i++)
    {
        if (G_occErrSlots[i]->iv_userDetails.iv_committed)
        {
            // Get the oldest entry by searching the Mininum Err entry ID
            if ((l_timestamp == 0) ||
                (G_occErrSlots[i]->iv_userDetails.iv_timeStamp <= l_timestamp) )
            {
                l_timestamp = G_occErrSlots[i]->iv_userDetails.iv_timeStamp;
                l_entryId = G_occErrSlots[i]->iv_entryId;
            }
        }
    }

    return l_entryId;
}


// Function Specification
//
// Name:  createErrl
//
// Description: Create an Error Log
//
// End Function Specification
errlHndl_t createErrl(
            const uint16_t i_modId,
            const uint8_t i_reasonCode,
            const uint32_t i_extReasonCode,
            const ERRL_SEVERITY i_sev,
            const tracDesc_t i_trace,
            const uint16_t i_traceSz,
            const uint32_t i_userData1,
            const uint32_t i_userData2
            )
{
    errlHndl_t  l_rc = INVALID_ERR_HNDL;
    uint64_t    l_time = 0;
    uint8_t     l_id = 0;
    uint8_t     l_errSlot = getErrSlotNumAndErrId( i_sev, &l_id, &l_time);


    if ( l_errSlot != ERRL_INVALID_SLOT )
    {
        TRAC_INFO("Creating error log in slot [%d]", l_errSlot);

        // get slot pointer
        l_rc = G_occErrSlots[ l_errSlot ];

        // save off default size
        l_rc->iv_userDetails.iv_entrySize = sizeof( ErrlEntry_t );

        // add trace
        addTraceToErrl( i_trace, i_traceSz,  l_rc );

        // save off entry Id
        l_rc->iv_entryId = l_id;

        //Save off version info
        l_rc->iv_version = ERRL_STRUCT_VERSION_1;

        // save off time
        l_rc->iv_userDetails.iv_timeStamp = l_time;

        // if its a call home error then set the sev to informational
        l_rc->iv_severity = (i_sev == ERRL_SEV_CALLHOME_DATA ? (uint8_t)ERRL_SEV_INFORMATIONAL : i_sev);

        l_rc->iv_userData4 = i_extReasonCode;

        // save off user detail section version
        l_rc->iv_userDetails.iv_version = ERRL_USR_DTL_STRUCT_VERSION_1;

        // save off rest of input parameters
        l_rc->iv_userDetails.iv_modId = i_modId;
        l_rc->iv_reasonCode = i_reasonCode;
        l_rc->iv_userDetails.iv_userData1 = i_userData1;
        l_rc->iv_userDetails.iv_userData2 = i_userData2;

        // set callout count to 0
        l_rc->iv_numCallouts = 0;

        // save off occ fields
        //NOTE: Design does not exist for these fields
        //TODO: fix this when design is done!
        l_rc->iv_userDetails.iv_fwLevel = 0;
        l_rc->iv_userDetails.iv_occId = G_pob_id.chip_id;
        l_rc->iv_userDetails.iv_occRole = G_occ_role;
        l_rc->iv_userDetails.iv_operatingState = CURRENT_STATE();
    }
    else
    {
        // TODO: put a threshold on this trace
        TRAC_INFO("Error Logs are FULL  - Slot [%d]", l_errSlot);
    }

    return l_rc;
}


// Function Specification
//
// Name:  addTraceToErrl
//
// Description: Add trace to an error log
//
// End Function Specification
void addTraceToErrl(
            const tracDesc_t i_trace,
            const uint16_t i_traceSz,
            errlHndl_t io_err)
{
    UINT l_expectLen = 0, l_rtLen = 0, l_bytes_left;
    void * l_traceAddr = io_err;
    uint16_t l_actualSizeOfUsrDtls = 0;
    pore_status_t l_gpe0_status;
    static bool L_gpe_halt_traced = FALSE;


    // check if GPE was frozen due to a checkstop
    l_gpe0_status.value = in64(PORE_GPE0_STATUS);
    if(l_gpe0_status.fields.freeze_action && !L_gpe_halt_traced)
    {
        L_gpe_halt_traced = TRUE;
        TRAC_ERR("addTraceToErrl: OCC GPE halted due to checkstop. GPE0 status[0x%08x%08x]",
                  l_gpe0_status.words.high_order, l_gpe0_status.words.low_order);
    }

    // 1. Check if error log is not null
    // 2. error log is not invalid
    // 3. error log has not been commited
    // 4. input trace is not zero
    // 5. free space is enough
    // 6. input trace descriptor is valid
    if( (io_err != NULL) &&
        (io_err != INVALID_ERR_HNDL) &&
        (io_err->iv_userDetails.iv_committed == 0) &&
        (i_traceSz != 0) &&
        ((io_err->iv_userDetails.iv_entrySize + sizeof(ErrlUserDetailsEntry_t)) < MAX_ERRL_ENTRY_SZ ) &&
        ((i_trace==g_trac_inf)||(i_trace==g_trac_err)||(i_trace==g_trac_imp)||(i_trace==NULL)) )
    {
        //local copy of the usr details entry
        ErrlUserDetailsEntry_t l_usrDtlsEntry;
        uint16_t l_headerSz = sizeof( l_usrDtlsEntry );

        //adjust user details entry size to available size (word align )
        uint16_t l_availableSize = MAX_ERRL_ENTRY_SZ - (io_err->iv_userDetails.iv_entrySize + l_headerSz );
        l_usrDtlsEntry.iv_size = ( i_traceSz < l_availableSize ) ? i_traceSz : l_availableSize; // @jh001c

        //set type
        l_usrDtlsEntry.iv_type = (uint8_t) ERRL_USR_DTL_TRACE_DATA;

        //set version
        l_usrDtlsEntry.iv_version = ERRL_TRACE_VERSION_1;

        //copy the data into error the offset is the size of the current errorlog
        void * l_p = io_err;

        // Caculate trace data address. Starting from errl address + sizeof(ErrlEntry_t + ErrlUserDetailsEntry_t).
        l_traceAddr = l_p + io_err->iv_userDetails.iv_entrySize + l_headerSz;

        // check if user request to add trace from a specific trace buffer
        if (i_trace != NULL)
        {
            // Ensure requested length are larger than one trace info size.
            if (l_usrDtlsEntry.iv_size >= sizeof (trace_buf_head_t))
            {
                l_rtLen = l_usrDtlsEntry.iv_size;
                TRAC_get_buffer_partial(i_trace, l_traceAddr, &l_rtLen);

                // Update data size
                l_usrDtlsEntry.iv_size = l_rtLen;
            }
            else
            {
                TRAC_IMP("addTraceToErrl: Not enough buffer size for trace, Avail[%d], Req[%d]\n", l_availableSize, l_usrDtlsEntry.iv_size);

                //Requested size is not able to fill in any trace info. Clear data length and give up.
                l_usrDtlsEntry.iv_size = 0;
            }
        }
        else
        {
            // User not specify which trace buffer to add.
            // We have three kinds of trace buffer.(INF/IMP/ERR).Get partial of them to fill in user detail section of this ERR Log.
            l_bytes_left = l_usrDtlsEntry.iv_size;
            l_expectLen = l_bytes_left / NUM_OF_TRACE_TYPE;

            // Ensure the size are able to fill in at least one trace info.
            if (l_expectLen >= sizeof (trace_buf_head_t))
            {
                l_rtLen = l_expectLen;
                TRAC_get_buffer_partial(TRAC_get_td("ERR"), l_traceAddr, &l_rtLen);
                l_actualSizeOfUsrDtls += l_rtLen;
                l_bytes_left -= l_rtLen;

                l_rtLen = l_bytes_left / 2;
                TRAC_get_buffer_partial(TRAC_get_td("IMP"), (l_traceAddr + l_actualSizeOfUsrDtls), &l_rtLen);
                l_actualSizeOfUsrDtls += l_rtLen;
                l_bytes_left -= l_rtLen;

                l_rtLen = l_bytes_left;
                TRAC_get_buffer_partial(TRAC_get_td("INF"), (l_traceAddr + l_actualSizeOfUsrDtls), &l_rtLen);
                l_actualSizeOfUsrDtls += l_rtLen;

                // Update data size
                l_usrDtlsEntry.iv_size = l_actualSizeOfUsrDtls;
            }
            else
            {
                // Check to see if we are still able to fill in 1 or 2 traces into this Err log.
                if (l_usrDtlsEntry.iv_size > sizeof (trace_buf_head_t))
                {
                    l_rtLen = l_usrDtlsEntry.iv_size;

                    // Added only ERR trace info
                    TRAC_get_buffer_partial(TRAC_get_td("ERR"), l_traceAddr, &l_rtLen);

                    // Update data size
                    l_usrDtlsEntry.iv_size = l_rtLen;
                }
                else
                {
                    TRAC_IMP("addTraceToErrl: Not enough buffer size for trace, Avail[%d], Req[%d]\n", l_availableSize, l_usrDtlsEntry.iv_size);

                    //We do not have enough size to fill in any trace info. Clear data length.
                    l_usrDtlsEntry.iv_size = 0;
                }
            }
        }

        if (l_usrDtlsEntry.iv_size)
        {
            // Finally, cacluate entire data length including usrDtl header.
            l_actualSizeOfUsrDtls = l_usrDtlsEntry.iv_size + l_headerSz;

            // save of user detail header for trace buf section we just added.
            // Address is starting from "errl address + sizeof(ErrlEntry_t)."
            //
            //  io_err  |----------------------------------------|
            //          | ErrlEntry_t                            |
            //          | {iv_userDetails.iv_userDetailEntrySize,| <== we may have more usrdtl sections, need to add length
            //          |       ... other elements ...           }|     of new usrdtl section(ErrlUserDetailsEntry_t + trace lens)
            //          |----------------------------------------|
            //          | ErrlUserDetailsEntry_t                 | <== copy usrdtl header to here (l_usrDtlsEntry)
            //          |----------------------------------------|
            //trace buf | trace1(ex.INF)                         | <== trace already filled in at this moment.
            //          | trace2(ex.INF&IMP&ERR)                 |
            //          |         ...                            |
            //
            l_p = memcpy( l_p+((io_err->iv_userDetails.iv_entrySize)),&l_usrDtlsEntry, l_headerSz );
        }

        //update usr data entry size
        io_err->iv_userDetails.iv_userDetailEntrySize += l_actualSizeOfUsrDtls;

        //update error log size
        io_err->iv_userDetails.iv_entrySize += l_actualSizeOfUsrDtls;

    }//end validation check
}


// Function Specification
//
// Name:  reportErrorLog
//
// Description: report the log to tmgt
//
// End Function Specification
void reportErrorLog( errlHndl_t i_err, uint16_t i_entrySize )
{
    // report the log
    // will need to give them the address and size to read

    // TODO: Guts still not defined yet
    TRAC_INFO("Reporting error @ %p with size %d",i_err, i_entrySize );
    TRAC_INFO("ModID: 0x%08X, RC: 0x%08X, UserData1: 0x%08X, UserData2: 0x%08X",
           i_err->iv_userDetails.iv_modId, i_err->iv_reasonCode,
           i_err->iv_userDetails.iv_userData1, i_err->iv_userDetails.iv_userData2);

    // TODO: remove this when tracing is enabled
    hexDumpLog( i_err );
}


// Function Specification
//
// Name:  commitErrl
//
// Description: Commit an Error Log
//
// End Function Specification
void commitErrl( errlHndl_t *io_err )
{
    pore_status_t l_gpe0_status;

    if ( io_err != NULL )
    {
        // check if handle is valid and is NOT empty
        if ((*io_err != NULL ) && ( *io_err != INVALID_ERR_HNDL ))
        {
            // check if GPE was frozen due to a checkstop
            l_gpe0_status.value = in64(PORE_GPE0_STATUS);
            if(l_gpe0_status.fields.freeze_action)
            {
                //Go to the reset state to minimize errors
                reset_state_request(RESET_REQUESTED_DUE_TO_ERROR);

                //clear out all other actions and set the safe mode req'd action
                (*io_err)->iv_actions.word = ERRL_ACTIONS_SAFE_MODE_REQUIRED;

                //set severity to informational
                (*io_err)->iv_severity = ERRL_SEV_INFORMATIONAL;

                //set callouts to 0
                (*io_err)->iv_numCallouts = 0;
            }

            // if reset action bit is set force severity to unrecoverable and
            // make sure there is at least one callout
            if((*io_err)->iv_actions.reset_required)
            {
                (*io_err)->iv_severity = ERRL_SEV_UNRECOVERABLE;
                if(!(*io_err)->iv_numCallouts)
                {
                    addCalloutToErrl(*io_err,
                                     ERRL_CALLOUT_TYPE_COMPONENT_ID,
                                     ERRL_COMPONENT_ID_FIRMWARE,
                                     ERRL_CALLOUT_PRIORITY_HIGH);
                }
            }

            // mark the last callout by zeroing out the next one
            if((*io_err)->iv_numCallouts < ERRL_MAX_CALLOUTS)
            {
                memset(&(*io_err)->iv_callouts[(*io_err)->iv_numCallouts], 0,
                        sizeof(ErrlCallout_t));
            }

            // number of callouts must be the max value as defined by the TMGT-OCC spec.
            (*io_err)->iv_numCallouts = ERRL_MAX_CALLOUTS;

            // save off committed
            (*io_err)->iv_userDetails.iv_committed = 1;

            // calculate checksum & save it off
            uint32_t    l_cnt = 2;  // starting point is after checksum field
            uint32_t    l_sum = 0;
            uint32_t    l_size = (*io_err)->iv_userDetails.iv_entrySize;
            uint8_t *   l_p = (uint8_t *)*io_err;

            for( ; l_cnt < l_size ; l_cnt++ )
            {
                l_sum += *(l_p+l_cnt);
            }

            (*io_err)->iv_checkSum = l_sum;

            // Flush error log out to SRAM since the FSP will directly read it
            dcache_flush( *io_err, MAX_ERRL_ENTRY_SZ );

            // report error to FSP
            reportErrorLog( *io_err, l_size );
        }

        *io_err = (errlHndl_t) NULL;
    }
}


// Function Specification
//
// Name:  getErrlLogId
//
// Description: Get Log Id from an Error Log
//
// End Function Specification
uint8_t getErrlLogId( errlHndl_t io_err )
{
    uint8_t l_logId = ERRL_INVALID_SLOT;

    // check if handle is valid and is NOT empty
    if ((io_err != NULL ) && ( io_err != INVALID_ERR_HNDL ))
    {
        l_logId = (io_err)->iv_entryId;
    }

    return l_logId;
}


// Function Specification
//
// Name:  deleteErrl
//
// Description: Delete an Error Log
//
// End Function Specification
errlHndl_t deleteErrl( errlHndl_t *io_err)
{
    errlHndl_t l_err = INVALID_ERR_HNDL;

    if (io_err != NULL)
    {
        // check if handle is valid and is NOT empty
        if ((*io_err != NULL ) &&
            (*io_err != INVALID_ERR_HNDL ))
        {

            // find the slot number by traversing the global array
            uint32_t l_slot = 0;

            for( ; l_slot < ERRL_MAX_SLOTS; l_slot++ )
            {
                if ( *io_err == G_occErrSlots[ l_slot ] )
                {
                    TRAC_INFO("deleting error @%p at slot [%d]", *io_err, l_slot );

                    // clear out space in that slot
                    memset(*io_err, 0, (*io_err)->iv_userDetails.iv_entrySize );

                    // Disable interrupts
                    SsxMachineContext   l_ctx;
                    ssx_critical_section_enter(SSX_NONCRITICAL, &l_ctx);

                    //clear the error log slot bit for reuse
                    G_occErrSlotBits &= ~(ERRL_SLOT_SHIFT >> l_slot);

                    // Enable interrupts
                    ssx_critical_section_exit(&l_ctx);

                    l_err = NULL;

                    // done doing the work
                    break;
                }
            }
        } // end if valid error log handl

        //set the handle to null
        *io_err = (errlHndl_t) NULL;
    }

    return l_err;
}


// Function Specification
//
// Name:  addCalloutToErrl
//
// Description: Add a callout to an Error Log
//
// End Function Specification
void addCalloutToErrl(
            errlHndl_t io_err,
            const ERRL_CALLOUT_TYPE i_type,
            const uint64_t i_calloutValue,
            const ERRL_CALLOUT_PRIORITY i_priority)
{
    // 1. check if handle is valid (not null or invalid)
    // 2. not committed
    // 3. severity is not informational (unless mfg action flag is set)
    // 4. callouts still not full
    if ( (io_err != NULL ) &&
         (io_err != INVALID_ERR_HNDL) &&
        (io_err->iv_userDetails.iv_committed == 0) &&
        (io_err->iv_actions.mfg_error || io_err->iv_severity != ERRL_SEV_INFORMATIONAL) &&
        (io_err->iv_numCallouts < ERRL_MAX_CALLOUTS) )
    {
        //set callout type
        io_err->iv_callouts[ io_err->iv_numCallouts ].iv_type = (uint8_t)i_type;

        //set callout value
        io_err->iv_callouts[ io_err->iv_numCallouts ].iv_calloutValue = i_calloutValue;

        //set priority
        io_err->iv_callouts[ io_err->iv_numCallouts].iv_priority = (uint8_t)i_priority;

        //increment actual number of callout
        io_err->iv_numCallouts++;
    }
}


// Function Specification
//
// Name:  addUsrDtlsToErrl
//
// Description: Add User Details to an Error Log
//
// End Function Specification
void addUsrDtlsToErrl(
            errlHndl_t io_err,
            uint8_t *i_dataPtr,
            const uint16_t i_size,
            const uint8_t i_version,
            const ERRL_USR_DETAIL_TYPE i_type)
{
    // Locals
    uint16_t l_maxSize = (i_type == ERRL_USR_DTL_CALLHOME_DATA) ? MAX_ERRL_CALL_HOME_SZ : MAX_ERRL_ENTRY_SZ;

    // 1.  check if handle is valid
    // 2.  NOT empty
    // 3.  not committed
    // 4.  size being passed in is valid
    // 5.  data pointer is valid
    // 6.  and we have enough size
    if ((io_err != NULL ) &&
        (io_err != INVALID_ERR_HNDL ) &&
        (io_err->iv_userDetails.iv_committed == 0) &&
        (i_size != 0) &&
        (i_dataPtr != NULL) &&
        ((io_err->iv_userDetails.iv_entrySize) < l_maxSize))
    {
        //local copy of the usr details entry
        ErrlUserDetailsEntry_t l_usrDtlsEntry;
        uint16_t l_headerSz = sizeof( l_usrDtlsEntry );

        //adjust user details entry size to available size (word align )
        uint16_t l_availableSize = l_maxSize - (io_err->iv_userDetails.iv_entrySize + l_headerSz );
        l_usrDtlsEntry.iv_size = ( i_size < l_availableSize ) ? i_size : l_availableSize;

        //set type
        l_usrDtlsEntry.iv_type = (uint8_t)i_type;

        //set version
        l_usrDtlsEntry.iv_version = i_version;

        //set the data
        uint16_t l_actualSizeOfUsrDtls = l_headerSz + l_usrDtlsEntry.iv_size;

        //copy the data into error the offset is the size of the current errorlog
        void * l_p = io_err;
        l_p = memcpy( l_p+((io_err->iv_userDetails.iv_entrySize)),&l_usrDtlsEntry, l_headerSz );
        memcpy( l_p+l_headerSz, i_dataPtr, l_usrDtlsEntry.iv_size);

        //update usr data entry size
        io_err->iv_userDetails.iv_userDetailEntrySize += l_actualSizeOfUsrDtls;

        //update error log size
        io_err->iv_userDetails.iv_entrySize += l_actualSizeOfUsrDtls;
    }
}


// Function Specification
//
// Name:  setErrlSevToInfo
//
// Description: Set Error Log Severity to Informational
//              NOTE: Any callouts in the current error log will be DROPPED!
//
// End Function Specification
void setErrlSevToInfo( errlHndl_t io_err )
{
    // check if handle is valid
    // NOT empty
    // not committed
    if ( (io_err != NULL )
         && ( io_err != INVALID_ERR_HNDL )
         && (io_err->iv_userDetails.iv_committed == 0) )
    {
        //set sev to informational
        io_err->iv_severity = ERRL_SEV_INFORMATIONAL;

        //clear any callouts
        uint32_t l_sizeOfcallouts = sizeof(ErrlCallout_t)*(io_err->iv_numCallouts);
        memset(io_err->iv_callouts, 0,l_sizeOfcallouts );

        //clear number of callouts
        io_err->iv_numCallouts = 0;
    }
}


// Function Specification
//
// Name:  setErrlActions
//
// Description: Set Actions to an Error Log
//
// End Function Specification
// @jh001a
void setErrlActions(errlHndl_t io_err, const uint8_t i_mask)
{
    // check if handle is valid
    // NOT empty
    // not committed
    if ( (io_err != NULL )
         && ( io_err != INVALID_ERR_HNDL )
         && (io_err->iv_userDetails.iv_committed == 0) )
    {
        // set the appropriate action bits$
        io_err->iv_actions.word |= i_mask;
    }
}

// TODO: Cleanup this function.
// Function Specification
//
// Name:  hexDumpLog
//
// Description: Hex Dump Log
//
// End Function Specification
void hexDumpLog( errlHndl_t i_log )
{
#if 0
    uint32_t    l_written = 0;
    uint32_t    l_counter = 0;
    uint8_t *   l_data = (uint8_t*) i_log;
    uint32_t    l_len = i_log->iv_userDetails.iv_entrySize;

    while ( l_counter < l_len)
    {
        if (( i_log == NULL ) ||
            ( i_log == INVALID_ERR_HNDL ))
        {
            // break out if log is invalid
            // do nothing
            break;
        }

        printf("|   %08X     ", (uint32_t) l_data + l_counter);

        // Display 16 bytes in Hex with 2 spaces in between
        l_written = 0;
        uint8_t i = 0;
        for ( i = 0; i < 16 && l_counter < l_len; i++ )
        {
            l_written += printf("%02X",l_data[l_counter++]);

            if ( ! ( l_counter % 4 ) )
            {
                l_written += printf("  ");
            }
        }

        // Pad with spaces
        uint8_t l_space[64] = {0};
        memset( l_space, 0x00, sizeof( l_space ));
        memset( l_space, ' ', 43-l_written);
        printf("%s", l_space );

        // Display ASCII
        l_written = 0;
        uint8_t l_char;
        for ( ; i > 0 ; i-- )
        {
            l_char = l_data[ l_counter-i ];

            if ( isprint( l_char ) &&
                 ( l_char != '&' ) &&
                 ( l_char != '<' ) &&
                 ( l_char != '>' )
               )
            {
                l_written += printf("%c",l_char );
            }
            else
            {
                l_written += printf("." );
            }
        }

        // Pad with spaces
        uint8_t l_space2[64] = {0};
        memset( l_space2, 0x00, sizeof( l_space2 ));
        memset( l_space2, ' ', 19-l_written);
        printf("%s|\n", l_space2 );
    }
#endif
}
