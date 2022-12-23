
#include "xpipc.h"
#include "snex.h"
#include <cstdio>

int main() {
    struct xpipc_shm shm;

    if (!xpipc_shm_create("/snex-ppu", sizeof(struct snex_shared), &shm)) {
        printf("%s\n", shm.last_error);
        return 1;
    }

    struct snex_shared *snex = (struct snex_shared *)shm.mapped;
    snex->version = 1;
    snex->v1.ppu_ready = 0;

    for (;;) {
        int32_t ready = 1;
        if (snex->v1.ppu_ready.compare_exchange_weak(ready, 0)) {
            printf("screen ready\n");
        }

        usleep(16000);
    }

    if (!xpipc_shm_close(&shm)) {
        printf("%s\n", shm.last_error);
        return 1;
    }

    return 0;
}
