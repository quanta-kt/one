#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>

typedef void* (*allocate_fn)(
    void* ctx, void* ptr, size_t old_size, size_t new_size
);

typedef struct {
    void* ctx;
    allocate_fn alloc;
} allocator_t;

#define ALLOC(allocator, type) \
    ((allocator)->alloc((allocator)->ctx, NULL, 0, sizeof(type)))
#define FREE(allocator, ptr, type) \
    ((allocator)->alloc((allocator)->ctx, ptr, sizeof(type), 0))

#define ALLOC_ARRAY(allocator, type, count) \
    ((allocator)->alloc((allocator)->ctx, NULL, 0, sizeof(type) * count))
#define FREE_ARRAY(allocator, ptr, type, count) \
    ((allocator)->alloc((allocator)->ctx, ptr, sizeof(type) * count, 0))
#define RESIZE_ARRAY(allocator, ptr, type, old_count, new_count) \
    ((allocator)->alloc(                                         \
        (allocator)->ctx,                                        \
        ptr,                                                     \
        sizeof(type) * old_count,                                \
        sizeof(type) * new_count                                 \
    ))

// General purpose allocator, uses realloc from stdlib
allocator_t* gpa();

#endif  // ALLOC_H
