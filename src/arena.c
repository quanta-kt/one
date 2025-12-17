#include "arena.h"

#include <string.h>
#include <stdint.h>

typedef struct _allocation {
    uintptr_t ptr;

    size_t size;
    size_t offset;

    struct _allocation* prev;
} allocation;

struct _arena {
    /* The base allocator to get memory from */
    allocator_t* base;

    /* List of allocations made with base */
    allocation* list;

    /* Total available unused memory we can use for future allocations */
    size_t free;

    /* Size of blocks to allocate from base allocator.
     * Actual size of the block MAY be greater than this if an allocation
     * greater than block size is requested. */
    size_t block_size;
};

static inline uintptr_t align_up(uintptr_t ptr) {
    return (ptr + (_Alignof(max_align_t) - 1)) &
           ~(uintptr_t)(_Alignof(max_align_t) - 1);
}

/* Allocates a block from base allocator */
static allocation* allocate_block(allocator_t* base, size_t size,
                                  allocation* prev) {
    void* raw = ALLOC_ARRAY(base, char, size);

    uintptr_t block = (uintptr_t) raw;
    allocation* header = (allocation*) raw;

    *header = (allocation){
        .ptr = block,
        .size = size,
        .offset = (size_t) (align_up(block + sizeof(allocation)) - block),
        .prev = prev,
    };

    return header;
}

static void* allocate_in_block(allocation* block, size_t size) {
    void* ret = (void*) (block->ptr + block->offset);
    block->offset = align_up((uintptr_t) (block->ptr + block->offset + size)) - block->ptr;
    return ret;
}

static void* allocate_in_new_block(arena* self, size_t size) {
    size_t header = align_up(sizeof(allocation));
    size_t needed = header + align_up(size);

    size_t block_size = needed > self->block_size ? needed : self->block_size;

    self->list = allocate_block(
        self->base,
        block_size,
        self->list
    );

    void* ret = allocate_in_block(self->list, size);

    self->free += self->list->size - self->list->offset;

    return ret;
}

arena* arena_make(allocator_t* base, size_t block_size) {
    allocation* list = allocate_block(base, block_size, NULL);
    arena* ret = allocate_in_block(list, sizeof(arena));

    *ret = (arena) {
        .base = base,
        .list = list,
        .free = list->size - list->offset,
        .block_size = block_size,
    };

    return ret;
}

void arena_destroy(arena* self) {
    allocation* curr = self->list;

    while (curr != NULL) {
        allocation* t = curr;
        curr = curr->prev;

        /* The allocation object itself is allocated within its block.
           Freeing this frees allocation object. */
        FREE_ARRAY(self->base, (void*) t->ptr, char, t->size);
    }


    /* Since arena object itself is allocated in the first block, freeing all
       the block frees it as well. */
}

static void* arena_alloc(void* ctx, void* ptr, size_t old_size,
                         size_t new_size) {
    void* ret = NULL;
    arena* self = (arena*) ctx;

    if (new_size > self->free) {
        ret = allocate_in_new_block(self, new_size);
    } else {
        /* cumulative free space available in current block we are seeing + all
         * the blocks that come after. */
        size_t cum_free = self->free;

        for (allocation* curr = self->list;
             /* Stop early if cumulative free space is less than `new_size` as
              * no existing block can satisfy it. */
             curr != NULL && cum_free >= new_size;
             curr = curr->prev) {

            size_t available = curr->size - curr->offset;

            if (available >= new_size) {
                /* Allocate_in_block may increate offset more than new_size to
                 * account for alignment. We can't determine the actual value
                 * without duplicating that logic here.
                 *
                 * Instead, we subtract all of that block's free size and
                 * restore it again after allocate_in_block call. */
                self->free -= curr->size - curr->offset;
                ret = allocate_in_block(curr, new_size);
                self->free += curr->size - curr->offset;
                break;
            }

            cum_free -= available;
        }

        if (ret == NULL) {
            ret = allocate_in_new_block(self, new_size);
        }
    }

    if (ptr != NULL) {
        memcpy(ret, ptr, old_size < new_size ? old_size : new_size);
    }

    return ret;
}

allocator_t arena_get_alloc(arena* self) {
    return (allocator_t) {
        .ctx = self,
        .alloc = arena_alloc,
    };
}

