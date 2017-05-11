/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/occ_405/lock/lock.h $                                     */
/*                                                                        */
/* OpenPOWER OnChipController Project                                     */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2011,2015                        */
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

#ifndef _LOCK_H
#define _LOCK_H

#include <occ_common.h>
#include <dimm.h>


// Release the OCC lock indefinitely
// This should be called when OCC goes into safe mode or will be reset
// to allow the host to use the specified I2C engines.
// Use PIB_I2C_ENGINE_ALL, if locks for all I2C engines should be released
void occ_i2c_lock_release(const uint8_t i_engine);


// Check and update lock ownership for the specified i2c engine
// If host has requesed lock, and there is no other outstanding interrupt
// release the lock, generate and external interrupt and return false.
// If the host has not released the lock, set ownership back to OCC and
// return true.
//
// Returns true if OCC owns the lock, or false if host owns lock
bool check_and_update_i2c_lock(const uint8_t i_engine);

#endif //_LOCK_H
