
#include "xpipc.h"

#ifndef __cplusplus
# include <stdatomic.h>
#else
# include <atomic>
# define _Atomic(X) std::atomic< X >
#endif
#include <cstdio>

const int snex_ppu_screen_size = 1024*1024;
struct snex_ppu_screen {
    // 15-bit BGR color, MSB = 1 to override
    uint16_t color[snex_ppu_screen_size];
    // 0000 ppll
    //      ||||
    //      ||\+-- layer    (0=BG1, 1=BG2, 2=BG3, 3=OBJ)
    //      \+---- priority (0..1 for BG1-3, 0..3 for OBJ)
    uint8_t  layer_prio[snex_ppu_screen_size];
};

struct snex_client {
    _Atomic(int32_t)        ppu_frames_ttl;
    struct snex_ppu_screen  ppu_main;
    struct snex_ppu_screen  ppu_sub;
};

struct snex_shared_v1 {
    _Atomic(int32_t)    last_client_idx;
    struct snex_client  clients[1];
};

struct snex_shared {
    int32_t                 version;

    struct snex_shared_v1   v1;
};

bool xpipc_shm_create(const char *name, size_t size, struct xpipc_shm *shm) {
#if XPIPC_WINDOWS
    strncpy(shm->name, name, 255);
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
        sprintf(shm->last_error, "err %08lx", GetLastError());
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
        sprintf(shm->last_error, "err %08lx", GetLastError());

        CloseHandle(shm->hMapFile);

        shm->hMapFile = NULL;
        return false;
    }

    return true;
#elif XPIPC_UNIX
    strncpy(shm->name, name, 255);
    shm->size = size;
    shm->fd = -1;
    shm->mapped = NULL;

    shm->fd = shm_open(shm->name, O_RDWR | O_CREAT | O_EXCL, 0666);
    if (shm->fd < 0) {
        perror("shm_open");
        return false;
    }

    if (ftruncate(shm->fd, shm->size) < 0) {
        perror("ftruncate");
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
        perror("mmap");
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
    UnmapViewOfFile(shm->mapped);
    shm->mapped = NULL;

    CloseHandle(shm->hMapFile);
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

int main() {
    struct xpipc_shm shm;

    if (!xpipc_shm_create("/snex-ppu", sizeof(struct snex_shared), &shm)) {
        printf("%s\n", shm.last_error);
        return 1;
    }

    struct snex_shared *snex = (struct snex_shared *)shm.mapped;
    snex->version = 1;
    snex->v1.last_client_idx = -1;

    xpipc_shm_close(&shm);

    return 0;
}
