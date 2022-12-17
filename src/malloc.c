// Copyright 2022 Vadim Sukhomlinov

// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "noc_internal/common.h"

#ifndef MALLOC_ALIGN
// Set reasonable default to size of pointer.
#define MALLOC_ALIGN sizeof(void *)
#endif

// Ideas:
// 1. move head into allocated space, effectively sbrk_start will point to that
// chunk
//    - this also allows to only use size to compute 'next' address
// 2. On realloc() try to merge "left"

// Order of fields is important, as size becomes size of allocated block and
// block starts at 'next'
struct free_chunk {
    size_t size;
    // Pointer to next free chunk if not allocated
    // TODO: make a union with byte buffer
    struct free_chunk *next;
};

// We need header size to be aligned, so by maintaining alignment
// of chunk we can maintain alignment of data.
STATIC_ASSERT(offsetof(struct free_chunk, next) % MALLOC_ALIGN == 0);

// Compute size of data block within chunk
static inline size_t chunk_data_size(const struct free_chunk *c) {
    return c->size - offsetof(struct free_chunk, next);
}

// Get address of chunk from address of data block.
static inline struct free_chunk *chunk_from_data(void *ptr) {
    // (void *) to silence cast alignment warning
    return (struct free_chunk *)(void *)((char *)ptr -
                                         offsetof(struct free_chunk, next));
}

// Get address of data block from chunk
static inline void *chunk_to_data(struct free_chunk *chunk) {
    // (void *) to silence cast alignment warning
    return (void *)(&chunk->next);
}

// Compute end of chunk address (or `adjacent` on the right )
static inline void *chunk_end(struct free_chunk *c) {
    return (char *)c + c->size;
}

// Compute size of chunk adjusted for alignment
static inline size_t chunk_size_for_data(size_t size) {
    size_t pad = size % MALLOC_ALIGN;
    if (pad) size += MALLOC_ALIGN - pad;
    size += offsetof(struct free_chunk, next);
    // Padding again not needed as 'next' is MALLOC_ALIGN
    return size;
}

// Minimal feasible allocation size. We can reuse `next` field for data.
const size_t MALLOC_MIN_SIZE = sizeof(struct free_chunk);

// Internal state, should be in .bss
struct malloc_state {
    // Initial address returned by first ever call to sbrk().
    // Allocated heap is between sbrk_start & sbrk_end.
    void *sbrk_start;
    // Top address allocated by sbrk(), also expected value
    // for next sbrk() call.
    void *sbrk_end;

    // Pointer to first free memory chunk if any.
    struct free_chunk *head;

    // TODO: is there a need for lock?
    // volatile uintptr_t lock;
};

static inline struct free_chunk **
malloc_state_head(struct malloc_state *state) {
    return &state->head;
}

// Internal functions with explicit state parameter.
// TODO: consider make it public to work on arena
static void *noc_malloc(size_t size, struct malloc_state *state);
static void noc_free(void *ptr, struct malloc_state *state);
static void *noc_realloc(void *ptr, size_t size, struct malloc_state *state);

// Wrapper around sbrk() to get aligned address.
// Used to maintain invariant that all chunks are aligned. Practically alignment
// may happen only on first allocation, as we always request aligned chunks.
static void *aligned_sbrk(size_t size, struct malloc_state *state) {
    // Check for signed overflow since we cast to (intptr_t).
    if (size > INTPTR_MAX) return SBRK_FAILURE;

    char *p = sbrk((intptr_t)size);

    if (p == SBRK_FAILURE) {
        errno = ENOMEM;
        return p;
    }

    state->sbrk_end = p + size;

    intptr_t pad_size = (intptr_t)p % (intptr_t)MALLOC_ALIGN;

    if (pad_size) {
        // sbrk() returned not properly aligned address.
        pad_size = MALLOC_ALIGN - pad_size;
        char *pad = sbrk(pad_size);
        if (pad != state->sbrk_end) {
            // Something unexpected, may be OS aligns sizes too?
            LOG("sbrk returned unexpected address %p vs. %p\n", pad,
                state->sbrk_end);
            // TODO: try to return it back (?) update malloc_state.sbrk_end?
            return SBRK_FAILURE;
        }
        state->sbrk_end = pad + pad_size;
        p += pad_size;
    }
    if (state->sbrk_start == NULL) state->sbrk_start = p;
    return p;
}

// Make free chunk at given address.
static inline void insert_free_chunk(struct free_chunk *c, size_t size,
                                     struct malloc_state *state) {
    // Set valid size and reuse `free` to add it to list of free chunks
    c->size = size;
    noc_free(chunk_to_data(c), state);
}

// Attempt to grow chunk if it is at top of sbrk()-allocated memory.
static bool chunk_grow(struct free_chunk *c, size_t new_size,
                       struct malloc_state *state) {
    if (c == NULL) return false;

    char *chunk_e = chunk_end(c);

    // Check if this chunk is last
    if (chunk_e != state->sbrk_end) return false;

    size_t add_size = new_size - c->size;
    // Make it possible to create a free chunk in case of failures
    if (add_size < MALLOC_MIN_SIZE) add_size = MALLOC_MIN_SIZE;

    char *heap = aligned_sbrk(add_size, state);

    if (heap == chunk_e) {
        // If got what expected adjust size and return
        c->size += add_size;
        return true;
    }

    if (heap != SBRK_FAILURE) {
        // WTF? other users of sbrk()?
        // TODO: revise recovery if ever needed.
        insert_free_chunk((struct free_chunk *)(void *)heap, add_size, state);
    }
    return false;
}

// The pointer returned if the allocation succeeds is suitably aligned so
// that it may be assigned to a pointer to any type of object with a
// fundamental alignment requirement and then used to access such an object
// or an array of such objects in the space allocated (until the space is
// explicitly deallocated). The lifetime of an allocated object extends from
// the allocation until the deallocation. Each such allocation shall yield a
// pointer to an object disjoint from any other object. The pointer returned
// points to the start (lowest byte address) of the allocated space. If the
// space cannot be allocated, a null pointer is returned. If the size of the
// space requested is zero, the behavior is implementation-defined: either a
// null pointer is returned, or the behavior is as if the size were some
// nonzero value, except that the returned pointer shall not be used to
// access an object.
void *noc_malloc(size_t size, struct malloc_state *state) {
    struct free_chunk **parent = malloc_state_head(state);
    struct free_chunk *chunk = NULL;
    struct free_chunk **min_parent = NULL;

    // Implementation defined behavior. Return non-null pointer?
    if (size == 0) return NULL;

    size = chunk_size_for_data(size);
    size_t min_extra = -1U;

    while ((chunk = *parent) != NULL) {
        // Find minimum sized free chunk to allocate from
        if (chunk->size >= size) {
            size_t extra = chunk->size - size;
            if (extra < min_extra) {
                min_extra = extra;
                min_parent = parent;
                // Almost exact match, stop search
                if (extra < MALLOC_ALIGN) break;
            }
        }
        parent = &chunk->next;
    }

    // `parent` points to last block
    // We found suitable block
    if (min_parent != NULL) {
        chunk = *min_parent;
        struct free_chunk *next = chunk->next;

        // if remaining size is less than minimally allocatable, allocate full
        // chunk, make parent to point to next free chunk if any.
        if (min_extra < MALLOC_MIN_SIZE) {
            size += min_extra;
            *min_parent = chunk->next;
        } else {
            // Split large chunk in two, return first
            struct free_chunk *other =
                (struct free_chunk *)(void *)((char *)chunk + size);
            other->size = min_extra;
            other->next = next;
            *min_parent = other;
        }
    } else {
        LOG("parent=%p, *parent=%p\n", parent, *parent);

        // Can we extend a last block that may be too small?
        if (chunk_grow(*parent, size, state)) return chunk_to_data(*parent);

        // If there is no suitable chunk, request from environment
        chunk = aligned_sbrk(size, state);
        if (chunk == SBRK_FAILURE) return NULL;
    }
    // Set the size of the block
    chunk->size = size;
    return chunk_to_data(chunk);
}

// The free function causes the space pointed to by ptr to be deallocated, that
// is, made available for further allocation. If ptr is a null pointer, no
// action occurs. Otherwise, if the argument does not match a pointer earlier
// returned by a memory management function, or if the space has been
// deallocated by a call to free or realloc, the behavior is undefined.
void noc_free(void *ptr, struct malloc_state *state) {
    struct free_chunk *chunk;
    struct free_chunk *prev;
    // parent of `prev` chunk
    struct free_chunk **parent = malloc_state_head(state);

    // Check pointer actually could come from this malloc()
    if (ptr < state->sbrk_start || ptr >= state->sbrk_end) return;

    chunk = chunk_from_data(ptr);
    if (chunk_end(chunk) > state->sbrk_end) {
        LOG("free: invalid chunks size? %zu\n", chunk->size);
        return;
    }

    // Note, this also partially clears content.
    // TODO: Should we clean memory? set it to some value?
    chunk->next = NULL;

    // As an invariant, there should be no consequent free chunks, so
    // merge if we create adjacent chunks.
    // All free chunks are sorted by address.
    while (((prev = *parent) != NULL) && (prev < chunk)) {
        // `Left` merge blocks together
        if (chunk_end(prev) == chunk) {
            LOG("free: `left` merge blocks prev=%p, chunk=%p\n", prev, chunk);
            prev->size += chunk->size;
            chunk = prev;
            prev = prev->next;
            goto no_insert;
        } else if (chunk == prev) {
            // Probably it is double free
            LOG("free: Double free %p?\n", chunk);
            return;
        }
        parent = &prev->next;
    }
    LOG("free: inserting free chunk %p\n", chunk);
    chunk->next = prev;
    *parent = chunk;

no_insert:

    // TODO: check if we can release memory via sbrk()?
    // if chunk_end(chunk) == state->sbrk_end?

    // `Right` merge blocks together if needed
    if (chunk_end(chunk) == prev) {
        LOG("free: `right` merge blocks chunk=%p\n", chunk);
        chunk->size += prev->size;
        chunk->next = prev->next;
    }
}

// The realloc function deallocates the old object pointed to by ptr and
// returns a pointer to a new object that has the size specified by size. The
// contents of the new object shall be the same as that of the old object prior
// to deallocation, up to the lesser of the new and old sizes. Any bytes in the
// new object beyond the size of the old object have indeterminate values. If
// ptr is a null pointer, the realloc function behaves like the malloc function
// for the specified size. Otherwise, if ptr does not match a pointer earlier
// returned by a memory management function, or if the space has been
// deallocated by a call to the free or realloc function, the behavior is
// undefined. If memory for the new object cannot be allocated, the old object
// is not deallocated and its value is unchanged.
// The realloc function returns a pointer to the new object (which may have the
// same value as a pointer to the old object), or a null pointer if the new
// object could not be allocated.
void *noc_realloc(void *ptr, size_t size, struct malloc_state *state) {
    if (ptr == NULL) return noc_malloc(size, state);
    if (size == 0) {
        noc_free(ptr, state);
        return NULL;
    }

    if (ptr < state->sbrk_start || ptr > state->sbrk_end) return NULL;

    struct free_chunk *chunk = chunk_from_data(ptr);
    void *chunk_e = chunk_end(chunk);
    // If data is damaged do nothing
    if (chunk_e > state->sbrk_end) return NULL;

    // Update to metadata / alignment
    size_t alloc_size = chunk_size_for_data(size);
    size_t old_size = chunk->size;

    if (old_size < alloc_size) {
        // If block is last to allocated space, try to allocate more
        if (chunk_grow(chunk, alloc_size, state)) {
            old_size = alloc_size;
        } else {
            // Search for adjacent chunk
            struct free_chunk **parent = malloc_state_head(state);
            struct free_chunk *adjacent;
            // Check if we can merge with adjacent free chunks
            while (((adjacent = *parent) != NULL) &&
                   ((void *)adjacent <= chunk_e)) {
                size_t adjacent_size = adjacent->size;
                if (chunk_end(adjacent) == chunk) {
                    // adjacent is on the `left`, so merge and copy data
                    // from current `chunk` to adjacent, and update size
                    *parent = adjacent->next;
                    // chunks can overlap
                    memmove(chunk_to_data(adjacent), chunk_to_data(chunk),
                            chunk_data_size(chunk));
                    old_size += adjacent_size;
                    adjacent->size = old_size;
                    LOG("realloc: merge `left` old_size = %zu, new_size = "
                        "%zu\n",
                        old_size - adjacent_size, old_size);
                    chunk = adjacent;
                    ptr = chunk_to_data(chunk);
                    // Parent is already set, `chunk_e` remains same
                    continue;
                } else if (adjacent == chunk_e) {
                    // if adjacent is on the `right` of current chunk
                    // merge adjacent chunk to current and remove it
                    // from the list of free chunks.
                    *parent = adjacent->next;
                    old_size += adjacent_size;
                    chunk->size = old_size;
                    LOG("realloc: merge `right` old_size = %zu, new_size = "
                        "%zu\n",
                        old_size - adjacent_size, old_size);
                    adjacent->size = 0;
                    adjacent->next = NULL;
                    break;
                }
                parent = &adjacent->next;
            }
        }
    }

    // If chunk is oversized, create a free chunk split it into twoCan we reduce
    // current chunk?
    if (old_size >= alloc_size) {
        size_t extra = old_size - alloc_size;
        if (extra >= MALLOC_MIN_SIZE) {
            LOG("realloc: split larger block sized %zu into %zu and %zu\n",
                old_size, alloc_size, extra);
            chunk->size = alloc_size;
            insert_free_chunk(chunk_end(chunk), extra, state);
        }
        return ptr;
    }

    // Fall back to simple allocate/copy
    void *new = noc_malloc(size, state);
    if (new) {
        memcpy(new, ptr, chunk_data_size(chunk));
        free(ptr);
    }
    return new;
}

void *noc_calloc(size_t nmemb, size_t size, struct malloc_state *state) {
    size_t total_size;
    if (__builtin_mul_overflow(nmemb, size, &total_size)) {
        LOG("calloc: overflow %zu x %zu!\n", nmemb, size);
        return NULL;
    }
    void *ptr = noc_malloc(size, state);
    if (ptr) memset(ptr, 0, total_size);
    return ptr;
}

// Global 'heap' state.
static struct malloc_state malloc_state;

void *malloc(size_t size) { return noc_malloc(size, &malloc_state); }

void *realloc(void *ptr, size_t size) {
    return noc_realloc(ptr, size, &malloc_state);
}

void free(void *ptr) { return noc_free(ptr, &malloc_state); }

void *calloc(size_t nmemb, size_t size) {
    return noc_calloc(nmemb, size, &malloc_state);
}

// TEST functions
size_t mem_free(void) {
    size_t count = 0;
    struct free_chunk *head = *malloc_state_head(&malloc_state);
    while (head) {
        count += head->size;
        head = head->next;
    }
    return count;
}
