/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/common/rand32.c $                                     */
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
// $Id$

/// \file rand32.c
/// \brief 32-bit unsigned pseudo-random number generation

#include "rand.h"

/// The default seed for rand32()

uint32_t _seed32 = 405405405;


/// Generate a random 32-bit unsigned integer from an explicit seed
///
/// \param seed A pointer to the random seed (updated by this routine).
///
/// \param limit The (exclusive) upper bound of the range of generated random
/// integers.  The (inclusive) lower bound is always 0.
///
/// \retval A pseudo-random unsigned 32-bit integer uniformly selected from
/// the range 0 to \a limit - 1 (inclusive). However if the \a limit parameter
/// is \c RAND32_ALL (0), then the return value is a full 32-bit random bit
/// vector. 
///
/// This is a 32-bit linear congruential generator, taken from the 'ranqd1'
/// generator from "Numerical Recipes in C".  The authors' only praise for
/// this generator is that it is "\e very fast"; the quality of random numbers
/// is deemed "entirely adequate for many uses".
///
/// The initial 32 pseudo-random result is treated as a 32-bit binary fraction
/// that is multipled by the limit to yield the final random 32-bit
/// integer. If the limit is 0, then the full 32-bit result is returned. As
/// with all LCG, do not count on the low-order bits to be particularly
/// random.

uint32_t
_rand32(uint32_t *seed, uint32_t limit)
{
    uint64_t x;

    *seed = (*seed * 1664525) + 1013904223;
    if (limit == RAND32_ALL) {
	return *seed;
    } else {
	x = (uint64_t)(*seed) * limit;
	return x >> 32;
    }
}


/// Generate a random 32-bit unsigned integer from a system-wide seed
///
/// \param limit The (exclusive) upper bound of the range of generated random
/// integers. The (inclusive) lower bound is always 0.
///
/// \retval A pseudo-random unsigned 32-bit integer uniformly selected from
/// the range 0 to \a limit - 1 (inclusive). However if the \a limit parameter
/// is \c RAND32_ALL (0), then the return value is a full 32-bit random bit
/// vector. 
///
/// rand32() is not thread safe.  There is a small possibility that multiple
/// threads may observe the same random numbers, and it is also possible that
/// the random sequence may appear to repeat due to thread interactions.  If
/// these are concerns then the application should either call rand32() from
/// within a critical section, or provide a unique seed to each thread or
/// process and use the underlying _rand32() API explicitly.

uint32_t
rand32(uint32_t limit)
{
    return _rand32(&_seed32, limit);
}


/// Set the global random seed for rand32()

void
srand32(uint32_t seed)
{
    _seed32 = seed;
}


/// Select an integer from a weighted distribution using a specific seed
///
/// \param seed A 32-bit unsigned random seed (accumulator)
///
/// \param map An array of RandMapInt structures, the final element of which
/// must have the \a weight field = 0.  This array will typically be allocated
/// statically. 
///
/// \param index An optional pointer to an integer which will recieve the
/// index of the item selected.  NULL \a index are ignored. This is provided
/// for appplications that require statistics on selections.
///
/// \retval One of the \a selection from the array with a non-0 \a weight.  If
/// the weight array is NULL (= {{0, \<dont care\>}}), then by convention the
/// return value is 0, and the return index is -1.
///
/// This routine selects items from the \a map randomly, given the weighting
/// implied by (map[i].weight / SUM(i = 0,...,N, map[i].weight)).  For
/// example, the following two maps are equivalent in that they select 'a' and
/// 'c' with 25% probability, and 'b' with 50% probability:
///
/// RandMapInt map0[] = {{1, 'a'}, {2, 'b'}, {1, 'c'}, {0, 0}};
///
/// RandMapInt map1[] = {{25, 'a'}, {50, 'b'}, {25, 'c'}, {0, 0}};
///
/// Note that several errors including negative weights, or the overflow of
/// the sum of weights as an \a unsigned number are neither detected nor
/// reported.
///
/// \todo We could probably merge the code for the integer and pointer versions
/// somewhat.  This is a great example of where C++ would be nice, as we could
/// easily cache the sum of weights when the map was constructed.

int
_rand_map_int(uint32_t *seed, RandMapInt *map, int *index)
{
    unsigned weight, sum;
    RandMapInt *p;
    uint32_t rand;
    int i = -1;
    int selection = 0;

    sum = 0;
    p = map;
    while (p->weight != 0) {
	sum += p->weight;
	p++;
    }

    if (sum != 0) {

	rand = _rand32(seed, sum);

	weight = 0;
	p = map;
	i = 0;
	while (p->weight != 0) {
	    weight += p->weight;
	    if (rand < weight) {
		selection = p->selection;
		break;
	    }
	    p++;
	    i++;
	}
    }

    if (index != 0) {
	*index = i;
    }

    return selection;
}

    
/// Select an integer from a weighted distribution using the system-side seed
/// \a _seed32
///
/// See _rand_map_int() for documentation

int
rand_map_int(RandMapInt *map)
{
    return _rand_map_int(&_seed32, map, 0);
}


/// Select a pointer from a weighted distribution using a specific seed
///
/// \param seed A 32-bit unsigned random seed (accumulator)
///
/// \param map An array of RandMapPtr structures, the final element of which
/// must have the \a weight field = 0.  This array will typically be allocated
/// statically. 
///
/// \param index An optional pointer to an integer which will recieve the
/// index of the item selected.  NULL \a index are ignored. This is provided
/// for appplications that require statistics on selections.
///
/// \retval One of the \a selection from the array with a non-0 \a weight.  If
/// the weight array is NULL (= {{0, \<dont care\>}}), then by convention the
/// return value is 0, and the return index is -1;
///
/// This routine selects items from the \a map randomly, given the weighting
/// implied by (map[i].weight / SUM(i = 0,...,N, map[i].weight)).  For
/// example, the following two maps are equivalent in that they select &a and
/// &c with 25% probability, and &b with 50% probability:
///
/// RandMapPtr map0[] = {{1, &a}, {2, &b}, {1, &c}, {0, 0}};
///
/// RandMapPtr map1[] = {{25, &a}, {50, &b}, {25, &c}, {0, 0}};
///
/// Note that several errors including negative weights, or the overflow of
/// the sum of weights as an \a unsigned number are neither detected nor
/// reported.


void *
_rand_map_ptr(uint32_t *seed, RandMapPtr *map, int *index)
{
    unsigned weight, sum;
    RandMapPtr *p;
    uint32_t rand;
    int i = -1;
    void *selection = 0;

    sum = 0;
    p = map;
    while (p->weight != 0) {
	sum += p->weight;
	p++;
    }

    if (sum != 0) {

	rand = _rand32(seed, sum);

	weight = 0;
	p = map;
	i = 0;
	while (p->weight != 0) {
	    weight += p->weight;
	    if (rand < weight) {
		selection = p->selection;
		break;
	    }
	    p++;
	    i++;
	}
    }
    
    if (index != 0) {
	*index = i;
    }

    return selection;
}

    
/// Select a pointer from a weighted distribution using the system-side seed
/// \a _seed32
///
/// See _rand_map_ptr() for documentation

void *
rand_map_ptr(RandMapPtr *map)
{
    return _rand_map_ptr(&_seed32, map, 0);
}


////////////////////////////////////////////////////////////////////////////

#ifdef __TEST_RAND_C__

#include <stdio.h>
#include <stdlib.h>

#ifdef RANDOM_MAP

// Weighted distribution testing

int a, b, c;
int aa, bb, cc;

int x[3];

RandMapPtr map0[] = {{1, &a}, {2, &b}, {1, &c}, {0, 0}};
RandMapPtr map1[] = {{25, &aa}, {50, &bb}, {25, &cc}, {0, 0}};

RandMapInt map2[] = {{25, 0}, {50, 1}, {25, 2}, {0, 0}};

int
main()
{
    int i, j;
    int *p;

    for (i = 0; i < 1000000; i++) {
	p = (int *)(rand_map_ptr(map0));
	*p = *p + 1;
	p = (int *)(rand_map_ptr(map1));
	*p = *p + 1;
	j = rand_map_int(map2);
	x[j]++;
    }
    
    printf("%d %d %d\n", a, b, c);
    printf("%d %d %d\n", aa, bb, cc);
    printf("%d %d %d\n", x[0], x[1], x[2]);

    return 0;
}

#endif	/* RANDOM_MAP */


#ifdef BASIC_TEST

// Simple self-checking uniform distrubution tests for rand32.

void
test(int *a, int size, int count, double max_error)
{
    int i;
    double error;

    for (i = 0; i < size; i++) {
	a[i] = 0;
    }

    for (i = 0; i < size * count; i++) {
	a[rand32(size)]++;
    }

    for (i = 0; i < size; i++) {
	error = (a[i] / (double)count) - 1.0;
	printf("a[%4d] : %10d %.5f\n", i, a[i], error);
	if (abs(error) > max_error) {
	    printf("Too much error\n");
	    exit(1);
	}
    }
}
    
#define MAX_SIZE 128

int
main()
{
    int a[MAX_SIZE];
    int count = 1000000;
    double max_error = .005;
    int i;

    for (i = 2; i <= MAX_SIZE; i *= 2) {
	printf("\nTest %d\n\n", i);
	test(a, i, count, max_error);
    }

    srand32(0);
    for (i = 2; i <= MAX_SIZE; i *= 2) {
	printf("\nTest %d\n\n", i);
	test(a, i, count, max_error);
    }

    srand32(0xdeadbeef);
    for (i = 2; i <= MAX_SIZE; i *= 2) {
	printf("\nTest %d\n\n", i);
	test(a, i, count, max_error);
    }
}
    
#endif	/* BASIC_TEST */

#endif	/* __TEST_RAND_C__ */

