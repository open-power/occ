/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/firdata/scoms.C $                                     */
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

#include <native.h>
#include <scom_trgt.h>
#include <scom_util.h>

/*------------------------------------------------------------------------------ */

SCOM_Trgt_t SCOM_Trgt_getTrgt( TrgtType_t i_type, uint8_t i_procPos,
                               uint8_t i_procUnitPos, uint32_t i_fsiBaseAddr,
                               bool i_isMaster )
{
    SCOM_Trgt_t trgt = {
        .type        = i_type,
        .procPos     = i_procPos,
        .procUnitPos = i_procUnitPos,
        .isMaster    = i_isMaster,
        .fsiBaseAddr = i_fsiBaseAddr,
    };

    if ( PROC == trgt.type ) trgt.procUnitPos = 0;

    if ( MEMB == trgt.type || MBA == trgt.type ) trgt.isMaster = false;

    return trgt;
}

/*------------------------------------------------------------------------------ */

uint8_t SCOM_Trgt_getChipPos( SCOM_Trgt_t i_trgt )
{
    uint32_t p = 0;

    switch ( i_trgt.type )
    {
        case PROC:
        case EX:
        case MCS:
            p = i_trgt.procPos;
            break;

        case MEMB:
            p = (i_trgt.procPos * MAX_MEMB_PER_PROC) + i_trgt.procUnitPos;
            break;

        case MBA:
            p = (i_trgt.procPos     * MAX_MEMB_PER_PROC) +
                (i_trgt.procUnitPos / MAX_MBA_PER_MEMB);
            break;

        default: ;
    }

    return p;
}

/*------------------------------------------------------------------------------ */

uint8_t SCOM_Trgt_getChipUnitPos( SCOM_Trgt_t i_trgt )
{
    uint32_t u = 0;

    switch ( i_trgt.type )
    {
        case PROC:
        case MEMB: u = 0; break;

        case EX:
        case MCS:  u = i_trgt.procUnitPos; break;

        case MBA:  u = i_trgt.procUnitPos % MAX_MBA_PER_MEMB; break;

        default: ;
    }

    return u;
}

/*------------------------------------------------------------------------------ */

SCOM_Trgt_t SCOM_Trgt_getParentChip( SCOM_Trgt_t i_trgt )
{
    TrgtType_t t = MAX_TRGTS;
    switch ( i_trgt.type )
    {
        case PROC:
        case EX:
        case MCS:  t = PROC; break;

        case MEMB:
        case MBA:  t = MEMB; break;

        default: ;
    }

    uint8_t u = 0;
    switch ( i_trgt.type )
    {
        case PROC:
        case EX:
        case MCS:
        case MEMB: u = i_trgt.procUnitPos;                    break;

        case MBA:  u = i_trgt.procUnitPos / MAX_MBA_PER_MEMB; break;

        default: ;
    }

    return SCOM_Trgt_getTrgt( t, i_trgt.procPos, u, i_trgt.fsiBaseAddr,
                              i_trgt.isMaster );
}

