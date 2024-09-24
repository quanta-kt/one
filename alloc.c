#include "alloc.h"

#include <stdbool.h>
#include <stdlib.h>

void* gpa_alloc(void* _ctx, void* ptr, size_t _old_size, size_t new_size) {
    return realloc(ptr, new_size);
}

allocator_t* gpa() {
    static allocator_t allocator = (allocator_t){
        .alloc = gpa_alloc,
        .ctx = NULL,
    };

    return &allocator;
}
