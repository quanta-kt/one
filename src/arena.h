/**
 * Arena allocator
 */

#ifndef ARENA_H
#define ARENA_H

#include "alloc.h"

typedef struct _arena arena;

/**
 * Initializes a new arena allocator.
 *
 * @param base The base allocator to get memory from
 * @param block_size Size of arena blocks to allocate
 *                   from the base allocator.
 */
arena* arena_make(allocator_t* base, size_t block_size);

/**
 * Destroys the arena and frees all the allocations made.
 */
void arena_destroy(arena* self);

/**
 * Returns the allocator which can be used to make allocations from this arena.
 */
allocator_t arena_get_alloc(arena* self);

#endif  // ARENA_H

