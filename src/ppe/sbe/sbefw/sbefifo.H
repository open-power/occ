/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/sbe/sbefw/sbefifo.H $                                 */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
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
/*
 * @file: ppe/sbe/sbefw/sbefifo.H
 *
 * @brief This file contains the SBE FIFO Commands
 *
 */

#ifndef __SBEFW_SBEFIFO_H
#define __SBEFW_SBEFIFO_H

#include "sbeexeintf.H"
#include "sbetrace.H"
#include "ppe42_scom.h"
#include "sbe_sp_intf.H"

/**
  * @brief SBE FIFO Access addresses
  *
  */
const uint32_t SBE_FIFO_BASE                     =  0x000B0000;

const uint32_t SBE_UPSTREAM_FIFO_DEQ_ADD         = SBE_FIFO_BASE + 0x0000;
const uint32_t SBE_UPSTREAM_FIFO_STATUS          = SBE_FIFO_BASE + 0x0001;
const uint32_t SBE_UPSTREAM_FIFO_SIGNAL_EOT      = SBE_FIFO_BASE + 0x0002;
const uint32_t SBE_UPSTREAM_FIFO_REQ_RESET       = SBE_FIFO_BASE + 0x0003;
const uint32_t SBE_UPSTREAM_FIFO_PERFORM_RESET   = SBE_FIFO_BASE + 0x0004;
const uint32_t SBE_UPSTREAM_FIFO_ACK_EOT         = SBE_FIFO_BASE + 0x0005;

const uint32_t SBE_DOWNSTREAM_FIFO_ENQ_ADD       = SBE_FIFO_BASE + 0x0010;
const uint32_t SBE_DOWNSTREAM_FIFO_STATUS        = SBE_FIFO_BASE + 0x0011;
const uint32_t SBE_DOWNSTREAM_FIFO_SIGNAL_EOT    = SBE_FIFO_BASE + 0x0012;
const uint32_t SBE_DOWNSTREAM_FIFO_REQ_RESET     = SBE_FIFO_BASE + 0x0013;
const uint32_t SBE_DOWNSTREAM_FIFO_PERFORM_RESET = SBE_FIFO_BASE + 0x0014;
const uint32_t SBE_DOWNSTREAM_FIFO_ACK_EOT       = SBE_FIFO_BASE + 0x0015;

/**
  * @brief SBE Upstream FIFO Status bits
  *
  */

typedef struct
{
    uint32_t   valid_flag:1;         // Bit 0
    uint32_t   eot_flag:1;           // Bit 1
    uint32_t   parity_err:1;         // Bit 2
    uint32_t   reserved3_5:3;        // Bit 3:5
    uint32_t   req_upfifo_reset:1;   // Bit 6
    uint32_t   req_downfifo_reset:1; // Bit 7
    uint32_t   signaling_eot:1;      // Bit 8
    uint32_t   reserved9:1;          // Bit 9
    uint32_t   fifo_full:1;          // Bit 10
    uint32_t   fifo_empty:1;         // Bit 11
    uint32_t   fifo_entry_count:4;   // Bit 12:15
    uint32_t   fifo_valid_flags:8;   // Bit 16:23
    uint32_t   fifo_eot_flags:8;     // Bit 24:31

} sbe_upfifo_status_bitset_t ;

typedef union
{
    sbe_upfifo_status_bitset_t upfifo_status_bitset;
    uint32_t upfifo_status_uint32;
} sbe_upfifo_status_t;

/**
  * @brief FIFO access return codes for internal purpose
  *
  */
typedef enum
{
    SBE_FIFO_RC_ACCESS_SUCCESS = 0,
    SBE_FIFO_RC_RESET          = 0xE0,
    SBE_FIFO_RC_FULL,
    SBE_FIFO_RC_EMPTY,
    SBE_FIFO_RC_DUMMY_DATA,
    SBE_FIFO_RC_EOT_ACKED,
    SBE_FIFO_RC_EOT_ACK_FAILED,
    SBE_FIFO_RC_UNKNOWN,
} sbe_fifo_access_rc_t;


/**
  * @brief 64-bit DW structure for Upstream FIFO Read
  *
  */
typedef struct
{
    uint32_t upfifo_data;
    sbe_upfifo_status_t upfifo_status;
} sbe_upfifo_entry_t ;


/**
  * @brief SBE Downstream FIFO Status bits
  *
  */
typedef struct
{
    uint32_t   reserved0_1:2;        // Bit 0:1
    uint32_t   parity_err:1;         // Bit 2
    uint32_t   reserved3_5:3;        // Bit 3:5
    uint32_t   req_downfifo_reset:1; // Bit 6
    uint32_t   req_upfifo_reset:1;   // Bit 7
    uint32_t   signaling_eot:1;      // Bit 8
    uint32_t   reserved9:1;          // Bit 9
    uint32_t   fifo_full:1;          // Bit 10
    uint32_t   fifo_empty:1;         // Bit 11
    uint32_t   fifo_entry_count:4;   // Bit 12:15
    uint32_t   fifo_valid_flags:8;   // Bit 16:23
    uint32_t   fifo_eot_flags:8;     // Bit 24:31

} sbe_downfifo_status_bitset_t ;

typedef union
{
    sbe_downfifo_status_bitset_t downfifo_status_bitset;
    uint32_t                     downfifo_status_uint32;
} sbe_downfifo_status_t;


// @TODO via RTC 129073.
// Put these structures in separate file as these are not FIFO specific.
// Also make sure all FIFO structures are 32 bit alligned ( the largest
// member should be atleast 4 byte). It is required as in sbe fifo
// operation we are casting these structures to uint32_t pointer. It can
// cause allignment issue if largest member of structure is not atleast
// 32 bit. We can use bit fields to optimize  memory requirements.
/**
  * @brief Command Request Header
  */
typedef struct
{
    uint32_t len;
    uint16_t reserved;
    uint8_t cmdClass;
    uint8_t command;
}sbeCmdReqBuf_t;

extern sbeCmdReqBuf_t g_sbeCmdHdr;

/**
  * @brief structure for generic header for fifo response.
  *
  */
typedef struct
{
    uint16_t magicCode;
    uint8_t cmdClass;
    uint8_t command;
    uint16_t primaryStatus;
    uint16_t secondaryStatus;

    /**
      * @brief set the primary and secondary status
      *
      * @param[in] i_prim  Primary status
      * @param[in] i_sec   Secondary status
      *
      * @return
      */
    void setStatus( const uint16_t i_prim, const uint16_t i_sec)
    {
        primaryStatus = i_prim;
        secondaryStatus = i_sec;
    }

    /**
      * @brief set initial values for response header
      *
      * @note  We did not set this in constructor as based on use case
      *        it is possible that g_sbeCmdHdr does not have proper
      *        values at time of object creation.
      *
      */
    void init()
    {
        magicCode = 0xC0DE;
        cmdClass  = g_sbeCmdHdr.cmdClass;
        command = g_sbeCmdHdr.command;
        primaryStatus = SBE_PRI_OPERATION_SUCCESSFUL;
        secondaryStatus = SBE_SEC_OPERATION_SUCCESSFUL;
    }

}sbeResponseGenericHeader_t;

/**
  * @brief structure for ffdc header for fifo response.
  *
  */
typedef struct sbeResponseFfdc
{
    uint16_t magicBytes;
    uint16_t lenInWords;  // length in word( 4 byte )
    //@TODO via RTC 129073.
    //make fapiRc 64 bit
    uint32_t fapiRc;

    /**
      * @brief set rc
      *
      * @param[in] i_rc  FAPI RC
      *
      * @return
      */
    void setRc(const uint32_t i_rc)
    {
        fapiRc = i_rc;
    }

    /**
      * @brief constructor
      *
      * @param[in] i_rc  FAPI RC
      *
      * @return
      */
    sbeResponseFfdc()
    {
        magicBytes = 0xFFDC;
        //TODO via 129076.
        //Need to change value for length once FFDC design is final.
        lenInWords  = ( sizeof(magicBytes) + sizeof(lenInWords)
                            + sizeof(fapiRc) )/ sizeof(uint32_t);
        fapiRc = 0;
    }
}sbeResponseFfdc_t;

/**
  * @brief structure for execute istep chipop (0xA101) contents.
  *
  */
typedef struct
{
    uint8_t reserved1;
    uint8_t major;
    uint8_t reserved2;
    uint8_t minor;
}sbeIstepReqMsg_t;


/**
  * @brief Command response structure to hold the primary and secondary
  *        status values. This will be utilized when a command class
  *        validation or state machine check fails.
  *
  */
typedef struct
{
    uint16_t          prim_status ;    // Primary Response Status
    uint16_t          sec_status  ;    // Secondary Response Status
} sbeCmdRespHdr_t;

extern sbeCmdRespHdr_t g_sbeCmdRespHdr;

typedef struct
{
    uint16_t magic_bytes;
    uint16_t len;
} sbeCmdResp_FFDC_t;


/*****************************************************************/
/** Upstream FIFO access utilities **/
/*****************************************************************/

/**
  * @brief sbeUpFifoDeq : Read entry and status from Upstream FIFO
  *
  * @param[out] 64-Bit Data read from Upstream FIFO
  *
  * @return     Rc from the underlying scom utility
  *
  */
extern inline uint32_t sbeUpFifoDeq (uint64_t *o_data)
{
    /* For SBE FIFO (PIB) access, chiplet ID should be passed as 0 */
    return getscom(0, SBE_UPSTREAM_FIFO_DEQ_ADD, o_data);
}


/**
  * @brief sbeUpFifoPerformReset : Perform Upstream FIFO reset request
  *
  * @return     Rc from the underlying scom utility
  *
  */
extern inline uint32_t sbeUpFifoPerformReset (void)
{
    SBE_TRACE(">sbeUpFifoPerformReset");
    return putscom(0, SBE_UPSTREAM_FIFO_PERFORM_RESET, ((uint64_t)0x1)<<32);
}


/**
  * @brief sbeUpFifoAckEot : Acknowledge EOT in Upstream FIFO
  *
  * @return     Rc from the underlying scom utility
  *
  */
extern inline uint32_t sbeUpFifoAckEot (void)
{
    SBE_DEBUG("sbeUpFifoAckEot");

    return putscom(0, SBE_UPSTREAM_FIFO_ACK_EOT, ((uint64_t)0x1)<<32);
}


/*****************************************************************/
/** Downstream FIFO access utilities **/
/*****************************************************************/

/**
  * @brief sbeDownFifoEnq : Write data into Downstream FIFO
  *
  * @param[in] 64-Bit Data write into Downstream FIFO
  *            Bit 0-31  : Data
  *            Bit 32-63 : Unused
  *
  * @return    Rc from the underlying scom utility
  */
extern inline uint32_t sbeDownFifoEnq (const uint64_t i_data)
{
    SBE_DEBUG(">sbeDownFifoEnq");
    return putscom(0, SBE_DOWNSTREAM_FIFO_ENQ_ADD, i_data);
}


/**
  * @brief sbeDownFifoGetStatus : Read status from downstream FIFO
  *
  * @param[out] 64-Bit Read status from downstream FIFO
  *            Bit 0-31  : Data
  *            Bit 32-63 : Unused
  *
  * @return    Rc from the underlying scom utility
  */
extern inline uint32_t sbeDownFifoGetStatus (uint64_t *o_data)
{
    SBE_DEBUG(">sbeDownFifoStatus");
    return getscom(0, SBE_DOWNSTREAM_FIFO_STATUS, o_data);
}

/**
  * @brief sbeDownFifoSignalEot : Signal EOT in Downstream FIFO
  *
  * @return     Rc from the underlying scom utility
  *
  */
extern inline uint32_t sbeDownFifoSignalEot (void)
{
    SBE_DEBUG(">sbeDownFifoSignalEot");
    return putscom(0, SBE_DOWNSTREAM_FIFO_SIGNAL_EOT, ((uint64_t)0x1)<<32);
}


/**********************************************************************/
// Utilities
/**********************************************************************/

/**
  * @brief sbeUpFifoDeq_mult : Dequeue multiple entries from upstream FIFO
  *
  * @param[in/out] io_len
  *                    number of entries to dequeue as input,
  *                    number of entries dequeued as output without
  *                    taking EOT dummy entry into consideration
  * @param[out]    o_pData  entries dequeued into the buffer
  * @param[in]     i_flush true / false
  *                    true - caller requested FIFO flush,
  *                       io_len would be returned as 0
  *                    false - default case,
  *                       io_len would be number of entries dequeued
  *
  * @return        Rc from the underlying scom utility
  *
  */
extern uint32_t sbeUpFifoDeq_mult (uint8_t     &io_len,
                                   uint32_t    *o_pData,
                                   const bool  i_flush = false);


/**
  * @brief sbeDownFifoEnq_mult : Enqueue into downstream FIFO
  *
  * @param[in/out] io_len   number of entries to enqueue as input,
  *                         number of entries enqueued as output
  * @param[in]     i_pData  buffer containting data to be enqueued
  *
  * @return        Rc from the underlying scom utility
  *
  */
extern uint32_t sbeDownFifoEnq_mult (uint8_t        &io_len,
                                     const uint32_t *i_pData) ;


/**
  * @brief sbeBuildRespHeaderMagicCodeCmdClass
  *          Builds the header word containing the magic code,
  *          the command class and the opcode
  *
  * @return     Returns the header word in the response header
  *             containing  the magic code, command class and opcode
  *
  */
extern inline uint32_t sbeBuildRespHeaderMagicCodeCmdClass (void)
{
    return ( (0xC0DE0000 ) |
             (uint32_t)(g_sbeCmdHdr.cmdClass << 8) |
             (uint32_t)(g_sbeCmdHdr.command ));
}

/**
  * @brief sbeBuildRespHeaderStatusWordGlobal
  *          Builds the status header word from global variables
  *
  * @return     Returns the status word in the response header
  *
  */
extern inline uint32_t sbeBuildRespHeaderStatusWordGlobal (void)
{
    return ( (((uint32_t)g_sbeCmdRespHdr.prim_status)<<16) |
                        (g_sbeCmdRespHdr.sec_status) );
}

/**
  * @brief sbeBuildRespHeaderStatusWordLocal
  *          Builds the status header word from passed in parameters
  *
  * @param[in]  const uint16_t i_primStatus Primary Response Status Code
  * @param[in]  const uint16_t i_secStatus  Secondary Response Status Code
  *
  * @return     Returns the status word in the response header
  *
  */
extern inline uint32_t sbeBuildRespHeaderStatusWordLocal (
                                const uint16_t i_primStatus,
                                const uint16_t i_secStatus)
{
    return ( (((uint32_t)i_primStatus)<<16) | (i_secStatus) );
}

/**
 * @brief sbeBuildMinRespHdr : Builds minimum response header
 *
 * @desc  This builds the buffer with the following status words
 *           - Magic Bytes, Command Class, Command opcode
 *           - Primary Status Code, Secondary Status Code
 *           - PCB / PIB Status Code [optional]
 *           - Distance to Status Header
 * @param[in/out]    uint32_t *io_pBuf      Buffer to be filled in
 * @param[in/out]    uint8_t  &io_curIndex  Current Index into the buffer
 * @param[in]  const uint16_t  i_primStatus Primary Response Status Code
 * @param[in]  const uint16_t  i_secStatus  Secondary Response Status Code
 * @param[in]  const uint32_t  i_pcbpibStatus PCB-PIB Response Status Code
 * @param[in]  const uint8_t   i_startIndex   Starting Index into the buffer
 */

void sbeBuildMinRespHdr ( uint32_t *io_pBuf,
                          uint8_t  &io_curIndex,
                    const uint16_t  i_primStatus,
                    const uint16_t  i_secStatus,
                    const uint32_t  i_pcbpibStatus,
                    const uint8_t   i_startIndex = 0 );


#endif // __SBEFW_SBEFIFO_H
