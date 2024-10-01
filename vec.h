#ifndef VEC_H
#define VEC_H

#include <stddef.h>
#include <stdint.h>

#include "alloc.h"

typedef struct {
    void* items;

    // size of a single item
    size_t item_size;

    size_t len;
    size_t capacity;

    allocator_t* allocator;
} vec;

#define make_vec(type, alloc)                                              \
    (vec) {                                                                \
        .capacity = 0, .len = 0, .items = NULL, .item_size = sizeof(type), \
        .allocator = alloc,                                                \
    }

void vec_free(vec* v);

void* _vec_push(vec* v, void* item);
#define vec_push(v, type, item) ((type*)_vec_push(v, (void*)item))

void* _vec_get(vec* v, size_t inx);
#define vec_get(v, type, inx) ((type*)_vec_get(v, inx))

#define foreach_vec_item(v, type, item)                   \
    for (item = (type*)((v)->items);                      \
         item && item < ((type*)((v)->items)) + (v)->len; \
         ++item)

#endif
