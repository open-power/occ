/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe1_24x7.h $                                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2018                        */
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

#ifndef _GPE1_24x7_H
    #define _GPE1_24x7_H
//
//PMU config table defined below.
#define TOTAL_CONFIGS 60
#define TOTAL_POSTINGS 238
#define INC_UPD_COUNT 1
//
#define TOTAL_CONFIG_SCOMS 60
#define TOTAL_COUNTER_SCOMS 71
//
#define MAX_32 4294967295ULL
#define MAX_48 281474976710655ULL
static volatile uint64_t  G_CUR_UAV = 0;
static volatile uint64_t  G_CUR_MODE = 0;
static volatile uint64_t  G_MBA_pmulets[24];
static volatile uint64_t  G_PHB_pmulets[24];
uint64_t G_PMU_CONFIGS_8[][2] = 
{
//cnpm//
    {0x5011c13, 0x92aaaaaaac000bf8},//0
    {0x5011c14, 0x92a8aaaaac000bf8},//1
    {0x5011c15, 0xaaaaaaaaaaaaaaaa},//2
    {0x5011c16, 0x0400c0ff00ff0000},//3
//MC23//
    {0x3010829, 0x0500000000000000},//port0//4
    {0x3010839, 0x0500000000000000},//port1//5
    {0x30108a9, 0x0500000000000000},//port2//6
    {0x30108b9, 0x0500000000000000},//port3//7
    {0x301083c, 0x0000000000000000},//port0,1//8
    {0x301083d, 0x000000005345755b},//port0,1//9
    {0x301083e, 0x6471000000000000},//port0,1//10
    {0x301083f, 0x00f0800000000000},//port0,1//11
    {0x30108bc, 0x0000000000000000},//port2,3//12
    {0x30108bd, 0x0000000000000000},//port2,3//13
    {0x30108be, 0x00005345755b6000},//port2,3//14
    {0x30108bf, 0x000e800000000000},//port2,3//15
//MC01//
    {0x5010829, 0x0500000000000000},//port0//16
    {0x5010839, 0x0500000000000000},//port1//17
    {0x50108a9, 0x0500000000000000},//port2//18
    {0x50108b9, 0x0500000000000000},//port3//19
    {0x501083c, 0x0000000000000000},//port0,1//20
    {0x501083d, 0x000000005345755b},//port0,1//21
    {0x501083e, 0x6471000000000000},//port0,1//22
    {0x501083f, 0x00f0800000000000},//port0,1//23
    {0x50108bc, 0x0000000000000000},//port2,3//24
    {0x50108bd, 0x0000000000000000},//port2,3//25
    {0x50108be, 0x00005345755b6000},//port2,3//26
    {0x50108bf, 0x000e800000000000},//port2,3//27
//xlinks//
    {0x501341a, 0xff015daa15555000},//all xlinks//28
//nx//
    {0x20110a6, 0xcaa0000080000000},//29
    {0x20110a9, 0xcaa0492480000000},//30
    {0x2011054, 0x0000000000000000},//31
    {0x2011055, 0x00000000600002aa},//32
//nvlinks-NTL,ATS,XTS//
    {0x501111e, 0xb0aa0144868a0000},//NTL0//33
    {0x501113e, 0xb0aa0144868a0000},//NTL1//34
    {0x501131e, 0xb0aa0144868a0000},//NTL2//35
    {0x501133e, 0xb0aa0144868a0000},//NTL3//36
    {0x501151e, 0xb0aa0144868a0000},//NTL4//37
    {0x501153e, 0xb0aa0144868a0000},//NTL5//38
    {0x5011600, 0xb0aa420000000000},//ATS//39
    {0x5011645, 0xb0aa03050d0e0000},//XTS//40
//PHB
    {0xd010917, 0x8000011516190000},//PHB0//41
    {0xe010917, 0x8000011516190000},//PHB1//42
    {0xe010957, 0x8000011516190000},//PHB2//43
    {0xf010917, 0x8000011516190000},//PHB3//44
    {0xf010957, 0x8000011516190000},//PHB4//45
    {0xf010997, 0x8000011516190000},//PHB5//46
//nvlinks-NPCQ//
    {0x50110C7, 0x90aa1012181a0000},//NPCQ01//47
    {0x50112C7, 0x90aa1012181a0000},//NPCQ02//48
    {0x50114C7, 0x90aa1012181a0000},//NPCQ03//49
//CAPP
    {0x2010814, 0x8058812200000000},//CAPP01//50
    {0x2010816, 0x6000000000000000},//CAPP01//51
    {0x2010824, 0x8058914202000000},//CAPP01//52
    {0x2010822, 0x1000000000000000},//CAPP01//53
    {0x2010817, 0x0000000000000000},//CAPP01//54
    {0x4010814, 0x8058812200000000},//CAPP02//55
    {0x4010816, 0x6000000000000000},//CAPP02//56
    {0x4010824, 0x8058914202000000},//CAPP02//57
    {0x4010822, 0x1000000000000000},//CAPP02//58
    {0x4010817, 0x0000000000000000} //CAPP02//59
};

uint64_t G_PMU_CONFIGS_16[][2] = 
{
//cnpm
    {0x5011c13, 0x92aaaaaaac000bf8},//0
    {0x5011c14, 0x92a8aaaaac000bf8},//1
    {0x5011c15, 0x5555555555555555},//2
    {0x5011c16, 0x0400c0ff00ff0000},//3
//MC23//	                        
    {0x3010829, 0x0500000000000000},//port0//4
    {0x3010839, 0x0500000000000000},//port1//5
    {0x30108a9, 0x0500000000000000},//port2//6
    {0x30108b9, 0x0500000000000000},//port3//7
    {0x301083c, 0x0000000000000000},//port0,1//8
    {0x301083d, 0x000000005345755b},//port0,1//9
    {0x301083e, 0x6471000000000000},//port0,1//10
    {0x301083f, 0x00f0800000000000},//port0,1//11
    {0x30108bc, 0x0000000000000000},//port2,3//12
    {0x30108bd, 0x0000000000000000},//port2,3//13
    {0x30108be, 0x00005345755b6000},//port2,3//14
    {0x30108bf, 0x000e800000000000},//port2,3//15
//MC01//                            
    {0x5010829, 0x0500000000000000},//port0//16
    {0x5010839, 0x0500000000000000},//port1//17
    {0x50108a9, 0x0500000000000000},//port2//18
    {0x50108b9, 0x0500000000000000},//port3//19
    {0x501083c, 0x0000000000000000},//port0,1//20
    {0x501083d, 0x000000005345755b},//port0,1//21
    {0x501083e, 0x6471000000000000},//port0,1//22
    {0x501083f, 0x00f0800000000000},//port0,1//23
    {0x50108bc, 0x0000000000000000},//port2,3//24
    {0x50108bd, 0x0000000000000000},//port2,3//25
    {0x50108be, 0x00005345755b6000},//port2,3//26
    {0x50108bf, 0x000e800000000000},//port2,3//27
//xlinks                            
    {0x501341a, 0xff015d5515555000},//all xlinks//28
//nx                                
    {0x20110a6, 0xc550000080000000},//29
    {0x20110a9, 0xc550492480000000},//30
    {0x2011054, 0x0000000000000000},//31
    {0x2011055, 0x00000000600002aa},//32
//nvlinks-NTL,ATS,XTS//             
    {0x501111e, 0xb0550144868a0000},//NTL0//33
    {0x501113e, 0xb0550144868a0000},//NTL1//34
    {0x501131e, 0xb0550144868a0000},//NTL2//35
    {0x501133e, 0xb0550144868a0000},//NTL3//36
    {0x501151e, 0xb0550144868a0000},//NTL4//37
    {0x501153e, 0xb0550144868a0000},//NTL5//38
    {0x5011600, 0xb055420000000000},//ATS//39
    {0x5011645, 0xb05503050d0e0000},//XTS//40
//PHB                               
    {0xd010917, 0x8000011516190000},//PHB0//41
    {0xe010917, 0x8000011516190000},//PHB1//42
    {0xe010957, 0x8000011516190000},//PHB2//43
    {0xf010917, 0x8000011516190000},//PHB3//44
    {0xf010957, 0x8000011516190000},//PHB4//45
    {0xf010997, 0x8000011516190000},//PHB5//46
//nvlinks-NPCQ//                    
    {0x50110C7, 0x90551012181a0000},//NPCQ01//47
    {0x50112C7, 0x90551012181a0000},//NPCQ02//48
    {0x50114C7, 0x90551012181a0000},//NPCQ03//49
//CAPP                              
    {0x2010814, 0x803840a100000000},//CAPP01//50
    {0x2010816, 0x6000000000000000},//CAPP01//51
    {0x2010824, 0x803850c102000000},//CAPP01//52
    {0x2010822, 0x1000000000000000},//CAPP01//53
    {0x2010817, 0x0000000000000000},//CAPP01//54
    {0x4010814, 0x803840a100000000},//CAPP02//55
    {0x4010816, 0x6000000000000000},//CAPP02//56
    {0x4010824, 0x803850c102000000},//CAPP02//57
    {0x4010822, 0x1000000000000000},//CAPP02//58
    {0x4010817, 0x0000000000000000} //CAPP02//59
};
//
//PMU counters below
uint64_t G_PMULETS_1[] =
{
//CNPM counters (4 east and 4 west)
    0x5011c1a,//0//0
    0x5011c1b,//1//1
    0x5011c1c,//2//2
    0x5011c1d,//3//3
    0x5011c1e,//4//4
    0x5011c1f,//5//5
    0x5011c20,//6//6
    0x5011c21 //7//7
};

uint64_t G_PMULETS_2[] =
{
//x1
    0x501341b,//0//8
    0x501341c,//1//9
//x0
    0x501341d,//2//10
    0x501341e,//3//11
//x2
    0x501341f,//4//12
    0x5013420,//5//13
//nx
    0x20110a7,//6//14
    0x20110aa //7//15
};

uint64_t G_PMULETS_3[] =
{
//NTL0
    0x501111f,//0//16
//NTL1
    0x501113f,//1//17
//NTL2
    0x501131f,//2//18
//NTL3
    0x501133f,//3//19
//NTL4
    0x501151f,//4//20
//NTL5
    0x501153f,//5//21
//ATS
    0x5011601,//6//22
//XTS
    0x5011648 //7//23
};

uint64_t G_PMULETS_4a[] =
{//PHB0
    0xd010918,//0//24
    0xd010919,//1//25
    0xd01091a,//2//26
    0xd01091b //3//27
};

uint64_t G_PMULETS_4b[] =
{//PHB1
    0xe010918,//0//28
    0xe010919,//1//29
    0xe01091a,//2//30
    0xe01091b //3//31
};

uint64_t G_PMULETS_4c[] =
{//PHB2
    0xe010958,//0//32
    0xe010959,//1//33
    0xe01095a,//2//34
    0xe01095b //3//35
};

uint64_t G_PMULETS_4d[] =
{//PHB3
    0xf010918,//0//36
    0xf010919,//1//37
    0xf01091a,//2//38
    0xf01091b //3//39
};

uint64_t G_PMULETS_4e[] =
{//PHB4
    0xf010958,//0//40
    0xf010959,//1//41
    0xf01095a,//2//42
    0xf01095b //3//43
};

uint64_t G_PMULETS_4f[] =
{//PHB5
    0xf010998,//0//44
    0xf010999,//1//45
    0xf01099a,//2//46
    0xf01099b //3//47
};

uint64_t G_PMULETS_5[] =
{
//MBA0
    0x7010937,//0//48
    0x7010939,//1//49
//MBA1
    0x7010977,//2//50
    0x7010979,//3//51
//MBA2
    0x70109b7,//4//52
    0x70109b9,//5//53
//MBA3
    0x70109f7,//6//54
    0x70109f9 //7//55
};

uint64_t G_PMULETS_6[] =
{
//MBA4
    0x8010937,//0//56
    0x8010939,//1//57
//MBA5
    0x8010977,//2//58
    0x8010979,//3//59
//MBA6
    0x80109b7,//4//60
    0x80109b9,//5//61
//MBA7
    0x80109f7,//6//62
    0x80109f9 //7//63
};

uint64_t G_PMULETS_7[] =
{
//NPCQ01
    0x50110c6,//0//64
//NPCQ02
    0x50112c6,//1//65
//NPCQ03
    0x50114c6,//2//66
//CAPP01
    0x2010815,//3//67
    0x2010825,//4//68
//CAPP02
    0x4010815,//5//69
    0x4010825//6//70
};

enum groups {G1=1,G2=2,G3=3,G4=4,G5=5,G6=6,G7=7};

enum
{
    TOD_VALUE_REG               = 0x00040020,
    ZERO                        = 0,
    PBA_ENABLE                  = 0x80000000,
    SGPE_24x7_OFFSET            = 0x00180000,//1MB + 512KB
    TEST_ADDR                   = 0x00180110,
    PB_PMU_CNPME_05011C13       = 0x05011C13,
    PB_PMU_CNPME_05011C14       = 0x05011C14,
    PB_PMU_CNPME_05011C13_CONF  = 0x92aaaa82ac002bf8,
    PB_PMU_CNPME_05011C13_CONF1 = 0x92aaaa82ac002bf8,
    PB_PMU_CNPME_05011C14_CONF  = 0x92a8aaaaac002bf8,
    PB_PMU_CNPME_05011C14_CONF1 = 0x92a8aaaaac002bf8,
    PBASLVCTL3_C0040030         = 0xC0040030,
    PBASLV_SET_DMA              = 0x97005EC000000000,
    PBASLV_SET_ATOMIC           = 0x97405EC060000000,
    //POSTING OFFSETS for groups
    POSTING_START               = 0x00180000,//1MB + 512KB
//
    POST_OFFSET_G1H             = 0x00180000,
    POST_OFFSET_G1T             = 0x00180108,
//
    POST_OFFSET_G2H             = 0x00180110,
    POST_OFFSET_G2_1            = 0x00180118,
    POST_OFFSET_G2_2            = 0x00180158,
    POST_OFFSET_G2_3            = 0x00180198,
    POST_OFFSET_G2_4            = 0x001801D8,
    POST_OFFSET_G2T             = 0x00180218,
//
    POST_OFFSET_G3H             = 0x00180220,
    POST_OFFSET_G3_1            = 0x00180228,
    POST_OFFSET_G3_2            = 0x00180248,
    POST_OFFSET_G3_3            = 0x00180268,
    POST_OFFSET_G3_4            = 0x00180288,
    POST_OFFSET_G3_5            = 0x001802A8,
    POST_OFFSET_G3_6            = 0x001802C8,
    POST_OFFSET_G3_7            = 0x001802E8,
    POST_OFFSET_G3T             = 0x00180328,
//
    POST_OFFSET_G4H             = 0x00180330,
    POST_OFFSET_G4a             = 0x00180338,
    POST_OFFSET_G4b             = 0x00180358,
    POST_OFFSET_G4c             = 0x00180378,
    POST_OFFSET_G4d             = 0x00180398,
    POST_OFFSET_G4e             = 0x001803B8,
    POST_OFFSET_G4f             = 0x001803D8,
    POST_OFFSET_G4T             = 0x00180438,
//
    POST_OFFSET_G5H             = 0x00180440,
    POST_OFFSET_G5_1            = 0x00180448,
    POST_OFFSET_G5_2            = 0x00180460,
    POST_OFFSET_G5_3            = 0x00180478,
    POST_OFFSET_G5_4            = 0x00180490,
    POST_OFFSET_G5T             = 0x00180548,
//
    POST_OFFSET_G6H             = 0x00180550,
    POST_OFFSET_G6_1            = 0x00180558,
    POST_OFFSET_G6_2            = 0x00180570,
    POST_OFFSET_G6_3            = 0x00180588,
    POST_OFFSET_G6_4            = 0x001805A0,
    POST_OFFSET_G6T             = 0x00180658,
//
    POST_OFFSET_G7H             = 0x00180660,
    POST_OFFSET_G7_1            = 0x00180668,
    POST_OFFSET_G7_2            = 0x00180688,
    POST_OFFSET_G7_3            = 0x001806A8,
    POST_OFFSET_G7_4            = 0x001806C8,
    POST_OFFSET_G7_5            = 0x00180708,
    POST_OFFSET_G7_6            = 0x00180748,
    POST_OFFSET_G7T             = 0x00180768,
//DEBUG mode offsets start here
    POSTING_START_DBG           = 0x00190000,
    POST_OFFSET_DBG1H           = 0x00190000,
    POST_OFFSET_DBG1T           = 0x00190108,
//
    //CONTROL BLOCK FIELD OFFSETS
    //cntl block @ last 1k of 24x7 space
    CNTL_STATUS_OFFSET          = 0x001BFC00,
    CNTL_CMD_OFFSET             = 0x001BFC08,
    CNTL_SPEED_OFFSET           = 0x001BFC10,
    CNTL_UAV_OFFSET             = 0x001BFC18,
    CNTL_MODE_OFFSET            = 0x001BFC20,
    CNTL_MODE_STATE_OFFSET      = 0x001BFC28,
///////////////////////////////////////////////////
    //error tracking fields(internal)
    //last conf scom
    DBG_VER_OFFSET              = 0x001AFC00,
    DBG_CONF_OFFSET             = 0x001AFC08,
    DBG_GRP_OFFSET              = 0x001AFC10,
    DBG_UNIT_OFFSET             = 0x001AFC18,
    DBG_TICS_OFFSET             = 0x001AFC20,
    DBG_MARK                    = 0x001AFC28,
    DBG_ITER                    = 0x001AFC30,
    DBG_STATE                   = 0x001AFC38,
    DBG_CONF_I                  = 0x001AFC40,
    DBG_UAV                     = 0x001AFC48,
    DBG_0                       = 0x001AFC50,
    DBG_1                       = 0x001AFC58,
    DBG_2                       = 0x001AFC60,
    DBG_3                       = 0x001AFC68,
    DBG_4                       = 0x001AFC70,
    DBG_5                       = 0x001AFC78,
    DBG_6                       = 0x001AFC80,
    DBG_7                       = 0x001AFC88,
    DBG_8                       = 0x001AFC90,
//////////////////////////////////////////////////
    //cntl status values
    CNTL_STATUS_INIT            = 0x0,
    CNTL_STATUS_RUN             = 0x1,
    CNTL_STATUS_PAUSE           = 0x2,
    CNTL_STATUS_ERR1            = 0x3,
    CNTL_STATUS_ERR2            = 0x4,
    //cntl cmd values
    CNTL_CMD_NOP                = 0x0,
    CNTL_CMD_RESUME             = 0x1,
    CNTL_CMD_PAUSE              = 0x2,
    CNTL_CMD_CLEAR              = 0x3,
    //cntl speed values
    CNTL_SPEED_1MS              = 0x0,
    CNTL_SPEED_2MS              = 0x1,
    CNTL_SPEED_4MS              = 0x2,
    CNTL_SPEED_8MS              = 0x3,
    CNTL_SPEED_16MS             = 0x4,
    CNTL_SPEED_32MS             = 0x5,
    CNTL_SPEED_64MS             = 0x6,
    CNTL_SPEED_128MS            = 0x7,
    CNTL_SPEED_256MS            = 0x8,
    CNTL_SPEED_512MS            = 0x9,
    CNTL_SPEED_1024MS           = 0xA,
    CNTL_SPEED_2048MS           = 0xB,
    CNTL_UAV_TEMP               = 0xFFFF8073F007E000,
    MARKER1                     = 0xCAFEBABEFA11DEA1,
    MARKER2                     = 0xCAFEBABEFA11DEA2,
    MARKER3                     = 0xCAFEBABEFA11DEA3,
    MARKER4                     = 0xCAFEBABEFA11DEA4,
    MARKER5                     = 0xCAFEBABEFA11DEA5,
    //cntl block modes
    CNTL_MODE_MONITOR           = 0x0,
    CNTL_MODE_DEBUG1            = 0x1,
    //code version
    VERSION                     = 0x0000000100000005
};
//MASKS used to identify individual units from Unit availability vector (UAV)
enum MASKS
{
    MASK_PB     = 0x8000000000000000,
    MASK_MCS0   = 0x4000000000000000,
    MASK_MCS1   = 0x2000000000000000,
    MASK_MCS2   = 0x1000000000000000,
    MASK_MCS3   = 0x800000000000000,
    MASK_MCS4   = 0x400000000000000,
    MASK_MCS5   = 0x200000000000000,
    MASK_MCS6   = 0x100000000000000,
    MASK_MCS7   = 0x80000000000000,
    MASK_MBA0   = 0x40000000000000,
    MASK_MBA1   = 0x20000000000000,
    MASK_MBA2   = 0x10000000000000,
    MASK_MBA3   = 0x8000000000000,
    MASK_MBA4   = 0x4000000000000,
    MASK_MBA5   = 0x2000000000000,
    MASK_MBA6   = 0x1000000000000,
    MASK_MBA7   = 0x800000000000,
    MASK_Cen1   = 0x400000000000,
    MASK_Cen2   = 0x200000000000,
    MASK_Cen0   = 0x100000000000,
    MASK_Cen3   = 0x80000000000,
    MASK_Cen4   = 0x40000000000,
    MASK_Cen5   = 0x20000000000,
    MASK_Cen6   = 0x10000000000,
    MASK_Cen7   = 0x8000000000,
    MASK_XLNK0  = 0x4000000000,
    MASK_XLNK1  = 0x2000000000,
    MASK_XLNK2  = 0x1000000000,
    MASK_MCD0   = 0x800000000,
    MASK_MCD1   = 0x400000000,
    MASK_PHB0   = 0x200000000,
    MASK_PHB1   = 0x100000000,
    MASK_PHB2   = 0x80000000,
    MASK_PHB3   = 0x40000000,
    MASK_PHB4   = 0x20000000,
    MASK_PHB5   = 0x10000000,
    MASK_NX     = 0x8000000,
    MASK_CAPP0  = 0x4000000,
    MASK_CAPP1  = 0x2000000,
    MASK_VAS    = 0x1000000,
    MASK_INT    = 0x800000,
    MASK_ALNK0  = 0x400000,
    MASK_ALNK1  = 0x200000,
    MASK_ALNK2  = 0x100000,
    MASK_ALNK3  = 0x80000,
    MASK_NVLNK0 = 0x40000,
    MASK_NVLNK1 = 0x20000,
    MASK_NVLNK2 = 0x10000,
    MASK_NVLNK3 = 0x8000,
    MASK_NVLNK4 = 0x4000,
    MASK_NVLNK5 = 0x2000
};
//
//function declarations below.
//
void configure_pmu (uint8_t, uint64_t);
void post_pmu_events (int);
void initialize_postings();
void set_speed(uint64_t*, uint8_t*, volatile uint64_t*);
uint64_t get_mba_event(uint64_t, uint64_t);
uint64_t get_phb_event(uint64_t, uint64_t);
#endif //_GPE1_24x7_H
