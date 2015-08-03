/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/common/rand.h $                                       */
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
#ifndef __RAND_H__
#define __RAND_H__

// $Id$

/// \file rand.h
/// \brief Random number generation

#include <stdint.h>

// Error/Panic codes

#define RAND64_UNIMPLEMENTED 0x00726301


/// RAND32_ALL is used as the \a limit argument to rand32() and _rand32() to
/// request the return of a full 32-bit random unsigned integer.

#define RAND32_ALL 0

extern uint32_t _seed32;

uint32_t 
_rand32(uint32_t *seed, uint32_t limit);

uint32_t
rand32(uint32_t limit);

void
srand32(uint32_t seed);


/// RAND64_ALL is used as the \a limit argument to rand64() and _rand64() to
/// request the return of a full 64-bit random unsigned integer.

#define RAND64_ALL 0

extern uint64_t _seed64;

//void
//davidmult64to128(uint64_t u, uint64_t v, uint64_t &h, uint64_t &l);

uint64_t 
_rand64(uint64_t *seed, uint64_t limit);

uint64_t
rand64(uint64_t limit);

void
srand64(uint64_t seed);


/// A random weighting map for integer selections
///
/// See rand_map_int() for details.

typedef struct {

    /// The relative weight of this selection
    ///
    /// The final weight of the map array \e must be 0 to terminate the map.
    unsigned weight;

    /// The selection
    int selection;

} RandMapInt;


/// A random weighting map for pointer selections
///
/// See rand_map_ptr() for details.

typedef struct {

    /// The relative weight of this selection
    ///
    /// The final weight of the map array \e must be 0 to terminate the map.
    unsigned weight;

    /// The selection
    void *selection;

} RandMapPtr;


int
_rand_map_int(uint32_t *seed, RandMapInt *map, int *index);

int
rand_map_int(RandMapInt *map);

void *
_rand_map_ptr(uint32_t *seed, RandMapPtr *map, int *index);

void *
rand_map_ptr(RandMapPtr *map);


#endif	/* __RAND_H__ */
