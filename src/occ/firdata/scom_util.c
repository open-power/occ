/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ/firdata/scom_util.C $                                 */
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

/* Support for SCOM operations */
#include <scom_util.h>
#include <fsi.h>
#include <native.h>

enum {
    /*FSI addresses are byte offsets, so need to multiply by 4
      since each register is 4 bytes long.
      prefix with 0x10xx for FSI2PIB engine offset */
    DATA0_REG         = 0x1000,  /* SCOM Data Register 0 (0x00) */
    DATA1_REG         = 0x1004,  /* SCOM Data Register 1 (0x01) */
    COMMAND_REG       = 0x1008,  /* SCOM Command Register (0x02) */
    ENGINE_RESET_REG  = 0x1018,  /* Engine Reset Register (0x06) */
    STATUS_REG        = 0x101C,  /* STATUS Register (0x07) */
    PIB_RESET_REG     = 0x101C,  /* PIB Reset Register (0x07) */

    PIB_ABORT_BIT     = 0x00100000, /* 12= PIB Abort */
    PIB_ERROR_BITS    = 0x00007000, /* 17:19= PCB/PIB Errors */
};

/**
 * @brief Indirect SCOM Status
 */
typedef union
{
    uint64_t data64;
    struct
    {
        uint64_t :12;        /*0:11*/
        uint64_t addr:20;    /*12:31*/
        uint64_t done:1;     /*32*/
        uint64_t piberr:3;   /*33:35*/
        uint64_t userstat:4; /*36:39*/
        uint64_t :8;         /*40:47*/
        uint64_t data:16;    /*48:63*/
    };
} IndirectScom_t;

enum {
    MCS_MASK =             0xFFFFFFFF7FFFFF80,
    MCS_BASEADDR =         0x0000000002011800,
    MCS_DMI_BASEADDR =     0x0000000002011A00,
    IND_MCS_BASEADDR =     0x8000006002011A00,
    IND_MCS_DMI_BASEADDR = 0x8000006002011A3F,
    MBA_MASK =             0xFFFFFFFF7FFFFC00,
    MBA_BASEADDR =         0x0000000003010400,
    TCM_MBA_BASEADDR =     0x0000000003010800,
    IND_MBA_BASEADDR =     0x800000000301143f,
};



/**
 * @brief Translate a relative unit address to a real physical address
 * @param[in] Chip/unit to scom
 * @param[in] Address to scom, unit0's address for unit scoms
 * @return Physical address
 */
uint32_t translate_scom( SCOM_Trgt_t i_target,
                         uint32_t i_address )
{
    TrgtType_t l_type = i_target.type;
    uint32_t l_addr = i_address;

    /* No translation needed for non-unit scoms */
    if( (l_type == PROC) || (l_type == MEMB) )
    {
        l_addr = i_address;
    }
    else /* it is a Unit */
    {
        uint8_t l_num = SCOM_Trgt_getChipUnitPos(i_target);

        if( l_type == EX )
        {
            /*first byte is 0x10, second nibble of that byte is the EX number */
            l_addr |= (l_num << 24);
        }
        else if( l_type == MCS )
        {
            /*Non-DMI address */
            if( (i_address & MCS_MASK) == MCS_BASEADDR )
            {
                /* MC0 MCS0   = 0x02011800   MCS-0    range 0 */
                /* MC0 MCS1   = 0x02011880   MCS-1    range 0 + remainder */
                /* MC1 MCS0   = 0x02011900   MCS-2    range 1 */
                /* MC1 MCS0   = 0x02011980   MCS-3    range 1 + remainder */
                /* MC2 MCS0   = 0x02011C00   MCS-4     range 2 */
                /* MC2 MCS1   = 0x02011C80   MCS-5     range 2 + remainder */
                /* MC3 MCS0   = 0x02011D00   MCS-6     range 3 */
                /* MC3 MCS1   = 0x02011D80   MCS-7     range 3 + remainder */
                if( (l_num / 2) == 1) /*range 1 */
                {
                    l_addr += 0x100;
                }
                else if( (l_num / 2) == 2)  /*range 2 */
                {
                    l_addr += 0x400;
                }
                else if( (l_num / 2) == 3)  /*range 3 */
                {
                    l_addr += 0x500;
                }

                /* Add 0x80 for the odd numbers */
                if( l_num % 2)
                {
                    l_addr += 0x80;
                }
            }
            else if( (i_address & MCS_MASK) == MCS_DMI_BASEADDR )
            {
                /* 0x00000000_02011A00      MCS      0-3 # MCS/DMI0 Direct SCOM */
                /* 0x00000000_02011E00      MCS      4-7 # MCS/DMI4 Direct SCOM */
                if( l_num > 3 )
                {
                    l_addr |= 0x400; /* A00->E00 */
                }
            }
            else if( (i_address & MCS_MASK) == IND_MCS_DMI_BASEADDR )
            {
                /*   0x80000060_02011A3F      MCS   0  # DMI0 Indirect SCOM RX3 */
                /*   0x80000040_02011A3F      MCS   1  # DMI1 Indirect SCOM RX2 */
                /*   0x80000000_02011A3F      MCS   2  # DMI3 Indirect SCOM RX0 */
                /*   0x80000020_02011A3F      MCS   3  # DMI2 Indirect SCOM RX1 */
                /* */
                /*   0x80000060_02011E3F      MCS   4  # DMI4 Indirect SCOM RX3 */
                /*   0x80000040_02011E3F      MCS   5  # DMI5 Indirect SCOM RX2 */
                /*   0x80000000_02011E3F      MCS   6  # DMI7 Indirect SCOM RX0 */
                /*   0x80000020_02011E3F      MCS   7  # DMI6 Indirect SCOM RX1 */
                /* */
                /*   0x80000460_02011A3F      MCS   0  # DMI0 Indirect SCOM TX3 */
                /*   0x80000440_02011A3F      MCS   1  # DMI1 Indirect SCOM TX2 */
                /*   0x80000400_02011A3F      MCS   2  # DMI3 Indirect SCOM TX0 */
                /*   0x80000420_02011A3F      MCS   3  # DMI2 Indirect SCOM TX1 */
                /* */
                /*   0x80000460_02011E3F      MCS   4  # DMI4 Indirect SCOM TX3 */
                /*   0x80000440_02011E3F      MCS   5  # DMI5 Indirect SCOM TX2 */
                /*   0x80000400_02011E3F      MCS   6  # DMI7 Indirect SCOM TX0 */
                /*   0x80000420_02011E3F      MCS   7  # DMI6 Indirect SCOM TX1 */

                /* zero out the instance bits */
                l_addr &= 0xFFFFFF9FFFFFFFFF;
                switch( l_num )
                {
                    case(0):
                    case(4):
                        l_addr |= 0x0000006000000000;
                        break;
                    case(1):
                    case(5):
                        l_addr |= 0x0000004000000000;
                        break;
                    case(2):
                    case(6):
                        /*nothing to do */
                        break;
                    case(3):
                    case(7):
                        l_addr |= 0x0000002000000000;
                        break;
                    default:
                        l_addr = SCOMFAIL;
                }
                if( l_num > 3 )
                {
                    l_addr |= 0x400; /* A00->E00 */
                }
            }
        }
        else if( l_type == MBA )
        {
            if( (i_address & MBA_MASK) == MBA_BASEADDR )
            {
                /* 0x00000000_03010400   MBA 0   # MBA01 */
                /* 0x00000000_03010C00   MBA 1   # MBA23 */
                if( l_num == 1 )
                {
                    l_addr |= 0x00000800;
                }
            }
            else if( (i_address & MBA_MASK) == TCM_MBA_BASEADDR )
            {
                /* 0x00000000_03010880   MBA 0    # Trace for MBA01 */
                /* 0x00000000_030110C0   MBA 1    # Trace for MBA23 */
                l_addr += (l_num * 0x840);
            }
            else if( (i_address & MBA_MASK) == IND_MBA_BASEADDR )
            {
                /* 0x00000000_03011400   MBA 0   # DPHY01 (indirect addressing) */
                /* 0x00000000_03011800   MBA 1   # DPHY23 (indirect addressing) */
                /* 0x80000000_0301143f   MBA  0  # DPHY01 (indirect addressing) */
                /* 0x80000000_0301183f   MBA  1  # DPHY23 (indirect addressing) */
                /* 0x80000000_0701143f   MBA 0   # DPHY01 (indirect addressing) */
                /* 0x80000000_0701183f   MBA 1   # DPHY23 (indirect addressing) */
                if( l_num == 1 )
                {
                    /* 030114zz->030118zz */
                    l_addr &= 0xFFFFFFFFFFFFFBFF;
                    l_addr |= 0x0000000000000800;
                }
            }
        }
        else
        {
            l_addr = SCOMFAIL;
        }
    }

    return l_addr;
}

/**
 * @brief Perform a getscom operation with no address translation
 */
uint64_t getscomraw( SCOM_Trgt_t i_chip,
                     uint32_t i_address )
{
    int32_t rc = SUCCESS;

    uint64_t scomdata = SCOMFAIL;

    /* SCOMs to the master chip are done via XSCOM. */
    if ( i_chip.isMaster )
    {
        rc = xscom_read( i_address, &scomdata );
        if ( SUCCESS != rc ) scomdata = SCOMFAIL;
        return scomdata;
    }

    /* 1) Sent the command to do the SCOM read. */
    rc = putfsi( i_chip, COMMAND_REG, i_address );
    if ( SUCCESS != rc ) return SCOMFAIL;

    /*2) check status next -- TODO */

    /* 3) Read the two data registers. */
    uint32_t data0, data1;

    rc = getfsi( i_chip, DATA0_REG, &data0 );
    if ( SUCCESS != rc ) return SCOMFAIL;

    rc = getfsi( i_chip, DATA1_REG, &data1 );
    if ( SUCCESS != rc ) return SCOMFAIL;

    scomdata = ((uint64_t)data0 << 32) | (uint64_t)data1;

    return scomdata;
}

/**
 * @brief Perform a scom operation with no address translation
 */
void putscomraw( SCOM_Trgt_t i_chip,
                 uint32_t i_address,
                 uint64_t i_data )
{
    int32_t rc = SUCCESS;

    /* SCOMs to the master chip are done via XSCOM. */
    if ( i_chip.isMaster )
    {
        rc = xscom_write( i_address, i_data );
        return; // TODO: Will need to return rc.
    }

    /* 1) Write the two data registers. */
    rc = putfsi( i_chip, DATA0_REG, i_data >> 32 );
    if ( SUCCESS != rc ) return;

    rc = putfsi( i_chip, DATA1_REG, (uint32_t)i_data );
    if ( SUCCESS != rc ) return;

    /* 2) Send the command to do the SCOM write. */
    rc = putfsi( i_chip, COMMAND_REG, i_address | 0x80000000 );
    if ( SUCCESS != rc ) return;

    /*3) check status next -- TODO */
}

/**
 * @brief Execute standard getscom and return result
 */
int32_t SCOM_getScom( SCOM_Trgt_t i_trgt, uint32_t i_addr, uint64_t * o_val )
{
    int32_t rc = SUCCESS;

    /* Translate the input args into physical addr/chip */
    uint32_t real_addr = translate_scom( i_trgt, i_addr );
    SCOM_Trgt_t chip_targ = SCOM_Trgt_getParentChip(i_trgt);

    /* Do the scom */
    *o_val = getscomraw( chip_targ, real_addr );
    if ( SCOMFAIL == *o_val ) { rc = FAIL; }

    return rc;
}

/**
 * @brief Execute indirect getscom and return result
 */
int32_t SCOM_getIdScom( SCOM_Trgt_t i_trgt, uint64_t i_addr, uint32_t * o_val )
{
    int32_t rc = SUCCESS;

    /* First translate the address */
    uint64_t trans_addr = translate_scom( i_trgt, i_addr );
    SCOM_Trgt_t chip_targ = SCOM_Trgt_getParentChip(i_trgt);

    /* An indirect scom is performed by putting the top of the */
    /*  64-bit address into the first data word of the scom */

    /* zero out the indirect address from the buffer.. */
    /* bit 0-31 - indirect area.. */
    /* bit 32 - always 0 */
    /* bit 33-47 - bcast/chipletID/port */
    /* bit 48-63 - local addr */
    uint32_t phys_addr = trans_addr & 0x000000007FFFFFFF;

    /* To do a read we need to do a write first */

    /* start with the 20bit indirect address */
    uint64_t data_buffer = trans_addr & 0x001FFFFF00000000;
    /* turn the read bit on. */
    data_buffer |= 0x8000000000000000;

    /* perform write before the read with the new */
    putscomraw( i_trgt, phys_addr, data_buffer );

    // Loop on read until we see done, error, or we timeout
    IndirectScom_t scomout;
    uint64_t elapsed_indScom_time_ns = 0;
    do
    {
        /* Now perform the op requested using the passed in */
        /* IO_Buffer to pass the read data back to caller. */
        scomout.data64 = getscomraw( chip_targ, phys_addr );
        if( scomout.data64 == SCOMFAIL )
        {
            break;
        }

        /* jump out on done or error */
        if( scomout.done || scomout.piberr )
        {
            break;
        }

        sleep( 10000 ); /*sleep for 10,000 ns */
        elapsed_indScom_time_ns += 10000;

    } while( elapsed_indScom_time_ns <= 100000 ); /*wait for .1ms */

    if( (scomout.data64 == SCOMFAIL)
        || (scomout.piberr)
        || !(scomout.done) )
    {
        *o_val = SCOMFAIL;
    }
    else
    {
        *o_val = scomout.data;
    }

    if ( SCOMFAIL == *o_val ) { rc = FAIL; }

    return rc;
}
