/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/ssx/pgp/registers/centaur_register_addresses.h $          */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2014,2016                        */
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
#ifndef __CENTAUR_REGISTER_ADDRESSES_H__
#define __CENTAUR_REGISTER_ADDRESSES_H__

/// \file centaur_register_addresses.h
/// \brief Symbolic addresses for the CENTAUR unit

// *** WARNING *** - This file is generated automatically, do not edit.


#define CENTAUR_PIB_BASE 0
#define CENTAUR_DEVICE_ID 0x000f000f
#define CENTAUR_MBS_FIR_REG 0x02011400
#define CENTAUR_MBS_FIR_REG_AND 0x02011401
#define CENTAUR_MBS_FIR_REG_OR 0x02011402
#define CENTAUR_MBS_FIR_MASK_REG 0x02011403
#define CENTAUR_MBS_FIR_MASK_REG_AND 0x02011404
#define CENTAUR_MBS_FIR_MASK_REG_OR 0x02011405
#define CENTAUR_MBS_FIR_ACTION0_REG 0x02011406
#define CENTAUR_MBS_FIRACT1 0x02011407
#define CENTAUR_MBSCFGQ 0x02011411
#define CENTAUR_MBSEMERTHROQ 0x0201142d
#define CENTAUR_MBSOCC01HQ 0x02011429
#define CENTAUR_MBSOCC23HQ 0x0201142a
#define CENTAUR_MBSOCCITCQ 0x02011428
#define CENTAUR_MBSOCCSCANQ 0x0201142b
#define CENTAUR_MBARPC0QN(n) (CENTAUR_MBARPC0Q0 + ((CENTAUR_MBARPC0Q1 - CENTAUR_MBARPC0Q0) * (n)))
#define CENTAUR_MBARPC0Q0 0x03010434
#define CENTAUR_MBARPC0Q1 0x03010c34
#define CENTAUR_MBA_FARB3QN(n) (CENTAUR_MBA_FARB3Q0 + ((CENTAUR_MBA_FARB3Q1 - CENTAUR_MBA_FARB3Q0) * (n)))
#define CENTAUR_MBA_FARB3Q0 0x03010416
#define CENTAUR_MBA_FARB3Q1 0x03010c16
#define CENTAUR_PMU0QN(n) (CENTAUR_PMU0Q0 + ((CENTAUR_PMU0Q1 - CENTAUR_PMU0Q0) * (n)))
#define CENTAUR_PMU0Q0 0x03010437
#define CENTAUR_PMU0Q1 0x03010c37
#define CENTAUR_PMU1QN(n) (CENTAUR_PMU1Q0 + ((CENTAUR_PMU1Q1 - CENTAUR_PMU1Q0) * (n)))
#define CENTAUR_PMU1Q0 0x03010438
#define CENTAUR_PMU1Q1 0x03010c38
#define CENTAUR_PMU2QN(n) (CENTAUR_PMU2Q0 + ((CENTAUR_PMU2Q1 - CENTAUR_PMU2Q0) * (n)))
#define CENTAUR_PMU2Q0 0x03010439
#define CENTAUR_PMU2Q1 0x03010c39
#define CENTAUR_PMU3QN(n) (CENTAUR_PMU3Q0 + ((CENTAUR_PMU3Q1 - CENTAUR_PMU3Q0) * (n)))
#define CENTAUR_PMU3Q0 0x0301043a
#define CENTAUR_PMU3Q1 0x03010c3a
#define CENTAUR_PMU4QN(n) (CENTAUR_PMU4Q0 + ((CENTAUR_PMU4Q1 - CENTAUR_PMU4Q0) * (n)))
#define CENTAUR_PMU4Q0 0x0301043b
#define CENTAUR_PMU4Q1 0x03010c3b
#define CENTAUR_PMU5QN(n) (CENTAUR_PMU5Q0 + ((CENTAUR_PMU5Q1 - CENTAUR_PMU5Q0) * (n)))
#define CENTAUR_PMU5Q0 0x0301043c
#define CENTAUR_PMU5Q1 0x03010c3c
#define CENTAUR_PMU6QN(n) (CENTAUR_PMU6Q0 + ((CENTAUR_PMU6Q1 - CENTAUR_PMU6Q0) * (n)))
#define CENTAUR_PMU6Q0 0x0301043d
#define CENTAUR_PMU6Q1 0x03010c3d
#define CENTAUR_SENSOR_CACHE_DATA0_3 0x020115ca
#define CENTAUR_SENSOR_CACHE_DATA4_7 0x020115cb
#define CENTAUR_DTS_THERMAL_SENSOR_RESULTS 0x02050000

#endif // __CENTAUR_REGISTER_ADDRESSES_H__

