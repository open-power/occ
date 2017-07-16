/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/firdata/firData.c $                               */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2017                        */
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

#include <homerData_common.h>
#include <pnorData_common.h>
#include <pnor_util.h>
#include <scom_trgt.h>
#include <scom_util.h>

/** Keeps track of pointers to register lists in the HOMER data for each target
 *  type. */
typedef struct
{
    uint32_t * glbl;     /*/< Global registers */
    uint32_t * fir;      /*/< Normal FIRs */
    uint32_t * reg;      /*/< Normal registers */
    uint64_t * idFir;    /*/< Indirect-SCOM FIRs */
    uint64_t * idReg;    /*/< Indirect-SCOM registers */
} FirData_ListPointers_t;

/** Contains pointers and sizes for the HOMER and PNOR data buffers. */
typedef struct
{
    uint8_t      * hBuf;        /*/< Pointer to the HOMER data buffer */
    uint32_t       maxHBufSize; /*/< Maximum size of the HOMER data buffer */
    HOMER_Data_t * hData;       /*/< Pointer to the HOMER header data */

    uint8_t     * pBuf;        /*/< Pointer to the PNOR data buffer */
    uint32_t      maxPBufSize; /*/< Maximum size of the PNOR data buffer */
    PNOR_Data_t * pData;       /*/< Pointer to the PNOR header data */
    uint32_t      pBufSize;    /*/< Current size of the PNOR data buffer */

    FirData_ListPointers_t hPtrs[TRGT_MAX]; /*/< Pointers to the reg lists */

    HOMER_ChipSpecAddr_t * ecDepReg; /*/< EC dependent regs       */

} FirData_t;


/** We move the chiplet exist bit fields into this structure left  */
/** justified for easy checking of the fields with a mask          */
typedef struct
{
    uint32_t xbusMask;
    uint32_t obusMask;
    uint32_t ecMask;

    uint32_t eqMask;
    uint32_t exMask;
    uint32_t mcbist_mc_Mask;
    uint32_t mcs_mi_Mask;
    uint32_t mca_dmi_Mask;

    uint32_t cappMask;
    uint32_t pecMask;
    uint32_t phbMask;

} FirData_existBits_t;

/* Uncomment for additional debug traces */
#if 0
#define DEBUG_PRD_CHKSTOP_ANALYSIS
#endif

/*------------------------------------------------------------------------------ */

/** @brief  Add generic data to the PNOR buffer.
 *  @param  io_fd      The FIR data stuct.
 *  @param  i_data     Pointer to the data to add to the buffer.
 *  @param  i_dataSize Size of the data to add to the buffer.
 *  @return True, if where is no room to add the new data. False, otherwise.
 */
bool FirData_addDataToPnor( FirData_t * io_fd, void * i_data,
                            uint32_t i_dataSize )
{
    bool full = (io_fd->maxPBufSize < io_fd->pBufSize + i_dataSize);

    if ( full )
    {
        /* Indicate the PNOR data is full. */
        io_fd->pData->full = 1;
    }
    else
    {
        /* Copy data to PNOR buffer. */
        memcpy( &io_fd->pBuf[io_fd->pBufSize], i_data, i_dataSize );
        io_fd->pBufSize += i_dataSize;
    }

    return full;
}

/*------------------------------------------------------------------------------ */

/** @brief  SCOMs hardware and adds a normal register to PNOR.
 *  @param  io_fd     The FIR data stuct.
 *  @param  io_pTrgt  Pointer to PNOR target.
 *  @param  i_sTrgt   SCOM target.
 *  @param  i_addr    32-bit address to SCOM.
 *  @param  o_nonZero True if the value of the register was non-zero. False,
 *                    otherwise.
 *  @return True if the PNOR buffer is full, false if there was room.
 */
bool FirData_addRegToPnor( FirData_t * io_fd, PNOR_Trgt_t * io_pTrgt,
                           SCOM_Trgt_t i_sTrgt, uint32_t i_addr,
                           bool * o_nonZero )
{
    bool full = false;

    int32_t rc = 0;
    PNOR_Reg_t reg = { i_addr, 0 };

    *o_nonZero = false;

    do
    {
        rc = SCOM_getScom( i_sTrgt, i_addr, &(reg.val) );
        if ( SUCCESS != rc )
        {
            TRAC_ERR( "[FirData_addRegToPnor] t=%d p=%d u=%d rc=%d ",
                i_sTrgt.type, i_sTrgt.procPos, i_sTrgt.procUnitPos, rc);
            TRAC_ERR( "addr=0x%08x val=0x%08x%08x",i_addr,
                      (uint32_t)(reg.val >> 32), (uint32_t)reg.val );

            if ( io_pTrgt->scomErrs < PNOR_Trgt_MAX_SCOM_ERRORS )
                io_pTrgt->scomErrs++;

            break;
        }

        if ( 0 == reg.val ) break; // Skip zero value registers.

        full = FirData_addDataToPnor( io_fd, &reg, sizeof(reg) );
        if ( full ) break;

        *o_nonZero = true;

        if ( io_pTrgt->regs < PNOR_Trgt_MAX_REGS_PER_TRGT )
            io_pTrgt->regs++;

    } while (0);

#ifdef DEBUG_PRD_CHKSTOP_ANALYSIS
    TRAC_INFO("   addToPnor: Type:%d Pos:%d Unit:%d  Addr:%X  Full:%d",
               i_sTrgt.type, i_sTrgt.procPos, i_sTrgt.procUnitPos,
               i_addr, full );
#endif

    return full;
}

/*------------------------------------------------------------------------------ */

/** @brief  SCOMs hardware and add a indirect-SCOM register to PNOR.
 *  @param  io_fd     The FIR data stuct.
 *  @param  io_pTrgt  Pointer to PNOR target.
 *  @param  i_sTrgt   SCOM target.
 *  @param  i_sTrgt   Target to SCOM.
 *  @param  i_addr    64-bit address to SCOM.
 *  @param  o_nonZero True if the value of the register was non-zero. False,
 *                    otherwise.
 *  @return True if the PNOR buffer is full, false if there was room.
 */
bool FirData_addIdRegToPnor( FirData_t * io_fd, PNOR_Trgt_t * io_pTrgt,
                             SCOM_Trgt_t i_sTrgt, uint64_t i_addr,
                             bool * o_nonZero )
{
    bool full = false;

    int32_t rc = 0;
    PNOR_IdReg_t reg = { i_addr, 0 };

    *o_nonZero = false;

    do
    {
        rc = SCOM_getIdScom( i_sTrgt, i_addr, &(reg.val) );
        if ( SUCCESS != rc )
        {
            if ( io_pTrgt->scomErrs < PNOR_Trgt_MAX_SCOM_ERRORS )
                io_pTrgt->scomErrs++;
            break;
        }

        if ( 0 == reg.val ) break; // Skip zero value registers.

        full = FirData_addDataToPnor( io_fd, &reg, sizeof(reg) );
        if ( full ) break;

        *o_nonZero = true;

        if ( io_pTrgt->idRegs < PNOR_Trgt_MAX_ID_REGS_PER_TRGT )
            io_pTrgt->idRegs++;

    } while (0);

    return full;
}

/*------------------------------------------------------------------------------ */

/** @brief  Iterates a list of global registers and adds them to the PNOR.
 *  @param  io_fd    The FIR data stuct.
 *  @param  io_pTrgt Pointer to PNOR target.
 *  @param  i_sTrgt  SCOM target.
 *  @param  o_noAttn True, if the global registers showed no active attentions
 *                   on the target. False, otherwise.
 *  @param  i_chipStruct  Provides chipType and ecLevel we need
 *                        for EC dependent regs
 *
 *  @return True if the PNOR buffer is full, false if there was room.
 */
bool FirData_addGlblsToPnor( FirData_t * io_fd, PNOR_Trgt_t * io_pTrgt,
                             SCOM_Trgt_t i_sTrgt, bool * o_noAttn,
                             HOMER_Chip_t *i_chipStruct )
{
    bool full = false;
    bool l_isAnyGlobal  = false;  /* true when we find global reg      */
    bool l_isAnyNonZero = false;  /* true if ATTN is active on any reg */

    uint8_t t   = i_sTrgt.type;
    uint8_t cnt = io_fd->hData->regCounts[t][REG_GLBL];

    uint32_t i = 0;

    uint32_t addr    = 0;
    bool     nonZero = false;

    *o_noAttn = false; /* Must be false if there are no global regs. */

    if ( 0 != cnt )
    {
        /** If we get here, we have GLOBALS for this target **/
        l_isAnyGlobal = true;

        for ( i = 0; i < cnt; i++ )
        {
            addr    = io_fd->hPtrs[t].glbl[i];
            nonZero = false;

            full = FirData_addRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr,
                                         &nonZero );
            if ( full ) break;

            if ( nonZero ) l_isAnyNonZero = true;
        }
    }


    /** Handle EC dependent registers, if any */
    HOMER_ChipSpecAddr_t  *l_ecAddrtPtr =  io_fd->ecDepReg;
    cnt = io_fd->hData->ecDepCounts;

    /** We have a structure with one element for each EC dep address. */
    /** Need to loop thru and verify we have same regType among       */
    /** other things prior to using the address.                      */
    for ( i = 0; i < cnt; i++ )
    {
#ifdef DEBUG_PRD_CHKSTOP_ANALYSIS
        TRAC_INFO(" addGlblsToPnor In/Array: chipType:%X::%X trgType:%X::%X"
                  " regType:%X::%X ecLevel %X::%X",
                  i_chipStruct->chipType, l_ecAddrtPtr->chipType,
                  t, l_ecAddrtPtr->trgtType,
                  REG_GLBL, l_ecAddrtPtr->regType,
                  i_chipStruct->chipEcLevel, l_ecAddrtPtr->ddLevel );
#endif

        /** Need same chipType (nimbus,cumulus,etc..), same target type, */
        /** same register type and EC level must match too               */
        if ( (l_ecAddrtPtr->chipType == i_chipStruct->chipType)     &&
             (l_ecAddrtPtr->trgtType == t )                         &&
             (l_ecAddrtPtr->regType  == REG_GLBL)                   &&
             (l_ecAddrtPtr->ddLevel  == i_chipStruct->chipEcLevel)
           )
        {
            /** If we get here, we have GLOBALS for this target **/
            l_isAnyGlobal = true;

            /* address is right justified in 64 bits */
            addr = (uint32_t)(l_ecAddrtPtr->address);
            nonZero = false;

            full = FirData_addRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr,
                                         &nonZero );
            if ( full ) break;

            if ( nonZero ) l_isAnyNonZero = true;
        } /* end if we found an EC dep reg */

        /* prep pointer for next element (if any) */
        l_ecAddrtPtr++;

    } /** end EC dependent reg loop */

    /* If we have no GLOBALS, we want to collect other regs */
    /*   so then we want to return 'attn is active)'.       */
    /* If we have GLOBALS, then we need non-zero reg to     */
    /*   collect the rest of the regs (attn is active).     */
    if ( (true == l_isAnyGlobal) && (false == l_isAnyNonZero) )
    {
        /* This indicates we won't collect additional regs */
        *o_noAttn = true; /* starts out as false */
    }

    return full;
}

/*------------------------------------------------------------------------------ */

/** @brief  Iterates a list of FIRs and adds them to the PNOR.
 *  @param  io_fd    The FIR data stuct.
 *  @param  io_pTrgt Pointer to PNOR target.
 *  @param  i_sTrgt  SCOM target.
 *  @param  i_chipStruct  Provides chipType and ecLevel we need
 *                        for EC dependent regs
 *
 *  @return True if the PNOR buffer is full, false if there was room.
 */
bool FirData_addFirsToPnor( FirData_t * io_fd, PNOR_Trgt_t * io_pTrgt,
                            SCOM_Trgt_t i_sTrgt,
                            HOMER_Chip_t *i_chipStruct )
{
    bool full = false;

    uint8_t t   = i_sTrgt.type;
    uint8_t cnt = io_fd->hData->regCounts[t][REG_FIR];

    uint32_t i = 0;

    uint32_t addr    = 0;
    bool     nonZero = false;
    bool     tmp     = false; /* ignored, not used */

    for ( i = 0; i < cnt; i++ )
    {
        addr    = io_fd->hPtrs[t].fir[i];
        nonZero = false;

        /* Add FIR */
        full = FirData_addRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr, &nonZero );
        if ( full ) break;

        /* Add MASK */
        full = FirData_addRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr + 3, &tmp );
        if ( full ) break;

        if ( nonZero )
        {
            /* Add ACT0 */
            full = FirData_addRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr + 6,
                                         &tmp );
            if ( full ) break;

            /* Add ACT1 */
            full = FirData_addRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr + 7,
                                         &tmp );
            if ( full ) break;
        }
    }


    /** Handle EC dependent registers, if any */
    HOMER_ChipSpecAddr_t  *l_ecAddrtPtr =  io_fd->ecDepReg;
    cnt = io_fd->hData->ecDepCounts;

    /** We have a structure with one element for each EC dep address. */
    /** Need to loop thru and verify we have same regType among       */
    /** other things prior to using the address.                      */
    for ( i = 0; i < cnt; i++ )
    {
#ifdef DEBUG_PRD_CHKSTOP_ANALYSIS
        TRAC_INFO(" addFirsToPnor In/Array: chipType:%X::%X trgType:%X::%X"
                  " regType:%X::%X ecLevel %X::%X",
                  i_chipStruct->chipType, l_ecAddrtPtr->chipType,
                  t, l_ecAddrtPtr->trgtType,
                  REG_GLBL, l_ecAddrtPtr->regType,
                  i_chipStruct->chipEcLevel, l_ecAddrtPtr->ddLevel );
#endif

        /** Need same chipType (nimbus,cumulus,etc..), same target type, */
        /** same register type and EC level must match too               */
        if ( (l_ecAddrtPtr->chipType == i_chipStruct->chipType)     &&
             (l_ecAddrtPtr->trgtType == t )                         &&
             (l_ecAddrtPtr->regType  == REG_FIR)                    &&
             (l_ecAddrtPtr->ddLevel  == i_chipStruct->chipEcLevel)
           )
        {   /* address is right justified in 64 bits */
            addr = (uint32_t)(l_ecAddrtPtr->address);
            nonZero = false;

            /* Add FIR */
            full = FirData_addRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr, &nonZero );
            if ( full ) break;

            /* Add MASK */
            full = FirData_addRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr + 3, &tmp );
            if ( full ) break;

            if ( nonZero )
            {
                /* Add ACT0 */
                full = FirData_addRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr + 6,
                                             &tmp );
                if ( full ) break;

                /* Add ACT1 */
                full = FirData_addRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr + 7,
                                         &tmp );
                if ( full ) break;
            }

        } /* end if we found an EC dep reg */

        /* prep pointer for next element (if any) */
        l_ecAddrtPtr++;

    } /** end EC dependent reg loop */


    return full;
}

/*------------------------------------------------------------------------------ */

/** @brief  Iterates a list of REGs and adds them to the PNOR.
 *  @param  io_fd    The FIR data stuct.
 *  @param  io_pTrgt Pointer to PNOR target.
 *  @param  i_sTrgt  SCOM target.
 *  @param  i_chipStruct  Provides chipType and ecLevel we need
 *                        for EC dependent regs
 *
 *  @return True if the PNOR buffer is full, false if there was room.
 */
bool FirData_addRegsToPnor( FirData_t * io_fd, PNOR_Trgt_t * io_pTrgt,
                            SCOM_Trgt_t i_sTrgt,
                            HOMER_Chip_t *i_chipStruct )
{
    bool full = false;

    uint8_t t   = i_sTrgt.type;
    uint8_t cnt = io_fd->hData->regCounts[t][REG_REG];

    uint32_t i = 0;

    uint32_t addr    = 0;
    bool     tmp     = false; /* ignored, not used */

    for ( i = 0; i < cnt; i++ )
    {
        addr = io_fd->hPtrs[t].reg[i];

        full = FirData_addRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr, &tmp );
        if ( full ) break;
    }

    /** Handle EC dependent registers, if any */
    HOMER_ChipSpecAddr_t  *l_ecAddrtPtr =  io_fd->ecDepReg;
    cnt = io_fd->hData->ecDepCounts;

    /** We have a structure with one element for each EC dep address. */
    /** Need to loop thru and verify we have same regType among       */
    /** other things prior to using the address.                      */
    for ( i = 0; i < cnt; i++ )
    {
#ifdef DEBUG_PRD_CHKSTOP_ANALYSIS
        TRAC_INFO(" addRegsToPnor In/Array: chipType:%X::%X trgType:%X::%X"
                  " regType:%X::%X ecLevel %X::%X",
                  i_chipStruct->chipType, l_ecAddrtPtr->chipType,
                  t, l_ecAddrtPtr->trgtType,
                  REG_GLBL, l_ecAddrtPtr->regType,
                  i_chipStruct->chipEcLevel, l_ecAddrtPtr->ddLevel );
#endif

        /** Need same chipType (nimbus,cumulus,etc..), same target type, */
        /** same register type and EC level must match too               */
        if ( (l_ecAddrtPtr->chipType == i_chipStruct->chipType)     &&
             (l_ecAddrtPtr->trgtType == t )                         &&
             (l_ecAddrtPtr->regType  == REG_REG)                    &&
             (l_ecAddrtPtr->ddLevel  == i_chipStruct->chipEcLevel)
           )
        {   /* address is right justified in 64 bits */
            addr = (uint32_t)(l_ecAddrtPtr->address);

            full = FirData_addRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr,
                                         &tmp );
            if ( full ) break;
        } /* end if we found an EC dep reg */

        /* prep pointer for next element (if any) */
        l_ecAddrtPtr++;

    } /** end EC dependent reg loop */

    return full;
}

/*------------------------------------------------------------------------------ */

/** @brief  Iterates a list of IDFIRs and adds them to the PNOR.
 *  @param  io_fd    The FIR data stuct.
 *  @param  io_pTrgt Pointer to PNOR target.
 *  @param  i_sTrgt  SCOM target.
 *  @param  i_chipStruct  Provides chipType and ecLevel we need
 *                        for EC dependent regs
 *
 *  @return True if the PNOR buffer is full, false if there was room.
 */
bool FirData_addIdFirsToPnor( FirData_t * io_fd, PNOR_Trgt_t * io_pTrgt,
                              SCOM_Trgt_t i_sTrgt,
                              HOMER_Chip_t *i_chipStruct )
{
    bool full = false;

    uint8_t t   = i_sTrgt.type;
    uint8_t cnt = io_fd->hData->regCounts[t][REG_IDFIR];

    uint32_t i = 0;

    uint64_t addr    = 0;
    bool     nonZero = false;
    bool     tmp     = false; /* ignored, not used */

    for ( i = 0; i < cnt; i++ )
    {
        addr    = io_fd->hPtrs[t].idFir[i];
        nonZero = false;

        /* Add FIR */
        full = FirData_addIdRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr,
                                       &nonZero );
        if ( full ) break;

        /* Add MASK */
        full = FirData_addIdRegToPnor( io_fd, io_pTrgt, i_sTrgt,
                                       addr + 0x300000000ll, &tmp );
        if ( full ) break;

        if ( nonZero )
        {
            /* Add ACT0 */
            full = FirData_addIdRegToPnor( io_fd, io_pTrgt, i_sTrgt,
                                           addr + 0x600000000ll, &tmp );
            if ( full ) break;

            /* Add ACT1 */
            full = FirData_addIdRegToPnor( io_fd, io_pTrgt, i_sTrgt,
                                           addr + 0x700000000ll, &tmp );
            if ( full ) break;
        }
    }


    /** Handle EC dependent registers, if any */
    HOMER_ChipSpecAddr_t  *l_ecAddrtPtr =  io_fd->ecDepReg;
    cnt = io_fd->hData->ecDepCounts;

    /** We have a structure with one element for each EC dep address. */
    /** Need to loop thru and verify we have same regType among       */
    /** other things prior to using the address.                      */
    for ( i = 0; i < cnt; i++ )
    {
#ifdef DEBUG_PRD_CHKSTOP_ANALYSIS
        TRAC_INFO(" addIdFirsToPnor In/Array: chipType:%X::%X trgType:%X::%X"
                  " regType:%X::%X ecLevel %X::%X",
                  i_chipStruct->chipType, l_ecAddrtPtr->chipType,
                  t, l_ecAddrtPtr->trgtType,
                  REG_GLBL, l_ecAddrtPtr->regType,
                  i_chipStruct->chipEcLevel, l_ecAddrtPtr->ddLevel );
#endif

        /** Need same chipType (nimbus,cumulus,etc..), same target type, */
        /** same register type and EC level must match too               */
        if ( (l_ecAddrtPtr->chipType == i_chipStruct->chipType)     &&
             (l_ecAddrtPtr->trgtType == t )                         &&
             (l_ecAddrtPtr->regType  == REG_IDFIR)                  &&
             (l_ecAddrtPtr->ddLevel  == i_chipStruct->chipEcLevel)
           )
        {   /* need full 64 bit address here */
            addr = l_ecAddrtPtr->address;
            nonZero = false;

            /* Add FIR */
            full = FirData_addIdRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr,
                                           &nonZero );
            if ( full ) break;

            /* Add MASK */
            full = FirData_addIdRegToPnor( io_fd, io_pTrgt, i_sTrgt,
                                           addr + 0x300000000ll, &tmp );
            if ( full ) break;

            if ( nonZero )
            {
                /* Add ACT0 */
                full = FirData_addIdRegToPnor( io_fd, io_pTrgt, i_sTrgt,
                                               addr + 0x600000000ll, &tmp );
                if ( full ) break;

                /* Add ACT1 */
                full = FirData_addIdRegToPnor( io_fd, io_pTrgt, i_sTrgt,
                                               addr + 0x700000000ll, &tmp );
                if ( full ) break;
            }

        } /* end if we found an EC dep reg */

        /* prep pointer for next element (if any) */
        l_ecAddrtPtr++;

    } /** end EC dependent reg loop */

    return full;
}

/*------------------------------------------------------------------------------ */

/** @brief  Iterates a list of IDREGs and adds them to the PNOR.
 *  @param  io_fd    The FIR data stuct.
 *  @param  io_pTrgt Pointer to PNOR target.
 *  @param  i_sTrgt  SCOM target.
 *  @param  i_chipStruct  Provides chipType and ecLevel we need
 *                        for EC dependent regs
 *
 *  @return True if the PNOR buffer is full, false if there was room.
 */
bool FirData_addIdRegsToPnor( FirData_t * io_fd, PNOR_Trgt_t * io_pTrgt,
                              SCOM_Trgt_t i_sTrgt,
                              HOMER_Chip_t *i_chipStruct )
{
    bool full = false;

    uint8_t t   = i_sTrgt.type;
    uint8_t cnt = io_fd->hData->regCounts[t][REG_IDREG];

    uint32_t i = 0;

    uint64_t addr    = 0;
    bool     tmp     = false; /* ignored, not used */

    for ( i = 0; i < cnt; i++ )
    {
        addr = io_fd->hPtrs[t].idReg[i];

        full = FirData_addIdRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr, &tmp );
        if ( full ) break;
    }


    /** Handle EC dependent registers, if any */
    HOMER_ChipSpecAddr_t  *l_ecAddrtPtr =  io_fd->ecDepReg;
    cnt = io_fd->hData->ecDepCounts;

    /** We have a structure with one element for each EC dep address. */
    /** Need to loop thru and verify we have same regType among       */
    /** other things prior to using the address.                      */
    for ( i = 0; i < cnt; i++ )
    {
#ifdef DEBUG_PRD_CHKSTOP_ANALYSIS
        TRAC_IMP(" addIdRegsToPnor In/Array: chipType:%X::%X trgType:%X::%X"
                  " regType:%X::%X ecLevel %X::%X",
                  i_chipStruct->chipType, l_ecAddrtPtr->chipType,
                  t, l_ecAddrtPtr->trgtType,
                  REG_GLBL, l_ecAddrtPtr->regType,
                  i_chipStruct->chipEcLevel, l_ecAddrtPtr->ddLevel );
#endif

        /** Need same chipType (nimbus,cumulus,etc..), same target type, */
        /** same register type and EC level must match too               */
        if ( (l_ecAddrtPtr->chipType == i_chipStruct->chipType)     &&
             (l_ecAddrtPtr->trgtType == t )                         &&
             (l_ecAddrtPtr->regType  == REG_IDREG)                  &&
             (l_ecAddrtPtr->ddLevel  == i_chipStruct->chipEcLevel)
           )
        {   /* Using full 64 bit address here */
            addr = l_ecAddrtPtr->address;

            full = FirData_addIdRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr, &tmp );
            if ( full ) break;
        } /* end if we found an EC dep reg */

        /* prep pointer for next element (if any) */
        l_ecAddrtPtr++;

    } /** end EC dependent reg loop */


    return full;
}


/*------------------------------------------------------------------------------ */

/** @brief  Adds a target to the PNOR.
 *  @param  io_fd    The FIR data stuct.
 *  @param  i_sTrgt   SCOM Target.
 *  @param  o_noAttn True, if the global registers showed no active
 *                   attentions on the target. False, otherwise.
 *  @param  i_chipStruct  Provides chipType and ecLevel we need
 *                        for EC dependent regs
 *
 *  @return True if the PNOR buffer is full, false if there was room.
 */
bool FirData_addTrgtToPnor( FirData_t * io_fd, SCOM_Trgt_t i_sTrgt,
                            bool * o_noAttn, HOMER_Chip_t *i_chipStruct )
{
    bool full = false;

    PNOR_Trgt_t * pTrgt = (PNOR_Trgt_t *)(&io_fd->pBuf[io_fd->pBufSize]);

    PNOR_Trgt_t tmp_pTrgt = PNOR_getTrgt( i_sTrgt.type, i_sTrgt.procPos,
                                          i_sTrgt.procUnitPos );

    *o_noAttn = false; /* Must be false if there are no global regs. */

    TRAC_IMP( "FIRDATA: t=%d p=%d u=%d FSI=0x%08x", i_sTrgt.type,
              i_sTrgt.procPos, i_sTrgt.procUnitPos, (uint32_t)i_sTrgt.fsiBaseAddr);

    do
    {
        /* Add the target info to PNOR. */
        full = FirData_addDataToPnor( io_fd, &tmp_pTrgt, sizeof(tmp_pTrgt) );
        if ( full ) break;

        /* Update the number of targets in the PNOR data. */
        io_fd->pData->trgts++;

        /* NOTE: Must add all regular registers (REG_GLBL, REG_FIR, REG_REG)
         *       before all indirect-SCOM registers. Also, must check REG_GLBL
         *       registers first to determine whether it is necessary to do the
         *       other registers. */

        /* Add the GLBLs. */
        full = FirData_addGlblsToPnor( io_fd, pTrgt, i_sTrgt, o_noAttn, i_chipStruct );
        if ( full || *o_noAttn ) break;

        /* Add the FIRs. */
        full = FirData_addFirsToPnor( io_fd, pTrgt, i_sTrgt, i_chipStruct );
        if ( full ) break;

        /* Add the REGs. */
        full = FirData_addRegsToPnor( io_fd, pTrgt, i_sTrgt, i_chipStruct );
        if ( full ) break;

        /* Add the IDFIRs. */
        full = FirData_addIdFirsToPnor( io_fd, pTrgt, i_sTrgt, i_chipStruct );
        if ( full ) break;

        /* Add the IDREGs. */
        full = FirData_addIdRegsToPnor( io_fd, pTrgt, i_sTrgt, i_chipStruct );
        if ( full ) break;

    } while (0);

    return full;
}


/*------------------------------------------------------------------------------ */

/** @brief Iterates through configured targets and adds the data to PNOR.
 *  @param io_fd The FIR data stuct.
 */
void FirData_addTrgtsToPnor( FirData_t * io_fd )
{
    bool full   = false;
    bool noAttn = false;

    uint8_t p  = 0;
    uint8_t u  = 0;
    uint8_t l_unit = 0;

    bool     isM = false;
    uint32_t fsi = 0;

    SCOM_Trgt_t sTrgt;
    FirData_existBits_t  l_existBits;

    do
    {
        memset(&l_existBits, 0x00, sizeof(FirData_existBits_t) );

        /* Point past HOMER header to first chiplet info */
        /* The HOMER_Data_t struct may have some padding added after the struct
         * to ensure the HOMER_Chip_t structs are 4-byte word aligned. */
        uint32_t sz_word = sizeof(uint32_t);
        uint32_t pad = (sz_word - (sizeof(HOMER_Data_t) % sz_word)) % sz_word;
        uint8_t       *l_bytePtr = io_fd->hBuf + sizeof(HOMER_Data_t) + pad;
        HOMER_Chip_t  *l_chipPtr = NULL;

        TRAC_IMP("AddTgtsMain:numChips:%d", (uint32_t)io_fd->hData->chipCount);

        /* Iterate ALL CHIPs  */
        for ( p = 0; p < io_fd->hData->chipCount; p++ )
        {
            l_chipPtr = (HOMER_Chip_t *)l_bytePtr;

            /* get FSI base address and chip position (proc or centaur) */
            fsi = l_chipPtr->fsiBaseAddr;
            p   = l_chipPtr->chipPos;

            TRAC_IMP( " AddTgtsMain:ChipType:%d  ChipNumber:%d",
                       l_chipPtr->chipType, p );

            /* Is this a PROC or Centaur chip ? */
            if ( (HOMER_CHIP_NIMBUS  == l_chipPtr->chipType) ||
                 (HOMER_CHIP_CUMULUS == l_chipPtr->chipType)   )
            {
                /* To access the 'chiplet exist' structure */
                l_bytePtr += sizeof(HOMER_Chip_t);

                /* 'Existing chiplet area' varies in size  */
                if (HOMER_CHIP_NIMBUS == l_chipPtr->chipType)
                {
                    HOMER_ChipNimbus_t  *l_nimExistBitPtr =
                                        (HOMER_ChipNimbus_t *)l_bytePtr;

                    /* get master proc indicator */
                    isM = l_nimExistBitPtr->isMaster;

                    /* Exist bit structure can differ so move to      */
                    /* common format and left justify for ease of use */
                    l_existBits.xbusMask = ((uint32_t)(l_nimExistBitPtr->xbusMask))
                                            << (32 - MAX_XBUS_PER_PROC);
                    l_existBits.obusMask = ((uint32_t)(l_nimExistBitPtr->obusMask))
                                            << (32 - MAX_OBUS_PER_PROC);
                    l_existBits.ecMask   = ((uint32_t)(l_nimExistBitPtr->ecMask))
                                            << (32 - MAX_EC_PER_PROC);
                    l_existBits.eqMask   = ((uint32_t)(l_nimExistBitPtr->eqMask))
                                            << (32 - MAX_EQ_PER_PROC);
                    l_existBits.exMask   = ((uint32_t)(l_nimExistBitPtr->exMask))
                                            << (32 - MAX_EX_PER_PROC);
                    l_existBits.mcbist_mc_Mask = ((uint32_t)(l_nimExistBitPtr->mcbistMask))
                                                  << (32 - MAX_MCBIST_PER_PROC);
                    l_existBits.mcs_mi_Mask    = ((uint32_t)(l_nimExistBitPtr->mcsMask))
                                                  << (32 - MAX_MCS_PER_PROC);
                    l_existBits.mca_dmi_Mask   = ((uint32_t)(l_nimExistBitPtr->mcaMask))
                                                  << (32 - MAX_MCA_PER_PROC);
                    l_existBits.cappMask = ((uint32_t)(l_nimExistBitPtr->cappMask))
                                            << (32 - MAX_CAPP_PER_PROC);
                    l_existBits.pecMask  = ((uint32_t)(l_nimExistBitPtr->pecMask))
                                            << (32 - MAX_PEC_PER_PROC);
                    l_existBits.phbMask  = ((uint32_t)(l_nimExistBitPtr->phbMask))
                                            << (32 - MAX_PHB_PER_PROC);

                    /* advance our pointer to next chip type */
                    l_bytePtr += sizeof(HOMER_ChipNimbus_t);

                } /* if nimbus */
                else if (HOMER_CHIP_CUMULUS == l_chipPtr->chipType)
                {
                    HOMER_ChipCumulus_t  *l_cumExistBitPtr =
                                        (HOMER_ChipCumulus_t *)l_bytePtr;

                    /* get master proc indicator */
                    isM = l_cumExistBitPtr->isMaster;

                    /* Exist bit structure can differ so move to      */
                    /* common format and left justify for ease of use */
                    l_existBits.xbusMask = ((uint32_t)(l_cumExistBitPtr->xbusMask))
                                            << (32 - MAX_XBUS_PER_PROC);
                    l_existBits.obusMask = ((uint32_t)(l_cumExistBitPtr->obusMask))
                                            << (32 - MAX_OBUS_PER_PROC);
                    l_existBits.ecMask   = ((uint32_t)(l_cumExistBitPtr->ecMask))
                                            << (32 - MAX_EC_PER_PROC);
                    l_existBits.eqMask   = ((uint32_t)(l_cumExistBitPtr->eqMask))
                                            << (32 - MAX_EQ_PER_PROC);
                    l_existBits.exMask   = ((uint32_t)(l_cumExistBitPtr->exMask))
                                            << (32 - MAX_EX_PER_PROC);
                    l_existBits.mcbist_mc_Mask = ((uint32_t)(l_cumExistBitPtr->mcMask))
                                                  << (32 - MAX_MC_PER_PROC);
                    l_existBits.mcs_mi_Mask    = ((uint32_t)(l_cumExistBitPtr->miMask))
                                                  << (32 - MAX_MI_PER_PROC);
                    l_existBits.mca_dmi_Mask   = ((uint32_t)(l_cumExistBitPtr->dmiMask))
                                                  << (32 - MAX_DMI_PER_PROC);
                    l_existBits.cappMask = ((uint32_t)(l_cumExistBitPtr->cappMask))
                                            << (32 - MAX_CAPP_PER_PROC);
                    l_existBits.pecMask  = ((uint32_t)(l_cumExistBitPtr->pecMask))
                                            << (32 - MAX_PEC_PER_PROC);
                    l_existBits.phbMask  = ((uint32_t)(l_cumExistBitPtr->phbMask))
                                            << (32 - MAX_PHB_PER_PROC);

                    /* advance our pointer to next chip type */
                    l_bytePtr += sizeof(HOMER_ChipCumulus_t);
                } /* else if cumulus */

//                TRAC_IMP("Masks XBUS:%X OBUS:%X EC:%X EQ:%X EX:%X CAPP:%X PEC:%X PHB:%X",
//                           (uint32_t)l_existBits.xbusMask, (uint32_t)l_existBits.obusMask, (uint32_t)l_existBits.ecMask,
//                           (uint32_t)l_existBits.eqMask,   (uint32_t)l_existBits.exMask,   (uint32_t)l_existBits.cappMask,
//                           (uint32_t)l_existBits.pecMask,  (uint32_t)l_existBits.phbMask);
//                TRAC_IMP("Masks MCBIST:%X MCS:%X MCA:%X",
//                           (uint32_t)l_existBits.mcbist_mc_Mask, (uint32_t) l_existBits.mcs_mi_Mask,
//                           (uint32_t)l_existBits.mca_dmi_Mask);

                /* Add this PROC to the PNOR. */
                sTrgt = SCOM_Trgt_getTrgt(TRGT_PROC, p, 0, fsi, isM);
                full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn, l_chipPtr );
                /* noAttn is true when we have global regs but none */
                /* indicate an attention is present                 */
                if ( full ) break;
                if ( noAttn ) continue; /* Skip other proc chiplets */


                /* gather other chiplets on the processor */
                for ( u = 0; u < MAX_XBUS_PER_PROC; u++ )
                {
                    /* Check if the XBUS  is configured. */
                    if ( 0 == (l_existBits.xbusMask  & (0x80000000 >> u)) ) continue;

                    /* Add this XBUS to the PNOR. */
                    sTrgt = SCOM_Trgt_getTrgt(TRGT_XBUS, p, u, fsi, isM);
                    full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn, l_chipPtr );
                    if ( full ) break;
                }
                if ( full ) break;

                /* gather other chiplets on the processor */
                for ( u = 0; u < MAX_OBUS_PER_PROC; u++ )
                {
                    /* Check if the OBUS  is configured. */
                    if ( 0 == (l_existBits.obusMask  & (0x80000000 >> u)) ) continue;

                    /* Add this OBUS to the PNOR. */
                    sTrgt = SCOM_Trgt_getTrgt(TRGT_OBUS, p, u, fsi, isM);
                    full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn, l_chipPtr );
                    if ( full ) break;
                }
                if ( full ) break;

                /* gather more proc chiplets  */
                for ( u = 0; u < MAX_CAPP_PER_PROC; u++ )
                {
                    /* Check if the CAPP is configured. */
                    if ( 0 == (l_existBits.cappMask  & (0x80000000 >> u)) ) continue;

                    /* Add this CAPP to the PNOR. */
                    sTrgt = SCOM_Trgt_getTrgt(TRGT_CAPP, p, u, fsi, isM);
                    full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn, l_chipPtr );
                    if ( full ) break;
                }
                if ( full ) break;

                /* gather other chiplets on the processor */
                for ( u = 0; u < MAX_PEC_PER_PROC; u++ )
                {
                    /* Check if the PEC is configured. */
                    if ( 0 == (l_existBits.pecMask  & (0x80000000 >> u)) ) continue;

                    /* Add this PEC to the PNOR. */
                    sTrgt = SCOM_Trgt_getTrgt(TRGT_PEC, p, u, fsi, isM);
                    full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn, l_chipPtr );
                    if ( full ) break;
                    if ( noAttn ) continue; /* Skip the rest */

                    /* gather PHB's under the PEC  */
                    /*  ************************** */
                    /*  PEC0-> PHB0                */
                    /*  PEC1-> PHB1 PHB2           */
                    /*  PEC2-> PHB3 PHB4 PHB5      */
                    /*  ************************** */
                    uint32_t l_PhbPos;

                    /* u will be 0, l_unit 0             */
                    /* u will be 1, l_unit 0 and 1       */
                    /* u will be 2, l_unit 0 and 1 and 2 */
                    for ( l_unit = 0; l_unit < (u+1); l_unit++ )
                    {
                        l_PhbPos = u + l_unit;
                        /** When we hit PEC2, need to bump PHB position */
                        if  ((MAX_PEC_PER_PROC - 1) == u)
                        {
                            l_PhbPos++;
                        } /* if last PEC unit */

                        /* Check if the PHB is configured. */
                        if ( 0 == (l_existBits.phbMask  & (0x80000000 >> l_PhbPos)) ) continue;

                        /* Add this PHB to the PNOR. */
                        sTrgt = SCOM_Trgt_getTrgt(TRGT_PHB, p, l_PhbPos, fsi, isM);
                        full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn, l_chipPtr );
                        if ( full ) break;
                        if ( noAttn ) continue; /* Skip the rest */

                    } /* end for on PHB chiplet */
                    if ( full ) break;

                } /* end for on PEC chiplet */
                if ( full ) break;

                /* gather other chiplets on the processor */
                for ( u = 0; u < MAX_EC_PER_PROC; u++ )
                {
                    /* Check if the EC  is configured. */
                    if ( 0 == (l_existBits.ecMask  & (0x80000000 >> u)) ) continue;

                    /* Add this EC to the PNOR. */
                    sTrgt = SCOM_Trgt_getTrgt(TRGT_EC, p, u, fsi, isM);
                    full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn, l_chipPtr );
                    if ( full ) break;
                    if ( noAttn ) continue; /* Skip the rest */

                } /* end for on EC chiplet */
                if ( full ) break;

                /* gather other chiplets on the processor */
                for ( u = 0; u < MAX_EQ_PER_PROC; u++ )
                {
                    /* Check if the EQ is configured. */
                    if ( 0 == (l_existBits.eqMask  & (0x80000000 >> u)) ) continue;

                    /* Add this EQ to the PNOR. */
                    sTrgt = SCOM_Trgt_getTrgt(TRGT_EQ, p, u, fsi, isM);
                    full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn, l_chipPtr );
                    if ( full ) break;
                    if ( noAttn ) continue; /* Skip the rest */

                    /* gather other chiplets on the processor */
                    uint32_t l_ExPerEq = (MAX_EX_PER_PROC/MAX_EQ_PER_PROC);
                    uint32_t l_ExPos;

                    for (l_unit=0; l_unit < l_ExPerEq; l_unit++)
                    {
                        l_ExPos = (l_ExPerEq * u) + l_unit;

                        /* Check if the EX is configured. */
                        if ( 0 == (l_existBits.exMask  & (0x80000000 >> l_ExPos)) ) continue;

                        /* Add this EX to the PNOR. */
                        sTrgt = SCOM_Trgt_getTrgt(TRGT_EX, p, l_ExPos, fsi, isM);
                        full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn, l_chipPtr );
                        if ( full ) break;
                        if ( noAttn ) continue; /* Skip the rest */

                    } /* end for on EX chiplet */
                    if ( full ) break;

                } /* end for on EQ chiplet */
                if ( full ) break;


                /* processor type can impact next few units */
                TrgtType_t  l_pnorTarget = (HOMER_CHIP_NIMBUS == l_chipPtr->chipType) ?
                                            TRGT_MCBIST : TRGT_MC;
                uint32_t    l_maxPerProc = (HOMER_CHIP_NIMBUS == l_chipPtr->chipType) ?
                                            MAX_MCBIST_PER_PROC : MAX_MC_PER_PROC;

                TrgtType_t  l_pnorTarg2 = (HOMER_CHIP_NIMBUS == l_chipPtr->chipType) ?
                      TRGT_MCA : TRGT_DMI;
                uint32_t    l_maxPerProc2 = (HOMER_CHIP_NIMBUS == l_chipPtr->chipType) ?
                      MAX_MCA_PER_PROC : MAX_DMI_PER_PROC;

                uint32_t l_UnitPerMc = l_maxPerProc2 / l_maxPerProc;
                uint8_t  l_unitNumber;

                for ( u = 0; u < l_maxPerProc; u++ )
                {
                    /* Check if MCBIST / MC is configured. */
                    if ( 0 == (l_existBits.mcbist_mc_Mask  & (0x80000000 >> u)) ) continue;

                    /* Add this MCBIST or MC to the PNOR. */
                    sTrgt = SCOM_Trgt_getTrgt(l_pnorTarget, p, u, fsi, isM);
                    full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn, l_chipPtr );
                    if ( full ) break;
                    if ( noAttn ) continue; /* Skip the rest */

                    /* Grab underlying MCA/DMI chiplet */
                    for ( l_unit = 0; l_unit < l_UnitPerMc; l_unit++ )
                    {
                        /* u=0 or 1 while  l_unit is 0 thru 3 */
                        /* Leading to unit number 0:3 or 4:7  */
                        l_unitNumber = l_unit + (u * l_UnitPerMc);
                        /* Check if the MCA / DMI is configured. */
                        if ( 0 == (l_existBits.mca_dmi_Mask  & (0x80000000 >> l_unitNumber)) ) continue;

                        /* Add this MCA / DMI to the PNOR. */
                        sTrgt = SCOM_Trgt_getTrgt(l_pnorTarg2, p, l_unitNumber, fsi, isM);
                        full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn, l_chipPtr );
                        if ( full ) break;

                    } /* end for on MCA/DMI */
                    if ( full ) break;

                } /* end for on MCBIST */
                if ( full ) break;


                l_pnorTarget = (HOMER_CHIP_NIMBUS == l_chipPtr->chipType) ?
                                           TRGT_MCS : TRGT_MI;
                l_maxPerProc = (HOMER_CHIP_NIMBUS == l_chipPtr->chipType) ?
                                           MAX_MCS_PER_PROC : MAX_MI_PER_PROC;

                for ( u = 0; u < l_maxPerProc; u++ )
                {
                    /* Check if the MCS / MI is configured. */
                    if ( 0 == (l_existBits.mcs_mi_Mask  & (0x80000000 >> u)) ) continue;

                    /* Add this MCS or MI to the PNOR. */
                    sTrgt = SCOM_Trgt_getTrgt(l_pnorTarget, p, u, fsi, isM);
                    full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn, l_chipPtr );
                    if ( full ) break;
                }
                if ( full ) break;


            } /* if processor chip type */
            else if  (HOMER_CHIP_CENTAUR == l_chipPtr->chipType)
            {
                /* HOMER_ChipCentaur_t  *l_cenExistBitPtr =
                                       (HOMER_ChipCentaur_t *)l_bytePtr; */
                /* uint32_t l_mbaMask = ((uint32_t) l_cenExistBitPtr->mbaMask) <<
                                         (sizeof(uint32_t) - MAX_MBA_PER_MEMBUF) */

                /* we have Centaur chipPos in 'p'  and fsi address in 'fsi'  */

                /*  add this centaur later and iterate thru l_mbaMask */
                /*  don't need for initial support */
                /* TODO RTC 173614  -- with CUMULUS   */

                /* advance our pointer to next chip type */
                l_bytePtr += sizeof(HOMER_ChipCentaur_t);

            } /* end if centaur chip type */
            else
            {
                /* unexpected chip type */
                TRAC_ERR( "addTrgtsToPnor saw invalid chip:0x%X",
                          (uint32_t)l_chipPtr->chipType );
                break;
            } /* end if centaur chip type */

        } /* end for on all CHIPS we have in homer data */

    } while (0);

} /* end FirData_addTrgtsToPnor */

/*------------------------------------------------------------------------------ */

/** @brief  Initializes the FIR data struct. Does range checking for the HOMER
 *          data. Initializes the PNOR header data.
 *  @param  io_fd      The FIR data stuct.
 *  @param  i_hBuf     SRAM pointer to the beginning of the HOMER data buffer.
 *                     This should contain the FIR data information provided by
 *                     PRD that is used to define which registers the OCC will
 *                     need to SCOM.
 *  @param  i_hBufSize Total size of the HOMER data buffer.
 *  @param  i_pBuf     SRAM pointer to the beginning of the PNOR data buffer.
 *                     This will be used by this function as a temporary area of
 *                     memory to store the PNOR data before writing that data to
 *                     the PNOR.
 *  @param  i_pBufSize Total size of the PNOR data buffer.
 *  @return Non-SUCCESS if HOMER or PNOR range checking fails or if an
 *          internal function fails. SUCCESS otherwise.
 */
int32_t FirData_init( FirData_t * io_fd,
                      uint8_t * i_hBuf, uint32_t i_hBufSize,
                      uint8_t * i_pBuf, uint32_t i_pBufSize )
{
    #define FUNC "[FirData_init] "

    int32_t rc = SUCCESS;

    uint32_t sz_hData    = sizeof(HOMER_Data_t);
    uint32_t sz_pnoNoEcc = 0;
    uint32_t sz_u32      = sizeof(uint32_t);
    uint32_t sz_u64      = sizeof(uint64_t);

    bool full = false;

    uint32_t x[TRGT_MAX][REG_MAX];
    uint32_t curIdx = 0;

    uint32_t t = TRGT_FIRST;

    uint8_t * reglist = NULL;

    PNOR_Data_t pData = PNOR_getData();

    do
    {
        /* Init the struct. */
        io_fd->hBuf        = i_hBuf;
        io_fd->maxHBufSize = i_hBufSize;
        io_fd->hData       = (HOMER_Data_t *)i_hBuf;

        io_fd->pBuf        = i_pBuf;
        io_fd->maxPBufSize = i_pBufSize;
        io_fd->pData       = (PNOR_Data_t *)i_pBuf;
        io_fd->pBufSize    = 0;

        memset( io_fd->hPtrs, 0x00, sizeof(io_fd->hPtrs) );

        /* Check HOMER header data size. */
        if ( io_fd->maxHBufSize < sz_hData )
        {
            TRAC_ERR( FUNC"HOMER header data size %d is larger than HOMER "
                      "data buffer %d", sz_hData, io_fd->maxHBufSize );
            rc = FAIL;
            break;
        }

        /* Check for valid HOMER data. */
        if ( HOMER_FIR2 != io_fd->hData->header )
        {
            break; /* nothing to analyze. */
        }

        /* The actual maximum PNOR size may possibly be less then the PNOR data */
        /* buffer. If so, adjust maximum size. */
        sz_pnoNoEcc = (io_fd->hData->pnorInfo.pnorSize / 9) * 8;
        if ( sz_pnoNoEcc < io_fd->maxPBufSize )
            io_fd->maxPBufSize = sz_pnoNoEcc;

        /* Initialize the PNOR header data. */
        full = FirData_addDataToPnor( io_fd, &pData, sizeof(pData) );
        if ( full )
        {
            TRAC_ERR( FUNC"Unable to add header to PNOR buffer" );
            rc = FAIL;
            break;
        }

        /* Copy the IPL state from the HOMER data to the PNOR data. */
        io_fd->pData->iplState = io_fd->hData->iplState;

        /* Get the register list byte indexes in HOMER data buffer */
        memset( x, 0x00, sizeof(x) );
        for ( t = TRGT_FIRST; t < TRGT_MAX; t++ )
        {
            x[t][REG_GLBL]  = curIdx;
            x[t][REG_FIR]   = x[t][REG_GLBL]  + sz_u32 * io_fd->hData->regCounts[t][REG_GLBL];
            x[t][REG_REG]   = x[t][REG_FIR]   + sz_u32 * io_fd->hData->regCounts[t][REG_FIR];
            x[t][REG_IDFIR] = x[t][REG_REG]   + sz_u32 * io_fd->hData->regCounts[t][REG_REG];
            x[t][REG_IDREG] = x[t][REG_IDFIR] + sz_u64 * io_fd->hData->regCounts[t][REG_IDFIR];
            curIdx          = x[t][REG_IDREG] + sz_u64 * io_fd->hData->regCounts[t][REG_IDREG];
        }

        /* Check to make sure the list data is not larger than the available */
        /* Homer buffer. */
        if ( io_fd->maxHBufSize - sz_hData < curIdx )
        {
            TRAC_ERR( FUNC"HOMER list size %d is larger than HOMER data "
                      "buffer %d", (uint32_t)curIdx, (uint32_t)(io_fd->maxHBufSize - sz_hData ));
            rc = FAIL;
            break;
        }

        /* Now, skip chip sections. Note that the HOMER_Data_t struct may have
         * some padding added after the struct to ensure the HOMER_Chip_t
         * structs are 4-byte word aligned. */
        uint32_t pad = (sz_u32 - (sz_hData % sz_u32)) % sz_u32;
        reglist = io_fd->hBuf + sz_hData + pad;
        HOMER_Chip_t   *l_chiptPtr = NULL;

        /* Need to skip over chip list **/
        uint32_t  l_chipNum;
        for ( l_chipNum=0;
             (l_chipNum <  io_fd->hData->chipCount);
              l_chipNum++ )
        {
            l_chiptPtr = (HOMER_Chip_t *)reglist;

            /* Skip section on chip type, chip position, etc... */
            reglist += sizeof(HOMER_Chip_t);

            /* 'Existing chiplet area' varies in size  */
            if (HOMER_CHIP_NIMBUS == l_chiptPtr->chipType)
            {
                reglist += sizeof(HOMER_ChipNimbus_t);
            }
            else if (HOMER_CHIP_CUMULUS == l_chiptPtr->chipType)
            {
                reglist += sizeof(HOMER_ChipCumulus_t);
            }
            else if (HOMER_CHIP_CENTAUR == l_chiptPtr->chipType)
            {
                reglist += sizeof(HOMER_ChipCentaur_t);
            }
            else
            {
                TRAC_ERR(FUNC"Chiptype is invalid %X ", (uint32_t)l_chiptPtr->chipType);
                rc = FAIL;
                break;
            }

        } /* end for loop skipping chip info sections */


        /* Now, get the pointers for each list. */
        for ( t = TRGT_FIRST; t < TRGT_MAX; t++ )
        {
            (io_fd->hPtrs[t]).glbl  = (uint32_t *)(reglist + x[t][REG_GLBL] );
            (io_fd->hPtrs[t]).fir   = (uint32_t *)(reglist + x[t][REG_FIR]  );
            (io_fd->hPtrs[t]).reg   = (uint32_t *)(reglist + x[t][REG_REG]  );
            (io_fd->hPtrs[t]).idFir = (uint64_t *)(reglist + x[t][REG_IDFIR]);
            (io_fd->hPtrs[t]).idReg = (uint64_t *)(reglist + x[t][REG_IDREG]);
        }

        /* Set EC level dep reg list ptr - at very end of list */
        io_fd->ecDepReg = (HOMER_ChipSpecAddr_t *)(reglist + curIdx);


    } while (0);

    return rc;

    #undef FUNC
}

/*------------------------------------------------------------------------------ */
/* External functions */
/*------------------------------------------------------------------------------ */

int32_t FirData_captureCsFirData( uint8_t * i_hBuf, uint32_t i_hBufSize,
                                  uint8_t * i_pBuf, uint32_t i_pBufSize )
{
    #define FUNC "[FirData_captureCsFirData] "

    int32_t rc = SUCCESS;

    do
    {
        /* Init the FIR data struct. */
        FirData_t fd;
        rc = FirData_init( &fd, i_hBuf, i_hBufSize, i_pBuf, i_pBufSize );
        if ( SUCCESS != rc )
        {
            TRAC_ERR( FUNC"Failed to init FIR data" );
            break;
        }

        /* Check for valid HOMER data. */
        if ( HOMER_FIR2 != fd.hData->header )
        {
            TRAC_ERR( FUNC"No HOMER data detected: header=0x%08x",
                      fd.hData->header );
            break; /* nothing to analyze. */
        }

        /* Start adding register data to PNOR for each target. */
        FirData_addTrgtsToPnor( &fd );

        /* Write Buffer to PNOR */
/* TODO: enable when function is supported.
        rc = PNOR_writeFirData( fd.hData->pnorInfo, fd.pBuf, fd.pBufSize );
        if ( SUCCESS != rc )
        {
            TRAC_ERR( FUNC"Failed to process FIR data" );
            break;
        }
*/

    } while (0);

    if ( SUCCESS != rc )
    {
        TRAC_ERR( FUNC"Failed: i_hBuf=%08x, i_hBufSize=0x%08x, i_pBuf=%08x, "
                  "i_pBufSize=%08x", (uint32_t)i_hBuf, (uint32_t)i_hBufSize,
                  (uint32_t)i_pBuf, (uint32_t)i_pBufSize );
    }

    return rc;

    #undef FUNC
}

