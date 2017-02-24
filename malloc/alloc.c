/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>


#define SBRK_FAILURE ((void *) -1)
// Logging
#define LOG 0
#define LOG_COMB 0
#define LOG_CALL 0
#define LOG_LOOP 0
// Magic Numbers
#define MIN_SPLIT_SIZE 131072

typedef struct metadata {
    void *prev_mem;
    void *next_mem;

    void *prev_free;
    void *next_free;

    size_t size;
    char empty;
} metadata;

void *mem_tail  = NULL;
void *free_head = NULL;

void insert_block_into_free_list(void *block) {
    if (LOG_CALL) fprintf(stderr, "insert_block_into_free_list: %p\n", block);
    metadata *meta      = (metadata *) block;
    metadata *meta_head = (metadata *) free_head;

    // INSERT AT FRONT
    meta->prev_free = NULL;
    meta->next_free = meta_head;

    if (meta_head != NULL) {
        meta_head->prev_free = meta;
    }

    // CHANGE HEAD
    free_head = meta;
}

void remove_block_from_free_list(void *block) {
    if (LOG_CALL) fprintf(stderr, "remove_block_from_free_list: %p\n", block);
    metadata *meta = (metadata *) block;

    metadata *prev_meta = (metadata *) meta->prev_free;
    metadata *next_meta = (metadata *) meta->next_free;
    meta->prev_free = NULL;
    meta->next_free = NULL;

    // REMOVE BLOCK FROM LIST
    if (prev_meta != NULL) {
        prev_meta->next_free = next_meta;
    }

    if (next_meta != NULL) {
        next_meta->prev_free = prev_meta;
    }

    // CHANGE HEAD IF NEEDED
    if (block == free_head) {
        free_head = next_meta;
    }
}

/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    if (LOG_CALL) fprintf(stderr, "calloc: %zu, %zu\n", num, size);

    size_t m_size = num * size;
    void *addr = malloc(m_size);
    if (addr)
        memset(addr, 0, m_size);
    return addr;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc_new_block(size_t size) {
    if (LOG_CALL) fprintf(stderr, "malloc_new_block: %zu\n", size);

    void *block = sbrk(size + sizeof(metadata));
    if (block == SBRK_FAILURE)
        return NULL;

    metadata *meta  = (metadata *) block;
    meta->prev_mem  = mem_tail;
    meta->next_mem  = NULL;
    meta->next_free = NULL;
    meta->prev_free = NULL;
    meta->size      = size;
    meta->empty     = 0;

    if (mem_tail)
        ((metadata *) mem_tail)->next_mem = block;
    mem_tail = block;

    return block + sizeof(metadata);
}

// aka malloc_old_spice
void malloc_old_block_spilt(size_t size, void *block) {
    void *extra_block    = block + sizeof(metadata) + size;

    metadata *meta       = (metadata *) block;
    // fprintf(stderr, "meta->size: %zu, size: %zu\n", meta->size, size);

    metadata *extra_meta = (metadata *) extra_block;

    metadata *next_block = (metadata *) meta->next_mem;

    // CORRECT LINKED LISTS
    extra_meta->next_mem = next_block;
    if (next_block) {
        next_block->prev_mem = extra_meta;
    }

    meta->next_mem = extra_meta;
    extra_meta->prev_mem = meta;

    // CORRECT METADATA
    // meta->next_free       = NULL;
    // meta->prev_free       = NULL;

    // extra_meta->next_free = NULL;
    // extra_meta->prev_free = NULL;

    extra_meta->size      = meta->size - (sizeof(metadata) + size);
    meta->size            = size;

    extra_meta->empty     = 1;

    // INSERT EXTRA BLOCK INTO THE FREE LIST
    insert_block_into_free_list(extra_meta);
}

void *malloc_old_block(size_t size, void *block) {
    metadata *meta = (metadata *) block;

    if (meta->size >= size + sizeof(metadata) + MIN_SPLIT_SIZE)
        malloc_old_block_spilt(size, block);

    meta->empty = 0;
    return block + sizeof(metadata);
}

size_t reuseCount = 0;

void *malloc(size_t size) {
    if (LOG_CALL) fprintf(stderr, "malloc: %zu\n", size);

    if (!size) {
        return NULL;
    }

    void *curr = free_head;
    metadata *meta;

    if (LOG_LOOP) fprintf(stderr, "(%zu) start: ", size);
    while (curr != NULL) {
        meta = (metadata *) curr;
        if (LOG_LOOP) fprintf(stderr, "meta: (%p, %zu), ", meta, meta->size);
        if (meta->size >= size) {
            reuseCount++;
            if (LOG || LOG_LOOP) fprintf(stderr, "end (reuse, %zu)\n", reuseCount);
            remove_block_from_free_list(meta);
            return malloc_old_block(size, meta);
        }
        curr = meta->next_free;

    }
    if (LOG) fprintf(stderr, "end (new, %zu)\n", size);
    return malloc_new_block(size);
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    if (LOG_CALL) fprintf(stderr, "free: %p\n", ptr - sizeof(metadata));

    if (ptr == NULL) {
        return;
    }

    // MOVE POINTER BACK
    void *block = ptr - sizeof(metadata);
    metadata *meta = (metadata *) block;

    // PREVENT DOUBLE FREE
    if (meta->empty == 1) {
        return;
    }

    // MERGE FREE NEIGHBORS
    metadata *next_meta = (metadata *) meta->next_mem;
    metadata *prev_meta = (metadata *) meta->prev_mem;

    int prev_cont = prev_meta && ((void *) prev_meta) + sizeof(metadata) + prev_meta->size == meta;
    int next_cont = next_meta && ((void *) meta)      + sizeof(metadata) + meta->size      == next_meta;

    if (next_meta && next_meta->empty && prev_meta && prev_meta->empty &&
        prev_cont && next_cont) {

        // remove_block_from_free_list(prev_meta);
        remove_block_from_free_list(next_meta);

        metadata *next_next_meta = (metadata *) next_meta->next_mem;
        if (next_next_meta) {
            next_next_meta->prev_mem = prev_meta;
        }
        prev_meta->next_mem = next_next_meta;

        if (next_meta == mem_tail) {
            mem_tail = prev_meta;
        }

        prev_meta->size += meta->size + next_meta->size + 2*sizeof(metadata);
        // prev_meta->empty = 1;
        if (LOG_COMB) fprintf(stderr, "comb: 3\n");
        // insert_block_into_free_list(prev_meta);
        return;
    }
    //(meta != mem_tail) &&
    if (prev_meta && prev_meta->empty &&
        prev_cont) {
        // remove_block_from_free_list(meta);
        // remove_block_from_free_list(prev_meta);
        if (next_meta) {
            next_meta->prev_mem = prev_meta;
        }
        prev_meta->next_mem = next_meta;

        if (meta == mem_tail) {
            mem_tail = prev_meta;
        }
        if (LOG_COMB) fprintf(stderr, "comb: 2-prev\n");

        prev_meta->size += meta->size + sizeof(metadata);
        // prev_meta->empty = 1;
        // insert_block_into_free_list(prev_meta);
        return;
    }

    if (next_meta && (next_meta != mem_tail) && next_meta->empty &&
        next_cont) {
        remove_block_from_free_list(next_meta);

        metadata *next_next_meta = (metadata *) next_meta->next_mem;

        if (next_next_meta) {
            next_next_meta->prev_mem = meta;
        }
        meta->next_mem = next_next_meta;

        // if (next_meta == mem_tail) {
        //     mem_tail = meta;
        // }

        meta->size += sizeof(metadata) + next_meta->size;
        meta->empty = 1;
        insert_block_into_free_list(meta);
        return;
    }

    // SET EMPTY
    meta->empty = 1;

    // INSERT BLOCK INTO THE FREE LIST
    insert_block_into_free_list(meta);
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    if (LOG_CALL) fprintf(stderr, "free: %p, %zu\n", ptr - sizeof(metadata), size);

    if (ptr == NULL) {
        return malloc(size);
    }

    void *block = ptr - sizeof(metadata);
    metadata *meta = (metadata *) block;

    if (meta->size >= size) {
        return ptr;
    }

    void *mem = malloc(size);
    if (mem == NULL) {
        return NULL;
    }

    memcpy(mem, ptr, meta->size);
    free(ptr);

    return mem;
}
