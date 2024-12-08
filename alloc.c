#include "alloc.h"

#include <stdbool.h>
#include <stdlib.h>

#ifdef TRACE_ALLOC

#include <stdio.h>
static size_t total_allocated = 0;

struct allocation {
    void* ptr;
    bool freed;
};

static struct allocation allocated[10240] = {0};
static size_t len = 0;

struct allocation* find(void* ptr) {
    for (size_t i = 0; i < len; i++) {
        struct allocation* alloc = &allocated[i];
        if (alloc->ptr == ptr) {
            return alloc;
        }
    }

    return NULL;
}

#endif  // TRACE_ALLOC

void* gpa_alloc(void* _ctx, void* ptr, size_t _old_size, size_t new_size) {
    void* new_ptr;

    if (new_size == 0) {
        free(ptr);
        new_ptr = NULL;
    } else {
        new_ptr = realloc(ptr, new_size);
    }

#ifdef TRACE_ALLOC
    if (_old_size == 0 && new_size > 0) {
        allocated[len++] = (struct allocation){.ptr = new_ptr, .freed = false};
    } else if (_old_size > 0) {
        struct allocation* alloc = find(ptr);

        if (alloc == NULL || alloc->freed) {
            fprintf(
                stderr,
                "trying to resize a previously unallocated pointer.\n"
            );
            allocated[len++] =
                (struct allocation){.ptr = new_ptr, .freed = false};
        } else {
            alloc->ptr = new_ptr;
            alloc->freed = new_size == 0;
        }
    }

    long long diff = (long long)new_size - (long long)_old_size;
    total_allocated += diff;

    fprintf(stderr, "%s: %lld\n", diff > 0 ? "allocated" : "freed", diff);
#endif  // TRACE_ALLOC

    return new_ptr;
}

allocator_t* gpa() {
    static allocator_t allocator = (allocator_t){
        .alloc = gpa_alloc,
        .ctx = NULL,
    };

    return &allocator;
}
