
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


int main() {
    struct xpipc_shm shm;

    if (!xpipc_shm_create("/snex-ppu", sizeof(struct snex_shared), &shm)) {
        printf("%s\n", shm.last_error);
        return 1;
    }

    struct snex_shared *snex = (struct snex_shared *)shm.mapped;
    snex->version = 1;
    snex->v1.last_client_idx = -1;

    if (!xpipc_shm_close(&shm)) {
        printf("%s\n", shm.last_error);
        return 1;
    }

    return 0;
}
