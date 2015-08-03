/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/lib/ppc405lib/byte_pool.c $                               */
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

/// \file byte_pool.c
/// \brief An implementation of a constant-time malloc() algorithm.
///
/// The 'byte-pool' API defined in this file is similar to ThreadX' byte-pool
/// operations, with the major difference that here there is no concept of a
/// thread blocking on memory allocation.  This is a concept that is difficult
/// to implement correctly and efficiently, and several semantic options
/// exist for how allocation and freeing memory should work in the presence of
/// blocked threads. The application is always free to implement a blocking
/// API using these APIs and SSX synchronization primitives.
///
/// For convenience this implementation also provides malloc() and calloc()
/// calls.  These APIs depend on the application creating a byte pool and
/// assigning it to the library variable \a _malloc_byte_pool.
///
/// \todo Consider separating the idea of creating a byte pool with a specific
/// maximum-sized block, from the idea of adding memory to a byte pool. The
/// idea is to allow pieces of memory required during initialization of the
/// application to be added back to the pool at run time.  We could also
/// simply add an API to allow memory to be added to a previously created
/// pool, with the special case that if the block were 'too big' that it would
/// be split into smaller chunks.
///
/// \todo Consider
/// adding an option to store a pointer to the originating byte pool in the
/// block header.  This would allow deallocation of any block with free()
/// regardless of which pool it was allocated from, at a cost of 1 pointer per
/// block.  This would simplify some of our validation test cases. We could
/// also accomplish this by having each pool register ranges of addresses that
/// it allocates from, but that would require a search every time we freed a
/// block.

#include "ssx.h"
#include "byte_pool.h"

// This is an implementation of the TLSF (Two-Level Segregate Fit) algorithm
// described by M. Masmano, I. Ripoll and A Crespol,
// http:/rtportal.upv.es/rtmalloc. A couple of their papers and presentations
// are archived in /lib/doc/tlsf.  This is a 'clean-room' implementation of
// their published ideas based solely on their papers and presentations.  No
// part of their GPL implementation was used to create the byte-pool facility
// implemented here. The algorithm as implemented here should port without
// problem to either 32-bit or 64-bit implementations.
//
// TLSF has the nice property that it is a constant-time algorithm, both for
// allocation and freeing allocated blocks.  This property is guaranteed by
// always trading space for speed in the algorithm. This means that we can
// (and do) run the allocation and freeing in a critical section. With all
// error checking and statistics, a worst-case byte_bool_alloc() was
// timed at 318 PowerPC instructions in the PgP OCC (PPC405) Simics
// simulation.  A worst-case byte_pool_free() was timed at 368 PowerPC
// instructions. These times are expected to equate to ~3us in OCC, a
// reasonable value for a critical section. [The above times include all
// error checking].
//
// An allocation unit requires two pointers of overhead - a pointer to the
// previous block and a pointer to the next block (both in terms of linear
// addresses). The minimum block size also includes the requirement for two
// extra pointers used to link free blocks into their free lists. The final
// size of the block (including user data) is simply computed as (next - self)
// in terms of linear addresses.
//
// An allocated block is marked by setting the low-order bit of the 'previous'
// pointer, which bit is otherwise guaranteed to be 0 due to alignment
// restrictions. Whenever a block is freed it is immediately merged with the
// previous and next blocks, if possible. Several places in the block
// splitting and merging code take advantage of this invariant, and assume
// that if a block is not already merged with its 'next' partner, then the
// 'next' partner must be allocated.  Sentinel blocks are allocated at either
// end of the managed area to avoid special checks for the first and last
// memory blocks during merging.
//
// The 'Two-Level' in TLSF refers to the fact that there are multiple free
// lists arranged in a 2-dimensional array.  Each free lists contains blocks
// that fall into a particular size range.  The free list pointers and other
// data structures described below are carved out of the initial free area
// when a byte pool is initialized.
//
// The first dimension of the free list array is simply the floor(log2(size))
// of the block. For the second dimension, a tuning parameter selects how many
// columns each row in the table will contain. The number of columns must be
// an even power-of-2. Each column represents a fixed power-of-2 size
// increment of block sizes.  Given a block size, it is easy to compute the
// row and column indices of a free list containing blocks of that size with
// shifting and masking operation.
//
// It is assumed that the C type 'unsigned long' is the same size as a
// pointer.  Therefore the number of rows in the table is less than or equal
// to the number of bits in an unsigned long.  The number of columns is also
// restricted to being in the range (1, 2, 4, ... number of bits in unsigned
// long).
//
// The above restrictions make it very fast (and constant time) to find a free
// list that contains a block that will satisfy the allocation request. The
// byte pool structure maintains a 'row status' word that indicates whether
// there are any blocks free in any of the free lists of the row.  Each row
// also has an associated 'column status' word that indicates which free lists
// have blocks free. A row status bit is set if and only if at least one bit
// in the column status for that row is set.
//
// Note that although the 32-bit PowerPC implementation conceptually contains
// a 32x32 array of free list pointers, only the free list pointers actually
// required to hold the representable blocks are allocated. 
//
// The algorithm uses the GCC __builtin_clzl() function to count leading zeros
// in the status words to find rows/columns that contain free blocks.  This
// generates the 'cntlzw' instruction on 32-bit PowerPC and a similar
// instruction on X86.  So the algorithm is also portable across machines -
// which simplifies testing.
//
// A couple of final facts: When the application requests memory, the block
// header overhead is added to the request and we look for a free list
// guaranteed to contain blocks of the requested size. That means that the
// request size must be rounded up to the next free list size, to avoid having
// to search a list that might not contain a block of the proper size. This
// leads to cases where allocation will fail, even though the requested memory
// is actually available. That's just the price we have to pay for a
// constant-time guarantee.
//
// This memory allocator will never be used in mission-mode for hard-real-time
// applications, so the statistics are always kept up-to-date.  This adds some
// overhead, but does not effect the constant-time behavior.
// 
// Given the above description, hopefuly the brief comments with the
// implementation will make sense.


/// The byte pool for use by malloc() and calloc().
///
/// The application must define a byte pool and assign it to _malloc_byte_pool
/// in order for malloc() and calloc() to work.

BytePool *_malloc_byte_pool = 0;


// The byte pool memory block header.
//
// Each memory block requires 2 pointers of overhead - the pointers to the
// previous and next (in terms of linear addresses) blocks. The low-order bit
// of the \a previous pointer is used as the \e allocated flag, and is set
// when a block is allocated.  The size of a block is computed simply as the
// \a next - \a self. This header layout makes it very simple to merge blocks
// when they are deallocated.

typedef struct ByteBlock {

    // Pointer to the previous (in terms of linear address) block.
    //
    // The low-order bit of the pointer is set to indicate a block that has
    // been allocated.
    struct ByteBlock *previous;

    // Pointer to the next (in terms of linear address) block.
    //
    // The size of the block is computed simply as \a next - \a self.
    struct ByteBlock *next;

} ByteBlock;


// A free byte-pool memory block
//
// Blocks stored in free lists require an additional 2 pointers of
// overhead. The blocks are doubly-linked in the free lists to make deletion
// a constant-time operation.  Note that the previous pointer is a pointer to
// a pointer to a ByteBlock - it may be pointing to the free list
// header. Since all blocks must be freeable, this structure defines the
// minimum block size.

typedef struct FreeByteBlock {

    // The base object
    ByteBlock block;

    // Pointer to the next block in the free list
    struct FreeByteBlock *next;

    // Pointer to the \a next pointer of the previous element in the free
    // list, or a pointer to the free list header.
    struct FreeByteBlock **previous;

} FreeByteBlock;


// All blocks will be aligned to this size, so this size also defines the
// minimum quantum of memory allocation.  The coice of 8 should give
// good results for both 32-bit and 64-bit implementations.  
//
// NB : This implmentation assumes that the ByteBlock and FreeByteBLock are
// aligned to this alignment - if this constant is ever changed from 8 then
// the ByteBlock and FreeByteBlock may need to be padded to meet the alignment
// assumptions, and the \a minimum_block_size may need to be adjusted.

#define ALIGNMENT 8


// An unsigned long, big-endian bit mask

#define UL_BE_MASK(i) \
    ((unsigned long)1 << (BITS_PER_UNSIGNED_LONG - (i) - 1))


// Align a value to the alignment.  The direction is either positive or
// negative to indicate alignment up or down.

static inline unsigned long
align(unsigned long x, int direction)
{
    if (x % ALIGNMENT) {
	if (direction > 0) {
	    return x + (ALIGNMENT - (x % ALIGNMENT));
	} else {
	    return x - (x % ALIGNMENT);
	}	    
    } else {
	return x;
    }
}


// Compute the floor(log2(x)) of x.  This is used to compute the row indices
// of blocks based on the block size.

static inline int
floor_log2(unsigned long x)
{
    return BITS_PER_UNSIGNED_LONG - 1 - __builtin_clzl(x);
}


// In theory the tuning parameters might vary based on the amount of memory
// being managed, but for now we simply use constants.  
// 
// The minimum block size includes both the size of the header, as well as the
// requirement that the number of columns be <= the mimumum block size to make
// the addressing uniform. For example, on PPC405 the minimum block size is 16
// bytes (4 pointers) -- unless the number of columns is 32, in which case it
// has to grow to 32 bytes.
//
// Note that no matter what, we may allocate free list pointers in the
// lower-numbered rows that will never be populated due to alignment
// constraints.

#ifndef BYTE_POOL_TLSF_COLUMNS
#define BYTE_POOL_TLSF_COLUMNS 8 /* 1,2,4, ... BITS_PER_UNSIGNED_LONG */
#endif

static void
compute_tuning(BytePool *pool, size_t size, int columns)
{
    int log2_min_size;

    pool->columns = columns;
    pool->log2_columns = floor_log2(pool->columns);
    pool->column_mask = (1 << pool->log2_columns) - 1;

    log2_min_size = MAX(pool->log2_columns, floor_log2(sizeof(FreeByteBlock)));
    pool->minimum_block_size = align(1 << log2_min_size, 1);
}


// Compute the size of a block

static inline size_t
block_size(ByteBlock *block)
{
    return (unsigned long)(block->next) - (unsigned long)block;
}


/// Return implementation information for a block
///
/// \param memory The memory block to query.  This pointer must have been
/// returned by one of the byte_pool functions or derivitives, or may also be
/// 0. 
///
/// \param actual_address : Returned as the address of the block header.
///
/// \param actual_size Returned as the size of the complete block, including
/// the header.
///
/// \param useful_size : Returned as the actual amount of space available from
/// \a memory to the end of the block. The \a useful_size may be useful to
/// applications that allocate big blocks then carve them up into smaller
/// structures.
///
/// Note that any of \a actual_address, \a actual_size and \a useful_size may
/// be passed in as 0 if the caller does not require the information.

void
byte_pool_block_info(void* memory,
                     void** actual_address, size_t* actual_size,
                     size_t* useful_size)
{
    ByteBlock* block;

    if (memory == 0) {

        if (actual_address) *actual_address = 0;
        if (actual_size) *actual_size = 0;
        if (useful_size) *useful_size = 0;

    } else {

        // This implementation uses the convention that if the \a next pointer
        // of the putative ByteBlock == 1, then this is actually an aligned
        // allocation and the actual ByteBlock is located at the address
        // contained in the \a previous field of the dummy header.

        block = (ByteBlock *)(((unsigned long)memory) - sizeof(ByteBlock));
        if ((int)(block->next) == 1) {
            block = block->previous;
        }

        if (actual_address) *actual_address = block;
        if (actual_size) *actual_size = block_size(block);
        if (useful_size) 
            *useful_size = 
                (unsigned long)(block->next) - (unsigned long)memory;
    }
}


// Mark a block as allocated by setting the low-order bit of the \a previous
// pointer.

static inline ByteBlock *
allocated(ByteBlock *p)
{
    return (ByteBlock *)((unsigned long)p | 1ul);
}


static void
mark_allocated(BytePool *pool, ByteBlock *block)
{
    size_t bytes = block_size(block);

    pool->bytes_allocated += bytes;
    pool->bytes_free -= bytes;
    pool->blocks_allocated += 1;
    pool->blocks_free -= 1;

    block->previous = allocated(block->previous);
}


// Mark a block as free by clearing the low-order bit of the \a previous
// pointer. 

static inline ByteBlock *
deallocated(ByteBlock *p)
{
    return (ByteBlock *)((unsigned long)p & ~1ul);
}


static void
mark_free(BytePool *pool, ByteBlock *block)
{
    size_t bytes = block_size(block);

    pool->bytes_allocated -= bytes;
    pool->bytes_free += bytes;
    pool->blocks_allocated -= 1;
    pool->blocks_free += 1;

    block->previous = deallocated(block->previous);
}


// Check for a block being free

static inline int
block_is_free(ByteBlock *block)
{
    return (((unsigned long)(block->previous)) & 1ul) == 0;
}


// Normalize a 'previous' pointer

static inline ByteBlock *
normalize_previous(ByteBlock *previous)
{
    return (ByteBlock *)((unsigned long)previous & ~1ul);
}


// Check for correct linkage.  This is such a critical check for application
// memory corruption that it is always done.

static int
check_linkage(ByteBlock *block)
{
    if (normalize_previous(block->next->previous) != block) {
        printk("byte_pool: Forward linkage error\n"
               "    block                 : %p\n"
               "    block->next           : %p\n"
               "    block->next->previous : %p\n",
               block,
               block->next,
               block->next->previous);
        SSX_ERROR(BYTE_POOL_REVERSE_LINKAGE);
    } else if (normalize_previous(block->previous)->next != block) {
        printk("byte_pool:  linkage error\n"
               "    block->previous       : %p\n"
               "    block->pevious->next  : %p\n"
               "    block                 : %p\n",
               block->previous,
               block->previous->next,
               block);
        SSX_ERROR(BYTE_POOL_FORWARD_LINKAGE);
    } 
    return 0;
}


// Mark a free list as empty

static inline void
mark_empty(BytePool *pool, int row, int column)
{
    pool->column_status[row] &= ~UL_BE_MASK(column);
    if (pool->column_status[row] == 0) {
	pool->row_status &= ~UL_BE_MASK(row);
    }
}


// Mark a free list as non-empty

static inline void
mark_non_empty(BytePool *pool, int row, int column)
{
    pool->column_status[row] |= UL_BE_MASK(column);
    pool->row_status |= UL_BE_MASK(row);
}


// Convert a size into row and column indices

static inline void
size2rc(BytePool *pool, size_t size, int *row, int *column)
{
    *row = floor_log2(size);
    *column = (size >> (*row - pool->log2_columns)) & pool->column_mask;
}    


// Given a block size, find the free list that contains blocks of that size
// (or greater, up to the next free list). When called during block freeing,
// the block size is known to be valid.  When called during allocation, the
// block size may be invalid (too big), in which case 0 is returned.

static FreeByteBlock **
find_free_list(BytePool *pool, size_t size, int *row, int *column)
{
    size2rc(pool, size, row, column);
    if (*row > pool->last_row) {
	return 0;
    }
    return &((pool->free[*row])[*column]);
}


// Remove an arbitrary block from its free list due to a merging operation.

static void 
unlink_free_block(BytePool *pool, ByteBlock *block)
{
    FreeByteBlock **free_list;
    FreeByteBlock *free_block;
    int row, column;

    free_list = find_free_list(pool, block_size(block), &row, &column);

    if (SSX_ERROR_CHECK_KERNEL) {
	if (free_list == 0) {
	    SSX_PANIC(BYTE_POOL_NULL_FREE_LIST);
	}
    }

    // Unlink the block from the free list

    free_block = (FreeByteBlock *)block;
    *(free_block->previous) = free_block->next;
    if (free_block->next) {
	free_block->next->previous = free_block->previous;
    }

    // If the free list is now 0, mark the free list as empty.

    if (*free_list == 0) {
	mark_empty(pool, row, column);
    }
}


// Link a block into the head of its free list due to freeing memory

static void 
link_free_block(BytePool *pool, ByteBlock *block)
{
    FreeByteBlock **free_list;
    FreeByteBlock *free_block;
    int row, column;

    free_list = find_free_list(pool, block_size(block), &row, &column);

    if (SSX_ERROR_CHECK_KERNEL) {
	if (free_list == 0) {
	    SSX_PANIC(BYTE_POOL_NULL_FREE_LIST);
	}
    }

    // Link the block into the free list, and mark the free list as
    // non-empty. 

    free_block = (FreeByteBlock *)block;

    free_block->next = *free_list;
    if (*free_list) {
	(*free_list)->previous = &(free_block->next);
    }
    *free_list = free_block;
    free_block->previous = free_list;

    mark_non_empty(pool, row, column);
}


// Round up the block size (if required) to the next column. Note that the
// block_size input here is aligned, and remains aligned even after rounding.

static size_t
round_up_size(BytePool *pool, size_t block_size)
{
    size_t residue, column_span, column_mask;
    int row = floor_log2(block_size);

    column_span = 1 << (row - pool->log2_columns);
    column_mask = column_span - 1;
    residue = block_size & column_mask;

    if (residue == 0) {
    	return block_size;
    } else {
    	return block_size + (column_span - residue);
    } 
}


// The implemenation of freeing a block of memory. When freed, a block is
// immediately merged with its neighbors if possible, and the final merged
// block is inserted into the proper free list.
//
// The linkage check is done here so that it can also protect internal uses of
// this API (but only if SSX errors lead to panics, the expected default).

static int
byte_pool_free_block(BytePool *pool, ByteBlock *block)
{
    int rc;
    SsxMachineContext ctx;

    rc = check_linkage(block);
    if (rc) return rc;

    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    pool->free_calls++;

    mark_free(pool, block);

    if (block_is_free(block->next)) {

	// Merge next block into current block

	unlink_free_block(pool, block->next);

	block->next = (ByteBlock *)((unsigned long)(block->next) +
				    block_size(block->next));
	block->next->previous = allocated(block);

	pool->blocks_free--;
    }

    if (block_is_free(block->previous)) {

	// Merge current block into previous block

	unlink_free_block(pool, block->previous);

	block->previous->next = 
	    (ByteBlock *)((unsigned long)(block->previous->next) +
			  block_size(block));
	block = block->previous;
	block->next->previous = allocated(block);

	pool->blocks_free--;
    }

    // Finally, insert the block into the proper free list.

    link_free_block(pool, block);

    ssx_critical_section_exit(&ctx);

    return 0;
}


/// Free a block of memory back to a byte pool
///
/// \param pool A pointer to the BytePool structure that allocated the memory.
///
/// \param memory A pointer to memory returned by byte_pool_alloc() or
/// byte_pool_alloc_aligned() for the pool.  This pointer may be NULL (0), in
/// which case the byte_pool_free() request succeeds immediately.
///
/// The part of this API that manipulates the \a pool runs as an
/// SSX_NONCRITICAL critical section. byte_pool_free() uses a constant-time
/// algorithm.
///
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval 0 Success
///
/// \retval -BYTE_POOL_INVALID_OBJECT The \a pool argument was NULL (0).
///
/// \retval -BYTE_POOL_INVALID_ARGUMENT The block is not marked as being
/// allocated, or does not appear to have been allocated from this byte_pool.
///
/// \retval -BYTE_POOL_LINKAGE_ERROR The block being freed is not linked
/// correctly with the other blocks managed by the pool, most likely
/// indicating that the memory being freed was not allocated by
/// byte_pool_alloc(), or that memory corruption has occured.

// This implementation uses the convention that if the \a next pointer of the
// putative ByteBlock == 1, then this is actually an aligned allocation and
// the actual ByteBlock is located at the address contained in the \a previous
// field of the dummy header.

int
byte_pool_free(BytePool *pool, void *memory)
{
    ByteBlock *block;

    if (memory == 0) {
	return 0;
    }

    block = (ByteBlock *)(((unsigned long)memory) - sizeof(ByteBlock));
    if ((int)(block->next) == 1) {
        if (0) {
            printk("byte_pool_free(%p, %p) [%p] : Aligned\n",
                   pool, memory, block);
        }
        block = block->previous;
    }

    if (0) {
	printk("byte_pool_free(%p, %p) [%p] : %d %d %d\n",
               pool, memory, block,
               block_is_free(block), 
               block < pool->first_block,
               block > pool->last_block);
    }

    if (SSX_ERROR_CHECK_API) {
	SSX_ERROR_IF(pool == 0, BYTE_POOL_INVALID_OBJECT);
	SSX_ERROR_IF(block_is_free(block)        ||
		     (block < pool->first_block) ||
		     (block > pool->last_block), 
		     BYTE_POOL_INVALID_ARGUMENT);
    }

    return byte_pool_free_block(pool, block);
}


/// Create a BytePool with explicit specification of tuning parameters
///
/// This routine is the real body of byte_pool_create(), however this
/// underlying interface is provided for testing and experimentation and allows
/// the specification of non-default tuning parameters.  
///
/// There is actually only one tuning parameter for TLSF - the number of
/// columns.  The number of columns must be an even power of two no larger
/// than the number of bits in an unsigned long.
///
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval -BYTE_POOL_INVALID_OBJECT The \a pool pointer was NULL (0).
///
/// \retval -BYTE_POOL_INVALID_ARGUMENT Either the \a memory pointer was NULL
/// (0), the amount of memory was insufficient for the management overhead, or
/// the parameterization was invalid.

int
byte_pool_create_tuned(BytePool *pool, void *memory, size_t size,
		       int columns)
{
    size_t overhead, free_list_overhead;
    unsigned long memory_ul, aligned_memory;
    int i;
    FreeByteBlock **free;

    if (SSX_ERROR_CHECK_API) {
	SSX_ERROR_IF(pool == 0, BYTE_POOL_INVALID_OBJECT);
	SSX_ERROR_IF((memory == 0) ||
		     (columns < 1) ||
		     ((columns & (columns - 1)) != 0) ||
		     (floor_log2(columns) > floor_log2(BITS_PER_UNSIGNED_LONG)),
		     BYTE_POOL_INVALID_ARGUMENT);
    }

    // Compute tuning parameters

    compute_tuning(pool, size, columns);

    // Clear free list vector pointers and column status
    
    for (i = 0; i < BITS_PER_UNSIGNED_LONG; i++) {
	pool->free[i] = 0;
	pool->column_status[i] = 0;
    }

    // Determine the first and last allocated rows. 

    pool->first_row = floor_log2(pool->minimum_block_size);
    pool->last_row = floor_log2(size);

    // The dynamic overhead consists of aligment overhead, 2 sentinel nodes,
    // the vectors of pointers to free lists, plus 2 alignments. There must
    // also be enough room for at least 1 block to allocate.

    memory_ul = (unsigned long)memory;
    aligned_memory = align(memory_ul, 1);

    free_list_overhead = 
	(((pool->last_row - pool->first_row + 1) * pool->columns) * 
	 sizeof(FreeByteBlock *));

    overhead = 
	(aligned_memory - memory_ul) +
	(2 * sizeof(ByteBlock)) +
	free_list_overhead +
	(2 * ALIGNMENT) +
	pool->minimum_block_size;

    if (SSX_ERROR_CHECK_API) {
	SSX_ERROR_IF(overhead >= size, BYTE_POOL_INVALID_ARGUMENT);
    }
	
    // Allocate the overhead items. The free list vectors and column status
    // arrays are carved out and zeroed. For good measure we re-align after
    // each of these operations. The sentinel blocks are carved off of either
    // end of the remaining free space and marked allocated.  The remaining
    // initial "big block" is also initialized (as if it were allocated).

    size = size - (aligned_memory - memory_ul);
    size = align(size, -1);
    
    pool->row_status = 0;

    free = (FreeByteBlock **)aligned_memory;
    memset((void *)free, 0, free_list_overhead);

    aligned_memory += free_list_overhead;
    size -= free_list_overhead;
    aligned_memory = align(aligned_memory, 1);
    size = align(size, -1);

    for (i = pool->first_row; i <= pool->last_row; i++) {
	pool->free[i] = free;
	free += pool->columns;
    }

    pool->first_block = (ByteBlock *)aligned_memory;
    aligned_memory += sizeof(ByteBlock);
    size -= sizeof(ByteBlock);

    pool->big_block = (ByteBlock *)aligned_memory;

    pool->last_block = 
	(ByteBlock *)(aligned_memory + size - sizeof(ByteBlock));
    size -= sizeof(ByteBlock);

    pool->first_block->next = pool->big_block;
    pool->first_block->previous = 0;
    mark_allocated(pool, pool->first_block);

    pool->last_block->next = 0;
    pool->last_block->previous = pool->big_block;
    mark_allocated(pool, pool->last_block);

    pool->big_block->previous = pool->first_block;
    pool->big_block->next = pool->last_block;

    // Initialize statistics

    pool->bytes_allocated = 0;
    pool->bytes_free = block_size(pool->big_block);
    pool->initial_allocation = pool->bytes_free;
    pool->blocks_allocated = 0;
    pool->blocks_free = 1;
    pool->alloc_calls = 0;
    pool->free_calls = 0;

    // Free the big block and we're ready to go.

    mark_allocated(pool, pool->big_block);
    byte_pool_free_block(pool, pool->big_block);

    return 0;
}


/// Create a BytePool
///
/// \param pool A pointer to an uninitialized BytePool structure
///
/// \param memory A pointer to the memory to be managed by the BytePool
///
/// \param size The size of the managed area in bytes
///
/// byte_pool_create() sets up the \a memory area to be used as a memory pool
/// for malloc()-style allocation using byte_pool_alloc() and
/// byte_pool_free().  Note that the actual memory area available for
/// allocation will be smaller than \a size due to alignment, and reservation
/// of a portion of the area for management overhead.
///
/// Return values other then SSX_OK (0) are errors; see \ref ssx_errors
///
/// \retval -BYTE_POOL_INVALID_OBJECT The \a pool pointer was NULL (0).
///
/// \retval -BYTE_POOL_INVALID_ARGUMENT Either the \a memory pointer was NULL
/// (0), the amount of memory was insufficient for the management overhead, or
/// the parameterization was invalid.

int
byte_pool_create(BytePool *pool, void *memory, size_t size)
{
    return byte_pool_create_tuned(pool, memory, size, BYTE_POOL_TLSF_COLUMNS);
}


/// Allocate memory from a byte pool
///
/// \param pool A pointer to an initialized BytePool
///
/// \param memory An address to recieve a pointer to the allocated memory.
/// This address will be set to NULL (0) if the allocation request can not be
/// satisfied (or the \a size is 0).
///
/// \param size The number of bytes to allocate.
///
/// The part of this API that manipulates the \a pool runs as an
/// SSX_NONCRITICAL critical section. byte_pool_alloc() uses a constant-time
/// algorithm.
///
/// Return values other than 0 are not necessarily errors; see \ref
/// ssx_errors.   
///
/// The following return codes are not considered errors:
///
/// \retval 0 Success
///
/// \retval -BYTE_POOL_NO_MEMORY The allocation request could not be
/// satisfied.  The memory pointer will also be NULL (0) in this case.
///
/// The following return codes are considered errors:
///
/// \retval -BYTE_POOL_INVALID_OBJECT The \a pool argument was NULL (0).
///
/// \retval -BYTE_POOL_INVALID_ARGUMENT The \a memory argument is NULL (0).

int
byte_pool_alloc(BytePool *pool, void **memory, size_t size)
{
    SsxMachineContext ctx;
    size_t request_size, actual_size;
    int found, row, column;
    unsigned long row_status, column_status;
    FreeByteBlock **free_list;
    FreeByteBlock *free_block;
    ByteBlock *block;
    ByteBlock *residue_block;

    if (SSX_ERROR_CHECK_API) {
	SSX_ERROR_IF(pool == 0, BYTE_POOL_INVALID_OBJECT);
	SSX_ERROR_IF(memory == 0, BYTE_POOL_INVALID_ARGUMENT);
    }

    // Quickly dispense with NULL requests

    if (size == 0) {
	*memory = 0;
	return 0;
    }

    // Compute the requested block size (which includes the header).  If the
    // size went down we overflowed due to a huge request (which can't be
    // filled).  Otherwise if the request is small it is boosted up to the
    // (aligned) minimum size.  To guarantee fast search, the requested size
    // must then be rounded up to a size that is represented in the 2-D array
    // of free list pointers.

    request_size = align(size + sizeof(ByteBlock), 1);
    if (request_size < size) {
        *memory = 0;
        return -BYTE_POOL_NO_MEMORY;
    }

    if (request_size < pool->minimum_block_size) {
	request_size = pool->minimum_block_size;
    }

    request_size = round_up_size(pool, request_size);

    // Up to this point, all accesses of the memory pool object have been to
    // read only constants. Now we get serious.

    ssx_critical_section_enter(SSX_NONCRITICAL, &ctx);

    pool->alloc_calls++;

    // See if a block of the correct or larger size exists in the row.  The
    // search is first via a single bit in the row_status. If that hits then
    // we check for columns >= the target column.

    found = 0;
    size2rc(pool, request_size, &row, &column);

    if (pool->row_status & UL_BE_MASK(row)) {

	column_status = pool->column_status[row] &
	    ((UL_BE_MASK(column) << 1) - 1);

	if (column_status != 0) {
	    column = __builtin_clzl(column_status);
	    found = 1;
	}
    }

    // If the block was not found in the 'optimum' row, look in all rows of
    // larger size and take the first block that fits.

    if (!found) {

	row_status = pool->row_status & (UL_BE_MASK(row) - 1);

	if (row_status != 0) {
	    row = __builtin_clzl(row_status);
	    column = __builtin_clzl(pool->column_status[row]);
	    found = 1;
	}
    }

    // Another out of memory case.

    if (!found) {
	ssx_critical_section_exit(&ctx);
	*memory = 0;
	return -BYTE_POOL_NO_MEMORY;
    }

    // Now we can get the pointer to the free list and take the block. 

    free_list = &((pool->free[row])[column]);

    if (SSX_ERROR_CHECK_KERNEL) {
	if ((free_list == 0) || (*free_list == 0)) {
	    SSX_PANIC(BYTE_POOL_INVALID_FREE_LIST);
	}
    }

    free_block = *free_list;
    *free_list = free_block->next;
    if (free_block->next) {
	free_block->next->previous = free_list;
    } else {
	mark_empty(pool, row, column);
    }

    // Mark the block as allocated

    block = (ByteBlock *)free_block;
    mark_allocated(pool, block);

    // If there is enough residue, split the excess memory off of the end of
    // the block.  This is a kind of dummy transaction for our statistical
    // purposes.

    actual_size = block_size(block);
    if ((actual_size - request_size) >= pool->minimum_block_size) {

	residue_block = (ByteBlock *)((unsigned long)block + request_size);

	residue_block->next = block->next;
	residue_block->previous = block;
	residue_block->previous->next = residue_block;
	residue_block->next->previous = allocated(residue_block);

	pool->blocks_allocated++;
	byte_pool_free_block(pool, residue_block);
	pool->free_calls--;
    }

    // Set the memory pointer to the area to be used by the application and
    // return.

    *memory = (void *)((unsigned long)block + sizeof(ByteBlock));

    ssx_critical_section_exit(&ctx);

    if (0) {
        ByteBlock* block = 
            (ByteBlock*)((unsigned long)*memory - sizeof(ByteBlock));

        printk("byte_pool_alloc(%p, -> %p, %zu)\n"
               "    request_size = %u, Previous = %p, Next = %p\n",
               pool, *memory, size,
               request_size, block->previous, block->next);
    }

    return 0;
}


/// Allocate memory from a byte pool and clear
///
/// byte_pool_calloc() allocates memory using byte_pool_alloc() then clears
/// the memory area using memset().  The arguments conform to the POSIX
/// standard for calloc(). See byte_pool_alloc() for return codes and usage
/// notes.

int
byte_pool_calloc(BytePool *pool, void **memory, size_t nmemb, size_t size)
{
    int rc;

    rc = byte_pool_alloc(pool, memory, nmemb * size);
    if (rc || (*memory == 0)) {
	return rc;
    }

    memset(*memory, 0, nmemb * size);

    return 0;
}


/// Allocate an aligned memory area
///
/// \param pool A pointer to an initialized BytePool
///
/// \param memory An address to recieve a pointer to the allocated memory.
/// This address will be set to NULL (0) if the allocation request can not be
/// satisfied (or the \a size is 0).
///
/// \param size The size of the memory area required (in bytes). This can be
/// any size - it \e does \e not have to be a multiple of the aligned size (as
/// is required by other common aligned memory allocators).
///
/// \param alignment The alignment constraint, specified as the base 2
/// logarithm of the alignment.  For example, to align on a 128-byte boundary
/// the \a alignment would be specified as 7.
///
/// byte_pool_alloc_aligned() is a convenience interface for allocating memory
/// with a guaranteed alignment. The BytePool APIs do not normally do aligned
/// allocation. byte_pool_alloc_aligned() first uses byte_pool_alloc() to
/// allocate a block of memory large enough to satisfy the request and
/// guarantee that a subset of the memory allocation will satisfy the
/// alignment constraint plus the overhead of a dummy block header.  Note that
/// it is space-inefficient to allocate many small aligned areas. If possble
/// it would be better to allocate a single aligned area and then have the
/// application partition the memory as required.
///
/// Memory areas allocated by byte_pool_alloc_aligned() can be freed with
/// byte_pool_free(), just like any other dynamic memory allocation.
///
/// The part of this API that manipulates the \a pool runs as an
/// SSX_NONCRITICAL critical section. The underlying call of byte_pool_alloc()
/// uses a constant-time algorithm.
///
/// Return values other than 0 are not necessarily errors; see \ref
/// ssx_errors.   
///
/// The following return codes are not considered errors:
///
/// \retval 0 Success
///
/// \retval -BYTE_POOL_NO_MEMORY The allocation request could not be
/// satisfied.  The memory pointer will also be NULL (0) in this case.
///
/// The following return codes are considered errors:
///
/// \retval -BYTE_POOL_INVALID_OBJECT The \a pool argument was NULL (0).
///
/// \retval -BYTE_POOL_INVALID_ARGUMENT The \a memory argument is NULL (0), or
/// the \a alignment argument is invalid.

// The allocation must be big enough for the size requested + the alignment
// amount (to guarantee alignment) + room for a dummy ByteBlock.  The dummy
// ByteBlock is marked by setting the \a next pointer to 1 to indicate that
// this is an aligned allocation.  In this case the \a previous pointer of the
// dummy ByteBlock points to the ByteBlock of the original allocation.

int
byte_pool_alloc_aligned(BytePool *pool, void **memory, size_t size, 
			int alignment)
{
    int rc;
    unsigned long pow2_alignment, mask, aligned;
    void *unaligned_memory;
    ByteBlock *dummy_block, *unaligned_block;

    if (SSX_ERROR_CHECK_API) {
	SSX_ERROR_IF((alignment < 1) || (alignment >= BITS_PER_UNSIGNED_LONG),
		     BYTE_POOL_INVALID_ARGUMENT);
    }

    pow2_alignment = (unsigned long)1 << (unsigned long)alignment;
    mask = pow2_alignment - 1;

    rc = byte_pool_alloc(pool, &unaligned_memory, 
			 size + pow2_alignment + sizeof(ByteBlock));

    if (rc || (unaligned_memory == 0)) {
	*memory = 0;
	return rc;
    }
    unaligned_block = (ByteBlock *)(((unsigned long)unaligned_memory) - 
                                    sizeof(ByteBlock));

    aligned = (unsigned long)unaligned_memory + sizeof(ByteBlock);
    if (aligned & mask) {
	aligned += (pow2_alignment - (aligned & mask));
    }
    *memory = (void *)aligned;
    
    dummy_block = (ByteBlock *)(aligned - sizeof(ByteBlock));
    dummy_block->previous = unaligned_block;
    dummy_block->next = (ByteBlock*)1;

    if (0) {
        printk("byte_pool_alloc_aligned(%p, -> %p, %zu, %d)\n",
               pool, *memory, size, alignment);
    }

    return 0;
}


/// Allocate aligned memory from a byte pool and clear
///
/// byte_pool_calloc_alligned() allocates memory using
/// byte_pool_alloc_aligned() then clears the memory area using memset().  The
/// arguments conform to the POSIX standard for calloc(). See
/// byte_pool_alloc_aligned() for return codes and usage notes. In particular
/// note that this memory must be freed with byte_pool_free_aligned().

int
byte_pool_calloc_aligned(BytePool *pool, void **memory, 
			 size_t nmemb, size_t size, int alignment)
{
    int rc;

    rc = byte_pool_alloc_aligned(pool, memory, nmemb * size, alignment);
    if (rc || (*memory == 0)) return rc;

    memset(*memory, 0, nmemb * size);

    return 0;
}


/// malloc() allocates \a size bytes and returns a pointer to the allocated
/// memory.  The memory is not cleared. The value returned is a pointer to the
/// allocated memory, which is suitably aligned for any kind of variable, or
/// NULL if the requested \a size is 0 or the request fails.
///
/// NB: The aplication must create and assign a BytePool object to the
/// library variable _malloc_byte_pool in order for malloc() to work.

void *
malloc(size_t size)
{
    void *memory;

    if (byte_pool_alloc(_malloc_byte_pool, &memory, size)) {
	memory = 0;
    }
    return memory;
}


/// calloc() allocates memory for an array of \a nmemb elements of \a size
/// bytes each and returns a pointer to the allocated memory.  The memory is
/// set to zero. The value returned is a pointer to the allocated and cleared
/// memory, which is suitably aligned for any kind of variable, or NULL if the
/// requested \a size is 0 or the request fails.
///
/// NB: The aplication must create and assign a BytePool object to the
/// library variable _malloc_byte_pool in order for calloc() to work.

void *
calloc(size_t nmemb, size_t size)
{
    void *memory;

    if (byte_pool_calloc(_malloc_byte_pool, &memory, nmemb, size)) {
	return 0;
    }
    return memory;
}


/// free() frees the memory space pointed to by \a ptr, which must have been
/// returned by a previous call to malloc(), posix_memalign, calloc() or
/// realloc().  Otherwise, or if free(ptr) has already been called before,
/// undefined behavior occurs.  If \a ptr is NULL, no operation is performed.
///
/// NB: The aplication must create and assign a BytePool object to the
/// library variable _malloc_byte_pool in order for free() to work.

void
free(void *ptr)
{
    byte_pool_free(_malloc_byte_pool, ptr);
}


/// realloc() changes the size of the memory block pointed to by \a ptr to \a
/// size bytes.  The contents will be unchanged to the minimum of the old and
/// new sizes; newly allocated memory will be uninitialized.  If \a ptr is
/// NULL, the call is equivalent to malloc(size); if \a size is equal to zero,
/// the call is equivalent to free(ptr).  Unless \a ptr is NULL, it must have
/// been returned by an earlier call to malloc(), calloc() or realloc().  If
/// the area pointed to was moved, a free(ptr) is done.
///
/// realloc() returns a pointer to the newly allocated memory, which is
/// suitably aligned for any kind of variable and may be different from \a
/// ptr, or NULL if the request fails.  If \a size was equal to 0, either NULL
/// or a pointer suitable to be passed to free() is returned.  If realloc()
/// fails the original block is left untouched; it is not freed or moved.

void*
realloc(void *ptr, size_t size)
{
    void *memory;
    size_t useful_size;

    // Handle simple case

    if (ptr == 0) {

        memory = malloc(size);

    } else if (size == 0) {

        free(ptr);
        memory = 0;

    } else {

        // Find out the useful size of the block. If we need more than this we
        // need to allocate a new block and memcpy() the old data to the new
        // block and free the old block. If we need less then this we also
        // need to allocate a new block and move the head of the current
        // data. If the new size is the same as the current size we do nothing.

        byte_pool_block_info(ptr, 0, 0, &useful_size);

        if (size == useful_size) {

            memory = ptr;

        } else {

            memory = malloc(size);
            if (memory != 0) {
                memcpy(memory, ptr, (size > useful_size) ? useful_size : size);
                free(ptr);
            }
        }
    }
    return memory;
}


/// The posix_memalign() function allocates \a size bytes aligned on a
/// boundary specified by \a alignment, and returns a pointer to the allocated
/// memory in \a memptr. The value of \a alignment shall be a multiple of
/// sizeof(void*), that is also a power of two. Upon successful completion,
/// the value pointed to by \a memptr will be a multiple of alignment.
///
/// Note that memory allocated with posix_memalign() can be freed with
/// free(). 
///
/// In the event of errors, the contents of \a memptr will be returned as 0.
///
/// The following return codes are mandated by POSIX, and are always returned
/// in the event of the specified condition. 
///
/// \retval 0 Success
///
/// \retval -EINVAL The value of the \a alignment parameter is not a power of
/// two multiple of sizeof(void*).
///
/// \retval -ENOMEM There is insufficient memory available with the requested
/// alignment.
///
/// The following return codes are implementation-specific and may be
/// configured to cause a kernel panic.
///
/// \retval -BYTE_POOL_INVALID_OBJECT The \a _malloc_byte_pool is NULL (0).
///
/// \retval -BYTE_POOL_INVALID_ARGUMENT The \a memptr argument is NULL (0).

int
posix_memalign(void** memptr, size_t alignment, size_t size)
{
    int rc;

    if (((alignment & (alignment - 1)) != 0) ||
        (alignment < sizeof(void*))) {
        rc = -EINVAL;
    } else {
        rc = byte_pool_alloc_aligned(_malloc_byte_pool, memptr, size,
                                     floor_log2(alignment));
        if (!rc && (*memptr == 0)) {
            rc = -ENOMEM;
        }
    }
    if (rc && memptr) {
        *memptr = 0;
    }
    return rc;
}


/// Print a dump of a byte pool, including the header and allocation report
///
/// \param stream The stream to receive the dump
///
/// \param pool The BytePool object to dump
///
/// \bug This routine is not thread safe.

void
byte_pool_report(FILE* stream, BytePool* pool)
{
    ByteBlock* block;
    uint8_t* p8;
    uint32_t* p32;
    int i;

    fprintf(stream, ">>> Byte Pool Report for Pool %p <<<\n", pool);

    fprintf(stream, ">>> BytePool Object Dump <<<\n");

#define DUMPIT(x, fmt)                                  \
    fprintf(stream, "%20s : " #fmt "\n", #x, pool->x)

    DUMPIT(first_row, %d);
    DUMPIT(last_row, %d);
    DUMPIT(columns, %d);
    DUMPIT(log2_columns, %d);
    DUMPIT(column_mask, 0x%08x);
    DUMPIT(minimum_block_size, %d);
    DUMPIT(free, %p);
    DUMPIT(column_status, %p);
    DUMPIT(row_status, %lu);
    DUMPIT(first_block, %p);
    DUMPIT(big_block, %p);
    DUMPIT(last_block, %p);
    DUMPIT(initial_allocation, %d);
    DUMPIT(bytes_allocated, %d);
    DUMPIT(bytes_free, %d);
    DUMPIT(blocks_allocated, %d);
    DUMPIT(blocks_free, %d);
    DUMPIT(alloc_calls, %d);
    DUMPIT(free_calls, %d);

    fprintf(stream, ">>> Byte Pool Allocation Report <<<\n");
    fprintf(stream, 
            ">>> status :   address  :  size  :      binary[0:7]      : ASCII[0:7] <<<\n");

    for (block = pool->first_block->next; 
         block != pool->last_block;
         block = block->next) {

        fprintf(stream, "      %c    : %p : %6zu : ",
                (block_is_free(block) ? 'F' : 'A'),
                block, block_size(block));

        p8 = (uint8_t*)((unsigned long)block + sizeof(ByteBlock));
        p32 = (uint32_t*)p8;

        fprintf(stream, "0x%08x 0x%08x : ", p32[0], p32[1]);
        for (i = 0; i < 8; i++) {
            if (isprint(p8[i])) {
                fputc(p8[i], stream);
            } else {
                fputc('.', stream);
            }
        }                
        fputc('\n', stream);
    }

    fprintf(stream, ">>> End Report <<<\n");
}
