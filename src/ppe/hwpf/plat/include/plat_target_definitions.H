/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ppe/hwpf/plat/include/plat_target_definitions.H $         */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2012,2015                        */
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
/**
 * @file plat_ppe_target.H
 * @brief Definitions for fapi2 PPE targets
 */

#ifndef __FAPI2_PPE_TARGET__
#define __FAPI2_PPE_TARGET__

#define TARGET_CHIP(_name, _index) \
            fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP > _name((uint64_t)_index);

#define TARGET_CHIP_PERV(_name, _index) \
            fapi2::Target<fapi2::TARGET_TYPE_PERV> _name((uint64_t)_index);

#define TARGET_EQ(_name, _index) \
            fapi2::Target<fapi2::TARGET_TYPE_EQ> _name((uint64_t)_index);

#define TARGET_CORE(_name, _index) \
            fapi2::Target<fapi2::TARGET_TYPE_CORE> _name((uint64_t)_index);
            
#define TARGET_EX(_name, _index) \
            fapi2::Target<fapi2::TARGET_TYPE_EX> _name((uint64_t)_index);

namespace fapi2 
{

           TARGET_CHIP         (chip_target,   0);       
           TARGET_CHIP_PERV    (perv_target,   1);
           TARGET_CHIP_PERV    (n0_target,     2);
           TARGET_CHIP_PERV    (n1_target,     3);
           TARGET_CHIP_PERV    (n2_target,     4);
           TARGET_CHIP_PERV    (n3_target,     5);
           TARGET_CHIP_PERV    (xb_target,     6);
           TARGET_CHIP_PERV    (mc0_target,    7);
           TARGET_CHIP_PERV    (mc1_target,    8);
           TARGET_CHIP_PERV    (ob0_target,    9);
           TARGET_CHIP_PERV    (ob1_target,   10);
           TARGET_CHIP_PERV    (ob2_target,   11);
           TARGET_CHIP_PERV    (ob3_target,   12);
           TARGET_CHIP_PERV    (pci0_target,  13);
           TARGET_CHIP_PERV    (pci1_target,  14);  
           TARGET_CHIP_PERV    (pci2_target,  15);
           TARGET_EQ           (eq0_target,    0); 
           TARGET_EQ           (eq1_target,    1);  
           TARGET_EQ           (eq2_target,    2); 
           TARGET_EQ           (eq3_target,    3);  
           TARGET_EQ           (eq4_target,    4); 
           TARGET_EQ           (eq5_target,    5);   
           TARGET_EX           (ex0_target,    0);                
           TARGET_EX           (ex1_target,    1);                
           TARGET_EX           (ex2_target,    2);                
           TARGET_EX           (ex3_target,    3);                
           TARGET_EX           (ex4_target,    4);                
           TARGET_EX           (ex5_target,    5);                
           TARGET_EX           (ex6_target,    6);                
           TARGET_EX           (ex7_target,    7);                
           TARGET_EX           (ex8_target,    8);                
           TARGET_EX           (ex9_target,    9);                
           TARGET_EX           (ex10_target,  10);                
           TARGET_EX           (ex11_target,  11);                
           TARGET_CORE         (core0_target,  0);
           TARGET_CORE         (core1_target,  1);
           TARGET_CORE         (core2_target,  2);
           TARGET_CORE         (core3_target,  3);
           TARGET_CORE         (core4_target,  4);
           TARGET_CORE         (core5_target,  5);
           TARGET_CORE         (core6_target,  6);
           TARGET_CORE         (core7_target,  7);
           TARGET_CORE         (core8_target,  8);
           TARGET_CORE         (core9_target,  9);
           TARGET_CORE         (core10_target,10);
           TARGET_CORE         (core11_target,11);
           TARGET_CORE         (core12_target,12);
           TARGET_CORE         (core13_target,13);
           TARGET_CORE         (core14_target,14);
           TARGET_CORE         (core15_target,15);
           TARGET_CORE         (core16_target,16);
           TARGET_CORE         (core17_target,17);
           TARGET_CORE         (core18_target,18);
           TARGET_CORE         (core19_target,19);
           TARGET_CORE         (core20_target,20);
           TARGET_CORE         (core21_target,21);
           TARGET_CORE         (core22_target,22);
           TARGET_CORE         (core23_target,23); 

};  // fapi2 namespace

#endif  // __FAPI2_PPE_TARGET__
