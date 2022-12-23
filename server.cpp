
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
    snex->v1.sync_state = 0;

    for (;;) {
        // switch to state 1 NMI-sync:
        printf("send NMI syn\n");
        snex->v1.sync_state.store(1);
        usleep(1000);

        // expect NMI acknowledged:
        int32_t expected_state = 2;
        for (int n = 0; n < 10000; n++) {
            expected_state = 2;
            if (snex->v1.sync_state.compare_exchange_strong(expected_state, 3)) {
                printf("recv NMI syn-ack\n");
                break;
            }
        }

        usleep(1000);

        // reset state to IDLE:
        snex->v1.sync_state.store(0);
        printf("present\n");
        usleep(14000);
    }

    if (!xpipc_shm_close(&shm)) {
        printf("%s\n", shm.last_error);
        return 1;
    }

    return 0;
}
