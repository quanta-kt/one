#ifndef VEC_H
#define VEC_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "alloc.h"

#define VEC_INITIAL_SIZE 8

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

#define VEC2(item_type)         \
    struct {                    \
        item_type* items;       \
        size_t len;             \
        size_t capacity;        \
        allocator_t* allocator; \
    }

#define vec2_make(allocator) {NULL, 0, 0, allocator}

#define vec2_free(v)               \
    do {                           \
        FREE_ARRAY(                \
            (v)->allocator,        \
            (v)->items,            \
            typeof(*((v)->items)), \
            (v)->capacity          \
        );                         \
    } while (0)

#define vec2_push(v, item)                                                 \
    do {                                                                   \
        if ((v)->len == (v)->capacity) {                                   \
            size_t new_capacity =                                          \
                (v)->capacity == 0 ? VEC_INITIAL_SIZE : (v)->capacity * 2; \
                                                                           \
            (v)->items = RESIZE_ARRAY(                                     \
                (v)->allocator,                                            \
                (v)->items,                                                \
                typeof(*item),                                             \
                (v)->capacity,                                             \
                new_capacity                                               \
            );                                                             \
            (v)->capacity = new_capacity;                                  \
        }                                                                  \
                                                                           \
        typeof(item) ptr = (v)->items + (v)->len++;                        \
        memcpy(ptr, item, sizeof(item));                                   \
                                                                           \
    } while (0);

#define vec2_get(v, inx) inx >= v->len ? NULL : v->items[inx]

#define vec2_foreach(v, item) \
    item = (v)->items;        \
    for (size_t i = 0; i < (v)->len; i++, (item)++)
