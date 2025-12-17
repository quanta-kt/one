#include "arena.h"
#include "mmio_alloc.h"

#include <assert.h>

typedef struct {
    int i;
    char c;
    float f;
} testtype;

#include <stdio.h>

int main() {
    printf("hi");

    arena* ar = arena_make(&mmio_alloc, 1024);
    allocator_t alloc = arena_get_alloc(ar);

    testtype* v0 = ALLOC(&alloc, testtype);
    v0->i = 123;
    v0->c = 'X';
    v0->f = 456;

    for (size_t i = 0; i < 1024; i++) {
        testtype* v1 = ALLOC(&alloc, testtype);
        v1->i = 321;
        v1->c = 'x';
        v1->f = 654;

        assert(v0->i == 123);
        assert(v0->c == 'X');
        assert(v0->f == 456);
    }

    long* arr = ALLOC_ARRAY(&alloc, long, 32);
    arr[0] = 12345;
    arr[20] = 54321;

    arr = RESIZE_ARRAY(&alloc, arr, long, 32, 64); 
    assert(arr[0] == 12345);
    assert(arr[20] == 54321);

    arena_destroy(ar);

    return 0;
}

