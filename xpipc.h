
#ifndef SNEX_XPIPC_H
#define SNEX_XPIPC_H

#include "xplat.h"

// OS-specific includes:
#if XPIPC_WINDOWS
// Windows OS:
#  include <windows.h>
#  include <unistd.h>
#elif XPIPC_UNIX
// Linux | Apple | Unix
#  include <sys/mman.h>
#  include <sys/stat.h>
#  include <unistd.h>
#else
#  error Unsupported platform
#endif

// Common includes:
#include <cstdint>
#include <cstring>
#include <fcntl.h>

// Common definitions:
struct xpipc_shm;

// OS-specific definitions:
#if XPIPC_WINDOWS
struct xpipc_shm {
    char    name[256];
    size_t  size;

    HANDLE  hMapFile;
    LPVOID  mapped;

    char    last_error[4096];
};
#elif XPIPC_UNIX
struct xpipc_shm {
    char    name[256];
    size_t  size;

    int     fd;
    void*   mapped;

    char    last_error[4096];
};
#endif

bool xpipc_shm_create(const char *name, size_t size, struct xpipc_shm *shm);
bool xpipc_shm_open(const char *name, size_t size, struct xpipc_shm *shm);
bool xpipc_shm_close(struct xpipc_shm *shm);

#endif //SNEX_XPIPC_H
