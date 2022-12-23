
#include "xpipc.h"
#include "snex.h"
#include <cstdio>

int main(void) {
    struct xpipc_shm shm;
    if (!xpipc_shm_open("/snex-ppu", sizeof(struct snex_shared), &shm)) {
        printf("%s\n", shm.last_error);
        return 1;
    }

    struct snex_shared *snex = (struct snex_shared *) shm.mapped;
    if (snex->version != 1) {
        printf("error; snex version != 1; got %d\n", snex->version);
        return 1;
    }

    for (;;) {
        // wait for NMI syn:
        int32_t expected_state = 1;
        if (!snex->v1.sync_state.compare_exchange_strong(expected_state, 2)) {
            usleep(1000);
            continue;
        }

        printf("recv NMI syn\n");

        // ack:
        for (int n = 0; n < 10000; n++) {
            expected_state = 3;
            if (snex->v1.sync_state.compare_exchange_strong(expected_state, 4)) {
                printf("send NMI syn-ack\n");
                break;
            }
        }
    }

    printf("done\n");
    return 0;
}
