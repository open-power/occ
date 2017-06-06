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
    uint32_t * glbl;    /*/< Global registers */
    uint32_t * fir;     /*/< Normal FIRs */
    uint32_t * reg;     /*/< Normal registers */
    uint64_t * idFir;   /*/< Indirect-SCOM FIRs */
    uint64_t * idReg;   /*/< Indirect-SCOM registers */

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

    FirData_ListPointers_t hPtrs[TRGT_MAX]; /*/< Pointers to the register lists */

} FirData_t;

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
            TRAC_ERR( "[FirData_addRegToPnor] t=%d p=%d u=%d rc=%d "
                      "addr=0x%08x val=0x%08x%08x", i_sTrgt.type,
                      i_sTrgt.procPos, i_sTrgt.procUnitPos, rc, i_addr,
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
 *  @return True if the PNOR buffer is full, false if there was room.
 */
bool FirData_addGlblsToPnor( FirData_t * io_fd, PNOR_Trgt_t * io_pTrgt,
                             SCOM_Trgt_t i_sTrgt, bool * o_noAttn )
{
    bool full = false;

    uint8_t t   = i_sTrgt.type;
    uint8_t cnt = io_fd->hData->counts[t][REG_GLBL];

    uint32_t i = 0;

    uint32_t addr    = 0;
    bool     nonZero = false;

    *o_noAttn = false; /* Must be false if there are no global regs. */

    if ( 0 != cnt )
    {
        *o_noAttn = true; /* Assume no attentions. */
        for ( i = 0; i < cnt; i++ )
        {
            addr    = io_fd->hPtrs[t].glbl[i];
            nonZero = false;

            full = FirData_addRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr,
                                         &nonZero );
            if ( full ) break;

            if ( nonZero ) *o_noAttn = false;
        }
    }

    return full;
}

/*------------------------------------------------------------------------------ */

/** @brief  Iterates a list of FIRs and adds them to the PNOR.
 *  @param  io_fd    The FIR data stuct.
 *  @param  io_pTrgt Pointer to PNOR target.
 *  @param  i_sTrgt  SCOM target.
 *  @return True if the PNOR buffer is full, false if there was room.
 */
bool FirData_addFirsToPnor( FirData_t * io_fd, PNOR_Trgt_t * io_pTrgt,
                            SCOM_Trgt_t i_sTrgt )
{
    bool full = false;

    uint8_t t   = i_sTrgt.type;
    uint8_t cnt = io_fd->hData->counts[t][REG_FIR];

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

    return full;
}

/*------------------------------------------------------------------------------ */

/** @brief  Iterates a list of REGs and adds them to the PNOR.
 *  @param  io_fd    The FIR data stuct.
 *  @param  io_pTrgt Pointer to PNOR target.
 *  @param  i_sTrgt  SCOM target.
 *  @return True if the PNOR buffer is full, false if there was room.
 */
bool FirData_addRegsToPnor( FirData_t * io_fd, PNOR_Trgt_t * io_pTrgt,
                            SCOM_Trgt_t i_sTrgt )
{
    bool full = false;

    uint8_t t   = i_sTrgt.type;
    uint8_t cnt = io_fd->hData->counts[t][REG_REG];

    uint32_t i = 0;

    uint32_t addr    = 0;
    bool     tmp     = false; /* ignored, not used */

    for ( i = 0; i < cnt; i++ )
    {
        addr = io_fd->hPtrs[t].reg[i];

        full = FirData_addRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr, &tmp );
        if ( full ) break;
    }

    return full;
}

/*------------------------------------------------------------------------------ */

/** @brief  Iterates a list of IDFIRs and adds them to the PNOR.
 *  @param  io_fd    The FIR data stuct.
 *  @param  io_pTrgt Pointer to PNOR target.
 *  @param  i_sTrgt  SCOM target.
 *  @return True if the PNOR buffer is full, false if there was room.
 */
bool FirData_addIdFirsToPnor( FirData_t * io_fd, PNOR_Trgt_t * io_pTrgt,
                              SCOM_Trgt_t i_sTrgt )
{
    bool full = false;

    uint8_t t   = i_sTrgt.type;
    uint8_t cnt = io_fd->hData->counts[t][REG_IDFIR];

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

    return full;
}

/*------------------------------------------------------------------------------ */

/** @brief  Iterates a list of IDREGs and adds them to the PNOR.
 *  @param  io_fd    The FIR data stuct.
 *  @param  io_pTrgt Pointer to PNOR target.
 *  @param  i_sTrgt  SCOM target.
 *  @return True if the PNOR buffer is full, false if there was room.
 */
bool FirData_addIdRegsToPnor( FirData_t * io_fd, PNOR_Trgt_t * io_pTrgt,
                              SCOM_Trgt_t i_sTrgt )
{
    bool full = false;

    uint8_t t   = i_sTrgt.type;
    uint8_t cnt = io_fd->hData->counts[t][REG_IDREG];

    uint32_t i = 0;

    uint64_t addr    = 0;
    bool     tmp     = false; /* ignored, not used */

    for ( i = 0; i < cnt; i++ )
    {
        addr = io_fd->hPtrs[t].idReg[i];

        full = FirData_addIdRegToPnor( io_fd, io_pTrgt, i_sTrgt, addr, &tmp );
        if ( full ) break;
    }

    return full;
}

/*------------------------------------------------------------------------------ */

/** @brief  Adds a target to the PNOR.
 *  @param  io_fd    The FIR data stuct.
 *  @param  i_sTrgt   SCOM Target.
 *  @param  o_noAttn True, if the global registers showed no active
 *                   attentions on the target. False, otherwise.
 *  @return True if the PNOR buffer is full, false if there was room.
 */
bool FirData_addTrgtToPnor( FirData_t * io_fd, SCOM_Trgt_t i_sTrgt,
                            bool * o_noAttn )
{
    bool full = false;

    PNOR_Trgt_t * pTrgt = (PNOR_Trgt_t *)(&io_fd->pBuf[io_fd->pBufSize]);

    PNOR_Trgt_t tmp_pTrgt = PNOR_getTrgt( i_sTrgt.type, i_sTrgt.procPos,
                                          i_sTrgt.procUnitPos );

    *o_noAttn = false; /* Must be false if there are no global regs. */

    TRAC_IMP( "FIRDATA: t=%d p=%d u=%d FSI=0x%08x isM=%c", i_sTrgt.type,
              i_sTrgt.procPos, i_sTrgt.procUnitPos, i_sTrgt.fsiBaseAddr,
              i_sTrgt.isMaster ? 'T' : 'F' );

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
        full = FirData_addGlblsToPnor( io_fd, pTrgt, i_sTrgt, o_noAttn );
        if ( full || *o_noAttn ) break;

        /* Add the FIRs. */
        full = FirData_addFirsToPnor( io_fd, pTrgt, i_sTrgt );
        if ( full ) break;

        /* Add the REGs. */
        full = FirData_addRegsToPnor( io_fd, pTrgt, i_sTrgt );
        if ( full ) break;

        /* Add the IDFIRs. */
        full = FirData_addIdFirsToPnor( io_fd, pTrgt, i_sTrgt );
        if ( full ) break;

        /* Add the IDREGs. */
        full = FirData_addIdRegsToPnor( io_fd, pTrgt, i_sTrgt );
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
    uint8_t mu = 0;
    uint8_t i = 0;
    uint8_t j = 0;

    bool     isM = false;
    uint32_t fsi = 0;

    SCOM_Trgt_t sTrgt;

    do
    {
        /* Iterate all PROCs. */
        for ( p = 0; p < MAX_PROC_PER_NODE; p++ )
        {
            /* Check if the PROC is configured. */
            if ( 0 == (io_fd->hData->procMask & (0x80 >> p)) ) continue;

            /* Check if this PROC is the master PROC and get the FSI base addr. */
            isM = ( p == io_fd->hData->masterProc );
            fsi = io_fd->hData->procFsiBaseAddr[p];

            /* Add this PROC to the PNOR. */
            sTrgt = SCOM_Trgt_getTrgt(TRGT_PROC, p, 0, fsi, isM);
            full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn );
            if ( full ) break;
            if ( noAttn ) continue; /* Skip the PROC, EXs, and MCSs */

            for ( u = 0; u < MAX_EX_PER_PROC; u++ )
            {
                /* Check if the EX is configured. */
                if ( 0 == (io_fd->hData->exMasks[p] & (0x8000 >> u)) ) continue;

                /* Add this EX to the PNOR. */
                sTrgt = SCOM_Trgt_getTrgt(TRGT_EX, p, u, fsi, isM);
                full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn );
                if ( full ) break;
                if ( noAttn ) continue; /* Skip the EX */
            }
            if ( full ) break;

            for ( u = 0; u < MAX_MCS_PER_PROC; u++ )
            {
                /* Check if the MCS is configured. */
                if ( 0 == (io_fd->hData->mcsMasks[p] & (0x80 >> u)) ) continue;

                /* Add this MCS to the PNOR. */
                sTrgt = SCOM_Trgt_getTrgt(TRGT_MCS, p, u, fsi, isM);
                full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn );
                if ( full ) break;
                if ( noAttn ) continue; /* Skip the MCS */
            }
            if ( full ) break;
        }
        if ( full ) break;

        /* Iterate all MEMBs. Must do this separate of from the PROCs because */
        /* it is possible a MEMBUF could be reporting an attention but the */
        /* connected PROC is not. */
        for ( i = 0; i < MAX_MEMBUF_PER_NODE; i++ )
        {
            p = i / MAX_MEMBUF_PER_PROC;
            u = i % MAX_MEMBUF_PER_PROC;

            /* Check if the MEMBUF is configured. */
            if ( 0 == (io_fd->hData->membMasks[p] & (0x80 >> u)) ) continue;

            /* Get the FSI base address. */
            fsi = io_fd->hData->membFsiBaseAddr[p][u];

            /* Add this MEMBUF to the PNOR. */
            sTrgt = SCOM_Trgt_getTrgt(TRGT_MEMBUF, p, u, fsi, false);
            full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn );
            if ( full ) break;
            if ( noAttn ) continue; /* Skip the MEMBUF and MBAs */

            for ( j = 0; j < MAX_MBA_PER_MEMBUF; j++ )
            {
                mu = u * MAX_MBA_PER_MEMBUF + j;

                /* Check if the MBA is configured. */
                if ( 0 == (io_fd->hData->mbaMasks[p] & (0x8000 >> mu)) )
                    continue;

                /* Add this MBA to the PNOR. */
                sTrgt = SCOM_Trgt_getTrgt(TRGT_MBA, p, mu, fsi, false);
                full = FirData_addTrgtToPnor( io_fd, sTrgt, &noAttn );
                if ( full ) break;
                if ( noAttn ) continue; /* Skip the MEMBUF */
            }
            if ( full ) break;
        }
        if ( full ) break;

    } while (0);

}

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

    size_t sz_hData    = sizeof(HOMER_Data_t);
    size_t sz_pnoNoEcc = 0;
    size_t sz_u32      = sizeof(uint32_t);
    size_t sz_u64      = sizeof(uint64_t);

    bool full = false;

    uint32_t x[TRGT_MAX][REG_MAX];
    size_t curIdx = 0;

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
        if ( HOMER_FIR1 != io_fd->hData->header )
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
            x[t][REG_FIR]   = x[t][REG_GLBL]  + sz_u32 * io_fd->hData->counts[t][REG_GLBL];
            x[t][REG_REG]   = x[t][REG_FIR]   + sz_u32 * io_fd->hData->counts[t][REG_FIR];
            x[t][REG_IDFIR] = x[t][REG_REG]   + sz_u32 * io_fd->hData->counts[t][REG_REG];
            x[t][REG_IDREG] = x[t][REG_IDFIR] + sz_u64 * io_fd->hData->counts[t][REG_IDFIR];
            curIdx          = x[t][REG_IDREG] + sz_u64 * io_fd->hData->counts[t][REG_IDREG];
        }

        /* Check to make sure the list data is not larger than the available */
        /* Homer buffer. */
        if ( io_fd->maxHBufSize - sz_hData < curIdx )
        {
            TRAC_ERR( FUNC"HOMER list size %d is larger than HOMER data "
                      "buffer %d", curIdx, io_fd->maxHBufSize - sz_hData );
            rc = FAIL;
            break;
        }

        /* Now, get the pointers for each list. */
        reglist = io_fd->hBuf + sz_hData;
        for ( t = TRGT_FIRST; t < TRGT_MAX; t++ )
        {
            (io_fd->hPtrs[t]).glbl  = (uint32_t *)(reglist + x[t][REG_GLBL] );
            (io_fd->hPtrs[t]).fir   = (uint32_t *)(reglist + x[t][REG_FIR]  );
            (io_fd->hPtrs[t]).reg   = (uint32_t *)(reglist + x[t][REG_REG]  );
            (io_fd->hPtrs[t]).idFir = (uint64_t *)(reglist + x[t][REG_IDFIR]);
            (io_fd->hPtrs[t]).idReg = (uint64_t *)(reglist + x[t][REG_IDREG]);
        }

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
        if ( HOMER_FIR1 != fd.hData->header )
        {
            TRAC_ERR( FUNC"No HOMER data detected: header=0x%08x",
                      fd.hData->header );
            break; /* nothing to analyze. */
        }

        /* Start adding register data to PNOR for each target. */
        FirData_addTrgtsToPnor( &fd );

        /* Write the buffer to PNOR. */
/* TODO: 175241
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
        TRAC_ERR( FUNC"Failed: i_hBuf=%p, i_hBufSize=0x%08x, i_pBuf=%p, "
                  "i_pBufSize=%08x", i_hBuf, i_hBufSize, i_pBuf, i_pBufSize );
    }

    return rc;

    #undef FUNC
}

