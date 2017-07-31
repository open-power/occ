/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/firdata/lpc.c $                                   */
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
#include <lpc.h>
#include <scom_util.h>
#include <gpe_util.h>

#define    LPCHC_FW_SPACE   0xF0000000 /**< LPC Host Controller FW Space */
#define    LPCHC_MEM_SPACE  0xE0000000 /**< LPC Host Controller Mem Space */
#define    LPCHC_IO_SPACE   0xD0010000 /**< LPC Host Controller I/O Space */
#define    LPCHC_REG_SPACE  0xC0012000 /**< LPC Host Ctlr Register Space */
#define    ECCB_NON_FW_RESET_REG  0x000B0001 /**< ECCB Reset Reg (non-FW) */

#define    LPC_BASE_REG    0x00090040 /**< LPC Base Address Register */
#define    LPC_CMD_REG     0x00090041 /**< LPC Command Register */
#define    LPC_DATA_REG    0x00090042 /**< LPC Data Register */
#define    LPC_STATUS_REG  0x00090043 /**< LPC Status Register */

#define    ECCB_CTL_REG   0x000B0020 /**< ECCB Control Reg (FW) */
#define    ECCB_RESET_REG   0x000B0021 /**< ECCB Reset Reg (FW) */
#define    ECCB_STAT_REG  0x000B0022 /**< ECCB Status Reg (FW) */
#define    ECCB_DATA_REG  0x000B0023 /**< ECCB Data Reg (FW) */

/* Default Values to set for all operations
    1101.0100.0000.000x.0000.0001.0000.0000.<address> */
#define    ECCB_CTL_REG_DEFAULT  0xD400010000000000

/* Error bits: wh_todo comments here */
#define    LPC_STAT_REG_ERROR_MASK  0x0000000000000000 /**< Error Bits */ //wh_todo correctly set mask

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

#define    LPC_POLL_TIME_NS  400000 /**< max time should be 400ms */
#define    LPC_POLL_INCR_NS  100000 /**< increase for testing */

#define    LPCHC_SYNC_CYCLE_COUNTER_INFINITE  0xFF000000

/**
 * @brief  LPC Base Register Layout
*/
typedef union
{

    uint64_t data64;

    struct
    {
        /* unused sections should be set to zero */
        uint64_t unused0   : 8;  /**< 0:7 */
        uint64_t base_addr : 24; /**< 8:31 */
        uint64_t unused1   : 31; /**< 32:62 */
        uint64_t disable   : 1;  /**< 63 */
    };
} BaseReg_t;

/**
 * @brief  LPC Control Register Layout
 */
typedef union
{

    uint64_t data64;

    struct
    {
        /* unused sections should be set to zero */
        //       rnw == read not write
        uint64_t rnw      : 1;  /**< 0 = Setting to 1 causes read */
        uint64_t unused0  : 4;  /**< 1:4 */
        uint64_t size     : 7;  /**< 5:11 */
        uint64_t unused1  : 20; /**< 12:31 */
        uint64_t address  : 32; /**< 32:63 = LPC Address */
    };
} CommandReg_t;

/**
 * @brief  LPC Status Register Layout
 */
typedef union
{
    uint64_t data64;
    struct
    {
        uint64_t op_done     : 1;  /**< 0 */
        uint64_t unused0     : 9;  /**< 1:9 */
        uint64_t opb_valid   : 1;  /**< 10 */
        uint64_t opb_ack     : 1;  /**< 11 */
        uint64_t unused1     : 52; /**< 12:63 */
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


errorHndl_t pollComplete(CommandReg_t* i_ctrl,
                         StatusReg_t* o_stat)
{
    errorHndl_t l_err = NO_ERROR;

    do {
        uint64_t poll_time = 0;
        uint64_t loop = 0;
        do
        {
            SCOM_Trgt_t l_target;
            l_target.type = TRGT_PROC;
            l_target.isMaster = TRUE;
            l_err = SCOM_getScom(l_target, LPC_STATUS_REG, &(o_stat->data64));

            if( l_err )
            {
                break;
            }

            if( o_stat->op_done )
            {
                break;
            }

            /* Want to start out incrementing by small numbers then get bigger
               to avoid a really tight loop in an error case so we'll increase
               the wait each time through */
            busy_wait( (LPC_POLL_INCR_NS*(++loop)));
            poll_time += LPC_POLL_INCR_NS * loop;
        } while ( poll_time < LPC_POLL_TIME_NS );

        /* Check for hw errors or timeout if no previous logs */
        if( (l_err == NO_ERROR) &&
            ((o_stat->data64 & LPC_STAT_REG_ERROR_MASK)
             || (!o_stat->op_done)) )
        {
            TRAC_ERR( "LpcDD::pollComplete> LPC error or timeout: addr=0x%.8X, status=0x%.8X%.8X",
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
                     uint32_t i_size )
{
    errorHndl_t l_err = NO_ERROR;
    int32_t l_addr = 0;
    uint64_t l_ret;
    uint32_t l_shift_amount;
    uint64_t l_temp_data;

    do {
        if( o_data == NULL )
        {
            TRAC_ERR("o_data is NULL!" );
            l_err = -2;
            break;
        }

        /* Generate the full absolute LPC address */
        l_addr = checkAddr( i_type, i_addr );

        /* Setup command */
        CommandReg_t lpc_cmd;
        lpc_cmd.rnw     = 1;  //Indicate read not write
        lpc_cmd.size    = i_size;
        lpc_cmd.address = l_addr;

        /* Execute command via Scom */
        SCOM_Trgt_t l_target;
        l_target.type = TRGT_PROC;
        l_target.isMaster = TRUE;

        //First write the address we want to read from in the
        //LPC_CMD_REG scom address
        l_err = SCOM_putScom(l_target, LPC_CMD_REG, lpc_cmd.data64);
        if(l_err != SUCCESS)
        {
            TRAC_ERR("lpc_read: SCOM_putScom failed to write to LPC_CMD_REG command rc=0x%08x",
                    (uint32_t)l_err);
            break;
        }

        /* Poll for completion */
        StatusReg_t lpc_status;
        l_err = pollComplete( &lpc_cmd, &lpc_status );
        if( l_err ) { break; }

        // Read data from the LPC_DATA_REG
        l_err = SCOM_getScom(l_target, LPC_DATA_REG, &l_ret);
        if(l_err != SUCCESS)
        {
            TRAC_ERR("lpc_read: SCOM_getScom failed rc=0x%08x", (uint32_t)l_err);
            break;
        }

        //The scom returns the data in the byte offset represented by last
        //3 bits of the address. For example, addr 0x21 will have have data
        //starting in byte1 on value returned from scom.
        //addr & 0x7 <-- this gives the byte offset
        //7 - (addr & 0x7) <-- subratcing from 7 as the data is left aligned
        //adding (size - 1) <-- to incorporate reading more than one byte
        //multiply by 8 to convert from byte to bits
        l_shift_amount = (7 - ((l_addr & 0x7) + (i_size-1))) * 8;
        l_temp_data    = l_ret >> l_shift_amount;

        //Had some weird problems with memcpy, that's why typecasting
        //to the size of data asked.
        if (i_size == sizeof(uint8_t))
        {
            uint8_t* l_temp_ptr = (uint8_t*)(o_data);
            *l_temp_ptr = l_temp_data;
        }
        else if (i_size == sizeof(uint16_t))
        {
            uint16_t* l_temp_ptr = (uint16_t*)(o_data);
            *l_temp_ptr = l_temp_data;
        }
        else if (i_size == sizeof(uint32_t))
        {
            uint32_t* l_temp_ptr = (uint32_t*)(o_data);
            *l_temp_ptr = l_temp_data;
        }
        else if (i_size == sizeof(uint64_t))
        {
            uint64_t* l_temp_ptr = (uint64_t*)(o_data);
            *l_temp_ptr = l_temp_data;
        }
        else
        {
            TRAC_ERR("lpc_read: unsupported size length");
            l_err = -1;
            break;
        }

    } while(0);

    return l_err;
}

errorHndl_t lpc_write( LpcTransType i_type,
                      uint32_t i_addr,
                      uint8_t* i_data,
                      uint32_t i_size )
{
    errorHndl_t l_err = NO_ERROR;
    uint32_t l_addr = 0;
    uint64_t l_write_data = 0;
    uint64_t l_data = 0;
    uint32_t l_shift_amount;

    do {
        /* Generate the full absolute LPC address */
        l_addr = checkAddr( i_type, i_addr );

        /* Setup Write Command */
        CommandReg_t lpc_cmd;
        lpc_cmd.rnw     = 0;  //Indicate write
        lpc_cmd.size    = i_size;
        lpc_cmd.address = l_addr;

        /* Setup Write command via Scom */
        SCOM_Trgt_t l_target;
        l_target.type = TRGT_PROC;
        l_target.isMaster = TRUE;

        //First write the address we want to write to in LPC_CMD_REG
        l_err = SCOM_putScom(l_target, LPC_CMD_REG, lpc_cmd.data64);
        if(l_err != SUCCESS)
        {
            TRAC_ERR("ERROR> lpc_write: SCOM_putScom failed to write to LPC_CMD_REG command: rc=0x%08x",
                    (uint32_t)l_err);
            break;
        }

        //There were some weird memcpy problems. That's why, typecasting
        //to the size of data requested to write
        if (i_size == sizeof(uint8_t))
        {
            l_write_data = (*i_data);
        }
        else if (i_size == sizeof(uint16_t))
        {
            uint16_t* l_temp_ptr = (uint16_t*)(i_data);
            l_write_data = *l_temp_ptr;
        }
        else if (i_size == sizeof(uint32_t))
        {
            uint32_t* l_temp_ptr = (uint32_t*)(i_data);
            l_write_data = *l_temp_ptr;
        }
        else if (i_size == sizeof(uint64_t))
        {
            uint64_t* l_temp_ptr = (uint64_t*)(i_data);
            l_write_data = *l_temp_ptr;
        }
        else
        {
            TRAC_ERR("lpc_write: unsupported size length");
            l_err = -1;
            break;
        }

        //The scom expects the data in the byte offset represented by last
        //3 bits of the address. For example, addr 0x21 will have have data
        //starting in byte1 on value returned from scom.
        //addr & 0x7 <-- this gives the byte offset
        //7 - (addr & 0x7) <-- subratcing from 7 as the data in the scom reg
        //is expected to be left aligned
        //adding (size - 1) <-- to incorporate reading more than one byte
        //multiply by 8 to convert from byte to bits

        l_shift_amount = (7 - ((l_addr & 0x7) + (i_size-1))) * 8;
        l_data = (l_write_data << l_shift_amount);

        //Write the value to the LPC_DATA_REG
        l_err = SCOM_putScom(l_target, LPC_DATA_REG, l_data);
        if(l_err != SUCCESS)
        {
            TRAC_ERR("ERROR> lpc_write: SCOM_putScom failed to write to LPC_DATA_REG data: rc=0x%08x",
                    (uint32_t)l_err);
            break;
        }

        /* Poll for completion */
        StatusReg_t lpc_stat;
        l_err = pollComplete( &lpc_cmd, &lpc_stat );
        if( l_err ) { break; }


    } while(0);

    return l_err;
}
