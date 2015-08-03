/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/byte_pool.h $                               */
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
#ifndef __BYTE_POOL_H__
#define __BYTE_POOL_H__

// $Id$

#ifndef __ASSEMBLER__

#include "ssx_io.h"

struct ByteBlock;
struct FreeByteBlock;

// A handy constant

#define BITS_PER_UNSIGNED_LONG (8 * sizeof(unsigned long))


/// A control structure for a byte pool. The application will never modify the
/// structure fields directly, but some applications may be interested in
/// reading the statistics.

typedef struct {

    // The index of the first allocated row of free list pointers
    int first_row;

    // The index of the last allocated row of free list pointers
    int last_row;

    // The number of columns in each row of free list pointers
    int columns;

    // The log2 of the number of columns in each row of free list pointers
    int log2_columns;

    // The shifted block size is ANDed with this mask to extract the column
    // number. 
    size_t column_mask;

    // The minimum block size.
    int minimum_block_size;

    // The vectors of free list pointers
    struct FreeByteBlock **free[BITS_PER_UNSIGNED_LONG];

    // The array of column status bit masks
    unsigned long column_status[BITS_PER_UNSIGNED_LONG];

    // The row status bit mask
    unsigned long row_status;

    // A sentinel node - the first allocated block.  Kept for error checking
    // purposes.
    struct ByteBlock *first_block;

    // The initial memory allocation. Kept here only for debugging purposes. 
    struct ByteBlock *big_block;

    // A sentinel node - the last allocated block.  Kept here for error
    // checking purposes.
    struct ByteBlock *last_block;

    // The initial allocation. Kept here for debugging and statistics.
    size_t initial_allocation;

    // The total number of bytes currently allocated (excludes overhead)
     size_t bytes_allocated;

    // The total number of bytes currently free in the pool
     size_t bytes_free;

    // The total number of blocks allocated from the pool
     size_t blocks_allocated;

    // The total number of blocks free in the pool
     size_t blocks_free;

    // The number of calls to allocate memory
     size_t alloc_calls;

    // The number of calls to free memory
     size_t free_calls;

} BytePool;

extern BytePool *_malloc_byte_pool;

int
byte_pool_create(BytePool *pool, void *memory, size_t size);

int
byte_pool_create_tuned(BytePool *pool, void *memory, size_t size,
		       int columns);

int
byte_pool_alloc(BytePool *pool, void **memory, size_t size);

int
byte_pool_calloc(BytePool *pool, void **memory, size_t nmemb, size_t size);

int
byte_pool_free(BytePool *pool, void *memory);

void
byte_pool_block_info(void* memory,
                     void** actual_address, size_t* actual_size,
                     size_t* useful_size);

int
byte_pool_alloc_aligned(BytePool *pool, void **memory, size_t size, 
			int alignment);

void *
malloc(size_t size);

void *
calloc(size_t nmemb, size_t size);

void
free(void *ptr);

int
posix_memalign(void** memptr, size_t alignment, size_t size);

void
byte_pool_report(FILE* stream, BytePool* pool);

#endif	/* __ASSEMBLER__ */


// Error/panic codes

#define BYTE_POOL_INVALID_OBJECT    0x00b98e01
#define BYTE_POOL_INVALID_ARGUMENT  0x00b98e02
#define BYTE_POOL_REVERSE_LINKAGE   0x00b98e03
#define BYTE_POOL_FORWARD_LINKAGE   0x00b98e04
#define BYTE_POOL_NO_MEMORY         0x00b98e05

#define BYTE_POOL_NULL_FREE_LIST    0x00b98e10
#define BYTE_POOL_INVALID_FREE_LIST 0x00b98e11

#endif	/* __BYTE_POOL_H__ */
