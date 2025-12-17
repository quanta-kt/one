/**
 * Portable viritual memory allocation and memory mapped IO.
 * Conditionally compiles to use mmap (POSIX) or VirutalAlloc (WIN32).
 */

#include "mmio.h"

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif

static const mmio_mapping MMIO_MAPPING_INIT = {
    .ptr = NULL,
    .length = 0,
#ifdef _WIN32
    .mapping = NULL,
#endif
};

static int64_t filesize(file_des fd) {
#ifdef _WIN32
    LARGE_INTEGER length;
    if (!GetFileSizeEx(fd, &length)) {
        return -1;
    }

    return (int64_t) length.QuadPart;
#else
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        return -1;
    }

    return (int64_t) st.st_size;
#endif
}

size_t mmio_get_page_size() {
#ifdef _WIN32
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwPageSize;
#else
    return (size_t) sysconf(_SC_PAGE_SIZE);
#endif
}

void* mmio_virtual_alloc(size_t size) {
#ifdef _WIN32
    void* ret = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (ret == NULL) {
        fprintf(stderr, "VirtualAlloc: unable to allocate\n");
        return NULL;
    }
#else
    void* ret = mmap(NULL, size,
        PROT_READ | PROT_WRITE,
        MAP_ANONYMOUS | MAP_PRIVATE,
        -1, 0);

    if (ret == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }
#endif

    return ret;
}

void mmio_virtual_free(void* ptr, size_t size) {
#ifdef _WIN32
    (void) size;

    if (!VirtualFree(ptr, 0, MEM_RELEASE)) {
        fprintf(stderr, "VirtualFree: unable to free\n");
    }
#else
    if (munmap(ptr, size) == -1) {
        perror("munmap");
    }
#endif
}

bool mmio_mm_fd(file_des fd, mmio_mapping *out) {
    bool ret = true;

    *out = MMIO_MAPPING_INIT;

    int64_t length = filesize(fd);
    if (length < 0) {
        goto err;
    }

    out->length = (uint64_t) length;

    /* Creating a 0-length file mapping is not supported on either platforms.
       We handle that ourselves as a special case. */
    if (out->length == 0) {
        out->ptr = NULL;
        out->length = 0;

#ifdef _WIN32
        out->mapping = NULL;
#endif

        return true;
    }

#ifdef _WIN32
    out->mapping = CreateFileMapping(fd, NULL, PAGE_READONLY, 0, 0, NULL);
    if (out->mapping == NULL) {
        ret = false;
        goto err;
    }

    void* ptr = MapViewOfFile(out->mapping, FILE_MAP_READ, 0, 0, 0);
    if (ptr == NULL) {
        ret = false;
        goto err;
    }


    out->ptr = ptr;
#else
    void* ptr = mmap(NULL, out->length, PROT_READ, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED) {
        ret = false;
        perror("mmap");
        goto err;
    }

    out->ptr = ptr;
#endif

out:
    return ret;

err:
    fprintf(stderr, "mmio: failed to create file mapping\n");
    mmio_unmap(out);
    goto out;
}

bool mmio_mm_path(char* path, mmio_mapping *out) {
    bool ret;

#ifdef _WIN32
    HANDLE hFile = CreateFile(
            path,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "CreateFile: unable to open file '%s'\n", path);
        return false;
    }

    ret = mmio_mm_fd(hFile, out);

    CloseHandle(hFile);
    return ret;
#else
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return false;
    }

    ret = mmio_mm_fd(fd, out);
    close(fd);
    return ret;
#endif
}

void mmio_unmap(mmio_mapping *mapping) {

#ifdef _WIN32
    if (mapping->ptr != NULL) {
        UnmapViewOfFile(mapping->ptr);
    }

    if (mapping->mapping != NULL) {
        CloseHandle(mapping->mapping);
    }
#else
    if (mapping->ptr != NULL) {
        munmap(mapping->ptr, mapping->length);
    }
#endif
}

