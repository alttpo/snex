
#include "xpipc.h"
#include <cstdio>

static void xpipc_set_last_error(struct xpipc_shm *shm) {
#if XPIPC_WINDOWS
    sprintf(shm->last_error, "err %08lx", GetLastError());
#elif XPIPC_UNIX
    strerror_r(errno, shm->last_error, 4096);
#endif
}

// takes a POSIX standard name like "/file" and strips the leading / off for Windows.
static void xpipc_shm_set_name(const char *name, struct xpipc_shm *shm) {
#if XPIPC_WINDOWS
    shm->name[0] = 0;
    if (name[0] == '/') {
        //strcpy(shm->name, "Global\\");
        strncat(shm->name, name+1, 255);
    } else {
        strncpy(shm->name, name, 255);
    }
    //printf("%s\n", shm->name);
#else
    strncpy(shm->name, name, 255);
#endif
}

bool xpipc_shm_create(const char *name, size_t size, struct xpipc_shm *shm) {
#if XPIPC_WINDOWS
    xpipc_shm_set_name(name, shm);
    shm->size = size;
    shm->hMapFile = NULL;
    shm->mapped = NULL;

    shm->hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        shm->size >> 32,
        shm->size & 0xFFFFFFFF,
        shm->name
    );

    if (shm->hMapFile == NULL) {
        xpipc_set_last_error(shm);
        return false;
    }

    shm->mapped = MapViewOfFile(
        shm->hMapFile,   // handle to map object
        FILE_MAP_ALL_ACCESS, // read/write permission
        0,
        0,
        shm->size
    );
    if (shm->mapped == NULL) {
        xpipc_set_last_error(shm);

        CloseHandle(shm->hMapFile);

        shm->hMapFile = NULL;
        return false;
    }

    return true;
#elif XPIPC_UNIX
    xpipc_shm_set_name(name, shm);
    shm->size = size;
    shm->fd = -1;
    shm->mapped = NULL;

    shm->fd = shm_open(shm->name, O_RDWR | O_CREAT | O_EXCL, 0666);
    if (shm->fd < 0) {
        xpipc_set_last_error(shm);
        return false;
    }

    if (ftruncate(shm->fd, shm->size) < 0) {
        xpipc_set_last_error(shm);
        return false;
    }

    shm->mapped = mmap(
        NULL,
        shm->size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        shm->fd,
        0
    );
    if (shm->mapped == MAP_FAILED) {
        xpipc_set_last_error(shm);
        shm->mapped = NULL;
        close(shm->fd);
        shm->fd = -1;
        return false;
    }

    return true;
#else
#  error Unsupported platform
#endif
}

bool xpipc_shm_open(const char *name, size_t size, struct xpipc_shm *shm) {
#if XPIPC_WINDOWS
    xpipc_shm_set_name(name, shm);
    shm->size = size;
    shm->hMapFile = NULL;
    shm->mapped = NULL;

    shm->hMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        shm->name
    );

    if (shm->hMapFile == NULL) {
        xpipc_set_last_error(shm);
        return false;
    }

    shm->mapped = MapViewOfFile(
        shm->hMapFile,   // handle to map object
        FILE_MAP_ALL_ACCESS, // read/write permission
        0,
        0,
        shm->size
    );
    if (shm->mapped == NULL) {
        xpipc_set_last_error(shm);

        CloseHandle(shm->hMapFile);

        shm->hMapFile = NULL;
        return false;
    }

    return true;
#elif XPIPC_UNIX
    xpipc_shm_set_name(name, shm);
    shm->size = size;
    shm->fd = -1;
    shm->mapped = NULL;

    shm->fd = shm_open(shm->name, O_RDWR, 0666);
    if (shm->fd < 0) {
        xpipc_set_last_error(shm);
        return false;
    }

    shm->mapped = mmap(
        NULL,
        shm->size,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        shm->fd,
        0
    );
    if (shm->mapped == MAP_FAILED) {
        xpipc_set_last_error(shm);
        shm->mapped = NULL;
        close(shm->fd);
        shm->fd = -1;
        return false;
    }

    return true;
#else
#  error Unsupported platform
#endif
}

bool xpipc_shm_close(struct xpipc_shm *shm) {
#if XPIPC_WINDOWS
    if (!UnmapViewOfFile(shm->mapped)) {
        xpipc_set_last_error(shm);
        return false;
    }
    shm->mapped = NULL;

    if (!CloseHandle(shm->hMapFile)) {
        xpipc_set_last_error(shm);
        return false;
    }
    shm->hMapFile = NULL;

    return true;
#elif XPIPC_UNIX
    munmap(shm->mapped);
    shm->mapped = NULL;

    close(shm->fd);
    shm->fd = -1;

    shm_unlink(shm->name);

    return true;
#endif
}
