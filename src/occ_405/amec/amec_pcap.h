/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/amec/amec_pcap.h $                                */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2016                        */
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

#ifndef _AMEC_PCAP_H
#define _AMEC_PCAP_H

//*************************************************************************
// Includes
//*************************************************************************
#include <occ_common.h>
#include <errl.h>

//*************************************************************************
// Externs
//*************************************************************************

//*************************************************************************
// Macros
//*************************************************************************

//*************************************************************************
// Defines/Enums
//*************************************************************************

//Number of watts power must be below the node power cap before raising
//ppb_fmax
#define PDROP_THRESH        0   //TODO: need better value.

//Structure used in g_amec
typedef struct amec_pcap
{
    uint16_t ovs_node_pcap;     //Oversub node power cap in 1W units
    uint16_t norm_node_pcap;    //Normal node power cap in 1W units
    uint16_t active_node_pcap;  //Currently active node power cap in 1W units
    uint16_t active_proc_pcap;  //Currently active proc power cap in 1W units
    uint16_t nominal_mem_pwr;   //Memory power in 1W units for nominal throttles
    uint16_t turbo_mem_pwr;     //Memory power in 1W units for turbo throttles
    uint16_t pcap1_mem_pwr;     //Memory power in 1W units for power capping level 1 throttles
    uint8_t active_mem_level;   //Currently active memory throttles pcap level 0=no active mem pcap
} amec_pcap_t;

//*************************************************************************
// Structures
//*************************************************************************

//*************************************************************************
// Globals
//*************************************************************************

//*************************************************************************
// Function Prototypes
//*************************************************************************

//*************************************************************************
// Functions
//*************************************************************************

// Calls all power control algorithms
void amec_power_control();

#endif

