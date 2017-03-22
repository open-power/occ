/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_gpe1/gpe1_memory_power_control.h $                    */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015,2016                        */
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

#ifndef _GPE1_MEMORY_POWER_CONTROL_H
#define _GPE1_MEMORY_POWER_CONTROL_H

#define PCR0_MASTER_ENABLE_BIT           2
#define PCR0_POWERDOWN_ENABLE_BIT        22

#define STR0_STR_ENABLE_BIT              0
#define STR0_DISABLE_MEMORY_CLOCKS_BIT   1

// Big Endian set/clear bit MACROS
#define SET_BIT(var, bit)           (var |  (0x8000000000000000 >> bit) )

#define CLR_BIT(var, bit)           (var & ~(0x8000000000000000 >> bit) )


// Big Endian set/clear 2 different bits MACROS
#define SET_2BITS(var, bit1, bit2)  SET_BIT(SET_BIT(var, bit1), bit2)

#define CLR_2BITS(var, bit1, bit2)  CLR_BIT(CLR_BIT(var, bit1), bit2)


#endif // _GPE1_MEMORY_POWER_CONTROL_H
