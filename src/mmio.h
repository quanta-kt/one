#ifndef MMIO_H
#define MMIO_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#define file_des HANDLE
#else
#define file_des int
#endif

struct mmio_mapping;

typedef struct mmio_mapping {
    /* The base pointer to the memory mapped file */
    void *ptr;

    /* Length of file contents mapped at ptr */
    uint64_t length;

#ifdef _WIN32
    HANDLE mapping;
#endif
} mmio_mapping;

/**
 * Allocates virtual memory with no file backing
 */
void* mmio_virtual_alloc(size_t size);

/**
 * Frees memory allocated with mmio_virtual_alloc
 */
void mmio_virtual_free(void* ptr, size_t size);

/**
 * Returns the page size (minimum sizes mmio_virtual_alloc can allocate)
 */
size_t mmio_get_page_size();

/**
 * Maps a file descriptor to virtual memory.
 */
bool mmio_mm_fd(file_des fd, mmio_mapping *out);

/**
 * Maps a file to virtual memory.
 */
bool mmio_mm_path(char* path, mmio_mapping *out);

/**
 * Unmaps a memory mapping
 */
void mmio_unmap(mmio_mapping *mapping);

#endif // MMIO_H

