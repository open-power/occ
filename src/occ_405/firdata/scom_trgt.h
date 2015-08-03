/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/firdata/scom.H $                                      */
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

#ifndef __scom_trgt_h
#define __scom_trgt_h

#include <firDataConst_common.h>
#include <native.h>

typedef struct
{
    /** See enum TrgtType_t. NOTE: This value is not consistant with Hostboot
     *  target types. */
    TrgtType_t type;

    /** Absolute position of the connected PROC within the node. This value
     *  should be consistant with the Hostboot target positions. */
    uint8_t procPos;

    /** Unit position relative to the connected PROC. This value should be
     *  consistant with the Hostboot target positions. */
    uint8_t procUnitPos;

    /** Indicates this target is, or is connected to, the master processor. */
    bool isMaster;

    /** This target's FSI base address. */
    uint32_t fsiBaseAddr;

} SCOM_Trgt_t;

/** @param  i_type        See enum Type.
 *  @param  i_procPos     Absolute position within the node of the connected
 *                        PROC target.
 *  @param  i_procUnitPos Unit position relative to the connected PROC. Will be
 *                        explicitly set to 0 for PROC targets.
 *  @param  i_fsiBaseAddr For EX and MCS, the FSI base address for the
 *                        connected PROC. For MEMB and MBA, the FSI base
 *                        address for the connected MEMB.
 *  @param  i_isMaster    True, if this target is, or is connected to, the
 *                        master processor. False, otherwise. Will be explicitly
 *                        set to false for MEMB and MBA targets.
 *  @return A SCOM_Trgt_t struct.
 */
SCOM_Trgt_t SCOM_Trgt_getTrgt( TrgtType_t i_type, uint8_t i_procPos,
                               uint8_t i_procUnitPos, uint32_t i_fsiBaseAddr,
                               bool i_isMaster );

/** @param  i_trgt The SCOM target.
 *  @return This target's absolute position of the parent chip (PROC or
 *          MEMB) within the node.
 */
uint8_t SCOM_Trgt_getChipPos( SCOM_Trgt_t i_trgt );

/** @param  i_trgt The SCOM target.
 *  @return This target's unit position relative to the parent chip. Only
 *          valid for EX, MCS, and MBA units. Will return 0 for PROC and
 *          MEMB chips.
 */
uint8_t SCOM_Trgt_getChipUnitPos( SCOM_Trgt_t i_trgt );

/** @param  i_trgt The SCOM target.
 *  @return A target for the containing parent chip (PROC or MEMB).
 */
SCOM_Trgt_t SCOM_Trgt_getParentChip( SCOM_Trgt_t i_trgt );

#endif /* __scom_trgt_h */
