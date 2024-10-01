#include "vec.h"

#include <string.h>

#define VEC_INITIAL_SIZE 8

void vec_free(vec* v) {
    FREE_ARRAY(v->allocator, v->items, uint8_t, v->item_size * v->capacity);

    v->items = NULL;
    v->capacity = 0;
    v->len = 0;
}

void* _vec_push(vec* v, void* item) {
    if (v->len == v->capacity) {
        size_t new_capacity =
            v->capacity == 0 ? VEC_INITIAL_SIZE : v->capacity * 2;

        v->items = RESIZE_ARRAY(
            v->allocator,
            v->items,
            uint8_t,
            v->item_size * v->capacity,
            v->item_size * new_capacity
        );
        v->capacity = new_capacity;
    }

    void* ptr = v->items + v->item_size * v->len++;
    memcpy(ptr, item, v->item_size);

    return ptr;
}

void* _vec_get(vec* v, size_t inx) {
    if (inx >= v->len) return NULL;

    return v->items + v->item_size * inx;
}
