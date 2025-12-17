/**
 * mmap/VirtualAlloc based allocator.
 * Allocates everthing with mmap/VirtualAlloc
 */

#include "mmio_alloc.h"

#include "mmio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void* mmio_allocate(void* _ctx, void* ptr, size_t old_size, size_t new_size) {
    (void) _ctx;

    if (new_size == 0) {
        mmio_virtual_free(ptr, old_size);
        return NULL;
    }

    void* ret = mmio_virtual_alloc(new_size);
    if (ret == NULL) {
        exit(1);
    }

    if (ptr != NULL) {
        memcpy(ret, ptr, old_size < new_size ? old_size : new_size);
        mmio_virtual_free(ptr, old_size);
    }

    return ret;
}

allocator_t mmio_alloc = {
    .alloc = mmio_allocate,
    .ctx = NULL,
};

