/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/firdata/lpc.C $                                       */
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
#include <lpc.h>
#include <trac_interface.h>

#define    LPCHC_FW_SPACE   0xF0000000 /**< LPC Host Controller FW Space */
#define    LPCHC_MEM_SPACE  0xE0000000 /**< LPC Host Controller Mem Space */
#define    LPCHC_IO_SPACE   0xD0010000 /**< LPC Host Controller I/O Space */
#define    LPCHC_REG_SPACE  0xC0012000 /**< LPC Host Ctlr Register Space */

#define    ECCB_NON_FW_RESET_REG  0x000B0001 /**< ECCB Reset Reg (non-FW) */

#define    ECCB_CTL_REG   0x000B0020 /**< ECCB Control Reg (FW) */
#define    ECCB_RESET_REG   0x000B0021 /**< ECCB Reset Reg (FW) */
#define    ECCB_STAT_REG  0x000B0022 /**< ECCB Status Reg (FW) */
#define    ECCB_DATA_REG  0x000B0023 /**< ECCB Data Reg (FW) */

/* Default Values to set for all operations
    1101.0100.0000.000x.0000.0001.0000.0000.<address> */
#define    ECCB_CTL_REG_DEFAULT  0xD400010000000000

/* Error bits: 41-43 56  (52cmd complete) (not 57: only non-fw use) */
#define    ECCB_STAT_REG_ERROR_MASK  0x0000000000700080 /**< Error Bits */

/**< OPB LPCM Sync FIR Reg - used to read the FIR*/
#define    OPB_LPCM_FIR_REG  0x01010C00

/**< OPB LPCM Sync FIR Reg WOX_AND - used to clear the FIR */
#define    OPB_LPCM_FIR_WOX_AND_REG  0x01010C01

/**< OPB LPCM Sync FIR Mask Reg WO_OR - used to set the mask */
#define    OPB_LPCM_FIR_MASK_WO_OR_REG  0x01010C05

#define    OPB_LPCM_FIR_ERROR_MASK  0xFF00000000000000 /**< Error Bits MASK */

/* LPCHC reset-related registers */
#define    OPB_MASTER_LS_CONTROL_REG  0x008 /**<OPBM LS Control Reg */
#define    LPCHC_RESET_REG            0x0FC /**<LPC HC Reset Register */

#define    ECCB_RESET_LPC_FAST_RESET  (1ULL << 62) /**< bit 1  Fast reset */

#define    ECCB_POLL_TIME_NS  400000 /**< max time should be 400ms */
//dc99 #define    ECCB_POLL_INCR_NS  10 /**< minimum increment during poll */
#define    ECCB_POLL_INCR_NS  100000 /**< increase for testing */

#define    LPCHC_SYNC_CYCLE_COUNTER_INFINITE  0xFF000000

int TRACE_LPC = 0;
#define TRACZCOMP(args...) if(TRACE_LPC){TRACFCOMP(args);}

/* Set to enable LPC tracing. */
/* #define LPC_TRACING 1 */
#ifdef LPC_TRACING
#define LPC_TRACFCOMP(des,printf_string,args...) \
    TRACFCOMP(des,printf_string,##args) /* FIX FIRDATA */
#else
#define LPC_TRACFCOMP(args...)
#endif

/**
 * @brief  ECCB Control Register Layout
 */
typedef union
{
    uint64_t data64;
    struct
    {
        /* unused sections should be set to zero */
        uint64_t magic1      : 4;  /**< 0:3 = b1101 per spec */
        uint64_t data_len    : 4;  /**< 4:7 = b0100 means 4 byte */
        uint64_t unused1     : 7;  /**< 8:14 */
        uint64_t read_op     : 1;  /**< 15 = set for read operation */
        uint64_t unused2     : 7;  /**< 16:22 */
        uint64_t addr_len    : 3;  /**< 23:25 = b100 means 4 byte */
        uint64_t unused3     : 6;  /**< 26:31 */
        uint64_t address     : 32; /**< 32:63 = LPC Address */
    };
} ControlReg_t;

/**
 * @brief  ECCB Status Register Layout
 */
typedef union
{
    uint64_t data64;
    struct
    {
        uint64_t unused      : 6;  /**< 0:5  */
        uint64_t read_data   : 32; /**< 6:37 */
        uint64_t unused1     : 3;  /**< 38:40 */
        uint64_t eccb_err    : 3;  /**< 41:43 = ECCB_Error_Info */
        uint64_t busy        : 1;  /**< 44 = Operation Busy */
        uint64_t unused2     : 7;  /**< 45:51 */
        uint64_t op_done     : 1;  /**< 52 = Command Complete */
        uint64_t unused3     : 3;  /**< 53:55 */
        uint64_t addr_parity_err : 1; /**< 56 = ECC Address Register
                                       Parity Error */
        uint64_t unused4     : 7;  /**< 57:63 */
    };
} StatusReg_t;


uint32_t checkAddr(LpcTransType i_type,
                   uint32_t i_addr)
{
    uint32_t full_addr = 0;
    switch ( i_type )
    {
        case LPC_TRANS_IO:
            full_addr = i_addr | LPCHC_IO_SPACE;
            break;
        case LPC_TRANS_FW:
            full_addr = i_addr | LPCHC_FW_SPACE;
            break;
    }
    return full_addr;
}


errorHndl_t pollComplete(const ControlReg_t* i_ctrl,
                               StatusReg_t* o_stat)
{
    errorHndl_t l_err = NO_ERROR;

    do {
        uint64_t poll_time = 0;
        uint64_t loop = 0;
        do
        {
            o_stat->data64 = xscom_read( ECCB_STAT_REG );
            LPC_TRACFCOMP( "writeLPC> Poll on ECCB Status, "
                           "poll_time=0x%.16x, stat=0x%.16x",
                           poll_time,
                           o_stat->data64 );
            if( l_err )
            {
                break;
            }

            if( o_stat->op_done )
            {
                break;
            }

            /* want to start out incrementing by small numbers then get bigger
               to avoid a really tight loop in an error case so we'll increase
               the wait each time through */
            sleep( ECCB_POLL_INCR_NS*(++loop) );
            poll_time += ECCB_POLL_INCR_NS * loop;
        } while ( poll_time < ECCB_POLL_TIME_NS );

        /* Check for hw errors or timeout if no previous logs */
        if( (l_err == NO_ERROR) &&
            ((o_stat->data64 & ECCB_STAT_REG_ERROR_MASK)
             || (!o_stat->op_done)) )
        {
            TRACFCOMP( "LpcDD::pollComplete> LPC error or timeout: addr=0x%.8X, status=0x%.8X%.8X",
                       i_ctrl->address, (uint32_t)(o_stat->data64>>32), (uint32_t)o_stat->data64 );
            l_err = -1;
            break;
        }
    } while(0);

    return l_err;
}


/*========================================================*/

errorHndl_t lpc_read( LpcTransType i_type,
                     uint32_t i_addr,
                     uint8_t* o_data,
                     size_t i_size )
{
    errorHndl_t l_err = NO_ERROR;
    uint32_t l_addr = 0;

    do {
        if( o_data == NULL )
        {
            TRACFCOMP( "o_data is NULL!" );
            l_err = -2;
            break;
        }

        /* Generate the full absolute LPC address */
        l_addr = checkAddr( i_type, i_addr );

        /* Execute command. */
        ControlReg_t eccb_cmd;
        eccb_cmd.data64 = ECCB_CTL_REG_DEFAULT;
        eccb_cmd.data_len = i_size;
        eccb_cmd.read_op = 1;
        eccb_cmd.addr_len = sizeof(l_addr);
        eccb_cmd.address = l_addr;
        xscom_write( ECCB_CTL_REG, eccb_cmd.data64 );

        /* Poll for completion */
        StatusReg_t eccb_stat;
        l_err = pollComplete( &eccb_cmd, &eccb_stat );
        if( l_err ) { break; }

        /* Copy data out to caller's buffer. */
        if( i_size <= sizeof(uint32_t) )
        {
            uint32_t tmpbuf = eccb_stat.read_data;
            memcpy( o_data, &tmpbuf, i_size );
        }
        else
        {
            TRACFCOMP( "readLPC> Unsupported buffer size : %d", i_size );
            l_err = -1;
            break;
        }

    } while(0);

    LPC_TRACFCOMP( "readLPC> %08X[%d] = %08X", l_addr, i_size, *reinterpret_cast<uint32_t*>( o_data )  >> (8 * (4 - i_size)) );

    return l_err;
}

errorHndl_t lpc_write( LpcTransType i_type,
                      uint32_t i_addr,
                      uint8_t* i_data,
                      size_t i_size )
{
    errorHndl_t l_err = NO_ERROR;
    uint32_t l_addr = 0;

    do {
        /* Generate the full absolute LPC address */
        l_addr = checkAddr( i_type, i_addr );

        uint64_t eccb_data = 0;
        /* Left-justify user data into data register. */
        switch ( i_size )
        {
            case 1:
                eccb_data = (uint64_t)
                   (*(const uint8_t*)(i_data)) << 56;
                break;
            case 2:
                eccb_data = (uint64_t)
                   (*(const uint16_t*)( i_data ) ) << 48;
                break;
            case 4:
                eccb_data = (uint64_t)
                   (*(const uint32_t*)( i_data ) ) << 32;
                break;
            default:
                TRACFCOMP( "writeLPC> Unsupported buffer size : %d", i_size );
                break;
        }

        /* Write data out */
        TRACZCOMP("ECCB_DATA_REG=%.8X%.8X",(uint32_t)(eccb_data>>32),(uint32_t)eccb_data);
        xscom_write( ECCB_DATA_REG, eccb_data );

        /* Execute command. */
        ControlReg_t eccb_cmd;
        eccb_cmd.data64 = ECCB_CTL_REG_DEFAULT;
        eccb_cmd.data_len = i_size;
        eccb_cmd.read_op = 0;
        eccb_cmd.addr_len = sizeof(l_addr);
        eccb_cmd.address = l_addr;
        xscom_write( ECCB_CTL_REG, eccb_cmd.data64 );
        TRACZCOMP("ECCB_CTL_REG=%.8X%.8X",(uint32_t)(eccb_cmd.data64>>32),(uint32_t)eccb_cmd.data64);

        /* Poll for completion */
        StatusReg_t eccb_stat;
        l_err = pollComplete( &eccb_cmd, &eccb_stat );
        if( l_err ) { break; }

    } while(0);

    return l_err;
}
