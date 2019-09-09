/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe0/firdata/scom_trgt.c $                            */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2019                        */
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

/*----------------------------------------------------------------------------*/

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

    if (TRGT_PROC == trgt.type)
    {
        trgt.procUnitPos = 0;
    }

    /* Only processor targets can be on the master processor. */
    if ( TRGT_OCMB == trgt.type )
    {
        trgt.isMaster = false;
    }

    return trgt;
}

/*----------------------------------------------------------------------------*/

uint8_t SCOM_Trgt_getChipPos( SCOM_Trgt_t i_trgt )
{
    uint32_t p = 0;

    switch ( i_trgt.type )
    {
        case TRGT_PROC:
        case TRGT_CAPP:
        case TRGT_XBUS:
        case TRGT_OBUS:
        case TRGT_PEC:
        case TRGT_PHB:
        case TRGT_EQ:
        case TRGT_EX:
        case TRGT_EC:
        case TRGT_MCBIST:
        case TRGT_MCS:
        case TRGT_MCA:
        case TRGT_MC:
        case TRGT_MI:
        case TRGT_MCC:
        case TRGT_OMIC:
        case TRGT_NPU:
            p = i_trgt.procPos;
            break;

        case TRGT_OCMB:
            p = (i_trgt.procPos * MAX_OCMB_PER_PROC) + i_trgt.procUnitPos;
            break;

        default: ;
    }

    return p;
}

/*----------------------------------------------------------------------------*/

uint8_t SCOM_Trgt_getChipUnitPos( SCOM_Trgt_t i_trgt )
{
    uint32_t u = 0;

    switch ( i_trgt.type )
    {
        case TRGT_PROC:
        case TRGT_OCMB: u = 0; break;

        case TRGT_CAPP:
        case TRGT_XBUS:
        case TRGT_OBUS:
        case TRGT_PEC:
        case TRGT_PHB:
        case TRGT_EQ:
        case TRGT_EX:
        case TRGT_EC:
        case TRGT_MCBIST:
        case TRGT_MCS:
        case TRGT_MCA:
        case TRGT_MC:
        case TRGT_MI:
        case TRGT_MCC:
        case TRGT_OMIC:
        case TRGT_NPU: u = i_trgt.procUnitPos; break;

        default: ;
    }

    return u;
}

/*----------------------------------------------------------------------------*/

SCOM_Trgt_t SCOM_Trgt_getParentChip( SCOM_Trgt_t i_trgt )
{
    TrgtType_t t = TRGT_MAX;
    switch ( i_trgt.type )
    {
        case TRGT_PROC:
        case TRGT_CAPP:
        case TRGT_XBUS:
        case TRGT_OBUS:
        case TRGT_PEC:
        case TRGT_PHB:
        case TRGT_EQ:
        case TRGT_EX:
        case TRGT_EC:
        case TRGT_MCBIST:
        case TRGT_MCS:
        case TRGT_MCA:
        case TRGT_MC:
        case TRGT_MI:
        case TRGT_MCC:
        case TRGT_OMIC:
        case TRGT_NPU:   t = TRGT_PROC; break;

        case TRGT_OCMB:  t = TRGT_OCMB; break;

        default: ;
    }

    uint8_t u = 0;
    switch ( i_trgt.type )
    {
        case TRGT_PROC:
        case TRGT_CAPP:
        case TRGT_XBUS:
        case TRGT_OBUS:
        case TRGT_PEC:
        case TRGT_PHB:
        case TRGT_EQ:
        case TRGT_EX:
        case TRGT_EC:
        case TRGT_MCBIST:
        case TRGT_MCS:
        case TRGT_MCA:
        case TRGT_MC:
        case TRGT_MI:
        case TRGT_MCC:
        case TRGT_OMIC:
        case TRGT_NPU:
        case TRGT_OCMB: u = i_trgt.procUnitPos; break;

        default: ;
    }

    return SCOM_Trgt_getTrgt( t, i_trgt.procPos, u, i_trgt.fsiBaseAddr,
                              i_trgt.isMaster );
}

