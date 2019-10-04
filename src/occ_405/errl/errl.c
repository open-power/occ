/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/errl/errl.c $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2019                        */
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

#include "ssx.h"
#include <errl.h>
#include <occ_common.h>
#include <comp_ids.h>
#include <trac.h>
#include <state.h>
#include <dcom.h>
#include <common.h>
#include <ocb_firmware_registers.h>
#include <ocb_register_addresses.h>
#include <amec_sys.h>
#include <pgpe_shared.h>
#include <wof.h>

uint32_t    G_occErrSlotBits = 0x000000000;
uint8_t     G_occErrIdCounter= 0x00;

uint8_t     G_errslot1[MAX_ERRL_ENTRY_SZ] = {0};
uint8_t     G_errslot2[MAX_ERRL_ENTRY_SZ] = {0};
uint8_t     G_errslot3[MAX_ERRL_ENTRY_SZ] = {0};
uint8_t     G_errslot4[MAX_ERRL_ENTRY_SZ] = {0};
uint8_t     G_errslot5[MAX_ERRL_ENTRY_SZ] = {0};
uint8_t     G_errslot6[MAX_ERRL_ENTRY_SZ] = {0};

uint8_t     G_callslot[MAX_ERRL_CALL_HOME_SZ] = {0};

errlHndl_t  G_occErrSlots[ERRL_MAX_SLOTS] = {
                (errlHndl_t) G_errslot1,
                (errlHndl_t) G_errslot2,
                (errlHndl_t) G_errslot3,
                (errlHndl_t) G_errslot4,
                (errlHndl_t) G_errslot5,
                (errlHndl_t) G_errslot6,
                (errlHndl_t) G_callslot
                };

hcode_elog_entry_t *G_hcode_elog_table = NULL;
uint32_t            G_hcode_elog_table_slots = 0;

// Array of error counters that are only cleared on OCC reset
uint8_t G_error_history[ERR_HISTORY_SIZE] = {0};

extern uint8_t G_occ_interrupt_type;
extern bool G_fir_collection_required;
extern amec_sys_t g_amec_sys;



unsigned int leastSignificantErrlID();



// Function Specification
//
// Name:  getErrSlotNumAndErrId
//
// Description: Get Error Slot Number and Error Id
//
// End Function Specification
uint8_t getErrSlotNumAndErrId(ERRL_SEVERITY i_severity,
                              uint8_t      *o_errlId,
                              uint64_t     *o_timeStamp,
                              uint16_t     *o_maxSize)
{
    uint8_t     l_rc = ERRL_INVALID_SLOT;

    uint32_t    l_mask = ERRL_SLOT_MASK_OCC_ERROR;
    uint16_t    l_maxSize = MAX_ERRL_ENTRY_SZ;
    if (ERRL_SEV_CALLHOME_DATA == i_severity)
    {
        // Call Home logs have a dedicated slot
        l_mask = ERRL_SLOT_MASK_CALL_HOME_DATA;
        l_maxSize = MAX_ERRL_CALL_HOME_SZ;
    }

    // 1.  Find an available slot
    //     l_slotBitWord represents the available slots given the severity type.
    uint8_t             l_slot = ERRL_INVALID_SLOT;
    uint32_t            l_slotBitWord = ~(G_occErrSlotBits | l_mask);
    SsxMachineContext   l_ctx;

    // 2. use assembly cntlzw (count leading zeros) to get available slot based on
    // severity type, and (disable/enable interrupts)
    ssx_critical_section_enter(SSX_NONCRITICAL, &l_ctx);
    __asm__ __volatile__ ( "cntlzw %0, %1;" : "=r" (l_slot) : "r" (l_slotBitWord));
    ssx_critical_section_exit(&l_ctx);

    // A slot is available and valid
    if ( l_slot < ERRL_MAX_SLOTS )
    {
        ssx_critical_section_enter(SSX_NONCRITICAL, &l_ctx);
        // 3.  Get time stamp & save off timestamp
        //     Internal caller so assuming valid pointer
        *o_timeStamp = ssx_timebase_get();
        // save of counter and then increment it
        // Note: Internal caller so assuming valid pointer

        //Provide next ErrorId; G_occErrIdCounter should never be 0.
        *o_errlId = ((++G_occErrIdCounter) == 0) ? ++G_occErrIdCounter : G_occErrIdCounter;

        //Set slot bit in list of used up slots.
        G_occErrSlotBits |= (ERRL_SLOT_SHIFT >> l_slot);
        ssx_critical_section_exit(&l_ctx);

        // return the maximum size
        *o_maxSize = l_maxSize;

        l_rc = l_slot;
    }
    else if ((ERRL_SEV_INFORMATIONAL != i_severity) &&
             (ERRL_SEV_CALLHOME_DATA != i_severity))
    {
        // Slot was not found for critical error.  Check for an INFO error that can be replaced
        l_slot = leastSignificantErrlID();
        if (l_slot != ERRL_MAX_SLOTS)
        {
            TRAC_ERR("getErrSlotNumAndErrId: Slots full, but overriding INFO log in slot %d", l_slot);
            {
                ssx_critical_section_enter(SSX_NONCRITICAL, &l_ctx);

                // Override this elog slot (uncommit so does not get added to poll)
                G_occErrSlots[l_slot]->iv_userDetails.iv_committed = FALSE;

                // 3.  Get time stamp & save off timestamp
                //     Internal caller so assuming valid pointer
                *o_timeStamp = ssx_timebase_get();
                // save of counter and then increment it
                // Note: Internal caller so assuming valid pointer

                //Provide next ErrorId; G_occErrIdCounter should never be 0.
                *o_errlId = ((++G_occErrIdCounter) == 0) ? ++G_occErrIdCounter : G_occErrIdCounter;

                // No need to set used slot bit (since already was being used)
                ssx_critical_section_exit(&l_ctx);
            }

            // clear out space in that slot
            const unsigned int l_length = G_occErrSlots[l_slot]->iv_userDetails.iv_entrySize;
            memset(G_occErrSlots[l_slot], 0, l_length);

            *o_maxSize = l_maxSize;

            l_rc = l_slot;
        }
    }

    // return slot
    return l_rc;

} // end getErrSlotNumAndErrId()


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
// Name:  leastSignificantErrlID
//
// Description: find the least significant info elog to use for critical error
//
// End Function Specification
unsigned int leastSignificantErrlID()
{
    unsigned int i = 0;
    uint64_t l_info_timestamp = 0;
    // Poll finds/reports elogs by using getOldestErrlID(), so do NOT want to take that slot
    const unsigned int l_oldest = getOldestErrlID();
    unsigned int slot_to_use = ERRL_MAX_SLOTS;

    for (i = 0; i < ERRL_MAX_SLOTS; i++)
    {
        if (G_occErrSlots[i]->iv_userDetails.iv_committed) // slot is used
        {
            if (l_oldest != G_occErrSlots[i]->iv_entryId) // slot not reported in POLL response
            {
                if (ERRL_SEV_INFORMATIONAL == G_occErrSlots[i]->iv_severity) // slot has INFO elog
                {
                    // Use the slot from the most recently created INFO log
                    if ((l_info_timestamp == 0) ||
                        (G_occErrSlots[i]->iv_userDetails.iv_timeStamp > l_info_timestamp) )
                    {
                        // Use this slot
                        l_info_timestamp = G_occErrSlots[i]->iv_userDetails.iv_timeStamp;
                        slot_to_use = i;
                    }
                }
            }
        }
    }

    return slot_to_use;
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
            const uint16_t i_extReasonCode,
            const ERRL_SEVERITY i_sev,
            const trace_descriptor_array_t* i_trace,
            const uint16_t i_traceSz,
            const uint32_t i_userData1,
            const uint32_t i_userData2
            )
{
    errlHndl_t  l_rc = INVALID_ERR_HNDL;
    uint64_t    l_time = 0;
    uint16_t    l_maxSize = 0;
    uint8_t     l_id = 0;
    uint8_t     l_errSlot = getErrSlotNumAndErrId( i_sev, &l_id, &l_time, &l_maxSize);
    static uint8_t L_traceCount = 5;


    if ( l_errSlot != ERRL_INVALID_SLOT )
    {
        TRAC_INFO("createErrl: Creating error log in slot[%d] max size[%d]", l_errSlot, l_maxSize);

        // get slot pointer
        l_rc = G_occErrSlots[ l_errSlot ];

        // save off maximum error log size
        l_rc->iv_maxSize = l_maxSize;

        // save off default size
        l_rc->iv_userDetails.iv_entrySize = sizeof( ErrlEntry_t );

        // add error history
        addErrHistory( l_rc );

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

        l_rc->iv_extendedRC = i_extReasonCode;

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
        l_rc->iv_userDetails.iv_fclipHistory = g_amec->proc[0].chip_f_reason_history;
        l_rc->iv_userDetails.iv_occId = G_pbax_id.chip_id;
        l_rc->iv_userDetails.iv_occRole = G_occ_role;
        l_rc->iv_userDetails.iv_operatingState = CURRENT_STATE();
    }
    else
    {
        if( L_traceCount > 0 )
        {
            TRAC_INFO("createErrl: Error Logs are FULL  - Slot [%d]", l_errSlot);
            L_traceCount--;
        }
    }

    return l_rc;
}

// Function Specification
//
// Name:  createPgpeErrl
//
// Description: Create an Error Log due to a PGPE failure
//
// End Function Specification
errlHndl_t createPgpeErrl(const uint16_t i_modId,
                          const uint8_t i_reasonCode,
                          const uint16_t i_extReasonCode,
                          const ERRL_SEVERITY i_sev,
                          const uint32_t i_userData1,
                          const uint32_t i_userData2)
{
    errlHndl_t  l_err = INVALID_ERR_HNDL;
    uint64_t    l_time = 0;
    uint16_t    l_maxSize = 0;
    uint8_t     l_id = 0;
    uint8_t     l_errSlot = getErrSlotNumAndErrId(ERRL_SEV_PGPE_ERROR, &l_id, &l_time, &l_maxSize);
    static uint8_t L_traceCount = 5;


    if ( l_errSlot != ERRL_INVALID_SLOT )
    {
        TRAC_INFO("createPgpeErrl: Creating error log in slot[%d] max size[%d]", l_errSlot, l_maxSize);

       // Trace a few regs for PGPE debug
       uint32_t l_oisr0_status; // OCC Interrupt Source 0 Register
       uint32_t l_oisr1_status; // OCC Interrupt Source 1 Register
       uint32_t l_occs2_status; // OCC Scratch 2
       l_oisr0_status = in32(OCB_OISR0);
       l_oisr1_status = in32(OCB_OISR1);
       l_occs2_status = in32(OCB_OCCS2);
       TRAC_IMP("createPgpeErrl: OISR0=0x%08X, OISR1=0x%08X, OCCS2=0x%08X",
                l_oisr0_status, l_oisr1_status, l_occs2_status);

        // get slot pointer
        l_err = G_occErrSlots[ l_errSlot ];

        // save off maximum error log size
        l_err->iv_maxSize = l_maxSize;

        // save off default size
        l_err->iv_userDetails.iv_entrySize = sizeof( ErrlEntry_t );

        // add error history
        addErrHistory( l_err );

        // if this is a WOF error add WOF parameters to error log
        // only add for FSP systems where there is support for larger error logs
        if( (i_reasonCode == WOF_DISABLED_RC) &&
            (G_occ_interrupt_type == FSP_SUPPORTED_OCC) )
        {
           addUsrDtlsToErrl( l_err,
                             (uint8_t*)&(g_amec_sys.wof),
                             sizeof(amec_wof_t),
                             ERRL_USR_DTL_STRUCT_VERSION_1,
                             ERRL_USR_DTL_WOF_DATA);
        }

        // add PGPE specific data
        addPgpeDataToErrl( l_err );

        // add trace last, it will only add upto size that is left
        addTraceToErrl( NULL, DEFAULT_TRACE_SIZE,  l_err );

        // save off entry Id
        l_err->iv_entryId = l_id;

        //Save off version info
        l_err->iv_version = ERRL_STRUCT_VERSION_1;

        // save off time
        l_err->iv_userDetails.iv_timeStamp = l_time;

        // set severity
        l_err->iv_severity = i_sev;

        l_err->iv_extendedRC = i_extReasonCode;

        // save off user detail section version
        l_err->iv_userDetails.iv_version = ERRL_USR_DTL_STRUCT_VERSION_1;

        // save off rest of input parameters
        l_err->iv_userDetails.iv_modId = i_modId;
        l_err->iv_reasonCode = i_reasonCode;
        l_err->iv_userDetails.iv_userData1 = i_userData1;
        l_err->iv_userDetails.iv_userData2 = i_userData2;

        // set callout count to 0
        l_err->iv_numCallouts = 0;

        // save off occ fields
        l_err->iv_userDetails.iv_fclipHistory = g_amec->proc[0].chip_f_reason_history;
        l_err->iv_userDetails.iv_occId = G_pbax_id.chip_id;
        l_err->iv_userDetails.iv_occRole = G_occ_role;
        l_err->iv_userDetails.iv_operatingState = CURRENT_STATE();
    }
    else
    {
        if( L_traceCount > 0 )
        {
            TRAC_INFO("createPgpeErrl: Error Logs are FULL - Slot [%d]", l_errSlot);
            L_traceCount--;
        }
    }

    return l_err;
}

// Function Specification
//
// Name:  addTraceToErrl
//
// Description: Add trace to an error log
//
// End Function Specification
void addTraceToErrl(
            const trace_descriptor_array_t* i_trace,
            const uint16_t i_traceSz,
            errlHndl_t io_err)
{
    UINT l_expectLen = 0, l_rtLen = 0, l_bytes_left;
    void * l_traceAddr = io_err;
    uint16_t l_actualSizeOfUsrDtls = 0;

    ocb_oisr1_t  l_oisr1_status;       // OCC Interrupt Source 1 Register

    static bool L_sys_checkstop_traced = FALSE;

    // Check if there is any system checkstop
    l_oisr1_status.value = in32(OCB_OISR1);

    // Level triggered interrupts?
    if (l_oisr1_status.fields.check_stop_ppc405 &&
        !L_sys_checkstop_traced)
    {
            L_sys_checkstop_traced = TRUE;
            TRAC_IMP("addTraceToErrl: System checkstop detected: ppc405, OISR1[0x%08x]",
                     l_oisr1_status.value);
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
        ((io_err->iv_userDetails.iv_entrySize + sizeof(ErrlUserDetailsEntry_t)) < io_err->iv_maxSize) &&
        ((i_trace==&g_des_array[INF_TRACE_DESCRIPTOR]) ||
         (i_trace==&g_des_array[ERR_TRACE_DESCRIPTOR]) ||
         (i_trace==&g_des_array[IMP_TRACE_DESCRIPTOR]) ||
         (i_trace==NULL)) )
    {
        //local copy of the usr details entry
        ErrlUserDetailsEntry_t l_usrDtlsEntry;
        uint16_t l_headerSz = sizeof( l_usrDtlsEntry );

        //adjust user details entry size to available size (word align )
        uint16_t l_availableSize = io_err->iv_maxSize - (io_err->iv_userDetails.iv_entrySize + l_headerSz );
        l_usrDtlsEntry.iv_size = ( i_traceSz < l_availableSize ) ? i_traceSz : l_availableSize;

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
// Name:  addErrHistory
//
// Description: Add error trace history to log
//
// End Function Specification
void addErrHistory(errlHndl_t io_err)
{
    // 1. Check if error log is not null
    // 2. error log is not invalid
    // 3. error log has not been commited
    // 4. free space is enough (to hold new user detail header + data)
    if( (io_err != NULL) &&
        (io_err != INVALID_ERR_HNDL) &&
        (io_err->iv_userDetails.iv_committed == 0) &&
        ((io_err->iv_userDetails.iv_entrySize + sizeof(ErrlUserDetailsEntry_t) + sizeof(G_error_history)) < io_err->iv_maxSize ) )
    {
        void * l_errPtr = io_err;
        ErrlUserDetailsEntry_t l_usrDtlsEntry;
        const uint16_t l_usrDtlsHeaderSz = sizeof(l_usrDtlsEntry);

        //  io_err  |-----------------------------------------|
        //          | ErrlEntry_t                             |
        //          | {iv_userDetails.iv_userDetailEntrySize, | <== elog header and optional user details sections, need to add length
        //          |       ... other elements ...           }|     of new usrdtl section(ErrlUserDetailsEntry_t + data len)
        //          |-----------------------------------------|
        //          | ErrlUserDetailsEntry_t                  | <== new user detail header (l_usrDtlsEntry)
        //          |-----------------------------------------|
        //          | data ...                                | <== new user detail data (G_error_history)
        //          |      ...                                |

        l_usrDtlsEntry.iv_size = sizeof(G_error_history);
        l_usrDtlsEntry.iv_type = (uint8_t) ERRL_USR_DTL_HISTORY_DATA;
        l_usrDtlsEntry.iv_version = 1;

        // Calculate trace data address (err address + sizeof(ErrlEntry_t + existing usrDtls) + sizeof(ErrlUserDetailsEntry_t))
        void * l_dataAddr = l_errPtr + io_err->iv_userDetails.iv_entrySize + l_usrDtlsHeaderSz;
        memcpy(l_dataAddr, G_error_history, sizeof(G_error_history));

        // Calculate entire data length including usrDtl header.
        const uint16_t l_sizeOfUsrDtls = l_usrDtlsHeaderSz + l_usrDtlsEntry.iv_size;

        // Copy new user details header into log
        memcpy(l_errPtr+((io_err->iv_userDetails.iv_entrySize)), &l_usrDtlsEntry, l_usrDtlsHeaderSz);

        //update master user details size with new data
        io_err->iv_userDetails.iv_userDetailEntrySize += l_sizeOfUsrDtls;

        //update error log with new size
        io_err->iv_userDetails.iv_entrySize += l_sizeOfUsrDtls;
    }

} // end addErrHistory()

// Function Specification
//
// Name:  addPgpeDataToErrl
//
// Description: Add PGPE specific data to log
//              NOTE:  checking for valid error log and size will be done when addUsrDtlsToErrl() is called
// End Function Specification
void addPgpeDataToErrl(errlHndl_t io_err)
{
    // Add PGPE PK trace
    uint32_t l_trace_sram_addr = in32(PGPE_DEBUG_PTRS_ADDR + PGPE_DEBUG_TRACE_ADDR_OFFSET);
    // use non-cachable address
    l_trace_sram_addr &= 0xF7FFFFFF;

    // get the data length, we will clip to our allowed max size if needed
    uint32_t l_data_length = in32(PGPE_DEBUG_PTRS_ADDR + PGPE_DEBUG_TRACE_SIZE_OFFSET);

    // determine if pgpe data needs to be clipped
    if(l_data_length > MAX_PGPE_DBUG_DATA)
    {
        TRAC_INFO("addPgpeDataToErrl: clipping pgpe trace size from %d to %d",
                   l_data_length, MAX_PGPE_DBUG_DATA);
        l_data_length = MAX_PGPE_DBUG_DATA;
    }

    // make sure address and length are non-zero
    if(l_trace_sram_addr && l_data_length)
    {
       // Add the data to the error log
       addUsrDtlsToErrl( io_err,
                        (uint8_t*)l_trace_sram_addr,
                        (uint16_t)l_data_length,
                        ERRL_USR_DTL_STRUCT_VERSION_1,
                        ERRL_USR_DTL_PGPE_PK_TRACE);

       // set action bit to force this error to be sent to BMC so it is seen even if info
       setErrlActions(io_err, ERRL_ACTIONS_FORCE_SEND);
    }
    else
    {
        TRAC_ERR("PGPE trace address[%d] or length[%d] is 0!", l_trace_sram_addr, l_data_length);
    }

} // end addPgpeDataToErrl()

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

    TRAC_INFO("Reporting error @ %p with size %d",i_err, i_entrySize );
    TRAC_INFO("ModID: 0x%08X, RC: 0x%08X, UserData1: 0x%08X, UserData2: 0x%08X",
           i_err->iv_userDetails.iv_modId, i_err->iv_reasonCode,
           i_err->iv_userDetails.iv_userData1, i_err->iv_userDetails.iv_userData2);

    // Defer the interrupt if FIR collection is required
    if (!G_fir_collection_required)
    {
        // If this system is not FSP, send an interrupt to Host so that
        // Host can inform HTMGT to collect the error log
        if (G_occ_interrupt_type != FSP_SUPPORTED_OCC)
        {
            notify_host(INTR_REASON_HTMGT_SERVICE_REQUIRED);
        }
    }
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
    ocb_oisr1_t l_oisr1_status;

    static bool L_log_commits_suspended_by_safe_mode = FALSE;

    if (!L_log_commits_suspended_by_safe_mode && io_err != NULL)
    {
        // check if handle is valid and is NOT empty
        if ((*io_err != NULL ) && ( *io_err != INVALID_ERR_HNDL ))
        {
            // Check if there is a system checkstop
            l_oisr1_status.value = in32(OCB_OISR1);

            if (l_oisr1_status.fields.check_stop_ppc405)
            {
                TRAC_IMP("commitErrl: System checkstop detected: ppc405, OISR1[0x%08x]",
                         l_oisr1_status.value);
                //Go to the reset state to minimize errors
                reset_state_request(RESET_REQUESTED_DUE_TO_ERROR);

                //clear out all other actions and set the safe mode req'd action
                (*io_err)->iv_actions.word = ERRL_ACTIONS_SAFE_MODE_REQUIRED;

                //set severity to informational
                (*io_err)->iv_severity = ERRL_SEV_INFORMATIONAL;

                //set callouts to 0
                (*io_err)->iv_numCallouts = 0;

                TRAC_IMP("SAFE mode required, suspending error log commits. FIR capture required.");

                // Suspend all error log commits
                L_log_commits_suspended_by_safe_mode = TRUE;

                // Motivate FIR data collection
                G_fir_collection_required = TRUE;
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
            dcache_flush( *io_err, (*io_err)->iv_maxSize );

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
    else
    {
        TRAC_ERR("Callout type 0x%02X was NOT added to elog", i_type);
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
        ((io_err->iv_userDetails.iv_entrySize) < io_err->iv_maxSize))
    {
        //local copy of the usr details entry
        ErrlUserDetailsEntry_t l_usrDtlsEntry;
        uint16_t l_headerSz = sizeof( l_usrDtlsEntry );

        //adjust user details entry size to available size (word align )
        uint16_t l_availableSize = io_err->iv_maxSize - (io_err->iv_userDetails.iv_entrySize + l_headerSz );
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
void setErrlActions(errlHndl_t io_err, const uint8_t i_mask)
{
    // check if handle is valid
    // NOT empty
    // not committed
    if ( (io_err != NULL )
         && ( io_err != INVALID_ERR_HNDL )
         && (io_err->iv_userDetails.iv_committed == 0) )
    {
        // set the appropriate action bits
        io_err->iv_actions.word |= i_mask;
    }
}

