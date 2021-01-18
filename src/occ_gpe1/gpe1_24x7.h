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

#include "gpe_membuf.h"
#include "membuf_configuration.h"

//PMU config table defined below.
//------------------------------
#define TOTAL_CONFIGS  123
#define TOTAL_POSTINGS 450
#define INC_UPD_COUNT  1

#define TOTAL_CONFIG_SCOMS  123
#define TOTAL_COUNTER_SCOMS 84

#define MAX_32 4294967295ULL
#define MAX_48 281474976710655ULL

#define UNDEF 0xFF
static volatile uint64_t  G_CUR_UAV   = 0;
static volatile uint64_t  G_CUR_MODE  = 0;

//Config/pre-scaler: CNPM-10 bit + MCS-12bit + All other units - 16 bit
uint64_t G_PMU_CONFIGS_8[][2] = 
{
//cnpm//
    {0x1001181f, 0x93abaffffc000000},//scom10//0
    {0x1101181f, 0x93abaffffc000000},//scom23//1
    {0x1201181f, 0x90aa7eaaa8000000},//scom45//2
    {0x1301181f, 0x92aaabebfc000000},//scom67//3
    {0x10011820, 0xaaaaaaaa55555555},//scom10//4
    {0x11011820, 0xaaaaaaaa55555555},//scom23//5
    {0x12011820, 0xaaaaaaaa55555555},//scom45//6
    {0x13011820, 0xaaaaaaaa55555555},//scom67//7

    {0x301121a, 0x000058ffffffff06},//scom_en1//8
    {0x301125a, 0x000058ffffffff06},//scom_en2//9
    {0x301129a, 0x000058ffffffff06},//scom_en3//10
    {0x30112da, 0x000058ffffffff06},//scom_en4//11
    {0x301131a, 0x000701ffffffff86},//scom_es1//12
    {0x301135a, 0x000701ffffffff86},//scom_es2//13
    {0x301139a, 0x000701ffffffff86},//scom_es3//14
    {0x30113da, 0x000701ffffffff86},//scom_es4//15
    {0x301101a, 0x000701ffffffff86},//scom_eq0//16
    {0x301105a, 0x000701ffffffff86},//scom_eq1//17
    {0x301109a, 0x000701ffffffff86},//scom_eq2//18
    {0x30110da, 0x000701ffffffff86},//scom_eq3//19
    {0x301111a, 0x000701ffffffff86},//scom_eq4//20
    {0x301115a, 0x000701ffffffff86},//scom_eq5//21
    {0x301119a, 0x000701ffffffff86},//scom_eq6//22
    {0x30111da, 0x000701ffffffff86},//scom_eq7//23

    {0x301101f, 0x0000000600000000},//trace_eq0//24
    {0x301105f, 0x0000000600000000},//trace_eq1//25
    {0x301109f, 0x0000000600000000},//trace_eq2//26
    {0x30110df, 0x0000000600000000},//trace_eq3//27
    {0x301111f, 0x0000000600000000},//trace_eq4//28
    {0x301115f, 0x0000000600000000},//trace_eq5//29
    {0x301119f, 0x0000000600000000},//trace_eq6//30
    {0x30111df, 0x0000000600000000},//trace_eq7//31
    {0x301121f, 0x0000000600000000},//trace_en1//32
    {0x301125f, 0x0000000600000000},//trace_en2//33
    {0x301129f, 0x0000000600000000},//trace_en3//34
    {0x30112df, 0x0000000600000000},//trace_en4//35
    {0x301131f, 0x0000000600000000},//trace_es1//36
    {0x301135f, 0x0000000600000000},//trace_es2//37
    {0x301139f, 0x0000000600000000},//trace_es3//38
    {0x30113df, 0x0000000600000000},//trace_es4//39

//MC0-3//MCC0-7//
    {0xc010fa4, 0x0005755b60000000},//MCC0/1-MC0-scom-en0//40
    {0xc010fa5, 0x0000000000000000},//MCC0/1-MC0-scom-en1//41
    {0xc010fa6, 0x0000000000000000},//MCC0/1-MC0-scom-en2//42
    {0xc010fa7, 0x6000800000000000},//MCC0/1-MC0-scom-en3//43
    {0xc010c29, 0xf800000000000000},//MCC0-MC0-chan-en0//44
    {0xc010c39, 0xf800000000000000},//MCC1-MC0-chan-en1//45
    
    {0xd010fa4, 0x0000000000000005},//MCC0/1-MC1-scom-en0//46
    {0xd010fa5, 0x755b600000000000},//MCC0/1-MC1-scom-en1//47
    {0xd010fa6, 0x0000000000000000},//MCC0/1-MC1-scom-en2//48
    {0xd010fa7, 0x0600800000000000},//MCC0/1-MC1-scom-en3//49
    {0xd010c29, 0xf800000000000000},//MCC0-MC1-chan-en0//50
    {0xd010c39, 0xf800000000000000},//MCC1-MC1-chan-en1//51
    
    {0xe010fa4, 0x0005755b60000000},//MCC0/1-MC2-scom-en0//52
    {0xe010fa5, 0x0000000000000000},//MCC0/1-MC2-scom-en1//53
    {0xe010fa6, 0x0000000000000000},//MCC0/1-MC2-scom-en2//54
    {0xe010fa7, 0x6000800000000000},//MCC0/1-MC2-scom-en3//55
    {0xe010c29, 0xf800000000000000},//MCC0-MC2-chan-en0//56
    {0xe010c39, 0xf800000000000000},//MCC1-MC2-chan-en1//57
    
    {0xf010fa4, 0x0000000000000005},//MCC0/1-MC3-scom-en0//58
    {0xf010fa5, 0x755b600000000000},//MCC0/1-MC3-scom-en1//59
    {0xf010fa6, 0x0000000000000000},//MCC0/1-MC3-scom-en2//60
    {0xf010fa7, 0x0600800000000000},//MCC0/1-MC3-scom-en3//61
    {0xf010c29, 0xf800000000000000},//MCC0-MC3-chan-en0//62
    {0xf010c39, 0xf800000000000000},//MCC1-MC3-chan-en1//63

//PEC0-1//
    {0x3011804, 0x1000780000000000},//pec0//64
    {0x2011804, 0x1000780000000000},//pec1//65

//TLPM0-7:Xlink/Alink//
    {0x1001181a, 0x9003015754000000},//tl-0-scom10//66
    {0x10011820, 0xaaaaaaaa55555555},//tl-0-scom10//67
    {0x1001182c, 0x1c0428d00852ad1c},//tl-0-scom10//68
    {0x1001182d, 0x0214680429568000},//tl-0-scom10//69
    {0x1101181a, 0x9003015754000000},//tl-1-scom23//70
    {0x11011820, 0xaaaaaaaa55555555},//tl-1-scom23//71
    {0x1101182c, 0x1c0428d00852ad1c},//tl-1-scom23//72
    {0x1101182d, 0x0214680429568000},//tl-1-scom23//73
    {0x1201181a, 0x9003015754000000},//tl-2-scom45//74
    {0x12011820, 0xaaaaaaaa55555555},//tl-2-scom45//75
    {0x1201182c, 0x1c0428d00852ad1c},//tl-2-scom45//76
    {0x1201182d, 0x0214680429568000},//tl-2-scom45//77
    {0x1301181a, 0x9003015754000000},//tl-3-scom67//78
    {0x13011820, 0xaaaaaaaa55555555},//tl-3-scom67//79
    {0x1301182c, 0x1c0428d00852ad1c},//tl-3-scom67//80
    {0x1301182d, 0x0214680429568000},//tl-3-scom67//81

//TMPM0-7:OCAPI//
    {0x10010a22, 0xb055d08081128100},//tl-0-ocapi//82
    {0x10010a23, 0x0000000000000000},//tl-0-ocapi//83
    {0x10010a52, 0xb055d08081128100},//tl-0-ocapi//84
    {0x10010a53, 0x0000000000000000},//tl-0-ocapi//85
    {0x11010a22, 0xb055d08081128100},//tl-3-ocapi//86
    {0x11010a23, 0x0000000000000000},//tl-3-ocapi//87
    {0x11010a52, 0xb055d08081128100},//tl-3-ocapi//88
    {0x11010a53, 0x0000000000000000},//tl-3-ocapi//89
    {0x12010a22, 0xb055d08081128100},//tl-4-ocapi//90
    {0x12010a23, 0x0000000000000000},//tl-4-ocapi//91
    {0x12010a52, 0xb055d08081128100},//tl-4-ocapi//92
    {0x12010a53, 0x0000000000000000},//tl-4-ocapi//93
    {0x12011222, 0xb055d08081128100},//tl-5-ocapi//94
    {0x12011223, 0x0000000000000000},//tl-5-ocapi//95
    {0x12011252, 0xb055d08081128100},//tl-5-ocapi//96
    {0x12011253, 0x0000000000000000},//tl-5-ocapi//97
    {0x13010a22, 0xb055d08081128100},//tl-6-ocapi//98
    {0x13010a23, 0x0000000000000000},//tl-6-ocapi//99
    {0x13010a52, 0xb055d08081128100},//tl-6-ocapi//100
    {0x13010a53, 0x0000000000000000},//tl-6-ocapi//101
    {0x13011222, 0xb055d08081128100},//tl-7-ocapi//102
    {0x13011223, 0x0000000000000000},//tl-7-ocapi//103
    {0x13011252, 0xa055d08081128100},//tl-7-ocapi//104
    {0x13011253, 0x0000000000000000},//tl-7-ocapi//105

//PHB0-5//
    {0x8010918, 0x1616010119190101},//phb-0//106
    {0x8010917, 0x8000550000000000},//phb-0//107
    {0x8010958, 0x1616010119190101},//phb-1//108
    {0x8010957, 0x8000550000000000},//phb-1//109
    {0x8010998, 0x1616010119190101},//phb-2//110
    {0x8010997, 0x8000550000000000},//phb-2//111
    {0x9010918, 0x1616010119190101},//phb-3//112
    {0x9010917, 0x8000550000000000},//phb-3//113
    {0x9010958, 0x1616010119190101},//phb-4//114
    {0x9010957, 0x8000550000000000},//phb-4//115
    {0x9010998, 0x1616010119190101},//phb-5//116
    {0x9010997, 0x8000550000000000},//phb-5//117

//OCMB//
    {0x8011440, 0x0451805515000000},//ocmb//118

//NX//
    {0x20110a6, 0x855a300080000000},//PMU-0 //119
    {0x2011054, 0x0000000000000000},//PMON-0//120
    {0x20110a9, 0x8550492480000000},//PMU-1 //121
    {0x2011055, 0x0000000060000000} //PMON-1//122
};


//------------------
//PMU counters below
//------------------
//------------------------- CNPM ---------------------------//
uint64_t G_PMULETS_1A[] =
{
//Group 1A
//CNPM counters (4 SE and 4 NE)
    0x10011821,//0//0
    0x10011822,//1//1
    0x10011823,//2//2
    0x10011824,//3//3
    0x11011821,//4//4
    0x11011822,//5//5
    0x11011823,//6//6
    0x11011824,//7//7
};

uint64_t G_PMULETS_1B[] =
{
//Group 1B
//CNPM counters (4 SW and 4 NW)
    0x12011821,//0//0
    0x12011822,//1//1
    0x12011823,//2//2
    0x12011824,//3//3
    0x13011821,//4//4
    0x13011822,//5//5
    0x13011823,//6//6
    0x13011824,//7//7
};

//------------------------- Xlink/Alink ---------------------------//
uint64_t G_PMULETS_2A[] =
{
//Group 2A
//Xlink/Alink counters (4 SE and 4 NE)
    0x1001181b,//0//0
    0x1001181c,//0//1
    0x1001181d,//1//2
    0x1001181e,//1//3
    0x1101181b,//2//4
    0x1101181c,//2//5
    0x1101181d,//3//6
    0x1101181e,//3//7
};

uint64_t G_PMULETS_2B[] =
{
//Group 2B
//Xlink/Alink counters (4 SW and 4 NW)
    0x1201181b,//4//0
    0x1201181c,//4//1
    0x1201181d,//5//2
    0x1201181e,//5//3
    0x1301181b,//6//4
    0x1301181c,//6//5
    0x1301181d,//7//6
    0x1301181e,//7//7
};

//------------------------- OCAPI ---------------------------//
uint64_t G_PMULETS_3A[] =
{
//Group 3A
//OCAPI counters - [0:3]
    0x10010a25,//OTL0_0
    0x10010a55,//OTL0_1
    0x11010a25,//OTL3_0
    0x11010a55,//OTL3_1
    0x12010a25,//OTL4_0
    0x12010a55,//OTL4_1
    0x12011225,//OTL5_0
    0x12011255,//OTL5_1
};

uint64_t G_PMULETS_3B[] =
{
//Group 3B
//OCAPI counters - [4:5]
    0x13010a25,//OTL6_0
    0x13010a55,//OTL6_1
    0x13011225,//OTL7_0
    0x13011255,//OTL7_1
};

//------------------------- PHB ---------------------------//
uint64_t G_PMULETS_4[] =
{
    0x8010919,//PHB0//
    0x8010959,//PHB1//
    0x8010999,//PHB2//
    0x9010919,//PHB3//
    0x9010959,//PHB4//
    0x9010999,//PHB5/

    0x20110a7,//NX PMU-0//
    0x20110aa,//NX PMU-1//
    0x20110a6,//NX PMU-0 control reg//
    0x20110a9,//NX PMU-1 control reg//
};

//------------------------- OCMB ---------------------------//
uint64_t G_PMULETS_5678[] =
{//OCMB0-15
    0x801143f,//0-15
};


/**
 * Groups 
 **/
enum groups {G1A=1,G1B=2,G2A=3,G2B=4,G3A=5,G3B=6,G4=7,G5=8,G6=9,G7=10,G8=11};

enum
{
    ZERO                        = 0,
    PBA_ENABLE                  = 0x80000000,
    TEST_ADDR                   = 0x00180110,
    PBASLVCTL1_C0040028         = 0xC0040028,
    PBASLVCTL2_C0040030         = 0xC0040030,
    PBASLV_SET_DMA              = 0x97005EC000000000,
    PBASLV_SET_ATOMIC           = 0x97405EC060000000,

    //POSTING OFFSETS for groups
    //--------------------------
    POSTING_START               = 0x00180000,//1MB + 512KB

    //------------------------- CNPM ---------------------------//
    // Group G1A - CNPM counters
    POST_OFFSET_G1AH             = 0x00180000,
    POST_OFFSET_G1AT             = 0x00180108,

    // Group G1B - CNPM counters 
    POST_OFFSET_G1BH             = 0x00180110,
    POST_OFFSET_G1BT             = 0x00180218,

    //------------------------- Xlink ---------------------------//
    // Group G2A - Xlink
    POST_OFFSET_G2A_X_H         = 0x00180220,
    POST_OFFSET_G2A_X_0         = 0x00180228,
    POST_OFFSET_G2A_X_1         = 0x00180268,
    POST_OFFSET_G2A_X_2         = 0x001802a8,
    POST_OFFSET_G2A_X_3         = 0x001802e8,
    POST_OFFSET_G2A_X_T         = 0x00180328,
    // Group G2B - Xlink
    POST_OFFSET_G2B_X_H         = 0x00180330,
    POST_OFFSET_G2B_X_4         = 0x00180338,
    POST_OFFSET_G2B_X_5         = 0x00180378,
    POST_OFFSET_G2B_X_6         = 0x001803b8,
    POST_OFFSET_G2B_X_7         = 0x001803f8,
    POST_OFFSET_G2B_X_T         = 0x00180438,

    //------------------------- Alink ---------------------------//
    // Group G2A - Alink
    POST_OFFSET_G2A_A_H         = 0x00180440,
    POST_OFFSET_G2A_A_0         = 0x00180448,
    POST_OFFSET_G2A_A_1         = 0x00180488,
    POST_OFFSET_G2A_A_2         = 0x001804c8,
    POST_OFFSET_G2A_A_3         = 0x00180508,
    POST_OFFSET_G2A_A_T         = 0x00180548,
    // Group G2B - Alink
    POST_OFFSET_G2B_A_H         = 0x00180550,
    POST_OFFSET_G2B_A_4         = 0x00180558,
    POST_OFFSET_G2B_A_5         = 0x00180598,
    POST_OFFSET_G2B_A_6         = 0x001805d8,
    POST_OFFSET_G2B_A_7         = 0x00180618,
    POST_OFFSET_G2B_A_T         = 0x00180658,

    //------------------------- OCAPI ---------------------------//
    // Group G3A - OCAPI
    POST_OFFSET_G3A_O_H         = 0x00180660,
    POST_OFFSET_G3A_O_0         = 0x00180668,
    POST_OFFSET_G3A_O_3         = 0x001806a8,
    POST_OFFSET_G3A_O_4         = 0x001806e8,
    POST_OFFSET_G3A_O_5         = 0x00180728,
    POST_OFFSET_G3A_O_T         = 0x00180768,
    // Group G3B - OCAPI
    POST_OFFSET_G3B_O_H         = 0x00180770,
    POST_OFFSET_G3B_O_6         = 0x00180778,
    POST_OFFSET_G3B_O_7         = 0x001807b8,
    POST_OFFSET_G3B_O_T         = 0x00180878,

    //------------------------- PHB & NX -----------------------//
    // Group G4 - PHB 0-5
    POST_OFFSET_G4H             = 0x00180880,
    POST_OFFSET_G4_0            = 0x00180888,
    POST_OFFSET_G4_1            = 0x001808a8,
    POST_OFFSET_G4_2            = 0x001808c8,
    POST_OFFSET_G4_3            = 0x001808e8,
    POST_OFFSET_G4_4            = 0x00180908,
    POST_OFFSET_G4_5            = 0x00180928,
    // Group G4 - NX
    POST_OFFSET_G4_6            = 0x00180948,
    POST_OFFSET_G4_7            = 0x00180968,
    POST_OFFSET_G4T             = 0x00180988,

    //------------------------- OCMB ---------------------------//
    // Group G5 - OCMB 0-3
    POST_OFFSET_G5H             = 0x00180990,
    POST_OFFSET_G5_0            = 0x00180998,
    POST_OFFSET_G5_1            = 0x001809b8,
    POST_OFFSET_G5_2            = 0x001809d8,
    POST_OFFSET_G5_3            = 0x001809f8,
    POST_OFFSET_G5T             = 0x00180a98,

    // Group G6 - OCMB 4-7
    POST_OFFSET_G6H             = 0x00180aa0,
    POST_OFFSET_G6_4            = 0x00180aa8,
    POST_OFFSET_G6_5            = 0x00180ac8,
    POST_OFFSET_G6_6            = 0x00180ae8,
    POST_OFFSET_G6_7            = 0x00180b08,
    POST_OFFSET_G6T             = 0x00180ba8,

    // Group G7 - OCMB 8-11
    POST_OFFSET_G7H             = 0x00180bb0,
    POST_OFFSET_G7_8            = 0x00180bb8,
    POST_OFFSET_G7_9            = 0x00180bd8,
    POST_OFFSET_G7_10           = 0x00180bf8,
    POST_OFFSET_G7_11           = 0x00180c18,
    POST_OFFSET_G7T             = 0x00180cb8,

    // Group G8 - OCMB 12-15
    POST_OFFSET_G8H             = 0x00180cc0,
    POST_OFFSET_G8_12           = 0x00180cc8,
    POST_OFFSET_G8_13           = 0x00180ce8,
    POST_OFFSET_G8_14           = 0x00180d08,
    POST_OFFSET_G8_15           = 0x00180d28,
    POST_OFFSET_G8T             = 0x00180dc8,

    //------------------------- Debug ---------------------------//
    //DEBUG mode offsets start here
    POSTING_START_DBG           = 0x00190000,
    POST_OFFSET_DBG1AH          = 0x00190000,
    POST_OFFSET_DBG1AT          = 0x00190108,
    POST_OFFSET_DBG1BH          = 0x00190110,
    POST_OFFSET_DBG1BT          = 0x00190218,

    //------------------------ Control Block ---------------------//
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
    MARKER1                     = 0xCAFEBABEFA11DEA1,
    MARKER2                     = 0xCAFEBABEFA11DEA2,
    MARKER3                     = 0xCAFEBABEFA11DEA3,
    MARKER4                     = 0xCAFEBABEFA11DEA4,
    MARKER5                     = 0xCAFEBABEFA11DEA5,
    //cntl block modes
    CNTL_MODE_MONITOR           = 0x0,
    CNTL_MODE_DEBUG1            = 0x1,
    //code version
    //VERSION                     = 0x0000000100000005
};


//MASKS used to identify individual units from Unit availability vector (UAV)
enum MASKS
{
    MASK_PB      = 0x8000000000000000,
    MASK_MC00    = 0x4000000000000000,
    MASK_MC01    = 0x2000000000000000,
    MASK_MC10    = 0x1000000000000000,
    MASK_MC11    = 0x0800000000000000,
    MASK_MC20    = 0x0400000000000000,
    MASK_MC21    = 0x0200000000000000,
    MASK_MC30    = 0x0100000000000000,
    MASK_MC31    = 0x0080000000000000,
    MASK_PEC0    = 0x0040000000000000,
    MASK_PEC1    = 0x0020000000000000,
    MASK_TLPM0   = 0x001C000000000000,
    MASK_TLPM1   = 0x0003800000000000,
    MASK_TLPM2   = 0x0000700000000000,
    MASK_TLPM3   = 0x00000E0000000000,
    MASK_TLPM4   = 0x000001C000000000,
    MASK_TLPM5   = 0x0000003800000000,
    MASK_TLPM6   = 0x0000000700000000,
    MASK_TLPM7   = 0x00000000E0000000,
    MASK_XLINK0  = 0x0014000000000000,
    MASK_XLINK1  = 0x0002800000000000,
    MASK_XLINK2  = 0x0000500000000000,
    MASK_XLINK3  = 0x00000A0000000000,
    MASK_XLINK4  = 0x0000014000000000,
    MASK_XLINK5  = 0x0000002800000000,
    MASK_XLINK6  = 0x0000000500000000,
    MASK_XLINK7  = 0x00000000A0000000,
    MASK_ALINK0  = 0x0018000000000000,
    MASK_ALINK1  = 0x0003000000000000,
    MASK_ALINK2  = 0x0000600000000000,
    MASK_ALINK3  = 0x00000C0000000000,
    MASK_ALINK4  = 0x0000018000000000,
    MASK_ALINK5  = 0x0000003000000000,
    MASK_ALINK6  = 0x0000000600000000,
    MASK_ALINK7  = 0x00000000C0000000,
    MASK_OCAPI0  = 0x001C000000000000,
    MASK_OCAPI1  = 0x0003800000000000,
    MASK_OCAPI2  = 0x0000700000000000,
    MASK_OCAPI3  = 0x00000E0000000000,
    MASK_OCAPI4  = 0x000001C000000000,
    MASK_OCAPI5  = 0x0000003800000000,
    MASK_OCAPI6  = 0x0000000700000000,
    MASK_OCAPI7  = 0x00000000E0000000,
    MASK_PHB0    = 0x0000000010000000,
    MASK_PHB1    = 0x0000000008000000,
    MASK_PHB2    = 0x0000000004000000,
    MASK_PHB3    = 0x0000000002000000,
    MASK_PHB4    = 0x0000000001000000,
    MASK_PHB5    = 0x0000000000800000,
    MASK_PHB     = (MASK_PHB0 | MASK_PHB1 | MASK_PHB2 | MASK_PHB3 | MASK_PHB4 | MASK_PHB5),
    MASK_OCMB0   = 0x0000000000400000,
    MASK_OCMB1   = 0x0000000000200000,
    MASK_OCMB2   = 0x0000000000100000,
    MASK_OCMB3   = 0x0000000000080000,
    MASK_OCMB4   = 0x0000000000040000,
    MASK_OCMB5   = 0x0000000000020000,
    MASK_OCMB6   = 0x0000000000010000,
    MASK_OCMB7   = 0x0000000000008000,
    MASK_OCMB8   = 0x0000000000004000,
    MASK_OCMB9   = 0x0000000000002000,
    MASK_OCMB10  = 0x0000000000001000,
    MASK_OCMB11  = 0x0000000000000800,
    MASK_OCMB12  = 0x0000000000000400,
    MASK_OCMB13  = 0x0000000000000200,
    MASK_OCMB14  = 0x0000000000000100,
    MASK_OCMB15  = 0x0000000000000080,
    MASK_OCMB    = 0x00000000007FFF80,
    MASK_NX      = 0x0000000000000040
};

//
//version structure
//
typedef union version {
    uint64_t value;
    struct {
        uint64_t  major:4;
        uint64_t  minor:8;
        uint64_t  bugfix:4;
        uint64_t  day:8;
        uint64_t  month:8;
        uint64_t  year:16;
        uint64_t  spec_major:6;
        uint64_t  spec_minor:2;
        uint64_t  reserved:8;
    } val;
} version_t;

//
//function declarations below.
//
uint32_t getScom (const uint32_t i_addr, uint64_t* o_data, GpeErrorStruct* o_err);
uint32_t putScom (const uint32_t i_addr, uint64_t i_data, GpeErrorStruct* o_err);
uint32_t configure_pmu (uint8_t i_state, uint64_t i_speed, GpeErrorStruct* o_err);
uint32_t post_pmu_events (int group, GpeErrorStruct* o_err);
uint32_t initialize_postings (GpeErrorStruct* o_err);
uint32_t set_speed (uint64_t* i_speed, uint8_t* i_delay, volatile uint64_t* o_status,
                    GpeErrorStruct* o_err);

#endif //_GPE1_24x7_H
